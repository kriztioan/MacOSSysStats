/**
 *  @file   mem_mac.c
 *  @brief  Memory Usage
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories mem_mac.c -o mem
 *
 ***********************************************/

#include <mach/mach.h>
#include <mach/mach_host.h>
#include <stdio.h>

int main(void) {

  mach_msg_type_number_t count = HOST_VM_INFO_COUNT;

  vm_statistics64_data_t vm_stat;

  if (KERN_SUCCESS != host_statistics64(mach_host_self(), HOST_VM_INFO,
                                        (host_info_t)&vm_stat, &count)) {

    printf("{error:1}");

    return -1;
  }

  float sum = vm_stat.active_count + vm_stat.inactive_count +
              vm_stat.wire_count + vm_stat.free_count;

  printf("{\"free\":%f, \"active\":%f, \"inactive\":%f, \"wired\":%f, "
         "\"error\":0}",
         (double)(vm_stat.free_count / sum),
         (double)(vm_stat.active_count / sum),
         (double)(vm_stat.inactive_count / sum),
         (double)(vm_stat.wire_count / sum));

  return 0;
}
