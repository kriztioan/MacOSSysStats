/**
 *  @file   smc_mac.c
 *  @brief  SMC Temperature/Fan Speed
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -framework IOKit -Weverything -Wno-poison-system-directories
 *    smc_mac.c -o smc
 *
 ***********************************************/

#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <string.h>

#define KERNEL_INDEX_SMC 2

#define SMC_CMD_READ_BYTES 5
#define SMC_CMD_READ_KEYINFO 9

#define DATATYPE_FPE2 "fpe2"
#define DATATYPE_SP78 "sp78"

typedef struct {
  char major;
  char minor;
  char build;
  char reserved[1];
  UInt16 release;
} SMCKeyData_vers_t;

typedef struct {
  UInt16 version;
  UInt16 length;
  UInt32 cpuPLimit;
  UInt32 gpuPLimit;
  UInt32 memPLimit;
} SMCKeyData_pLimitData_t;

typedef struct {
  UInt32 dataSize;
  UInt32 dataType;
  char dataAttributes;
} SMCKeyData_keyInfo_t;

typedef char SMCBytes_t[32];

typedef struct {
  UInt32 key;
  SMCKeyData_vers_t vers;
  SMCKeyData_pLimitData_t pLimitData;
  SMCKeyData_keyInfo_t keyInfo;
  char result;
  char status;
  char data8;
  UInt32 data32;
  SMCBytes_t bytes;
} SMCKeyData_t;

typedef char UInt32Char_t[5];

typedef struct {
  UInt32Char_t key;
  UInt32 dataSize;
  UInt32Char_t dataType;
  SMCBytes_t bytes;
} SMCVal_t;

static io_connect_t conn;

static UInt32 _strtoul(const char *str, UInt32 size, UInt32 base) {

  UInt32 total = 0;

  for (UInt32 i = 0; i < size; i++) {
    if (base == 16)
      total += (UInt32)(str[i] << (size - 1 - i) * 8);
    else
      total += (UInt32)((unsigned char)(str[i] << (size - 1 - i) * 8));
  }

  return total;
}

static void _ultostr(char *str, UInt32 val) {

  str[0] = '\0';
  sprintf(str, "%c%c%c%c", (unsigned int)val >> 24, (unsigned int)val >> 16,
          (unsigned int)val >> 8, (unsigned int)val);
}

static kern_return_t SMCOpen(void) {

  kern_return_t result;
  io_iterator_t iterator;
  io_object_t device;

  CFMutableDictionaryRef matchingDictionary = IOServiceMatching("AppleSMC");
  result = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                        matchingDictionary, &iterator);
  if (result != kIOReturnSuccess) {
    fprintf(stderr, "IOServiceGetMatchingServices (%08x)\n", result);
    return 1;
  }

  device = IOIteratorNext(iterator);
  IOObjectRelease(iterator);
  if (device == 0) {
    fprintf(stderr, "no SMC found\n");
    return 1;
  }

  result = IOServiceOpen(device, mach_task_self(), 0, &conn);
  IOObjectRelease(device);
  if (result != kIOReturnSuccess) {
    fprintf(stderr, "IOServiceOpen (%08x)\n", result);
    return 1;
  }

  return kIOReturnSuccess;
}

static kern_return_t SMCClose() { return IOServiceClose(conn); }

static kern_return_t SMCCall(UInt32 index, SMCKeyData_t *inputStructure,
                      SMCKeyData_t *outputStructure) {

  size_t structureInputSize;
  size_t structureOutputSize;
  structureInputSize = sizeof(SMCKeyData_t);
  structureOutputSize = sizeof(SMCKeyData_t);

  return IOConnectCallStructMethod(conn, index, inputStructure,
                                   structureInputSize, outputStructure,
                                   &structureOutputSize);
}

