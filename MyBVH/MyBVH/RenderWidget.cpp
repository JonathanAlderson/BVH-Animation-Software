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

		std::cout << "COORDS" << '\n';
		std::cout << bvh->minCoords << '\n';
		std::cout << bvh->maxCoords << '\n';
		std::cout << bvh->boundingBoxSize << '\n';
		std::cout << "---------" << '\n';

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

	// background is a nice blue
	glClearColor(189. / 255., 215. / 255., 217. / 255., 1.0);
	} // RenderWidget::initializeGL()

// called every time the widget is resized
void RenderWidget::resizeGL(int w, int h)
	{ // RenderWidget::resizeGL()
	// reset the viewport
	glViewport(0, 0, w, h);

	// set projection matrix to be glOrtho based on zoom & window size
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();


	// compute the aspect ratio of the widget
	float aspectRatio = (float) w / (float) h;

	// depending on aspect ratio, set to accomodate a sphere of radius = diagonal without clipping
	std::cout << "AR: " << aspectRatio << '\n';
	gluPerspective(camera.Zoom, aspectRatio, 0.1, 1000.);

	} // RenderWidget::resizeGL()

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
	bvh->RenderFigure(cFrame, zoom);

	} // RenderWidget::paintGL()

// mouse-handling
void RenderWidget::mousePressEvent(QMouseEvent *event)
	{ // RenderWidget::mousePressEvent()
	// store the button for future reference
	whichButton = event->button();

	// find the minimum of height & width
	float size = (width() > height()) ? height() : width();

	// TODO Mouse Move To Camera
	float currX = event->x();    // (2.0 * event->x() - width()) / size
	float currY = event->y();    // (height() - 2.0 * event->y() ) / size;

	// start moving the camera if we click
	if(whichButton == 1)
	{
		movingCamera = true;
	}

	// Perform Mouse Picking
	if(mousePicker->click(currX, currY) != -1) // if are dragging a point
	{
			// TODO
			// Move Object
	}


	} // RenderWidget::mousePressEvent()

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{ // RenderWidget::mouseMoveEvent()
	// find the minimum of height & width
	float size = (width() > height()) ? height() : width();

	// TODO Mouse Move To Camera
	float currX = (2.0 * event->x() - width()) / size;
	float currY = (height() - 2.0 * event->y() ) / size;

	// rotate the camera if clicking
	if(movingCamera)
	{
		camera.ProcessMouseMovement((mouseLastX - currX) * 200., (mouseLastY - currY) * 200.);
		paintGL();
		updateGL();
	}

	// Move Correct Model Part

	// now either translate or rotate object or light
	if(mousePicker->dragging == true)
	{
		// TODO
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

// Update the timer and calculate the current frame of animation
void RenderWidget::timerUpdate()
{
	if(paused == false)
	{
		// MS Conversion
		cTime += 16 * playbackSpeed;
		int frame = (int)((cTime / (bvh->interval * 1000))) % (int)(bvh->numFrame);

		// Only Update If We need To
		if(frame != cFrame)
		{
			cFrame = frame;
			updateGL();
			// set the frame text in the parent window
			// and playback speed in the parent window
		}
	}
}
