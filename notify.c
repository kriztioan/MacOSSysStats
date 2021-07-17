/**
 *  @file   notify.c
 *  @brief  NSUserNotification Utility Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-17
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "notify.h"

static Class NSBundle = NULL;

static Class NSUserNotification = NULL;

static Class NSUserNotificationCenter = NULL;

static SEL mainBundle = NULL;

static SEL fakeBundleIdentifier = NULL;

id fakeBundleIdentifierIMP(id self, SEL _cmd) {
  if (self == objc_msgSend((id) NSBundle, mainBundle)) return (id) CFSTR("com.kriztioan.notify");
  else return objc_msgSend(self, fakeBundleIdentifier);
}

void init_notify() {

  NSBundle = objc_getClass("NSBundle");

  mainBundle = sel_registerName("mainBundle");

  fakeBundleIdentifier = sel_registerName("fakeBundleIdentifier");

  class_addMethod(NSBundle, fakeBundleIdentifier, (IMP) fakeBundleIdentifierIMP, "@@:");

  SEL bundleIdentifier = sel_registerName("bundleIdentifier");

  method_exchangeImplementations(class_getInstanceMethod(NSBundle, bundleIdentifier),
                                 class_getInstanceMethod(NSBundle, fakeBundleIdentifier));

  NSUserNotification = objc_getClass("NSUserNotification");

  NSUserNotificationCenter = objc_getClass("NSUserNotificationCenter");
}

void notify(const char *title, const char* subtitle, const char* message, int sec) {

  id notification = objc_msgSend(
      objc_msgSend((id)NSUserNotification, sel_registerName("alloc")),
      sel_registerName("init"));

  CFStringRef cfTitle = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

  objc_msgSend(notification, sel_registerName("setTitle:"), cfTitle);

  CFRelease(cfTitle);

  if(subtitle != NULL) {

    CFStringRef cfSubtitle = CFStringCreateWithCString(NULL, subtitle, kCFStringEncodingUTF8);

    objc_msgSend(notification, sel_registerName("setSubtitle:"), cfSubtitle);

    CFRelease(cfSubtitle);
  }

  CFStringRef cfMessage = CFStringCreateWithCString(NULL, message, kCFStringEncodingUTF8);

  objc_msgSend(notification, sel_registerName("setInformativeText:"), cfMessage);

  CFRelease(cfMessage);

  id userNotificationCenter = objc_msgSend((id) NSUserNotificationCenter,
					   sel_registerName("defaultUserNotificationCenter"));

  objc_msgSend(userNotificationCenter, sel_registerName("deliverNotification:"), notification);

  objc_msgSend(notification, sel_registerName("release"));

  if(sec > 0) {

    sleep(sec);
  }
}
