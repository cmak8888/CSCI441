/*
 *  CSCI 441, Computer Graphics, Fall 2017
 *
 *  Project: lab11
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with VAOs & VBOs using a
 *	MD5 model.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2017
 *
 *
 */

//******************************************************************************

#include <GL/glew.h>
#include <GLFW/glfw3.h>			// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SOIL/SOIL.h>		// for image loading

#include <stdio.h>				// for printf functionality
#include <stdlib.h>				// for exit functionality
#include <time.h>				// for time functionality

#include <vector>					// for vector

#include <CSCI441/objects3.hpp>
#include <CSCI441/ShaderProgram3.hpp>
#include <CSCI441/TextureUtils.hpp>

#include "include/Marble.h"

//******************************************************************************
//
// Global Parameters

int windowWidth, windowHeight;
bool controlDown = false;
bool leftMouseDown = false;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

glm::vec3 cameraAngles( 1.82f, 2.01f, 25.0f );
glm::vec3 eyePoint(   10.0f, 10.0f, 10.0f );
glm::vec3 lookAtPoint( 0.0f,  0.0f,  0.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );

GLuint platformVAOd;
GLuint platformTextureHandle;
GLuint brickTexHandle;

const unsigned int NUM_WALLS = 6;
GLuint skyboxVAOds[NUM_WALLS];						// all of our skybox VAOs
GLuint skyboxHandles[NUM_WALLS];                    // all of our skybox handles

CSCI441::ShaderProgram* textureShaderProgram = NULL;
struct TextureShaderUniformLocations {
    GLint modelMtx;
    GLint viewProjectionMtx;
    GLint tex;
    GLint color;
} textureShaderUniforms;
struct TextureShaderAttributeLocations {
    GLint vPos;
    GLint vTextureCoord;
} textureShaderAttributes;

std::vector< Marble* > marbles;
const GLfloat GROUND_SIZE = 10;
const GLfloat MARBLE_RADIUS = 1.0;
const GLint NUM_MARBLES = 13;

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

