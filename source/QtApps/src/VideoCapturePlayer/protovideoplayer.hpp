#pragma once
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// Note: This file has been modified from the original Qt source.
// Original comes from Qt6.2 examples - VideoWidget

#include <mainwindow.h>
#include <QMediaPlayer>
#include <QWidget>
#include <stream2qt_video_capture.hpp>

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QSlider;
class QLabel;
class QUrl;
QT_END_NAMESPACE

class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    VideoPlayer(QWidget *parent = nullptr, Ui::MainWindow *ui = nullptr);
    ~VideoPlayer();

    void setUrl(const QUrl &url);

public slots:
    void openFile();
    void play();
    void receiveFrameBuffer(Util::shared_ptr_uint8_data_t fbuf);

private slots:
    void mediaStateChanged(QMediaPlayer::PlaybackState state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void setPosition(int position);
    void handleError();

private:
    QMediaPlayer* m_mediaPlayer;
    QAbstractButton *m_playButton;
    QSlider *m_positionSlider;
    QLabel *m_errorLabel;
};

