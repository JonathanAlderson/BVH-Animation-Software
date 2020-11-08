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

  //
  //
  // // calculate the local matrix of this Joint
  // glm::mat4 parentMM = glm::mat4(1.); // identity
  // Joint * prntPtr = joint->parent;
  // // nice recursive one line loop!
  // while(prntPtr != NULL){ parentMM = parentMM * prntPtr->localMatrix; prntPtr = prntPtr->parent; }
  //
  // joint->localMatrix = modelMatrix * glm::inverse(parentMM);

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
  //mode = ROTATE;
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
    double c[3] = {pos[0]         , pos[1]         , pos[2]         };
    double t[3] = {pos[0] + move.x, pos[1] + move.y, pos[2] + move.z};

    // get position and orientatino of new end effector
    double y[6] = {pos[0], pos[1], pos[2], rot[0], rot[1], rot[2] };

    //std::cout << "Global End: " << pos[0] <<  "  " << pos[1] << "  " << pos[2] << '\n';

    // create jacobian matrix
    int l = jointAngles.size();
    double jacobian[3][l];

    // set all the values to 0
    for(int i = 0; i < 3; i++)
    {
      for(int j = 0; j < l; j++)
      {
        jacobian[i][j] = 0.; // set to 0.
      }
    }


    ///////////////////
    // derive Jacobian
    //////////////////
    float p1;
    float p2;
    float dSigma = 10.;


    // find list of parents
    vector<Joint *> fwdK;
    Joint *p = j->parent;

    fwdK.push_back(j);
    while(p != NULL)
    { // add to the stack
      fwdK.push_back(p);
      p = p->parent;
    }

    // local position of end effector
    float localTest[3] = {3.57476, -15.2148 -0.568876};

    Joint *cJ;       // current joint
    int cIndex;      // current index
    glm::vec4 cPos;  // homogenius current position
    glm::vec3 rotAxis; // roatation axis we are calculating
    glm::mat4 m;       // matrix for forward kinematics
    glm::mat4 mFinal = glm::mat4(1.);
    glm::vec4 thisTranslation;

    // for every parent joint
    for(int cParent = fwdK.size() - 1; cParent >0-1; cParent--)
    { // for every rotataion we neeed to test
      // std::cout << "------------" << '\n';
      // std::cout << cParent << " " << fwdK[cParent]->name << '\n';
      // std::cout << "" << '\n';

      for(int thisAction = 0; thisAction < 3; thisAction++)
      {

        // coordinate of the end effector we care about
        // if(thisAction == X_ROTATION){ p1 = pos[0]; }
        // if(thisAction == Y_ROTATION){ p1 = pos[1]; }
        // if(thisAction == Z_ROTATION){ p1 = pos[2]; }
        if(thisAction == X_ROTATION){ p1 = localTest[0]; }
        if(thisAction == Y_ROTATION){ p1 = localTest[1]; }
        if(thisAction == Z_ROTATION){ p1 = localTest[2]; }
        //std::cout << "This: " << localTest[0] << '\n';
        //std::cout << "P1: " << p1 << '\n';

        // reset global postion
        cPos.x = 0.;
        cPos.y = 0.;
        cPos.z = 0.;
        cPos.w = 1.;

        // forward Kinematics with one
        // extra rotation
        for(int i = fwdK.size() - 1; i > -1; i--)
        {
          // which joint we are talking about
          cJ = fwdK[i];
          cIndex = cJ->index;

          // get the local matrix
          m = cJ->localMatrix;

          // std::cout << "Local Matrix for: " << cJ->name << '\n';
          // for(int k = 0; k < 4; k++)
          // {
          //   for(int l = 0; l < 4; l++)
          //   {
          //     std::cout << m[l][k] << "          ";
          //   }
          //   std::cout << " " << '\n';
          // }


          // apply the translation from the local matrix
          // in the current mFinal space
          //thisTranslation = glm::vec4(m[3][0], m[3][1], m[3][2], 0.);

          //std::cout << "" << '\n';
          //std::cout << "This Translation: " << thisTranslation.x << " " << thisTranslation.y << " " << thisTranslation.z << '\n';
          //thisTranslation = thisTranslation * mFinal;
          //std::cout << "x mFinal:  " << thisTranslation.x << " " << thisTranslation.y << " " << thisTranslation.z << '\n';

          // reset translation in m
          //m[3][0] = 0.;
          //m[3][1] = 0.;
          //m[3][2] = 0.;


          // put translation into mFinal
          //mFinal[3][0] = thisTranslation.x;
          //mFinal[3][1] = thisTranslation.y;
          //mFinal[3][2] = thisTranslation.z;


          // if this is the start position we are interested in
          // apply the small delta rotation
          if(i == cParent)
          {
            //std::cout << "Yes" << '\n';

            // find the correct rotation axis
            if(thisAction == X_ROTATION){ rotAxis = glm::vec3(m[0][0], m[1][0], m[2][0]); }
            if(thisAction == Y_ROTATION){ rotAxis = glm::vec3(m[0][1], m[1][1], m[2][1]); }
            if(thisAction == Z_ROTATION){ rotAxis = glm::vec3(m[0][2], m[1][2], m[2][2]); }
            // rotate by the corract axis in local space
            m = glm::rotate(m, dSigma, rotAxis);
          }

          // report translation in the current matrix space
          mFinal = m * mFinal;
          //cPos = mFinal * cPos;



          // std::cout << "----------------" << '\n';
          // std::cout << "Current Total Rotation Matrix" << '\n';
          // std::cout << "----------------" << '\n';
          // for(int k = 0; k < 4; k++)
          // {
          //   for(int l = 0; l < 4; l++)
          //   {
          //     std::cout << mFinal[l][k] << "          ";
          //   }
          //   std::cout << " " << '\n';
          // }

          // std::cout << "" << '\n';
          // std::cout << "    Cpos:       " << cPos.x <<  "  " << cPos.y << "  " << cPos.z << "  " << cPos.w << '\n';
          // std::cout << "-----------------" << '\n';
          // add the offset
          //cPos.x += cJ->offset[0];
          //cPos.y += cJ->offset[1];
          //cPos.z += cJ->  //std::cout << "With Offset:       " << cPos.x <<  "  " << cPos.y << "  " << cPos.z << '\n';[2];

          //std::cout << "With Offset:       " << cPos.x <<  "  " << cPos.y << "  " << cPos.z << '\n';

          //std::cout << cJ->offset[0] << " " << cJ->offset[1] << " " << cJ->offset[2] << '\n';
        }

        cPos = mFinal * cPos;
        //std::cout << "End Pos" << '\n';
        //std::cout << cPos.x << " " << cPos.y << " " << cPos.z << '\n';

        // find the correct component from cPos at the end
        if(thisAction == X_ROTATION){ p2 = cPos.x; }
        if(thisAction == Y_ROTATION){ p2 = cPos.y; }
        if(thisAction == Z_ROTATION){ p2 = cPos.z; }

        // set the value in the jacobian
        jacobian[thisAction][fwdK[cParent]->index] = (p2 - p1) / dSigma;

        std::cout << "Setting: " << thisAction << " " << fwdK[cParent]->index << "   p1:  " << p1 << "  p2: " << p2 << "            --> " << (p2 - p1) / dSigma << '\n';

      }
    }

    /////////////////////////
    // Solve Jacobian
    ////////////////////////

    // V matrix
    Eigen::Vector3d v;
    v[0] =  (double)(t[0] - c[0]);
    v[1] =  (double)(t[1] - c[1]);
    v[2] =  (double)(t[2] - c[2]);

    // Jacobian
    Eigen::MatrixXd jaco;
    jaco.resize(3, l);

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

    //std::cout << psJaco << '\n';
    //Eigen::MatrixXd psJaco = (jacoT * jaco);
    //psJaco = psJaco.inverse();

    //psJaco = psJaco * jacoT;

    // psJaco * V = delta
    Eigen::MatrixXd deltaM = psJaco * v;

    std::cout << "deltaM" << '\n';
    std::cout << deltaM << '\n';


    //////////////////////////
    // APPLY CHANGES
    /////////////////////////
    vector<Channel *> chn;
    float change;

    for(int i = 0; i < deltaM.row(0).size(); i++)
    {
      std::cout << deltaM.row(0)[i] << '\n';
      change = deltaM.row(0)[i];
      chn = joints[i]->channels;
      
      for(int i = 0; i < chn.size(); i++)
      {
        if(chn[i]->type == 0){ joints[i]->dataStart[chn[i]->index] += change; } // X ROTATION
        if(chn[i]->type == 1){ joints[i]->dataStart[chn[i]->index] += change; } // Y ROTATION
        if(chn[i]->type == 2){ joints[i]->dataStart[chn[i]->index] += change; } // Z ROTATION
      }
    }

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
  //std::cout << "Calculating Local Matrix" << '\n';
  //std::cout << joint->name << '\n';
	glPushMatrix();

  // remeber where the data reading starts
  // for editing later
  joint->dataStart = data;

  // we also calculate the local matrix at this point in time too
  //glm::mat4 tempLocal = glm::mat4(1.); // create the identity
  float offsets[3] = {0., 0., 0.};
  float rotations[3] = {0., 0., 0.};
  GLfloat model[16];

	// If this is the root notde
	if ( joint->parent == NULL )
	{
    offsets[0] = data[0] * scale;
    offsets[1] = data[1] * scale;
    offsets[2] = data[2] * scale;

    // also include the translation we for forward kinematics
    glGetFloatv(GL_MODELVIEW_MATRIX, model);
    //offsets[0] += model[12]; // x translation
    //offsets[1] += model[13]; // y translaiton
    //offsets[2] += model[14]; // z translation
	}
	// If we are an average Joe, find our offset
	else
	{
    offsets[0] = joint->offset[ 0 ] * scale;
    offsets[1] = joint->offset[ 1 ] * scale;
    offsets[2] = joint->offset[ 2 ] * scale;
	}

  glTranslatef(offsets[0], offsets[1], offsets[2]);

  // compute the translation in our local matrix too
  //if(joint->parent == NULL){ tempLocal = glm::translate(tempLocal, glm::vec3(offsets[0] + model[12], offsets[1] + model[13], offsets[2] + model[14]));  }
  //else                     { tempLocal = glm::translate(tempLocal, glm::vec3(offsets[0]            , offsets[1]            , offsets[2]            )); }
  //tempLocal = glm::translate(tempLocal, glm::vec3(offsets[0]            , offsets[1]            , offsets[2]            ));
  //tempLocal = glm::translate(tempLocal, glm::vec3(offsets[0]            , offsets[1]            , offsets[2]            ));

  // for(int k = 0; k < 4; k++)
  // {
  //   for(int l = 0; l < 4; l++)
  //   {
  //     std::cout << tempLocal[l][k] << "    ";
  //   }
  //   std::cout << " " << '\n';
  // }

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
      //std::cout << "X Rot: " << thisRot << '\n';
      //tempLocal = glm::rotate(tempLocal, (float)thisRot, glm::vec3(1., 0., 0.)); // also find local matrix
      rotations[0] = thisRot; // for calculating local axis
    }
		else if ( channel->type == Y_ROTATION )
    {
      glRotatef( data[ channel->index ], 0.0f, 1.0f, 0.0f );
      jointAngles[3 * joint->index + 1] = thisRot; // set Y for this joint
      //std::cout << "Y Rot: " << thisRot << '\n';
      //tempLocal = glm::rotate(tempLocal, (float)thisRot, glm::vec3(0., 1., 0.)); // also find local matrix
      rotations[1] = thisRot; // for calculating local axis
    }
		else if ( channel->type == Z_ROTATION )
    {
      glRotatef( data[ channel->index ], 0.0f, 0.0f, 1.0f );
      jointAngles[3 * joint->index + 2]  = thisRot; // set Z for this joint
      //std::cout << "Z Rot: " << thisRot << '\n';
      //tempLocal = glm::rotate(tempLocal, (float)thisRot, glm::vec3(0., 0., 1.)); // also find local matrix
      rotations[2] = thisRot; // for calculating local axis
    }
	}

  // Now we have the local matrix for each position
  FindGlobalPosition(joint, camera);
  // for(int k = 0; k < 4; k++)
  // {
  //   for(int l = 0; l < 4; l++)
  //   {
  //     std::cout << tempLocal[l][k] << "    ";
  //   }
  //   std::cout << " " << '\n';
  // }
  // std::cout << "----------" << '\n';

  // find the local matrix from the bone calculations
  glm::mat4 localM;

  // we are at the end, so use SITE data
	if ( joint->children.size() == 0 )
	{
    //std::cout << "Bone " << joint->name << '\n';
		localM = RenderBone(0.0f, 0.0f, 0.0f, joint->site[ 0 ] * scale, joint->site[ 1 ] * scale, joint->site[ 2 ] * scale );
	}
	// We are going to a child, so draw to there
	if ( joint->children.size() == 1 )
	{
    //std::cout << "Bone " << joint->name << '\n';
		Joint *  child = joint->children[ 0 ];
		localM = RenderBone(0.0f, 0.0f, 0.0f, child->offset[ 0 ] * scale, child->offset[ 1 ] * scale, child->offset[ 2 ] * scale );
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
    //std::cout << "Bone " << joint->name << '\n';
		localM = RenderBone(0.0f, 0.0f, 0.0f, center[ 0 ] * scale, center[ 1 ] * scale, center[ 2 ] * scale );

		// Render the Bones of all your children
		for ( i = 0; i < joint->children.size(); i++ )
		{
			Joint *  child = joint->children[ i ];
      //std::cout << "Bone Center --> " << child->name << '\n';
			RenderBone(center[ 0 ] * scale, center[ 1 ] * scale, center[ 2 ] * scale,
				child->offset[ 0 ] * scale, child->offset[ 1 ] * scale, child->offset[ 2 ] * scale );
		}
	}

  //std::cout << "Local Matrix" << '\n';
  //std::cout << "" << '\n';

  // apply the rotations to the local matrix
  // for(int i = 0; i < 3; i ++)
  // {
  //   localM = glm::rotate(localM, (float)rotations[i], glm::vec3(localM[0][i], localM[1][i], localM[2][i])); // also find local matrix
  // }
  // apply the translations to the local matrix
  localM[3][0] = offsets[0]; // x offset
  localM[3][1] = offsets[1]; // y offset
  localM[3][2] = offsets[2]; // z offset

  // std::cout << offsets[0] << " " << offsets[1] << " " << offsets[2] << '\n';
  //
  // for(int i = 0; i < 4; i++)
  // {
  //   for(int j = 0; j < 4; j++)
  //   {
  //     std::cout << localM[i][j] << " ";
  //   }
  //   std::cout << " " << '\n';
  // }


  // assign the local matrix to be
  // returned value form Render Bone
  joint->localMatrix = localM; // * tempLocal;

	// Call this function recursively on children
	for ( i = 0; i < joint->children.size(); i++ )
	{
		RenderFigure( joint->children[ i ], data, scale, camera);
	}

	glPopMatrix();
}


// Renders a single bone (Happends a lot)
// returns the local rotation matrix
glm::mat4 BVH::RenderBone(float x0, float y0, float z0, float x1, float y1, float z1, float bRadius )
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
  // assign the to the pointer
  glm::mat4 localM = glm::make_mat4(m);


	glMultMatrixd( m );

	//
	GLdouble radius= bRadius; // defined in class
	GLdouble slices = 8.0;    // resolution of cyclinder
	GLdouble stack = 3.0;     // z subdivisions

	// with all our new found info, render a gluCylinder
	gluCylinder( quad_obj, radius, radius, bone_length, slices, stack );

	glPopMatrix();

  return localM;
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
