#version 330 core

uniform lowp sampler2D u_texture;
uniform lowp float threshold;
in vec2 texCoord;

void main(){
	vec4 color = texture2D(u_texture, texCoord);
	if(0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b > threshold){
		gl_FragColor = color;
	}else{
//		gl_FragColor = vec4(0.0f);
		discard;
	}
}

