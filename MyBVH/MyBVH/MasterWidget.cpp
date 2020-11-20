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
    QGroupBox   *saveLoadGroup     = new QGroupBox(tr("Save/Load"));
    QPushButton *loadButton        = new QPushButton("Load", this);
    QPushButton *saveButton        = new QPushButton("Save", this);
    QLabel      *addFramesLabel    = new QLabel(tr("Frames: "));
                 addFramesSpinBox  = new QSpinBox;
    QPushButton *newKeyframeButton = new QPushButton("Insert Keyframe", this);
    QPushButton *setKeyframeButton = new QPushButton("Set Keyframe", this);
    QPushButton *lerpKeyframeButton = new QPushButton("Lerp Keyframes", this);
    QVBoxLayout *saveLoadLayout    = new QVBoxLayout;

    addFramesSpinBox->setRange(0, 1000);
    addFramesSpinBox->setSingleStep(1);
    addFramesSpinBox->setValue(30);

    saveLoadLayout->addWidget(loadButton);
    saveLoadLayout->addWidget(saveButton);
    saveLoadLayout->addWidget(addFramesLabel);
    saveLoadLayout->addWidget(addFramesSpinBox);
    saveLoadLayout->addWidget(newKeyframeButton);
    saveLoadLayout->addWidget(setKeyframeButton);
    saveLoadLayout->addWidget(lerpKeyframeButton);
    saveLoadGroup ->setLayout(saveLoadLayout);


    // IK Control
    QGroupBox   *IKGroup              = new QGroupBox(tr("IK Control"));
                 toggleIKCheck        = new QCheckBox();
                 toggleDampeningCheck = new QCheckBox();
                 toggleControlCheck   = new QCheckBox();
                 lamdbaSpinBox        = new QSpinBox();
    QLabel      *lambdaSpinBoxLabel   = new QLabel(tr("Lambda"));
                 xGainSpinBox         = new QSpinBox();
    QLabel      *xGainSpinBoxLabel    = new QLabel(tr("X Gain"));
                 yGainSpinBox         = new QSpinBox();
    QLabel      *yGainSpinBoxLabel    = new QLabel(tr("Y Gain"));
                 zGainSpinBox         = new QSpinBox();
    QLabel      *zGainSpinBoxLabel    = new QLabel(tr("Z Gain"));
    QVBoxLayout *IKLayout             = new QVBoxLayout;

    toggleDampeningCheck ->setText("Dampening");
    toggleControlCheck   ->setText("Control");
    toggleIKCheck        ->setText("Rotations");
    lamdbaSpinBox        ->setRange(0, 1000);
    lamdbaSpinBox        ->setSingleStep(1);
    lamdbaSpinBox        ->setValue(1);
    xGainSpinBox         ->setRange(0, 1000);
    xGainSpinBox         ->setSingleStep(1);
    xGainSpinBox         ->setValue(1);
    yGainSpinBox         ->setRange(0, 1000);
    yGainSpinBox         ->setSingleStep(1);
    yGainSpinBox         ->setValue(1);
    zGainSpinBox         ->setRange(0, 1000);
    zGainSpinBox         ->setSingleStep(1);
    zGainSpinBox         ->setValue(1);

    IKLayout->addWidget(toggleIKCheck);
    IKLayout->addWidget(toggleDampeningCheck);
    IKLayout->addWidget(toggleControlCheck);
    IKLayout->addWidget(lambdaSpinBoxLabel);
    IKLayout->addWidget(lamdbaSpinBox);
    IKLayout->addWidget(xGainSpinBoxLabel);
    IKLayout->addWidget(xGainSpinBox);
    IKLayout->addWidget(yGainSpinBoxLabel);
    IKLayout->addWidget(yGainSpinBox);
    IKLayout->addWidget(zGainSpinBoxLabel);
    IKLayout->addWidget(zGainSpinBox);
    IKGroup ->setLayout(IKLayout);

    // playback
    QGroupBox   *playbackGroup       = new QGroupBox(tr("Playback"));
    QVBoxLayout *playbackGroupLayout = new QVBoxLayout;

    playbackSpeedLabel  = new QLabel(this);
    currentFrameLabel   = new QLabel(this);
    axisConstraintLabel = new QLabel(this);
    playbackSpeedLabel ->setText("Playback Speed: 1x");
    currentFrameLabel  ->setText("Current Frame: 1");
    axisConstraintLabel->setText("X: True\nY: True\nZ: True");

    playbackGroupLayout->addWidget(playbackSpeedLabel);
    playbackGroupLayout->addWidget(currentFrameLabel);
    playbackGroupLayout->addWidget(axisConstraintLabel);
    playbackGroup      ->setLayout(playbackGroupLayout);


    // play, pause, stop, etc.
    QGroupBox   *playbackButtonsGroup  = new QGroupBox(tr("Playback Buttons"));
    QHBoxLayout *playbackButtonsLayout = new QHBoxLayout;
    QPushButton *rewindButton          = new QPushButton;
    QPushButton *stopButton            = new QPushButton;
    QPushButton *playButton            = new QPushButton;
    QPushButton *pauseButton           = new QPushButton;
    QPushButton *fastForwardButton     = new QPushButton;

    rewindButton     ->setIcon(QIcon("../icons/rewind.png"));
    stopButton       ->setIcon(QIcon("../icons/stop.png"));
    playButton       ->setIcon(QIcon("../icons/play.png"));
    pauseButton      ->setIcon(QIcon("../icons/pause.png"));
    fastForwardButton->setIcon(QIcon("../icons/fastforward.png"));

    playbackButtonsLayout->addWidget(rewindButton);
    playbackButtonsLayout->addWidget(stopButton);
    playbackButtonsLayout->addWidget(playButton);
    playbackButtonsLayout->addWidget(pauseButton);
    playbackButtonsLayout->addWidget(fastForwardButton);
    playbackButtonsGroup ->setLayout(playbackButtonsLayout);

    // all ui group
    QGroupBox   *allUI       = new QGroupBox("Options");
    QVBoxLayout *allUILayout = new QVBoxLayout;

    allUILayout->addWidget(saveLoadGroup);
    allUILayout->addWidget(playbackGroup);
    allUILayout->addWidget(IKGroup);
    allUILayout->addWidget(playbackButtonsGroup);
    allUI      ->setLayout(allUILayout);
    allUI      ->setMaximumWidth(300);

    // render box group
    QGroupBox   *renderGroup       = new QGroupBox("Render");
    QVBoxLayout *renderGroupLayout = new QVBoxLayout;

    renderGroupLayout->addWidget(renderWidget);
    renderGroup      ->setLayout(renderGroupLayout);
    renderGroup      ->setMinimumWidth(600);
    renderGroup      ->setMinimumHeight(600);

    // Master layout
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(renderGroup, 0, 1);
    mainLayout->addWidget(allUI, 0, 0);
    setLayout(mainLayout);

    // Set the Title
    setWindowTitle(tr("BVH Editor"));

    // Keyboard Shortcuts
    QShortcut *playShortcut        = new QShortcut(QKeySequence("space"), this);
    QShortcut *stopShortcut        = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    QShortcut *fastForwardShortcut = new QShortcut(QKeySequence("Shift+."), this);
    QShortcut *rewindShortcut      = new QShortcut(QKeySequence("Shift+,"), this);

    // connecting keyboard shortcuts
    QObject::connect(playShortcut,        SIGNAL(activated()), this, SLOT(playPause()));
    QObject::connect(stopShortcut,        SIGNAL(activated()), this, SLOT(stop()));
    QObject::connect(fastForwardShortcut, SIGNAL(activated()), this, SLOT(fastForward()));
    QObject::connect(rewindShortcut,      SIGNAL(activated()), this, SLOT(rewind()));

    // Connecting
    connect(loadButton,           SIGNAL(pressed()),      renderWidget, SLOT(loadButtonPressed()));
    connect(saveButton,           SIGNAL(pressed()),      renderWidget, SLOT(saveButtonPressed()));
    connect(newKeyframeButton,    SIGNAL(pressed()),      this,         SLOT(addKeyframe()));
    connect(setKeyframeButton,    SIGNAL(pressed()),      this,         SLOT(setKeyframe()));
    connect(lerpKeyframeButton,   SIGNAL(pressed()),      this,         SLOT(lerpKeyframe()));
    connect(toggleIKCheck,        SIGNAL(pressed()),      this,         SLOT(toggleIK()));
    connect(toggleDampeningCheck, SIGNAL(pressed()),      this,         SLOT(toggleDampening()));
    connect(toggleControlCheck,   SIGNAL(pressed()),      this,         SLOT(toggleControl()));
    connect(lamdbaSpinBox,        SIGNAL(valueChanged(int)), this,      SLOT(lambdaUpdate(int)));
    connect(xGainSpinBox,         SIGNAL(valueChanged(int)), this,      SLOT(xGainUpdate(int)));
    connect(yGainSpinBox,         SIGNAL(valueChanged(int)), this,      SLOT(yGainUpdate(int)));
    connect(zGainSpinBox,         SIGNAL(valueChanged(int)), this,      SLOT(zGainUpdate(int)));
    connect(rewindButton,         SIGNAL(pressed()),      this,         SLOT(rewind()));
    connect(stopButton,           SIGNAL(pressed()),      this,         SLOT(stop()));
    connect(playButton,           SIGNAL(pressed()),      this,         SLOT(play()));
    connect(pauseButton,          SIGNAL(pressed()),      this,         SLOT(pause()));
    connect(fastForwardButton,    SIGNAL(pressed()),      this,         SLOT(fastForward()));
    connect(timer,                SIGNAL(timeout()),      renderWidget, SLOT(timerUpdate()));
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
        if(renderWidget->cFrame < renderWidget->bvh->numFrame-1 ){ renderWidget->cFrame += 1; renderWidget->cTime += renderWidget->bvh->interval * 1000; }
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
 renderWidget->cFrame = 0.;
 renderWidget->cTime = 0;
 renderWidget->playbackSpeed = 1.;


 // we also reset the axis constraints
 renderWidget->xAxis = true;
 renderWidget->yAxis = true;
 renderWidget->zAxis = true;

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

  currentFrameLabel->setText(QString::fromStdString("Current Frame: " + std::to_string(frameNo + 1)));
  playbackSpeedLabel->setText(QString::fromStdString(playback));
  axisConstraintLabel->setText(QString::fromStdString(axis));
}

