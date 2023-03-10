
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
    // , streamctl(ui)
{
    ui->setupUi(this);
    player = new VideoPlayer(this, ui);
    makeConnections();
    setInitialState();
    profctl.operateProfilingStats();

    // VidstreamController::setupVideoPlayer(ui);
    // streamctl.operateVidstreamThread();
}

MainWindow::~MainWindow()
{
    // delete ui;
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

    // TODO: fix last arg below
    // connect(&streamctl, &VidstreamController::update_Stream_signal, this, &MainWindow::NoOpProfilerStats);

    connect(&profctl, &ProfilingController::update_stats_signal, this, &MainWindow::NoOpProfilerStats);
    // Soon after start, the ProfilingController::update_stats_signal will be disconnected:
    // From Qt6 doc:  Disconnect everything connected to a specific signal:
    // disconnect(myObject, &MyObject::mySignal(), nullptr, nullptr);
    // see below...
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

void MainWindow::NoOpProfilerStats(long long numPausedframes, long long numUnpausedframes, double fps)
{
    static int ct = 0;
    std::shared_ptr<Log::Logger> loggerp = Util::UtilLogger::getLoggerPtr();

    if ((ct++ % 10) == 0)
    {
        if (loggerp != nullptr) loggerp->debug() <<
                                                    "MainWindow::NoOpProfilerStats: Got paused frame count = " << numPausedframes <<
                                                    ", Unpaused: " << numUnpausedframes << ", FPS = "<< fps;
    }
}

void MainWindow::UpdateProfilerStats(long long numPausedframes, long long numUnpausedframes, double fps)
{
    size_t frames_requested = Video::vcGlobals::framecount;
    QString srequested = (frames_requested == 0? QString("Continuous") : QString::number(frames_requested));

    QString fstr = " Frames requested: " + srequested +
            "      Paused frames: " + QString::number(numPausedframes) +
            "      Unpaused frames: " + QString::number(numUnpausedframes) +
            "      FPS: " + QString::number(fps, 'f', 3);
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
    QMessageBox::StandardButton resButton = QMessageBox::question( this, "VideoCapturePlayer",
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

    VideoCapture::video_plugin_base::base_start_streaming(Video::vcGlobals::framecount);

    // This disconnect relies on having only one connection from this signal.
    disconnect(&profctl, &ProfilingController::update_stats_signal, nullptr, nullptr);

    // Create a new connection
    connect(&profctl, &ProfilingController::update_stats_signal, this, &MainWindow::UpdateProfilerStats);
}


