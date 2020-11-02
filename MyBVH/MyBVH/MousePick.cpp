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
int MousePick::click(float x, float y, Camera *camera)
{
  // Get orthographic matrix
  float ortho[16];
  glGetFloatv(GL_PROJECTION_MATRIX, ortho);
  glm::mat4 proj = glm::make_mat4(ortho);

  // get modelview matrix
  float modelview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
  glm::mat4 modelView = glm::make_mat4(modelview);

  // ray starts from cameras position
  glm::vec3 rayStart = camera->Position;

  // ray points where camera is
  glm::vec3 rayDirection = camera->Front; // towards positive Z

  float radiusSquared = .5;

  glm::vec3 intersectPosition;
  glm::vec3 intersectNormal;
  glm::vec4 sphereCenter4;
  glm::vec3 sphereCenter;
  bool intersect = false;
  float closestDist = 99999.;

  // reset
  dragging = false;
  closest = -1;

  //
  // std::cout << "Proj" << '\n';
  // for(int i = 0; i < 4; i ++)
  // {
  //   for(int j = 0; j < 4; j ++)
  //   {
  //     std::cout << proj[i][j] << ' ';
  //   }
  //   std::cout << " " << '\n';
  // }
  //
  // std::cout << "modelView" << '\n';
  // for(int i = 0; i < 4; i ++)
  // {
  //   for(int j = 0; j < 4; j ++)
  //   {
  //     std::cout << modelView[i][j] << ' ';
  //   }
  //   std::cout << " " << '\n';
  // }
  //
  //

  std::cout << "x: " << x << '\n';
  std::cout << "y: " << y << '\n';
  std::cout << camera->Zoom << '\n';

  // std::cout << "Ray o: ";
  // for(int i = 0; i < 3; i++){ std::cout << rayStart[i] << ' ';}
  // std::cout << "" << '\n';
  // std::cout << "Ray Dir: ";
  // for(int i = 0; i < 3; i++){ std::cout << rayDirection[i] << ' ';}
  // std::cout << "" << '\n';

  //
  // std::cout << "First Target" << '\n';
  // std::cout << targetPoints[0][0] << '\n';


  // Test every point to see which is the closest
  for(unsigned int i = 0; i < 1; i++)
  {

    //std::cout << "Target: " << targetPoints[0][i] << " " << '\n';

    sphereCenter4 = glm::vec4(targetPoints[0][i].x,
                              targetPoints[0][i].y,
                              targetPoints[0][i].z,
                              1.0);



    // calculate point with model view matrix
    //sphereCenter4 = sphereCenter4 * glm::inverse(modelView);
    sphereCenter = glm::vec3(sphereCenter4.x, sphereCenter4.y, sphereCenter4.z);


    // std::cout << "Target" << '\n';
    // for(int i = 0; i < 4; i++){ std::cout << sphereCenter4[i] << ' ';}
    // std::cout << "" << '\n';
    // std::cout << " " << '\n';


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
      std::cout << "Intersect" << '\n';
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
