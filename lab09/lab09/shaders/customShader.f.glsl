/*
 *   Fragment Shader
 *
 *   CSCI 441, Computer Graphics, Colorado School of Mines
 */

#version 330 core

// TODO #F3
in vec3 theColor;
// TODO #D
out vec4 fragColorOut;
void main() {

    //*****************************************
    //******* Final Color Calculations ********
    //*****************************************

    // TODO #E
    fragColorOut = vec4(1.0,1.0,1.0,1.0);
    // TODO #F4
    fragColorOut = vec4(theColor, 1);
}
