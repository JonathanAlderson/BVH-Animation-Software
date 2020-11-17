///////////////////////////////////////////////////
//
//	Hamish Carr
//	January, 2018
//
//	------------------------
//	RenderWidget.h
//	------------------------
//
//	The main widget that shows the geometry
//
///////////////////////////////////////////////////

#include <math.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "RenderWidget.h"

// constructor
RenderWidget::RenderWidget(char *filename, MasterWidget *parent)
	: QGLWidget(parent)
	{ // constructor
		this->parentWidget = parent;

		// load a new file
		if(strlen(filename) == 0){bvh = new BVH(); }
		else {bvh = new BVH(filename); }
		bvh->FindMinMax();
		std::cout << "Reading: ";
		std::cout << filename << '\n';


		// movement constraints
		// unconstrained at start
		xAxis = true;
		yAxis = true;
		zAxis = true;

		// delete this
		doneOnce = false;

		// set default values
		whichButton = -1;

		// will change when scolling
		size = bvh->boundingBoxSize;

		// zoom amout
		zoom = 1.0;

		// timer Setup
		cTime = 0.;
		cFrame = 0;
		paused = true;
		playbackSpeed = 1.0;

		// default camera values
		fwd = false;
		lft = false;
		rht = false;
		bkw = false;

		// timing info
		thisTime = QDateTime::currentDateTimeUtc();
		lastTime = QDateTime::currentDateTimeUtc();

		// initialise the mouse clicker
		mousePicker = new MousePick(&(bvh->globalPositions), 1.0);

		// Construct Camera with default values
		camera = Camera();
		movingCamera = false;

		setMouseTracking(true);


	} // constructor

// destructor
RenderWidget::~RenderWidget()
	{ // destructor
	// nothing yet
	} // destructor

// called when OpenGL context is set up
void RenderWidget::initializeGL()
	{ // RenderWidget::initializeGL()
	// enable Z-buffering
	glEnable(GL_DEPTH_TEST);

	// set lighting parameters
	glShadeModel(GL_FLAT);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	glColor3f(1., 1., 1.);

	// background is a nice blue
	glClearColor(189. / 255., 215. / 255., 217. / 255., 1.0);
	} // RenderWidget::initializeGL()

// called every time the widget is resized
void RenderWidget::resizeGL(int w, int h)
	{ // RenderWidget::resizeGL()

	// save the size of the screen
	screenW = w;
	screenH = h;

	// reset the viewport
	glViewport(0, 0, w, h);

	// set projection matrix to be glOrtho based on zoom & window size
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();


	// compute the aspect ratio of the widget
	float aspectRatio = (float) w / (float) h;

	// depending on aspect ratio, set to accomodate a sphere of radius = diagonal without clipping
	std::cout << "AR: " << aspectRatio << '\n';
	std::cout << "Zoom: " << camera.Zoom << '\n';
	gluPerspective(camera.Zoom, aspectRatio, 0.1, 1000.);

	} // RenderWidget::resizeGL()

void RenderWidget::updatePerspective()
{ // updatePerspective()
	float aspectRatio = (float) screenW / (float) screenH;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(camera.Zoom, aspectRatio, 0.1, 1000.);

	std::cout << camera.Zoom << '\n';
} // updatePerspective()

