#version 330

attribute vec4 position;

uniform vec2 direction;
uniform vec2 size;

varying vec2 v_texCoords0;
varying vec2 v_texCoords1;
varying vec2 v_texCoords2;
varying vec2 v_texCoords3;
varying vec2 v_texCoords4;

const vec2 futher = vec2(3.2307692308, 3.2307692308);
const vec2 closer = vec2(1.3846153846, 1.3846153846);

void main(){
	vec2 texCoord = (position.xy + vec2(1.0f, 1.0f)) * 0.5f;
	
	vec2 sizeAndDir = direction / size;
	vec2 f = futher*sizeAndDir;
	vec2 c = closer*sizeAndDir;
	
	v_texCoords0 = texCoord - f;
	v_texCoords1 = texCoord - c;
	v_texCoords2 = texCoord;
	v_texCoords3 = texCoord + c;
	v_texCoords4 = texCoord + f;
	
	gl_Position = position;
}


