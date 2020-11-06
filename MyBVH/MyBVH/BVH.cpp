///////////////////////////////////////////////////
//
//	Jonathan Alderson
//	October, 2020
//
//	------------------------
//	BVH.cpp
//	------------------------
//
//	Capable of loading, rendering and updating a BVH model
//
///////////////////////////////////////////////////

#include <fstream>
#include <cstring>
#include <string>
#include <math.h>
#include <iostream>
#include "BVH.h"

// openGL includes
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif


////////////////////////////////////////////////
// CONSTRUCTORS
////////////////////////////////////////////////

// Destructor
BVH::~BVH()
{
  Clear();
}
// Empty Constructor
BVH::BVH()
{
	motion = NULL;
	Clear();
  activeJoint = -1;
}

// Standard Constructor With File Name
BVH::BVH( const char * bvhFileName )
{
	motion = NULL;
	Clear();
	Load( bvhFileName );
  activeJoint = -1;
}

// Called before we call a new model
void  BVH::Clear()
{
	unsigned int  i;
	for ( i=0; i < channels.size(); i++ )
		delete  channels[ i ];
	for ( i=0; i < joints.size(); i++ )
		delete  joints[ i ];
	if ( motion != NULL )
		delete  motion;

	isLoadSuccess = false;

	fileName = "";
	motionName = "";

	numChannel = 0;
	channels.clear();
	joints.clear();
	jointIndex.clear();

	numFrame = 0;
	interval = 0.0;
	motion = NULL;
}

