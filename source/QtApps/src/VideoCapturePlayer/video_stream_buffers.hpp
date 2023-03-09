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

#include <QObject>
#include <QThread>
#include <control_main.hpp>
#include <Utility.hpp>
#include <ui_mainwindow.h>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include <MainLogger.hpp>
#include <condition_data.hpp>
#include <vidcap_profiler_thread.hpp>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <algorithm>

class VidstreamWorker : public QObject
{
    Q_OBJECT

public:
    VidstreamWorker(Ui::MainWindow *ui);

public slots:
    void runVidstreamWork();

signals:
    void postStream(long long numPausedframes, long long numUnpausedframes, double fps);

private:
    Ui::MainWindow *uip;
};

class VidstreamController : public QObject
{
    Q_OBJECT
    QThread VidstreamWorkerThread;
public:
    VidstreamController(Ui::MainWindow *ui) : uip(ui)
    {
        VidstreamWorker *worker = new VidstreamWorker(uip);
        // setupVideoPlayer();
        worker->moveToThread(&VidstreamWorkerThread);
        connect(&VidstreamWorkerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &VidstreamController::operateVidstreamThread, worker, &VidstreamWorker::runVidstreamWork);
        connect(worker, &VidstreamWorker::postStream, this, &VidstreamController::handleWorkerResults);
        VidstreamWorkerThread.start();
    }
    ~VidstreamController() {
        VidstreamWorkerThread.quit();
        VidstreamWorkerThread.wait();
    }
    static void setupVideoPlayer(Ui::MainWindow *ui);
public slots:
    void handleWorkerResults(long long numPausedframes, long long numUnpausedframes, double fps)
    {
      emit update_Stream_signal(numPausedframes, numUnpausedframes, fps);
    }
signals:
    void operateVidstreamThread();
    void update_Stream_signal(long long, long long, double);
public:
    Ui::MainWindow *uip;
    static QMediaPlayer mp_player;
    static QVideoWidget mp_vw;
};

