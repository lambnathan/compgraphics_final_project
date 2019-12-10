/*
 *  CSCI 441, Computer Graphics, Fall 2017
 *
 *  Project: a5
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with GLSL shaders.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2017
 *
 *  Nathan Lambert
 *
 */

//*************************************************************************************

// LOOKHERE #2
#include <GL/glew.h>        	// include GLEW to get our OpenGL 3.0+ bindings
#include <GLFW/glfw3.h>			// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>				// for printf functionality
#include <stdlib.h>				// for exit functionality

// note that all of these headers end in *3.hpp
// these class library files will only work with OpenGL 3.0+
#include <CSCI441/modelLoader3.hpp> // to load in OBJ models
#include <CSCI441/OpenGLUtils3.hpp> // to print info about OpenGL
#include <CSCI441/objects3.hpp>     // to render our 3D primitives
#include <CSCI441/TextureUtils.hpp>

#include "include/Shader_Utils.h"   // our shader helper functions
#include "include/Terrain.h"

//image loader, used to load in pictures for skybox
//#define STB_IMAGE_IMPLEMENTATION
//#include "include/stb_image.h"

#include <iostream>

//*************************************************************************************
//
// Global Parameters

int windowWidth, windowHeight;

bool controlDown = false;
bool leftMouseDown = false;
bool camSwap = true;
bool keysDown[256] = { 0 };           // status of our keys

glm::vec2 mousePosition( -9999.0f, -9999.0f );

glm::vec3 cameraAngles( 0.0f, 2.01f, 15.0f );
glm::vec3 eyePoint(   0.0f, 10.0f, 10.0f );
glm::vec3 lookAtPoint( 0.0f,  0.0f,  0.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );
glm::vec3 objectRotation(4.7124f, 0.0f, 0.0f);
glm::vec3 objectDirection(0.0f, 0.0f, 0.5f);
glm::vec3 objectPosistion(0.0f, 0.0f, 0.0f);
float pitch = 0, yaw = 0, roll = 0;

GLfloat propAngle = 0.0f;

CSCI441::ModelLoader* model = NULL;
CSCI441::ModelLoader* propModel = NULL;
GLuint modelTextureHandle;

GLuint skyboxShaderHandle = 0;
GLuint objectShaderHandle = 0;
GLuint propShaderHandle = 0;

GLint mvp_uniform_location_box = -1;
GLint eye_uniform_location_box = -1;
GLint vpos_attrib_location_box = -1;
GLint time_uniform_location_box = -1;

//atribute locations for uav
GLint mvp_uniform_location_obj = -1;
GLint vpos_attrib_location_obj = -1;
GLint norm_attrib_location = -1;
GLint cam_pos_location = -1;
GLint time_uniform_location = -1;
GLint vdir_uniform_location_obj = -1;
GLint vTex_attrib_location = -1;

//attribute locations for propeller
GLint mvp_uniform_location_prop = -1;
GLint vpos_attrib_location_prop = -1;
GLint norm_attrib_location_prop = -1;
GLint cam_pos_location_prop = -1;
GLint rotation_location_prop = -1;

//vbo and vao for skybox
GLuint skyboxVBO;
GLuint skyboxVAO;
unsigned int skyboxID; //holds texture
unsigned int sandboxID;

Terrain ground(50, 10, 0);

//******************************************************************************
//
// Helper Functions

// convertSphericalToCartesian() ///////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraAngles is updated.
//
////////////////////////////////////////////////////////////////////////////////

/*
 * rotate about heading by roll
 * rotate about heading x normal by pitch
 * dir(0) = +z
 *
 * y direction = sin(pitch)
 * x direction = sin(yaw)
 * z direction = cos(yaw
 */

glm::vec3 getHeading(float pitch, float yaw, float roll) {

}
glm::vec3 getNormal(float pitch, float yaw, float roll) {

}
glm::vec3 toEuler(glm::vec3 cartesian) {
    float lastPitch = atan(cartesian.y / cartesian.z) + (cartesian.z < 0 ? M_PI : 0);
    float lastYaw = atan(cartesian.x / cartesian.z) + (cartesian.z < 0 ? M_PI : 0);
    float lastRoll = atan(cartesian.y / cartesian.x) + (cartesian.x < 0 ? M_PI : 0);

    return glm::vec3(lastPitch, lastYaw, lastRoll);
}
glm::vec3 toCartesian(glm::vec3 euler) {
    glm::mat4 conversion(1);
    conversion = glm::rotate(conversion, euler.x, glm::vec3(1, 0, 0)); //rotate by pitch about x axis
    conversion = glm::rotate(conversion, euler.y, glm::vec3(0, 1, 0)); //rotate by yaw about y
    conversion = glm::rotate(conversion, euler.z, glm::vec3(0, 0, 1)); //rotate by roll about z

    glm::vec4 coords = glm::vec4(euler, 1);

    coords = conversion * coords;
    return glm::vec3(coords.z, coords.y, coords.x);
}

