//Vertex Shader

#version 330 core

in vec3 vPosition;

in vec3 normal;

out vec3 theColor;

uniform mat4 mvpMatrix;
uniform vec3 camPos;
uniform float rotation;

mat4 rotationX(in float angle){
    return mat4(1.0,    0,      0,      0,
                0,   cos(angle),  -sin(angle), 0,
                0,   sin(angle),  cos(angle),  0,
                0,      0,      0,      1);
}

void main(){
    vec3 lightColor = vec3(1.0, 1.0, 1.0); //white
    vec3 objectColor = vec3(0.08, 0.77, 0.91);
    vec3 lightPos = vec3(10, 10, -10);

    //ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    vec3 resultAmb = ambient * objectColor;

    //diffuse lighting
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - vPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 resultDiffuse = diffuse * objectColor;


    //specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(camPos - vPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;

    theColor = result;

    gl_Position = mvpMatrix * vec4(vPosition, 1.0);
}