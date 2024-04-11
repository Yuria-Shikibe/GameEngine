#version 430 core

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float clamp;
uniform vec4 mixColor;
uniform vec4 srcColor;

in vec2 texCoord;

out vec4 FragColor;

void main(){
    vec4 src = texture(texture0, texCoord);
    vec4 dst = texture(texture1, texCoord);
    vec4 mask = texture(texture2, texCoord);

    src *= mix(src, vec4(mixColor.rgb, 1.0f), mixColor.a);
    src *= srcColor;

    mask.a = min(mask.a, clamp);

    FragColor = mix(dst, src, mask.a);
}
