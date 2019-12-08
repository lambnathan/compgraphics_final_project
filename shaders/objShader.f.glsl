//Fragment shader

#version 330 core

in vec3 theColor;

out vec4 fragColorOut;

void main(){

    fragColorOut = vec4(theColor, 1.0);

}