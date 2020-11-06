///////////////////////////////////////////////////
//
//	Jonathan Alderson
//	October, 2020
//
//	------------------------
//	MasterWidget.h
//	------------------------
//
//	The main widget that houses the GUI and the OpenGL
//
///////////////////////////////////////////////////

#ifndef _MASTER_WIDGET_H
#define _MASTER_WIDGET_H

#include <QGLWidget>
#include <QSlider>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QSpinBox>
#include <QShortcut>
#include <QWheelEvent>
#include <QCoreApplication>


class RenderWidget;

class MasterWidget : public QWidget
{
    Q_OBJECT

public:
    MasterWidget(char *filename, QWidget *parent);

    void keyPressEvent(QKeyEvent* event); // so we can escape to quit
    void keyReleaseEvent(QKeyEvent* event); // so we can control our camera nicely

    void wheelEvent(QWheelEvent* event); // zooms in/out

private:
    QSlider *createSlider();

    RenderWidget *renderWidget;

    QPushButton *loadButton;

    QTimer *timer;

    QLabel *currentFrameLabel;
    QLabel *playbackSpeedLabel;
    QLabel *axisConstraintLabel;

    bool shiftHeld;

// playback controls
public slots:
	void rewind();
	void fastForward();
  void pause();
  void play();
  void stop();
  void playPause();
  void updateText(int frameNo, float playbackSpeed);

};

#endif
