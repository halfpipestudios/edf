//
//  edf_collision_sys.c
//  edf
//
//  Created by Manuel Cabrerizo on 28/06/2024.
//

#include "edf_collision_sys.h"
#include "../edf_entity.h"
#include "../edf.h"
#include "../edf_debug.h"
#include "../edf_particles.h"
#include "../edf_level.h"


SYSTEM_UPDATE(collision_system) {

    // this is not necesary
    // becouse the colliders are define in local space not world
    /*
    if(entity->collision.type == COLLISION_TYPE_CIRLCE) {
        entity->collision.circle.c = v2_add(entity->pos.xy, entity->collision.offset);
    }
    if(entity->collision.type == COLLISION_TYPE_AABB) {
        // TODO: ...
    }
    if(entity->collision.type == COLLISION_TYPE_OBB) {
        entity->collision.obb.c = entity->pos.xy;
        entity->collision.obb.r = entity->angle;
    }
    */

    if(!entity->collides) {
        return;
    }

    i32 hit = false;
    for(i32 i = 0; i < others_count; i++) {
        Entity *other = others[i];
        if(entity != other && (other->components & ENTITY_TRIGGER_COMPONENT) == 0) {
            hit = test_entity_entity(entity, other);
            if(hit) { 
                break;
            }
        }
    }

    if(hit) {
        if(entity->animation) {
            if(entity->animation->playing == false) {
                entity->save_tex = entity->tex;
                entity->animation->playing = true;
            }
            entity->vel = v2(0, 0);
            entity->acc = v2(0, 0);
            particle_system_stop(gs->ps);
            gs->level->camera_vel = v3(0, 0, 0);

        }
    }
}

void collision_system_update(struct GameState *gs, struct EntityManager *em, f32 dt) {
    u64 components = ENTITY_COLLISION_COMPONENT;
    entity_manager_forall(gs, em, collision_system, components, dt);
}
