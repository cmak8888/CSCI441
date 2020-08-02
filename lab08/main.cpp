/*
 *  CSCI 441, Computer Graphics, Fall 2019
 *
 *  Project: lab08
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

#include "include/lab08.h"				// for Lab08BlackMagic
#include "include/MD5/md5model.h"	// for our MD5 Model

//******************************************************************************
//
// Global Parameters

#ifndef M_PI
#define M_PI 3.14159
#endif

int windowWidth, windowHeight;
bool controlDown = false;
bool leftMouseDown = false;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

glm::vec3 cameraAngles( 1.82f, 2.01f, 25.0f );
glm::vec3 eyePoint(   10.0f, 10.0f, 10.0f );
glm::vec3 lookAtPoint( 0.0f,  0.0f,  0.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );

md5_model_t md5model;
md5_anim_t md5animation;

md5_joint_t *skeleton = NULL;
anim_info_t animInfo;

bool animated = false;
bool displaySkeleton = false;
bool displayWireframe = false;
bool displayMesh = true;

// TODO #04A - create a global variable to store our VAO descriptor
GLuint vaods[4];
GLuint vbods[4];
GLuint indices;
GLuint platformTextureHandle;

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

// loadAndRegisterTexture() ////////////////////////////////////////////////////
//
// Load and register a texture with OpenGL
//
////////////////////////////////////////////////////////////////////////////////
GLuint loadAndRegisterTexture( const char *filename ) {
	GLuint texHandle = SOIL_load_OGL_texture( filename,
																					 SOIL_LOAD_AUTO,
																					 SOIL_CREATE_NEW_ID,
																					 SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT );

	if( texHandle == 0 ) {
			printf( "[ERROR]: Could not load texture \"%s\"\n[SOIL]: %s\n\n", filename, SOIL_last_result() );
	} else {
			printf( "[INFO]: Successfully loaded texture \"%s\"\n[SOIL]: %s\n\n", filename, SOIL_last_result() );
			glBindTexture(   GL_TEXTURE_2D,  platformTextureHandle );
			glTexParameteri( GL_TEXTURE_2D,  GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTexParameteri( GL_TEXTURE_2D,  GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D,  GL_TEXTURE_WRAP_S,     GL_REPEAT );
			glTexParameteri( GL_TEXTURE_2D,  GL_TEXTURE_WRAP_T,     GL_REPEAT );
	}

	return texHandle;
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
	else if( key == GLFW_KEY_S && action == GLFW_PRESS )
	  displaySkeleton = !displaySkeleton;
	else if( key == GLFW_KEY_W && action == GLFW_PRESS )
		displayWireframe = !displayWireframe;
	else if( key == GLFW_KEY_M && action == GLFW_PRESS )
		displayMesh = !displayMesh;
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
	GLFWwindow *window = glfwCreateWindow(640, 480, "Lab08: VAOs & VBOs", NULL, NULL);
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
		exit(EXIT_FAILURE);
	} else {
		 fprintf( stdout, "[INFO]: GLEW initialized\n" );
	}
}

// setupTextures() /////////////////////////////////////////////////////////////
//
//      Load and register all the textures for our program
//
////////////////////////////////////////////////////////////////////////////////
void setupTextures() {
	platformTextureHandle = loadAndRegisterTexture( "textures/metal.jpg" );
}

// setupTextures() /////////////////////////////////////////////////////////////
//
//      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {
	// TODO #01 - create our struct here
	struct VertexTextured {
	    float x,y,z,s,t;
	};

	// TODO #02: create our platform vertex array
	VertexTextured vertices[4] {
        {-10,0,-10,0,0},
        {10,0,-10,1,0},
        {10,0,10,1,1},
        {-10,0,10,0,1},
	};

    // TODO #03: create the index array
	unsigned short vertexQuad[4] {0,1,3,2};

	// TODO #04B: generate and bind the VAO
	glGenVertexArrays(4, vaods);
    glBindVertexArray(*vaods);
	// TODO #05A: generate and bind the array VBO
	glGenBuffers(1, vbods);
    glBindBuffer(GL_ARRAY_BUFFER,*vbods);
	// TODO #05B: send the vertex data to the GPU
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// TODO #06A: state where the vertex position is located within our array data
	glEnableVertexAttribArray(vertexPositionLocation);
	glVertexAttribPointer(vertexPositionLocation,3,GL_FLOAT,GL_FALSE,sizeof(VertexTextured),(void*)0);

	// TODO #06B: state where the texture coordinate is located within our array data

    glEnableVertexAttribArray(vertexTexCoordLocation);
    glVertexAttribPointer(vertexTexCoordLocation,2,GL_FLOAT,GL_FALSE,sizeof(VertexTextured),(void*)(sizeof(float) * 3));

	// TODO #07: generate and bind the element array VBO.  send data to the GPU
    glGenBuffers(1, &indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexQuad), vertexQuad, GL_STATIC_DRAW);
}

// loadMD5Model() //////////////////////////////////////////////////////////////
//
//      Load in the MD5 Model
//
////////////////////////////////////////////////////////////////////////////////
void loadMD5Model() {
	/* Load MD5 model file */
	if (!read_MD5_model("models/monsters/hellknight/mesh/hellknight.md5mesh", &md5model))
			exit (EXIT_FAILURE);

    alloc_vertex_arrays();	// allocate memory for arrays and create VAO for MD5 Model

	/* Load MD5 animation file */
	if (!read_MD5_anim("models/monsters/hellknight/animations/idle2.md5anim", &md5animation)) {
			exit (EXIT_FAILURE);
	} else {
			// successful loading...set up animation parameters
			animInfo.curr_frame = 0;
			animInfo.next_frame = 1;

			animInfo.last_time = 0;
			animInfo.max_time = 1.0 / md5animation.frameRate;

			/* Allocate memory for animated skeleton */
			skeleton = (md5_joint_t *)
			malloc (sizeof (md5_joint_t) * md5animation.num_joints);
	}

	printf("\n");
}

