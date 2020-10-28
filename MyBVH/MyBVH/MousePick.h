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
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include "gtx/intersect.hpp"
#include <vector>

class MousePick
{
public:

  MousePick(std::vector<Cartesian3> *targetPoints, float size);

  int click(float x, float y);

  glm::vec2 drag(float x, float y);

  bool dragging;
  int closest;
  glm::vec2 start;
  float size;

  std::vector<Cartesian3> *targetPoints;

};

#endif
