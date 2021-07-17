/**
 *  @file   load_mac.c
 *  @brief  System Load
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories load_mac.c -o load
 *
 ***********************************************/

#include <stdlib.h>
#include <stdio.h>

int main() {

  double loadavg[3];

  getloadavg(loadavg, 3);

  printf("%3.2f %3.2f %3.2f", loadavg[0], loadavg[1], loadavg[2]);

  return 0; 
}

/*int main() {

  struct loadavg loadavg_s;

  int mib[2] = {CTL_VM, VM_LOADAVG};

  size_t s;

  if(-1 == sysctl(mib,
		  sizeof(mib) / sizeof(int),
		  &loadavg_s,
		  &s,
		  NULL,
		  0)) {
    printf("%s\n", strerror(errno));
    return 1;
  }

  printf("%3.2f %3.2f %3.2f", (double) loadavg_s.ldavg[0] / loadavg_s.fscale,
	                      (double) loadavg_s.ldavg[1] / loadavg_s.fscale,
	                      (double) loadavg_s.ldavg[2] / loadavg_s.fscale);

  return 0;
}*/
