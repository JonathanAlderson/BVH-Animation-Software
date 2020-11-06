///////////////////////////////////////////////////
//
//	Jonathan Alderson
//	October, 2020
//
//	------------------------
//	BVH.h
//	------------------------
//
//	Capable of loading, rendering and updating a BVH model
//
///////////////////////////////////////////////////

#ifndef  _BVH_H_
#define  _BVH_H_

using namespace std;

#include <vector>
#include <map>
#include <string>
#include <iomanip>
#include "Cartesian3.h"
#include "camera.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"

class BVH
{
// constructors and destructors
public:
    ~BVH();   // destructor

    BVH();    // empty constructor

    BVH( const char * bvhFileName ); // filename constructor

    // setup functions

    void Clear();

    void Load( const char * bvhFileName );

    // Used at setup to find a bounding box
    void FindMinMax();


// enum and struct declarations
public:

  // Each float in file is one of these 6
  enum  ChannelEnum
  {
    X_ROTATION, Y_ROTATION, Z_ROTATION,
    X_POSITION, Y_POSITION, Z_POSITION
  };

  // For Using Either IK Or Rotations
  enum MoveEnum {
      ROTATE,
      INVERSEKINEMATICS
  };

  // Forward Declaration of Joint
  struct Joint;

  // Each joint has usually 6 channels, which
  // are each related to a Joint and have a type
  struct Channel
  {
    // parent
    Joint * joint;
    // which emun this Channel is
    ChannelEnum type;
    // index
    int index;
  };

  // Actual Declaration of a Joint
  struct Joint
  {
    // e.g Spine
    string name;
    // index
    int index;
    // parent to this Joint
    Joint * parent;
    // all children ( can be multiple )
    vector< Joint *> children;
    // 3D relative position to parent
    double offset[3];
    // lets us know the recursion is going to end
    bool hasSite;
    // length of preceeding segment
    double site[3];
    // how many channels of information this Joint has (for reading)
    vector< Channel * > channels;
    // used for calculating the global position
    glm::mat4 localMatrix;
    // where the data starts for position editing
    double *dataStart;

  };

public:
  // all our public variables and funcitons
  // that our GUI and users can call

  // used for error messages
  bool isLoadSuccess;

  // filename and motion name from file
  string fileName;
  string motionName;

  // main storage
  int numChannel;
  vector< Channel * > channels;
  vector < Joint * > joints;
  map< string, Joint *> jointIndex;

  // Over the whole animation,
  // how the ROOT node moves
  Cartesian3 minCoords;
  Cartesian3 maxCoords;
  float boundingBoxSize;

  // for mouse interaction
  vector<double> globalPositions;

  // for inverse kinematics
  vector<double> jointAngles;


  // which item the user is controlling
  int activeJoint;

  // for playback
  int numFrame;
  double interval;
  double * motion;

public:

  // calculates the new global positions at every frame
  void FindGlobalPosition(Joint *joint, Camera *camera);

  // Rendering Functions

  // Initial Call For Rendering a Figure
  void RenderFigure(int frameNo, float scale, Camera *camera);

  // Render Figure Function Made to be called Recursively
	void  RenderFigure(Joint * root, double * data, float scale, Camera *camera);

  // Expect a lot of calls to this one
  void RenderBone(float x0, float y0, float z0, float x1, float y1, float z1, float bRadius = 0.1 );

  // Renders the points where a user can click
  void RenderControlPoints();

  // moves a specific joint with inverse kinematics
  void MoveJoint(int id, glm::vec3 move, int mode);

};

#endif
