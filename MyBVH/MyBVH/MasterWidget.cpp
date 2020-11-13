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

#include "MasterWidget.h"
#include "RenderWidget.h"

MasterWidget::MasterWidget(char *filename, QWidget *parent)
{
    // Setup the Render Widget, passing the filename
    renderWidget = new RenderWidget(filename, this);
    renderWidget->setParent(this);

    // Timer for running animation
    timer = new QTimer(this);

    // default value
    shiftHeld = false;
    renderWidget->dwn = false;
    renderWidget->upp = false;

    // GUI Layout

    // Save/Load
    QGroupBox *saveLoadGroup = new QGroupBox(tr("Save/Load"));
    QPushButton *loadButton = new QPushButton("Load", this);
    QVBoxLayout *saveLoadLayout = new QVBoxLayout;
    saveLoadLayout->addWidget(loadButton);
    saveLoadGroup->setLayout(saveLoadLayout);

    // playback
    QGroupBox *playbackGroup = new QGroupBox(tr("Playback"));
    QVBoxLayout *playbackGroupLayout = new QVBoxLayout;
    playbackSpeedLabel = new QLabel(this);
    playbackSpeedLabel->setText("Playback Speed: 1x");
    currentFrameLabel = new QLabel(this);
    currentFrameLabel->setText("Current Frame: 1");
    axisConstraintLabel = new QLabel(this);
    axisConstraintLabel->setText("X: True\nY: True\nZ: True");

    playbackGroupLayout->addWidget(playbackSpeedLabel);
    playbackGroupLayout->addWidget(currentFrameLabel);
    playbackGroupLayout->addWidget(axisConstraintLabel);
    playbackGroup->setLayout(playbackGroupLayout);


    // play, pause, stop, etc.
    QGroupBox *playbackButtonsGroup = new QGroupBox(tr("Playback Buttons"));
    QHBoxLayout *playbackButtonsLayout = new QHBoxLayout;
    QPushButton *rewindButton = new QPushButton;
    rewindButton->setIcon(QIcon("../icons/rewind.png"));
    QPushButton *stopButton = new QPushButton;
    stopButton->setIcon(QIcon("../icons/stop.png"));
    QPushButton *playButton = new QPushButton;
    playButton->setIcon(QIcon("../icons/play.png"));
    QPushButton *pauseButton = new QPushButton;
    pauseButton->setIcon(QIcon("../icons/pause.png"));
    QPushButton *fastForwardButton = new QPushButton;
    fastForwardButton->setIcon(QIcon("../icons/fastforward.png"));

    playbackButtonsLayout->addWidget(rewindButton);
    playbackButtonsLayout->addWidget(stopButton);
    playbackButtonsLayout->addWidget(playButton);
    playbackButtonsLayout->addWidget(pauseButton);
    playbackButtonsLayout->addWidget(fastForwardButton);
    playbackButtonsGroup->setLayout(playbackButtonsLayout);

    // all ui group
    QGroupBox *allUI = new QGroupBox("Options");
    QVBoxLayout *allUILayout = new QVBoxLayout;
    allUILayout->addWidget(saveLoadGroup);
    allUILayout->addWidget(playbackGroup);
    allUILayout->addWidget(playbackButtonsGroup);
    allUI->setLayout(allUILayout);
    allUI->setMaximumWidth(300);

    // render box group
    QGroupBox *renderGroup = new QGroupBox("Render");
    QVBoxLayout *renderGroupLayout = new QVBoxLayout;
    renderGroupLayout->addWidget(renderWidget);
    renderGroup->setLayout(renderGroupLayout);
    renderGroup->setMinimumWidth(600);
    renderGroup->setMinimumHeight(600);

    // Master layout
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(renderGroup, 0, 1);
    mainLayout->addWidget(allUI, 0, 0);
    setLayout(mainLayout);

    // Set the Title
    setWindowTitle(tr("BVH Editor"));

    // Keyboard Shortcuts
    QShortcut *playShortcut = new QShortcut(QKeySequence("space"), this);
    QShortcut *stopShortcut = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    QShortcut *fastForwardShortcut = new QShortcut(QKeySequence("Shift+."), this);
    QShortcut *rewindShortcut = new QShortcut(QKeySequence("Shift+,"), this);
    QObject::connect(playShortcut, SIGNAL(activated()), this, SLOT(playPause()));
    QObject::connect(stopShortcut, SIGNAL(activated()), this, SLOT(stop()));
    QObject::connect(fastForwardShortcut, SIGNAL(activated()), this, SLOT(fastForward()));
    QObject::connect(rewindShortcut, SIGNAL(activated()), this, SLOT(rewind()));

    // Connecting
    connect(loadButton, SIGNAL(pressed()), renderWidget, SLOT(loadButtonPressed()));
    connect(rewindButton, SIGNAL(pressed()), this, SLOT(rewind()));
    connect(stopButton, SIGNAL(pressed()), this, SLOT(stop()));
    connect(playButton, SIGNAL(pressed()), this, SLOT(play()));
    connect(pauseButton, SIGNAL(pressed()), this, SLOT(pause()));
    connect(fastForwardButton, SIGNAL(pressed()), this, SLOT(fastForward()));
    connect(timer, SIGNAL(timeout()), renderWidget, SLOT(timerUpdate()));
    timer->start(16);

}

