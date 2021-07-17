/**
 *  @file   battery_mac.c
 *  @brief  Battery Status
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories battery_mac.c
 *    -o battery -framework IOKit -framework Foundation
 *
 ***********************************************/

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPSKeys.h>
#include <IOKit/ps/IOPowerSources.h>

int main() {

  CFTypeRef blob = IOPSCopyPowerSourcesInfo();

  CFArrayRef sources = IOPSCopyPowerSourcesList(blob);

  if (CFArrayGetCount(sources) == 0) {

    return -1;
  }

  CFDictionaryRef source =
      IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, 0));

  if (NULL == source) {

    return -1;
  }

  long timeRemaining = 0L;

  CFNumberRef nValue;

  long currentCapacity, maxCapacity;

  nValue =
      (CFNumberRef)CFDictionaryGetValue(source, CFSTR(kIOPSCurrentCapacityKey));

  CFNumberGetValue(nValue, kCFNumberSInt64Type, &currentCapacity);

  nValue =
      (CFNumberRef)CFDictionaryGetValue(source, CFSTR(kIOPSMaxCapacityKey));

  CFNumberGetValue(nValue, kCFNumberSInt64Type, &maxCapacity);

  int batteryState = 0;

  CFStringRef sValue = (CFStringRef)CFDictionaryGetValue(
      source, CFSTR(kIOPSPowerSourceStateKey));

  if (kCFCompareEqualTo ==
      CFStringCompare(CFSTR(kIOPSACPowerValue), sValue, 0)) {

    nValue = (CFNumberRef)CFDictionaryGetValue(source,
                                               CFSTR(kIOPSTimeToFullChargeKey));

    CFNumberGetValue(nValue, kCFNumberSInt64Type, &timeRemaining);

    CFBooleanRef bValue =
        (CFBooleanRef)CFDictionaryGetValue(source, CFSTR(kIOPSIsChargingKey));

    if (CFBooleanGetValue(bValue)) {

      batteryState = 1;
    }
  } else if (kCFCompareEqualTo ==
             CFStringCompare(CFSTR(kIOPSBatteryPowerValue), sValue, 0)) {

    nValue =
        (CFNumberRef)CFDictionaryGetValue(source, CFSTR(kIOPSTimeToEmptyKey));

    CFNumberGetValue(nValue, kCFNumberSInt64Type, &timeRemaining);

    batteryState = 2;
  }

  CFRelease(blob);

  CFRelease(sources);

  if (-1 == timeRemaining) {

    timeRemaining = 0;
  }

  printf("%ld %1lu:%02lu %d\n", 100 * currentCapacity / maxCapacity,
         timeRemaining / 60, timeRemaining % 60, batteryState);

  return 0;
}
