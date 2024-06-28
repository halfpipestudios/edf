//
//  edf_collision_sys.h
//  edf
//
//  Created by Manuel Cabrerizo on 28/06/2024.
//

#ifndef EDF_COLLISION_SYS_H
#define EDF_COLLISION_SYS_H

#include "../edf_common.h"

struct EntityManager;

void collision_system_update(struct EntityManager *em, f32 dt);


#endif /* EDF_COLLISION_SYS_H */
