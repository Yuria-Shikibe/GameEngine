#version 330 core

uniform lowp sampler2D texture0;
uniform lowp sampler2D texture1;

uniform lowp float intensity_blo;
uniform lowp float intensity_ori;

in vec2 texCoord;

void main(){
	vec4 original = texture2D(texture0, texCoord) * intensity_ori;
	vec4 bloom = texture2D(texture1, texCoord) * intensity_blo;
	original = original *  (vec4(1.0) - bloom);
	vec4 combined =  original + bloom;
	float mx = min(max(combined.r, max(combined.g, combined.b)), 1.0);

	gl_FragColor = vec4(combined.rgb / max(mx, 0.0001), mx);
}
