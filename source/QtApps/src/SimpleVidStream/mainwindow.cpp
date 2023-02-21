#include "control_main.hpp"
#include <mainwindow.h>
#include <./ui_mainwindow.h>
#include <TestUtil.hpp>
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
}

void MainWindow::StartButtonClicked()
{
  ui->NoteLabel->setText("Start button clicked...");
  std::shared_ptr<Log::Logger> loggerp = Util::UtilLogger::getLoggerPtr();
  VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

  if (loggerp != nullptr) loggerp->debug() << "MainWindow: StartButtonClicked: Setting the capture engine to RESUME";
  if (ifptr) ifptr->set_paused(false);
}

void MainWindow::PauseButtonClicked()
{
  ui->NoteLabel->setText("Pause button clicked...");
  std::shared_ptr<Log::Logger> loggerp = Util::UtilLogger::getLoggerPtr();
  VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

  if (loggerp != nullptr) loggerp->debug() << "MainWindow: PauseButtonClicked: Setting pause in the capture engine to TRUE.";
  if (ifptr) ifptr->set_paused(true);
}

void MainWindow::StopButtonClicked()
{
  QWidget::close();  // This goes to the closeEvent() override method below
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  ui->NoteLabel->setText("Stop/Exit button clicked...");

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

