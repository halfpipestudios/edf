//
//  edf_render.c
//  edf
//
//  Created by Manuel Cabrerizo on 25/06/2024.
//

#include "edf_render_sys.h"
#include "../edf_entity.h"
#include "../edf_asset.h"
#include "../edf.h"

SYSTEM_UPDATE(render_system) {
    gpu_draw_quad_texture_tinted(gs->gpu, 
                                 entity->pos.x, entity->pos.y,
                                 entity->scale.x, entity->scale.y,
                                 entity->angle, entity->tex, entity->tint); 
    if(gs->debug_show && (entity->components & ENTITY_COLLISION_COMPONENT) != 0) {
        if(entity->collision.type == COLLISION_TYPE_CIRLCE) {
            gpu_draw_quad_texture_tinted(gs->gpu, 
                                 entity->collision.circle.c.x, entity->collision.circle.c.y,
                                 entity->collision.circle.r * 2.0f, entity->collision.circle.r * 2.0f,
                                 0, am_get_texture(gs->am, "move_outer.png"), v4(0, 1, 0, 0.4f)); 
        }
        else if(entity->collision.type == COLLISION_TYPE_AABB) {
            AABB aabb = entity->collision.aabb;
            V2 size = v2_sub(aabb.max, aabb.min);
            V2 pos = v2_add(aabb.min, v2_scale(size, 0.5f));
            gpu_draw_quad_color(gs->gpu, pos.x, pos.y, size.x, size.y, 0, v4(0, 1, 0, 0.4f));
        }
    }
}

void render_system_update(struct GameState *gs, struct EntityManager *em) {
    u64 components = ENTITY_RENDER_COMPONENT;
    entity_manager_forall(gs, em, render_system, components, 0);
}
