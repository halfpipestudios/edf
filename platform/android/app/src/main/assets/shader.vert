#version 300 es
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUvs;
layout (location = 2) in vec4 aColor;

out vec4 Color;
out vec2 Uvs;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(aPos.x, aPos.y, 0.0, 1.0);
    Color = aColor;
    Uvs = aUvs;
}