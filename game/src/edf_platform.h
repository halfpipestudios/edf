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
    TouchEvent event;
    V2i pos;
    u32 id;
} Touch;

typedef struct Input {
    Touch touches[MAX_TOUCHES];
    u32 touches_count;
} Input;

typedef struct Bitmap {
    u8 *data;
    u32 widht;
    u32 height;
} Bitmap;

typedef struct Wave {
    int unused;
} Wave;

typedef struct FileResult {

} FileResult;

typedef void * Texture;
typedef void * Sound;
typedef void * Gpu;
typedef void * Spu;

Bitmap bitmap_load(struct Arena *arena, char *path);
Wave   wave_load(struct Arena *Arena, char *path);

FileResult os_file_read(struct Arena *arena, char *path);
bool os_file_write(u8 *data, sz size);

Gpu gpu_load(struct Arena *arena);
void gpu_unload(Gpu gpu);
void gpu_frame_begin(Gpu gpu);
void gpu_frame_end(Gpu gpu);

Texture gpu_texture_load(Bitmap *bitmap);
void gpu_texture_unload(Texture texture);
void gpu_shader_set(void);

void gpu_draw_quad_texture(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, Texture texture);
void gpu_draw_quad_color(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, V3 color);
void gpu_camera_set(V3 pos, f32 angle);

Spu spu_load(struct Arena *arena);
void spu_unload(Spu spu);
void spu_clear(Spu spu);
Sound spu_add(Spu spu, Wave wave);
void spu_sound_play(Spu spu, Sound sound);
void spu_sound_pause(Spu spu, Sound sound);
void spu_sound_restart(Spu spu, Sound sound);

#endif // EDF_PLATFORM_H
