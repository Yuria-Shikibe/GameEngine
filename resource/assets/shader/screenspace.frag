#version 430 core

uniform sampler2D u_texture;

varying vec4 v_srcColor;
varying vec2 v_texCoord;
varying vec4 v_mixColor;

void main()
{
	if(v_srcColor.a < 0.0033)discard;
	vec4 c = texture2D(u_texture, v_texCoord);
	gl_FragColor = v_srcColor * mix(c, vec4(v_mixColor.rgb, c.a), v_mixColor.a);
}