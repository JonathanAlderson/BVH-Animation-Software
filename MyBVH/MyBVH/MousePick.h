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

#ifndef MOUSEPICK_H
#define MOUSEPICK_H

#include "Cartesian3.h"
#include "camera.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include "gtx/intersect.hpp"
#include <vector>

class MousePick
{
public:

  MousePick(std::vector<double> *targetPoints, float size);

  int click(float x, float y, Camera *camera);

  glm::vec3 drag(float x, float y, Camera *camera);

  bool dragging;
  int closest;
  glm::vec3 start;
  float size;

  std::vector<double> *targetPoints;

};

#endif
