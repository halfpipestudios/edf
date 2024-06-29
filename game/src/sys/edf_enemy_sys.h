//
//  edf_enemy_sys.h
//  edf
//
//  Created by Manuel Cabrerizo on 29/06/2024.
//

#ifndef EDF_ENEMY_SYS_H
#define EDF_ENEMY_SYS_H

#include "../edf_common.h"

struct EntityManager;
struct GameState;

void trigger_system_update(struct GameState *gs, struct EntityManager *em, f32 dt);

void asteroid_system_update(struct EntityManager *em, f32 dt);

#endif /* EDF_ENEMY_SYS_H */
