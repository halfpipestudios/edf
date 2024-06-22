//
//  ios_sound.h
//  edf
//
//  Created by Manuel Cabrerizo on 22/06/2024.
//

#ifndef IOS_SOUND_H
#define IOS_SOUND_H

#include "edf_common.h"

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

struct Arena;

typedef i32 IosSoundHandle;

typedef struct IosSoundStream {
    void *data;
    sz size;
} IosSoundStream;

typedef struct IosSoundChannel {
    IosSoundStream stream;
    i32 sample_count;
    i32 current_sample;
    i32 next;
    i32 prev;
    bool loop;
    bool playing;
} IosSoundChannel;

typedef struct IosSoundSystem {
    IosSoundChannel *channels;
    sz channel_buffer_size;
    i32 first;
    i32 first_free;
    i32 channel_count;
    i32 channel_used;
} IosSoundSystem;

void IosSoundSysInit(struct Arena *arena, IosSoundSystem *sound_sys, i32 max_channels);
IosSoundHandle IosSoundSysAdd(IosSoundSystem *sound_sys, IosSoundStream stream, bool playing, bool looping);
void IosSoundSysRemove(IosSoundSystem *sound_sys, IosSoundHandle *out_handle);
void IosSoundSysPlay(IosSoundSystem *sound_sys, IosSoundHandle handle);
void IosSoundSysPause(IosSoundSystem *sound_sys, IosSoundHandle handle);
void IosSoundSysRestart(IosSoundSystem *sound_sys, IosSoundHandle handle);

#endif /* IOS_SOUND_H */
