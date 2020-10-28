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

static GLfloat light_position[] = {0.0, 0.0, 1.0, 0.0};

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
		translate_x = translate_y = 0.0;

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

		// initialise arcballs to 80% of the widget's size
		Ball_Init(&lightBall);		Ball_Place(&lightBall, qOne, 0.80);
		Ball_Init(&objectBall);		Ball_Place(&objectBall, qOne, 0.80);

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
	// if (aspectRatio > 1.0)
	// 	glOrtho(-aspectRatio * size, aspectRatio * size, -size, size, -size, size);
	// else
	// 	glOrtho(-size, size, -size/aspectRatio, size/aspectRatio, -size, size);
	std::cout << "AR: " << aspectRatio << '\n';
	gluPerspective(60., aspectRatio, 0.1, 1000.);

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
	GLfloat mNow[16];
	Ball_Value(&lightBall, mNow);
	glMultMatrixf(mNow);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	// apply translation for interface control
	glLoadIdentity();
	glTranslatef(translate_x, translate_y, 0.0);

	// translate based on animation
	Cartesian3 min = bvh->minCoords;
	Cartesian3 max = bvh->maxCoords;
	float xTrans = (max.x + min.x) / 2.;
	float yTrans = (max.y + min.y) / 2.;
	float zTrans = min.z;

	//std::cout << xTrans << " " << yTrans << " " << zTrans << '\n';
	glTranslatef(-xTrans, -yTrans, zTrans - 25.);


	// apply rotation matrix from arcball
	Ball_Value(&objectBall, mNow);
	glMultMatrixf(mNow);


	//glTranslatef(0., 0., -(float)cFrame);

	// test render
	bvh->RenderFigure(cFrame, zoom);

	} // RenderWidget::paintGL()

// mouse-handling
void RenderWidget::mousePressEvent(QMouseEvent *event)
	{ // RenderWidget::mousePressEvent()
	// store the button for future reference
	whichButton = event->button();

	// find the minimum of height & width
	float size = (width() > height()) ? height() : width();

	// convert to the ArcBall's vector type
	HVect vNow;

	// scale both coordinates from that
	vNow.x = (2.0 * event->x() - width()) / size;
	vNow.y = (height() - 2.0 * event->y() ) / size;

	// Perform Mouse Picking
	if(mousePicker->click(vNow.x, vNow.y) == -1) // if we arn't dragging a grid
	{
		// now either translate or rotate object or light
		switch(whichButton)
		{ // button switch
			case Qt::RightButton:
			// save the last x, y
			last_x = vNow.x; last_y = vNow.y;
			// and update
			updateGL();
			break;
			case Qt::MiddleButton:
			// pass the point to the arcball code
			Ball_Mouse(&lightBall, vNow);
			// start dragging
			Ball_BeginDrag(&lightBall);
			// update the widget
			updateGL();
			break;
			case Qt::LeftButton:
			// pass the point to the arcball code
			Ball_Mouse(&objectBall, vNow);
			// start dragging
			Ball_BeginDrag(&objectBall);
			// update the widget
			updateGL();
			break;
		} // button switch
	}
	else
	{
	}

	} // RenderWidget::mousePressEvent()

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
	{ // RenderWidget::mouseMoveEvent()
	// find the minimum of height & width
	float size = (width() > height()) ? height() : width();

	// convert to the ArcBall's vector type
	HVect vNow;

	// scale both coordinates from that
	vNow.x = (2.0 * event->x() - width()) / size;
	vNow.y = (height() - 2.0 * event->y() ) / size;

	// now either translate or rotate object or light
	if(mousePicker->dragging == false)
	{
		switch(whichButton)
		{ // button switch
			case Qt::RightButton:
			// subtract the translation
			translate_x += vNow.x - last_x;
			translate_y += vNow.y - last_y;
			last_x = vNow.x;
			last_y = vNow.y;
			// update the widget
			updateGL();
			break;
			case Qt::MiddleButton:
			// pass it to the arcball code
			Ball_Mouse(&lightBall, vNow);
			// start dragging
			Ball_Update(&lightBall);
			// update the widget
			updateGL();
			break;
			case Qt::LeftButton:
			// pass it to the arcball code
			Ball_Mouse(&objectBall, vNow);
			// start dragging
			Ball_Update(&objectBall);
			// update the widget
			updateGL();
			break;
		} // button switch
	}
	// Adjust one of the grid points
	else
	{
		// TODO
		paintGL();
		updateGL();
	}
	} // RenderWidget::mouseMoveEvent()

void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
	{ // RenderWidget::mouseReleaseEvent()
	// now either translate or rotate object or light
	mousePicker->dragging = false; // stop from dragging

	switch(whichButton)
		{ // button switch
		case Qt::RightButton:
			// just update
			updateGL();
			break;
		case Qt::MiddleButton:
			// end the drag
			Ball_EndDrag(&lightBall);
			// update the widget
			updateGL();
			break;
		case Qt::LeftButton:
			// end the drag
			Ball_EndDrag(&objectBall);
			// update the widget
			updateGL();
			break;
		} // button switch
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
		translate_x = translate_y = 0.0;
		zoom = 1.0;
		cTime = 0.;
		cFrame = 0;

		bvh = new BVH(newFileName.toStdString().c_str());
		bvh->FindMinMax();

		// initialise the mouse clicker
		mousePicker = new MousePick(&(bvh->globalPositions), 1.0);

		// reset the ball
		Ball_Init(&objectBall);		Ball_Place(&objectBall, qOne, 0.80);

		updateGL();
		paintGL();

	}
}

void RenderWidget::timerUpdate()
{
	if(paused == false)
	{
		cTime += 16 * playbackSpeed;

		int frame = (int)((cTime / (bvh->interval * 1000)) * playbackSpeed) % (int)(bvh->numFrame);

		if(frame != cFrame)
		{
			cFrame = frame;
			updateGL();
			std::cout << frame << '\n';
			//std::cout << "CTime: " << cTime << " cFrame: " << cFrame << '\n';
		}
	}
}
