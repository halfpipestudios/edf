//
//  ios_shader_types.h
//  edf
//
//  Created by Manuel Cabrerizo on 19/06/2024.
//

#ifndef IOS_SHADER_TYPES_H
#define IOS_SHADER_TYPES_H

#include <simd/simd.h>

typedef enum VertexInputIndex {
    VertexInputIndexVertices = 0,
    VertexInputIndexWorld = 1,
    VertexInputIndexView = 2,
    VertexInputIndexProj = 3
} VertexInputIndex;

typedef struct {
    vector_float2 pos;
    vector_float3 color;
    vector_float2 uvs;
} Vertex;

typedef struct Uniform {
    matrix_float4x4 world;
    uint32_t texture_id;
} Uniform;

#endif /* IOS_SHADER_TYPES_H */