////////////////////////////////////////////////
// FILE LOADING
// Huge Function To Load File
// Very glad I didn't have to write this
////////////////////////////////////////////////
void  BVH::Load( const char * bvhFileName )
{
	#define  BUFFER_LENGTH  1024*4

	ifstream  file;
	char      line[ BUFFER_LENGTH ];
	char *    token;
	char      separater[] = " :,\t";
	vector< Joint * >   jointStack; // how a good night starts
	Joint *   joint = NULL;
	Joint *   newJoint = NULL;
	bool      isSite = false;
	double    x, y ,z;
	unsigned int       i, j;

	// Just make sure everything is reset
	Clear();


	// finds the motion name from the file name
  // with a bunch of char * operations
	fileName = bvhFileName;
	const char *  mnFirst = bvhFileName;
	const char *  mnLast = bvhFileName + strlen( bvhFileName );

	if ( strrchr( bvhFileName, '\\' ) != NULL )
		mnFirst = strrchr( bvhFileName, '\\' ) + 1;

	else if ( strrchr( bvhFileName, '/' ) != NULL )
		mnFirst = strrchr( bvhFileName, '/' ) + 1;

	if ( strrchr( bvhFileName, '.' ) != NULL )
		mnLast = strrchr( bvhFileName, '.' );

	if ( mnLast < mnFirst )
		mnLast = bvhFileName + strlen( bvhFileName );

	motionName.assign( mnFirst, mnLast );

	// Read in from buffer
	file.open( bvhFileName, ios::in );
	if ( file.is_open() == 0 )  return; // can't be opened for whatever reason

  ///////////////////////////////////
	// HEIRARCHY READ IN
  ///////////////////////////////////
	while ( ! file.eof() )
	{

		// Stream closed while reading
		if ( file.eof() )  goto bvh_error;

		// Read a bit more data
		file.getline( line, BUFFER_LENGTH );
		token = strtok( line, separater );


    // Remove Carridge Returns to Avoid Errors
    if((int)token[strlen(token) -1] == 13)
    {
      token[strlen(token) - 1] = '\0';
    }

		// Empty Space or Null
		if ( token == NULL ){ continue; };

		// New JOINT discovered
		if ( strcmp( token, "{" ) == 0)
		{
			// add to class joints
			jointStack.push_back( joint );
			joint = newJoint;


			continue;
		}

		// Now we are talking about the parent again
		if ( strcmp( token, "}" ) == 0 )
		{
			// go back to the parent
      // cannot be a site
			joint = jointStack.back();
			jointStack.pop_back();
			isSite = false;
			continue;
		}

		// Both can be treaded as the same
		if ( ( strcmp( token, "ROOT" ) == 0 ) ||
		     ( strcmp( token, "JOINT" ) == 0 ) )
		{
			// Create Joint Info and Push Back
			newJoint = new Joint();
			newJoint->index = joints.size();
			newJoint->parent = joint;
			newJoint->hasSite = false;
			newJoint->offset[0] = 0.0;  newJoint->offset[1] = 0.0;  newJoint->offset[2] = 0.0;
			newJoint->site[0] = 0.0;  newJoint->site[1] = 0.0;  newJoint->site[2] = 0.0;
			joints.push_back( newJoint );
			if ( joint )
				joint->children.push_back( newJoint );

			// Remove white space to get Joint name
			token = strtok( NULL, "" );
			while ( *token == ' ' )  token ++;
			newJoint->name = token;

			// set Joint name to thisJoint name
			jointIndex[ newJoint->name ] = newJoint;

      // set a default global position
      globalPositions.push_back(0.);
      globalPositions.push_back(0.);
      globalPositions.push_back(0.);

      // set a default rotation
      jointAngles.push_back(0.);
      jointAngles.push_back(0.);
      jointAngles.push_back(0.);

			continue;
		}

		// Sites e.g. head, fingertips
		if ( ( strcmp( token, "End" ) == 0 ) )
		{
			newJoint = joint;
			isSite = true;
			continue;
		}

		// Next read in the OFFSET as either site/offset
		if ( strcmp( token, "OFFSET" ) == 0 )
		{
			// split and convert string to double
			token = strtok( NULL, separater );
			x = token ? atof( token ) : 0.0;
			token = strtok( NULL, separater );
			y = token ? atof( token ) : 0.0;
			token = strtok( NULL, separater );
			z = token ? atof( token ) : 0.0;

			// Positional data is either a site
			if ( isSite )
			{
				joint->hasSite = true;
				joint->site[0] = x;
				joint->site[1] = y;
				joint->site[2] = z;
			}
			else
			// or an offset
			{
				joint->offset[0] = x;
				joint->offset[1] = y;
				joint->offset[2] = z;
			}
			continue;
		}

		// Convert words in file to enums
		if ( strcmp( token, "CHANNELS" ) == 0 )
		{
			// Read how many channels there are and resize vector
			token = strtok( NULL, separater );
			joint->channels.resize( token ? atoi( token ) : 0 );

			// Loop through previously discovered number and assign
			for (i = 0; i < joint->channels.size(); i++ )
			{
				// Create a new instance of Channel to link to Joint
				Channel *  channel = new Channel();
				channel->joint = joint;
				channel->index = channels.size();
				channels.push_back( channel );
				joint->channels[ i ] = channel;

				// Big if statment innit
				token = strtok( NULL, separater );
				if ( strcmp( token, "Xrotation" ) == 0 )
					channel->type = X_ROTATION;
				else if ( strcmp( token, "Yrotation" ) == 0 )
					channel->type = Y_ROTATION;
				else if ( strcmp( token, "Zrotation" ) == 0 )
					channel->type = Z_ROTATION;
				else if ( strcmp( token, "Xposition" ) == 0 )
					channel->type = X_POSITION;
				else if ( strcmp( token, "Yposition" ) == 0 )
					channel->type = Y_POSITION;
				else if ( strcmp( token, "Zposition" ) == 0 )
					channel->type = Z_POSITION;
			}
		}

		// Oh No! We Read Too Far, Abandon Ship!
		if ( strcmp( token, "MOTION" ) == 0 )
    {
      break;
    }
	}
  ///////////////////////////////////
	// MOTION READ IN
  ///////////////////////////////////

  // read in number of frames
	file.getline( line, BUFFER_LENGTH );
	token = strtok( line, separater );
	if ( strcmp( token, "Frames" ) != 0 )  goto bvh_error;
	token = strtok( NULL, separater );
	if ( token == NULL )  goto bvh_error;
	numFrame = atoi( token );

  // read in FPS
	file.getline( line, BUFFER_LENGTH );
	token = strtok( line, ":" );
	if ( strcmp( token, "Frame Time" ) != 0 )  goto bvh_error;
	token = strtok( NULL, separater );
	if ( token == NULL )  goto bvh_error;
	interval = atof( token );

	numChannel = channels.size();
	motion = new double[ numFrame * numChannel ];

	// Data is all stored sequentially and we already have
  // info to calculate line nums so can just read absolutely
  // everything in to an array, work out who it belongs to
  // later
	for (i = 0; (int)i < numFrame; i++ )
	{
		file.getline( line, BUFFER_LENGTH );
		token = strtok( line, separater );
		for ( j = 0; (int)j < numChannel; j++ )
		{
			if ( token == NULL )
				goto bvh_error;
			motion[ i * numChannel + j ] = atof( token );
			token = strtok( NULL, separater );
		}
	}
	// Avoids multiple read errors
	file.close();

	// We've made it to the end so
  // something must have went right
	isLoadSuccess = true;

	return;
//
bvh_error:
	file.close();
}
// Find the Min and Max 3D positions of the root node
// for camera scaling and bounding boxes
void BVH::FindMinMax()
{
  float minX, maxX, minY, maxY, minZ, maxZ;
  minX = 99999.;
  minY = 99999.;
  minZ = 99999.;
  maxX = -99999.;
  maxY = -99999.;
  maxZ = -99999.;

  double * address;
  double thisX;
  double thisY;
  double thisZ;

  for(int i = 0; i < numFrame; i++)
  {

    address = motion + i * numChannel;

    thisX = address[0];
    thisY = address[1];
    thisZ = address[2];

    if(thisX > maxX) maxX = thisX;
    if(thisX < minX) minX = thisX;
    if(thisY > maxY) maxY = thisY;
    if(thisY < minY) minY = thisY;
    if(thisZ > maxZ) maxZ = thisZ;
    if(thisZ < minZ) minZ = thisZ;
  }
  // apply to class
  this->minCoords = Cartesian3(minX, minY, minZ);
  this->maxCoords = Cartesian3(maxX, maxY, maxZ);

  this->boundingBoxSize = max(max((maxX - minX), (maxY - minY)), (maxZ - minZ)) + 2.0;

}

