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
#include <mainwindow.h>
#include <control_main.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <condition_data.hpp>
#include <vidcap_profiler_thread.hpp>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <algorithm>


class ProfilingWorker : public QObject
{
    Q_OBJECT

public slots:
    void runProfilingWork();

signals:
    void postStats(long long nframes, double fps);
};

class ProfilingController : public QObject
{
    Q_OBJECT
    QThread ProfilingWorkerThread;
public:
    ProfilingController() {
        ProfilingWorker *worker = new ProfilingWorker;
        worker->moveToThread(&ProfilingWorkerThread);
        connect(&ProfilingWorkerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &ProfilingController::operateProfilingStats, worker, &ProfilingWorker::runProfilingWork);
        // connect(worker, &ProfilingWorker::postStats, this, &ProfilingController::update_stats_signal);
        connect(worker, &ProfilingWorker::postStats, this, &ProfilingController::handleWorkerResults);
        ProfilingWorkerThread.start();
    }
    ~ProfilingController() {
        ProfilingWorkerThread.quit();
        ProfilingWorkerThread.wait();
    }
public slots:
    void handleWorkerResults(long long nframes, double fps)
    {
      emit update_stats_signal(nframes, fps);
    }
signals:
    void operateProfilingStats();
    void update_stats_signal(long long, double);
};

