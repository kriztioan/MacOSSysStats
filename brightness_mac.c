/**
 *  @file   brightness_mac.c
 *  @brief  LCD Brightness
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories brightness_mac.c
 *    -o brightness -framework CoreDisplay -framework CoreGraphics -framework
 *    IOKit -framework Foundation
 *
 ***********************************************/

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <math.h>

extern double CoreDisplay_Display_GetUserBrightness(CGDirectDisplayID id);

int main() {

  CGDirectDisplayID display = CGMainDisplayID();

  double brighness = CoreDisplay_Display_GetUserBrightness(display);

  printf("%.0f\n", round(100 * brighness));

  return 0;
}
