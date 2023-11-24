#version 330 core
layout (location = 0) attribute vec4 pos;
layout (location = 1) attribute vec2 texCoord;
layout (location = 2) attribute vec4 mixColor;
layout (location = 3) attribute vec4 scrColor;

uniform mat3 view;
//uniform mat3 projection;

varying vec2 v_texCoord;
varying vec4 v_mixColor;
varying vec4 v_srcColor;

void main()
{
	v_mixColor = mixColor;
	v_srcColor = scrColor;
	v_texCoord = texCoord;
	gl_Position = vec4((view * vec3(pos.r, pos.g, 1.0)).xy, pos.b, pos.a);
}