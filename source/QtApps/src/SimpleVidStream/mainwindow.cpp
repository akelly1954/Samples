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
}

void MainWindow::setInitialState()
{
  ui->StartButton->setEnabled(true);
  ui->PauseButton->setDisabled(true);
  ui->StopButton->setEnabled(true);

  // From Qt6 doc:     regexp: optional '-' followed by between 1 and 3 digits
  // From Qt6 doc:     QRegularExpression rx("-?\\d{1,3}");

  // Allow only numerics - at least 1 char, at most 5 chars - 99999 max frame count
  QRegularExpression rx("\\d{1,5}");
  QValidator *validator = new QRegularExpressionValidator(rx, this);

  // ensure that ony numbers go in the lne edit field.
  ui->FrameCountLineEdit->setValidator(validator);
}

void MainWindow::StartButtonClicked()
{
  std::shared_ptr<Log::Logger> loggerp = Util::UtilLogger::getLoggerPtr();
  VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

  if (loggerp != nullptr) loggerp->debug() << "MainWindow: StartButtonClicked: Setting the capture engine to RESUME";
  if (ifptr) ifptr->set_paused(false);
  ui->PauseButton->setEnabled(true);
  PauseButtonClicked();
  ui->StartButton->setDisabled(true);
}

void MainWindow::PauseButtonClicked()
{
  static bool ispaused = true;

  std::shared_ptr<Log::Logger> loggerp = Util::UtilLogger::getLoggerPtr();
  VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

  if (ispaused)
  {
    ui->PauseButton->setText("  Pause Video");
    ui->PauseButton->setIcon(QIcon(":/icons/8665214_circle_pause_icon.svg"));

    if (loggerp != nullptr) loggerp->debug() << "MainWindow: PauseButtonClicked: Setting pause in the capture engine to FALSE.";
    if (ifptr) ifptr->set_paused(false);
    ispaused = false;
  }
  else
  {
    ui->PauseButton->setText(" Resume Video");
    ui->PauseButton->setIcon(QIcon(":/icons/play-512.gif"));

    if (loggerp != nullptr) loggerp->debug() <<
                              "MainWindow: PauseButtonClicked: Setting pause in the capture engine to TRUE.";
    if (ifptr) ifptr->set_paused(true);
    ispaused = true;
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

  QString message;
  QMessageBox msgBox;

  QString field = ui->FrameCountLineEdit->text();
  int nframes = field.toInt();

  if (nframes == 0)
  {
    message = "Continuous video streaming selected.  Please Confirm.     ";
    msgBox.setText(message);
    msgBox.exec();
  }

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
  ui->FrameCountLineEdit->setDisabled(true);

  StartButtonClicked();
}


