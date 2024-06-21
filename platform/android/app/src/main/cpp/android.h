//
// Created by tomas on 21/6/2024.
//

#ifndef EDF_ANDROID_H
#define EDF_ANDROID_H

#include <android/log.h>

#define logd(tag, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)

#endif //EDF_ANDROID_H
