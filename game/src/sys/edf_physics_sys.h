//
//  edf_physics.h
//  edf
//
//  Created by Manuel Cabrerizo on 25/06/2024.
//

#ifndef EDF_PHYSICS_SYS_H
#define EDF_PHYSICS_SYS_H

#include "../edf_common.h"

struct GameState;
struct EntityManager;

void physics_system_update(struct EntityManager *em, f32 dt);

#endif /* EDF_PHYSICS_H */
