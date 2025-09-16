/**
 *  @file   top_mac.c
 *  @brief  Top for CPU Usage
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories top_mac.c -o top
 *
 ***********************************************/

#include <libgen.h>
#include <libproc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct pinfo {
  uint64_t cputime;
  char path[PROC_PIDPATHINFO_MAXSIZE];
  pid_t pid;
  char padding[4];
};

static int cmp(const void *a, const void *b) {
  return (((const struct pinfo *)b)->cputime >
          ((const struct pinfo *)a)->cputime)
             ? 1
             : ((((const struct pinfo *)b)->cputime <
                 ((const struct pinfo *)a)->cputime)
                    ? -1
                    : 0);
}

int main(int argc, char *argv[]) {

  unsigned int n = argc > 1 ? (unsigned int)atoi(argv[1]) : 5;
  unsigned int t = argc > 2 ? (unsigned int)atoi(argv[2]) : 2;

  int nproc_bytes = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
  pid_t *pids = (pid_t *)malloc((size_t)nproc_bytes);
  size_t nproc = (size_t)nproc_bytes / sizeof(pid_t);
  bzero(pids, nproc * sizeof(pid_t));
  proc_listpids(PROC_ALL_PIDS, 0, pids, (int)(nproc * sizeof(pid_t)));

  struct pinfo *info = (struct pinfo *)malloc(nproc * sizeof(struct pinfo));
  bzero(info, nproc * sizeof(pid_t));

  char path[PROC_PIDPATHINFO_MAXSIZE];
  rusage_info_current rusage;
  ino_t timer = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW); // nanoseconds
  size_t nvalid = 0;
  for (unsigned int i = 0; i < nproc; i++) {
    if (pids[i] != 0 &&
        proc_pid_rusage(pids[i], RUSAGE_INFO_CURRENT, (void *)&rusage) == 0) {
      info[nvalid].pid = pids[i];
      info[nvalid].cputime =
          rusage.ri_user_time + rusage.ri_system_time; // nanoseconds
      bzero(path, PROC_PIDPATHINFO_MAXSIZE);
      proc_pidpath(pids[i], path, PROC_PIDPATHINFO_MAXSIZE);
      strcpy(info[nvalid].path, basename(path));
      ++nvalid;
    }
  }

  sleep(t);

  timer = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW) - timer;

  for (size_t i = 0; i < nvalid; i++) {
    if (proc_pid_rusage(info[i].pid, RUSAGE_INFO_CURRENT, (void *)&rusage) == 0)
      info[i].cputime =
          rusage.ri_user_time + rusage.ri_system_time - info[i].cputime;
    else
      info[i].cputime = info[i].cputime = 0;
  }

  free(pids);

  qsort((void *)info, nvalid, sizeof(struct pinfo), cmp);

  printf("{\"top\":[");

  unsigned int i = 0;
  for (; i < (n - 1); i++)
    printf("{\"program\":\"%s\",\"percentage\":%0.1f},", info[i].path,
           (double)(100.0f * (float)info[i].cputime / (float)timer));

  printf("{\"program\":\"%s\",\"percentage\":%0.1f}], \"error\":0}",
         info[i].path,
         (double)(100.0f * (float)info[i].cputime / (float)timer));

  free(info);

  return 0;
}
