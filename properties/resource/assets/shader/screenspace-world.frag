#version 450 core

uniform sampler2DArray texArray;

in float depth;

in vec4 v_srcColor;
in vec2 v_texCoord;
in vec4 v_mixColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 NormalColor;
layout (location = 2) out vec4 LightColor;

layout (depth_less) out float gl_FragDepth;

float layer2coord(uint capacity, uint layer)
{
	return max(0, min(float(capacity - 1), floor(float(layer) + 0.5)));
}

void main()
{
	if(v_srcColor.a < 0.0033)discard;
	vec4 baseColor = texture(texArray, vec3(v_texCoord, 0.0f));
	vec4 normalColor = texture(texArray, vec3(v_texCoord, 1.0f));
	vec4 lightColor = texture(texArray, vec3(v_texCoord, 2.0f));

	baseColor = v_srcColor * mix(baseColor, vec4(v_mixColor.rgb, baseColor.a), v_mixColor.a);

	float invDepth = 1 - gl_FragCoord.z;

	if(invDepth > gl_FragDepth){
		float weightedA = max(baseColor.a, lightColor.a * 0.85f);

		if(weightedA > 0.895f){
			gl_FragDepth = invDepth;//mix(gl_FragDepth, invDepth, baseColor.a * 0.75f + 0.25f);

			FragColor = baseColor;
			NormalColor.rgb = normalColor.rgb;
			LightColor = mix(vec4(0.0f, 0.0f, 0.0f, 1.0f), lightColor, lightColor.a);

			//Dont care the normal texture map's color, using the alpha channel as the z data;
			NormalColor.a = gl_FragCoord.z;
		}else{
			discard;
		}
	}else{
		discard;
	}
}