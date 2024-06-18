#include <jni.h>
#include <game.h>

JNIEXPORT jstring JNICALL Java_com_halfpipe_edf_MainActivity_stringFromJNI(JNIEnv* env, jobject thiz) {
    const char *hello = hello_from_game();
    return (*env)->NewStringUTF(env, hello);
}