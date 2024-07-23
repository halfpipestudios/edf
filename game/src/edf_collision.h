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

struct Entity;

typedef struct Interval {
    f32 min;
    f32 max;
} Interval;

typedef struct AABB {
    V2 min;
    V2 max;
} AABB;

typedef struct Circle {
    // TODO: remove the center
    V2 c;
    f32 r;
} Circle;

typedef struct OBB {
    // TODO: remove the center
    V2 c;
    V2 he;
    f32 r;
} OBB;

typedef struct Polygon {
    V2 *vertices;
    u32 vertices_count;
} Polygon;

V2 closest_point_point_circle(V2 a, Circle b);
V2 closest_point_point_aabb(V2 a, AABB b);
V2 closest_point_point_obb(V2 a, OBB b);

i32 test_cirlce_circle(Circle a, Circle b);
i32 test_circle_aabb(Circle a, AABB b);
i32 test_circle_obb(Circle a, OBB b);
i32 test_aabb_aabb(AABB a, AABB b);
i32 test_aabb_obb(AABB a, OBB b);
i32 test_obb_obb(OBB a, OBB b);
i32 test_entity_entity(struct Entity *entity, struct Entity *other);


#endif /* EDF_COLLISION_H */
