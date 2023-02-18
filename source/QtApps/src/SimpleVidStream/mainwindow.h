#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void StartButtonClicked();
  void PauseButtonClicked();
  void StopButtonClicked();

private:
  void makeConnections();
  void closeEvent(QCloseEvent *event);

private:
  Ui::MainWindow *ui;
};
