//
//  edf_input_sys.h
//  edf
//
//  Created by Manuel Cabrerizo on 25/06/2024.
//

#ifndef EDF_INPUT_SYS_H
#define EDF_INPUT_SYS_H

#include "../edf_common.h"

struct GameState;
struct EntityManager;
struct Input;

void input_system_update(struct GameState *gs, struct Input *input, struct EntityManager *em, f32 dt);

#endif /* EDF_INPUT_SYS_H */
