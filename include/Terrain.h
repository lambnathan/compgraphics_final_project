//
// Created by drfresco on 12/8/2019.
//

#include <GL/glew.h>        	// include GLEW to get our OpenGL 3.0+ bindings
#include <GLFW/glfw3.h>			// include GLFW framework header

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include "CSCI441/objects3.hpp"

class Terrain {
private:
    float map[250][250];
    int skyboxSize, seed, divisions;
    static constexpr float MAX_DIFF = 15.0;
    static constexpr float MIN_HEIGHT = 15.0;
    GLuint texHandle;

public:
    Terrain(int skyboxSize, int divisions, int seed) {
        if(divisions > 250) divisions = 250;
        this->skyboxSize = skyboxSize;
        this->divisions = divisions;
        this->seed = seed;

        srand(seed);
        for(int x = 0; x < divisions; x++) {
            for(int y = 0; y < divisions; y++) {
                map[x][y] = (float)((rand()%100) / 100.0) * MAX_DIFF + MIN_HEIGHT;
                cout << map[x][y] << endl;
            }
        }
    }
    void setTexHandle(GLuint handle) {
        this->texHandle = handle;
    }
    void draw(glm::mat4 &mvpMtx, GLint mvp_uniform_location) {
        for(int x = 0; x < divisions; x++) {
            for(int y = 0; y < divisions; y++) {
                float cubeLength = (skyboxSize * 2) / divisions;
                glm::vec3 location =  glm::vec3(x * (cubeLength) - skyboxSize, -map[x][y], y * (cubeLength) - skyboxSize);
                glm::mat4 translate = glm::translate(glm::mat4(1), location);
                glm::mat4 scale = glm::scale(translate, glm::vec3(cubeLength, map[x][y], cubeLength));
                mvpMtx *= scale;
                glUniformMatrix4fv(mvp_uniform_location, 1, GL_FALSE, &mvpMtx[0][0]);
                CSCI441::drawSolidCube(1);
                mvpMtx *= glm::inverse(scale);
            }
        }
    }
};
