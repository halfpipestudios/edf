//
//  edf_animation_sys.h
//  edf
//
//  Created by Manuel Cabrerizo on 29/06/2024.
//

#ifndef EDF_ANIMATION_SYS_H
#define EDF_ANIMATION_SYS_H

#include "../edf_common.h"

struct EntityManager;

void animation_system_update(struct EntityManager *em, f32 dt);

#endif /* EDF_ANIMATION_SYS_H */
