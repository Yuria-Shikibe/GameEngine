#version 430 core

uniform sampler2D srcTex;
uniform sampler2D dstTex;
uniform sampler2D maskTex;

//uniform float clamp;
uniform vec4 mixColor;
uniform vec4 srcColor;
//uniform vec4 baseColor;

in vec2 texCoord;

out vec4 FragColor;

void main(){
    vec4 src = texture(srcTex, texCoord);
    vec4 dst = texture(dstTex, texCoord);
    vec4 mask = texture(maskTex, texCoord);
//
    src = mix(src, vec4(mixColor.rgb, src.a), mixColor.a);
    src *= srcColor;

    FragColor = vec4(mix(dst, src, mask.a));
}
