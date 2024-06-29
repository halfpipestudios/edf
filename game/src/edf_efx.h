//
// Created by tomas on 29/6/2024.
//

#ifndef EDF_EFX_H

#include "edf_particles.h"

void stars_init(struct GameState *gs);
void stars_update(struct GameState *gs, f32 dt);
void stars_render(struct GameState *gs);

PARTICLE_SYSTEM_UPDATE(ship_ps_update);
PARTICLE_SYSTEM_UPDATE(neon_ps_update);
PARTICLE_SYSTEM_UPDATE(pixel_ps_update);
PARTICLE_SYSTEM_UPDATE(smoke_ps_update);
PARTICLE_SYSTEM_UPDATE(confeti_ps_update);

#define EDF_EFX_H

#endif // EDF_EFX_H
