//
//  edf_particles.h
//  edf
//
//  Created by Manuel Cabrerizo on 26/06/2024.
//

#ifndef EDF_PARTICLES_H
#define EDF_PARTICLES_H

#include "edf_common.h"
#include "edf_math.h"
#include "edf_platform.h"

struct Arena;
struct GameState;

typedef struct Particle {
    V2 pos;
    V2 vel;
    f32 angular_vel;
    f32 scale;
    f32 angle;
    Texture tex;
    f32 lifetime;
    f32 save_lifetime;
    V4 tint;
    b32 init;
} Particle;

#define PARTICLE_SYSTEM_UPDATE(name) void name(struct GameState *gs, Particle *particle, f32 dt)
typedef PARTICLE_SYSTEM_UPDATE(ParticleSystemUpdateFunc);

typedef struct ParticleSystem {
    i32 particle_count;
    Particle *particles;
    ParticleSystemUpdateFunc *update;
    Texture tex;
    V2 pos;
    i32 emision_count;
    i32 current_particle;
    f32 spawn_time;
    f32 current_spawn_time;
    bool pause;
    GpuBlendState blend_state;
} ParticleSystem;

ParticleSystem *particle_system_create(struct Arena *arena, 
                                       i32 particle_count, i32 emision_count, 
                                       f32 spawn_time, V2 pos, Texture texture,
                                       ParticleSystemUpdateFunc *particle_update,
                                       GpuBlendState blend_state);
void particle_system_update(struct GameState *gs, ParticleSystem *ps, f32 dt);
void particle_system_render(Gpu gpu, ParticleSystem *ps);
void particle_system_start(ParticleSystem *ps);
void particle_system_stop(ParticleSystem *ps);
void particle_system_reset(ParticleSystem *ps);
void particle_system_set_position(ParticleSystem *ps, V2 pos);

#endif /* EDF_PARTICLES_H */
