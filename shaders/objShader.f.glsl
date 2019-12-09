//Fragment shader

#version 330 core

in vec3 theColor;
in vec2 texCoords;
out vec4 fragColorOut;

uniform sampler2D txtr;

void main(){
    vec4 texel = texture(txtr, texCoords);
    fragColorOut = vec4(theColor, 1.0);
    fragColorOut *= texel;
}