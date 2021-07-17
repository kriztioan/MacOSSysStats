/**
 *  @file   notify.h
 *  @brief  NSUserNotification Utility API
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 * 
 ***********************************************/

#ifndef NOTIFY_H
#define NOTIFY_H

#define OBJC_OLD_DISPATCH_PROTOTYPES 1
#include <CoreFoundation/CoreFoundation.h>
#include <objc/message.h>
#include <objc/runtime.h>

void init_notify();
void notify(const char *title, const char *subtitle, const char *message,
            int sec);

#endif // End of notify.h