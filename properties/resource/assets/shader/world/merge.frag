#version 430 core
#define HIGHP

#define MaxKernalSize 32

in vec2 texCoord;

uniform sampler2D texBase;
uniform sampler2D texNormal;
uniform sampler2D texLight;
uniform sampler2D texData;
uniform sampler2D texBloom;

uniform vec2 kernal[MaxKernalSize];
uniform uint kernalSize;
uniform vec2 scale;

const float intensity_blo = 1.25f;
const float intensity_ori = 1.05f;

layout (location = 0) out vec4 FragColor;
//layout (location = 1) out vec4 SSAOColor;
const float radius = 6.f;
const float zDiffScl = 1.8f;

float valueOf(vec3 color){
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

void main() {
    float originalAlpha = 0.0f;
    vec4 baseColor = texture2D(texBase, texCoord);
    originalAlpha = baseColor.a;


    vec4 normalColor = texture2D(texNormal, texCoord);
    vec4 lightColor = texture2D(texLight, texCoord);
    vec4 coordData = texture2D(texData, texCoord);


    //    vec4 fog = vec4(0.2f, 0.2f, 0.23f, 0.00f);

    vec4 original = lightColor * intensity_ori;
    vec4 bloom = texture2D(texBloom, texCoord) * intensity_blo;
    vec4 combined =  original * (vec4(1.0) - bloom) + bloom;
    float mx = min(max(combined.r, max(combined.g, combined.b)), 1.0);
    lightColor = vec4(combined.rgb / max(mx, 0.00015f), mx);

    //    fog.a = max(fog.a - lightColor.a * (0.85f - fog.a * 0.035f), 0.0f);
    //    lightColor.a = max(lightColor.a * 0.15f, lightColor.a - fog.a * (0.2f * lightColor.a));

    //    baseColor = mix(vec4(
    //        baseColor.rgb * (1.0f - lightColor.a) + lightColor.rgb * lightColor.a,
    //        originalAlpha * (1.0f - lightColor.a) + lightColor.a
    //    ), normalColor, 0.00001f);

    baseColor =  vec4(
        baseColor.rgb * (1.0f - lightColor.a) + lightColor.rgb * lightColor.a,
        originalAlpha * (1.0f - lightColor.a) + lightColor.a
    );

    //    vec3 fragPos = vec3(texCoord.xy, normalColor.a);
    //    const vec3 normal = vec3(0.0, 0.0, 1.0);//texture(gNormal, TexCoords).rgb;
    // vec3 randomVec = vec3(1.0, 0.0f, 0.0f);//texture(texNoise, TexCoords * noiseScale).xyz;
    // Create TBN change-of-basis matrix: from tangent-space to view-space
    //    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    //    vec3 bitangent = cross(normal, tangent);
    //    mat3 TBN = mat3(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), normal);


    float occlusion = 0.0;
    const float depth = coordData.x;

    if (originalAlpha > 0.0033f){
        for (uint i = 0; i < kernalSize; ++i){
            // get smp position
            const vec2 smp = texCoord + kernal[i] * radius * scale;// From tangent to view-space

            const float sampleDepth = texture2D(texData, smp).x;// Get depth value of kernel smp
            if(sampleDepth >= depth)continue;

            const float alphaCheck = texture2D(texBase, smp).a;// Get depth value of kernel smp
            const float light = texture2D(texData, smp).y * 3.0f;// Get depth value of kernel smp

            const float thresHold = smoothstep(0.8, 1.0, (1 - light) * alphaCheck);

            if(thresHold > 0.0f){
                const float rangeCheck = smoothstep(0.0, 1.0, radius / abs(depth - sampleDepth) * zDiffScl);
                occlusion += rangeCheck * thresHold;
            }else{
                occlusion -= light;
            }
            // range check & accumulate
        }
    }

    occlusion = 1.0 - (max(occlusion * (1 - lightColor.a * 1.15f), 0) / kernalSize);

    FragColor = vec4(baseColor.rgb * occlusion, baseColor.a);

//    FragColor = vec4(vec3(texture(texBloom, texCoord).a), baseColor.a);
//    FragColor = vec4(lightColor);

    //    baseColor = vec4(
    //    baseColor.rgb * (1.0f - fog.a) + fog.rgb * fog.a,
    //    originalAlpha * (1.0f - fog.a) + fog.a
    //    );


    //    FragColor = vec4(vec3(depth), baseColor.a);
    //    FragColor = baseColor;
    //    FragColor = vec4(vec3(normalColor.a), mix(baseColor.a, originalAlpha, 1 -0.001f));
    //    SSAOColor = vec4(occlusion, occlusion, occlusion, originalAlpha);
}
