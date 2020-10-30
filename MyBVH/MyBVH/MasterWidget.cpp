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
    QLabel *playbackSpeedLabel = new QLabel(this);
    playbackSpeedLabel->setText("Playback Speed: 1x");
    QLabel *currentFrameLabel = new QLabel(this);
    currentFrameLabel->setText("Current Frame: 1");

    playbackGroupLayout->addWidget(playbackSpeedLabel);
    playbackGroupLayout->addWidget(currentFrameLabel);
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

// Escape to quit
void MasterWidget::keyPressEvent(QKeyEvent* event)
{
  switch(event->key())
  {
    // Quit Program
    case Qt::Key_Escape:
      QCoreApplication::quit();
      break;
    // Move Camera Forwards
    case Qt::Key_W:
      renderWidget->camera.ProcessKeyboard(FORWARD, 0.1);
      renderWidget->updateGL();
      break;
    // Move Camera Left
    case Qt::Key_A:
      renderWidget->camera.ProcessKeyboard(LEFT, 0.1);
      renderWidget->updateGL();
      break;
    // Move Camera Backwards
    case Qt::Key_S:
      renderWidget->camera.ProcessKeyboard(BACKWARD, 0.1);
      renderWidget->updateGL();
      break;
    // Move Camera Right
    case Qt::Key_D:
      renderWidget->camera.ProcessKeyboard(RIGHT, 0.1);
      renderWidget->updateGL();
      break;
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
  }

  // Default
  QWidget::keyPressEvent(event);
}

// Zooming in/out
void MasterWidget::wheelEvent(QWheelEvent* event)
{
  QPoint numDegrees = event->angleDelta();

  if(numDegrees.y() > 0)
  {
    if(renderWidget->zoom < 2.){ renderWidget->zoom += 0.02;}
  }
  else
  {
    if(renderWidget->zoom > 0.1){ renderWidget->zoom -= 0.02; }
  }
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
