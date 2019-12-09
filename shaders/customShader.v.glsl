/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

layout (location = 0) in vec3 vPosition;

out vec3 texCoords;
out vec4 fragPos;
out vec3 camPos;

uniform vec3 eyePos;
uniform mat4 mvpMatrix;


void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************

    texCoords = vPosition;
    camPos = eyePos; 
    fragPos = mvpMatrix * vec4(vPosition, 1.0);
    gl_Position = mvpMatrix * vec4(vPosition, 1.0);
}
