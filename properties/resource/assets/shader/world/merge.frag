#version 430 core

in vec2 texCoord;

uniform sampler2D texBase;
uniform sampler2D texNormal;
uniform lowp sampler2D texLight;
uniform lowp sampler2D bloom;

const lowp float intensity_blo = 1.1f;
const lowp float intensity_ori = 1.25f;

out vec4 FragColor;

void main() {
    vec4 baseColor = texture(texBase, texCoord);
    vec4 normalColor = texture(texNormal, texCoord);
    vec4 lightColor = texture(texLight, texCoord);

    vec4 original = lightColor * intensity_ori;
    vec4 bloom = texture2D(bloom, texCoord) * intensity_blo;
    vec4 combined =  original * (vec4(1.0) - bloom) + bloom;
    float mx = min(max(combined.r, max(combined.g, combined.b)), 1.0);
    lightColor = vec4(combined.rgb / max(mx, 0.0001), mx);

    baseColor = vec4(
        baseColor.rgb * (1.0f - lightColor.a) + lightColor.rgb * lightColor.a,
        baseColor.a * (1.0f - lightColor.a) + lightColor.a
    );

    FragColor = baseColor;
}
