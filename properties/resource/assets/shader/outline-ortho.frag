#version 430

out vec4 FragColor;

uniform vec2 scaleInv;
uniform float stepLength;
uniform sampler2D texture;
uniform float rot;

in vec2 texCoord;


vec4 sampleEdge(){
    float sin_ = sin(radians(rot));
    float cos_ = cos(radians(rot));

    vec2 rotated = vec2(cos_ - sin_, sin_ + cos_);
    rotated *= stepLength;

    return max(max(max(
        texture2D(texture, texCoord + vec2( rotated.x,  rotated.y) * scaleInv),
        texture2D(texture, texCoord + vec2(-rotated.y,  rotated.x) * scaleInv)),
        texture2D(texture, texCoord + vec2(-rotated.x, -rotated.y) * scaleInv)),
        texture2D(texture, texCoord + vec2( rotated.y, -rotated.x) * scaleInv)
    );
}


void main() {
    vec4 color = texture2D(texture, texCoord);
    vec4 maxed = sampleEdge();

//    FragColor = color;
    if(maxed.a > 0.0033f && color.a <= 0.0033f){
        FragColor = vec4(maxed.rgb, 1.0f);
        //        FragColor = v_srcColor * mix(color, vec4(v_mixColor.rgb, color.a), v_mixColor.a);
    }else{
        discard;
    }

}
