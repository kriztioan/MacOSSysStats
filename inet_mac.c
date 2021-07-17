/**
 *  @file   inet_mac.c
 *  @brief  Network Monitor
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 inet_mac.c notify.c -o inet -framework Foundation
 *
 ***********************************************/

#include <arpa/inet.h>
#include <libproc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <search.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include <growl.h>
#include "notify.h"

#define ICON "/System/Library/CoreServices/Applications/Network Utility.app"

#define HDR_SIZE 64
#define BODY_SIZE 512
#define HOSTNAME_SIZE 128

struct inet {
  unsigned long timer;
  char remote_address[INET6_ADDRSTRLEN];
  int local_port;
  int remote_port;
  char protocol[4];
  char retain;
};

int inet_ip_cmp(const void *a, const void *b) {

  return strncmp(((const struct inet *)a)->remote_address,
                 ((const struct inet *)b)->remote_address, INET6_ADDRSTRLEN);
}

typedef struct _procinfo {
  pid_t pid;
  char name[64];
  char user[9];
  size_t inet_size;
  size_t ninet;
  struct inet *inet;
} procinfo;

int procinfo_init(procinfo *pi) {

  memset(pi, 0, sizeof(procinfo));

  pi->inet_size = 8;

  pi->inet = (struct inet *)malloc(pi->inet_size * sizeof(struct inet));

  if (pi->inet == NULL)
    return 1;

  memset(pi->inet, 0, pi->inet_size * sizeof(struct inet));

  return 0;
}

void free_procinfo(procinfo *pi) {

  if (pi->inet != NULL)
    free(pi->inet);

  memset(pi, 0, sizeof(procinfo));
}

void procinfo_grow(procinfo *pi) {

  if (pi->ninet == pi->inet_size) {

    pi->inet_size *= 2;

    pi->inet =
        (struct inet *)realloc(pi->inet, pi->inet_size * sizeof(struct inet));

    memset(pi->inet + (pi->ninet * sizeof(struct inet)), 0,
           pi->inet_size - pi->ninet);
  }
}

void procinfo_copy(procinfo *dst, procinfo *src) {

  if (src->ninet != dst->ninet)
    dst->inet =
        (struct inet *)realloc(dst->inet, src->inet_size * sizeof(struct inet));

  struct inet *ip = dst->inet;

  memcpy(dst, src, sizeof(procinfo));

  memcpy(ip, src->inet, src->inet_size * sizeof(struct inet));

  dst->inet = ip;
}

int procinfo_pid_cmp(const void *a, const void *b) {

  return ((procinfo *)a)->pid - ((procinfo *)b)->pid;
}

int procinfo_intersect(procinfo *pi1, procinfo *pi2, procinfo *pi3) {

  pi3->ninet = 0;

  int i;

  struct inet *in;

  for (i = 0; i < pi1->ninet; i++) {

    if ((in = lfind(&(pi1->inet[i]), pi2->inet, &(pi2->ninet),
                    sizeof(struct inet), inet_ip_cmp)) == NULL) {

      pi1->inet[i].retain = '+';

      pi1->inet[i].timer = 0;

      memcpy(&(pi3->inet[pi3->ninet++]), &(pi1->inet[i]), sizeof(struct inet));
    } else {

      if (++in->timer > 1)
        in->timer = 1;

      memcpy(&(pi3->inet[pi3->ninet++]), in, sizeof(struct inet));
    }

    procinfo_grow(pi3);
  }

  for (i = 0; i < pi2->ninet; i++) {

    if ((in = lfind(&(pi2->inet[i]), pi1->inet, &(pi1->ninet),
                    sizeof(struct inet), inet_ip_cmp)) == NULL) {

      if (pi2->inet[i].retain == '-')
        continue;

      pi2->inet[i].retain = '-';

      pi2->inet[i].timer = 0;

      memcpy(&(pi3->inet[pi3->ninet++]), &(pi2->inet[i]), sizeof(struct inet));

      procinfo_grow(pi3);
    }
  }

  return 0;
}

typedef struct _proclist {
  size_t proc_size;
  size_t nproc;
  procinfo *proc;
} proclist;

int proclist_init(proclist *plist) {

  memset(plist, 0, sizeof(proclist));

  plist->proc_size = 16;

  plist->proc = (procinfo *)malloc(plist->proc_size * sizeof(procinfo));

  if (plist->proc == NULL)
    return 1;

  int i;

  for (i = 0; i < plist->proc_size; i++)
    procinfo_init(&(plist->proc[i]));

  return 0;
}

void free_proclist(proclist *plist) {

  int i;

  if (plist->proc != NULL) {

    for (i = 0; i < plist->nproc; i++)
      free_procinfo(&(plist->proc[i]));

    free(plist->proc);
  }

  memset(plist, 0, sizeof(proclist));
}

void proclist_grow(proclist *plist) {

  int i;

  if (plist->nproc == plist->proc_size) {

    plist->proc_size *= 2;

    plist->proc =
        (procinfo *)realloc(plist->proc, plist->proc_size * sizeof(procinfo));

    for (i = plist->nproc; i < plist->proc_size; i++)
      procinfo_init(&(plist->proc[i]));
  }
}

