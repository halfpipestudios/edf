//
//  ios_sound.h
//  edf
//
//  Created by Manuel Cabrerizo on 22/06/2024.
//

#ifndef IOS_SOUND_H
#define IOS_SOUND_H

#include "edf_common.h"
#include "edf_platform.h"

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

struct Arena;

typedef i32 IosSoundHandle;

typedef struct IosSoundChannel {
    Wave stream;
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
IosSoundHandle IosSoundSysAdd(IosSoundSystem *sound_sys, Wave stream, bool playing, bool looping);
void IosSoundSysRemove(IosSoundSystem *sound_sys, IosSoundHandle *out_handle);
void IosSoundSysPlay(IosSoundSystem *sound_sys, IosSoundHandle handle);
void IosSoundSysPause(IosSoundSystem *sound_sys, IosSoundHandle handle);
void IosSoundSysRestart(IosSoundSystem *sound_sys, IosSoundHandle handle);

OSStatus CoreAudioCallback(void *in_ref_con, AudioUnitRenderActionFlags *io_action_flags,
                           const AudioTimeStamp *in_time_stamp, UInt32 in_bus_number,
                           UInt32 in_number_frames, AudioBufferList *io_data);

#endif /* IOS_SOUND_H */
