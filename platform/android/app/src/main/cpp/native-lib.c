#include <stdlib.h>
#include <assert.h>

#include <jni.h>
#include <GLES3/gl3.h>

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <edf.h>
#include <edf_platform.h>

#define logd(tag, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)

static jobject *asset_manager_ref   = 0;
static AAssetManager *asset_manager = 0;
static Memory global_memory;

File os_file_read(struct Arena *arena, char *path) {
    return (File){0};
}

typedef struct OpenglGPU {
    unsigned int program;
    unsigned int vao, vbo;
} OpenglGPU;

float vertices[] = { -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, 0.0f,
                     0.0f,  1.0f,  0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f,  1.0f };

unsigned int gpu_create_program(const char *vert_src, const char *frag_src) {

    unsigned int vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_src, 0);
    glCompileShader(vert_shader);
    int success;
    char info[512];
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vert_shader, 512, 0, info);
        logd("Game", "[Vertex Shader] %s\n", info);
    }

    unsigned int frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_src, 0);
    glCompileShader(frag_shader);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(frag_shader, 512, 0, info);
        logd("Game", "[Fragment Shader] %s\n", info);
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program, 512, 0, info);
        logd("Game", "[Program] %s\n", info);
    }
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return program;
}

Gpu gpu_load(struct Arena *arena) {

    const char *version = (const char *)glGetString(GL_VERSION);
    logd("Game", "OpenGL initialized: %s", version);

    OpenglGPU *renderer = arena_push(arena, sizeof(*renderer), 8);

    AAsset *vert_asset = AAssetManager_open(asset_manager, "shader.vert", AASSET_MODE_BUFFER);
    AAsset *frag_asset = AAssetManager_open(asset_manager, "shader.frag", AASSET_MODE_BUFFER);

    char *vert_src = (char *)AAsset_getBuffer(vert_asset);
    char *frag_src = (char *)AAsset_getBuffer(frag_asset);

    renderer->program = gpu_create_program(vert_src, frag_src);

    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);

    glBindVertexArray(renderer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return (Gpu *)renderer;
}

void gpu_unload(Gpu gpu) {
    OpenglGPU *renderer = (OpenglGPU *)gpu;
    glDeleteProgram(renderer->program);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteVertexArrays(1, &renderer->vao);
}

void gpu_frame_begin(Gpu gpu) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    OpenglGPU *renderer = (OpenglGPU *)gpu;
    glUseProgram(renderer->program);
    glBindVertexArray(renderer->vao);
}

void gpu_frame_end(Gpu gpu) {
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void gpu_draw_quad_color(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, V3 color) {

}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gameInit(JNIEnv *env, jobject thiz, jobject manager) {
    (void)thiz;
    asset_manager_ref = (*env)->NewLocalRef(env, manager);
    asset_manager     = AAssetManager_fromJava(env, asset_manager_ref);
    assert(asset_manager_ref);
    assert(asset_manager);

    global_memory.size = GAME_MEMORY_SIZE;
    global_memory.used = 0;
    global_memory.data = malloc(global_memory.size);
    game_init(&global_memory);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gameUpdate(JNIEnv *env, jobject thiz, jfloat dt) {
    (void)env;
    (void)thiz;
    game_render(&global_memory);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gameRender(JNIEnv *env, jobject thiz) {
    (void)env;
    (void)thiz;
    game_update(&global_memory, 0, 0);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameRenderer_gpuSetViewport(JNIEnv *env, jobject thiz, jint x, jint y, jint w, jint h) {
    (void)env;
    (void)thiz;
    glViewport(x, y, w, h);
}