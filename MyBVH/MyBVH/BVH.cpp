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
  moveMode = INVERSEKINEMATICS;
	Clear();
  activeJoints.clear();
}

// Standard Constructor With File Name
BVH::BVH( const char * bvhFileName )
{
	motion = NULL;
  moveMode = INVERSEKINEMATICS;
	Clear();
	Load( bvhFileName );
  activeJoints.clear();
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

  ///////////////////////////////
  // FIRST READ
  //////////////////////////////
  // first we read the file in for saving purposes later
  file.open( bvhFileName, ios::in );
  std::vector<char> charVec;
	if ( file.is_open() == 0 )  return; // can't be opened for whatever reason
	while ( ! file.eof() )
  {
    file.getline( line, BUFFER_LENGTH );

    if(strcmp(line,"MOTION\r") == 0){ break; }
    if(strcmp(line,"MOTION") == 0){ break; }

    for(int i = 0; i < strlen(line); i++)
    {
      charVec.push_back(line[i]);
    }
    charVec.push_back('\n');

  }
  file.close();
  fileContents = std::string(charVec.begin(), charVec.end());

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

  glm::vec4 emp = glm::vec4(0., 0., 0., 1.);

  glm::vec4 global = modelMatrix * emp;


  // set the global position from what we have just calculated
  globalPositions[3 * joint->index]     = global[0];
  globalPositions[3 * joint->index + 1] = global[1];
  globalPositions[3 * joint->index + 2] = global[2];
}


