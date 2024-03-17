#version 450 core

uniform sampler2DArray texArray;

in float depth;

in vec4 v_srcColor;
in vec2 v_texCoord;
in vec4 v_mixColor;

layout (location = 0) out vec4 FragColor;

layout (depth_less) out float gl_FragDepth;

float layer2coord(uint capacity, uint layer)
{
	return max(0, min(float(capacity - 1), floor(float(layer) + 0.5)));
}

void main()
{
//	if(v_srcColor.a < 0.0033)discard;
	vec4 c = texture(texArray, vec3(v_texCoord, 0.0f));

	c = v_srcColor * mix(c, vec4(v_mixColor.rgb, c.a), v_mixColor.a);

	if(gl_FragDepth == 0.0f){
		gl_FragDepth = 1.0f;
	}

	if(depth < gl_FragDepth){
		if(c.a > 0.995){
			gl_FragDepth = depth;
			FragColor = c;
		}
	}else{
		discard;
	}
}