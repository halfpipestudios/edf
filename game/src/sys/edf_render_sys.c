//
//  edf_render.c
//  edf
//
//  Created by Manuel Cabrerizo on 25/06/2024.
//

#include "edf_render_sys.h"
#include "../edf_entity.h"
#include "../edf.h"

SYSTEM_UPDATE(render_system) {
    gpu_draw_quad_texture_tinted(gs->gpu, 
                                 entity->pos.x, entity->pos.y,
                                 entity->scale.x, entity->scale.y,
                                 entity->angle, entity->tex, entity->tint); 
    if(gs->debug_show && (entity->components & ENTITY_COLLISION_COMPONENT) != 0) {
        gpu_draw_quad_texture_tinted(gs->gpu, 
                             entity->collision.circle.c.x, entity->collision.circle.c.y,
                             entity->collision.circle.r * 2.0f, entity->collision.circle.r * 2.0f,
                             0, gs->move_outer_texture, v4(0, 1, 0, 0.4f)); 
    }
}

void render_system_update(struct GameState *gs, struct EntityManager *em) {
    u64 components = ENTITY_RENDER_COMPONENT;
    entity_manager_forall(gs, em, render_system, components, 0);
}