void recomputeCameraOrientation() {
    eyePoint.x = 10 * sinf( cameraAngles.x ) * sinf( cameraAngles.y ) + objectPosistion.x;
    eyePoint.y = 10 * -cosf( cameraAngles.y ) + objectPosistion.y;
    eyePoint.z = 10 * -cosf( cameraAngles.x ) * sinf( cameraAngles.y ) + objectPosistion.z;
}


void recomputeOrientation() {
    objectDirection.x = sinf(objectRotation.y) * sinf(objectRotation.x);
    objectDirection.z = cosf(objectRotation.y) * sinf(objectRotation.x);
    objectDirection.y = cosf(objectRotation.x);

    //and normalize this directional vector!
    objectDirection = glm::normalize(objectDirection);
}

//******************************************************************************
//
// Event Callbacks

// error_callback() ////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
////////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char* description) {
    fprintf(stderr, "[ERROR]: %s\n", description);
}

// key_callback() //////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's keypress callback.
//	Responds to key presses and key releases
//
////////////////////////////////////////////////////////////////////////////////
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_1:
                //lightOne = !(lightOne);
                camSwap = !(camSwap);
                break;

            case GLFW_KEY_2:
                //lightTwo = !(lightTwo);

            default:
                keysDown[key] = true;
        }
    }
    else if (action == GLFW_RELEASE) {
        keysDown[key] = false;
    }
}

// mouse_button_callback() /////////////////////////////////////////////////////
//
//		We will register this function as GLFW's mouse button callback.
//	Responds to mouse button presses and mouse button releases.  Keeps track if
//	the control key was pressed when a left mouse click occurs to allow
//	zooming of our arcball camera.
//
////////////////////////////////////////////////////////////////////////////////
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS ) {
        leftMouseDown = true;
    } else {
        leftMouseDown = false;
        mousePosition.x = -9999.0f;
        mousePosition.y = -9999.0f;
    }
    //controlDown = mods & GLFW_MOD_CONTROL;
}

void animate() {
    recomputeCameraOrientation();

    recomputeOrientation();
    //because the direction vector is unit length, and we probably don't want
    //to move one full unit every time a button is pressed, just create a constant
    //to keep track of how far we want to move at each step. you could make
    //this change w.r.t. the amount of time the button's held down for
    //simple scale-sensitive movement!
    const float movementConstant = 0.1f;

    if (keysDown[GLFW_KEY_SPACE]) {
	    objectPosistion -= objectDirection * movementConstant;
//        objectPosistion -= toCartesian(glm::vec3(pitch, yaw, roll)) * movementConstant;
        //just move FORWARDS along the direction.
        //camPos.x += camDir.x * movementConstant;
        //camPos.y += camDir.y * movementConstant;
        //camPos.z += camDir.z * movementConstant;

        //propAngle += M_PI / 16.0f;
        //if (propAngle > 2 * M_PI) propAngle -= 2 * M_PI;

    }
    if (keysDown[GLFW_KEY_S]) {
        pitch -= 0.01;

        objectRotation.x -= 0.01;
        recomputeOrientation();
    }

    if (keysDown[GLFW_KEY_W]) {
        pitch += 0.01;

        objectRotation.x += 0.01;
        recomputeOrientation();
    }

    if (keysDown[GLFW_KEY_D]) {
        roll -= 0.01;

        // turn right
        objectRotation.y -= 0.01;
        //cameraAngles.z += 0.01;
    }

    if (keysDown[GLFW_KEY_A]) {
        roll += 0.01;

        // turn left
        objectRotation.y += 0.01;
        //cameraAngles.z -= 0.01;
    }

}

