//
//  edf_animation_sys.c
//  edf
//
//  Created by Manuel Cabrerizo on 29/06/2024.
//

#include "edf_animation_sys.h"
#include "../edf_entity.h"

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
                }
            }
            animation->current_time = 0.0f;
        }
        animation->current_time += dt;
        entity->tex = animation->frames[animation->current_frame];
    }  

}

void animation_system_update(struct EntityManager *em, f32 dt) {
    u64 components = ENTITY_ANIMATION_COMPONENT;
    entity_manager_forall(0, em, animation_system, components, dt);
}