// called every time the widget needs painting
void RenderWidget::paintGL()
	{ // RenderWidget::paintGL()
	// clear the buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set lighting on
	glEnable(GL_LIGHTING);

	// set model view matrix based on stored translation, rotation &c.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set light position first
	// retrieve rotation from arcball & apply
	glMultMatrixf(lightMatrix);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	// apply translation for interface control
	glLoadIdentity();

	// stop Qt stealing focus
	this->setFocus();




	// Apply Rotation From Camera
	GLfloat view[16];
	glm::mat4 viewIn = camera.GetViewMatrix();

	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{ // convert from Mat4 to float[16]
			view[i * 4 + j] = viewIn[i][j];
		}
	}
	glMultMatrixf(view);

	// translate based on animation
	Cartesian3 min = bvh->minCoords;
	Cartesian3 max = bvh->maxCoords;
	float xTrans = (max.x + min.x) / 2.;
	float yTrans = (max.y + min.y) / 2.;
	float zTrans = min.z;

	//std::cout << xTrans << " " << yTrans << " " << zTrans << '\n';
	glTranslatef(-xTrans, -yTrans, zTrans - 25.);

	// Render Skeleton At Current Frame
	bvh->RenderFigure(cFrame, 1.0, &camera);

	glTranslatef(xTrans, yTrans, -(zTrans - 25.));

	// render the control points
	bvh->RenderControlPoints();


	} // RenderWidget::paintGL()

// mouse-handling
void RenderWidget::mousePressEvent(QMouseEvent *event)
	{ // RenderWidget::mousePressEvent()
	// store the button for future reference
	whichButton = event->button();

	// find the minimum of height & width
	float size = (width() > height()) ? height() : width();

	float currX = (2.0 * event->x() - width()) / width();
	float currY = (height() - 2.0 * event->y() ) / height();

	// start moving the camera if we click
	if(whichButton == 1)
	{
		movingCamera = true;
	}
	camera.updateCameraVectors();

	// Perform Mouse Picking -1 if no match
	int clicked = mousePicker->click(currX, currY, &camera);

  // if clicked on nothign clear list
	if(clicked == -1)
	{
		bvh->activeJoints.clear();
	}
	else
	{
		if(parentWidget->shiftHeld)
		{
			std::cout << "Shift Helf" << '\n';
			bvh->activeJoints.push_back(clicked);
		}
		else
		{
			std::cout << "Shift Not Helf" << '\n';
			bvh->activeJoints.clear();
			bvh->activeJoints.push_back(clicked);
		}
	}

	// So we can see the newly highlighted joint
	updateGL();
	} // RenderWidget::mousePressEvent()

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{ // RenderWidget::mouseMoveEvent()
	// find the minimum of height & width
	float size = (width() > height()) ? height() : width();

	// TODO Mouse Move To Camera
	float currX = (2.0 * event->x() - width()) / width();
	float currY = (height() - 2.0 * event->y() ) / height();

	// rotate the camera if clicking
	// now either translate or rotate object or light
	if(mousePicker->dragging == true)
	{
		glm::vec3 mouseMove = mousePicker->drag(currX, currY, &camera);

		mouseMove.x = -mouseMove.x;
		mouseMove.y = -mouseMove.y;

		// TODO make this a sensetivity
		mouseMove *= 1000.0;

		// now contrain the axis based on current settings
		if(xAxis == false){ mouseMove.x = 0; }
		if(yAxis == false){ mouseMove.y = 0; }
		if(zAxis == false){ mouseMove.z = 0; }


		bvh->MoveJoint(bvh->activeJoints[0], mouseMove);

		// if(doneOnce != true)
		// {
		//
		// 	std::cout << "Mouse Movement" << '\n';
		// 	for(int i = 0; i < 3; i++)
		// 	{
		// 		std::cout << mouseMove[i] << "  ";
		// 	}
		// 	// global positoin before
		// 	std::cout << "Global Positoin" << '\n';
		// 	std::cout << bvh->joints[22]->name << '\n';
		// 	std::cout << bvh->globalPositions[22*3] << " " << bvh->globalPositions[22*3 + 1] << " " << bvh->globalPositions[22*3 + 2] << '\n';
		//
		// 	// move one joint
		// 	bvh->MoveJoint(mousePicker->closest, mouseMove, 1);
		//
		//
		// 	doneOnce = true;
		// }
		// else
		// {
		// 	std::cout << "Global Positoin After" << '\n';
		// 	std::cout << bvh->joints[22]->name << '\n';
		// 	std::cout << bvh->globalPositions[22*3] << " " << bvh->globalPositions[22*3 + 1] << " " << bvh->globalPositions[22*3 + 2] << '\n';
		//
		// }

		// TODO
		paintGL();
		updateGL();
	}

	// only move the camera if we are not dragging
	else if(movingCamera)
	{
		camera.ProcessMouseMovement((mouseLastX - currX) * 200., (mouseLastY - currY) * 200.);
		paintGL();
		updateGL();
	}

	// Update
	mouseLastY = currY;
	mouseLastX = currX;
} // RenderWidget::mouseMoveEvent()


