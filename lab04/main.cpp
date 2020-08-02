/*
 *  CSCI 441, Computer Graphics, Fall 2019
 *
 *  Project: lab04
 *  File: main.cpp
 *
 *	Author: Calvin Mak
 *
 *  Description:
 *      Contains the base code for Flight Simulator v0.33 alpha w/ lighting!
 *
 */

// HEADERS /////////////////////////////////////////////////////////////////////

// include the OpenGL library header
#ifdef __APPLE__				    // if compiling on Mac OS
#include <OpenGL/gl.h>
#else							    // if compiling on Linux or Windows OS
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>			    // include GLFW framework header

#include <CSCI441/objects.hpp>      // for our 3D objects
#include <CSCI441/OpenGLUtils.hpp>  // for OpenGL helper functions

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>                   // for powf functionality
#include <stdio.h>			        // for printf functionality
#include <stdlib.h>			        // for exit functionality
#include <time.h>			        // for time() functionality

//*************************************************************************************
//
// Global Parameters

// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 640, windowHeight = 480;

int leftMouseButton;    	 		// status of the mouse button
bool keysDown[256] = {0};           // status of our keys
bool controlDown;                   // status of control key
glm::vec2 mousePosition;			// last known X and Y of the mouse

glm::vec3 camPos;            		// camera position in cartesian coordinates
glm::vec3 camAngles;               	// camera DIRECTION in spherical coordinates stored as (theta, phi, radius)
glm::vec3 camDir; 			        // camera DIRECTION in cartesian coordinates

GLfloat propAngle = 0.0f;           // angle of rotation for our plane propeller

GLuint environmentDL;               // display list for the 'city'

