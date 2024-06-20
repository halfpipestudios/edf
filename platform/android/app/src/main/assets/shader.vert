#version 300 es
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUvs;
layout (location = 2) in vec3 aColor;

out vec3 Color;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(aPos.x, aPos.y, 0.0, 1.0);
    Color = aColor;
}