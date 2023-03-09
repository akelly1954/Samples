
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

#include <mainwindow.h>
#include <QApplication>
#include <QVBoxLayout>
// #include <video_stream_buffers.hpp>

#include <QDebug>

QMediaPlayer VidstreamController::mp_player;
QVideoWidget VidstreamController::mp_vw;

// The constructor runs in the MainWindow thread
VidstreamWorker::VidstreamWorker(Ui::MainWindow *ui) : uip(ui)
{
    if (uip == nullptr)
    {
        qDebug("ERROR: Ui class ptr is null!");
        return;
    }
}

void VidstreamWorker::runVidstreamWork()
{
    if (uip == nullptr)
    {
        qDebug() << "ERROR: Ui class ptr is null!";
        return;
    }
}

void VidstreamController::setupVideoPlayer(Ui::MainWindow *sui)
{
    QVBoxLayout *vlayout = sui->verticalLayout;

    // vlayout->addWidget(player);     // , 1, Qt::Alignment);
    vlayout->addWidget(&mp_vw);     // , 1, Qt::Alignment);

    mp_player.setVideoOutput(&mp_vw);
    mp_player.setSource(QUrl::fromLocalFile("/home/andrew/mpgs/747-400 - 2020-04-03 07.50.28.avi"));
    // vw->setGeometry(200, 200, 500, 400);

    mp_vw.show();

    qDebug() << mp_player.mediaStatus();

    return;
}
