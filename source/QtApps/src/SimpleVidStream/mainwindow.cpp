
/////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2022 Andrew Kelly
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
/////////////////////////////////////////////////////////////////////////////////

#include "control_main.hpp"
#include <mainwindow.h>
#include <./ui_mainwindow.h>
#include <QPushButton>
#include <QCloseEvent>
#include <QMessageBox>
#include <QThread>
#include <MainLogger.hpp>
#include <vidcap_capture_thread.hpp>
#include <condition_data.hpp>
#include <unistd.h>
#include <memory>

MainWindow::MainWindow(std::shared_ptr<Log::Logger> loggerp, QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::MainWindow)
  , uloggerp(loggerp)
{
  ui->setupUi(this);
  makeConnections();
  setInitialState();
  profctl.operateProfilingStats();
}

MainWindow::~MainWindow()
{
  delete ui;
}

// Static method used from outside Qt, to exit the app properly.
void MainWindow::CallCloseEvent()
{
  QWidget::close();  // This goes to the closeEvent() override method below
}

void MainWindow::makeConnections()
{
  connect(ui->StartButton, &QPushButton::clicked, this, &MainWindow::StartButtonClicked);
  connect(ui->PauseButton, &QPushButton::clicked, this, &MainWindow::PauseButtonClicked);
  connect(ui->StopButton, &QPushButton::clicked, this, &MainWindow::StopButtonClicked);
  connect(ui->FrameCountLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onFrameCountLineEditReturnPressed);
  connect(&profctl, &ProfilingController::update_stats_signal, this, &MainWindow::UpdateProfilerStats);
}

void MainWindow::setInitialState()
{
  ui->StartButton->setEnabled(true);
  ui->PauseButton->setDisabled(true);
  ui->StopButton->setEnabled(true);

  // Allow only numerics - optional single space followed by at least 1 numeric char,
  // at most 5 chars - 99999 max frame count
  QRegularExpression rx(" ?\\d{1,5}");
  QValidator *validator = new QRegularExpressionValidator(rx, this);

  // ensure that ony numbers go in the lne edit field.
  ui->FrameCountLineEdit->setValidator(validator);
}

void MainWindow::UpdateProfilerStats(long long nframes, double fps)
{
  size_t frames_requested = Video::vcGlobals::framecount;
  QString srequested = (frames_requested == 0? QString("Continuous") : QString::number(frames_requested));

  QString fstr = " Frames requested: " + srequested + "       Streamed: " + QString::number(nframes) + "       FPS: " + QString::number(fps, 'f', 3);
  ui->TopBarLabel->setText(fstr);
}

void MainWindow::StartButtonClicked()
{
  onFrameCountLineEditReturnPressed();
}

void MainWindow::PauseButtonClicked()
{
  static bool isDiagPaused = true;

  std::shared_ptr<Log::Logger> loggerp = Util::UtilLogger::getLoggerPtr();
  VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

  if (isDiagPaused)
  {
    ui->PauseButton->setText("  Pause Video");
    ui->PauseButton->setIcon(QIcon(":/icons/8665214_circle_pause_icon.svg"));

    if (loggerp != nullptr) loggerp->debug() << "MainWindow: PauseButtonClicked: Setting pause in the capture engine to FALSE.";
    if (ifptr) ifptr->set_paused(false);
    isDiagPaused = false;
  }
  else
  {
    ui->PauseButton->setText(" Resume Video");
    ui->PauseButton->setIcon(QIcon(":/icons/play-512.gif"));

    if (loggerp != nullptr) loggerp->debug() <<
                              "MainWindow: PauseButtonClicked: Setting pause in the capture engine to TRUE.";
    if (ifptr) ifptr->set_paused(true);
    isDiagPaused = true;
  }
}

void MainWindow::StopButtonClicked()
{
  QWidget::close();  // This goes to the closeEvent() override method below
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  QMessageBox::StandardButton resButton = QMessageBox::question( this, "SimpleVidStream",
                                                                 tr("Click Yes to exit...\n"),
                                                                 QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                 QMessageBox::Yes);
  if (resButton != QMessageBox::Yes) {
      event->ignore();
  } else {
    set_terminated("MainWindow: Exit/StopButtonClicked: Requesting NORMAL termination of video capture.");
    control_main_wait_for_ready();
    event->accept();
  }
}

void MainWindow::set_terminated(std::string str)
{
  std::shared_ptr<Log::Logger> loggerp = Util::UtilLogger::getLoggerPtr();
  VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

  if (loggerp != nullptr) loggerp->debug() << str;
  if (ifptr) ifptr->set_terminated(true);
}

void MainWindow::onFrameCountLineEditReturnPressed()
{
  std::shared_ptr<Log::Logger> loggerp = Util::UtilLogger::getLoggerPtr();
  VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

  QString message;
  QMessageBox msgBox;

  QString field = ui->FrameCountLineEdit->text();

  int nframes = field.toInt();

  Video::vcGlobals::set_framecount(nframes);
  loggerp->debug() << "MainWindow: Setting frame count to " << Video::vcGlobals::framecount;
  field = Video::vcGlobals::str_frame_count.c_str();
  if (nframes == 0)
  {
    ui->FrameCountLineEdit->setText(" Continuous");
  }
  else
  {
    ui->FrameCountLineEdit->setText(" " + field + " frames");

  }
  // ui->FrameCountLineEdit->setDisabled(true);
  if (nframes == 0)
  {
    message = "Continuous video streaming selected.  Please Confirm.     ";
    msgBox.setText(message);
    msgBox.exec();
  }

  // The line edit and start button have nothing else to do - remove them.
  ui->horizontalLayout->removeWidget(ui->FrameCountLineEdit);
  ui->FrameCountLineEdit->hide();
  ui->horizontalLayout->removeWidget(ui->StartButton);
  ui->StartButton->hide();
  // not deleting the start button and line edit - deliberately.

  if (loggerp != nullptr) loggerp->debug() << "MainWindow: Framecount entered/StartButtonClicked: Setting the capture engine to RESUME";
  if (ifptr) ifptr->set_paused(false);

  ui->PauseButton->setEnabled(true);
  PauseButtonClicked();
  ui->StartButton->setDisabled(true);
}