// cursor_callback() ///////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's cursor movement callback.
//	Responds to mouse movement.  When active motion is used with the left
//	mouse button an arcball camera model is followed.
//
////////////////////////////////////////////////////////////////////////////////
static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
    // make sure movement is in bounds of the window
    // glfw captures mouse movement on entire screen
    if( xpos > 0 && xpos < windowWidth ) {
        if( ypos > 0 && ypos < windowHeight ) {
            // active motion
            if( leftMouseDown ) {
                if( (mousePosition.x - -9999.0f) < 0.001f ) {
                    mousePosition.x = xpos;
                    mousePosition.y = ypos;
                } else {
                    if( !controlDown ) {
                        cameraAngles.x += (xpos - mousePosition.x)*0.005f;
                        cameraAngles.y += (ypos - mousePosition.y)*0.005f;

                        if( cameraAngles.y < 0 ) cameraAngles.y = 0.0f + 0.001f;
                        if( cameraAngles.y >= M_PI ) cameraAngles.y = M_PI - 0.001f;
                    } else {
                        double totChgSq = (xpos - mousePosition.x) + (ypos - mousePosition.y);
                        cameraAngles.z += totChgSq*0.01f;

                        if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
                        if( cameraAngles.z >= 50.0f ) cameraAngles.z = 50.0f;
                    }
                    //convertSphericalToCartesian();

                    mousePosition.x = xpos;
                    mousePosition.y = ypos;
                }
            }
                // passive motion
            else {

            }
        }
    }
}

// scroll_callback() ///////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's scroll wheel callback.
//	Responds to movement of the scroll where.  Allows zooming of the arcball
//	camera.
//
////////////////////////////////////////////////////////////////////////////////
static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset ) {
    double totChgSq = yOffset;
    cameraAngles.z += totChgSq*0.01f;

    if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
    if( cameraAngles.z >= 50.0f ) cameraAngles.z = 50.0f;

    //convertSphericalToCartesian();
}

//******************************************************************************
//
// Setup Functions

// setupGLFW() /////////////////////////////////////////////////////////////////
//
//		Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
////////////////////////////////////////////////////////////////////////////////
GLFWwindow* setupGLFW() {
    // set what function to use when registering errors
    // this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
    // all other GLFW calls must be performed after GLFW has been initialized
    glfwSetErrorCallback(error_callback);

    // initialize GLFW
    if (!glfwInit()) {
        fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
        exit(EXIT_FAILURE);
    } else {
        fprintf( stdout, "[INFO]: GLFW initialized\n" );
    }

    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );						// request forward compatible OpenGL context
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );	// request OpenGL Core Profile context
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );		// request OpenGL 3.x context
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );		// request OpenGL 3.3 context

    // create a window for a given size, with a given title
    GLFWwindow *window = glfwCreateWindow(640, 480, "The Grand (Re)Opening!", NULL, NULL);
    if( !window ) {						// if the window could not be created, NULL is returned
        fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    } else {
        fprintf( stdout, "[INFO]: GLFW Window created\n" );
    }

    glfwMakeContextCurrent(	window );	// make the created window the current window
    glfwSwapInterval( 1 );				    // update our screen after at least 1 screen refresh

    glfwSetKeyCallback( 			  window, key_callback				  );	// set our keyboard callback function
    glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function
    glfwSetCursorPosCallback(	  window, cursor_callback  			);	// set our cursor position callback function
    glfwSetScrollCallback(			window, scroll_callback			  );	// set our scroll wheel callback function

    return window;										// return the window that was created
}

// setupOpenGL() ///////////////////////////////////////////////////////////////
//
//      Used to setup everything OpenGL related.
//
////////////////////////////////////////////////////////////////////////////////
void setupOpenGL() {
    glEnable( GL_DEPTH_TEST );					// enable depth testing
    glDepthFunc( GL_LESS );							// use less than depth test

    glEnable(GL_BLEND);									// enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// use one minus blending equation

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// clear the frame buffer to black
}

