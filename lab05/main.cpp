/*
 *  CSCI 441, Computer Graphics, Fall 2017
 *
 *  Project: lab05
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to add textures to our
 *  objects.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2017
 *
 *
 */

// include the OpenGL library header
#ifdef __APPLE__											// if compiling on Mac OS
#include <OpenGL/gl.h>
#else																	// if compiling on Linux or Windows OS
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>								// include GLFW framework header

#include <CSCI441/objects.hpp> 				// for our 3D objects
#include <CSCI441/OpenGLUtils.hpp>		// for OpenGL helpers
#include <CSCI441/TextureUtils.hpp>		// for Texture helpers
#include <SOIL/SOIL.h>

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>										// for printf functionality
#include <stdlib.h>										// for exit functionality
#include <iostream>
#include <fstream>
#include <string>

//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 512, windowHeight = 512;

int leftMouseButton;    	 									// status of the mouse button
bool controlDown;                           // status of control key
glm::vec2 mousePosition;				            // last known X and Y of the mouse

glm::vec3 camPos;            							// camera position in cartesian coordinates
glm::vec3 camAngles;               				// camera DIRECTION in spherical coordinates stored as (theta, phi, radius)
glm::vec3 camDir; 			                    // camera DIRECTION in cartesian coordinates

int objectIndex;

GLuint brickTexHandle, minesTexHandle;

//*************************************************************************************
//
// Helper Function

// getRand() ///////////////////////////////////////////////////////////////////
//
//  Simple helper function to return a random number between 0.0f and 1.0f.
//
////////////////////////////////////////////////////////////////////////////////
float getRand() {
    return rand() / (float)RAND_MAX;
}

// recomputeOrientation() //////////////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraTheta, cameraPhi, or cameraRadius is updated.
//
////////////////////////////////////////////////////////////////////////////////
void recomputeOrientation() {
    camDir.x =  sinf(camAngles.x)*sinf(camAngles.y);
    camDir.z = -cosf(camAngles.x)*sinf(camAngles.y);
    camDir.y = -cosf(camAngles.y);

    //and normalize this directional vector!
    camDir = glm::normalize( camDir );
}

bool registerOpenGLTexture(unsigned char *textureData,
                           unsigned int texWidth, unsigned int texHeight,
                           GLuint &textureHandle) {

    if( textureData == 0 ) {
        fprintf(stderr,"Cannot register texture; no data specified.");
        return false;
    }

    // TODO #2b: Fill ALL this in
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    return true;
}

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

static void keyboard_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
    if( action == GLFW_PRESS ) {
        switch( key ) {
            case GLFW_KEY_ESCAPE:
            case GLFW_KEY_Q:
                exit(EXIT_SUCCESS);

                // switches between all the objects we can draw
            case GLFW_KEY_1:
            case GLFW_KEY_2:
            case GLFW_KEY_3:
            case GLFW_KEY_4:
            case GLFW_KEY_5:
            case GLFW_KEY_6:
            case GLFW_KEY_7:
            case GLFW_KEY_8:
            case GLFW_KEY_9:
                objectIndex = key - GLFW_KEY_1; // they go in sequence from GLFW_KEY_1
        }
    }
}

static void cursor_callback( GLFWwindow *window, double x, double y ) {
    if(leftMouseButton == GLFW_PRESS) {
        if( !controlDown ) {
            camAngles.x += (x - mousePosition.x)*0.005;
            camAngles.y += (mousePosition.y - y)*0.005;

            // make sure that phi stays within the range (0, M_PI)
            if(camAngles.y <= 0)
                camAngles.y = 0+0.001;
            if(camAngles.y >= M_PI)
                camAngles.y = M_PI-0.001;
        } else {
            camAngles.z += ((x - mousePosition.x) + (y - mousePosition.y)) * 0.01;
            if( camAngles.z < -25 ) {
                camAngles.z = -25.0f;
            }
            if( camAngles.z > 0.5 ) {
                camAngles.z = 0.5f;
            }
        }

        recomputeOrientation();     //update camera (x,y,z) based on (radius,theta,phi)
    }

    mousePosition.x = x;
    mousePosition.y = y;
}

