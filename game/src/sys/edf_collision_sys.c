//
//  edf_collision_sys.c
//  edf
//
//  Created by Manuel Cabrerizo on 28/06/2024.
//

#include "edf_collision_sys.h"
#include "../edf_entity.h"
#include "../edf_debug.h"
#include "../edf.h"

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
            hit = test_entity_entity(entity, other);
            if(hit) { 
                break;
            }
        }
    }

    if(hit) {
        if(entity->animation) {
            entity->animation->playing = true;
        }
    }
}

void collision_system_update(struct GameState *gs, struct EntityManager *em, f32 dt) {
    u64 components = ENTITY_COLLISION_COMPONENT;
    entity_manager_forall(gs, em, collision_system, components, dt);
}
