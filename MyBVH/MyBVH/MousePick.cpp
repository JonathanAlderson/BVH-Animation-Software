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

MousePick::MousePick(std::vector<double> *targetPoints, float size)
{
  this->targetPoints = targetPoints;
  dragging = false;
  closest = -1;
  start = glm::vec3(0., 0., 0.);
  this->size = size;
}

glm::vec3 MousePick::drag(float x, float y, Camera *camera)
{
  // make homogenius and work out the actual effect in world space
  glm::vec4 curr = glm::vec4(x, y, 0., 1.);

  curr = curr * glm::inverse(camera->GetViewMatrix());

  glm::vec3 curr3 = glm::vec3(curr.x, curr.y, curr.z);

  glm::vec3 change = start - curr3;
  start = curr3;

  return change;
}

// Finds the point that has been clicked
int MousePick::click(float x, float y, Camera *camera)
{

  // Find projection matrix
  float projM[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projM);
  glm::mat4 proj = glm::make_mat4(projM);


  // ray starts from cameras position
  glm::vec3 rayStart = camera->Position;


  // take into account where the mouse is looking
  // ray direction based on where mouse is looking
  // also depends on viewport aspect ratio and scale
  glm::vec3 rayDirection = glm::vec3(x / proj[0][0], y / proj[1][1], -1.);
  rayDirection = glm::normalize(rayDirection);

  // hitbox size
  float radiusSquared = .3;

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
  for(unsigned int i = 0; i < (targetPoints->size() / 3); i++)
  {
    sphereCenter4 = glm::vec4(targetPoints[0][3 * i]     - rayStart.x,
                              targetPoints[0][3 * i + 1] - rayStart.y,
                              targetPoints[0][3 * i + 2] - rayStart.z,
                              1.0);

    // calculate point with model view matrix
    sphereCenter4 = sphereCenter4 * glm::inverse(camera->GetViewMatrix());

    // readjust for the position
    sphereCenter = glm::vec3(sphereCenter4.x + rayStart.x,
                             sphereCenter4.y + rayStart.y,
                             sphereCenter4.z + rayStart.z);

    // calculate the intersection
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
      std::cout << "" << '\n';
      dragging = true;
      start = glm::vec3(x, y, 0.);
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
