///////////////////////////////////////////////////
//
//	Jonathan Alderson
//	October, 2020
//
//	------------------------
//	MousePick.h
//	------------------------
//
//  Class to get mouse position in world space from screen space
//
///////////////////////////////////////////////////

#include "MousePick.h"
#include <math.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "MousePick.h"

MousePick::MousePick(std::vector<Cartesian3> *targetPoints, float size)
{
  this->targetPoints = targetPoints;
  dragging = false;
  closest = -1;
  start = glm::vec2(0., 0.);
  this->size = size;
}

glm::vec2 MousePick::drag(float x, float y)
{
  glm::vec2 curr = glm::vec2(x, y);

  glm::vec2 change = start - curr;
  start = curr;

  return change;
}

// Finds the point that has been clicked
int MousePick::click(float x, float y)
{
  // Get orthographic matrix
  float ortho[16];
  glGetFloatv(GL_PROJECTION_MATRIX, ortho);
  glm::mat4 proj = glm::make_mat4(ortho);

  // get modelview matrix
  float modelview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
  glm::mat4 modelView = glm::make_mat4(modelview);

  // set ray start to mouse pos
  glm::vec4 rayStart4 = glm::vec4(x, y, -100., 1.);
  //rayStart4 = rayStart4 * glm::inverse(proj);

  float div = std::max(ortho[0], ortho[5]);

  rayStart4.x /= div;
  rayStart4.y /= div;

  // why can't cpp have swizzling? :(
  glm::vec3 rayStart = glm::vec3(rayStart4.x, rayStart4.y, rayStart4.z);

  // view ray pointing in positive Z axis
  glm::vec3 rayDirection = glm::vec3(0., 0., 1.); // towards positive Z

  float radiusSquared = .05 * size;

  glm::vec3 intersectPosition;
  glm::vec3 intersectNormal;
  glm::vec4 sphereCenter4;
  glm::vec3 sphereCenter;
  bool intersect = false;
  float closestDist = 99999.;

  // reset
  dragging = false;
  closest = -1;

  // Test every point to see which is the closest
  for(unsigned int i = 0; i < targetPoints->size(); i++)
  {
    sphereCenter4 = glm::vec4(targetPoints[0][i].x,
                              targetPoints[0][i].y,
                              targetPoints[0][i].z,
                              1.0);
    // calculate point with model view matrix
    sphereCenter4 = sphereCenter4 * glm::inverse(modelView);
    sphereCenter = glm::vec3(sphereCenter4.x, sphereCenter4.y, sphereCenter4.z);

    intersect = glm::intersectRaySphere(rayStart,
                                        rayDirection,
                                        sphereCenter,
                                        radiusSquared,
                                        intersectPosition,
                                        intersectNormal
                                       );

    // If a point has intersected
    if(intersect == true)
    {
      dragging = true;
      start = glm::vec2(x, y);
      // keep track of which point is closest to the camera
      if(sphereCenter4.z < closestDist)
      {
        closest = i;
        closestDist = sphereCenter4.z;
      }
    }
  }
  return closest;
}
