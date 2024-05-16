#version 450 core

uniform sampler2DArray texArray;

in float depth;

in vec4 v_srcColor;
in vec2 texCoord;
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
	float invDepth = 1 - gl_FragCoord.z;

	if(v_srcColor.a < 0.0033 || invDepth < gl_FragDepth)discard;
	vec4 baseColor = texture(texArray, vec3(texCoord, 0.0f));
	vec4 normalColor = texture(texArray, vec3(texCoord, 1.0f));
	vec4 lightColor = texture(texArray, vec3(texCoord, 2.0f));


	lightColor = v_srcColor * mix(lightColor, vec4(v_mixColor.rgb, lightColor.a), v_mixColor.a);

	float weightedA = max(baseColor.a, lightColor.a * 0.9f);

	if(weightedA > 0.895f){
		gl_FragDepth = invDepth;//mix(gl_FragDepth, invDepth, baseColor.a * 0.75f + 0.25f);
	}

	FragColor = baseColor;
	NormalColor.rgb = normalColor.rgb;
	LightColor = mix(vec4(0.0f, 0.0f, 0.0f, 1.0f), lightColor, lightColor.a);
	//Dont care the normal texture map's color, using the alpha channel as the z data;
	NormalColor.a = gl_FragCoord.z;
}