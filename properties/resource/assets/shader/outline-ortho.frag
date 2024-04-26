#version 430

in vec2 texCoord;
out vec4 FragColor;

uniform vec2 scaleInv;
uniform float stepLength;
uniform sampler2D texture;

vec4 sampleEdge(){
    return max(max(max(
        texture2D(texture, texCoord + vec2( stepLength,  stepLength) * scaleInv),
        texture2D(texture, texCoord + vec2( stepLength, -stepLength) * scaleInv)),
        texture2D(texture, texCoord + vec2(-stepLength,  stepLength) * scaleInv)),
        texture2D(texture, texCoord + vec2(-stepLength, -stepLength) * scaleInv)
    );
}


void main() {
    vec4 color = texture2D(texture, texCoord);
    vec4 maxed = sampleEdge();

    if(maxed.a > 0.001f && color.a == 0.0f){
        FragColor = maxed;
    }else{
        discard;
    }

}
