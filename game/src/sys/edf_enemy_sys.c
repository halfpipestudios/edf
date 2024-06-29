//
//  edf_enemy_sys.c
//  edf
//
//  Created by Manuel Cabrerizo on 29/06/2024.
//

#include "edf_enemy_sys.h"
#include "../edf_entity.h"
#include "../edf.h"


//=================================================================================
// Trigger System
//=================================================================================
SYSTEM_UPDATE(trigger_system) {
    if(test_entity_entity(entity, gs->hero)) {
        Entity *to_trigger = entity->to_trigger;
        while(to_trigger) {
            to_trigger->active = true;
            to_trigger = to_trigger->to_trigger;
        }
    }  
}

void trigger_system_update(struct GameState *gs, struct EntityManager *em, f32 dt) {
    u64 components = ENTITY_TRIGGER_COMPONENT;
    entity_manager_forall(gs, em, trigger_system, components, dt);
}
//=================================================================================
//=================================================================================


//=================================================================================
// Asteroid System
//=================================================================================
SYSTEM_UPDATE(asteroid_system) {
    if(entity->active) {
        entity->pos.xy = v2_add(entity->pos.xy, v2_scale(entity->vel, dt));
        entity->angle += dt*2.0f;
    }
}

void asteroid_system_update(struct EntityManager *em, f32 dt) {
    u64 components = ENTITY_ASTEROID_COMPONENT;
    entity_manager_forall(0, em, asteroid_system, components, dt);
}

SYSTEM_UPDATE(asteroid_reset) {
    entity->active = false;
    entity->pos = entity->save_pos;
}

void asteroid_system_reset(struct EntityManager *em) {
    u64 components = ENTITY_ASTEROID_COMPONENT;
    entity_manager_forall(0, em, asteroid_reset, components, 0);
}

//=================================================================================
//=================================================================================