static void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
    if( button == GLFW_MOUSE_BUTTON_LEFT ) {
        leftMouseButton = action;
        controlDown = (mods & GLFW_MOD_CONTROL);
    }
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
    // create a window for a given size, with a given title
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		// do not allow our window to be able to be resized

    GLFWwindow *window = glfwCreateWindow( windowWidth, windowHeight, "Lab05: Texture That Teapot", NULL, NULL );
    if( !window ) {						// if the window could not be created, NULL is returned
        fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    } else {
        fprintf( stdout, "[INFO]: GLFW Window created\n" );
    }

    glfwMakeContextCurrent(window);		// make the created window the current window
    glfwSwapInterval(1);				     // update our screen after at least 1 screen refresh

    glfwSetKeyCallback( window, keyboard_callback );	// set our keyboard callback function
    glfwSetCursorPosCallback( window, cursor_callback );					// set our cursor position callback function
    glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function

    return window;						       // return the window that was created
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
    // tell OpenGL to perform depth testing with the Z-Buffer to perform hidden
    //		surface removal.  We will discuss this more very soon.
    glEnable( GL_DEPTH_TEST );

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black

    glEnable( GL_CULL_FACE );           // enable back face culling to speed render times
    glFrontFace( GL_CCW );              // denote front faces specified by counter-clockwise winding order
    glCullFace( GL_BACK );              // cull our back faces

    glShadeModel( GL_SMOOTH );          // use smooth shading
}

//
//	setupScene()
//
//	setup everything specific to our scene.  in this case,
//	position our camera
//
void setupScene() {
    // give the camera a scenic starting point.
    camAngles.x = -M_PI / 3.0f;    // theta
    camAngles.y = M_PI / 2.8f;    // phi
    camAngles.z = -15.0f;            // radius
    recomputeOrientation();
}

//
// setupLights()
//
// Set the types and colors of our lights
//
void setupLights() {
    glEnable( GL_LIGHTING );            // we are now using lighting
    glEnable( GL_LIGHT0 );              // and turn on Light 0 please (and thank you)
    float diffuseCol[4] = { 1.0, 1.0, 1.0, 1.0 };           // white diffuse light
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseCol );
    float specularCol[4] = { 1.0, 1.0, 1.0, 1.0 };          // white specular light
    glLightfv( GL_LIGHT0, GL_SPECULAR, specularCol );
    float ambientCol[4] = { 0.2, 0.2, 0.2, 1.0 };           // soft gray ambient light
    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientCol );
}

