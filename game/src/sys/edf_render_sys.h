//
//  edf_render.h
//  edf
//
//  Created by Manuel Cabrerizo on 25/06/2024.
//

#ifndef EDF_RENDER_SYS_H
#define EDF_RENDER_SYS_H

#include "../edf_common.h"

struct GameState;
struct EntityManager;

void render_system_update(struct GameState *gs, struct EntityManager *em);

#endif /* EDF_RENDER_SYS_H */
