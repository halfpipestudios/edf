//
//  ios_sound.c
//  edf
//
//  Created by Manuel Cabrerizo on 22/06/2024.
//

#include "edf_memory.h"
#include "ios_sound.h"

void IosSoundSysInit(struct Arena *arena, IosSoundSystem *sound_sys, i32 max_channels) {
    sound_sys->channel_count = max_channels;
    sound_sys->channel_used = 0;
    sound_sys->channel_buffer_size = max_channels * sizeof(IosSoundChannel);

    sound_sys->channels = (IosSoundChannel *)arena_push(arena, sound_sys->channel_buffer_size, 8);
    memset(sound_sys->channels, 0, sound_sys->channel_buffer_size);

    sound_sys->first = -1;
    sound_sys->first_free = 0;

    for(i32 i = 0; i < max_channels; i++) {
        IosSoundChannel *channel = sound_sys->channels + i;
        channel->stream.data = 0;
        channel->stream.size = 0;
        channel->loop = false;
        channel->playing = false;
        if(i < (max_channels - 1)) {
            channel->next = i + 1;
        }
        else {
            channel->next = -1;
        }
        if(i == 0) {
            channel->prev = -1;
        }
        else {
            channel->prev = i - 1;
        }
    }
}

IosSoundHandle IosSoundSysAdd(IosSoundSystem *sound_sys, Wave *wave,
                              bool playing, bool looping) {

    if((sound_sys->channel_used + 1) > sound_sys->channel_count) {
        return -1;
    }

    // find a free channel
    IosSoundHandle handle = sound_sys->first_free;
    if(handle < 0 || handle >= sound_sys->channel_count) {
        return -1;
    }
    IosSoundChannel *channel = sound_sys->channels + handle;
    sound_sys->first_free = channel->next;
    // initialize the channel
    channel->stream = *wave;
    channel->loop = looping;
    channel->playing = playing;
    channel->sample_count = (i32)(wave->size / sizeof(i32));
    channel->current_sample = 0;
    channel->next = sound_sys->first;
    channel->prev = -1;

    sound_sys->first = handle;
    if(channel->next >= 0) {
        IosSoundChannel *next_channel = sound_sys->channels + channel->next;
        next_channel->prev = handle;
    }

    sound_sys->channel_used++;

    return handle;
}


void IosSoundSysClear(IosSoundSystem *sound_sys) {
    sound_sys->channel_used = 0;
    sound_sys->first = -1;
    sound_sys->first_free = 0;
    for(i32 i = 0; i < sound_sys->channel_count; i++) {
        IosSoundChannel *channel = sound_sys->channels + i;
        channel->stream.data = 0;
        channel->stream.size = 0;
        channel->loop = false;
        channel->playing = false;
        if(i < (sound_sys->channel_count - 1)) {
            channel->next = i + 1;
        }
        else {
            channel->next = -1;
        }
        if(i == 0) {
            channel->prev = -1;
        }
        else {
            channel->prev = i - 1;
        }
    }

}

void IosSoundSysRemove(IosSoundSystem *sound_sys, IosSoundHandle *out_handle) {
    IosSoundHandle handle = *out_handle;
    if(handle < 0 || handle >= sound_sys->channel_count) {
        return;
    }
    IosSoundChannel *channel = sound_sys->channels + handle;
    if(sound_sys->first == handle) {
        sound_sys->first = channel->next;
    }

    // remove this channel from the list
    IosSoundChannel *prev_channel = sound_sys->channels + channel->prev;
    IosSoundChannel *next_channel = sound_sys->channels + channel->next;
    prev_channel->next = channel->next;
    next_channel->prev = channel->prev;

    // add this channel to the free list
    channel->prev = -1;
    channel->next = sound_sys->first_free;
    sound_sys->first_free = handle;

    sound_sys->channel_used--;
    *out_handle = -1;
}

void IosSoundSysPlay(IosSoundSystem *sound_sys, IosSoundHandle handle) {
    if(handle < 0 || handle >= sound_sys->channel_count) {
        return;
    }
    IosSoundChannel *channel = sound_sys->channels + handle;
    channel->playing = true;
}

void IosSoundSysPause(IosSoundSystem *sound_sys, IosSoundHandle handle) {
    if(handle < 0 || handle >= sound_sys->channel_count) {
        return;
    }
    IosSoundChannel *channel = sound_sys->channels + handle;
    channel->playing = false;
}

void IosSoundSysRestart(IosSoundSystem *sound_sys, IosSoundHandle handle) {
    if(handle < 0 || handle >= sound_sys->channel_count) {
        return;
    }
    IosSoundChannel *channel = sound_sys->channels + handle;
    channel->playing = false;
    channel->current_sample = 0;
}

// CoreAudio Callback
OSStatus CoreAudioCallback(void *in_ref_con, AudioUnitRenderActionFlags *io_action_flags,
                           const AudioTimeStamp *in_time_stamp, UInt32 in_bus_number,
                           UInt32 in_number_frames, AudioBufferList *io_data) {
    IosSoundSystem *sound_sys = (IosSoundSystem *)in_ref_con;
    i16 *sound_buffer = (i16 *)io_data->mBuffers[0].mData;
    memset(sound_buffer, 0, sizeof(i32) * in_number_frames);

    if(sound_sys->channels == 0) {
        return noErr;
    }

     IosSoundHandle handle = sound_sys->first;
    while(handle != -1) {
        IosSoundChannel *channel = sound_sys->channels + handle;

        if(channel->playing) {
            i32 samples_left = channel->sample_count - channel->current_sample;
            i32 samples_to_stream = min(in_number_frames, samples_left);

            i16 *dst = sound_buffer;
            i16 *src = (i16 *)((i32 *)channel->stream.data + channel->current_sample);
            
            for(i32 i = 0; i < samples_to_stream; i++) {
                i32 old_value_0 = (i32)dst[0];
                i32 old_value_1 = (i32)dst[1];
                i32 new_value_0 = (i32)src[0];
                i32 new_value_1 = (i32)src[1];

                i32 sum0 = old_value_0 + new_value_0;
                i32 sum1 = old_value_1 + new_value_1;

                dst[0] = (i16)max(min(sum0, 32767), -32768);
                dst[1] = (i16)max(min(sum1, 32767), -32768);

                dst += 2;
                src += 2;
            }

            if(channel->loop) {
                channel->current_sample = (channel->current_sample + samples_to_stream) % channel->sample_count;
            }
            else {
                channel->current_sample = channel->current_sample + samples_to_stream;
                if(channel->current_sample >= channel->sample_count) {
                    channel->current_sample = 0;
                    channel->playing = false;
                }
            }
        }

        handle = channel->next;
    }
    return noErr;
}
