/*
 *   Fragment Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

in vec3 texCoords;
in vec4 fragPos;
in vec3 camPos;

uniform samplerCube skybox;

out vec4 fragColorOut;

void main() {

	vec4 theColor = texture(skybox, texCoords);

	vec4 fogColor = vec4(.75, .75, 1, 1);
	float fogDist = 100;

	float distFromCamera = distance(vec3(fragPos.x, fragPos.y, fragPos.z), camPos);

	if(distFromCamera > fogDist) {
		fragColorOut = fogColor;
	} else {
		float fogColorMult = distFromCamera / fogDist;
		float currentColorMult = 1.0 - fogColorMult;
		fragColorOut = (currentColorMult * theColor) + (fogColorMult * fogColor);
	}
}
