#include "../include/Marble.h"

#include <glm/gtc/matrix_transform.hpp>

#include <CSCI441/objects3.hpp>

#include <math.h>
#include <stdlib.h>

Marble::Marble() {
    _radius = 0.5;
    location = glm::vec3(0,0,0);
    direction = glm::vec3(1,0,0);
    _rotation = 0;
    _color = glm::vec4(	rand() % 50 / 100.0 + 0.5, 
						rand() % 50 / 100.0 + 0.5, 
						rand() % 50 / 100.0 + 0.5,
						1.0f );
}

Marble::Marble( glm::vec3 l, glm::vec3 d, double r ) : location(l), direction(d), _radius(r) {
    _rotation = 0;
    _color = glm::vec4(rand() % 50 / 100.0 + 0.5, 
					   rand() % 50 / 100.0 + 0.5, 
					   rand() % 50 / 100.0 + 0.5,
					   1.0f);
}

void Marble::draw( glm::mat4 modelMtx, GLint uniform_modelMtx_loc, GLint uniform_color_loc ) const {

    glm::vec3 directionNormalized = glm::normalize( direction );
    glm::vec3 rotationAxis = glm::cross( directionNormalized, glm::vec3(0,1,0) );

    modelMtx = glm::translate( modelMtx, location );
    modelMtx = glm::translate( modelMtx, glm::vec3( 0, _radius, 0 ) );
    modelMtx = glm::rotate( modelMtx, (float)_rotation, rotationAxis );
    glUniformMatrix4fv( uniform_modelMtx_loc, 1, GL_FALSE, &modelMtx[0][0] );

    glUniform4fv( uniform_color_loc, 1, &_color[0] );

    CSCI441::drawSolidSphere( _radius, 16, 16 );
}

void Marble::moveForward() {
    location += direction*0.1f;
    _rotation -= 0.1;
    if( _rotation < 0 ) {
        _rotation += 6.28;
    }
}

void Marble::moveBackward() {
    location -= direction*0.1f;
    _rotation += 0.1;
    if( _rotation > 6.28 ) {
        _rotation -= 6.28;
    }
}

double Marble::getRadius() const {
	return _radius;
}