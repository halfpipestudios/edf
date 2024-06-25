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
}

void render_system_update(struct GameState *gs, struct EntityManager *em) {
    u64 components = ENTITY_RENDER_COMPONENT;
    entity_manager_forall(gs, 0, em, render_system, components, 0);
}
