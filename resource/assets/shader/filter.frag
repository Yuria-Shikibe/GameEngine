#version 330

uniform sampler2D tex;

in vec2 texCoord;

out vec4 FragColor;

void main() {
	vec4 color = texture2D(tex, texCoord);
	
	color.rgb *= 1.1f;
	if(color.a < 0.3f){
		color = vec4(0.2f, 0.662f, 0.99f, 0.4f);
	}
	
	FragColor = color;
}
