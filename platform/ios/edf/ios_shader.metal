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
    uint array_index;
    uint texture_index;
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
    out.uvs = vertexArray[vertexID].uvs;
    out.uvs.x *= uniforms[instanceID].u_ratio;
    out.uvs.y *= uniforms[instanceID].v_ratio;
    out.array_index = uniforms[instanceID].array_index;
    out.texture_index = uniforms[instanceID].texture_index;
    out.color = uniforms[instanceID].color;
    return out;
    
}

fragment float4
fragmentShader(RasterizerData in [[stage_in]],
              array<texture2d_array<float ,  access::sample>, 5> textures [[texture(0)]] ) {

    constexpr sampler defaultSampler;
    float4 texture_color = textures[in.array_index].sample(defaultSampler, in.uvs, in.texture_index);
    float3 tint_color = texture_color.rgb * in.color;
    
    return float4(tint_color, texture_color.a);
}
