//
//  edf_math.h
//  edf
//
//  Created by Manuel Cabrerizo on 19/06/2024.
//

#ifndef EDF_MATH_H
#define EDF_MATH_H

#include <math.h>
#include "edf_common.h"

#define PI 3.14159265359f

static inline f32 lerp(f32 a, f32 b, f32 t) {
    return a * (1.0f - t) * b * t;
}

typedef union V2i {
    struct {
        i32 x, y;
    };
    i32 m[2];
} V2i;

static inline V2i v2i(i32 x, i32 y) {
    return (V2i){ x, y };
}

typedef union V2 {
    struct {
        f32 x, y;
    };
    f32 m[2];
} V2;

static inline V2 v2(f32 x, f32 y) {
    return (V2){ x, y };
}

static inline V2 v2_sub(V2 a, V2 b) {
    V2 result = { 0 };
    result.x  = a.x - b.x;
    result.y  = a.y - b.y;
    return result;
}

static inline V2 v2_scale(V2 v, f32 scale) {
    V2 result = { 0 };
    result.x  = v.x * scale;
    result.y  = v.y * scale;
    return result;
}

static inline f32 v2_dot(V2 a, V2 b) {
    return a.x * b.x + a.y * b.y;
}

static inline f32 v2_len(V2 v) {
    return sqrtf(v2_dot(v, v));
}

static inline V2 v2_normalized(V2 v) {
    f32 len = v2_len(v);
    if(len <= 0) {
        return v;
    }
    f32 inv_len = 1.0f / len;
    V2 result   = { v.x * inv_len, v.y * inv_len };
    return result;
}

typedef union V3 {
    struct {
        f32 x, y, z;
    };
    struct {
        f32 r, g, b;
    };
    f32 m[3];
} V3;

static inline V3 v3(f32 x, f32 y, f32 z) {
    return (V3){ x, y, z };
}

static inline V3 hex_to_v3(u32 hex) {
    f32 scale = 1.0f / 255.0f;
    f32 r = (f32)((hex >> 16) & 0xff) * scale;
    f32 g = (f32)((hex >> 8)  & 0xff) * scale;
    f32 b = (f32)((hex >> 0)  & 0xff) * scale;
    V3 result = v3(r, g, b);
    return result;
}

static inline V3 v3_lerp(V3 a, V3 b, f32 t) {
    V3 result;
    result.x = lerp(a.x, b.x, t);
    result.y = lerp(a.y, b.y, t);
    result.z = lerp(a.z, b.z, t);
    return result;
}

