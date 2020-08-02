#ifndef __CSCI441_LAB08_BLACKMAGE_H___
#define __CSCI441_LAB08_BLACKMAGE_H___

#include <GL/glew.h>

#include <glm/glm.hpp>

#include "MD5/md5model.h"

static const GLint vertexPositionLocation = 0, vertexTexCoordLocation = 1;

namespace Lab08BlackMagic {
  void setupOpenGL3Stuff();
  void printOpenGLInfo();
  void drawScene( glm::mat4 projMtx, glm::mat4 viewMtx );
  void setModelMatrix();
  void cleanUpOpenGL3Stuff();

  void drawSkeleton( const md5_joint_t *skeleton, int num_joints );
}

#endif // __CSCI441_LAB08_BLACKMAGE_H___
