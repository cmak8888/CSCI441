/*
 *  CSCI 441, Computer Graphics, Fall 2017
 *
 *  Project: lab12
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
#include <GLFW/glfw3.h>            // include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SOIL/SOIL.h>        // for image loading

#include <stdio.h>                // for printf functionality
#include <stdlib.h>                // for exit functionality
#include <iostream>

#include <vector>                    // for vector

#include <CSCI441/FramebufferUtils3.hpp>
#include <CSCI441/modelLoader3.hpp>
#include <CSCI441/objects3.hpp>
#include <CSCI441/ShaderProgram3.hpp>


//******************************************************************************
//
// Global Parameters

int windowWidth, windowHeight;
bool controlDown = false;
bool leftMouseDown = false;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

glm::vec3 cameraAngles( 1.82f, 2.01f, 25.0f );
glm::vec3 eyePoint( 10.0f, 10.0f, 10.0f );
glm::vec3 lookAtPoint( 0.0f, 0.0f, 0.0f );
glm::vec3 upVector( 0.0f, 1.0f, 0.0f );

GLuint platformVAOd;
GLuint platformTextureHandle;

GLuint skyboxVAOds[6];                      // all of our skybox VAOs
GLuint skyboxHandles[6];                    // all of our skybox handles

CSCI441::ShaderProgram *textureShaderProgram = NULL;
struct TextureShaderUniformLocs {
	GLint modelMtx;
	GLint viewProjectionMtx;
	GLint tex;
	GLint color;
} textureShaderUniforms;
struct TextureShaderAttributeLocs {
	GLint vPos;
	GLint vTextureCoord;
} textureShaderAttributes;

CSCI441::ShaderProgram *modelPhongShaderProgram = NULL;
struct ModelPhongShaderUniformLocs {
	GLint modelviewMtx;
	GLint viewMtx;
	GLint projectionMtx;
	GLint normalMtx;
	GLint materialDiffuse;
	GLint materialSpecular;
	GLint materialAmbient;
	GLint materialShininess;
	GLint txtr;
} modelPhongShaderUniforms;
struct ModelPhongShaderAttributeLocs {
	GLint vPos;
	GLint vNorm;
	GLint vTexCoord;
} modelPhongShaderAttributes;

CSCI441::ModelLoader *model = NULL;

GLuint fbo;
GLuint rbo;
int framebufferWidth = 1024, framebufferHeight = 1024;
GLuint framebufferTextureHandle;

CSCI441::ShaderProgram *postprocessingShaderProgram = NULL;
struct PostShaderUniformLocs {
	GLint projectionMtx;
	GLint fbo;
} postShaderUniforms;
struct PostShaderAttributeLocs {
	GLint vPos;
	GLint vTexCoord;
} postShaderAttributes;

GLuint texturedQuadVAO;

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

bool registerOpenGLTexture( unsigned char *textureData,
                            unsigned int texWidth, unsigned int texHeight,
                            GLuint &textureHandle ) {
    if( textureData == 0 ) {
        fprintf( stderr, "Cannot register texture; no data specified." );
        return false;
    }

    glGenTextures( 1, &textureHandle );
    glBindTexture( GL_TEXTURE_2D, textureHandle );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData );

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
static void error_callback( int error, const char *description ) {
    fprintf( stderr, "[ERROR]: %s\n", description );
}

// key_callback() //////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's keypress callback.
//	Responds to key presses and key releases
//
////////////////////////////////////////////////////////////////////////////////
static void key_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
    if((key == GLFW_KEY_ESCAPE || key == 'Q') && action == GLFW_PRESS )
        glfwSetWindowShouldClose( window, GLFW_TRUE );
}

// mouse_button_callback() /////////////////////////////////////////////////////
//
//		We will register this function as GLFW's mouse button callback.
//	Responds to mouse button presses and mouse button releases.  Keeps track if
//	the control key was pressed when a left mouse click occurs to allow
//	zooming of our arcball camera.
//
////////////////////////////////////////////////////////////////////////////////
static void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
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
static void cursor_callback( GLFWwindow *window, double xpos, double ypos ) {
    // make sure movement is in bounds of the window
    // glfw captures mouse movement on entire screen

    if( xpos > 0 && xpos < windowWidth ) {
        if( ypos > 0 && ypos < windowHeight ) {
            // active motion
            if( leftMouseDown ) {
                if((mousePosition.x - -9999.0f) < 0.001f ) {
                    mousePosition.x = xpos;
                    mousePosition.y = ypos;
                } else {
                    if( !controlDown ) {
                        cameraAngles.x += (xpos - mousePosition.x) * 0.005f;
                        cameraAngles.y += (ypos - mousePosition.y) * 0.005f;

                        if( cameraAngles.y < 0 ) cameraAngles.y = 0.0f + 0.001f;
                        if( cameraAngles.y >= M_PI ) cameraAngles.y = M_PI - 0.001f;
                    } else {
                        double totChgSq = (xpos - mousePosition.x) + (ypos - mousePosition.y);
                        cameraAngles.z += totChgSq * 0.01f;

                        if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
                        if( cameraAngles.z >= 35.0f ) cameraAngles.z = 35.0f;
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
static void scroll_callback( GLFWwindow *window, double xOffset, double yOffset ) {
    double totChgSq = yOffset;
    cameraAngles.z += totChgSq * 0.2f;

    if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
    if( cameraAngles.z >= 35.0f ) cameraAngles.z = 35.0f;

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
GLFWwindow *setupGLFW() {
    // set what function to use when registering errors
    // this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
    // all other GLFW calls must be performed after GLFW has been initialized
    glfwSetErrorCallback( error_callback );

    // initialize GLFW
    if( !glfwInit()) {
        fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
        exit( EXIT_FAILURE );
    } else {
        fprintf( stdout, "[INFO]: GLFW initialized\n" );
    }

    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT,
                    GL_TRUE );                        // request forward compatible OpenGL context
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );    // request OpenGL Core Profile context
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );        // request OpenGL 3.x context
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );        // request OpenGL 3.3 context

    // create a window for a given size, with a given title
    GLFWwindow *window = glfwCreateWindow( 640, 480, "Lab12: FBOs", NULL, NULL );
    if( !window ) {                        // if the window could not be created, NULL is returned
        fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    } else {
        fprintf( stdout, "[INFO]: GLFW Window created\n" );
    }

    glfwMakeContextCurrent( window );    // make the created window the current window
    glfwSwapInterval( 1 );                    // update our screen after at least 1 screen refresh

    glfwSetKeyCallback( window, key_callback );    // set our keyboard callback function
    glfwSetMouseButtonCallback( window, mouse_button_callback );    // set our mouse button callback function
    glfwSetCursorPosCallback( window, cursor_callback );    // set our cursor position callback function
    glfwSetScrollCallback( window, scroll_callback );    // set our scroll wheel callback function

    return window;                                        // return the window that was created
}

// setupOpenGL() ///////////////////////////////////////////////////////////////
//
//      Used to setup everything OpenGL related.
//
////////////////////////////////////////////////////////////////////////////////
void setupOpenGL() {
    glEnable( GL_DEPTH_TEST );                    // enable depth testing
    glDepthFunc( GL_LESS );                            // use less than depth test

    glFrontFace( GL_CCW );

    glEnable( GL_BLEND );                                    // enable blending
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    // use one minus blending equation

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );    // clear the frame buffer to black
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
        printf( "[ERROR]: Error initalizing GLEW\n" );
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf( stderr, "[ERROR]: %s\n", glewGetErrorString( glewResult ));
        exit( EXIT_FAILURE );
    } else {
        fprintf( stdout, "[INFO]: GLEW initialized\n" );
        fprintf( stdout, "[INFO]: Status: Using GLEW %s\n", glewGetString( GLEW_VERSION ));
    }

    if( !glewIsSupported( "GL_VERSION_2_0" )) {
        printf( "[ERROR]: OpenGL not version 2.0+.  GLSL not supported\n" );
        exit( EXIT_FAILURE );
    }
}

// setupTextures() /////////////////////////////////////////////////////////////
//
//      Load and register all the tetures for our program
//
////////////////////////////////////////////////////////////////////////////////
void setupTextures() {
    platformTextureHandle = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/ground.png" );

    // and get handles for our full skybox
    printf( "[INFO]: registering skybox...\n" );
    fflush( stdout );
    skyboxHandles[0] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/DOOM16BK.png" );
    skyboxHandles[1] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/DOOM16RT.png" );
    skyboxHandles[2] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/DOOM16FT.png" );
	skyboxHandles[3] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/DOOM16LF.png" );
    skyboxHandles[4] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/DOOM16DN.png" );
    skyboxHandles[5] = CSCI441::TextureUtils::loadAndRegisterTexture( "textures/skybox/DOOM16UP.png" );
    printf( "[INFO]: skybox textures read in and registered!\n\n" );
}

void setupShaders() {
    textureShaderProgram = new CSCI441::ShaderProgram( "shaders/textureShader.v.glsl", "shaders/textureShader.f.glsl" );
    textureShaderUniforms.modelMtx          = textureShaderProgram->getUniformLocation( "modelMtx" );
    textureShaderUniforms.viewProjectionMtx = textureShaderProgram->getUniformLocation( "viewProjectionMtx" );
    textureShaderUniforms.tex               = textureShaderProgram->getUniformLocation( "tex" );
    textureShaderUniforms.color             = textureShaderProgram->getUniformLocation( "color" );
    textureShaderAttributes.vPos            = textureShaderProgram->getAttributeLocation( "vPos" );
    textureShaderAttributes.vTextureCoord   = textureShaderProgram->getAttributeLocation( "vTextureCoord" );

    modelPhongShaderProgram = new CSCI441::ShaderProgram( "shaders/texturingPhong.v.glsl", "shaders/texturingPhong.f.glsl" );
    modelPhongShaderUniforms.modelviewMtx      = modelPhongShaderProgram->getUniformLocation( "modelviewMtx" );
    modelPhongShaderUniforms.viewMtx           = modelPhongShaderProgram->getUniformLocation( "viewMtx" );
    modelPhongShaderUniforms.projectionMtx     = modelPhongShaderProgram->getUniformLocation( "projectionMtx" );
    modelPhongShaderUniforms.normalMtx         = modelPhongShaderProgram->getUniformLocation( "normalMtx" );
    modelPhongShaderUniforms.materialDiffuse   = modelPhongShaderProgram->getUniformLocation( "materialDiffuse" );
    modelPhongShaderUniforms.materialSpecular  = modelPhongShaderProgram->getUniformLocation( "materialSpecular" );
    modelPhongShaderUniforms.materialAmbient   = modelPhongShaderProgram->getUniformLocation( "materialAmbient" );
    modelPhongShaderUniforms.materialShininess = modelPhongShaderProgram->getUniformLocation( "materialShininess" );
    modelPhongShaderUniforms.txtr              = modelPhongShaderProgram->getUniformLocation( "txtr" );
    modelPhongShaderAttributes.vPos            = modelPhongShaderProgram->getAttributeLocation( "vPos" );
    modelPhongShaderAttributes.vNorm           = modelPhongShaderProgram->getAttributeLocation( "vNormal" );
    modelPhongShaderAttributes.vTexCoord       = modelPhongShaderProgram->getAttributeLocation( "vTexCoord" );

    postprocessingShaderProgram = new CSCI441::ShaderProgram( "shaders/grayscale.v.glsl", "shaders/grayscale.f.glsl" );
    postShaderUniforms.projectionMtx = postprocessingShaderProgram->getUniformLocation( "projectionMtx" );
    postShaderUniforms.fbo           = postprocessingShaderProgram->getUniformLocation( "fbo" );
    postShaderAttributes.vPos        = postprocessingShaderProgram->getAttributeLocation( "vPos" );
    postShaderAttributes.vTexCoord   = postprocessingShaderProgram->getAttributeLocation( "vTexCoord" );
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
    // Model

    model = new CSCI441::ModelLoader();
    model->loadModelFile( "models/medstreet/medstreet.obj" );

    //////////////////////////////////////////
    //
    // PLATFORM

    GLfloat platformSize = 20.0f;

    VertexTextured platformVertices[4] = {
            {-platformSize, 0.0f, -platformSize, 0.0f, 0.0f}, // 0 - BL
            {platformSize,  0.0f, -platformSize, 1.0f, 0.0f}, // 1 - BR
            {-platformSize, 0.0f, platformSize,  0.0f, -1.0f}, // 2 - TL
            {platformSize,  0.0f, platformSize,  1.0f, -1.0f}  // 3 - TR
    };

    unsigned short platformIndices[4] = {0, 1, 2, 3};

    glGenVertexArrays( 1, &platformVAOd );
    glBindVertexArray( platformVAOd );

    GLuint vbods[2];
    glGenBuffers( 2, vbods );

    glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( platformVertices ), platformVertices, GL_STATIC_DRAW );

    glEnableVertexAttribArray( textureShaderAttributes.vPos );
    glVertexAttribPointer( textureShaderAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof( VertexTextured ), (void *)0 );

    glEnableVertexAttribArray( textureShaderAttributes.vTextureCoord );
    glVertexAttribPointer( textureShaderAttributes.vTextureCoord, 2, GL_FLOAT, GL_FALSE, sizeof( VertexTextured ), (void *)(sizeof( float ) * 3));

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    //////////////////////////////////////////
    //
    // SKYBOX

    unsigned short skyboxIndices[4] = {
            0, 1, 2, 3
    };

    const GLfloat SKYBOX_SIZE = 40.0f;
    VertexTextured skyboxVertices[6][4] = {
            { // back
                {-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,   0.0f,  0.0f}, // 0 - BL
                {-SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,  -1.0f,  0.0f}, // 1 - BR
                {-SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,   0.0f,  1.0f}, // 2 - TL
                {-SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  -1.0f,  1.0f}  // 3 - TR
            },

            { // right
                {-SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,   0.0f,  0.0f}, // 0 - BL
                { SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,  -1.0f,  0.0f}, // 1 - BR
                {-SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,   0.0f,  1.0f}, // 2 - TL
                { SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  -1.0f,  1.0f}  // 3 - TR
            },

            { // front
                { SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,   0.0f,  0.0f}, // 0 - BL
                { SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,   1.0f,  0.0f}, // 1 - BR
                { SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,   0.0f,  1.0f}, // 2 - TL
                { SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,   1.0f,  1.0f}  // 3 - TR
            },

            { // left
                {-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,   0.0f,  0.0f}, // 0 - BL
                { SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,   1.0f,  0.0f}, // 1 - BR
                {-SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,   0.0f,  1.0f}, // 2 - TL
                { SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,   1.0f,  1.0f}  // 3 - TR
            },

            { // bottom
                {-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,   0.0f,  0.0f}, // 0 - BL
                { SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,   0.0f,  1.0f}, // 1 - BR
                {-SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,   1.0f,  0.0f}, // 2 - TL
                { SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE,   1.0f,  1.0f}  // 3 - TR
            },

            { // top
                {-SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,  -1.0f,  1.0f}, // 0 - BL
                { SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE,  -1.0f,  0.0f}, // 1 - BR
                {-SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,   0.0f,  1.0f}, // 2 - TL
                { SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,   0.0f,  0.0f}  // 3 - TR
            }
    };

    glGenVertexArrays( 6, skyboxVAOds );

    for( int i = 0; i < 6; i++ ) {
        glBindVertexArray( skyboxVAOds[i] );
        glGenBuffers( 2, vbods );
        glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( skyboxVertices[i] ), skyboxVertices[i], GL_STATIC_DRAW );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( skyboxIndices ), skyboxIndices, GL_STATIC_DRAW );
        glEnableVertexAttribArray( textureShaderAttributes.vPos );
        glVertexAttribPointer( textureShaderAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof( VertexTextured ), (void *)0 );
        glEnableVertexAttribArray( textureShaderAttributes.vTextureCoord );
        glVertexAttribPointer( textureShaderAttributes.vTextureCoord, 2, GL_FLOAT, GL_FALSE, sizeof( VertexTextured ), (void *)(sizeof( float ) * 3));
    }

    //////////////////////////////////////////
    //
    // TEXTURED QUAD

    // LOOKHERE #1

    VertexTextured texturedQuadVerts[4] = {
            {-1.0f, -1.0f, 0.0f, 0.0f, 0.0f}, // 0 - BL
            {1.0f,  -1.0f, 0.0f, 1.0f, 0.0f}, // 1 - BR
            {-1.0f, 1.0f,  0.0f, 0.0f, 1.0f}, // 2 - TL
            {1.0f,  1.0f,  0.0f, 1.0f, 1.0f}  // 3 - TR
    };

    unsigned short texturedQuadIndices[4] = {0, 1, 2, 3};

    glGenVertexArrays( 1, &texturedQuadVAO );
    glBindVertexArray( texturedQuadVAO );
    glGenBuffers( 2, vbods );
    glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( texturedQuadVerts ), texturedQuadVerts, GL_STATIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( texturedQuadIndices ), texturedQuadIndices, GL_STATIC_DRAW );
    postprocessingShaderProgram->useProgram();
    glEnableVertexAttribArray( postShaderAttributes.vPos );
    glVertexAttribPointer( postShaderAttributes.vPos, 3, GL_FLOAT, GL_FALSE, sizeof( VertexTextured ), (void *)0 );
    glEnableVertexAttribArray( postShaderAttributes.vTexCoord );
    glVertexAttribPointer( postShaderAttributes.vTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof( VertexTextured ), (void *)(sizeof( float ) * 3));
}

void setupFramebuffer() {
    // TODO #1 - Setup everything with the framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebufferWidth, framebufferHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    glGenTextures(1, &framebufferTextureHandle);
    glBindTexture(GL_TEXTURE_2D, framebufferTextureHandle);
    glActiveTexture(framebufferTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri( GL_TEXTURE_2D,  GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D,  GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D,  GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D,  GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTextureHandle, 0);

    //std::cout << glCheckFramebufferStatus();
    CSCI441::FramebufferUtils::printFramebufferStatusMessage(GL_FRAMEBUFFER);
    CSCI441::FramebufferUtils::printFramebufferInfo( GL_FRAMEBUFFER, fbo );
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
    glm::mat4 m(1.0f), vp = projectionMatrix * viewMatrix;

    // Use our texture shader program
    textureShaderProgram->useProgram();

    glUniformMatrix4fv( textureShaderUniforms.modelMtx, 1, GL_FALSE, &m[0][0] );
    glUniformMatrix4fv( textureShaderUniforms.viewProjectionMtx, 1, GL_FALSE, &vp[0][0] );
    glUniform1ui( textureShaderUniforms.tex, GL_TEXTURE0 );

    glm::vec3 white( 1, 1, 1 );
    glUniform3fv( textureShaderUniforms.color, 1, &white[0] );

    // draw the skybox
    for( unsigned int i = 0; i < 6; i++ ) {
        glBindTexture( GL_TEXTURE_2D, skyboxHandles[i] );
        glBindVertexArray( skyboxVAOds[i] );
        glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void *)0 );
    }

    // draw the platform
    glBindTexture( GL_TEXTURE_2D, platformTextureHandle );
    glBindVertexArray( platformVAOd );
    glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void *)0 );

    // translate the model up slightly to prevent depth fighting on the platform
    m = glm::translate( m, glm::vec3( 4, 0.1, 0 ));

    glm::mat4 mv = viewMatrix * m;
    glm::mat4 nMtx = glm::transpose( glm::inverse( mv ));

    // use our textured phong shader program for the model
    modelPhongShaderProgram->useProgram();
    glUniformMatrix4fv( modelPhongShaderUniforms.modelviewMtx, 1, GL_FALSE, &mv[0][0] );
    glUniformMatrix4fv( modelPhongShaderUniforms.viewMtx, 1, GL_FALSE, &viewMatrix[0][0] );
    glUniformMatrix4fv( modelPhongShaderUniforms.projectionMtx, 1, GL_FALSE, &projectionMatrix[0][0] );
    glUniformMatrix4fv( modelPhongShaderUniforms.normalMtx, 1, GL_FALSE, &nMtx[0][0] );
    glUniform1ui( modelPhongShaderUniforms.txtr, GL_TEXTURE0 );

    // draw the model
    model->draw( modelPhongShaderAttributes.vPos, modelPhongShaderAttributes.vNorm, modelPhongShaderAttributes.vTexCoord,
                 modelPhongShaderUniforms.materialDiffuse, modelPhongShaderUniforms.materialSpecular, modelPhongShaderUniforms.materialShininess, modelPhongShaderUniforms.materialAmbient,
                 GL_TEXTURE0 );
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
    GLFWwindow *window = setupGLFW();    // initialize all of the GLFW specific information releated to OpenGL and our window
    setupOpenGL();                                        // initialize all of the OpenGL specific information
    setupGLEW();                                            // initialize all of the GLEW specific information
    setupShaders();                                        // load our shaders into memory
    setupBuffers();                                        // load all our VAOs and VBOs into memory
    setupTextures();                                    // load all textures into memory
    setupFramebuffer();                                // setup our framebuffer

    convertSphericalToCartesian();        // set up our camera position

    CSCI441::setVertexAttributeLocations( textureShaderAttributes.vPos, -1, textureShaderAttributes.vTextureCoord );
    CSCI441::drawSolidSphere( 1, 16, 16 );    // strange hack I need to make spheres draw - don't have time to investigate why..it's a bug with my library

    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while( !glfwWindowShouldClose( window )) {    // check if the window was instructed to be closed
        // Get the size of our window framebuffer.  Ideally this should be the same dimensions as our window, but
        // when using a Retina display the actual window can be larger than the requested window.  Therefore
        // query what the actual size of the window we are rendering to is.
        glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

        /////////////////////////////
        // FIRST PASS
        /////////////////////////////
        // TODO #2
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport( 0, 0, framebufferWidth, framebufferHeight );
        glClear( GL_COLOR_BUFFER_BIT |
                 GL_DEPTH_BUFFER_BIT );    // clear the current color contents and depth buffer in the framebuffer

        // set the projection matrix based on the window size
        // use a perspective projection that ranges
        // with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
        glm::mat4 projectionMatrix = glm::perspective( 45.0f, framebufferWidth / (float)framebufferHeight, 0.001f, 100.0f );

        // set up our look at matrix to position our camera
        glm::mat4 viewMatrix = glm::lookAt( eyePoint, lookAtPoint, upVector );

        // pass our view and projection matrices
        renderScene( viewMatrix, projectionMatrix );
        glFlush();

        /////////////////////////////
        // SECOND PASS
        /////////////////////////////
        // TODO #3
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport( 0, 0, windowWidth, windowHeight );
        glClear( GL_COLOR_BUFFER_BIT |
                 GL_DEPTH_BUFFER_BIT );    // clear the current color contents and depth buffer
        glm::mat4 projeMatrix = glm::ortho(-1,1,-1,1);
        postprocessingShaderProgram->useProgram();
        glUniformMatrix4fv(postShaderUniforms.projectionMtx, 1, GL_FALSE, &projeMatrix[0][0]);

        glBindTexture(GL_TEXTURE_2D, framebufferTextureHandle);
        glBindVertexArray(texturedQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0,4);

        // Show it all on the screen now!

        glfwSwapBuffers( window );// flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();                // check for any events and signal to redraw screen
    }

    glfwDestroyWindow( window );// clean up and close our window
    glfwTerminate();                        // shut down GLFW to clean up our context

    return EXIT_SUCCESS;                // exit our program successfully!
}
