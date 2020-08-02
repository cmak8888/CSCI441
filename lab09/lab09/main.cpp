/*
 *  CSCI 441, Computer Graphics, Fall 2017
 *
 *  Project: lab09
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with GLSL shaders.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2017
 *  Modified By: Calvin Mak
 *
 */

//*************************************************************************************

// LOOKHERE #2
#include <GL/glew.h>        	// include GLEW to get our OpenGL 3.0+ bindings
#include <GLFW/glfw3.h>			// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include <stdio.h>				// for printf functionality
#include <stdlib.h>				// for exit functionality

// note that all of these headers end in *3.hpp
// these class library files will only work with OpenGL 3.0+
#include <CSCI441/modelLoader3.hpp> // to load in OBJ models
#include <CSCI441/OpenGLUtils3.hpp> // to print info about OpenGL
#include <CSCI441/objects3.hpp>     // to render our 3D primitives

#include "include/Shader_Utils.h"   // our shader helper functions

//*************************************************************************************
//
// Global Parameters

int windowWidth, windowHeight;

bool controlDown = false;
bool leftMouseDown = false;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

glm::vec3 cameraAngles( 1.82f, 2.01f, 15.0f );
glm::vec3 eyePoint(   10.0f, 10.0f, 10.0f );
glm::vec3 lookAtPoint( 0.0f,  0.0f,  0.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );

int objectIndex = 2;

CSCI441::ModelLoader* model = NULL;

GLuint shaderProgramHandle = 0;
GLint mvp_uniform_location = -1, time_uniform_location = -1;
GLint vpos_attrib_location = -1;



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
void convertSphericalToCartesian() {
    eyePoint.x = cameraAngles.z * sinf( cameraAngles.x ) * sinf( cameraAngles.y );
    eyePoint.y = cameraAngles.z * -cosf( cameraAngles.y );
    eyePoint.z = cameraAngles.z * -cosf( cameraAngles.x ) * sinf( cameraAngles.y );
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
    if ((key == GLFW_KEY_ESCAPE || key == 'Q') && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if( action == GLFW_PRESS ) {
        switch( key ) {
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
                glfwSetWindowShouldClose( window, GLFW_TRUE );
                break;

            case GLFW_KEY_1:
            case GLFW_KEY_2:
            case GLFW_KEY_3:
            case GLFW_KEY_4:
            case GLFW_KEY_5:
            case GLFW_KEY_6:
            case GLFW_KEY_7:
                objectIndex = key - 49; // 49 is GLFW_KEY_1.  they go in sequence from there
                break;
        }
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
    controlDown = mods & GLFW_MOD_CONTROL;
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
                    convertSphericalToCartesian();

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

    convertSphericalToCartesian();
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
    GLFWwindow *window = glfwCreateWindow(640, 480, "Lab09: GLSL Shaders", NULL, NULL);
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
void setupShaders( const char *vertexShaderFilename, const char *fragmentShaderFilename ) {
    // TODO #7

    shaderProgramHandle = createShaderProgram(vertexShaderFilename, fragmentShaderFilename);

    // TODO #8A
    mvp_uniform_location = glGetUniformLocation( shaderProgramHandle, "mvpMatrix");


    // TODO #10A

    glUseProgram(shaderProgramHandle);
    time_uniform_location = glGetUniformLocation(shaderProgramHandle, "time");
    glUniform1f(shaderProgramHandle, time_uniform_location );

    // TODO #8B
    vpos_attrib_location = glGetAttribLocation(shaderProgramHandle, "vPosition");


}

// setupBuffers() //////////////////////////////////////////////////////////////
//
//      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {
    model = new CSCI441::ModelLoader();
    model->loadModelFile( "models/suzanne.obj" );
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
    // stores our model matrix
    glm::mat4 modelMtx = glm::mat4(1);

    // use our shader program
    // TODO #9A
    glUseProgram(shaderProgramHandle);

    // precompute our MVP CPU side so it only needs to be computed once
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;;
    glUniformMatrix4fv(mvp_uniform_location,1,GL_FALSE, &mvpMtx[0][0]);


    // send MVP to GPU
    // TODO #9B
    // send time to GPU
    // TODO #10B

    glUseProgram(shaderProgramHandle);
    glUniform1f(time_uniform_location, glfwGetTime());



    // draw all the cool stuff!
    switch( objectIndex ) {
        case 0: CSCI441::drawSolidTeapot( 2.0 );                            break;
        case 1: CSCI441::drawSolidCube( 4.0 );                              break;
        case 2: CSCI441::drawSolidSphere( 3.0, 16, 16 );                    break;
        case 3: CSCI441::drawSolidTorus( 1.0, 2.0, 16, 16 );                break;
        case 4: CSCI441::drawSolidCone( 2.0, 4.0, 16, 16 );                 break;
        case 5: CSCI441::drawSolidCylinder( 2.0, 2.0, 4.0, 16, 16 );        break;
        case 6: model->draw( vpos_attrib_location );                        break;
    }
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
    if( argc != 3 ) {
        // we need a vertex and fragment shader
        fprintf( stderr, "Usage: ./lab09 <shader.v.glsl> <shader.f.glsl>\n" );
        exit(EXIT_FAILURE);
    }

    // GLFW sets up our OpenGL context so must be done first
    GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
    setupOpenGL();										// initialize all of the OpenGL specific information
    // LOOKHERE #1A
    setupGLEW();											// initialize all of the GLEW specific information

    CSCI441::OpenGLUtils::printOpenGLInfo();

    setupShaders( argv[1], argv[2] ); // load our shader program into memory
    setupBuffers();										// load all our VAOs and VBOs into memory

    // needed to connect our 3D Object Library to our shader
    // LOOKHERE #3
    CSCI441::setVertexAttributeLocations( vpos_attrib_location );

    convertSphericalToCartesian();		// set up our camera position

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
        glm::mat4 viewMatrix = glm::lookAt( eyePoint,lookAtPoint, upVector );

        // draw everything to the window
        // pass our view and projection matrices as well as deltaTime between frames
        renderScene( viewMatrix, projectionMatrix );

        glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();				// check for any events and signal to redraw screen
    }

    glfwDestroyWindow( window );// clean up and close our window
    glfwTerminate();						// shut down GLFW to clean up our context

    return EXIT_SUCCESS;
}