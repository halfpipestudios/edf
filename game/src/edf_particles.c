//
//  edf_particles.c
//  edf
//
//  Created by Manuel Cabrerizo on 26/06/2024.
//

#include "edf_particles.h"
#include "edf_memory.h"

ParticleSystem *particle_system_create(struct Arena *arena, 
                                       i32 particle_count, i32 emision_count, 
                                       f32 spawn_time, V2 pos, Texture texture,
                                       ParticleSystemUpdateFunc *particle_update,
                                       GpuBlendState blend_state) {

    ParticleSystem *ps = (ParticleSystem *)arena_push(arena, sizeof(ParticleSystem), 8);
    ps->particle_count = particle_count;
    ps->particles = (Particle *)arena_push(arena, sizeof(Particle) * ps->particle_count, 8);
    memset(ps->particles, 0, sizeof(Particle) * ps->particle_count);
    ps->update = particle_update;
    ps->tex = texture;
    ps->pos = pos;
    ps->emision_count = emision_count;
    ps->current_particle = 0;
    ps->spawn_time = spawn_time;
    ps->current_spawn_time = 0;
    ps->pause = true;
    ps->blend_state = blend_state;
    return ps;

}

void particle_system_update(struct GameState* gs, ParticleSystem *ps, f32 dt) {
    if(ps->pause == false) {
    
        // spawn particles
        if(ps->current_spawn_time <= 0.0f) {
            i32 start = ps->current_particle;
            i32 end = min(ps->current_particle + ps->emision_count, ps->particle_count);
            for(i32 i = start; i < end; i++) {
                Particle *particle = ps->particles + i;
                particle->save_lifetime = 0.5f;//(f32)rand_range(25, 80) / 100.f;
                particle->lifetime = particle->save_lifetime;
                particle->pos = ps->pos;
                particle->scale = 0.1;
                particle->vel = v2(0, 0);
                particle->tex = ps->tex;
                particle->angle = 0;
                particle->tint = v4(1, 1, 1, 1);
                particle->init = false;
                ps->current_particle = (ps->current_particle + 1) % ps->particle_count;
            }
            ps->current_spawn_time = ps->spawn_time;
        }
        ps->current_spawn_time -= dt;

    } 

    // update alive particles
    for(i32 i = 0; i < ps->particle_count; i++) {
        Particle *particle = ps->particles + i;
        if(particle->lifetime > 0.0f) {
            ps->update(gs, particle, dt);
            particle->lifetime -= dt;
        }
    }

}

void particle_system_render(Gpu gpu, ParticleSystem *ps) {
    // render alive particles
    gpu_blend_state_set(gpu, ps->blend_state);
    for(i32 i = 0; i < ps->particle_count; i++) {
        Particle *particle = ps->particles + i;
        if(particle->lifetime > 0.0f) {
            gpu_draw_quad_texture_tinted(gpu, 
                             particle->pos.x, particle->pos.y,
                             particle->scale, particle->scale,
                             particle->angle, particle->tex,
                             particle->tint); 
        }
    }
    gpu_blend_state_set(gpu, GPU_BLEND_STATE_ALPHA);

}

void particle_system_start(ParticleSystem *ps) {
    ps->pause = false;
}

void particle_system_stop(ParticleSystem *ps) {
    ps->pause =  true;
}

void particle_system_reset(ParticleSystem *ps) {
    ps->current_particle = 0;
    ps->current_spawn_time = 0;
    ps->pause =  true;
    for(i32 i = 0; i < ps->particle_count; i++) {
        Particle *particle = ps->particles + i;
        particle->lifetime = 0.0f;
    }
}

void particle_system_set_position(ParticleSystem *ps, V2 pos) {
    ps->pos = pos;
}
