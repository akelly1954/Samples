#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QRegularExpressionValidator>
#include <MainLogger.hpp>
#include <memory>

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
  void onFrameCountLineEditReturnPressed();

public slots:
  void CallCloseEvent();

private:
  void makeConnections();
  void setInitialState();
  void initializeCapture();
  void closeEvent(QCloseEvent *event);
  void set_terminated(std::string str = "Termination requested (Qt)");

private:
  Ui::MainWindow *ui;
  std::shared_ptr<Log::Logger> uloggerp = nullptr;
};
