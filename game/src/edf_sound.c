#include "edf_memory.h"
#include "edf_platform.h"

Wave wave_load(Arena *arena, char *path) {
    File file = os_file_read(arena, path);
    Wave wave = {0};
    WaveFileHeader *header = (WaveFileHeader *)file.data;
    void *data = ((u8 *)file.data + sizeof(WaveFileHeader)); 
    if(!data) {
        return wave;
    }
    wave.data = data;
    wave.size = header->nSubChunk2Size;
    return wave;
}
