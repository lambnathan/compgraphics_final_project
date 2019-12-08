#ifndef _SHADER_UTILS_H_
#define _SHADER_UTILS_H_

#include <GL/glew.h>

#include <stdio.h>

#include <fstream>
#include <string>
using namespace std;

// readTextFromFile() //////////////////////////////////////////////////////////////
//
//  Reads in a text file as a single string. Used to aid in shader loading.
//
////////////////////////////////////////////////////////////////////////////////
void readTextFromFile( const char* filename, char* &output ){
    string buf = string("");
    string line;

    ifstream in( filename );
    while( getline(in, line) ) {
        buf += line + "\n";
    }
    output = new char[buf.length()+1];
    strncpy(output, buf.c_str(), buf.length());
    output[buf.length()] = '\0';

    in.close();
}

// printLog() //////////////////////////////////////////////////////////////////
//
//  Check for errors from compiling or linking a vertex/fragment/shader program
//	Prints to terminal
//
////////////////////////////////////////////////////////////////////////////////
void printLog( GLuint handle ) {
    int infologLength = 0;
    int maxLength;

    // check if the handle is to a vertex/fragment shader
    if( glIsShader( handle ) ) {
        glGetShaderiv(  handle, GL_INFO_LOG_LENGTH, &maxLength );
    }
    // check if the handle is to a shader program
    else {
        glGetProgramiv( handle, GL_INFO_LOG_LENGTH, &maxLength );
    }

    // create a buffer of designated length
    char infoLog[maxLength];

    bool isShader;
    if( glIsShader( handle ) ) {
        // get the info log for the vertex/fragment shader
        glGetShaderInfoLog(  handle, maxLength, &infologLength, infoLog );

        isShader = true;
    } else {
        // get the info log for the shader program
        glGetProgramInfoLog( handle, maxLength, &infologLength, infoLog );

        isShader = false;
    }

    // if the length of the log is greater than 0
    if( infologLength > 0 ) {
        // print info to terminal
        printf( "[INFO]: %s Handle %d: %s\n", (isShader ? "Shader" : "Program"), handle, infoLog );
    }
}

// compileShader() ///////////////////////////////////////////////////////////////
//
//  Compile a given shader program
//
////////////////////////////////////////////////////////////////////////////////
GLuint compileShader( const char* filename, GLenum shaderType ) {
    char *shaderString;
    // read in each text file and store the contents in a string
    readTextFromFile( filename, shaderString );
    // create a handle to our shader
    GLuint shaderHandle = 0;
    // TODO #1
	shaderHandle = glCreateShader(shaderType);
	if(shaderHandle < 0){
		printf("EERRRROORRRRRR\n");
	}

    // send the contents of each program to the GPU
    // TODO #2
	glShaderSource(shaderHandle, 1, (const char** )&shaderString, NULL);

    // we are good programmers so free up the memory used by each buffer
    delete [] shaderString;

    // compile each shader on the GPU
    // TODO #3
	glCompileShader(shaderHandle);
	
	// print out the shader log
    printLog( shaderHandle );

    return shaderHandle;
}

// createShaderProgram() ///////////////////////////////////////////////////////
//
//  Compile and Register our Vertex and Fragment Shaders
//
////////////////////////////////////////////////////////////////////////////////
GLuint createShaderProgram( const char *vertexShaderFilename, const char *fragmentShaderFilename ) {
    // compile each one of our shaders
    GLuint vertexShaderHandle   = compileShader( vertexShaderFilename,   GL_VERTEX_SHADER   );
    GLuint fragmentShaderHandle = compileShader( fragmentShaderFilename, GL_FRAGMENT_SHADER );
    // get a handle to a shader program
    GLuint shaderProgramHandle = 0;
    // TODO #4
	shaderProgramHandle = glCreateProgram();

    // attach the vertex, geometry, and fragment shaders to the shader program
    // TODO #5A
	glAttachShader(shaderProgramHandle, vertexShaderHandle);
    printLog( vertexShaderHandle   );
    // TODO #5B
	glAttachShader(shaderProgramHandle, fragmentShaderHandle);
    printLog( fragmentShaderHandle );

    // link all the programs together on the GPU
    // TODO #6A
	glLinkProgram(shaderProgramHandle);
	
	// print out the program log
    printLog( shaderProgramHandle );
	
	// detach the shaders
	// TODO #6B
	glDetachShader(shaderProgramHandle, vertexShaderHandle);
	glDetachShader(shaderProgramHandle, fragmentShaderHandle);
	
	// delete the shaders
	// TODO #6C
	glDeleteShader(vertexShaderHandle);
	glDeleteShader(fragmentShaderHandle);

    // return handle
    return shaderProgramHandle;
}

#endif