void BVH::FindGlobalPosition(Joint *joint, Camera *camera)
{
  // find the global position

  // find the global position and update it
  GLfloat matrix[16];
  glGetFloatv (GL_MODELVIEW_MATRIX, matrix);

  glm::mat4 modelMatrix = glm::make_mat4(matrix);



  // undo the camera
  modelMatrix = glm::inverse(camera->GetViewMatrix()) * modelMatrix;



  // calculate the local matrix of this Joint
  glm::mat4 parentMM = glm::mat4(1.); // identity
  Joint * prntPtr = joint->parent;
  // nice recursive one line loop!
  while(prntPtr != NULL){ parentMM = parentMM * prntPtr->localMatrix; prntPtr = prntPtr->parent; }

  joint->localMatrix = modelMatrix * glm::inverse(parentMM);


    glm::vec4 pos = glm::vec4(joint->offset[0], joint->offset[0], joint->offset[0], 1.);

    glm::vec4 emp = glm::vec4(0., 0., 0., 1.);

    glm::vec4 global = modelMatrix * emp;


    // set the global position from what we have just calculated
    globalPositions[3 * joint->index]     = global[0];
    globalPositions[3 * joint->index + 1] = global[1];
    globalPositions[3 * joint->index + 2] = global[2];

}

////////////////////////////////////////////////
// INVERSE KINEMATIC FUNCTIONS
////////////////////////////////////////////////

// void BVH::LerpPoint()
// {
//   // Lerp from positions ver t
// }