// Default slider params
QSlider *MasterWidget::createSlider()
{
    QSlider *slider = new QSlider(Qt::Vertical);
    slider->setRange(0, 360 * 16);
    slider->setSingleStep(16);
    slider->setPageStep(15 * 16);
    slider->setTickInterval(15 * 16);
    slider->setTickPosition(QSlider::TicksRight);
    return slider;
}

// Escape to quit and camera start moving
void MasterWidget::keyPressEvent(QKeyEvent* event)
{
  switch(event->key())
  {
    // Quit Program
    case Qt::Key_Escape:
      QCoreApplication::quit();
      break;

    ////////////////////////////////////////////
    //  CAMERA MOVEMENT
    ///////////////////////////////////////////

    // Move Camera Forwards
    case Qt::Key_W:
      if(shiftHeld){ renderWidget->upp = true; }
      else         { renderWidget->fwd = true; }
      break;
    // Move Camera Left
    case Qt::Key_A:
      renderWidget->lft = true;
      break;
    // Move Camera Backwards
    case Qt::Key_S:
      if(shiftHeld){ renderWidget->dwn = true; }
      else         { renderWidget->bkw = true; }
      break;
    // Move Camera Right
    case Qt::Key_D:
      renderWidget->rht = true;
      break;
    ////////////////////////////////////////////
    //  PLAYBACK CONTROLS
    ///////////////////////////////////////////
    // Step Backwards A Single Frame
    case Qt::Key_Q:
      if(renderWidget->paused == true)
      {
        if(renderWidget->cFrame > 0){ renderWidget->cFrame -= 1; renderWidget->cTime -= renderWidget->bvh->interval * 1000; }
        renderWidget->updateGL();
      }
      break;
    // Step Forwards A Single Frame
    case Qt::Key_E:
      if(renderWidget->paused == true)
      {
        if(renderWidget->cFrame < renderWidget->bvh->numFrame ){ renderWidget->cFrame += 1; renderWidget->cTime += renderWidget->bvh->interval * 1000; }
        renderWidget->updateGL();
      }
      break;
      ////////////////////////////////////////////
      //  AXIS CONSTRAINTS
      ///////////////////////////////////////////
      case Qt::Key_X:
        if(shiftHeld){ renderWidget->xAxis = false; renderWidget->yAxis = true; renderWidget->zAxis = true; }
        else         { renderWidget->xAxis = true; renderWidget->yAxis = false; renderWidget->zAxis = false;   }
        break;
      case Qt::Key_Y:
        if(shiftHeld){ renderWidget->xAxis = true; renderWidget->yAxis = false; renderWidget->zAxis = true; }
        else         { renderWidget->xAxis = false; renderWidget->yAxis = true; renderWidget->zAxis = false; }
        break;
      case Qt::Key_Z:
        if(shiftHeld){ renderWidget->xAxis = true; renderWidget->yAxis = true; renderWidget->zAxis = false; }
        else         { renderWidget->xAxis = false; renderWidget->yAxis = false; renderWidget->zAxis = true; }
        break;
      case Qt::Key_Shift:
        shiftHeld = true;
        break;
  }

  // Default
  QWidget::keyPressEvent(event);
}

