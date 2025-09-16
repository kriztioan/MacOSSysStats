/**
 *  @file   swap.c
 *  @brief  Swap Usage
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2023-08-28
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories swap_mac.c -o swap
 *
 ***********************************************/

#include <stdio.h>
#include <sys/sysctl.h>

int main(void) {

  int error = 0;

  struct xsw_usage xsw_usage_s = {0};

  int mib[2] = {CTL_VM, VM_SWAPUSAGE};

  size_t n;

  if (-1 == sysctl(mib, sizeof(mib) / sizeof(int), &xsw_usage_s, &n, NULL, 0)) {

    error = 1;
  }

  printf("{\"used\":%llu, \"total\":%llu, \"error\":%d}", xsw_usage_s.xsu_used,
         xsw_usage_s.xsu_total, error);

  return 0;
}
