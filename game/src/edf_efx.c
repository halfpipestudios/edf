//
// Created by tomas on 29/6/2024.
//

#include "edf_efx.h"

#include "edf.h"
#include "edf_level.h"
#include "edf_entity.h"
#include "edf_particles.h"
#include "edf_asset.h"

void stars_init(GameState *gs) {
    static u32 star_colors[4] = {
        0xFFFDF4F5,
        0xFFE8A0BF,
        0xFFBA90C6,
        0xFFC0DBEA
    };

    i32 hw = (r2f_width(gs->level->dim) * 0.5f) * 1.25f;
    i32 hh = (VIRTUAL_RES_Y * 0.5f) * 1.25f;
    
    Texture planet_textures[MAX_GALAXY] = {
        am_get_texture(gs->am, "galaxy.png"),
        am_get_texture(gs->am, "planet1.png"),
        am_get_texture(gs->am, "planet2.png"),
        am_get_texture(gs->am, "Satelite.png"),
        am_get_texture(gs->am, "Satelite.png"),
        am_get_texture(gs->am, "deathstar.png")
    };

    i32 color_index = 0;
    srand(123);
    for(i32 i = 0; i < MAX_GALAXY; i++) {
        Sprite *galaxy = gs->galaxy + i;
        galaxy->texture = planet_textures[color_index];
        galaxy->pos.x = rand_range(-hw, hw);
        galaxy->pos.y = rand_range(-hh, hh);
        galaxy->z = rand_range(2, 10);
        f32 ratio = ((f32)rand_range(50, 100) / 100.0f);
        galaxy->scale = v2(ratio, ratio);
        galaxy->tint = v4(0.4f, 0.4f, 0.4f, 1);
        galaxy->angle = 0;
        color_index++;
    }

    // Init star sprites
    color_index = 0;
    for(i32 i = 0; i < MAX_STARS; i++) {
        Sprite *star = gs->stars + i;
        star->texture = am_get_texture(gs->am, "star.png");
        star->pos.x = rand_range(-hw, hw);
        star->pos.y = rand_range(-hh, hh);
        star->z = (f32)rand_range(1, 10);
        star->scale.x = 1/star->z;
        star->scale.y = 1/star->z;
        star->tint = hex_to_v4(star_colors[color_index]);
        star->angle = 0;
        
        color_index = (color_index + 1) % array_len(star_colors);
    }
}

void stars_update(GameState *gs, f32 dt) {

    i32 hw = (r2f_width(gs->level->dim) * 0.5f);
    i32 hh = VIRTUAL_RES_Y * 0.5f;

    V2 pos = gs->level->camera_pos.xy;
    V2 vel = gs->level->camera_vel.xy;

    R2 bounds = {0};
    bounds.min.x = pos.x - (hw * 1.25f);
    bounds.min.y = pos.y - (hh * 1.25f);
    bounds.max.x = pos.x + (hw * 1.25f);
    bounds.max.y = pos.y + (hh * 1.25f);
    for(i32 i = 0; i < MAX_GALAXY; i++) {
        Sprite *galaxy = gs->galaxy + i;
        galaxy->pos.x -= (vel.x / galaxy->z) * dt;
        galaxy->pos.y -= (vel.y / galaxy->z) * dt;
        
        if(galaxy->pos.x > bounds.max.x) {
            galaxy->pos.x = bounds.min.x;
        }
        if(galaxy->pos.x < bounds.min.x) {
            galaxy->pos.x = bounds.max.x;
        }

        if(galaxy->pos.y > bounds.max.y) {
            galaxy->pos.y = bounds.min.y;
        }
        if(galaxy->pos.y < bounds.min.y) {
            galaxy->pos.y = bounds.max.y;
        }
    }

    for(i32 i = 0; i < MAX_STARS; i++) {
        Sprite *star = gs->stars + i;
        star->pos.x -= (vel.x / star->z) * dt;
        star->pos.y -= (vel.y / star->z) * dt;
        
#if 0
        if(star->pos.x > bounds.max.x) {
            star->pos.x = bounds.min.x;
        }
        if(star->pos.x < bounds.min.x) {
            star->pos.x = bounds.max.x;
        }

        if(star->pos.y > bounds.max.y) {
            star->pos.y = bounds.min.y;
        }
        if(star->pos.y < bounds.min.y) {
            star->pos.y = bounds.max.y;
        }
#endif
    }
}

