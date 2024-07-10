//
// Created by tomas on 10/7/2024.
//

#include "android_jni.h"

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <edf_memory.h>
#include <edf.h>

// -----------------------------------
// Globals
// -----------------------------------

static jobject *asset_manager_ref   = 0;
AAssetManager *asset_manager = 0;
static Memory global_memory;

// -----------------------------------
// Internals
// -----------------------------------

static Input input_from_java(JNIEnv *env, jobjectArray touches) {
    Input input = { 0 };

    for(u32 i = 0; i < MAX_TOUCHES; ++i) {
        jobject jtouch = (*env)->GetObjectArrayElement(env, touches, (jsize)i);

        jclass touch_class = (*env)->GetObjectClass(env, jtouch);
        jfieldID uid_id = (*env)->GetFieldID(env, touch_class, "uid", "I");
        jfieldID x_id  = (*env)->GetFieldID(env, touch_class, "x", "F");
        jfieldID y_id  = (*env)->GetFieldID(env, touch_class, "y", "F");
        jfieldID down_id  = (*env)->GetFieldID(env, touch_class, "down", "Z");
        jfieldID up_id  = (*env)->GetFieldID(env, touch_class, "up", "Z");

        Touch *touch = input.touches + i;

        touch->uid = (u64)(*env)->GetIntField(env, jtouch, uid_id);
        touch->pos.x = (i32)(*env)->GetFloatField(env, jtouch, x_id);
        touch->pos.y = (i32)(*env)->GetFloatField(env, jtouch, y_id);
        touch->down = (b32)(*env)->GetBooleanField(env, jtouch, down_id);
        touch->up = (b32)(*env)->GetBooleanField(env, jtouch, up_id);
    }

    return input;
}

// -----------------------------------
// JNI implementations
// -----------------------------------

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameJNI_gameInit(JNIEnv *env, jobject thiz,  jobject manager) {
    (void)thiz;
    asset_manager_ref = (*env)->NewGlobalRef(env, manager);
    asset_manager     = AAssetManager_fromJava(env, asset_manager_ref);
    assert(asset_manager_ref);
    assert(asset_manager);

    global_memory.size = GAME_MEMORY_SIZE;
    global_memory.used = 0;
    global_memory.data = malloc(global_memory.size);
    game_init(&global_memory);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameJNI_gameUpdate(JNIEnv *env, jobject thiz, jobjectArray touches, jfloat dt) {
    (void)env;
    (void)thiz;
    Input input = input_from_java(env, touches);
    game_update(&global_memory, &input, dt);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameJNI_gameRender(JNIEnv *env, jobject thiz) {
    (void)env;
    (void)thiz;
    game_render(&global_memory);
}

JNIEXPORT void JNICALL Java_com_halfpipe_edf_GameJNI_gameResize(JNIEnv *env, jobject thiz, jint x, jint y, jint w, jint h) {
    game_resize(&global_memory, w, h);
}