// setupGLEW() /////////////////////////////////////////////////////////////////
//
//      Used to initialize GLEW
//
////////////////////////////////////////////////////////////////////////////////
void setupGLEW() {
    // LOOKHERE #1B
    glewExperimental = GL_TRUE;
    GLenum glewResult = glewInit();

    // check for an error
    if( glewResult != GLEW_OK ) {
        printf( "[ERROR]: Error initalizing GLEW\n");
        // Problem: glewInit failed, something is seriously wrong.
        fprintf( stderr, "[ERROR]: %s\n", glewGetErrorString(glewResult) );
        exit(EXIT_FAILURE);
    } else {
        fprintf( stdout, "[INFO]: GLEW initialized\n" );
        fprintf( stdout, "[INFO]: Status: Using GLEW %s\n", glewGetString(GLEW_VERSION) );
    }

    if( !glewIsSupported( "GL_VERSION_2_0" ) ) {
        printf( "[ERROR]: OpenGL not version 2.0+.  GLSL not supported\n" );
        exit(EXIT_FAILURE);
    }
}

// setupShaders() //////////////////////////////////////////////////////////////
//
//      Create our shaders.  Send GLSL code to GPU to be compiled.  Also get
//  handles to our uniform and attribute locations.
//
////////////////////////////////////////////////////////////////////////////////
void setupShaders() {
    skyboxShaderHandle = createShaderProgram("shaders/customShader.v.glsl", "shaders/customShader.f.glsl");
    mvp_uniform_location_box = glGetUniformLocation(skyboxShaderHandle, "mvpMatrix");
    if(mvp_uniform_location_box < 0){
        cerr << "Error getting mvp uniform location for skybox" << endl;
        exit(-1);
    }
    eye_uniform_location_box = glGetUniformLocation(skyboxShaderHandle, "eyePos");
    if(eye_uniform_location_box < 0){
        cerr << "Error getting eyepos uniform location for skybox" << endl;
        exit(-1);
    }
    vpos_attrib_location_box = glGetAttribLocation(skyboxShaderHandle, "vPosition");
    if(vpos_attrib_location_box < 0){
        cerr << "Error getting vPosition for skybox" << endl;
        exit(-1);
    }
    time_uniform_location_box = glGetUniformLocation(skyboxShaderHandle, "time");
    if(time_uniform_location_box < 0){
        cerr << "Error getting time for skybox" << endl;
        exit(-1);
    }


    //for our loaded in object
    objectShaderHandle = createShaderProgram("shaders/objShader.v.glsl", "shaders/objShader.f.glsl");
    mvp_uniform_location_obj = glGetUniformLocation(objectShaderHandle, "mvpMatrix");
    if(mvp_uniform_location_obj < 0){
        cerr << "Error getting mvp uniform location for obj" << endl;
        exit(-1);
    }
    vpos_attrib_location_obj = glGetAttribLocation(objectShaderHandle, "vPosition");
    if(vpos_attrib_location_obj < 0){
        cerr << "Error getting vPosition for obj" << endl;
        exit(-1);
    }
    time_uniform_location = glGetUniformLocation(objectShaderHandle, "time");
    if (time_uniform_location != -1) {
        std::cout << "time loc Created" << std::endl;
    }
    else {
        std::cout << "time loc Not Created" << std::endl;
    }
    norm_attrib_location = glGetAttribLocation(objectShaderHandle, "normal");
    if(norm_attrib_location < 0){
        cerr << "Error getting object normal location" << endl;
        exit(-1);
    }
    cam_pos_location = glGetUniformLocation(objectShaderHandle, "camPos");
    if(cam_pos_location < 0){
        cerr << "Error getting cam pos location" << endl;
        exit(-1);
    }
    vdir_uniform_location_obj = glGetUniformLocation(objectShaderHandle, "dir");
    if(vdir_uniform_location_obj < 0){
        cerr << "Error getting object normal location" << endl;
        exit(-1);
    }
    vTex_attrib_location = glGetAttribLocation(objectShaderHandle, "vTexCoord");
    if(vTex_attrib_location < 0){
        cerr << "Error getting object tex location" << endl;
        exit(-1);
    }

    //set up stuff for the propeller shader
    propShaderHandle = createShaderProgram("shaders/propShader.v.glsl", "shaders/propShader.f.glsl");
    mvp_uniform_location_prop = glGetUniformLocation(propShaderHandle, "mvpMatrix");
    vpos_attrib_location_prop = glGetAttribLocation(propShaderHandle, "vPosition");
    norm_attrib_location_prop = glGetAttribLocation(propShaderHandle, "normal");
    cam_pos_location_prop = glGetUniformLocation(propShaderHandle, "camPos");
    rotation_location_prop = glGetUniformLocation(propShaderHandle, "rotation");
}

