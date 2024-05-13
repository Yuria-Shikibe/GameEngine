#version 330 core

uniform sampler2D u_texture;

in vec4 srcColor;
in vec2 texCoord;
in vec4 mixColor;

void main()
{
	if(srcColor.a < 0.0033)discard;
	vec4 c = texture2D(u_texture, texCoord);
	gl_FragColor = srcColor * mix(c, vec4(mixColor.rgb, c.a), mixColor.a);
}