bool registerOpenGLTexture(unsigned char *textureData,
                           unsigned int texWidth, unsigned int texHeight,
                           GLuint &textureHandle) {
    if(textureData == 0) {
        fprintf(stderr,"Cannot register texture; no data specified.");
        return false;
    }

    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

    return true;
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
    if( (key == GLFW_KEY_ESCAPE || key == 'Q') && action == GLFW_PRESS )
        glfwSetWindowShouldClose(window, GLFW_TRUE);
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
        controlDown = (mods & GLFW_MOD_CONTROL);
    } else {
        leftMouseDown = false;
        mousePosition.x = -9999.0f;
        mousePosition.y = -9999.0f;
        controlDown = false;
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
                        if( cameraAngles.z >= 30.0f ) cameraAngles.z = 30.0f;
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
    cameraAngles.z += totChgSq*0.2f;

    if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
    if( cameraAngles.z >= 30.0f ) cameraAngles.z = 30.0f;

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
    GLFWwindow *window = glfwCreateWindow(640, 480, "Lab11: Collision Detection", NULL, NULL);
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
    glewExperimental = GL_TRUE;
    GLenum glewResult = glewInit();

    /* check for an error */
    if( glewResult != GLEW_OK ) {
        printf( "[ERROR]: Error initalizing GLEW\n");
        /* Problem: glewInit failed, something is seriously wrong. */
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

// setupTextures() /////////////////////////////////////////////////////////////
//
//      Load and register all the tetures for our program
//
////////////////////////////////////////////////////////////////////////////////
void setupTextures() {
    platformTextureHandle = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/metal.jpg" );

    // and get handles for our full skybox
    printf( "[INFO]: registering skybox...\n" );
    skyboxHandles[0] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/back.png"   );
    skyboxHandles[1] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/left.png"   );
    skyboxHandles[2] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/front.png"  );
    skyboxHandles[3] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/right.png"  );
    skyboxHandles[4] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/bottom.png" );
    skyboxHandles[5] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/top.png"    );
    printf( "[INFO]: ...skybox textures read in and registered!\n\n" );

    unsigned char *brickTexData;
    int brickTexWidth, brickTexHeight;
    CSCI441::TextureUtils::loadPPM("textures/brick.ppm", brickTexWidth, brickTexHeight, brickTexData);
    registerOpenGLTexture(brickTexData, brickTexWidth, brickTexHeight, brickTexHandle);
    printf( "[INFO]: brick texture read in and registered\n" );
}

void setupShaders() {
    textureShaderProgram = new CSCI441::ShaderProgram( "shaders/textureShader.v.glsl", "shaders/textureShader.f.glsl" );

    textureShaderUniforms.modelMtx          = textureShaderProgram->getUniformLocation( "modelMtx" );
    textureShaderUniforms.viewProjectionMtx = textureShaderProgram->getUniformLocation( "viewProjectionMtx" );
    textureShaderUniforms.tex               = textureShaderProgram->getUniformLocation( "tex" );
    textureShaderUniforms.color             = textureShaderProgram->getUniformLocation( "color" );

    textureShaderAttributes.vPos            = textureShaderProgram->getAttributeLocation( "vPos" );
    textureShaderAttributes.vTextureCoord   = textureShaderProgram->getAttributeLocation( "vTextureCoord" );
}

// setupBuffers() //////////////////////////////////////////////////////////////
//
//      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {
    struct VertexTextured {
        float x, y, z;
        float s, t;
    };

    //////////////////////////////////////////
    //
    // PLATFORM

    GLfloat platformSize = GROUND_SIZE + MARBLE_RADIUS;

    VertexTextured platformVertices[4] = {
            { -platformSize, 0.0f, -platformSize,   0.0f,  0.0f }, // 0 - BL
            {  platformSize, 0.0f, -platformSize,   1.0f,  0.0f }, // 1 - BR
            { -platformSize, 0.0f,  platformSize,   0.0f,  1.0f }, // 2 - TL
            {  platformSize, 0.0f,  platformSize,   1.0f,  1.0f }  // 3 - TR
    };

    unsigned short platformIndices[4] = { 0, 1, 2, 3 };

    glGenVertexArrays( 1, &platformVAOd );
    glBindVertexArray( platformVAOd );

    GLuint vbods[2];
    glGenBuffers( 2, vbods );

    glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( platformVertices ), platformVertices, GL_STATIC_DRAW );

    glEnableVertexAttribArray( textureShaderAttributes.vPos );
    glVertexAttribPointer( textureShaderAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );

    glEnableVertexAttribArray( textureShaderAttributes.vTextureCoord );
    glVertexAttribPointer( textureShaderAttributes.vTextureCoord, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) (sizeof(float) * 3) );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    //////////////////////////////////////////
    //
    // SKYBOX

    VertexTextured skyboxVerts[NUM_WALLS][4] = {
            // back
            {
                    { -40.0f, -40.0f, -40.0f,   0.0f,   0.0f }, // 0 - BL
                    { -40.0f, -40.0f,  40.0f,  -1.0f,   0.0f }, // 1 - BR
                    { -40.0f,  40.0f, -40.0f,   0.0f,   1.0f }, // 2 - TL
                    { -40.0f,  40.0f,  40.0f,  -1.0f,   1.0f }  // 3 - TR
            },

            // right
            {
                    { -40.0f, -40.0f,  40.0f,   0.0f,   0.0f }, // 0 - BL
                    {  40.0f, -40.0f,  40.0f,  -1.0f,   0.0f }, // 1 - BR
                    { -40.0f,  40.0f,  40.0f,   0.0f,   1.0f }, // 2 - TL
                    {  40.0f,  40.0f,  40.0f,  -1.0f,   1.0f }  // 3 - TR
            },

            // front
            {
                    {  40.0f, -40.0f, -40.0f,   0.0f,   0.0f }, // 0 - BL
                    {  40.0f, -40.0f,  40.0f,   1.0f,   0.0f }, // 1 - BR
                    {  40.0f,  40.0f, -40.0f,   0.0f,   1.0f }, // 2 - TL
                    {  40.0f,  40.0f,  40.0f,   1.0f,   1.0f }  // 3 - TR
            },

            // left
            {
                    { -40.0f, -40.0f, -40.0f,   0.0f,   0.0f }, // 0 - BL
                    {  40.0f, -40.0f, -40.0f,   1.0f,   0.0f }, // 1 - BR
                    { -40.0f,  40.0f, -40.0f,   0.0f,   1.0f }, // 2 - TL
                    {  40.0f,  40.0f, -40.0f,   1.0f,   1.0f }  // 3 - TR
            },

            // ground
            {
                    { -40.0f, -40.0f, -40.0f,   1.0f,   1.0f }, // 0 - BL
                    {  40.0f, -40.0f, -40.0f,   1.0f,   0.0f }, // 1 - BR
                    { -40.0f, -40.0f,  40.0f,   0.0f,   1.0f }, // 2 - TL
                    {  40.0f, -40.0f,  40.0f,   0.0f,   0.0f }  // 3 - TR
            },

            // top
            {
                    { -40.0f,  40.0f, -40.0f,   1.0f,  -1.0f }, // 0 - BL
                    {  40.0f,  40.0f, -40.0f,   1.0f,   0.0f }, // 1 - BR
                    { -40.0f,  40.0f,  40.0f,   0.0f,  -1.0f }, // 2 - TL
                    {  40.0f,  40.0f,  40.0f,   0.0f,   0.0f }  // 3 - TR
            }
    };

    unsigned short skyBoxIndices[4] = {
            0, 1, 2, 3
    };

    glGenVertexArrays( 6, skyboxVAOds );

    for( unsigned int i = 0; i < NUM_WALLS; i++ ) {
        glBindVertexArray( skyboxVAOds[i] );
        glGenBuffers( 2, vbods );
        glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
        glBufferData( GL_ARRAY_BUFFER, sizeof(skyboxVerts[i]), skyboxVerts[i], GL_STATIC_DRAW );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(skyBoxIndices), skyBoxIndices, GL_STATIC_DRAW );
        glEnableVertexAttribArray(textureShaderAttributes.vPos);
        glVertexAttribPointer(textureShaderAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0);
        glEnableVertexAttribArray(textureShaderAttributes.vTextureCoord);
        glVertexAttribPointer(textureShaderAttributes.vTextureCoord, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) (sizeof(float) * 3));
    }
}

