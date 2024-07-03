#ifndef EDF_PLATFORM_H
#define EDF_PLATFORM_H

#include "edf_common.h"
#include "edf_math.h"
#include "edf_graphics.h"
#include "edf_sound.h"
#include "edf_input.h"

#define VIRTUAL_RES_X 1920
#define VIRTUAL_RES_Y 1080

struct Arena;


typedef struct File {
    void *data;
    sz size;
} File;

typedef enum GpuBlendState {
    GPU_BLEND_STATE_ALPHA,
    GPU_BLEND_STATE_ADDITIVE
} GpuBlendState;

typedef void *RenderTarget;
typedef void *Texture;
typedef void *Sound;
typedef void *Gpu;
typedef void *Spu;

File os_file_read(struct Arena *arena, char *path);
bool os_file_write(u8 *data, sz size, char *path);
R2 os_display_rect(void);
R2 os_device_rect(void);
void os_print(char *message, ...);

Gpu gpu_load(struct Arena *arena);
void gpu_unload(Gpu gpu);
void gpu_frame_begin(Gpu gpu);
void gpu_frame_end(Gpu gpu);
Texture gpu_texture_load(Gpu gpu, Bitmap *bitmap);
void gpu_texture_unload(Gpu gpu, Texture texture);
void gpu_blend_state_set(Gpu gpu, GpuBlendState blend_state);
void gpu_draw_quad_texture(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle,
                           Texture texture);
void gpu_draw_quad_texture_tinted(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle,
                           Texture texture, V4 color);
void gpu_draw_quad_color(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, V4 color);
void gpu_camera_set(Gpu gpu, V3 pos, f32 angle);
void gpu_resize(Gpu gpu, u32 w, u32 h);
RenderTarget gpu_render_targte_load(Gpu gpu, i32 width, i32 height);
void gpu_render_target_begin(Gpu gpu, RenderTarget rt);
void gpu_render_target_end(Gpu gpu, RenderTarget rt);
void gpu_render_target_draw(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, RenderTarget rt);
void gpu_viewport_set(Gpu gpu, f32 x, f32 y, f32 w, f32 h);

Spu spu_load(struct Arena *arena);
void spu_unload(Spu spu);
void spu_clear(Spu spu);
Sound spu_sound_add(Spu spu, Wave *wave, bool playing, bool looping);
void spu_sound_remove(Spu spu, Sound sound);
void spu_sound_play(Spu spu, Sound sound);
void spu_sound_pause(Spu spu, Sound sound);
void spu_sound_restart(Spu spu, Sound sound);

#endif // EDF_PLATFORM_H