//
// setupTextures()
//
// Load our textures into memory and register them with OpenGL
//
void setupTextures() {
    int textWidth,textHeight;
    unsigned char* imageData;
    // TODO #1: Read in the brick PPM file
    CSCI441::loadPPM("textures/brick.ppm", textWidth, textHeight, imageData);
    // TODO #2a: call the registerOpenGLTexture() function
    registerOpenGLTexture(imageData,textWidth,textHeight, brickTexHandle);

    // TODO #5: Read in non-PPM
    minesTexHandle = SOIL_load_OGL_texture( "textures/mines.png",SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,  SOIL_FLAG_MIPMAPS  | SOIL_FLAG_INVERT_Y  | SOIL_FLAG_COMPRESS_TO_DXT  );
    std::cout << minesTexHandle;
    // TODO #6: Register non-PPM
    glBindTexture(GL_TEXTURE_2D, minesTexHandle);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST | GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST | GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if( minesTexHandle == 0 ) {
        fprintf(stderr,"Cannot register texture; no data specified.");
    }
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

//
//  void drawAxes()
//
//      Draw a set of unit vectors at the origin, showing the user
//  the positive and negative X, Y, and Z axes for perspective.
//
void drawAxes() {
    /*
     *	We will get to why we need to do this when we talk about lighting,
     *	but for now whenever we want to draw something with an OpenGL
     *	Primitive - like a line or point - we need to disable lighting
     *	and then reenable it for use with the 3D Object.
     */
    glDisable( GL_LIGHTING );

    // draw our axes
    glBegin( GL_LINES ); {
        // +X axis in bright red
        glColor3f(1,0,0);
        glVertex3f(0,0,0); glVertex3f(1,0,0);
        // -X axis in dark red
        glColor3f(0.5,0,0);
        glVertex3f(0,0,0); glVertex3f(-1,0,0);

        // +Y axis in bright green
        glColor3f(0,1,0);
        glVertex3f(0,0,0); glVertex3f(0,1,0);
        // -Y axis in dark green
        glColor3f(0,0.5,0);
        glVertex3f(0,0,0); glVertex3f(0,-1,0);

        // +Z axis in bright blue
        glColor3f(0,0,1);
        glVertex3f(0,0,0); glVertex3f(0,0,1);
        // -Z axis in dark blue
        glColor3f(0,0,0.5);
        glVertex3f(0,0,0); glVertex3f(0,0,-1);
    }; glEnd();

    /*
     *	As noted above, we are done drawing with OpenGL Primitives, so we
     *	must turn lighting back on.
     */
    glEnable(GL_LIGHTING);
}

//
//  void drawGrid()
//
//      Simple function to draw a grid in the X-Z plane using GL_LINES.
//
void drawGrid() {
    glDisable(GL_LIGHTING);

    glColor3f(1,1,1);
    glBegin(GL_LINES); {
        //draw the lines along the Z axis
        for(int i = 0; i < 11; i++) {
            glVertex3f((i-5), 0, -5);
            glVertex3f((i-5), 0, 5);
        }
        //draw the lines along the X axis
        for(int i = 0; i < 11; i++) {
            glVertex3f(-5, 0, (i-5));
            glVertex3f(5, 0, (i-5));
        }
    }; glEnd();

    glEnable(GL_LIGHTING);
}

//
//	void renderScene()
//
//		This method will contain all of the objects to be drawn.
//
void renderScene(void)  {
    glm::mat4 scaleAxes = glm::scale( CSCI441::identityMatrixMat4(), glm::vec3( 5.0f, 5.0f, 5.0f ) );
    CSCI441::pushMatrix( scaleAxes ); {
        drawAxes();			// draw our axes so we know how we are oriented
    }; CSCI441::popMatrix( scaleAxes );

    //move the grid down a touch so that it doesn't overlap with the axes
    glm::mat4 transGrid = glm::translate( CSCI441::identityMatrixMat4(), glm::vec3( 0.0f, -1.0f, 0.0f ) );
    CSCI441::pushMatrix( transGrid ); {
        drawGrid();
    }; CSCI441::popMatrix( transGrid );

    CSCI441::MaterialStruct emeraldMaterial = {
            glm::vec4(0.396f, 0.74151f, 0.69102f, 1.0f),			// diffuse
            glm::vec4(0.1f, 0.18725f, 0.1745f, 1.0f),					// ambient
            glm::vec4(0.297254f, 0.30829f, 0.306678f, 1.0f),	// specular
            0.1f * 128																				// shininess
    };
    CSCI441::setMaterial( emeraldMaterial );

    // TODO #3a

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,minesTexHandle);
    switch( objectIndex ) {
        case 0: CSCI441::drawSolidTeapot( 2.0 );                            break;
        case 1: CSCI441::drawSolidCube( 4.0 );                              break;
        case 2: CSCI441::drawSolidSphere( 3.0, 16, 16 );                    break;
        case 3: CSCI441::drawSolidTorus( 1.0, 2.0, 16, 16 );                break;
        case 4: CSCI441::drawSolidCone( 2.0, 4.0, 16, 16 );                 break;
        case 5: CSCI441::drawSolidCylinder( 2.0, 2.0, 4.0, 16, 16 );        break;
        case 6: CSCI441::drawSolidDisk( 1.0, 2.0, 16, 16 );                 break;
        case 7: CSCI441::drawSolidPartialDisk( 1.0, 2.0, 16, 16, 30, 120 ); break;
        case 8:
            // TODO 4a/b/c
            glBegin(GL_TRIANGLE_STRIP); {
                glNormal3f(0,0,1);
                glTexCoord2i(2,0);
                glVertex2f(5,0);


                glNormal3f(0,0,1);
                glTexCoord2i(2,2);
                glVertex2f(5,5);

                glNormal3f(0,0,1);
                glTexCoord2i(0,0);
                glVertex2f(0,0);

                glNormal3f(0,0,1);
                glTexCoord2i(0,2);
                glVertex2f(0,5);
            } glEnd();
            break;
    }

    // TODO #3b
    glDisable(GL_TEXTURE_2D);
}

///*************************************************************************************
//
// Our main function

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main( int argc, char *argv[] ) {
    // GLFW sets up our OpenGL context so must be done first
    GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
    setupOpenGL();										// initialize all of the OpenGL specific information
    setupLights();										// set our light parameters
    setupTextures();                  // load our textures into memory and register with OpenGL
    setupScene();											// initialize objects in our scene
    CSCI441::OpenGLUtils::printOpenGLInfo();       // print information related to our OpenGL context

    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
        glDrawBuffer( GL_BACK );				// work with our back frame buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

        // update the projection matrix based on the window size
        // the GL_PROJECTION matrix governs properties of the view coordinates;
        // i.e. what gets seen - use a perspective projection that ranges
        // with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
        glm::mat4 projMtx = glm::perspective( 45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.001f, 1000.0f );
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
        glLoadIdentity();							// set the matrix to be the identity

        // set up our look at matrix to position our camera
        glm::mat4 viewMtx = glm::lookAt( camDir * camAngles.z,
                                         glm::vec3(  0,  0,  0 ),
                                         glm::vec3(  0,  1,  0 ) );
        // multiply by the look at matrix - this is the same as our view martix
        glMultMatrixf( &viewMtx[0][0] );

        float lightPosition[4] = { 15.0, 15.0, 15.0, 1.0 };
        glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );

        renderScene();					// draw everything to the window

        glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();				// check for any events and signal to redraw screen
    }

    glfwDestroyWindow( window );// clean up and close our window
    glfwTerminate();						// shut down GLFW to clean up our context

    return EXIT_SUCCESS;				// exit our program successfully!
}
