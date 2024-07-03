//
//  edf_animation_sys.c
//  edf
//
//  Created by Manuel Cabrerizo on 29/06/2024.
//

#include "edf_animation_sys.h"
#include "../edf_entity.h"
#include "../edf.h"
#include "../edf_level.h"
#include "../edf_particles.h"
#include "edf_enemy_sys.h"

SYSTEM_UPDATE(animation_system) {
    Animation *animation = entity->animation;
    if(animation->playing) {
        if(animation->current_time >= animation->speed) {
            if(animation->looping) {
                animation->current_frame = (animation->current_frame + 1) % animation->frame_count;
            }
            else {
                animation->current_frame++;
                if(animation->current_frame == animation->frame_count) {
                    animation->current_frame = 0;
                    animation->playing = false;
                    entity->tex = entity->save_tex;
                    animation->current_time = 0.0f;

                    gs->hero->pos.x = gs->level->dim.min.x + MAP_COORDS_X*0.5f;
                    gs->hero->pos.y = 0.0f;
                    gs->level->camera_pos.x = gs->level->dim.min.x + MAP_COORDS_X*0.5f;
                    asteroid_system_reset(gs->em);
                    particle_system_reset(gs->ps);

                    return;
                }
            }
            animation->current_time = 0.0f;
        }
        animation->current_time += dt;
        entity->tex = animation->frames[animation->current_frame];
    }  

}

void animation_system_update(struct GameState *gs, struct EntityManager *em, f32 dt) {
    u64 components = ENTITY_ANIMATION_COMPONENT;
    entity_manager_forall(gs, em, animation_system, components, dt);
}
