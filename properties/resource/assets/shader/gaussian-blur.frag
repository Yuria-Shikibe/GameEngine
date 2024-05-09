#version 330

uniform lowp sampler2D texture;
in vec2 texCoord;
in vec2 texCoord1;
in vec2 texCoord2;
in vec2 texCoord3;
in vec2 texCoord4;

//Original
//const float center = 0.2270270270;
//const float close = 0.3162162162;
//const float far = 0.0702702703;

const float center = 0.22970270270;
const float close = 0.33062162162;
const float far = 0.0822702703;

void main(){
	gl_FragColor = clamp(far * texture2D(texture, texCoord)
	+ close * texture2D(texture, texCoord1)
	+ center * texture2D(texture, texCoord2)
	+ close * texture2D(texture, texCoord3)
	+ far * texture2D(texture, texCoord4), vec4(0.0f), vec4(1.0f));
}