// adds a new keyframe to the animation
void MasterWidget::addKeyframe()
{
  int newFrames = addFramesSpinBox->value();
  renderWidget->bvh->AddKeyFrame(newFrames);
  renderWidget->cFrame += newFrames;
}

// sets the current frame to be a keyframe
void MasterWidget::setKeyframe()
{
  renderWidget->bvh->SetKeyFrame();
}

// sets the current frame to be a keyframe
void MasterWidget::lerpKeyframe()
{
  renderWidget->bvh->LerpKeyframes();
}

// toggles between Inverse Kinematics and Rotation
void MasterWidget::toggleIK()
{
  if(renderWidget->bvh->moveMode == 0)
  {
    renderWidget->bvh->moveMode = 1;
    toggleIKCheck->setChecked(false);
  }
  else
  {
    renderWidget->bvh->moveMode = 0;
    toggleIKCheck->setChecked(true);
  }
}

// sets on dampening mode and turns off the other modes
void MasterWidget::toggleDampening()
{
  if(renderWidget->bvh->useDampening == false)
  {
    // enable dampening
    renderWidget->bvh->useDampening = true;
    toggleDampeningCheck->setChecked(true);

    // disable control
    renderWidget->bvh->useControl = false;
    toggleControlCheck->setChecked(false);
  }
  else
  {
    renderWidget->bvh->useDampening = false;
    toggleDampeningCheck->setChecked(false);
  }
}

// sets on control and turns off the dampening
void MasterWidget::toggleControl()
{
  if(renderWidget->bvh->useControl == false)
  {
    // enable control
    renderWidget->bvh->useControl = true;
    toggleControlCheck->setChecked(true);

    // disable dampening
    renderWidget->bvh->useDampening = false;
    toggleDampeningCheck->setChecked(false);
  }
  else
  {
    renderWidget->bvh->useControl = false;
    toggleControlCheck->setChecked(false);
  }
}

// update lambda in the bvh class
void MasterWidget::lambdaUpdate(int i)
{
  renderWidget->bvh->lambda = i;
}

// update xgain in bvh class
void MasterWidget::xGainUpdate(int i)
{
  renderWidget->bvh->xGain = (float)i;
}

// update ygain in bvh class
void MasterWidget::yGainUpdate(int i)
{
  renderWidget->bvh->yGain = (float)i;
}

// update zgain in bvh class
void MasterWidget::zGainUpdate(int i)
{
  renderWidget->bvh->zGain = (float)i;
}
