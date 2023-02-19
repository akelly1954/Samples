#pragma once

#include <QWidget>
#include <memory>
#include <MainLogger.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  MainWindow(std::shared_ptr<Log::Logger> loggerp = nullptr, QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void StartButtonClicked();
  void PauseButtonClicked();
  void StopButtonClicked();

private:
  void makeConnections();
  void initializeCapture();
  void closeEvent(QCloseEvent *event);

private:
  Ui::MainWindow *ui;
  std::shared_ptr<Log::Logger> uloggerp = nullptr;

};
