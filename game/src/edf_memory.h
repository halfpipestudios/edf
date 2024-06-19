//
//  edf_memory.h
//  edf
//
//  Created by Manuel Cabrerizo on 19/06/2024.
//

#ifndef EDF_MEMORY_H
#define EDF_MEMORY_H

typedef struct Memory {
    void *data;
    sz used;
    sz size;
} Memory;

typedef struct Arena {
    void *data;
    sz used;
    sz size;
} Arena;


#endif // EDF_MEMORY_H
