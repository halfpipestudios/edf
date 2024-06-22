#ifndef EDF_PLATFORM_H
#define EDF_PLATFORM_H

#define MAX_TOUCHES 10

#include "edf_common.h"
#include "edf_math.h"

struct Arena;

typedef enum TouchEvent {
    TOUCH_EVENT_DOWN,
    TOUCH_EVENT_MOVE,
    TOUCH_EVENT_UP
} TouchEvent;

typedef struct Touch {
    V2i pos;
} Touch;

typedef struct Input {
    Touch touches[MAX_TOUCHES];
    u32 touches_count;
} Input;

typedef struct Bitmap {
    void *data;
    u32 width;
    u32 height;
} Bitmap;

typedef struct Wave {
    int unused;
} Wave;

typedef struct File {
    void *data;
    sz size;
} File;

typedef enum GpuBlendState {
    GPU_BLEND_STATE_ALPHA,
    GPU_BLEND_STATE_ADDITIVE
} GpuBlendState;

typedef void *Texture;
typedef void *Sound;
typedef void *Gpu;
typedef void *Spu;

Bitmap bitmap_load(struct Arena *arena, char *path);
Bitmap bitmap_empty(struct Arena *arena, i32 w, i32 h, sz pixel_size);
Bitmap bitmap_copy(struct Arena *arena, Bitmap *bitmap, sz pixel_size);
void bitmap_copy_u8_u32(struct Arena *arena, Bitmap *bitmap8, Bitmap *bitmap32);

Wave wave_load(struct Arena *Arena, char *path);

File os_file_read(struct Arena *arena, char *path);
bool os_file_write(u8 *data, sz size);
u32 os_display_width(void);
u32 os_display_height(void);
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
                           Texture texture, V3 color);
void gpu_draw_quad_color(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle,
                         V3 color);
void gpu_camera_set(Gpu gpu, V3 pos, f32 angle);
void gpu_resize(Gpu gpu, u32 w, u32 h);

Spu spu_load(struct Arena *arena);
void spu_unload(Spu spu);
void spu_clear(Spu spu);
Sound spu_add(Spu spu, Wave wave);
void spu_sound_play(Spu spu, Sound sound);
void spu_sound_pause(Spu spu, Sound sound);
void spu_sound_restart(Spu spu, Sound sound);

#endif // EDF_PLATFORM_H
