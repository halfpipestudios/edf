//
//  ios_shader.metal
//  edf
//
//  Created by Manuel Cabrerizo on 19/06/2024.
//

#include <metal_stdlib>
#include "ios_shader_types.h"
using namespace metal;

struct RasterizerData {
    float4 position [[position]];
    float3 color;
    float2 uvs;
    uint textureId;
};

vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             uint instanceID [[instance_id]],
             constant Vertex *vertexArray [[buffer(VertexInputIndexVertices)]],
             constant Uniform *uniforms [[buffer(VertexInputIndexWorld)]],
             constant matrix_float4x4 &view  [[buffer(VertexInputIndexView)]],
             constant matrix_float4x4 &proj  [[buffer(VertexInputIndexProj)]]) {
    
    RasterizerData out;
    float4 position = float4(vertexArray[vertexID].pos.xy, 0.0, 1.0f);
    out.position = proj * view * uniforms[instanceID].world * position;
    out.color = vertexArray[vertexID].color;
    out.uvs = vertexArray[vertexID].uvs;
    out.textureId = uniforms[instanceID].texture_id;
    return out;
    
}

fragment float4
fragmentShader(RasterizerData in [[stage_in]],
               texture2d_array<float ,  access::sample> textures [[ texture(0) ]]) {
    constexpr sampler defaultSampler;
    float4 color = textures.sample(defaultSampler, in.uvs, in.textureId);
    return color;
}
