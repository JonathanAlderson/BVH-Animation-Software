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

#ifndef _GEOMETRIC_WIDGET_H
#define _GEOMETRIC_WIDGET_H

#include <QGLWidget>
#include <QMouseEvent>
#include <QFileDialog>
#include "MousePick.h"
#include "MasterWidget.h"
#include "BVH.h"
#include "camera.h"

class RenderWidget : public QGLWidget
	{ // class RenderWidget
	Q_OBJECT
	public:

	// our parent
	MasterWidget *parentWidget;

	// the actual BVH anim
	BVH * bvh;

	// for resolving clicks on the screen to 3D coordinates
	MousePick *mousePicker;

	// translation in window x,y
	GLfloat lastX, lastY;

	// which button was last pressed
	int whichButton;

	QString newFileName;

	float size;

	// can be changed by scrolling
	float zoom;

	// playback options
	float cTime;
	float startTime;
	int cFrame;
	bool paused;
	float playbackSpeed;

	// Lights
	const GLfloat lightMatrix[16] = {1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1.};
	const GLfloat light_position[4] = {0.0, 0.0, 1.0, 0.0};

	// Camera
	Camera camera;
	bool movingCamera;

	// Mouse
	float mouseLastX;
	float mouseLastY;

	// Action
	RenderWidget(char *filename, MasterWidget *parent);

	// destructor
	~RenderWidget();

	protected:
	// called when OpenGL context is set up
	void initializeGL();
	// called every time the widget is resized
	void resizeGL(int w, int h);
	// called every time the widget needs painting
	void paintGL();

	// mouse-handling
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);


	// Resizing functions
	QSize minimumSizeHint();
	QSize sizeHint();

	// gui controls
	public slots:
		void loadButtonPressed();
		void timerUpdate();


	}; // class RenderWidget

#endif
