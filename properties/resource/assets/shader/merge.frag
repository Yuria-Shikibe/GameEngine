#version 430 core

in vec2 texCoord;

uniform sampler2D texBase;
uniform sampler2D texNormal;
uniform sampler2D texLight;

out vec4 FragColor;

void main() {
    vec4 baseColor = texture(texBase, texCoord);
    vec4 normalColor = texture(texNormal, texCoord);
    vec4 lightColor = texture(texLight, texCoord);

    FragColor =
        mix(vec4(
        baseColor.rgb * (1.0f - lightColor.a) + lightColor.rgb * lightColor.a,
        baseColor.a * (1.0f - lightColor.a) + lightColor.a
        ), normalColor, 0.0001f);

}
