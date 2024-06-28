//
//  edf_collision.h
//  edf
//
//  Created by Manuel Cabrerizo on 27/06/2024.
//

#ifndef EDF_COLLISION_H
#define EDF_COLLISION_H

#include "edf_common.h"
#include "edf_math.h"

typedef struct Interval {
    f32 min;
    f32 max;
} Interval;

typedef struct AABB {
    V2 min;
    V2 max;
} AABB;

i32 test_aabb_aabb(AABB a, AABB b);

typedef struct Circle {
    V2 c;
    f32 r;
} Circle;

i32 test_cirlce_circle(Circle a, Circle b);

typedef struct OBB {
    V2 c;
    V2 he;
    f32 r;
} OBB;

i32 test_aabb_obb(AABB a, OBB b);
i32 test_obb_obb(OBB a, OBB b);

typedef struct Polygon {
    V2 *vertices;
    u32 vertices_count;
} Polygon;

#endif /* EDF_COLLISION_H */
