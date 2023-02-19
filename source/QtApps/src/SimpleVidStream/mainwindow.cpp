#include <mainwindow.h>
#include <./ui_mainwindow.h>
#include <TestUtil.hpp>
#include <QPushButton>
#include <QCloseEvent>
#include <QMessageBox>
#include <QThread>
#include <MainLogger.hpp>

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

void MainWindow::makeConnections()
{
  connect(ui->StartButton, &QPushButton::clicked, this, &MainWindow::StartButtonClicked);
  connect(ui->PauseButton, &QPushButton::clicked, this, &MainWindow::PauseButtonClicked);
  connect(ui->StopButton, &QPushButton::clicked, this, &MainWindow::StopButtonClicked);
}

void MainWindow::StartButtonClicked()
{
  ui->NoteLabel->setText("Start button clicked...");
}

void MainWindow::PauseButtonClicked()
{
  ui->NoteLabel->setText("Pause button clicked...");
}

void MainWindow::StopButtonClicked()
{
  QWidget::close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  std::map<int,std::string> stringmap;
  stringmap[1] = "1 No time for This ";
  stringmap[2] = "2 Is time for This ";

  // TODO: This is for the initial debugging stage. Get rid of this.
  playwithstrings(stringmap, uloggerp);

  ui->NoteLabel->setText("Stop/Exit button clicked...");

  QMessageBox::StandardButton resButton = QMessageBox::question( this, "SimpleVidStream",
                                                                 tr("Are you sure?\n"),
                                                                 QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                 QMessageBox::Yes);
  if (resButton != QMessageBox::Yes) {
      event->ignore();
  } else {
      event->accept();
  }
}