void populateMarbles() {
    srand( time(NULL) );
    const float RANGE_X = GROUND_SIZE*2;
    const float RANGE_Z = GROUND_SIZE*2;
    for(int i = 0; i < NUM_MARBLES; i++) {
        // TODO: Populate our marble locations
        Marble* m = new Marble( glm::vec3( rand()/(float)RAND_MAX * RANGE_X - RANGE_X/2.0f, 0.0f, (RANGE_Z * (i/(float)NUM_MARBLES)) - RANGE_Z/2.0f),
                                glm::vec3( rand()/(float)RAND_MAX - 0.5, 0.0, rand()/(float)RAND_MAX - 0.5 ),
                                MARBLE_RADIUS * (rand()/(float)RAND_MAX+0.25) );
        marbles.push_back( m );
    }
}

//******************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// renderScene() ///////////////////////////////////////////////////////////////
//
//		This method will contain all of the objects to be drawn.
//
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMatrix, glm::mat4 projectionMatrix ) {
    // use our texture shader program
    textureShaderProgram->useProgram();

    // set all our uniforms
    glm::mat4 modelMatrix(1.0f), vp = projectionMatrix * viewMatrix;
    glm::vec4 white(1,1,1,1);

    glUniformMatrix4fv(textureShaderUniforms.modelMtx, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(textureShaderUniforms.viewProjectionMtx, 1, GL_FALSE, &vp[0][0]);
    glUniform1ui(textureShaderUniforms.tex, GL_TEXTURE0);
    glUniform4fv(textureShaderUniforms.color, 1, &white[0]);

    // draw the skybox
    for( unsigned int i = 0; i < 6; i++ ) {
        glBindTexture( GL_TEXTURE_2D, skyboxHandles[i] );
        glBindVertexArray( skyboxVAOds[i] );
        glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );
    }

    // draw the platform
    glBindTexture( GL_TEXTURE_2D, platformTextureHandle );
    glBindVertexArray( platformVAOd );
    glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0 );

    // draw the marbles
    glBindTexture( GL_TEXTURE_2D, brickTexHandle );
    for( auto marble : marbles ) {
        marble->draw( modelMatrix, textureShaderUniforms.modelMtx, textureShaderUniforms.color );
    }
}

