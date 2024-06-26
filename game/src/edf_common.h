//
//  edf_common.h
//  edf
//
//  Created by Manuel Cabrerizo on 19/06/2024.
//

#ifndef EDF_COMMON_H
#define EDF_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef float f32;
typedef double f64;
typedef u32 b32;

typedef size_t sz;

#define unused(v) ((void)(v))
#define offset_of(type, value) (&(((type *)0)->value))
#define array_len(v) (sizeof((v)) / sizeof((v)[0]))
#define is_power_of_two(expr) (((expr) & (expr - 1)) == 0)
#define next_power_of_two(value)                                                                   \
    do {                                                                                           \
        value--;                                                                                   \
        value |= value >> 1;                                                                       \
        value |= value >> 2;                                                                       \
        value |= value >> 4;                                                                       \
        value |= value >> 8;                                                                       \
        value |= value >> 16;                                                                      \
        value++;                                                                                   \
    } while(0)


#define kb(x) ((x) * 1024ll)
#define mb(x) (kb(x) * 1024ll)
#define gb(x) (mb(x) * 1024ll)

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define clamp(a, b, c) max(min(a, c), b)

#endif // EDF_COMMON_H