int proclist_socket(pid_t pid, int fd, procinfo *pi) {

  struct socket_fdinfo sockinfo;

  struct in_sockinfo *si;

  if (PROC_PIDFDSOCKETINFO_SIZE != proc_pidfdinfo(pid, fd, PROC_PIDFDSOCKETINFO,
                                                  &sockinfo,
                                                  PROC_PIDFDSOCKETINFO_SIZE))
    return 1;

  if (sockinfo.psi.soi_family != AF_INET && sockinfo.psi.soi_family != AF_INET6)
    return 1;

  if (sockinfo.psi.soi_kind != SOCKINFO_TCP)
    return 1;

  if (sockinfo.psi.soi_proto.pri_tcp.tcpsi_state != TSI_S_ESTABLISHED)
    return 1;

  si = &sockinfo.psi.soi_proto.pri_tcp.tcpsi_ini;

  pi->inet[pi->ninet].remote_port = ntohs(si->insi_fport);

  if (pi->inet[pi->ninet].remote_port == 0)
    return 1;

  if (sockinfo.psi.soi_family == AF_INET6) {

    inet_ntop(sockinfo.psi.soi_family,
              (struct in6_addr *)&(si->insi_faddr.ina_6),
              pi->inet[pi->ninet].remote_address, INET6_ADDRSTRLEN);

    if (strncmp(pi->inet[pi->ninet].remote_address, "::1", INET6_ADDRSTRLEN) ==
        0)
      return 1;
  } else {

    inet_ntop(sockinfo.psi.soi_family,
              (struct in_addr *)&(si->insi_faddr.ina_46.i46a_addr4),
              pi->inet[pi->ninet].remote_address, INET6_ADDRSTRLEN);

    if (strncmp(pi->inet[pi->ninet].remote_address, "127.0.0.1",
                INET_ADDRSTRLEN) == 0)
      return 1;
  }

  lsearch(&(pi->inet[pi->ninet]), pi->inet, &(pi->ninet), sizeof(struct inet),
          inet_ip_cmp);

  procinfo_grow(pi);

  return 0;
}

int proclist_fill(proclist *plist) {

  while (plist->nproc)
    plist->proc[--plist->nproc].ninet = 0;

  int npid, npid_bytes;

  pid_t *pids = NULL;

  npid_bytes = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);

  if (npid_bytes == 0)
    return 1;

  pids = (pid_t *)malloc(npid_bytes);

  if (pids == NULL)
    return 1;

  npid = npid_bytes / sizeof(pid_t);

  bzero(pids, npid_bytes);

  proc_listpids(PROC_ALL_PIDS, 0, pids, npid_bytes);

  int nfd_bytes, nfd, i, j;

  struct proc_fdinfo *fdinfo = NULL;

  struct proc_bsdshortinfo bsdinfo;

  struct passwd *pwd;

  for (i = npid; i--;) {

    plist->proc[plist->nproc].pid = pids[i];

    proc_pidinfo(plist->proc[plist->nproc].pid, PROC_PIDT_SHORTBSDINFO, 0,
                 &bsdinfo, PROC_PIDT_SHORTBSDINFO_SIZE);

    nfd_bytes = proc_pidinfo(plist->proc[plist->nproc].pid, PROC_PIDLISTFDS, 0, NULL,
                       0);

    nfd = nfd_bytes / sizeof(struct proc_fdinfo);

    fdinfo = (struct proc_fdinfo *)malloc(nfd_bytes);

    if (fdinfo == NULL)
      continue;

    proc_pidinfo(plist->proc[plist->nproc].pid, PROC_PIDLISTFDS, 0,
                       fdinfo, nfd_bytes);

    for (j = 0; j < nfd; j++) {

      if (fdinfo[j].proc_fdtype == PROX_FDTYPE_SOCKET) {

        proclist_socket(plist->proc[plist->nproc].pid, fdinfo[j].proc_fd,
                        &(plist->proc[plist->nproc]));
      }
    }

    if (plist->proc[plist->nproc].ninet) {

      if ((pwd = getpwuid(bsdinfo.pbsi_uid)) != NULL)
        strncpy(plist->proc[plist->nproc].user, pwd->pw_name, 8);

      proc_name(plist->proc[plist->nproc].pid, plist->proc[plist->nproc++].name,
                63);

      proclist_grow(plist);
    }

    free(fdinfo);

    fdinfo = NULL;
  }

  free(pids);

  return 0;
}

