#version 430 core

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;

layout (location = 0) out vec4 FragColor0;
layout (location = 1) out vec4 FragColor1;
layout (location = 2) out vec4 FragColor2;

in vec2 texCoord;

void main() {
	FragColor0 = texture2D(texture0, texCoord);
	FragColor1 = texture2D(texture1, texCoord);
	FragColor2 = texture2D(texture2, texCoord);
}