bool light0Toggle = true;
bool light1Toggle = true;

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
                glfwSetWindowShouldClose( window, GLFW_TRUE );
                break;
            case GLFW_KEY_1:
                if(light0Toggle) {
                    light0Toggle = false;
                    glDisable(GL_LIGHT0);
                } else {
                    light0Toggle = true;
                    glEnable(GL_LIGHT0);
                }
                break;
            case GLFW_KEY_2:
                if(light1Toggle) {
                    light1Toggle = false;
                    glDisable(GL_LIGHT1);
                } else {
                    light1Toggle = true;
                    glEnable(GL_LIGHT1);
                }
            default:
                keysDown[key] = true;
        }
    } else if( action == GLFW_RELEASE ) {
        keysDown[key] = false;
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
            //Zooming
            camAngles.z += ((x - mousePosition.x) + (y - mousePosition.y)) * 0.01;
            if( camAngles.z < -2.5 ) {
                camAngles.z = -2.5f;
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

void animate() {
    //because the direction vector is unit length, and we probably don't want
    //to move one full unit every time a button is pressed, just create a constant
    //to keep track of how far we want to move at each step. you could make
    //this change w.r.t. the amount of time the button's held down for
    //simple scale-sensitive movement!
    const float movementConstant = 0.3f;

    if( keysDown[ GLFW_KEY_SPACE ] ) {
        //just move FORWARDS along the direction.
        camPos.x += camDir.x * movementConstant;
        camPos.y += camDir.y * movementConstant;
        camPos.z += camDir.z * movementConstant;

        propAngle += M_PI / 16.0f;
        if( propAngle > 2 * M_PI ) propAngle -= 2 * M_PI;

    }

    if( keysDown[ GLFW_KEY_S ] ) {
        camAngles.y += 0.01;

        // make sure that phi stays within the range (0, M_PI)
        if(camAngles.y <= 0)
            camAngles.y = 0+0.001;
        if(camAngles.y >= M_PI)
            camAngles.y = M_PI-0.001;

        recomputeOrientation();     //update camera (x,y,z) based on (radius,theta,phi)
    }

    if( keysDown[ GLFW_KEY_W ] ) {
        camAngles.y -= 0.01;

        // make sure that phi stays within the range (0, M_PI)
        if(camAngles.y <= 0)
            camAngles.y = 0+0.001;
        if(camAngles.y >= M_PI)
            camAngles.y = M_PI-0.001;

        recomputeOrientation();     //update camera (x,y,z) based on (radius,theta,phi)
    }

    if( keysDown[ GLFW_KEY_D ] ) {
        // turn right
        camAngles.x += 0.01;
        recomputeOrientation();     //update camera (x,y,z) based on (radius,theta,phi)
    }

    if( keysDown[ GLFW_KEY_A ] ) {
        // turn left
        camAngles.x -= 0.01;
        recomputeOrientation();     //update camera (x,y,z) based on (radius,theta,phi)
    }
}

// generateEnvironmentDL() /////////////////////////////////////////////////////
//
//  This function creates a display list with the code to draw a simple
//      environment for the user to navigate through.
//
//  And yes, it uses a global variable for the display list.
//  I know, I know! Kids: don't try this at home. There's something to be said
//      for object-oriented programming after all.
//
////////////////////////////////////////////////////////////////////////////////
void generateEnvironmentDL() {
    int gridX = 100;
    int gridY = 100;
    float spacing = 0.5f;

    environmentDL = glGenLists(1);
    glNewList(environmentDL, GL_COMPILE); {
        //psych! everything's on a grid.
        for(int i = 0; i < gridX-1; i++) {
            for(int j = 0; j < gridY-1; j++) {
                //don't just draw a building ANYWHERE.
                if( i % 2 && j % 2 && getRand() < 0.4f ) {
                    float height = getRand();
                    height = powf(height, 2.5)*10 + 1;

                    // TODO #02: Add a random material color
                    GLfloat matColorD[4] = {getRand(), getRand(), getRand(), 1.0f};
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matColorD);
                    glm::mat4 transToPosMtx = glm::translate( glm::mat4(1.0f), glm::vec3( (i - gridX/2.0f)*1.1, 0.0f, (j - gridY/2.0f)*1.1 ) ) ;
                    glm::mat4 transToHeightMtx = glm::translate( transToPosMtx, glm::vec3( 0.0f, height/2.0f, 0.0f ) ) ;
                    glm::mat4 scaleHeightMtx = glm::scale( transToHeightMtx, glm::vec3( 1.0f, height, 1.0f ) );
                    CSCI441::pushMatrix( scaleHeightMtx ); {
                        CSCI441::drawSolidCube(1.0);
                    }; CSCI441::popMatrix( scaleHeightMtx );
                }
            }
        }

        // DRAW OUR GRID
        // TODO #07: convert to materials and set vertex attributes properly
        GLfloat newColorMaterial[4] = {0.07568, 0.61424, 0.07568, 1.0};
        glNormal3f( 0,1.0f,0 );
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, newColorMaterial);
        glBegin( GL_TRIANGLE_STRIP ); {              // TODO #04: Make this into a quad
            glNormal3f( 0,1.0f,0 );
            glVertex3f( -gridX, 0, -gridY );
            glNormal3f( 0,1.0f,0 );
            glVertex3f( -gridX, 0, gridY );
            glNormal3f( 0,1.0f,0 );
            glVertex3f( gridX, 0, -gridY );
            glNormal3f( 0,1.0f,0 );
            glVertex3f( gridX, 0, gridY );
        }; glEnd();


        // TODO #03: Make our teapot cool
        GLfloat matColorTD[4] = {0.18275,0.17,0.22525,1.0f};                       // make it RED!
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matColorTD);
        glm::mat4 transToHeightMtx = glm::translate( glm::mat4(1.0f), glm::vec3( 0.0f, 1.0f, 0.0f ) ) ;
        CSCI441::pushMatrix( transToHeightMtx ); {
            glShadeModel( GL_SMOOTH );
            CSCI441::drawSolidTeapot( 2.0 );         // draw our teapot
            glShadeModel( GL_FLAT );
        }; CSCI441::popMatrix( transToHeightMtx );

    }; glEndList();
}

void drawPlaneBody() {
    glColor3f( 0, 0, 1 );

    glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0f), glm::vec3( 2.0f, 0.5f, 1.0f ) );
    CSCI441::pushMatrix( scaleMtx ); {
        CSCI441::drawSolidCube( 0.1 );
    }; CSCI441::popMatrix( scaleMtx );
}

void drawPlaneWing( bool leftWing ) {
    glColor3f( 1, 0, 0 );
    glm::mat4 scaleMtx = glm::scale( glm::mat4(1.0f), glm::vec3( 1.5f, 0.5f, 1.0f ) );

    glm::mat4 rsMtx;
    if( leftWing )
        rsMtx = glm::rotate( scaleMtx, -1.57f, CSCI441::X_AXIS );
    else
        rsMtx = glm::rotate( scaleMtx, 1.57f, CSCI441::X_AXIS );

    CSCI441::pushMatrix( rsMtx ); {
        CSCI441::drawSolidCone( 0.05, 0.2, 16, 16 );
    }; CSCI441::popMatrix( rsMtx );
}

