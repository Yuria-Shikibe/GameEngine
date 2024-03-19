#version 430

const float stepMin = 1.00f;
const float stepMax = 2.0f;
const float paramHori[9] ={
stepMin, 0.0f, -stepMin,
stepMax, 0.0f, -stepMax,
stepMin, 0.0f, -stepMin,
};

const float paramVert[9] ={
stepMin, stepMax, stepMin,
0.0f, 0.0f, 0.0f,
-stepMin, -stepMax, -stepMin,
};

uniform vec2 screenSizeInv;

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D texture;

float sobel(vec2 coord){
    const float step = 1.75f;
    const vec2 offset[9] = {
    vec2(-step, -step),
    vec2(  0.0, -step),
    vec2( step, -step),

    vec2(-step,  0.0),
    vec2( 0.0,   0.0),
    vec2( step,  0.0),

    vec2(-step,  step),
    vec2(  0.0,  step),
    vec2( step,  step),
    };

    float hori = 0.0f;
    float vert = 0.0f;

    for(int i = 0; i < 9; i++){
        vec2 T = offset[i] * screenSizeInv + texCoord;
        hori += texture(texture, T).a * paramHori[i];
        vert += texture(texture, T).a * paramVert[i];
    }

    return sqrt(vert * vert + hori * hori);
}

void main() {
    vec4 baseColor = texture(texture, texCoord);

    vec4 edge = vec4(0.15f, 0.15f, 0.15f, smoothstep(0.15f, 0.25f, sobel(texCoord)));
    float factor = edge.a;// * (1 - baseColor.a);

    baseColor = vec4(
        baseColor.rgb * (1.0f - factor) + edge.rgb * factor,
        baseColor.a * (1.0f - factor) + edge.a
    );

    FragColor = baseColor;
}
