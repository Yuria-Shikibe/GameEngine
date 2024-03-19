#version 330 core

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 tex;

out vec2 texCoord;

void main() {
    texCoord = tex;
    gl_Position = pos;
}
