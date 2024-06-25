//
//  edf_input_sys.c
//  edf
//
//  Created by Manuel Cabrerizo on 25/06/2024.
//

#include "edf_input_sys.h"
#include "../edf_entity.h"
#include "../edf_input.h"
#include "../edf.h"

SYSTEM_UPDATE(input_system) {

    mt_begin(&gs->mt, input);
    
    ui_update(&gs->ui, &gs->mt, dt);


    if(ui_widget_is_active(&gs->mt, gs->joystick)) {
        V2 diff = v2_sub(gs->joystick->pos, gs->joystick->c_pos);
        f32 len = v2_len(diff);
        if(len > (gs->joystick->max_distance * 0.2f)) {
            V2 dir          = v2_normalized(diff);
            entity->angle = atan2f(dir.y, dir.x) + (PI / 2.0f);
        }
    }

    if(ui_widget_is_active(&gs->mt, gs->button)) {
        V2 dir = {0};
        dir.x = cosf(entity->angle + (PI / 2.0f));
        dir.y = sinf(entity->angle + (PI / 2.0f));
        if(v2_len(dir) > 0) {
            dir = v2_normalized(dir);
            entity->acc.x += dir.x * 800.0f;
            entity->acc.y += dir.y * 800.0f;
        }
    }


    mt_end(&gs->mt, input);
}

void input_system_update(struct GameState *gs, struct Input *input, struct EntityManager *em, f32 dt) {
    u64 components = ENTITY_INPUT_COMPONENT|ENTITY_RENDER_COMPONENT|ENTITY_PHYSICS_COMPONENT;
    entity_manager_forall(gs, input, em, input_system, components, dt);
}