void BVH::MoveJoint(int id, glm::vec3 move, int mode)
{

  // find current joints position and rotation
  Joint *j = joints[id];
  vector<Channel *> chn = j->channels;

  double rot[3] = {0., 0., 0.};
  double pos[3] = {0., 0., 0.};

  // read position
  pos[0] = globalPositions[3 * j->index];
  pos[1] = globalPositions[3 * j->index + 1];
  pos[2] = globalPositions[3 * j->index + 2];

  // read rotation
  for(int i = 0; i < chn.size(); i++)
  {
    if(chn[i]->type == 0){ rot[0] = j->dataStart[chn[i]->index]; }
    if(chn[i]->type == 1){ rot[1] = j->dataStart[chn[i]->index]; }
    if(chn[i]->type == 2){ rot[2] = j->dataStart[chn[i]->index]; }
  }

  // Do a Simple Rotation On The Object
  if(mode == ROTATE)
  {


    for(int i = 0; i < chn.size(); i++)
    {
      if(chn[i]->type == 0){ j->dataStart[chn[i]->index] += move.x; } // X ROTATION
      if(chn[i]->type == 1){ j->dataStart[chn[i]->index] += move.y; } // Y ROTATION
      if(chn[i]->type == 2){ j->dataStart[chn[i]->index] += move.z; } // Z ROTATION
    }
  }

  // Do IK
  if(mode == INVERSEKINEMATICS)
  {
    std::cout << "" << '\n';
    std::cout << "" << '\n';
    std::cout << "INVERSEKINEMATICS" << '\n';

    // start and end points
    double c[3] = {pos[0], pos[1]     , pos[2]};
    double t[3] = {pos[0], pos[1] + 1., pos[2]};

    // get position and orientatino of new end effector
    double y[6] = {pos[0], pos[1], pos[2], rot[0], rot[1], rot[2] };


    // create jacobian matrix
    int l = jointAngles.size();
    double jaco[3][l];

    // set all the values to 0
    for(int i = 0; i < 3; i++)
    {
      for(int j = 0; j < l; j++)
      {
        jaco[i][j] = 0.; // set to 0.
      }
    }

    // how we move to derive jacobian
    double dSigma = 0.001;

    // x coordinate of the end effector
    double p1 = pos[0];

    // where the hand ends up with forward kinematics
    double p2;

    // now find the list of parents to get to this point
    vector<Joint *> fwdK;
    Joint *p = j->parent;

    while(p != NULL)
    { // add to the stack
      fwdK.push_back(p);
      p = p->parent;
    }

    // cPos[1] = globalPositions[3 * cIndex + 1];
    // cPos[0] = globalPositions[3 * cIndex];
    // cPos[2] = globalPositions[3 * cIndex + 2];

    // final position of end effector
    Joint *cJ = fwdK[fwdK.size() - 1];   // current joint is the last one
    int cIndex = cJ->index;
    glm::vec4 cPos;                      // homogenius position

    // find global start position
    cPos.x = 1.;
    cPos.y = 1.;
    cPos.z = 1.;
    cPos.w = 1.;

    // now perform forward kinematics
    for(int i = fwdK.size() - 1; i > -1; i--)
    {
      // which joint we are talking about
      cJ = fwdK[i];
      cIndex = cJ->index;


      //add offset
      double cOff[3] = {cJ->offset[0], cJ->offset[1], cJ->offset[2]};

      cPos.x += cOff[0];
      cPos.y += cOff[1];
      cPos.z += cOff[2];

      // multiply by local matrix
      glm::mat4 m = cJ->localMatrix;

      std::cout << fwdK[i]->name << '\n';
      std::cout << "Pos: " << cPos.x << " " << cPos.y << " " << cPos.z << '\n';
      std::cout << "Off: " << cOff[0] << " " << cOff[1] << " " << cOff[2] << '\n';
      std::cout << "Local Matrix" << '\n';

      // add the delta theta in the correct axis
      // todo

      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          std::cout << m[i][j] << ' ';
        }
        std::cout << "" << '\n';
      }
      std::cout << "" << '\n';
      std::cout << "" << '\n';

      // multiply by local matrix
      cPos = cPos * m;

    }
    std::cout << "Final Pos: " << cPos.x << " " << cPos.y << " " << cPos.z << '\n';
  }





  //std::cout << "" << '\n';
}