void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
	{ // RenderWidget::mouseReleaseEvent()
	// now either translate or rotate object or light
	mousePicker->dragging = false; // stop from dragging
	movingCamera = false; // camera will not move anymore

	} // RenderWidget::mouseReleaseEvent()

	QSize RenderWidget::minimumSizeHint()
	{
		return QSize(50, 50);
	}


	QSize RenderWidget::sizeHint()
	{
		return QSize(600, 600);
	}



void RenderWidget::loadButtonPressed()
{
	newFileName = QFileDialog::getOpenFileName(this,
    tr("Open BVH File"), "../animFiles", tr("Anim Files (*.bvh)"));
  std::cout << "Reading: ";
	std::cout << newFileName.toStdString() << '\n';

	if(newFileName.toStdString().size() > 0)
	{
		// reset default values
		zoom = 1.0;
		cTime = 0.;
		cFrame = 0;

		bvh = new BVH(newFileName.toStdString().c_str());
		bvh->FindMinMax();

		// initialise the mouse clicker
		mousePicker = new MousePick(&(bvh->globalPositions), 1.0);

		// TODO
		// reset the camera here
		updateGL();
		paintGL();
	}
}

void RenderWidget::saveButtonPressed()
{
	// if we need to start playing the animation again
	bool reset = false;
	// pause the animation
	if(paused == false){ paused = true;; reset = true; }


	// get the filename
  QString fileName = QFileDialog::getSaveFileName(this,
                     tr("Save .bvh File"), tr(".bvh"),
                     tr("BVH (*.bvh)"));
  std::cout << "fileName: " << fileName.toStdString() << '\n';

	if(fileName.toStdString().length() > 0)
	{
		bvh->SaveFile(fileName.toStdString());
	}

	if(reset)
	{
		paused = false;
	}



}


// Update the timer and calculate the current frame of animation
void RenderWidget::timerUpdate()
{
	// Control Camera Movement
	thisTime = QDateTime::currentDateTimeUtc();


	qint64 delta = lastTime.msecsTo(thisTime);


	bool updateNeeded = false;

	if(fwd){ camera.ProcessKeyboard(FORWARD , delta / 300.); }
	if(lft){ camera.ProcessKeyboard(LEFT    , delta / 300.); }
	if(rht){ camera.ProcessKeyboard(RIGHT   , delta / 300.); }
	if(bkw){ camera.ProcessKeyboard(BACKWARD, delta / 300.); }
	if(upp){ camera.ProcessKeyboard(UP      , delta / 300.); }
	if(dwn){ camera.ProcessKeyboard(DOWN    , delta / 300.); }

	if(fwd || lft || rht || bkw || upp || dwn)
	{
		updateNeeded = true;
	}

	// Control Playback
	if(paused == false)
	{
		// MS Conversion
		cTime += delta * playbackSpeed;
		int frame = (int)((cTime / (bvh->interval * 1000))) % (int)(bvh->numFrame);

		// Only Update If We need To
		if(frame != cFrame)
		{
			cFrame = frame;
			updateNeeded = true;


		}
	}

	// Update Timer
	lastTime = thisTime;

	// update the frame if needed
	if(updateNeeded)
	{
		updateGL();
	}
	parentWidget->updateText(cFrame, playbackSpeed);
}
