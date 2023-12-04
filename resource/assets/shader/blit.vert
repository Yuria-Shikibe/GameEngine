#version 330

layout (location = 0) attribute vec4 pos;
layout (location = 1) attribute vec2 tex;

varying vec2 texCoord;

void main() {
	texCoord = tex;
	gl_Position = pos;
}
