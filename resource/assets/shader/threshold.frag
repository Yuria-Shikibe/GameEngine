#version 330 core

uniform lowp sampler2D u_texture;
uniform lowp vec2 threshold;
in vec2 texCoord;

void main(){
	vec4 color = texture2D(u_texture, texCoord);
	if(0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b > 0.5){
		gl_FragColor = color;
	}else{
		discard;
	}
	//gl_FragColor = (texture2D(u_texture0, v_texCoords) - vec4(threshold.r))  * threshold.g;
}

