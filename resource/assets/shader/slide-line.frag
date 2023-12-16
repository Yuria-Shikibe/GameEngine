#version 330 core


in vec2 v_texCoord;
in vec4 v_mixColor;
in vec4 v_srcColor;

uniform sampler2D u_texture;
uniform float time;

const float lineWidth = 25;
const float lineSpacing = lineWidth * 2.0f;

void main()
{
	if(v_srcColor.a < 0.0033)discard;
	vec4 c = texture2D(u_texture, v_texCoord);
	
	c = v_srcColor * mix(c, vec4(v_mixColor.rgb, c.a), v_mixColor.a);

	c *= 1.0f - 0.2f * step(mod(gl_FragCoord.x - gl_FragCoord.y - time, lineSpacing), lineWidth);
	
	gl_FragColor = c;
}