void moveMarbles() {
    // TODO #1 move every ball forward along its heading


    for(auto x: marbles) {
        x->moveForward();
    }


}

void collideMarblesWithWall() {
    // TODO #2 check if any ball passes beyond any wall

    for(auto marble: marbles) {
        if(marble->location.x > GROUND_SIZE) {

            glm::vec3 normal = glm::normalize(glm::vec3(-1, 0, 0));

            marble->moveBackward();
            marble->direction = marble->direction - 2 * glm::dot(marble->direction, normal) * normal;

        }

        else if(marble->location.z > GROUND_SIZE) {

            glm::vec3 normal = glm::normalize(glm::vec3(0, 0, -1));

            marble->moveBackward();
            marble->direction = marble->direction - 2 * glm::dot(marble->direction, normal) * normal;

        }

        else if(marble->location.x < -GROUND_SIZE) {

            glm::vec3 normal = glm::normalize(glm::vec3(1, 0, 0));

            marble->moveBackward();
            marble->direction = marble->direction - 2 * glm::dot(marble->direction, normal) * normal;
        }

        else if(marble->location.z < -GROUND_SIZE) {

            glm::vec3 normal = glm::normalize(glm::vec3(0, 0, 1));

            marble->moveBackward();
            marble->direction = marble->direction - 2 * glm::dot(marble->direction, normal) * normal;
        }
    }


}

void collideMarblesWithEachother() {
    // TODO #3
    // check for interball collisions
    // warning this isn't perfect...balls can get caught and
    // continually bounce back-and-forth in place off
    // each other

    for(unsigned int i = 0; i<marbles.size(); i++) {
        for(unsigned int j = 0; j<marbles.size(); j++) {

            if(i != j) {

                float distance = sqrt(pow(marbles[i]->location.x - marbles[j]->location.x, 2) + pow(marbles[i]->location.y - marbles[j]->location.y, 2) + pow(marbles[i]->location.z - marbles[j]->location.z, 2));
                if(distance < marbles[i]->getRadius() + marbles[j]->getRadius()) {
                    marbles[i]->moveBackward();
                    marbles[j]->moveBackward();
                    glm::vec3 normal = glm::normalize(marbles[i]->location - marbles[j]->location);
                    marbles[i]->direction = marbles[i]->direction - 2 * glm::dot(marbles[i]->direction, normal) * normal;
                    normal = glm::normalize(marbles[j]->location - marbles[i]->location);
                    marbles[j]->direction = marbles[j]->direction - 2 * glm::dot(marbles[j]->direction, normal) * normal;
                }
            }

        }
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
    // GLFW sets up our OpenGL context so must be done first
    GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
    setupOpenGL();										// initialize all of the OpenGL specific information
    setupGLEW();											// initialize all of the GLEW specific information
    setupShaders();										// load our shaders into memory
    setupBuffers();										// load all our VAOs and VBOs into memory
    setupTextures();									// load all textures into memory
    populateMarbles();								// generate marbles

    convertSphericalToCartesian();		// set up our camera position

    CSCI441::setVertexAttributeLocations( textureShaderAttributes.vPos, -1, textureShaderAttributes.vTextureCoord );
    CSCI441::drawSolidSphere( 1, 16, 16 );	// strange hack I need to make spheres draw - don't have time to investigate why..it's a bug with my library

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

        // THIS IS WHERE THE MAGICAL MAGIC HAPPENS!  Move everything
        moveMarbles();
        collideMarblesWithWall();
        collideMarblesWithEachother();
    }

    glfwDestroyWindow( window );// clean up and close our window
    glfwTerminate();						// shut down GLFW to clean up our context

    return EXIT_SUCCESS;				// exit our program successfully!
}