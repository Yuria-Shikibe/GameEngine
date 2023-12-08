#version 330 core
out vec4 FragColor;

void main()
{
   
   FragColor = vec4(gl_FragCoord.xy / 3000, vec2(1.0, 1.0) - gl_FragCoord.xy / 3000);
}