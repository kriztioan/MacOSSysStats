/**
 *  @file   freq_mac.c
 *  @brief  CPU Frequency
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -march=native -Weverything -Wno-poison-system-directories
 *    freq_mac.c -o freq -F /Library/Frameworks -framework IntelPowerGadget
 *
 ***********************************************/

#include <IntelPowerGadget/PowerGadgetLib.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

__attribute__((noreturn)) static void die(int code) {
  printf("{error:%d}", code);
  exit(code);
}

int main(const int argc, const char *argv[]) {

  if (!PG_Initialize()) {
    die(1);
  }

  // first CPU package
  int iPackage = 0;

  if (!PG_UsePMU(iPackage, false)) {
    die(2);
  }

  PGSampleID sampleID1 = 0, sampleID2 = 0;

  if (!PG_ReadSample(iPackage, &sampleID1)) {
    die(3);
  }

  unsigned int t = 1;
  if (argc > 1) {
    t = (unsigned int)atoi(argv[1]);
  }

  sleep(t);

  if (!PG_ReadSample(iPackage, &sampleID2)) {
    die(4);
  }

  double freqMinMHz, freqMeanMHz, freqMaxMHz;

  if (!PGSample_GetIAFrequency(sampleID1, sampleID2, &freqMeanMHz, &freqMinMHz,
                               &freqMaxMHz)) {
    die(5);
  }

  double powerW, energyJ;

  if (!PGSample_GetPackagePower(sampleID1, sampleID2, &powerW, &energyJ)) {
    die(6);
  }

  if (!PGSample_Release(sampleID1)) {
    die(7);
  }

  double tempC;

  if (!PGSample_GetPackageTemperature(sampleID2, &tempC)) {
    die(8);
  }

  if (!PGSample_Release(sampleID2)) {
    die(9);
  }

  if (!PG_Shutdown()) {
    die(10);
  }

  printf("{\"frequency\":%4.0f, \"temperature\":%3.0f, \"power\":%4.1f, "
         "\"error\":0}",
         freqMeanMHz, tempC, powerW);

  return 0;
}