////////////////////////////////////////////////
// RENDERING FUNCTIONS
////////////////////////////////////////////////


// The initial call to render figure, from this point on
// will go to the recursive method
void  BVH::RenderFigure( int frameNo, float scale, Camera *camera)
{
	// Calculate poistion into array to start based on frame
	RenderFigure( joints[ 0 ], motion + frameNo * numChannel, scale, camera );
}


// Going to be using a lot of
// https://www.youtube.com/watch?v=vCadcBR95oU
void  BVH::RenderFigure(Joint * joint, double * data, float scale, Camera *camera)
{
	glPushMatrix();

  // remeber where the data reading starts
  // for editing later
  joint->dataStart = data;

	// If this is the root notde
	if ( joint->parent == NULL )
	{
		glTranslatef( data[ 0 ] * scale, data[ 1 ] * scale, data[ 2 ] * scale );
	}
	// If we are an average Joe, find our offset
	else
	{
		glTranslatef( joint->offset[ 0 ] * scale, joint->offset[ 1 ] * scale, joint->offset[ 2 ] * scale );
	}

	// Apply local roations for this Joint
  // and find out the rotations of each joint if available
	unsigned int  i;
  double thisRot;
	for ( i = 0; i < joint->channels.size(); i++ )
	{
		Channel *  channel = joint->channels[ i ];
    thisRot = data[channel->index];
		if ( channel->type == X_ROTATION )
    {
      glRotatef( thisRot, 1.0f, 0.0f, 0.0f );
      jointAngles[3 * joint->index]   = thisRot; // set X for this joint
    }
		else if ( channel->type == Y_ROTATION )
    {
      glRotatef( data[ channel->index ], 0.0f, 1.0f, 0.0f );
      jointAngles[3 * joint->index + 1] = thisRot; // set Y for this joint
    }
		else if ( channel->type == Z_ROTATION )
    {
      glRotatef( data[ channel->index ], 0.0f, 0.0f, 1.0f );
      jointAngles[3 * joint->index + 2]  = thisRot; // set Z for this joint
    }
	}

  // Now we have the local matrix for each position
  FindGlobalPosition(joint, camera);

  // we are at the end, so use SITE data
	if ( joint->children.size() == 0 )
	{
		RenderBone(0.0f, 0.0f, 0.0f, joint->site[ 0 ] * scale, joint->site[ 1 ] * scale, joint->site[ 2 ] * scale );
	}
	// We are going to a child, so draw to there
	if ( joint->children.size() == 1 )
	{
		Joint *  child = joint->children[ 0 ];
		RenderBone(0.0f, 0.0f, 0.0f, child->offset[ 0 ] * scale, child->offset[ 1 ] * scale, child->offset[ 2 ] * scale );
	}
	// else, draw to midpoint
	if ( joint->children.size() > 1 )
	{
		// calculate the center of your kids
		float  center[ 3 ] = { 0.0f, 0.0f, 0.0f };
		for ( i = 0; i < joint->children.size(); i++ )
		{
			Joint *  child = joint->children[ i ];
			center[ 0 ] += child->offset[ 0 ];
			center[ 1 ] += child->offset[ 1 ];
			center[ 2 ] += child->offset[ 2 ];
		}
		center[ 0 ] /= joint->children.size() + 1;
		center[ 1 ] /= joint->children.size() + 1;
		center[ 2 ] /= joint->children.size() + 1;

		// Put this bone at the center of your kids
		RenderBone(0.0f, 0.0f, 0.0f, center[ 0 ] * scale, center[ 1 ] * scale, center[ 2 ] * scale );

		// Render the Bones of all your children
		for ( i = 0; i < joint->children.size(); i++ )
		{
			Joint *  child = joint->children[ i ];
			RenderBone(center[ 0 ] * scale, center[ 1 ] * scale, center[ 2 ] * scale,
				child->offset[ 0 ] * scale, child->offset[ 1 ] * scale, child->offset[ 2 ] * scale );
		}
	}

	// Call this function recursively on children
	for ( i = 0; i < joint->children.size(); i++ )
	{
		RenderFigure( joint->children[ i ], data, scale, camera);
	}

	glPopMatrix();
}


