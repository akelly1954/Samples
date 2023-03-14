#pragma once

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

#include <QWidget>
#include <QLineEdit>
#include <QRegularExpressionValidator>
#include <vidstream_profiler_thread.hpp>
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
  void UpdateProfilerStats(long long numPausedframes, long long numUnpausedframes, double fps);
  void NoOpProfilerStats(long long numPausedframes, long long numUnpausedframes, double fps);

private:
  void makeConnections();
  void setInitialState();
  void initializeCapture();
  void closeEvent(QCloseEvent *event);
  void set_terminated(std::string str = "Termination requested (Qt)");

private:
  Ui::MainWindow *ui;
  std::shared_ptr<Log::Logger> uloggerp = nullptr;
  ProfilingController profctl;
};