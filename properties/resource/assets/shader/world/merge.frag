#version 430 core

in vec2 texCoord;

uniform sampler2D texBase;
uniform sampler2D texNormal;
uniform mediump sampler2D texLight;
uniform mediump sampler2D bloom;

const mediump float intensity_blo = 1.15f;
const mediump float intensity_ori = 1.0f;

out vec4 FragColor;

void main() {
    vec4 baseColor = texture(texBase, texCoord);
    vec4 normalColor = texture(texNormal, texCoord);
    vec4 lightColor = texture(texLight, texCoord);

    vec4 fog = vec4(0.2f, 0.2f, 0.23f, 0.00f);

    vec4 original = lightColor * intensity_ori;
    vec4 bloom = texture2D(bloom, texCoord) * intensity_blo;
    vec4 combined =  original * (vec4(1.0) - bloom) + bloom;
    float mx = min(max(combined.r, max(combined.g, combined.b)), 1.0);
    lightColor = vec4(combined.rgb / max(mx, 0.00015f), mx);

    fog.a = max(fog.a - lightColor.a * (0.85f - fog.a * 0.035f), 0.0f);
//    lightColor.a = max(lightColor.a * 0.15f, lightColor.a - fog.a * (0.2f * lightColor.a));

    baseColor = mix(vec4(
        baseColor.rgb * (1.0f - lightColor.a) + lightColor.rgb * lightColor.a,
        baseColor.a * (1.0f - lightColor.a) + lightColor.a
    ), normalColor, 0.00001f);

//    baseColor = vec4(
//    baseColor.rgb * (1.0f - fog.a) + fog.rgb * fog.a,
//    baseColor.a * (1.0f - fog.a) + fog.a
//    );


    FragColor = baseColor;
}
