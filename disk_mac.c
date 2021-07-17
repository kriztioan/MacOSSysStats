/**
 *  @file   disk_mac.c
 *  @brief  Disk Usage
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories disk_mac.c -o disk
 *
 ***********************************************/

#include <sys/param.h>
#include <sys/mount.h>

#include <stdio.h>

    int main(int argc, char *argv[]) {

  struct statfs statfs_s;

  if(argc == 0 ||
     -1 == statfs(argv[1], &statfs_s)) {

    return -1;
  }

  printf("%f", 100.0 * (double)(statfs_s.f_blocks - statfs_s.f_bfree) / (double)statfs_s.f_blocks);

  return 0;
}
