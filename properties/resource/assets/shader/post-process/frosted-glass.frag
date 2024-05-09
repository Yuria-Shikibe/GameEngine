#version 430 core

in vec2 texCoord;
out vec4 FragColor;

uniform mediump sampler2D texture;
uniform mediump vec2 norStep;

void main(){
    vec4 sum = vec4(0.0f);

    sum += texture2D(texture, vec2(texCoord.x - 5.0 * norStep.x, texCoord.y + 3.0 * norStep.y)) * 0.025;
    sum += texture2D(texture, vec2(texCoord.x + 5.0 * norStep.x, texCoord.y - 3.0 * norStep.y)) * 0.025;
    sum += texture2D(texture, vec2(texCoord.x - 4.0 * norStep.x, texCoord.y + 2.5 * norStep.y)) * 0.05;
    sum += texture2D(texture, vec2(texCoord.x + 4.0 * norStep.x, texCoord.y - 2.5 * norStep.y)) * 0.05;
    sum += texture2D(texture, vec2(texCoord.x - 3.0 * norStep.x, texCoord.y + 2.0 * norStep.y)) * 0.09;
    sum += texture2D(texture, vec2(texCoord.x + 3.0 * norStep.x, texCoord.y - 2.0 * norStep.y)) * 0.09;
    sum += texture2D(texture, vec2(texCoord.x - 2.0 * norStep.x, texCoord.y + 1.5 * norStep.y)) * 0.12;
    sum += texture2D(texture, vec2(texCoord.x + 2.0 * norStep.x, texCoord.y - 1.5 * norStep.y)) * 0.12;
    sum += texture2D(texture, vec2(texCoord.x - 1.0 * norStep.x, texCoord.y + 1.0 * norStep.y)) * 0.15;
    sum += texture2D(texture, vec2(texCoord.x + 1.0 * norStep.x, texCoord.y - 1.0 * norStep.y)) * 0.15;

    sum += texture2D(texture, texCoord - 3.0 * norStep) * 0.025;
    sum += texture2D(texture, texCoord - 2.5 * norStep) * 0.05;
    sum += texture2D(texture, texCoord - 2.0 * norStep) * 0.09;
    sum += texture2D(texture, texCoord - 1.5 * norStep) * 0.12;
    sum += texture2D(texture, texCoord - 1.0 * norStep) * 0.15;

    sum += texture2D(texture, texCoord) * 0.16;

    sum += texture2D(texture, texCoord + 3.0 * norStep) * 0.15;
    sum += texture2D(texture, texCoord + 2.5 * norStep) * 0.12;
    sum += texture2D(texture, texCoord + 2.0 * norStep) * 0.09;
    sum += texture2D(texture, texCoord + 1.5 * norStep) * 0.05;
    sum += texture2D(texture, texCoord + 1.0 * norStep) * 0.025;

    FragColor = sum / 1.90f;

}
