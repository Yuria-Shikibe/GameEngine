#version 450 core

uniform sampler2DArray texArray;

in vec4 v_srcColor;
in vec2 texCoord;
in vec4 v_mixColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 NormalColor;
layout (location = 2) out vec4 LightColor;
layout (location = 3) out vec4 DataCoord;

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
	vec4 norColor = texture(texArray, vec3(texCoord, 1.0f));
	vec4 lightColor = texture(texArray, vec3(texCoord, 2.0f));

	lightColor = v_srcColor * mix(lightColor, vec4(v_mixColor.rgb, lightColor.a), v_mixColor.a);

	float weightedA = max(baseColor.a, lightColor.a * 0.9f);

	if(weightedA > 0.895f){
		gl_FragDepth = invDepth;//mix(gl_FragDepth, invDepth, baseColor.a * 0.75f + 0.25f);
	}

	FragColor = baseColor;

	NormalColor = vec4(norColor.rgb, baseColor.a);
	LightColor = vec4(mix(vec3(0.0f), lightColor.rgb, lightColor.a), 1.0);

	DataCoord.a = step(0.95f, baseColor.a);
	DataCoord.x = gl_FragCoord.z;
	DataCoord.y = lightColor.a;

	//Dont care the normal texture map's color, using the alpha channel as the z data;
//	FragColor = vec4(vec3(gl_FragCoord.z), baseColor.a);
}