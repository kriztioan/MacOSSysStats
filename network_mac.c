/**
 *  @file   netstat_mac.c
 *  @brief  Network IO
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories network_mac.c -o network
 *
 ***********************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/if_mib.h>
#include <net/route.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {


  if(1 == argc) {

    printf("Usage: %s interface\n", argv[0]);

    return 1;
  }

  unsigned int ifindex = if_nametoindex(argv[1]);

  int mib[6] = {CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, (int)ifindex};

  size_t n;

  if (-1 == sysctl(mib,
		   sizeof(mib) / sizeof(int),
		   NULL,
		   &n,
		   NULL,
		   0)) {

    return 1;
  }

  char *records = (char *)malloc(n),
       *record;

  if (-1 == sysctl(mib,
		   sizeof(mib) / sizeof(int),
		   records,
		   &n,
		   NULL,
		   0)) {

    return 1;
  }

  struct if_msghdr2 if_msghdr2_s;

  //printf("n=%zu\n",n);

  for (record = records; record < records + n;
       record += if_msghdr2_s.ifm_msglen) {

    memcpy(&if_msghdr2_s, record, sizeof(struct if_msghdr2));

    /*printf("ifm_index=%d\nifm_type=%d\nifm_flags=%d\n",
           if_msghdr2_s.ifm_index, if_msghdr2_s.ifm_type,
           if_msghdr2_s.ifm_flags);*/

    if(ifindex != if_msghdr2_s.ifm_index ||
       RTM_IFINFO2 != if_msghdr2_s.ifm_type ||
       (if_msghdr2_s.ifm_flags & IFF_LOOPBACK)) {
  
      continue;
    }

    printf("%llu %llu\n",
           if_msghdr2_s.ifm_data.ifi_obytes,
           if_msghdr2_s.ifm_data.ifi_ibytes);

    free(records);

    return 0;
  }

  free(records);

  return 1;
}
