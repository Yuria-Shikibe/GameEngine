#version 330

uniform lowp sampler2D u_texture;
in vec2 v_texCoords0;
in vec2 v_texCoords1;
in vec2 v_texCoords2;
in vec2 v_texCoords3;
in vec2 v_texCoords4;

//Original
//const float center = 0.2270270270;
//const float close = 0.3162162162;
//const float far = 0.0702702703;

const float center = 0.2370270270;
const float close = 0.3362162162;
const float far = 0.0822702703;

void main(){
	gl_FragColor = far * texture2D(u_texture, v_texCoords0)
	+ close * texture2D(u_texture, v_texCoords1)
	+ center * texture2D(u_texture, v_texCoords2)
	+ close * texture2D(u_texture, v_texCoords3)
	+ far * texture2D(u_texture, v_texCoords4);
}

