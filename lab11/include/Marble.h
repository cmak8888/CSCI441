#ifndef _MARBLE_H_
#define _MARBLE_H_ 1

#include <glm/glm.hpp>

#include <GL/glew.h>

class Marble {
public:

	// CONSTRUCTORS / DESTRUCTORS
  Marble();
  Marble( glm::vec3 loc, glm::vec3 dir, double r );

	// MISCELLANEOUS
  glm::vec3 location;
  glm::vec3 direction;

  void draw( glm::mat4 modelMtx, GLint uniform_modelMtx_loc, GLint uniform_color_loc ) const;
  void moveForward();
  void moveBackward();
  double getRadius() const;

private:
  double _radius;
  double _rotation;
  glm::vec4 _color;
};

#endif	// _MARBLE_H_