void stars_render(GameState *gs) {
    // draw the planets
    for(i32 i = 0; i < MAX_GALAXY; i++) {
        Sprite *galaxy = gs->galaxy + i;
        sprite_draw(gs->gpu, galaxy);
    }

    // draw the stars
    for(i32 i = 0; i < MAX_STARS; i++) {
        Sprite *star = gs->stars + i;
        sprite_draw(gs->gpu, star);
    }
}

PARTICLE_SYSTEM_UPDATE(ship_ps_update) {

    static u32 fire_tint_index = 0;
    static u32 fire_tint[] = {
        0xffff4200,
        0xffffa700,
        0xfffff796
    };

    if(!particle->init) {
        particle->init = true;

        f32 rand = ((f32)rand_range(-20, 20)/ 180.0f) * PI;
        f32 offset = (PI*0.5f) + rand;
        V2 dir;
        dir.x = -cosf(gs->hero->angle + offset);
        dir.y = -sinf(gs->hero->angle + offset);
        particle->vel.x = dir.x * 1.0f + gs->hero->vel.x;
        particle->vel.y = dir.y * 1.0f + gs->hero->vel.y;
        particle->tint = hex_to_v4(fire_tint[fire_tint_index]);
        fire_tint_index = (fire_tint_index + 1) % array_len(fire_tint);
    }
    
    float x = particle->lifetime;
    particle->scale =  clamp((1.0f-(2*x-1)*(2*x-1)) * 0.4f, 0.1f, 0.4f);
    
    x = particle->lifetime/particle->save_lifetime;
    particle->tint.w = x*x;
    
    particle->pos.x += particle->vel.x * dt;
    particle->pos.y += particle->vel.y * dt;
}

PARTICLE_SYSTEM_UPDATE(neon_ps_update) {

    static u32 neon_tint_index = 0;
    static u32 neon_tint[] = {
        0xFF059212,
        0xFF06D001,
        0xFF9BEC00,
        0xFFF3FF90
    };

    if(!particle->init) {
        particle->init = true;

        V2 dir;
        dir.x = -cosf(gs->hero->angle + (PI*0.5f));
        dir.y = -sinf(gs->hero->angle + (PI*0.5f));

        V2 perp = v2(-dir.y, dir.x);
        f32 x_offet = rand_range(gs->hero->scale.x*-0.15f, gs->hero->scale.x*0.15f);
        particle->pos.x += perp.x * x_offet;
        particle->pos.y += perp.y * x_offet;

        V2 offset_point = v2_add(gs->hero->pos.xy, v2_scale(dir, 120));
        dir = v2_normalized(v2_sub(offset_point, particle->pos));

        particle->vel.x = dir.x * 1.0f + gs->hero->vel.x;
        particle->vel.y = dir.y * 1.0f + gs->hero->vel.y;
        particle->tint = hex_to_v4(neon_tint[neon_tint_index]);
        neon_tint_index = (neon_tint_index + 1) % array_len(neon_tint);
    }
    
    float x = particle->lifetime;
    particle->scale =  clamp((1.0f-(2*x-1)*(2*x-1)) * 0.4f, 0.01f, 0.4f);
    
    x = particle->lifetime/particle->save_lifetime;
    particle->tint.w = x*x;
    
    particle->pos.x += particle->vel.x * dt;
    particle->pos.y += particle->vel.y * dt;
}

