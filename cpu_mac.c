/**
 *  @file   cpu_mac.c
 *  @brief  CPU Usage
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories cpu_mac.c -o cpu
 *
 ***********************************************/

#include <sys/sysctl.h>
#include <sys/types.h>
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>
#include <stdio.h>
#include <unistd.h>

int main() {

  processor_cpu_load_info_t cpu_info;

  mach_msg_type_number_t n_cpu_info;

  natural_t ncpu = 0U;

  kern_return_t err = host_processor_info(mach_host_self(),
					  PROCESSOR_CPU_LOAD_INFO,
					  &ncpu,
					  (processor_info_array_t *) &cpu_info,
					  &n_cpu_info);

  if(KERN_SUCCESS == err) {

    long used,
         usage = 0L,
         total = 0L;

    while(ncpu--) {

      used = cpu_info[ncpu].cpu_ticks[CPU_STATE_USER] +
             cpu_info[ncpu].cpu_ticks[CPU_STATE_SYSTEM] +
             cpu_info[ncpu].cpu_ticks[CPU_STATE_NICE];

      usage += used;

      total += used +
	          cpu_info[ncpu].cpu_ticks[CPU_STATE_IDLE];
    }

    printf("%ld %ld ", usage, total);

    return 0;
  }

  return -1;
}
