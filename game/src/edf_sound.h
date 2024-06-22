#ifndef EDF_SOUND_H
#define EDF_SOUND_H

#include "edf_common.h"

struct Arena;

typedef struct WaveFileHeader {
	//the main chunk
	u8 szChunkID[4];
	u32 nChunkSize;
	u8 szFormat[4];

	//sub chunk 1 "fmt "
	u8 szSubChunk1ID[4];
	u32 nSubChunk1Size;
	u16 nAudioFormat;
	u16 nNumChannels;
	u32 nSampleRate;
	u32 nByteRate;
	u16 nBlockAlign;
	u16 nBitsPerSample;

	//sub chunk 2 "data"
	u8 szSubChunk2ID[4];
	u32 nSubChunk2Size;

	//then comes the data!
} WaveFileHeader;

typedef struct Wave {
    void *data;
    sz size;
} Wave;

Wave wave_load(struct Arena *arena, char *path);

#endif /* EDF_SOUND_H */