// for stopping camera movement
void MasterWidget::keyReleaseEvent(QKeyEvent* event)
{
  switch(event->key())
  {
    ////////////////////////////////////////////
    //  CAMERA MOVEMENT
    ///////////////////////////////////////////
    // Quit Program
    case Qt::Key_Escape:
      QCoreApplication::quit();
      break;
    // Move Camera Forwards
    case Qt::Key_W:
      renderWidget->upp = false;
      renderWidget->fwd = false;
      break;
    // Move Camera Left
    case Qt::Key_A:
      renderWidget->lft = false;
      break;
    // Move Camera Backwards
    case Qt::Key_S:
      renderWidget->dwn = false;
      renderWidget->bkw = false;
      break;
    // Move Camera Right
    case Qt::Key_D:
      renderWidget->rht = false;
      break;

    // bool for rotation and movement locking
    case Qt::Key_Shift:
      shiftHeld = false;
      renderWidget->upp = false;
      renderWidget->dwn = false;
      break;
  }

  // Default
  QWidget::keyReleaseEvent(event);
}

// Zooming in/out
void MasterWidget::wheelEvent(QWheelEvent* event)
{
  QPoint numDegrees = event->angleDelta();

  if(numDegrees.y() > 0)
  {
    if(renderWidget->zoom < 2.){ renderWidget->camera.ProcessMouseScroll(1);}
  }
  else
  {
    if(renderWidget->zoom > 0.1){ renderWidget->camera.ProcessMouseScroll(-1); }
  }
  renderWidget->updatePerspective();
  renderWidget->updateGL();

}

// Halfes playback speed, if playing backwards will speed up
void MasterWidget::rewind()
{
  float ps = renderWidget->playbackSpeed;

  // Start playing backwards
  if(ps == 0.25){ renderWidget->playbackSpeed = -0.25; return; }

  // make bigger if negative, make smaller if positive
  if(ps > 0){ renderWidget->playbackSpeed /= 2.; }
  if(ps < 0){ renderWidget->playbackSpeed *= 2.; }
}

// Doubles playback speed, if playing backwards will slow down
void MasterWidget::fastForward()
{
  float ps = renderWidget->playbackSpeed;

  // Start playing backwards
  if(ps == -0.25){ renderWidget->playbackSpeed = 0.25; return; }

  // make bigger if negative, make smaller if positive
  if(ps > 0){ renderWidget->playbackSpeed *= 2.; }
  if(ps < 0){ renderWidget->playbackSpeed /= 2.; }
}

// Pauses The Current Animation
void MasterWidget::pause()
{
  renderWidget->paused = true;
}

// Plays The Current Animation
void MasterWidget::play()
{
  renderWidget->paused = false;
}

// Changes the boolean
void MasterWidget::playPause()
{
  bool p = renderWidget->paused;
  if(p){ renderWidget->paused = false; return; }
  else{ renderWidget->paused = true; return; }

}

// Stops playback of the current animation
void MasterWidget::stop()
{
 renderWidget->paused = true;
 renderWidget->cFrame = 16.;
 renderWidget->cTime = 0;
 renderWidget->playbackSpeed = 1.;
 renderWidget->updateGL();
}

// updates the UI to show the playback
void MasterWidget::updateText(int frameNo, float playbackSpeed)
{
  string axis = "X: ";
  if(renderWidget->xAxis){ axis += "True\nY: " ;}
  else                   { axis += "False\nY: ";}
  if(renderWidget->yAxis){ axis += "True\nZ: " ;}
  else                   { axis += "False\nZ: ";}
  if(renderWidget->zAxis){ axis += "True"      ;}
  else                   { axis += "False "    ;}

  // can't convert to 2dp
  string playback = "Playback Speed: " + std::to_string((float)((int)(playbackSpeed * 100.)) / 100.) + "x";

  currentFrameLabel->setText(QString::fromStdString("Current Frame: " + std::to_string(frameNo)));
  playbackSpeedLabel->setText(QString::fromStdString(playback));
  axisConstraintLabel->setText(QString::fromStdString(axis));
}