// setupBuffers() //////////////////////////////////////////////////////////////
//
//      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers(string obj) {
    float cube[] = {
            -50.0f,  50.0f, -50.0f,
            -50.0f, -50.0f, -50.0f,
            50.0f, -50.0f, -50.0f,
            50.0f, -50.0f, -50.0f,
            50.0f,  50.0f, -50.0f,
            -50.0f,  50.0f, -50.0f,

            -50.0f, -50.0f,  50.0f,
            -50.0f, -50.0f, -50.0f,
            -50.0f,  50.0f, -50.0f,
            -50.0f,  50.0f, -50.0f,
            -50.0f,  50.0f,  50.0f,
            -50.0f, -50.0f,  50.0f,

            50.0f, -50.0f, -50.0f,
            50.0f, -50.0f,  50.0f,
            50.0f,  50.0f,  50.0f,
            50.0f,  50.0f,  50.0f,
            50.0f,  50.0f, -50.0f,
            50.0f, -50.0f, -50.0f,

            -50.0f, -50.0f,  50.0f,
            -50.0f,  50.0f,  50.0f,
            50.0f,  50.0f,  50.0f,
            50.0f,  50.0f,  50.0f,
            50.0f, -50.0f,  50.0f,
            -50.0f, -50.0f,  50.0f,

            -50.0f,  50.0f, -50.0f,
            50.0f,  50.0f, -50.0f,
            50.0f,  50.0f,  50.0f,
            50.0f,  50.0f,  50.0f,
            -50.0f,  50.0f,  50.0f,
            -50.0f,  50.0f, -50.0f,

            -50.0f, -50.0f, -50.0f,
            -50.0f, -50.0f,  50.0f,
            50.0f, -50.0f, -50.0f,
            50.0f, -50.0f, -50.0f,
            -50.0f, -50.0f,  50.0f,
            50.0f, -50.0f,  50.0f
    };
    glGenBuffers(1, &skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), &cube, GL_STATIC_DRAW);

    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    //load the specified model
    model = new CSCI441::ModelLoader();
    model->loadModelFile( obj.c_str() );

    propModel = new CSCI441::ModelLoader();
    propModel->loadModelFile("models/prop2.obj");
}

//takes in a vector of strings of the files for the skybox and loads them
unsigned int loadCubeMap(vector<string> faces){
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for(unsigned int i = 0; i < faces.size(); i++){
        int width, height, n;
        unsigned char* imageData = SOIL_load_image(faces[i].c_str(), &width, &height, &n, 0);
        if (!imageData) {
            fprintf(stderr, "ERROR: could not load %s\n", faces[i].c_str());
            exit(1);
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
        SOIL_free_image_data(imageData);
    }
    //format the texture
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


//load all of our textures into memory
void setupTextures(){
    //get skybox texture by loading all the images
    vector<string> sand = {"textures/sand.v4.jpg", "textures/sand.v4.jpg", "textures/sand.v4.jpg",
                           "textures/sand.v4.jpg", "textures/sand.v4.jpg", "textures/sand.v4.jpg"};
    vector<string> faces = {"textures/skybox/hw_sahara/posx.jpg", "textures/skybox/hw_sahara/negx.jpg",
                            "textures/skybox/hw_sahara/posy.jpg", "textures/skybox/hw_sahara/negy.jpg",
                            "textures/skybox/hw_sahara/posz.jpg", "textures/skybox/hw_sahara/negz.jpg"};
    skyboxID = loadCubeMap(faces);
    sandboxID = loadCubeMap(sand);

    modelTextureHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/metal.jpg");
}



//******************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// renderScene() ///////////////////////////////////////////////////////////////
//
//		This method will contain all of the objects to be drawn.
//
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMtx, glm::mat4 projMtx ) {

    /////////////////////////////
    /// MODEL STUFF
    ////////////////////////////

    //draw the propeller DO NOT CHANGE ANY OF THESE TRANSFORMATIONS
    glm::mat4 modelMtx = glm::mat4(1.0f);
    modelMtx = glm::translate(modelMtx, objectPosistion);
    //modelMtx = glm::rotate(modelMtx, glm::radians(270.0f), glm::vec3(1.0, 0.0, 0.0));
    modelMtx = glm::rotate(modelMtx, objectRotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); //rotate the object
    modelMtx = glm::rotate(modelMtx, objectRotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); //rotate the object
    modelMtx = glm::rotate(modelMtx, objectRotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate the object
    modelMtx = glm::translate(modelMtx, glm::vec3(-0.06f, -1.2f, -0.35f)); //z changes y, y changes z?

    modelMtx = glm::scale(modelMtx, glm::vec3(0.1, 0.1, 0.1));

    modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, 0.0f, 2.9f));
    modelMtx = glm::rotate(modelMtx, propAngle, glm::vec3(0.0f, 1.0f, 0.0f)); //rotate the object
    modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, 0.0f, -2.9f));

    propAngle += M_PI / 16.0f;
    if( propAngle > 2 * M_PI ) propAngle -= 2 * M_PI;