// Renders a single bone (Happends a lot)
void  BVH::RenderBone(float x0, float y0, float z0, float x1, float y1, float z1, float bRadius )
{

	// Calculate a from -> to vector
	GLdouble  dir_x = x1 - x0;
	GLdouble  dir_y = y1 - y0;
	GLdouble  dir_z = z1 - z0;
	GLdouble  bone_length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );

	// Uses GLU for rendering
	static GLUquadricObj *  quad_obj = NULL;
	if ( quad_obj == NULL )
		quad_obj = gluNewQuadric();
	gluQuadricDrawStyle( quad_obj, GLU_FILL );
	gluQuadricNormals( quad_obj, GLU_SMOOTH );

	glPushMatrix();

	// translate to start
	glTranslated( x0, y0, z0 );

	// if bone is too short, do this to avoid
  // wierd rendering errors
	double  length;
	length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );
	if ( length < 0.0001 ) {
		dir_x = 0.0; dir_y = 0.0; dir_z = 1.0;  length = 1.0;
	}
	dir_x /= length;  dir_y /= length;  dir_z /= length;

	// for calculating normals
	GLdouble  up_x, up_y, up_z;
	up_x = 0.0;
	up_y = 1.0;
	up_z = 0.0;

	// for calculating local rotation matrix
	double  side_x, side_y, side_z;
	side_x = up_y * dir_z - up_z * dir_y;
	side_y = up_z * dir_x - up_x * dir_z;
	side_z = up_x * dir_y - up_y * dir_x;

  // if bone is too short, do this to avoid
  // wierd rendering errors
	length = sqrt( side_x*side_x + side_y*side_y + side_z*side_z );
	if ( length < 0.0001 ) {
		side_x = 1.0; side_y = 0.0; side_z = 0.0;  length = 1.0;
	}
	side_x /= length;  side_y /= length;  side_z /= length;

  // cross product to calculate up
	up_x = dir_y * side_z - dir_z * side_y;
	up_y = dir_z * side_x - dir_x * side_z;
	up_z = dir_x * side_y - dir_y * side_x;

	// local rotation matrix
	GLdouble  m[16] = { side_x, side_y, side_z, 0.0,
	                    up_x,   up_y,   up_z,   0.0,
	                    dir_x,  dir_y,  dir_z,  0.0,
	                    0.0,    0.0,    0.0,    1.0 };
	glMultMatrixd( m );

	//
	GLdouble radius= bRadius; // defined in class
	GLdouble slices = 8.0;    // resolution of cyclinder
	GLdouble stack = 3.0;     // z subdivisions

	// with all our new found info, render a gluCylinder
	gluCylinder( quad_obj, radius, radius, bone_length, slices, stack );

	glPopMatrix();
}

// Renders the points where a user can click
void BVH::RenderControlPoints()
{
  GLUquadric *quad;
  quad = gluNewQuadric();

  // save current state and load identity
  glPushMatrix();
  //glLoadIdentity();
  for(int i = 0; i < (globalPositions.size() / 3); i++)
  {
    // change the colour to blue if being clicked on
    if(activeJoint == i) { glColor3f(1., 0., 0.); }
    else { glColor3f(0. , 0. , .6); }

    // Find the global position to draw the point
    glPushMatrix();
    glTranslatef(globalPositions[3 * i], globalPositions[3 * i + 1], globalPositions[3 * i + 2]);
    gluSphere(quad, .5, 5, 5);
    glPopMatrix();
  }
  // get back to normal
  glPopMatrix();

  // set the colour to white
  glColor3f(1., 1., 1.);
}
