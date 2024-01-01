#version 330 core

uniform sampler2D texture;

in vec2 texCoord;

void main() {
	gl_FragColor = texture2D(texture, texCoord);
}