//    propAngle += 0.1f;
//    if(propAngle == 100000) propAngle = 0;
    // precompute our MVP CPU side so it only needs to be computed once
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    //draw the model that we loaded in
    glUseProgram(propShaderHandle);
    // send MVP to GPU
    glUniformMatrix4fv(mvp_uniform_location_prop, 1, GL_FALSE, &mvpMtx[0][0]);
    //send camera pos to GPU
    glUniform3fv(cam_pos_location_prop, 1, &eyePoint[0]);
    glUniform1f(rotation_location_prop, propAngle); //send in prop angle to shader
    propModel->draw(vpos_attrib_location_prop, norm_attrib_location_prop);


    //draw the uav
    // stores our model matrix

    modelMtx = glm::mat4(1.0f);
    modelMtx = glm::translate(modelMtx, objectPosistion);

    modelMtx = glm::scale(modelMtx, glm::vec3(15.0f, 15.0f, 15.0f)); //scale our object
    modelMtx = glm::rotate(modelMtx, objectRotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); //rotate the object
    modelMtx = glm::rotate(modelMtx,objectRotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); //rotate the object
    modelMtx = glm::rotate(modelMtx, objectRotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate the object

    // precompute our MVP CPU side so it only needs to be computed once
    mvpMtx = projMtx * viewMtx * modelMtx;
    //draw the model that we loaded in
    glUseProgram(objectShaderHandle);
    // send MVP to GPU


    glUniformMatrix4fv(mvp_uniform_location_obj, 1, GL_FALSE, &mvpMtx[0][0]);
    double time = glfwGetTime();
    glUniform1f(time_uniform_location, time);
    //send camera pos to GPU
    glUniform3fv(cam_pos_location, 1, &objectPosistion[0]);
    glUniform3fv(vdir_uniform_location_obj, 1, &objectDirection[0]);

    glBindTexture( GL_TEXTURE_2D, modelTextureHandle );

    model->draw(vpos_attrib_location_obj, norm_attrib_location, vTex_attrib_location);

    //reset model and mvp mtx to draw skybox
    modelMtx = glm::mat4(1.0f);
    mvpMtx = projMtx * viewMtx * modelMtx;


    //////////////////////////////////
    //SKYBOX STUFF
    ////////////////////////////////
    // use our skybox shader program
    glUseProgram(skyboxShaderHandle);
    glUniformMatrix4fv(mvp_uniform_location_box, 1, GL_FALSE, &mvpMtx[0][0]);
    glUniform3fv(eye_uniform_location_box, 1, &eyePoint[0]);
    glUniform1f(time_uniform_location_box, time);

    glDepthFunc(GL_LEQUAL);
    //glDepthMask(GL_FALSE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxID);
    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindTexture(GL_TEXTURE_CUBE_MAP, sandboxID);
    ground.draw(mvpMtx, mvp_uniform_location_box);

    //glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

///*****************************************************************************
//
// Our main function

// main() ///////////////////////////////////////////////////////////////
//
//		Really you should know what this is by now.
//
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] ) {
    // ensure proper number of arguments provided at runtime
    if( argc != 2 ) {
        // we need an object file
        fprintf( stderr, "Usage: ./a5 <wavefrontObjectFile.obj>\n" );
        exit(EXIT_FAILURE);
    }

    // GLFW sets up our OpenGL context so must be done first
    GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
    setupOpenGL();										// initialize all of the OpenGL specific information

    setupGLEW();											// initialize all of the GLEW specific information
    recomputeOrientation();

    CSCI441::OpenGLUtils::printOpenGLInfo();

    setupTextures();  //load and set up textures for skybox

    setupShaders(); // load our shader program into memory
    // load all our VAOs and VBOs into memory
    string in = argv[1];
    if(in == "submarine.obj"){
        setupBuffers("models/submarine.obj");
    }
    else if(in == "suzanne.obj"){
        setupBuffers("models/suzanne.obj");
    }
    else if(in == "chair.obj"){
        setupBuffers("models/chair.obj");
    }
    else if(in == "Plane.obj"){
        setupBuffers("models/Plane.obj");
    }
    else if(in == "cargo.obj"){
        setupBuffers("models/cargo.obj");
    }
    else if(in == "MQ-9.obj"){
        setupBuffers("models/MQ-9.obj");
    }
    else if(in == "uh60.obj"){
        setupBuffers("models/uh60.obj");
    }
    else{
        fprintf(stderr, "Unable to find that file\n");
        exit(-1);
    }

    // needed to connect our 3D Object Library to our shader
    CSCI441::setVertexAttributeLocations( vpos_attrib_location_obj, norm_attrib_location );
    CSCI441::setVertexAttributeLocations(vpos_attrib_location_prop, norm_attrib_location_prop);

    //convertSphericalToCartesian();		// set up our camera position

    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
        glDrawBuffer( GL_BACK );				// work with our back frame buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

        // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
        // when using a Retina display the actual window can be larger than the requested window.  Therefore
        // query what the actual size of the window we are rendering to is.
        glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

        // update the viewport - tell OpenGL we want to render to the whole window
        glViewport( 0, 0, windowWidth, windowHeight );

        // set the projection matrix based on the window size
        // use a perspective projection that ranges
        // with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
        glm::mat4 projectionMatrix = glm::perspective( 45.0f, windowWidth / (float) windowHeight, 0.001f, 100.0f );

        // set up our look at matrix to position our camera
//        std:cout << objectDirection.x << " " << objectDirection.y << " " << objectDirection.z << std::endl;
//        std:cout << objectPosistion.x << " " << objectPosistion.y << " " << objectPosistion.z << std::endl;
        glm::mat4 viewMatrix;
        if(camSwap) {
            upVector = glm::vec3(    0.0f,  1.0f,  0.0f );
            lookAtPoint = glm::vec3(objectPosistion.x, objectPosistion.y, objectPosistion.z);
        } else {
//            glm::vec3 eyeOffset = glm::cross(glm::vec3(0, 0, sin(objectRotation.x)), glm::vec3(sin(objectRotation.z), 0, 0));
//            if(glm::length(eyeOffset) != 0) eyeOffset = glm::normalize(eyeOffset);
//            cout << glm::length(eyeOffset) << endl;

            glm::mat4 modelMtx = glm::mat4(1.0f);
//            modelMtx = glm::translate(modelMtx, objectPosistion);

//            modelMtx = glm::scale(modelMtx, glm::vec3(15.0f, 15.0f, 15.0f)); //scale our object
            modelMtx = glm::rotate(modelMtx, objectRotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); //rotate the object
            modelMtx = glm::rotate(modelMtx,objectRotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); //rotate the object
            modelMtx = glm::rotate(modelMtx, objectRotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate the object

            glm::vec4 eyeOffset4(0, 3, 3, 1);
            upVector = glm::vec3(0, sin(objectRotation.x) > 0 ? -1 : 1, 0);
            cout<<objectRotation.x<<endl;
            eyeOffset4 = eyeOffset4 * modelMtx;
            glm::vec3 eyeOffset(eyeOffset4.x, eyeOffset4.y, eyeOffset4.z);

            eyePoint = glm::vec3(objectPosistion.x, objectPosistion.y+1, objectPosistion.z) + eyeOffset;
            lookAtPoint = glm::vec3(objectPosistion.x, objectPosistion.y- objectDirection.y+1, objectPosistion.z- objectDirection.z) + eyeOffset;
        }
        viewMatrix = glm::lookAt(eyePoint, lookAtPoint, upVector);

        // draw everything to the window
        // pass our view and projection matrices as well as deltaTime between frames
        renderScene( viewMatrix, projectionMatrix );

        glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();				// check for any events and signal to redraw screen
        animate();
    }

    glfwDestroyWindow( window );// clean up and close our window
    glfwTerminate();						// shut down GLFW to clean up our context

    return EXIT_SUCCESS;
}
