//
//  edf_collision.c
//  edf
//
//  Created by Manuel Cabrerizo on 27/06/2024.
//

#include "edf_collision.h"

//==================================================
// Circle
//==================================================
i32 test_cirlce_circle(Circle a, Circle b) {
    // calculate the square distance between centers
    V2 d = v2_sub(a.c, b.c);
    f32 dist2 = v2_len_sq(d);
    // the sphere intersect if sq dist is less than the
    // square sum of radii
    f32 radius_sum = a.r + b.r;
    return dist2 <= radius_sum * radius_sum;
    
}
//==================================================
//==================================================

static Interval get_interval_aabb(AABB aabb, V2 axis) {
    // find the vertices of this none oriented rect
    V2 verts[4] = {
        aabb.min, aabb.max,
        v2(aabb.min.x, aabb.max.y),
        v2(aabb.max.x, aabb.min.y)
    };
    // store the minimun and maximun points of every
    // projected vertex as the interval of the rectangle
    Interval interval;
    interval.min = interval.max = v2_dot(axis, verts[0]);
    for(i32 i = 1; i < 4; i++) {
        f32 proj = v2_dot(axis, verts[i]);
        interval.min = min(interval.min, proj);
        interval.max = max(interval.max, proj);
    }
    return interval;
} 

// TODO(manu): this can by optimized by taking
// the vers calculation out of this function ...
static Interval get_interval_obb(OBB obb, V2 axis) {
    // contruct a non oriented version of the rectangle
    AABB r = {
        v2_sub(obb.c, obb.he),
        v2_add(obb.c, obb.he)
    };

    // find the vertices of this none oriented rect
    V2 verts[4] = {
        r.min, r.max,
        v2(r.min.x, r.max.y),
        v2(r.max.x, r.min.y)
    };

    // create the rotation matrix for the orientation
    // of the rectangle
    M2 rot = {
        cosf(obb.r), sinf(obb.r),
       -sinf(obb.r), cosf(obb.r)
    };

    // rotate every vertex by this rotation matrix
    for(i32 i = 0; i < 4; i++) {
        V2 v = v2_sub(verts[i], obb.c); 
        v = m2_mul_v2(rot, v);
        verts[i] = v2_add(v, obb.c);
    }

    // store the minimun and maximun points of every
    // projected vertex as the interval of the rectangle
    Interval interval;
    interval.min = interval.max = v2_dot(axis, verts[0]);
    for(i32 i = 1; i < 4; i++) {
        f32 proj = v2_dot(axis, verts[i]);
        interval.min = min(interval.min, proj);
        interval.max = max(interval.max, proj);
    }
    return interval;
} 

static bool overlap_on_axis(AABB aabb, OBB obb, V2 axis) {
    Interval a = get_interval_aabb(aabb, axis);
    Interval b = get_interval_obb(obb, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

//==================================================
// AABB
//==================================================
i32 test_aabb_aabb(AABB a, AABB b) {
    if(a.max.m[0] < b.min.m[0] || a.min.m[0] > b.max.m[0]) return 0;
    if(a.max.m[1] < b.min.m[1] || a.min.m[1] > b.max.m[1]) return 0;
    // overlapping on all axis means AABBs are intersecting
    return 1;
}

i32 test_aabb_obb(AABB a, OBB b) {
    V2 obb_right = v2(cosf(b.r), sinf(b.r));
    V2 axis_to_test [4] = {
        v2(1, 0),           // aabb right axis
        v2(0, 1),           // aabb up axis
        obb_right,          // obb  right axis
        v2_perp(obb_right)  // obb  up axis

    };
    // check every axis for overlap
    for(i32 i = 0; i < 4; i++) {
        if(!overlap_on_axis(a, b, axis_to_test[i])) {
            return false;
        }
    }
    return 1;
}
//==================================================
//==================================================


//==================================================
// OBB
//==================================================
i32 test_obb_obb(OBB a, OBB b) {
    // transform a into local space
    AABB local_a = { v2_scale(b.he, -1.0f), b.he };

    V2 ab = v2_sub(b.c, a.c);

    OBB local_b = b;
    local_b.r = b.r - a.r;

    f32 t = -a.r;
    M2 rot = {
        cosf(t), sinf(t),
       -sinf(t), cosf(t)
    };

    ab = m2_mul_v2(rot, ab);
    local_b.c = ab;

    return test_aabb_obb(local_a, local_b);    
}
//==================================================
//==================================================