PARTICLE_SYSTEM_UPDATE(pixel_ps_update) {

    static u32 tint_index = 0;
    static u32 tint[] = {
        0xffff0000,
        0xff00ff00,
        0xff0000ff
    };

    if(!particle->init) {
        particle->init = true;

        V2 dir;
        dir.x = -cosf(gs->hero->angle + (PI*0.5f));
        dir.y = -sinf(gs->hero->angle + (PI*0.5f));

        V2 perp = v2(-dir.y, dir.x);
        f32 offet = rand_range(gs->hero->scale.x*-0.5f, gs->hero->scale.x*0.5f);
        particle->pos.x += perp.x * offet;
        particle->pos.y += perp.y * offet;

        particle->vel.x = dir.x * 1.0f + gs->hero->vel.x;
        particle->vel.y = dir.y * 1.0f + gs->hero->vel.y;
        particle->tint = hex_to_v4(tint[tint_index]);
        tint_index = (tint_index + 1) % array_len(tint);

        particle->angle = gs->hero->angle;
    }
    
    float x = particle->lifetime;
    particle->scale =  clamp((1.0f-(2*x-1)*(2*x-1)) * 0.1f, 0.01f, 0.1f);
    
    x = particle->lifetime/particle->save_lifetime;
    particle->tint.w = x*x;
    
    particle->pos.x += particle->vel.x * dt;
    particle->pos.y += particle->vel.y * dt;
}

PARTICLE_SYSTEM_UPDATE(smoke_ps_update) {

    static u32 tint_index = 0;
    static u32 tint[] = {
        0x22d8d8d8,
        0x22b1b1b1,
        0x227e7e7e,
        0x22474747,
        0x221a1a1a
    };

    if(!particle->init) {
        particle->init = true;
    
        V2 dir;
        dir.x = -cosf(gs->hero->angle + (PI*0.5f));
        dir.y = -sinf(gs->hero->angle + (PI*0.5f));

        V2 perp = v2(-dir.y, dir.x);
        f32 x_offet = rand_range(gs->hero->scale.x*-0.15f, gs->hero->scale.x*0.15f);
        particle->pos.x += perp.x * x_offet;
        particle->pos.y += perp.y * x_offet + (rand_range(0, 20) / 200.0f);

        particle->tint = hex_to_v4(tint[tint_index]);
        tint_index = (tint_index + 1) % array_len(tint);
        
        particle->scale = rand_range(10, 40) / 200.0f;
        particle->vel = v2(0, 0);
        particle->lifetime = 10;

        particle->angle = (f32)rand_range(0, 360) / 180.0f * PI;
    }

    

    if((particle->lifetime / particle->save_lifetime) < 0.30) {
        particle->tint.w = particle->lifetime;
    }
}

PARTICLE_SYSTEM_UPDATE(confeti_ps_update) {
    static u32 confeti_tint_index = 0;
    static u32 confeti_tint[] = {
        0xffa864fd,
        0xff29cdff,
        0xff78ff44,
        0xffff718d,
        0xfffdff6a
    };

    if(!particle->init) {
        particle->init = true;
        
        f32 rand = ((f32)rand_range(-20, 20)/ 180.0f) * PI;
        f32 offset = (PI*0.5f) + rand;
        V2 dir;
        dir.x = -cosf(gs->hero->angle + offset);
        dir.y = -sinf(gs->hero->angle + offset);
        V2 perp = v2(-dir.y, dir.x);

        f32 x_offet = rand_range(gs->hero->scale.x*-0.15f, gs->hero->scale.x*0.15f);
        particle->pos.x += perp.x * x_offet;
        particle->pos.y += perp.y * x_offet;
        
        particle->vel.x = dir.x * (f32)rand_range(300, 600)/200.0f + gs->hero->vel.x;
        particle->vel.y = dir.y * (f32)rand_range(300, 600)/200.0f + gs->hero->vel.y;
        particle->scale = 0.14f;
        particle->save_lifetime = (f32)rand_range(25, 400) / 100.0f;
        particle->lifetime = (f32)rand_range(25, 400) / 100.0f;
        particle->tex = gs->confeti_texture[confeti_tint_index];
        particle->tint = hex_to_v4(confeti_tint[confeti_tint_index]);
        confeti_tint_index = (confeti_tint_index + 1) % array_len(confeti_tint);
        particle->angular_vel = (f32)rand_range(-20, 20);
    }
    if((particle->lifetime / particle->save_lifetime) < 0.15) {
        particle->tint.w = particle->lifetime;
    }

    particle->angle += particle->angular_vel * dt;
    particle->pos.x += particle->vel.x * dt;
    particle->pos.y += particle->vel.y * dt;
}