static inline V3 v3_add(V3 a, V3 b) {
    return (V3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

static inline V3 v3_sub(V3 a, V3 b) {
    return (V3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

static inline V3 v3_neg(V3 a) {
    return (V3){ -a.x, -a.y, -a.z };
}

static inline V3 v3_scale(V3 a, float s) {
    return (V3){ a.x * s, a.y * s, a.z * s };
}

static inline float v3_dot(V3 a, V3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline V3 v3_cross(V3 a, V3 b) {
    return (V3){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

static inline float v3_len_sqr(V3 a) {
    return v3_dot(a, a);
}

static inline float v3_len(V3 a) {
    return sqrtf(v3_len_sqr(a));
}

static inline V3 v3_normalized(V3 a) {
    float len = v3_len(a);
    if(len != 0) {
        float inv_len = 1.0f / len;
        a             = v3_scale(a, inv_len);
    }
    return a;
}

typedef union V4 {
    struct {
        f32 x, y, z, w;
    };
    struct {
        f32 r, g, b, a;
    };
    f32 m[4];
} V4;

inline V4 v4(f32 x, f32 y, f32 z, f32 w) {
    return (V4){ x, y, z, w };
}

typedef struct M4 {
    f32 m[16];
} M4;

static inline M4 m4_identity(void) {
    M4 m = (M4){
        {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}
    };
    return m;
}

static inline M4 m4_translate(V3 t) {
    M4 m = (M4){
        {1, 0, 0, t.x, 0, 1, 0, t.y, 0, 0, 1, t.z, 0, 0, 0, 1}
    };
    return m;
}

static inline M4 m4_scale(V3 s) {
    M4 m = (M4){
        {s.x, 0, 0, 0, 0, s.y, 0, 0, 0, 0, s.z, 0, 0, 0, 0, 1}
    };
    return m;
}

static inline M4 m4_rotate_z(float r) {
    float s = sinf(r);
    float c = cosf(r);
    M4 m    = {
        {c, -s, 0, 0, s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}
    };
    return m;
}

static inline M4 m4_ortho(float l, float r, float t, float b, float n, float f) {
    float rml = 1.0f / (r - l);
    float tmb = 1.0f / (t - b);
    float fmn = 1.0f / (f - n);
    M4 m      = (M4){
        {2 * rml, 0, 0, -((r + l) * rml), 0, 2 * tmb, 0, -((t + b) * tmb), 0, 0, fmn, -(n * fmn),
         0, 0, 0, 1}
    };
    return m;
}

static inline M4 m4_mul(M4 a, M4 b) {
#define RTC(r, c)                                                                                  \
    a.m[(r << 2)] * b.m[c] + a.m[(r << 2) + 1] * b.m[c + 4] + a.m[(r << 2) + 2] * b.m[c + 8] +     \
        a.m[(r << 2) + 3] * b.m[c + 12]
    M4 m = {
        {RTC(0, 0), RTC(0, 1), RTC(0, 2), RTC(0, 3), RTC(1, 0), RTC(1, 1), RTC(1, 2), RTC(1, 3),
         RTC(2, 0), RTC(2, 1), RTC(2, 2), RTC(2, 3), RTC(3, 0), RTC(3, 1), RTC(3, 2), RTC(3, 3)}
    };
#undef RTC
    return m;
}

static inline V3 m4_mul_v3(M4 m, V3 v) {
    V3 result = { 0 };
    for(u32 row = 0; row < 4; ++row) {
        u32 row_offset = row << 2;
        result.m[row]  = m.m[row_offset + 0] * v.m[0] + m.m[row_offset + 1] * v.m[1] +
                        m.m[row_offset + 2] * v.m[2] + m.m[row_offset + 3] * 1.0f;
    }
    return result;
}

typedef struct R2 {
    V2i min, max;
} R2;


static inline R2 r2_from_wh(i32 x, i32 y, i32 w, i32 h) {
    R2 result;
    result.min.x = x;
    result.max.x = x + w - 1;
    result.min.y = y;
    result.max.y = y + h - 1;
    return result;
}

static inline i32 r2_width(R2 a) {
    return (a.max.x - a.min.x) + 1;
}

static inline i32 r2_height(R2 a) {
    return (a.max.y - a.min.y) + 1;
}

static inline R2 r2_intersection(R2 a, R2 b) {
    R2 result;
    result.min.x = max(a.min.x, b.min.x);
    result.min.y = max(a.min.y, b.min.y);
    result.max.x = min(a.max.x, b.max.x);
    result.max.y = min(a.max.y, b.max.y);
    return result;
}

static inline R2 r2_union(R2 a, R2 b) {
    R2 result;
    result.min.x = min(a.min.x, b.min.x);
    result.min.y = min(a.min.y, b.min.y);
    result.max.x = max(a.max.x, b.max.x);
    result.max.y = max(a.max.y, b.max.y);
    return result;
}

static inline b32 r2_inside(R2 a, R2 b) {
    return ((a.min.x >= b.min.x) && (a.min.y >= b.min.y) && (a.max.x <= b.max.x) &&
            (a.max.y <= b.max.y));
}

static inline b32 r2_equals(R2 a, R2 b) {
    return ((a.min.x == b.min.x) && (a.max.x == b.max.x) && (a.min.y == b.min.y) &&
            (a.max.y == b.max.y));
}

static inline b32 r2_invalid(R2 a) {
    return ((a.max.x < a.min.x) || (a.max.y < a.min.y));
}

static inline b32 r2_point_overlaps(R2 a, i32 x, i32 y) {
    return (a.min.x <= x && x <= a.max.x && a.min.y <= y && y <= a.max.y);
}

static inline R2 r2_set_invalid(void) {
    R2 result = (R2){ 1, 1, -1, -1 };
    assert(r2_invalid(result));
    return result;
}

static inline R2 r2_translate(R2 rect, i32 x, i32 y) {
    R2 result;
    result.min.x = x;
    result.min.y = y;
    result.max.x = result.min.x + r2_width(rect) - 1;
    result.max.y = result.min.y + r2_height(rect) - 1;
    return result;
}

#endif /* EDF_MATH_H */
