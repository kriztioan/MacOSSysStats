/**
 *  @file   netstat_mac.c
 *  @brief  Network Usage
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  API https://newosxbook.com/bonus/vol1ch16.html
 *
 *  Compile with:
 *    clang++ -O3 -std=c++17 -stdlib=libc++ -Weverything -Wno-c++98-compat
 *    -Wno-poison-system-directories -Wno-c++98-compat-pedantic netstat_mac.cpp
 *    -o netstat -framework CoreFoundation -framework NetworkStatistics
 *-framework CoreFoundation -F /System/Library/PrivateFrameworks
 *
 ***********************************************/

extern "C" {
#include "NStatManager.h"
}
#include <ctime>
// #include <google/dense_hash_map>
#include <unordered_map>
#include <vector>
// #include <memory_resource>

char *hr(char *buf, size_t size, float bytes);

/*  */

#define PROG_MAXSIZE 32

struct sample {
  float rx;
  float tx;
  char *prog;
};

char *hr(char *buf, size_t size, float bytes) {
  int i = 0;
  static constexpr char const *units[] = {"B", "kiB", "MiB", "GiB"};
  while (bytes >= 1024.0f) {
    bytes /= 1024.0f;
    ++i;
  }
  snprintf(buf, size, "%*.*f %*s", i > 0 ? 5 + i : 4, i,
           static_cast<double>(bytes), i > 0 ? 3 : 1, units[i]);
  return (buf);
}

struct ninfo {
  ino_t t1;
  ino_t t2;
  uint64_t rx1;
  uint64_t tx1;
  uint64_t rx2;
  uint64_t tx2;
  char prog[PROG_MAXSIZE];
  bool probed;
  char padding[7];
};

int main(int argc, char *argv[]) {

  // std::array<std::byte, 2048> pool;
  // std::pmr::monotonic_buffer_resource rsrc{std::data(pool),
  // std::size(pool)};
  // std::pmr::unordered_map<pid_t, struct ninfo> info {&rsrc};

  __block std::unordered_map<pid_t, struct ninfo> info;
  info.reserve(128);

  //__block google::dense_hash_map<pid_t, struct ninfo> info;
  // info.set_empty_key(-1);

  dispatch_queue_t queue = dispatch_queue_create(
      "com.christiaanboersma.netstat", DISPATCH_QUEUE_SERIAL);

  dispatch_semaphore_t queue_s = dispatch_semaphore_create(0);

  NStatManagerRef nm = NStatManagerCreate(
      kCFAllocatorDefault, queue, ^(NStatSourceRef Src, void *) {
        NStatSourceSetDescriptionBlock(Src, ^(CFDictionaryRef Desc) {
          CFNumberRef pPid = static_cast<CFNumberRef>(
              CFDictionaryGetValue(Desc, kNStatSrcKeyPID));
          pid_t pid;

          CFNumberGetValue(pPid, kCFNumberLongType, static_cast<void *>(&pid));

          CFNumberRef pRx = static_cast<CFNumberRef>(
              CFDictionaryGetValue(Desc, kNStatSrcKeyRxBytes));
          uint64_t rx;
          CFNumberGetValue(pRx, kCFNumberSInt64Type, &rx);
          CFNumberRef pTx = static_cast<CFNumberRef>(
              CFDictionaryGetValue(Desc, kNStatSrcKeyTxBytes));
          uint64_t tx;
          CFNumberGetValue(pTx, kCFNumberSInt64Type, &tx);
          auto &[_, proc] = *info.insert(
              info.find(pid), std::pair<pid_t, struct ninfo>(pid, {}));
          if (!proc.probed) {
            if (proc.t1 == 0) {
              proc.t1 =
                  clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW); // nanoseconds
              CFStringRef pName = static_cast<CFStringRef>(
                  CFDictionaryGetValue(Desc, kNStatSrcKeyProcessName));
              CFStringGetCString(pName, proc.prog, PROG_MAXSIZE,
                                 kCFStringEncodingUTF8);
            }
            proc.rx1 += rx;
            proc.tx1 += tx;
          } else {
            proc.t2 = clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW);
            proc.rx2 += rx;
            proc.tx2 += tx;
          }
        });
      });
  NStatManagerSetFlags(nm, 0);
  NStatManagerAddAllUDPWithFilter(nm, 0, 0);
  NStatManagerAddAllTCPWithFilter(nm, 0, 0);
  NStatManagerQueryAllSources(nm, ^() {
    NStatManagerQueryAllSourcesDescriptions(nm, ^() {
      for (auto &[_, proc] : info)
        proc.probed = true;
      sleep(2);
      NStatManagerQueryAllSources(nm, ^() {
        NStatManagerQueryAllSourcesDescriptions(nm, ^() {
          dispatch_suspend(queue);
          NStatManagerDestroy(nm);
          std::vector<struct sample> smpl;
          smpl.reserve(info.size());
          for (auto &[_, proc] : info) {
            if (proc.rx2 <= proc.rx1 && proc.tx2 <= proc.tx1)
              continue;
            float dt = static_cast<float>((proc.t2 - proc.t1)) /
                       1000000000.0f; // seconds
            struct sample s;
            s.rx = static_cast<float>((proc.rx2 - proc.rx1) / dt);
            s.tx = static_cast<float>((proc.tx2 - proc.tx1) / dt);
            s.prog = proc.prog;
            smpl.push_back(s);
          }
          std::sort(smpl.begin(), smpl.end(),
                    [](const struct sample &a, const struct sample &b) {
                      return ((b.rx + b.tx) < (a.rx + a.tx));
                    });

          int n = -1;
          if (argc > 1)
            n = static_cast<int>(strtol(argv[1], nullptr, 10));
          printf("%16s %14s %14s\n", "", "Rx", "Tx");
          char rxbuf[13], txbuf[13];
          for (const auto el : smpl) {
            printf("%16s %12s/s %12s/s\n", el.prog,
                   hr(rxbuf, sizeof(rxbuf), el.rx),
                   hr(txbuf, sizeof(txbuf), el.tx));
            if (--n == 0)
              break;
          }
          dispatch_semaphore_signal(queue_s);
        });
      });
    });
  });
  dispatch_semaphore_wait(queue_s, DISPATCH_TIME_FOREVER);
  dispatch_release(queue_s);
  return 0;
}
