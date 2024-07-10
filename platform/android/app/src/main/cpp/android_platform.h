//
// Created by tomas on 21/6/2024.
//

#ifndef EDF_ANDROID_PLATFORM_H
#define EDF_ANDROID_PLATFORM_H

#include <android/log.h>

#define logd(tag, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)

static inline void logv(const char *tag,  const char *format, va_list args) {
    __android_log_vprint(ANDROID_LOG_DEBUG, tag, format, args);
}

extern struct  AAssetManager *asset_manager;

#endif //EDF_ANDROID_PLATFORM_H
