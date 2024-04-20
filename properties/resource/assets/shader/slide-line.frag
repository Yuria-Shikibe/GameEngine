#version 330 core


in vec2 v_texCoord;
in vec4 v_mixColor;
in vec4 v_srcColor;

uniform sampler2D u_texture;
uniform float time;

uniform float width;
uniform float spacing;
uniform vec4 mulColor;
uniform float mulSub;


void main()
{
	if(v_srcColor.a < 0.0033)discard;
	vec4 c = texture2D(u_texture, v_texCoord);

	float s = spacing;
	if(s < 0.0f){
		s = width * 2.0f;
	}
	
	c = v_srcColor * mix(c, vec4(v_mixColor.rgb, c.a), v_mixColor.a);

	if(mulSub >= 0){
		c *= mix(vec4(1.0f), mulColor, mulSub * step(mod(gl_FragCoord.x - gl_FragCoord.y - time, s), width));
	}else{
		c *= c * (1 + mulSub * step(mod(gl_FragCoord.x - gl_FragCoord.y - time, s), width));
	}

	gl_FragColor = c;
}
