/*
 *  CSCI 441, Computer Graphics, Fall 2019
 *
 *  Project: lab00a
 *  File: main.cpp
 *
 *	Author: Dr. Jeffrey Paone - Fall 2019
 *
 *  Description:
 *      Contains the code for a simple 2D OpenGL / GLFW example.
 *
 */

// include the OpenGL library header
#ifdef __APPLE__				// if compiling on Mac OS
    #include <OpenGL/gl.h>
#else							// if compiling on Linux or Windows OS
    #include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>			// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>				// for printf functionality

//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 512, windowHeight = 512;

// needed for OS X Movaje nuance - described below in more detail
GLboolean mackHack = false;

//*************************************************************************************
//
// Event Callbacks

//
//	void error_callback( int error, const char* description )
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
static void error_callback( int error, const char* description ) {
    fprintf( stderr, "[ERROR]: %s\n", description );
}

//*************************************************************************************
//
// Setup Functions

//
//  void setupGLFW()
//
//      Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
GLFWwindow* setupGLFW() {
    // set what function to use when registering errors
    // this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
    // all other GLFW calls must be performed after GLFW has been initialized
    glfwSetErrorCallback( error_callback );

    // initialize GLFW
    if( !glfwInit() ) {
        fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
        exit( EXIT_FAILURE );
    } else {
        fprintf( stdout, "[INFO]: GLFW initialized\n" );
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );	// request OpenGL v2.X
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );	// request OpenGL v2.1
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		// do not allow our window to be able to be resized

    // create a window for a given size, with a given title
    GLFWwindow *window = glfwCreateWindow( windowWidth, windowHeight, "Lab00A", NULL, NULL );
    if( !window ) {						// if the window could not be created, NULL is returned
        glfwTerminate();
        exit( EXIT_FAILURE );
    } else {
        fprintf( stdout, "[INFO]: GLFW Window created\n" );
    }

    glfwMakeContextCurrent(window);		// make the created window the current window
    glfwSwapInterval(1);				// update our window after at least 1 screen refresh

    return window;						// return the window that was created
}

//
//  void setupOpenGL()
//
//      Used to setup everything OpenGL related.  For now, the only setting
//	we need is what color to make the background of our window when we clear
//	the window.  In the future we will be adding many more settings to this
//	function.
//
void setupOpenGL() {
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

//
//	void renderScene()
//
//		This method will contain all of the objects to be drawn.
//
void renderScene() {

    //*******************************************
    //***                                     ***
    //***   YOUR CODE WILL GO HERE            ***
    //***                                     ***
    //*******************************************

}

//*************************************************************************************
//
// Our main function

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main( int argc, char *argv[] ) {
    GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
                                        // GLFW sets up our OpenGL context so must be done first
    setupOpenGL();						// initialize all of the OpenGL specific information

    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
        glClear( GL_COLOR_BUFFER_BIT );	// clear the current color contents in the window

        // update the projection matrix based on the window size
        // the GL_PROJECTION matrix governs properties of the view coordinates;
        // i.e. what gets seen - use an Orthographic projection that ranges
        // from [0, windowWidth] in X and [0, windowHeight] in Y. (0,0) is the lower left.
        glm::mat4 projMtx = glm::ortho( 0.0f, (GLfloat)windowWidth, 0.0f, (GLfloat)windowHeight );
        glMatrixMode( GL_PROJECTION );	// change to the Projection matrix
        glLoadIdentity();				// set the matrix to be the identity
        glMultMatrixf( &projMtx[0][0] );// load our orthographic projection matrix into OpenGL's projection matrix state

        // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
        // when using a Retina display the actual window can be larger than the requested window.  Therefore
        // query what the actual size of the window we are rendering to is.
        GLint framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

        // update the viewport - tell OpenGL we want to render to the whole window
        glViewport( 0, 0, framebufferWidth, framebufferHeight );

        glMatrixMode( GL_MODELVIEW );	// make the ModelView matrix current to be modified by any transformations
        glLoadIdentity();				// set the matrix to be the identity

        renderScene();					// draw everything to the window

        glfwSwapBuffers(window);		// flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();				// check for any events and signal to redraw screen

        // the following code is a hack for OSX Mojave
        // the window is initially black until it is moved
        // so instead of having the user manually move the window,
        // we'll automatically move it and then move it back
        if( !mackHack ) {
            GLint xpos, ypos;
            glfwGetWindowPos(window, &xpos, &ypos);
            glfwSetWindowPos(window, xpos+10, ypos+10);
            glfwSetWindowPos(window, xpos, ypos);
            mackHack = true;
        }
    }

    glfwDestroyWindow( window );		// clean up and close our window
    glfwTerminate();					// shut down GLFW to clean up our context

    return EXIT_SUCCESS;				// exit our program successfully!
}
