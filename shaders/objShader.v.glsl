//Vertex Shader

#version 330 core

in vec3 vPosition;
in vec2 vTexCoord;
in vec3 normal;

out vec2 texCoords;

out vec3 theColor;

uniform mat4 mvpMatrix;
uniform vec3 camPos;
uniform float time;
uniform vec3 dir;

void main(){
    vec3 objectColor = vec3(0.95, 0.95, 0.95);
    vec3 lightPos = vec3(30*sin(.25*time), 30*sin(.25*time), 30*cos(.25*time));
    float t = lightPos.z + 30;
    float t2 = t/60;
    vec3 lightColor = vec3(t2,t2,t2); //white

    //ambient lighting
    float ambientStrength = 0.25;
    vec3 ambient = ambientStrength * lightColor;
    vec3 resultAmb = ambient * objectColor;

    //diffuse lighting
    float diffuseStrength = 0.4;
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - vPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (diff * diffuseStrength) * lightColor;
    vec3 resultDiffuse = diffuse * objectColor;


    //specular lighting
    float specularStrength = 0.775;
    vec3 viewDir = normalize(camPos - vPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;

    theColor = result;
    vec3 newVertex = vPosition;

    if(vPosition.x > 0.05 || vPosition.x < -0.05){
	newVertex.z = (vPosition.x*vPosition.x)*sin(25*abs(dir.y)*time)*3;
    }

    texCoords = vTexCoord;
    gl_Position = mvpMatrix * vec4(newVertex , 1.0);
}