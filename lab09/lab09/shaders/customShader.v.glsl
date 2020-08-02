/*
 *   Vertex Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// TODO #B
in vec3 vPosition;
// TODO #F1
out vec3 theTexColor;

// TODO #A
uniform mat4 mvpMatrix;
// TODO #G1
uniform float time;

void main() {
    //*****************************************
    //********* Vertex Calculations  **********
    //*****************************************

    // TODO #G2
    vec3 newVertex = vPosition;
    if(vPosition.x > 0 && vPosition.y > 0 && vPosition.z > 0) {
        newVertex = vPosition + 1.2*(sin(time) + 1)/2 - 0.2;
    }

    // TODO #C

     gl_Position = mvpMatrix * vec4(newVertex,1);


    // TODO #F2
    theTexColor = vPosition;
}
