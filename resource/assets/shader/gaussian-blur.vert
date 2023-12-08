#version 330

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 tex;

uniform vec2 direction;
uniform vec2 size;

out vec2 v_texCoords0;
out vec2 v_texCoords1;
out vec2 v_texCoords2;
out vec2 v_texCoords3;
out vec2 v_texCoords4;

const vec2 futher = vec2(3.2307692308, 3.2307692308);
const vec2 closer = vec2(1.3846153846, 1.3846153846);

void main(){
	vec2 texCoord = tex;
	
	vec2 sizeAndDir = direction / size;
	vec2 f = futher*sizeAndDir;
	vec2 c = closer*sizeAndDir;
	
	v_texCoords0 = texCoord - f;
	v_texCoords1 = texCoord - c;
	v_texCoords2 = texCoord;
	v_texCoords3 = texCoord + c;
	v_texCoords4 = texCoord + f;

	
	gl_Position = pos;
}


