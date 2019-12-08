/*
 *   Fragment Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

in vec3 texCoords;

uniform samplerCube skybox;

out vec4 fragColorOut;

void main() {

	fragColorOut = texture(skybox, texCoords);
}