//******************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// renderScene() ///////////////////////////////////////////////////////////////
//
//		This method will contain all of the objects to be drawn.
//
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMatrix, glm::mat4 projectionMatrix, double deltaTime ) {
	Lab08BlackMagic::drawScene( projectionMatrix, viewMatrix );

	// TODO #08: draw the platform!
    //glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D,platformTextureHandle);
    //glBindVertexArray (*vaods);
    //glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0);
	/* Calculate current and next frames */
   // animate(&md5animation, &animInfo, deltaTime);

	/* Interpolate skeletons between two frames */
    /*interpolate_skeletons(md5animation.skelFrames[animInfo.curr_frame],
                          md5animation.skelFrames[animInfo.next_frame],
                          md5animation.num_joints,
                          animInfo.last_time * md5animation.frameRate,
                          skeleton);
*/
	if( displaySkeleton ) {
        draw_skeleton(skeleton, md5model.num_joints);
	}

	/* Draw each mesh of the model */
	for( int i = 0; i < md5model.num_meshes; ++i ) {
			md5_mesh_t mesh = md5model.meshes[i];
        prepare_mesh(&mesh, skeleton);

			if( displayWireframe )
				glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

			if( displayMesh )
                draw_mesh(&mesh);
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

	Lab08BlackMagic::printOpenGLInfo();
	
	// hidden Lab08 Black Magic to set up additional OpenGL3 settings
	// specifically, compiles a shader for our renderer to use
	Lab08BlackMagic::setupOpenGL3Stuff();

	setupTextures();									// load all textures into memory
	setupBuffers();										// load all our VAOs and VBOs into memory
	loadMD5Model();	// load the MD5 Model & animation if provided

	convertSphericalToCartesian();		// set up our camera position

	// used to keep track of the time between frames rendered
	double current_time = 0, last_time = 0;

	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
		last_time = current_time;				// time the last frame was rendered
		current_time = glfwGetTime();		// time this frame is starting at

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
		renderScene( viewMatrix, projectionMatrix, current_time - last_time );

		glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				// check for any events and signal to redraw screen
	}

	glfwDestroyWindow( window );// clean up and close our window
	glfwTerminate();						// shut down GLFW to clean up our context

	// free up memory created by Lab08 Black Magic
	Lab08BlackMagic::cleanUpOpenGL3Stuff();

	// free up memory used by the MD5 model
    free_vertex_arrays();
    free_anim(&md5animation);

	// TODO #12B: delete our platform buffer
	glDeleteBuffers(1,vbods);
    glDeleteBuffers(1,vaods);
	return EXIT_SUCCESS;				// exit our program successfully!
}
