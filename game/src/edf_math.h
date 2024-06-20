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

typedef union V3 {
    struct {
        f32 x, y, z;
    };
    struct {
        f32 r, g, b;
    };
    f32 m[3];
} V3;

V3 v3(f32 x, f32 y, f32 z);

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
    V3 result;
    for(u32 row = 0; row < 4; ++row) {
        u32 row_offset = row<<2;
        result.m[row] = m.m[row_offset+0] * v.m[0] +
                        m.m[row_offset+1] * v.m[1] +
                        m.m[row_offset+2] * v.m[2] +
                        m.m[row_offset+3] * 1.0f;
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
    return ((a.min.x >= b.min.x) && (a.min.y >= b.min.y) && (a.max.x <= b.max.x) && (a.max.y <= b.max.y));
}

static inline b32 r2_equals(R2 a, R2 b) {
    return ((a.min.x == b.min.x) &&  (a.max.x == b.max.x) && (a.min.y == b.min.y) && (a.max.y == b.max.y));
}

static inline b32 r2_invalid(R2 a) {
    return ((a.max.x < a.min.x) || (a.max.y < a.min.y));
}

static inline b32 r2_point_overlaps(R2 a, i32 x, i32 y) {
    return (a.min.x <= x && x <= a.max.x && a.min.y <= y && y <= a.max.y);
}

static inline R2 r2_set_invalid(void) {
    R2 result = (R2){1, 1, -1, -1};
    assert(r2_invalid(result));
    return result;
}

static inline R2 r2_translate(R2 rect, i32 x, i32 y) {
    R2 result;
    result.min.x = x;
    result.min.y = y;
    result.max.x = result.min.x + r2_width(rect)-1;
    result.max.y = result.min.y + r2_height(rect)-1;
    return result;
}

#endif /* EDF_MATH_H */