void BVH::MoveJoint(int id, glm::vec3 move)
{
  // find current joints position and rotation
  Joint *j = joints[id];
  vector<Channel *> chn = j->channels;

  ////////////////
  // ROTATION
  ///////////////
  if(moveMode == ROTATE)
  {
    for(int i = 0; i < chn.size(); i++)
    {
      if(chn[i]->type == 0){ j->dataStart[chn[i]->index] += move.x; } // X ROTATION
      if(chn[i]->type == 1){ j->dataStart[chn[i]->index] += move.y; } // Y ROTATION
      if(chn[i]->type == 2){ j->dataStart[chn[i]->index] += move.z; } // Z ROTATION
    }
    return;
  }

  /////////////////////
  // INVERSE KINEMATICS
  /////////////////////
  if(moveMode == INVERSEKINEMATICS)
  {

    /////////////////
    // ROOT
    ////////////////
    if(j->parent == NULL)
    {
      float moveSens = 0.25;
      for(int i = 0; i < chn.size(); i++)
      {
        if(chn[i]->type == 3){ j->dataStart[chn[i]->index] += move.x * moveSens; } // X MOVEMENT
        if(chn[i]->type == 4){ j->dataStart[chn[i]->index] += move.y * moveSens; } // Y MOVEMENT
        if(chn[i]->type == 5){ j->dataStart[chn[i]->index] += move.z * moveSens; } // Z MOVEMENT
      }
      return;
    }



    ///////////////////////////////
    // CALCULATE DESIRED POSITION
    ///////////////////////////////

    // now subtract the worldspace transformation to put
    // the character in view
    glm::vec3 offset = glm::vec3(- (maxCoords.x + minCoords.x) / 2., -(maxCoords.y + minCoords.y) / 2., -(minCoords.z - 25.));

    // start and end points
    glm::vec3 p1 = glm::vec3(globalPositions[3 * j->index    ] + offset.x,
                             globalPositions[3 * j->index + 1] + offset.y,
                             globalPositions[3 * j->index + 2] + offset.z);

    glm::vec3 p2 = {p1.x + move.x         , p1.y + move.y         , p1.z + move.z        };

    // find the distance from the root to the desired position
    glm::vec3 rootPosition = glm::vec3(globalPositions[0] + offset.x,
                                       globalPositions[1] + offset.y,
                                       globalPositions[2] + offset.z);

    float rootToDesired = glm::distance(rootPosition, p2);

    // std::cout << "----------------------" << '\n';
    //  std::cout << "Start" << '\n';
    // std::cout << p1.x << " " << p1.y << " " << p1.z << '\n';
    // std::cout << p2.x << " " << p2.y << " " << p2.z << '\n';
    // std::cout << move.x << "  " << move.y << "  " << move.z << '\n';
    // std::cout << "" << '\n';

    // create jacobian matrix
    int l = jointAngles.size();

    double jacobian[3][l];

    // set all the values to 0
    for(int i = 0; i < 3; i++)
    {
      for(int j = 0; j < l; j++)
      {
        jacobian[i][j] = 0.;
      }
    }

    ///////////////////
    // derive Jacobian
    //////////////////
    float dSigma = .001;

    // find list of parents
    vector<Joint *> fwdK;
    Joint *p = j->parent;

    // find all the parents and check if we can possibly make the
    // movement possible with positional constraints
    float totalPossibleLength = 0.;

    fwdK.push_back(j);
    totalPossibleLength += sqrt(j->offset[0]*j->offset[0] + j->offset[1]*j->offset[1] + j->offset[2]*j->offset[2]);

    while(p != NULL)
    { // add to the stack
      fwdK.push_back(p);

      // add their length
      totalPossibleLength += sqrt(p->offset[0]*p->offset[0] + p->offset[1]*p->offset[1] + p->offset[2]*p->offset[2]);

      p = p->parent;
    }


    // now adjust the target position to never be too long
    if(rootToDesired > totalPossibleLength)
    {
      float ratio = totalPossibleLength / rootToDesired;

      p2 =   glm::vec3(((p2.x - rootPosition.x) * ratio) + rootPosition.x,
                       ((p2.y - rootPosition.y) * ratio) + rootPosition.y,
                       ((p2.z - rootPosition.z) * ratio) + rootPosition.z);
    }
    // local position of end effector

    Joint *cJ;       // current joint
    int cIndex;      // current index
    glm::vec4 cPos;  // homogenius current position
    glm::vec3 rotAxis; // roatation axis we are calculating
    glm::mat4 m;       // matrix for forward kinematics
    glm::mat4 mFinal;  // multiplicitive sum of matricies
    glm::vec4 thisTranslation;
    int numChannels = 0;
    int thisAction = -1;

    bool valid = true;  // is the channel we are iterating on valid
    m = glm::mat4(1.);

    glm::vec3 change; // final position change

    // for every parent joint
    for(int cParent = fwdK.size() - 1; cParent >  -1; cParent--)
    {
      // how many channels this specific joint has
      numChannels = fwdK[cParent]->channels.size();

      // iterate over every channel the joint has
      for(int chn = 0; chn < numChannels; chn++)
      {
        valid = true;
        // find out which action we are reffering to
        switch(fwdK[cParent]->channels[chn]->type)
        {
          case(X_POSITION): valid = false; break;
          case(Y_POSITION): valid = false; break;
          case(Z_POSITION): valid = false; break;
          case(X_ROTATION): thisAction = X_ROTATION; break;
          case(Y_ROTATION): thisAction = Y_ROTATION; break;
          case(Z_ROTATION): thisAction = Z_ROTATION; break;
        }
        // we cannot make a Jacobian from this element
        if(valid == false){ continue; }

        // reset matrix chain
        //matrixChain.clear();
        mFinal = glm::mat4(1.);

        // forward Kinematics with one
        // extra rotation
        for(int i = fwdK.size() - 1; i > -1; i--)
        {
          //std::cout << "                 "  << fwdK[i]->name << '\n';

          // which joint we are talking about
          cJ = fwdK[i];
          cIndex = cJ->index;

          // get the local matrix
          m = cJ->localMatrix;

          // if this is the start position we are interested in
          // apply the small delta rotation
          if(i == cParent)
          {
            // find the correct rotation axis
            if(thisAction == X_ROTATION){ rotAxis = glm::vec3(m[0][0], m[0][1], m[0][2]); }
            if(thisAction == Y_ROTATION){ rotAxis = glm::vec3(m[1][0], m[1][1], m[1][2]); }
            if(thisAction == Z_ROTATION){ rotAxis = glm::vec3(m[2][0], m[2][1], m[2][2]); }

            // rotate by the corract axis in local space
            m = glm::rotate(m, dSigma, rotAxis);
          }

          // add to the matrix chain
          mFinal = mFinal * m;
        }
        cPos.x = 0.;
        cPos.y = 0.;
        cPos.z = 0.;
        cPos.w = 1.;

        cPos = mFinal * cPos ;

        change = (glm::vec3(cPos.x, cPos.y, cPos.z) - p1);

        // Now loop over all the channels the Joint has
        // we must find the X, Y, and Z components
        // to find the correct index

        // making sure to subtract 3 as we do not care
        // about Root Positon.

        // find which channel we need to update
        int chnIdx = 0;
        for(int act = 0; act < fwdK[cParent]->channels.size(); act++)
        {
          if(fwdK[cParent]->channels[act]->type == thisAction)
          {
            chnIdx = fwdK[cParent]->channels[act]->index - 3;
          }
        }

        // only fill in the rotatations if the joint has the ability to rotate
        // in that axis
        for(int c = 0; c < fwdK[cParent]->channels.size(); c++)
        {
          switch(fwdK[cParent]->channels[c]->type)
          {
            case(X_ROTATION):
              jacobian[0][chnIdx] = change.x / dSigma;
              break;
            case(Y_ROTATION):
              jacobian[1][chnIdx] = change.y / dSigma;
              break;
            case(Z_ROTATION):
              jacobian[2][chnIdx] = change.z / dSigma;
              break;
          }
        }
      }
      //exit(0);
    }

    /////////////////////////
    // Solve Jacobian
    ////////////////////////
    //std::cout << "\n\n\n\n\n\n\nJacobian" << '\n';

    // V matrix
    Eigen::Vector3d v;
    v[0] = (double)(p2.x - p1.x);
    v[1] = (double)(p2.y - p1.y);
    v[2] = (double)(p2.z - p1.z);

    Eigen::MatrixXd jaco(3, l);
    // copy values in
    for(int i = 0; i < 3; i++)
    {
      for(int j = 0; j < l; j++)
      {
        jaco.row(i)[j] = (double)jacobian[i][j]; // set to 0.
      }
    }
    // J transpose
    Eigen::MatrixXd jacoT = jaco.transpose();

    // (J * J Transpose)^-1
    Eigen::MatrixXd psJaco = (jaco * jacoT);
    psJaco = psJaco.inverse();
    psJaco = jacoT * psJaco;

    // psJaco * V = delta
    Eigen::MatrixXd deltaM = psJaco * v;


     //////////////////////////
    // // APPLY CHANGES
    // /////////////////////////
    vector<Channel *> chn;

    int jointIndex;

    Channel *thisChannel;

    //std::cout << "APPLY CHANGES" << '\n';

    // step through delta M
    for(int i = 0; i < deltaM.size(); i++)
    {
      std::cout << "i: " << i << " m: " << deltaM(i, 0) << '\n';

      // the index of the joint this item relates to
      jointIndex = channels[i + 3]->joint->index;

      // find which channel index this reffers to
      // std::cout << "\n\ni: " << i << '\n';
      // std::cout << "    Chn Idx: " << channels[i+3]->index << '\n';
      // std::cout << "     Joint Index: " << jointIndex << '\n';
      thisChannel = channels[i+3];
      //
      // std::cout << "   Name: " << thisChannel->joint->name << std::endl;
      // std::cout << "   Type: " << thisChannel->type << std::endl;
      //std::cout << "    " << thisChannel->joint->name << " -- " << thisChannel->type << '\n';

      if(thisChannel->type == X_ROTATION) // X
      {
        //if(deltaM(i, 0) != 0.){ std::cout << "Name: " << joints[jointIndex]->name << std::endl; std::cout << " x " << deltaM(i, 0) << "   idx: " << chn[j]->index << "\n\n" << std::endl;}
      //  std::cout << "    X: " << thisChannel->index << " += " << deltaM(i, 0) << '\n';
        joints[jointIndex]->dataStart[thisChannel->index] += deltaM(i, 0);  // X ROTATION
      }
      if(thisChannel->type == Y_ROTATION) // Y
      {
      //  std::cout << "    Y: " << thisChannel->index << " += " << deltaM(i, 0) << '\n';
        //if(deltaM(i + 1, 0) != 0.){ std::cout << "Name: " <<  joints[jointIndex]->name << std::endl; std::cout << " y " << deltaM(i + 1, 0) << "   idx: " << chn[j]->index << "\n\n" << std::endl;}
        joints[jointIndex]->dataStart[thisChannel->index] += deltaM(i, 0); // Y ROTATION
      }
      if(thisChannel->type == Z_ROTATION) // Z
      {
  //      std::cout << "    Z: " << thisChannel->index << " += " << deltaM(i, 0) << '\n';
        //if(deltaM(i +2, 0) != 0.){ std::cout << "Name: " <<  joints[jointIndex]->name << std::endl; std::cout << " z " << deltaM(i + 2, 0) << "   idx: " << chn[j]->index << "\n\n" << std::endl;}
        joints[jointIndex]->dataStart[thisChannel->index] += deltaM(i, 0); // Z ROTATION
      }

      //std::cout << "" << '\n';
    }


    // for(int i = 0; i < deltaM.size(); i+= 3)
    // {
    //
    //   // the index of the joint this item relates to
    //   jointIndex = channels[i + 3]->joint->index;
    //
    //   //std::cout << "JI: " << jointIndex << '\n';
    //   //std::cout << "NM: " << channels[i + 3]->joint->name << '\n';
    //
    //   // iterate through this joints channels
    //   chn = joints[jointIndex]->channels;
    //
    //   //std::cout << deltaM(i, 0) << '\n';
    //
    //   for(int j = 0; j < chn.size(); j++)
    //   {
    //     if(chn[j]->type == X_ROTATION) // X
    //     {
    //       if(deltaM(i, 0) != 0.){ std::cout << "Name: " << joints[jointIndex]->name << std::endl; std::cout << " x " << deltaM(i, 0) << "   idx: " << chn[j]->index << "\n\n" << std::endl;}
    //       joints[jointIndex]->dataStart[chn[j]->index] += deltaM(i + 0, 0);  // X ROTATION
    //     }
    //     if(chn[j]->type == Y_ROTATION) // Y
    //     {
    //       if(deltaM(i + 1, 0) != 0.){ std::cout << "Name: " <<  joints[jointIndex]->name << std::endl; std::cout << " y " << deltaM(i + 1, 0) << "   idx: " << chn[j]->index << "\n\n" << std::endl;}
    //       joints[jointIndex]->dataStart[chn[j]->index] += deltaM(i + 1, 0); // Y ROTATION
    //     }
    //     if(chn[j]->type == Z_ROTATION) // Z
    //     {
    //       if(deltaM(i +2, 0) != 0.){ std::cout << "Name: " <<  joints[jointIndex]->name << std::endl; std::cout << " z " << deltaM(i + 2, 0) << "   idx: " << chn[j]->index << "\n\n" << std::endl;}
    //       joints[jointIndex]->dataStart[chn[j]->index] += deltaM(i + 2, 0); // Z ROTATION
    //     }
    //   }
    //   //std::cout << "" << '\n';
    // }

    //exit(0);

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
  // save the current frame
  cFrame = frameNo;
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

  // we also calculate the local matrix at this point in time too
  glm::mat4 tempLocal = glm::mat4(1.); // create the identity
  float offsets[3] = {0., 0., 0.};
  GLfloat model[16];

	// If this is the root notde
	if ( joint->parent == NULL )
	{
    offsets[0] = data[0] * scale;
    offsets[1] = data[1] * scale;
    offsets[2] = data[2] * scale;

    // also include the translation we for forward kinematics
    glGetFloatv(GL_MODELVIEW_MATRIX, model);
	}
	// If we are an average Joe, find our offset
	else
	{
    offsets[0] = joint->offset[ 0 ] * scale;
    offsets[1] = joint->offset[ 1 ] * scale;
    offsets[2] = joint->offset[ 2 ] * scale;
	}

  glTranslatef(offsets[0], offsets[1], offsets[2]);

  // add the translation in our local matrix too
  tempLocal = glm::translate(tempLocal, glm::vec3(offsets[0]            , offsets[1]            , offsets[2]            ));

	// Apply local roations for this Joint
  // and find out the rotations of each joint if available
	unsigned int  i;
  double thisRot;
  glm::vec3 axis;
	for ( i = 0; i < joint->channels.size(); i++ )
	{
		Channel *  channel = joint->channels[ i ];
    thisRot = data[channel->index];
		if ( channel->type == X_ROTATION )
    {
      glRotatef( thisRot, 1.0f, 0.0f, 0.0f );
      jointAngles[3 * joint->index]   = thisRot; // set X for this joint
      tempLocal = glm::rotate(tempLocal, (float)glm::radians(thisRot), glm::vec3(1., 0., 0.)); // also find local matrix
      //tempLocal = glm::rotate(tempLocal, glm::radians(thisRot), glm::vec3(1., 0., 0.));
    }
		else if ( channel->type == Y_ROTATION )
    {
      glRotatef( data[ channel->index ], 0.0f, 1.0f, 0.0f );
      jointAngles[3 * joint->index + 1] = thisRot; // set Y for this joint
      tempLocal = glm::rotate(tempLocal, (float)glm::radians(thisRot), glm::vec3(0., 1., 0.)); // also find local matrix
      //tempLocal = glm::rotate(tempLocal, glm::radians(thisRot), glm::vec3(0., 1., 0.));
    }
		else if ( channel->type == Z_ROTATION )
    {
      glRotatef( data[ channel->index ], 0.0f, 0.0f, 1.0f );
      jointAngles[3 * joint->index + 2]  = thisRot; // set Z for this joint
      //tempLocal = glm::rotate(tempLocal, glm::radians(thisRot), glm::vec3(0., 0., 1.)); // also find local matrix
      tempLocal = glm::rotate(tempLocal, (float)glm::radians(thisRot), glm::vec3(0., 0., 1.));
    }
	}

  // set the local matrix
  joint->localMatrix = tempLocal;

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
      //std::cout << "Bone Center --> " << child->name << '\n';
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
// returns the local rotation matrix
void BVH::RenderBone(float x0, float y0, float z0, float x1, float y1, float z1, float bRadius )
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

  // check if we are on a keyframe, if so draw points green
  bool isKeyframe = false;
  if (std::find(keyframes.begin(), keyframes.end(), cFrame) != keyframes.end()){ isKeyframe = true; }

  // save current state and load identity
  glPushMatrix();
  //glLoadIdentity();
  for(int i = 0; i < (globalPositions.size() / 3); i++)
  {
    // change the colour of the joint to be green if keyframe
    if(isKeyframe){ glColor3f(0., 0.6, 0.); }
    else          { glColor3f(0. , 0. , .6); }

    // change the colour to blue if being clicked on
    for(int j = 0; j < activeJoints.size(); j ++)
    {
      if(i == activeJoints[j]){ glColor3f(1., 0., 0.); }
    }


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

void BVH::AddKeyFrame(int advance)
{
  // put the current frame into the keyframes list
  if (std::find(keyframes.begin(), keyframes.end(), cFrame) == keyframes.end()){ keyframes.push_back(cFrame); }

  // put the final frame in the keyframes list
  if (std::find(keyframes.begin(), keyframes.end(), cFrame + advance) == keyframes.end()){ keyframes.push_back(cFrame + advance); }


  // find the start point of the data that needs to be changed
  double *start = motion + (cFrame * numChannel);

  // copy the data that can be unchanged
  double *end = motion + ((cFrame + advance) * numChannel);

  // find the values we want to copy every frame
  double *kFrame = new double[numChannel];

  for(int i = 0; i < numChannel; i++)
  {
    kFrame[i] = start[i];
  }


  // malloc for the correct amount
  double *newMotion = new double[numChannel * (numFrame + advance)];
  int idx = 0; // keeps track of how much writing

  // insert values before the update
  for(int i = 0; i < ((cFrame + 1) * numChannel); i++)
  {
    //std::cout << "B4: " << i << '\n';
    newMotion[idx] = motion[i];
    idx++;
  }

  // repeatedly copy in the keyframe
  for(int i = 0; i < (advance * numChannel); i++)
  {
    //std::cout << "NU: " << i << '\n';
    newMotion[idx] = kFrame[i % numChannel];
    idx++;
  }

  // reinsert the values for the end frames that are unchanged
  //std::cout << "After: " << cFrame+1 << " to " << numFrame << '\n';
  for(int i = ((cFrame+1) * numChannel); i < (numFrame * numChannel); i++)
  {
    //std::cout << "AT: " << i << '\n';
    newMotion[idx] = motion[i];
    idx++;
  }

  // reassign and delete
  delete[] motion;
  motion = newMotion;

  // update num frames
  numFrame += advance;
}

void BVH::SetKeyFrame()
{
  // put the current frame into the keyframes list
  if (std::find(keyframes.begin(), keyframes.end(), cFrame) == keyframes.end()){ keyframes.push_back(cFrame); }
}


double BVH::Lerp(double a, double b, double c)
{
  return((1. - c) * a + c * b);
}

// updates all the lerps between keyframes
void BVH::LerpKeyframes()
{
  // sort all our keyframes
  std::sort (keyframes.begin(), keyframes.end());

  // frame indexes into data
  int startI;
  int endI;

  // frame numbers
  int startFrame;
  int endFrame;

  // contains frame data
  double startF[numChannel];
  double endF[numChannel];

  // iterate and interpolate between keyframes
  for(int k = 0; k < keyframes.size() - 1; k++)
  {
    // find the first and second index
    startI = keyframes[k] * numChannel;
    endI = keyframes[k+1] * numChannel;

    // find start and end frame
    startFrame = keyframes[k];
    endFrame = keyframes[k+1];

    startF[numChannel];
    endF[numChannel];

    // load values
    for(int i = 0; i < numChannel; i++)
    {
      startF[i] = motion[startI + i];
      endF[i] = motion[endI + i];
    }

    // perform lerp on data
    // how much lerp is happending
    double progress = 0.;

    // for every frame we need to animate between
    for(int i = startFrame; i < endFrame; i++)
    {
      progress = (double)(i - startFrame) / (double)(endFrame - startFrame);

      // for every channel
      for(int c = 0; c < numChannel; c++)
      {
        motion[(i * numChannel) + c] = Lerp(startF[c], endF[c], progress);
      }
    }
  }
}

void BVH::SaveFile(std::string fileName)
{
  ofstream  file;
  file.open(fileName);

  file << fileContents;
  file << "MOTION\nFrames: ";
  file << to_string(numFrame);
  file << "\nFrame Time: ";
  file << to_string(interval);
  file << "\n";

  for(int i = 0; i < (numFrame * numChannel); i++)
  {
    if(i > 0 && i % numChannel == 0){ file << "\n"; }
    file << to_string(motion[i]);
    file << " ";
  }
  std::cout << "Saved File" << '\n';
}
