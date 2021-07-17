# MacOS System Stats

## Description

Here is a collection of small, highly efficient C/C++/Obj-C programs for gathering MacOS system stats. These programs are particularly useful when combined with software like [Übersicht](https://tracesof.net/uebersicht/), [GeekTool](https://www.tynsoe.org/geektool/), [NerdTool](http://mutablecode.com/apps/nerdtool.html), and others, that let their users run system commands and have the output displayed on the desktop. Notably, these small programs avoid the need to use cli-programs like `ps`, `top`, `netstat`, etc., who's output typically needs to be manipulated with tools like `awk`, `grep`, etc. Conveniently, most of the programs provided here have their output formatted as [JSON](https://www.json.org/json-en.html).

## Usage

Each sourcefile has as a preamble with a compile command. For example, `battery_mac.c` shows the following:

```C
/**
 *  @file   battery_mac.c
 *  @brief  Battery Status
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    clang -O3 -Weverything -Wno-poison-system-directories battery_mac.c
 *    -o battery -framework IOKit -framework Foundation
 *
 ***********************************************/
```

In the above case, an executable named `battery` is created.

## Notes

1. `freq_mac.c` requires [Intel Power Gadget](https://software.intel.com/content/www/us/en/develop/articles/intel-power-gadget.html) to be installed.
2. `netstat_mac.c` outputs simple text rather than in JSON.
3. `inet_mac.c` is a network monitor.
4. `NStatManager.h` contains Apple's private `NetworkStatistics` API.
5. `notify.c` is a `NSUserNotification` replacement for the [Growl](https://growl.github.io/growl/) framework. Be aware that MacOS versions Catalina and later now use `UNUserNotification` instead of `NSUserNotification` and incompatibilities might arise when on those MacOS versions.

## BSD-3 License

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