int proclist_intersect(proclist *plist1, proclist *plist2, proclist *plist3) {

  procinfo *pi;

  int i, j;

  while (plist3->nproc)
    plist3->proc[--plist3->nproc].ninet = 0;

  for (i = 0; i < plist1->nproc; i++) {

    if ((pi = lfind(&(plist1->proc[i]), plist2->proc, &(plist2->nproc),
                    sizeof(procinfo), procinfo_pid_cmp)) == NULL) {

      for (j = 0; j < plist1->proc[i].ninet; j++)
        plist1->proc[i].inet[j].retain = '+';

      procinfo_copy(&(plist3->proc[plist3->nproc++]), &(plist1->proc[i]));

      proclist_grow(plist3);
    } else {

      procinfo_intersect(&(plist1->proc[i]), pi,
                         &(plist3->proc[plist3->nproc]));

      if (plist3->proc[plist3->nproc].ninet) {

        plist3->proc[plist3->nproc].pid = pi->pid;

        strncpy(plist3->proc[plist3->nproc].name, pi->name, 63);

        strncpy(plist3->proc[plist3->nproc++].user, pi->user, 9);

        proclist_grow(plist3);
      }
    }
  }

  for (i = 0; i < plist2->nproc; i++) {

    if (lfind(&(plist2->proc[i]), plist1->proc, &(plist1->nproc),
              sizeof(procinfo), procinfo_pid_cmp) == NULL) {

      for (j = 0; j < plist2->proc[i].ninet; j++) {

        if (plist2->proc[i].inet[j].retain == '-')
          continue;

        plist2->proc[i].inet[j].retain = '-';

        plist2->proc[i].inet[j].timer = 0;

        memcpy(&(plist3->proc[plist3->nproc]
                     .inet[plist3->proc[plist3->nproc].ninet++]),
               &(plist2->proc[i].inet[j]), sizeof(struct inet));

        procinfo_grow(&(plist3->proc[plist3->nproc]));
      }

      if (plist3->proc[plist3->nproc].ninet) {

        plist3->proc[plist3->nproc].pid = plist2->proc[i].pid;

        strncpy(plist3->proc[plist3->nproc].name, plist2->proc[i].name, 63);

        strncpy(plist3->proc[plist3->nproc++].user, plist2->proc[i].user, 9);

        proclist_grow(plist3);
      }
    }
  }

  return 0;
}

int finished;

void signalhandler(int signal) { finished = 1; }

void registersignals() {

  signal(SIGHUP, signalhandler);

  signal(SIGINT, signalhandler);

  signal(SIGQUIT, signalhandler);

  signal(SIGKILL, signalhandler);

  signal(SIGTERM, signalhandler);

  signal(SIGSTOP, signalhandler);
}

int main(int argc, char *argv[], char **envp) {

  finished = 0;

  registersignals();

  proclist plist1, plist2, plist3, tmp;

  proclist_init(&plist1);

  proclist_init(&plist2);

  proclist_init(&plist3);

  int i, j, offset;

  struct addrinfo *ai;

  char hdr[HDR_SIZE], body[BODY_SIZE], hostname[HOSTNAME_SIZE];

  struct timeval start_timer, end_timer, diff_timer;

  memset(&end_timer, 0, sizeof(struct timeval));

  init_notify();

  while (!finished) {

    gettimeofday(&start_timer, NULL);

    timersub(&start_timer, &end_timer, &diff_timer);

    if (diff_timer.tv_sec > 10) {

      usleep(5000000);

      proclist_fill(&plist2);
    }

    proclist_fill(&plist1);

    proclist_intersect(&plist1, &plist2, &plist3);

    for (i = 0; i < plist3.nproc; i++) {

      offset = 0;

      for (j = 0; j < plist3.proc[i].ninet; j++) {

        if (plist3.proc[i].inet[j].timer < 1) {

          getaddrinfo(plist3.proc[i].inet[j].remote_address, NULL, NULL, &ai);

          getnameinfo(ai->ai_addr, ai->ai_addrlen, hostname, HOSTNAME_SIZE,
                      NULL, 0, 0);

          if (offset)
            offset += snprintf(body + offset, BODY_SIZE - offset, ", %c%s",
                               plist3.proc[i].inet[j].retain, hostname);
          else
            offset += snprintf(body + offset, BODY_SIZE - offset, "%c%s",
                               plist3.proc[i].inet[j].retain, hostname);

          freeaddrinfo(ai);
        }
      }

      if (offset) {

        snprintf(hdr, HDR_SIZE, "%s (%zu)", plist3.proc[i].name,
                 plist3.proc[i].ninet);

        notify(hdr, plist3.proc[i].user, body, 0);

        // snprintf(hdr, HDR_SIZE, "%s (%zu)\n%s", plist3.proc[i].name,
        // plist3.proc[i].ninet, plist3.proc[i].user); growl("localhost",
        // "Network Utility", "nu-message", hdr, body, ICON, NULL, NULL);
        //printf("%s\n%s\n\n", hdr, body);
      }
    }

    tmp = plist2;

    plist2 = plist3;

    plist3 = tmp;

    gettimeofday(&end_timer, NULL);

    timersub(&end_timer, &start_timer, &diff_timer);

    if (diff_timer.tv_sec == 0)
      usleep(1000000 - diff_timer.tv_usec);
    else if (diff_timer.tv_sec > 10) {

      usleep(5000000);

      proclist_fill(&plist2);
    }
  }

  free_proclist(&plist1);

  free_proclist(&plist2);

  free_proclist(&plist3);

  return 0;
}