void drawPlaneNose() {
    glColor3f( 0, 1, 0 );
    glm::mat4 rotateMtx = glm::rotate( glm::mat4(1.0f), 1.57f, CSCI441::Z_AXIS );
    CSCI441::pushMatrix( rotateMtx ); {
        CSCI441::drawSolidCone( 0.025, 0.3, 16, 16 );

        // move the origin to the front of our nose cone
        glm::mat4 transMtx = glm::translate( glm::mat4(1.0f), glm::vec3( 0.0f, 0.0f, 0.2f ) );
        CSCI441::pushMatrix( transMtx ); {
            // TODO #09: Place our second light here
            GLfloat light1Pos[4] =  {0,0,0,1.0f};
            glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
            GLfloat dir[4] = {0,1.0f,0,0};
            glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, dir);
        }; CSCI441::popMatrix( transMtx );
    }; CSCI441::popMatrix( rotateMtx );
}

void drawPlanePropeller() {
    glColor3f( 1, 1, 1 );
    glm::mat4 tMtx = glm::translate( glm::mat4(1.0f), glm::vec3( 0.1f, 0.0f, 0.0f ) );
    glm::mat4 rtMtx = glm::rotate( tMtx, propAngle, CSCI441::X_AXIS );
    glm::mat4 srtMtx = glm::scale( rtMtx, glm::vec3( 1.1, 1, 0.025 ) );
    CSCI441::pushMatrix( srtMtx ); {
        CSCI441::drawSolidCube( 0.1 );
    }; CSCI441::popMatrix( srtMtx );

    rtMtx = glm::rotate( tMtx, propAngle+1.57f, CSCI441::X_AXIS );
    srtMtx = glm::scale( rtMtx, glm::vec3( 1.1, 1, 0.025 ) );
    CSCI441::pushMatrix( srtMtx ); {
        CSCI441::drawSolidCube( 0.1 );
    }; CSCI441::popMatrix( srtMtx );
}

void drawPlaneTail() {
    glColor3f( 1, 1, 0 );
    CSCI441::drawSolidCone( 0.02, 0.1, 16, 16 );
}

// drawPlane() //////////////////////////////////////////////////////////////////
//
//  A very CRUDE plane
//
////////////////////////////////////////////////////////////////////////////////
void drawPlane() {
    glm::mat4 rotYMtx = glm::rotate( glm::mat4(1.0f), -1.57f, CSCI441::Y_AXIS );
    glm::mat4 rotZYMtx = glm::rotate( rotYMtx, 1.57f, CSCI441::Z_AXIS );
    CSCI441::pushMatrix( rotZYMtx ); {
        drawPlaneBody();        // the body of our plane
        drawPlaneWing( true );  // the left wing
        drawPlaneWing( false ); // the right wing
        drawPlaneNose();        // the nose
        drawPlanePropeller();   // the propeller
        drawPlaneTail();        // the tail
    }; CSCI441::popMatrix( rotZYMtx );
}

