//
//  edf_physics.c
//  edf
//
//  Created by Manuel Cabrerizo on 25/06/2024.
//

#include "edf_physics_sys.h"
#include "../edf_entity.h"
#include "../edf_math.h"

SYSTEM_UPDATE(physics_system) {
    entity->pos.x += entity->vel.x * dt;
    entity->pos.y += entity->vel.y * dt;

    entity->vel.x += entity->acc.x * dt;
    entity->vel.y += entity->acc.y * dt;

    entity->vel.x *= powf(entity->damping, dt);
    entity->vel.y *= powf(entity->damping, dt);

    entity->acc = v2(0, 0);
}




void physics_system_update(struct EntityManager *em, f32 dt) {
    u64 components = ENTITY_RENDER_COMPONENT|ENTITY_PHYSICS_COMPONENT;
    entity_manager_forall(0, em, physics_system, components, dt);
}
