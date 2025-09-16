/**
 *  @file   freq_mac2.c
 *  @brief  CPU Frequency
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2024-07-15
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -march=native -Weverything -Wno-poison-system-directories
 *    freq_mac2.c -o freq2
 *
 ***********************************************/

#include <stdio.h>
#include <sys/sysctl.h>

int main(void) {

  unsigned int freq;

  size_t size = sizeof(freq);

  int mib[2] = {CTL_HW, HW_CPU_FREQ}, err = -1;

  if (0 == sysctl(mib, 2, &freq, &size, NULL, 0)) {

    err = 0;

    freq /= 1000000;
  }

  printf("{\"frequency\":%u, \"error\":%d}", freq, err);

  return err;
}
