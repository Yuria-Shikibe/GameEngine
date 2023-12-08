#version 330

layout (location = 0) in vec4 pos;

out vec2 texCoord;

void main() {
	texCoord = (pos.xy + vec2(1.0f, 1.0f)) * 0.5f;
	gl_Position = pos;
}
