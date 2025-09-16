/**
 *  @file   memtop_mac.c
 *  @brief  Top for Memory Usage
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories memtop_mac.c -o top
 *
 ***********************************************/

#include <libgen.h>
#include <libproc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct pinfo {
  float memoryusage;
  char path[PROC_PIDPATHINFO_MAXSIZE];
  pid_t pid;
};

static int cmp(const void *a, const void *b) {
  return (((const struct pinfo *)b)->memoryusage >
          ((const struct pinfo *)a)->memoryusage)
             ? 1
             : ((((const struct pinfo *)b)->memoryusage <
                 ((const struct pinfo *)a)->memoryusage)
                    ? -1
                    : 0);
}

int main(int argc, char *argv[]) {

  unsigned int n = argc > 1 ? (unsigned int)atoi(argv[1]) : 5;

  uint64_t total;
  size_t len = sizeof(total);
  int mib[2] = {CTL_HW, HW_MEMSIZE};
  sysctl(mib, 2, &total, &len, NULL, 0);

  int nproc_bytes = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
  pid_t *pids = (pid_t *)malloc((size_t)nproc_bytes);
  size_t nproc = (size_t)nproc_bytes / sizeof(pid_t);
  bzero(pids, nproc * sizeof(pid_t));
  proc_listpids(PROC_ALL_PIDS, 0, pids, (int)(nproc * sizeof(pid_t)));

  struct pinfo *info = (struct pinfo *)malloc(nproc * sizeof(struct pinfo));
  bzero(info, nproc * sizeof(pid_t));

  char path[PROC_PIDPATHINFO_MAXSIZE];
  struct proc_taskinfo taskinfo;
  size_t nvalid = 0;
  for (unsigned int i = 0; i < nproc; i++) {
    if (pids[i] != 0 && proc_pidinfo(pids[i], PROC_PIDTASKINFO, 0, &taskinfo,
                                     PROC_PIDTASKINFO_SIZE) > 0) {
      info[nvalid].pid = pids[i];
      info[nvalid].memoryusage =
          100.0f * (float)taskinfo.pti_resident_size / (float)total;
      bzero(path, PROC_PIDPATHINFO_MAXSIZE);
      proc_pidpath(pids[i], path, PROC_PIDPATHINFO_MAXSIZE);
      strcpy(info[nvalid++].path, basename(path));
    }
  }

  free(pids);

  qsort((void *)info, nvalid, sizeof(struct pinfo), cmp);

  printf("{\"top\":[");

  unsigned int i = 0;
  for (; i < (n - 1); i++)
    printf("{\"program\":\"%s\",\"resident\":%f},", info[i].path,
           (double)info[i].memoryusage);

  printf("{\"program\":\"%s\",\"resident\"%f}], \"error\":0}", info[i].path,
         (double)info[i].memoryusage);

  free(info);

  return 0;
}