// renderScene() ///////////////////////////////////////////////////////////////
//
//  GLUT callback for scene rendering. Sets up the modelview matrix, renders
//      a scene to the back buffer, and switches the back buffer with the
//      front buffer (what the user sees).
//
////////////////////////////////////////////////////////////////////////////////
void renderScene(void)  {
    // execute our display list
    glCallList( environmentDL );

    // we are going to cheat and use our look at point to place our plane
    glm::mat4 planeTransMtx = glm::translate( glm::mat4(1.0f), camPos+camDir );
    // rotate the plane with our camera theta direction (we need to rotate the opposite direction so we always look at the back)
    glm::mat4 planeThetaMtx = glm::rotate( planeTransMtx, -camAngles.x, CSCI441::Y_AXIS );
    // rotate the plane with our camera phi direction
    glm::mat4 planePhiMtx = glm::rotate( planeThetaMtx,  camAngles.y, CSCI441::X_AXIS );
    CSCI441::pushMatrix( planePhiMtx ); {
        // draw our plane now
        drawPlane();
    }; CSCI441::popMatrix( planePhiMtx );
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
    GLFWwindow *window = glfwCreateWindow( windowWidth, windowHeight, "Lab04 - Flight Simulator v0.33 alpha", NULL, NULL );
    if( !window ) {						                // if the window could not be created, NULL is returned
        fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    } else {
        fprintf( stdout, "[INFO]: GLFW Window created\n" );
    }

    glfwMakeContextCurrent(window);		                // make the created window the current window
    glfwSwapInterval(1);				     	        // update our screen after at least 1 screen refresh

    glfwSetKeyCallback( window, keyboard_callback );	            // set our keyboard callback function
    glfwSetCursorPosCallback( window, cursor_callback );            // set our cursor position callback function
    glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function

    return window;						                // return the window that was created
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
    glEnable( GL_DEPTH_TEST );          // enable depth testing for hidden surface removal

    glEnable( GL_CULL_FACE );           // enable back face culling to speed render times
    glFrontFace( GL_CCW );              // denote front faces specified by counter-clockwise winding order
    glCullFace( GL_BACK );              // cull our back faces

    glShadeModel( GL_FLAT );            // use flat shading

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black

    glEnable( GL_LIGHTING );            // we are now using lighting
    glEnable( GL_LIGHT0 );              // and turn on Light 0 please (and thank you)
    float diffuseCol[4] = { 0.75164,0.60648,0.22648,0.2 };           // TODO #06: Change the colors!
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseCol );
    float ambientCol[4] = { 0.0, 0.0, 0.0, 1.0 };
    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientCol );

    // TODO #08: Set up the next light
    glEnable( GL_LIGHT1 );              // and turn on Light 0 please (and thank you)
    float diffuseColor[4] = { 1.0f,1.0f,1.0f,0.2 };           // TODO #06: Change the colors!
    glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuseColor );
    glLightf( GL_LIGHT1, GL_SPOT_CUTOFF, 12.0f );
}

//
//	setupScene()
//
//	setup everything specific to our scene.  in this case,
//	position our camera
//
void setupScene() {
    // give the camera a scenic starting point.
    camPos.x = 60;
    camPos.y = 40;
    camPos.z = 30;
    camAngles.x = -M_PI / 3.0f;	// theta
    camAngles.y = M_PI / 2.8f;	// phi
    camAngles.z = 0;            // radius
    recomputeOrientation();

    srand( time(NULL) );        // seed our random number generator
    generateEnvironmentDL();    // create our environment display list
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
    GLFWwindow *window = setupGLFW();	        // initialize all of the GLFW specific information releated to OpenGL and our window
    setupOpenGL();								// initialize all of the OpenGL specific information
    setupScene();								// initialize objects in our scene
    CSCI441::OpenGLUtils::printOpenGLInfo();    // print information related to our OpenGL context

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
        glm::mat4 projMtx = glm::perspective( 45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.001f, 10000.0f );
        glMatrixMode( GL_PROJECTION );	        // change to the Projection matrix
        glLoadIdentity();				        // set the matrix to be the identity
        glMultMatrixf( &projMtx[0][0] );        // load our orthographic projection matrix into OpenGL's projection matrix state

        // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
        // when using a Retina display the actual window can be larger than the requested window.  Therefore
        // query what the actual size of the window we are rendering to is.
        GLint framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

        // update the viewport - tell OpenGL we want to render to the whole window
        glViewport( 0, 0, framebufferWidth, framebufferHeight );

        glMatrixMode( GL_MODELVIEW );	        // make the ModelView matrix current to be modified by any transformations
        glLoadIdentity();						// set the matrix to be the identity

        // set up our look at matrix to position our camera
        glm::mat4 viewMtx = glm::lookAt( camPos + camDir * camAngles.z,
                                         camPos + camDir,
                                         glm::vec3(  0,  1,  0 ) );
        // multiply by the look at matrix - this is the same as our view martix
        glMultMatrixf( &viewMtx[0][0] );
        float lPosition[4] = { 0.0, 10.0, 0.0, 1.0 };           // TODO #05: MOVE THIS to where?
        glNormal3f(0,1.0f,0);
        glLightfv( GL_LIGHT0, GL_POSITION, lPosition );

        renderScene();					        // draw everything to the window

        glfwSwapBuffers(window);                // flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();				        // check for any events and signal to redraw screen

        animate();
    }

    glfwDestroyWindow( window );                // clean up and close our window
    glfwTerminate();						    // shut down GLFW to clean up our context

    return EXIT_SUCCESS;				        // exit our program successfully!
}
