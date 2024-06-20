//
//  edf_math.h
//  edf
//
//  Created by Manuel Cabrerizo on 19/06/2024.
//

#ifndef EDF_MATH_H
#define EDF_MATH_H

#include "edf_common.h"

typedef union V2i {
    struct {
        i32 x, y;
    };
    i32 m[2];
} V2i;

inline V2i v2i(i32 x, i32 y) {
    return (V2i){ x, y };
}

typedef union V2 {
    struct {
        f32 x, y;
    };
    f32 m[2];
} V2;

inline V2 v2(f32 x, f32 y) {
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

#endif /* EDF_MATH_H */
