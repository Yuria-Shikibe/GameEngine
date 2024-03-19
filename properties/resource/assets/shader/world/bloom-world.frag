#version 430 core

uniform lowp sampler2D texture0;
uniform lowp sampler2D texture1;
uniform lowp float intensity_blo;
uniform lowp float intensity_ori;

in vec2 texCoord;

out vec4 FragColor;

void main(){
	vec4 original = texture2D(texture0, texCoord) * intensity_ori;
	vec4 bloom = texture2D(texture1, texCoord) * intensity_blo;

	vec4 combined = original * (vec4(1.0) - bloom) + bloom;
	float mx = min(max(combined.r, max(combined.g, combined.b)), 1.0);

	FragColor = mix(vec4(
		original.rgb * (1.0f - bloom.a) + bloom.rgb * bloom.a,
		original.a * (1.0f - bloom.a) + bloom.a
	), vec4(combined.rgb / max(mx, 0.0001), mx), 0.7f);

	FragColor = mix(FragColor, bloom, 0.995f);
}
