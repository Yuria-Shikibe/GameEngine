#version 330 core


in vec2 texCoord;
in vec4 mixColor;
in vec4 srcColor;

uniform sampler2D texture;
uniform float time;

uniform float width;
uniform float spacing;
uniform vec4 mulColor;
uniform float mulSub;
uniform vec2 scale;
uniform float angle;

uniform mat3 view;
uniform mat3 localToWorld;

float getIfInLine(vec2 T){

	float sin_ = sin(radians(angle));
	float cos_ = cos(radians(angle));

	vec2 nor = vec2(cos_ - sin_, sin_ + cos_);
	float len = dot(nor, T);

	return mulSub * step(mod(len - time, spacing), width);
}

void main()
{
	if(srcColor.a < 0.0033)discard;
	vec4 c = texture2D(texture, texCoord);

	vec2 coord = (inverse(localToWorld) * inverse(view) * vec3(gl_FragCoord.xy * scale * 2.0f - vec2(1.0f, 1.0f), 1)).xy;

	c *= srcColor;

	if(mulSub >= 0){
		c *= mix(vec4(1.0f), mulColor, getIfInLine(coord));
	}else{
		c *= c * (1 + getIfInLine(coord));
	}

	c = mix(c, vec4(mixColor.rgb, c.a), mixColor.a);

	gl_FragColor = c;
}
