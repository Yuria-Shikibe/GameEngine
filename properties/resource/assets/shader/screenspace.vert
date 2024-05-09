#version 330 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec4 mixColor;
layout (location = 3) in vec4 scrColor;

uniform mat3 view;
uniform mat3 localToWorld;
//uniform mat3 projection;

out vec2 texCoord;
out vec4 v_mixColor;
out vec4 v_srcColor;


void main()
{
	v_mixColor = mixColor;
	v_srcColor = scrColor;
	texCoord = tex;
	gl_Position = vec4((view * localToWorld * vec3(pos.r, pos.g, 1.0)).xy, pos.b, pos.a);
}