static kern_return_t SMCReadKey(const UInt32Char_t key, SMCVal_t *val) {

  kern_return_t result;
  SMCKeyData_t inputStructure;
  SMCKeyData_t outputStructure;

  memset(&inputStructure, 0, sizeof(SMCKeyData_t));
  memset(&outputStructure, 0, sizeof(SMCKeyData_t));
  memset(val, 0, sizeof(SMCVal_t));

  inputStructure.key = _strtoul(key, 4, 16);
  inputStructure.data8 = SMC_CMD_READ_KEYINFO;

  result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
  if (result != kIOReturnSuccess)
    return result;

  val->dataSize = outputStructure.keyInfo.dataSize;
  _ultostr(val->dataType, outputStructure.keyInfo.dataType);
  inputStructure.keyInfo.dataSize = val->dataSize;
  inputStructure.data8 = SMC_CMD_READ_BYTES;

  result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
  if (result != kIOReturnSuccess)
    return result;

  memcpy(val->bytes, outputStructure.bytes, sizeof(outputStructure.bytes));

  return kIOReturnSuccess;
}

static double SMCReadTemperature(const char *key) {

  SMCVal_t val;
  kern_return_t result;

  result = SMCReadKey(key, &val);
  if (result == kIOReturnSuccess) {
    if (val.dataSize > 0) {
      if (strcmp(val.dataType, DATATYPE_SP78) == 0) {
        // convert sp78 value to temperature
        int intValue = val.bytes[0] * 256 + (unsigned char)val.bytes[1];
        return intValue / 256.0;
      }
    }
  }

  return 0.0;
}

static double SMCReadFanSpeed(const char *key) {

  SMCVal_t val;
  kern_return_t result;

  result = SMCReadKey(key, &val);
  if (result == kIOReturnSuccess) {
    if (val.dataSize > 0) {
      if (strcmp(val.dataType, DATATYPE_FPE2) == 0) {
        // convert fpe2 value to rpm
        int intValue =
            (unsigned char)val.bytes[0] * 256 + (unsigned char)val.bytes[1];
        return intValue / 4;
      }
    }
  }

  return 0.0;
}

static void CPUTemperatures() {

  size_t nkeys = 5, i;
  const char *keys[] = {"TB0T", "TC0P", "TM0P", "Ts0P", "Th1H"};

  printf("{\"sensors\":[");
  for (i = 0; i < nkeys; i++) {
    double temperature = SMCReadTemperature(keys[i]);
    printf("%0.3f", temperature);
    if (i != nkeys - 1)
      printf(",");
  }
  printf("],\"error\":0}");
}

#define IS_FLT(A) *(UInt32 *)(A.dataType) == *(UInt32 *)("flt ")
static float SMCReadFanRPM(const char *key) {

  SMCVal_t val;
  kern_return_t result;

  result = SMCReadKey(key, &val);
  if (result == kIOReturnSuccess) {
    if (val.dataSize > 0) {
      if (IS_FLT(val))
        return *((float *)val.bytes);
      else if (strcmp(val.dataType, DATATYPE_FPE2) == 0) {
        // convert fpe2 value to RPM
        return ntohs(*(UInt16 *)val.bytes) / 4.0f;
      }
    }
  }

  return -1.0f;
}

static void FanRPMs(void) {

  kern_return_t result;
  SMCVal_t val;
  size_t nkeys = 2, i;
  char *keys[] = {"F0Ac", "F1Ac"};

  printf("{\"sensors\":[");
  for (i = 0; i < nkeys; i++) {
    result = SMCReadKey(keys[i], &val);
    if (result != kIOReturnSuccess)
      continue;
    float rpm = SMCReadFanRPM(keys[i]);
    if (rpm < 0.0f)
      continue;
    printf("%.0f", rpm);
    if (i != nkeys - 1)
      printf(",");
  }
  printf("],\"error\":0}");
}

int main(int argc, char *argv[]) {

  int cpu = 0;
  int fan = 0;
  int c;

  while ((c = getopt(argc, argv, "fh")) != -1) {
    switch (c) {
    case 'f':
      fan = 1;
      break;
    case 'h':
    default:
      printf("usage: %s [-f]\n", argv[0]);
      return 1;
    }
  }
  if (!fan)
    cpu = 1;

  SMCOpen();
  if (cpu)
    CPUTemperatures();
  else if (fan)
    FanRPMs();
  SMCClose();

  return 0;
}
