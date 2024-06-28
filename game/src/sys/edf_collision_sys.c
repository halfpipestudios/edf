//
//  edf_collision_sys.c
//  edf
//
//  Created by Manuel Cabrerizo on 28/06/2024.
//

#include "edf_collision_sys.h"
#include "../edf_entity.h"
#include "../edf_debug.h"

SYSTEM_UPDATE(collision_system) {

    if(entity->collision.type == COLLISION_TYPE_CIRLCE) {
        entity->collision.circle.c = entity->pos.xy;
    }
    if(entity->collision.type == COLLISION_TYPE_AABB) {
        V2 min = entity->collision.aabb.min;
        V2 max = entity->collision.aabb.max;
        V2 mid = v2_add(min, v2_scale(max, 0.5f));
        min = v2_sub(min, mid);
        max = v2_sub(max, mid);
        min = v2_add(min, entity->pos.xy);
        max = v2_add(max, entity->pos.xy);
        entity->collision.aabb.min = min;
        entity->collision.aabb.max = max;
    }
    if(entity->collision.type == COLLISION_TYPE_OBB) {
        entity->collision.obb.c = entity->pos.xy;
        entity->collision.obb.r = entity->angle;
    }

    if(!entity->collides) {
        return;
    }

    i32 hit = false;
    for(i32 i = 0; i < others_count; i++) {
        Entity *other = others[i];
        if(entity != other) {
            if(entity->collision.type == COLLISION_TYPE_CIRLCE &&
               other->collision.type == COLLISION_TYPE_CIRLCE) {
                hit = test_cirlce_circle(entity->collision.circle, other->collision.circle);
            }
            else if(entity->collision.type == COLLISION_TYPE_AABB &&
               other->collision.type == COLLISION_TYPE_AABB) {
                hit = test_aabb_aabb(entity->collision.aabb, other->collision.aabb);
            }
            else if(entity->collision.type == COLLISION_TYPE_OBB &&
               other->collision.type == COLLISION_TYPE_OBB) {
                hit = test_obb_obb(entity->collision.obb, other->collision.obb);
            }
            else if(entity->collision.type == COLLISION_TYPE_CIRLCE &&
               other->collision.type == COLLISION_TYPE_AABB) {
                hit = test_circle_aabb(entity->collision.circle, other->collision.aabb);
            }
            else if(entity->collision.type == COLLISION_TYPE_AABB &&
               other->collision.type == COLLISION_TYPE_CIRLCE) {
                hit = test_circle_aabb(other->collision.circle, entity->collision.aabb);
            }
            else if(entity->collision.type == COLLISION_TYPE_CIRLCE &&
               other->collision.type == COLLISION_TYPE_OBB) {
                hit = test_circle_obb(entity->collision.circle, other->collision.obb);
            }
            else if(entity->collision.type == COLLISION_TYPE_OBB &&
               other->collision.type == COLLISION_TYPE_CIRLCE) {
                hit = test_circle_obb(other->collision.circle, entity->collision.obb);
            }
            else if(entity->collision.type == COLLISION_TYPE_AABB &&
               other->collision.type == COLLISION_TYPE_OBB) {
                hit = test_aabb_obb(entity->collision.aabb, other->collision.obb);
            }
            else if(entity->collision.type == COLLISION_TYPE_OBB &&
               other->collision.type == COLLISION_TYPE_AABB) {
                hit = test_aabb_obb(other->collision.aabb, entity->collision.obb);
            }
            else {
                assert(!"ERROR: collision pair not handle!!!");
            }
        }
    }

    if(hit) {
        cs_print(gcs, "Hit something\n");
    }
    else {
        cs_print(gcs, "No Hit\n");
    }

}

void collision_system_update(struct EntityManager *em, f32 dt) {
    u64 components = ENTITY_COLLISION_COMPONENT;
    entity_manager_forall(0, em, collision_system, components, dt);
}
