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
uniform float time;

out vec4 fragColorOut;

void main() {

	vec4 theColor = texture(skybox, texCoords);
	vec4 fogColor = vec4(.75*(cos(.25*time)+0.05), .75*(cos(.25*time)+0.05), 1*(cos(.25*time)+0.05), 1);
	float fogDist = 100;

	float distFromCamera = distance(fragPos.xyz, camPos);

	if(distFromCamera > fogDist) {
		fragColorOut = fogColor;
	} else {
		float fogColorMult = distFromCamera / fogDist;
		float currentColorMult = 1.0 - fogColorMult;
		fragColorOut = (currentColorMult * theColor) + (fogColorMult * fogColor);
	}
	if(distFromCamera < 20) fragColorOut += vec4(.1);
}
