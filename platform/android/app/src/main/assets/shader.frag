#version 300 es
precision mediump float;

out vec4 FragColor;

in vec4 Color;
in vec2 Uvs;

uniform sampler2D atlas;

void main() {
    FragColor = texture(atlas, Uvs) * Color, 1.0;
}