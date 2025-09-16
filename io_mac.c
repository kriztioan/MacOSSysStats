/**
 *  @file   io_mac.c
 *  @brief  Disk IO
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories io_mac.c
 *    -o io -framework IOKit -framework Foundation
 *
 ***********************************************/

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <IOKit/storage/IOMedia.h>
#include <mach/mach.h>

#if (MAC_OS_X_VERSION_MAX_ALLOWED < 120000) // before Monterey
#define kIOMainPortDefault kIOMasterPortDefault
#endif

int main(void) {

  long io_read, io_write, io_read_total = 0L, io_write_total = 0L;

  mach_port_t master_port = kIOMainPortDefault;

  CFMutableDictionaryRef match = IOServiceMatching("IOMedia");

  CFDictionaryAddValue(match, CFSTR(kIOMediaWholeKey), kCFBooleanTrue);

  io_iterator_t media_list;

  kern_return_t status =
      IOServiceGetMatchingServices(master_port, match, &media_list);

  if (status != KERN_SUCCESS) {

    printf("%ld %ld", io_read_total, io_write_total);

    exit(1);
  }

  io_registry_entry_t parent, media;

  CFNumberRef nValue;

  while ((media = IOIteratorNext(media_list))) {

    status = IORegistryEntryGetParentEntry(media, kIOServicePlane, &parent);

    if (status != KERN_SUCCESS) {

      continue;
    }

    if (IOObjectConformsTo(parent, "IOBlockStorageDriver")) {

      CFMutableDictionaryRef properties;

      CFDictionaryRef stats;

      status = IORegistryEntryCreateCFProperties(
          parent, (CFMutableDictionaryRef *)&properties, kCFAllocatorDefault,
          kNilOptions);

      if (status != KERN_SUCCESS) {
        IOObjectRelease(parent);

        IOObjectRelease(media);

        CFRelease(properties);

        continue;
      }

      stats = (CFDictionaryRef)CFDictionaryGetValue(
          properties, CFSTR(kIOBlockStorageDriverStatisticsKey));

      if (!stats) {
        IOObjectRelease(parent);

        IOObjectRelease(media);

        CFRelease(properties);

        continue;
      }

      nValue = (CFNumberRef)CFDictionaryGetValue(
          stats, CFSTR(kIOBlockStorageDriverStatisticsBytesReadKey));

      CFNumberGetValue(nValue, kCFNumberSInt64Type, &io_read);

      io_read_total += io_read;

      nValue = (CFNumberRef)CFDictionaryGetValue(
          stats, CFSTR(kIOBlockStorageDriverStatisticsBytesWrittenKey));

      CFNumberGetValue(nValue, kCFNumberSInt64Type, &io_write);

      io_write_total += io_write;

      IOObjectRelease(parent);

      IOObjectRelease(media);

      CFRelease(properties);
    }
  }

  IOObjectRelease(media_list);

  printf("%ld %ld\n", io_read_total, io_write_total);

  return 0;
}
