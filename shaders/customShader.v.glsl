/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

layout (location = 0) in vec3 vPosition;

out vec3 texCoords;

uniform mat4 mvpMatrix;


void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************

    texCoords = vPosition;
    gl_Position = mvpMatrix * vec4(vPosition, 1.0);
}
