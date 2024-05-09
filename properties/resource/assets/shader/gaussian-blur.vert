#version 330

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 tex;

uniform vec2 direction;
uniform vec2 size;

out vec2 texCoord;
out vec2 texCoord1;
out vec2 texCoord2;
out vec2 texCoord3;
out vec2 texCoord4;

const vec2 futher = vec2(3.2307692308, 3.2307692308);
const vec2 closer = vec2(1.3846153846, 1.3846153846);

void main(){
	vec2 sizeAndDir = direction * size;
	vec2 f = futher*sizeAndDir;
	vec2 c = closer*sizeAndDir;
	
	texCoord = tex - f;
	texCoord1 = tex - c;
	texCoord2 = tex;
	texCoord3 = tex + c;
	texCoord4 = tex + f;
	
	gl_Position = pos;
}


