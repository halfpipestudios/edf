//
//  edf_collision.c
//  edf
//
//  Created by Manuel Cabrerizo on 27/06/2024.
//

#include "edf_collision.h"
#include "edf_entity.h"

//==================================================
// Separated Axis Theorem (SAT)
//==================================================
static Interval get_interval(V2 *verts, i32 count, V2 axis) {
    // store the minimun and maximun points of every
    // projected vertex as the interval of the rectangle
    Interval interval;
    interval.min = interval.max = v2_dot(axis, verts[0]);
    for(i32 i = 1; i < count; i++) {
        f32 proj = v2_dot(axis, verts[i]);
        interval.min = min(interval.min, proj);
        interval.max = max(interval.max, proj);
    }
    return interval;
}

static bool overlap_on_axis(V2 *verts0, i32 count0,
                            V2 *verts1, i32 count1,
                            V2 axis) {
    Interval a = get_interval(verts0, count0, axis);
    Interval b = get_interval(verts1, count1, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}
//==================================================
//==================================================

//==================================================
// Closest Point
//==================================================
V2 closest_point_point_circle(V2 a, Circle b) {
    V2 d = v2_sub(a, b.c);
    f32 dist2 = v2_len_sq(d);
    if(dist2 <= b.r*b.r) {
        return a;
    }
    else {
        return v2_add(b.c, v2_scale(v2_normalized(d), b.r));
    }
}

V2 closest_point_point_aabb(V2 a, AABB b) {
    V2 closest = a;
    closest.x = clamp(closest.x, b.min.x, b.max.x);
    closest.y = clamp(closest.y, b.min.y, b.max.y);
    return closest;
}

V2 closest_point_point_obb(V2 a, OBB b) {
    V2 local_a = v2_sub(a, b.c);
    V2 r = v2(cosf(b.r), sinf(b.r));
    V2 u    = v2_perp(r);
    f32 proj_r = clamp(v2_dot(local_a, r), -b.he.x, b.he.x);
    f32 proj_u = clamp(v2_dot(local_a, u), -b.he.y, b.he.y);
    local_a = v2_add(v2_scale(r, proj_r), v2_scale(u, proj_u));
    local_a = v2_add(local_a, b.c);
    return local_a;
}
//==================================================
//==================================================

//==================================================
// Intersection Tests
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

i32 test_circle_aabb(Circle a, AABB b) {
    V2 closest = closest_point_point_aabb(a.c, b);
    f32 dist2 = v2_len_sq(v2_sub(a.c, closest));
    f32 radius2 = a.r * a.r;
    return dist2 <= radius2;
}

i32 test_circle_obb(Circle a, OBB b) {
    V2 closest = closest_point_point_obb(a.c, b);
    f32 dist2 = v2_len_sq(v2_sub(a.c, closest));
    f32 radius2 = a.r * a.r;
    return dist2 <= radius2;
}

i32 test_aabb_aabb(AABB a, AABB b) {
    if(a.max.m[0] < b.min.m[0] || a.min.m[0] > b.max.m[0]) return 0;
    if(a.max.m[1] < b.min.m[1] || a.min.m[1] > b.max.m[1]) return 0;
    // overlapping on all axis means AABBs are intersecting
    return 1;
}

i32 test_aabb_obb(AABB aabb, OBB obb) {
    // get axis to test
    V2 obb_right = v2(cosf(obb.r), sinf(obb.r));
    V2 axis_to_test [4] = {
        v2(1, 0),           // aabb right axis
        v2(0, 1),           // aabb up axis
        obb_right,          // obb  right axis
        v2_perp(obb_right)  // obb  up axis

    };

    // get aabb vertices
    V2 verts_aabb[4] = {
        aabb.min, aabb.max,
        v2(aabb.min.x, aabb.max.y),
        v2(aabb.max.x, aabb.min.y)
    };
    
    // get obb vertices
    AABB r = {
        v2_sub(obb.c, obb.he),
        v2_add(obb.c, obb.he)
    };
    // find the vertices of this none oriented rect
    V2 verts_obb[4] = {
        r.min, r.max,
        v2(r.min.x, r.max.y),
        v2(r.max.x, r.min.y)
    };
    // create the rotation matrix for the orientation
    // of the rectangle
    M2 rot = {
        cosf(obb.r), -sinf(obb.r),
        sinf(obb.r),  cosf(obb.r)
    };
    // rotate every vertex by this rotation matrix
    for(i32 i = 0; i < 4; i++) {
        V2 v = v2_sub(verts_obb[i], obb.c); 
        v = m2_mul_v2(rot, v);
        verts_obb[i] = v2_add(v, obb.c);
    }

    // Do SAT intersection test
    for(i32 i = 0; i < 4; i++) {
        if(!overlap_on_axis(verts_aabb, 4, verts_obb, 4, axis_to_test[i])) {
            return false;
        }
    }
    return 1;
}

i32 test_obb_obb(OBB a, OBB b) {
    // transform a into local space
    AABB local_a = { v2_scale(b.he, -1.0f), b.he };

    V2 ab = v2_sub(b.c, a.c);

    OBB local_b = b;
    local_b.r = b.r - a.r;

    f32 t = -a.r;
    M2 rot = {
        cosf(t), -sinf(t),
        sinf(t),  cosf(t)
    };

    ab = m2_mul_v2(rot, ab);
    local_b.c = ab;

    return test_aabb_obb(local_a, local_b);    
}

Collision entity_get_world_collision_circle(Entity *entity) {
    Collision collision;
    collision.type = COLLISION_TYPE_CIRLCE;
    V2 offset = v2_rotate(entity->collision.offset, entity->angle);
    collision.circle.c = v2_add(entity->pos.xy, offset);
    collision.circle.r = entity->collision.circle.r;
    return collision;
}

Collision entity_get_world_collision_aabb(Entity *entity) {
    Collision collision;
    collision.type = COLLISION_TYPE_AABB;
    V2 offset = v2_rotate(entity->collision.offset, entity->angle);
    V2 pos = v2_add(entity->pos.xy, offset);
    collision.aabb.min = v2_add(pos, entity->collision.aabb.min);
    collision.aabb.max = v2_add(pos, entity->collision.aabb.max);
    return collision;
}

Collision entity_get_world_collision_obb(Entity *entity) {
    Collision collision;
    collision.type = COLLISION_TYPE_OBB;
    V2 offset = v2_rotate(entity->collision.offset, entity->angle);
    collision.obb.c = v2_add(entity->pos.xy, offset);
    collision.obb.r = entity->angle;
    collision.obb.he = entity->collision.obb.he;
    return collision;
}

Collision entity_get_world_collision(Entity *entity) {
    assert(entity->components & ENTITY_COLLISION_COMPONENT);
    switch(entity->collision.type) {
        case COLLISION_TYPE_CIRLCE: {
            return entity_get_world_collision_circle(entity);
        } break;
        case COLLISION_TYPE_AABB: {
            return entity_get_world_collision_aabb(entity);
        } break;
        case COLLISION_TYPE_OBB: {
            return entity_get_world_collision_obb(entity);
        } break;
    }
}

i32 test_entity_entity(struct Entity *entity, struct Entity *other) {

    Collision e = entity_get_world_collision(entity);
    Collision o = entity_get_world_collision(other);

    if(e.type == COLLISION_TYPE_CIRLCE &&
       o.type == COLLISION_TYPE_CIRLCE) {
        return test_cirlce_circle(e.circle, o.circle);
    }
    else if(e.type == COLLISION_TYPE_AABB &&
       o.type == COLLISION_TYPE_AABB) {
        return test_aabb_aabb(e.aabb, o.aabb);
    }
    else if(e.type == COLLISION_TYPE_OBB &&
       o.type == COLLISION_TYPE_OBB) {
        return test_obb_obb(e.obb, o.obb);
    }
    else if(e.type == COLLISION_TYPE_CIRLCE &&
       o.type == COLLISION_TYPE_AABB) {
        return test_circle_aabb(e.circle, o.aabb);
    }
    else if(e.type == COLLISION_TYPE_AABB &&
       o.type == COLLISION_TYPE_CIRLCE) {
        return test_circle_aabb(o.circle, e.aabb);
    }
    else if(e.type == COLLISION_TYPE_CIRLCE &&
       o.type == COLLISION_TYPE_OBB) {
        return test_circle_obb(e.circle, o.obb);
    }
    else if(e.type == COLLISION_TYPE_OBB &&
       o.type == COLLISION_TYPE_CIRLCE) {
        return test_circle_obb(o.circle, e.obb);
    }
    else if(e.type == COLLISION_TYPE_AABB &&
       o.type == COLLISION_TYPE_OBB) {
        return test_aabb_obb(e.aabb, o.obb);
    }
    else if(e.type == COLLISION_TYPE_OBB &&
       o.type == COLLISION_TYPE_AABB) {
        return test_aabb_obb(o.aabb, e.obb);
    }
    else {
        assert(!"ERROR: collision pair not handle!!!");
    }
    return 0;
}
//==================================================
//==================================================
