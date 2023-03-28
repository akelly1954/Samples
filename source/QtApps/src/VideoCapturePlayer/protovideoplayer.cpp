// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// Note: This file has been modified from the original Qt source.
// Original comes from Qt6.2 examples - VideoWidget

#include <protovideoplayer.hpp>
#include <shared_data_items.hpp>
#include <stream2qt_video_capture.hpp>
#include <QByteArray>
#include <QtWidgets>
#include <QVideoWidget>

VideoPlayer::VideoPlayer(QWidget *parent, Ui::MainWindow *ui )
    : QWidget(parent)
{
    m_mediaPlayer = new QMediaPlayer(this);
    QVideoWidget *videoWidget = new QVideoWidget;

    // QAbstractButton *openButton = new QPushButton(tr("Open..."));
    // connect(openButton, &QAbstractButton::clicked, this, &VideoPlayer::openFile);

    // m_playButton = new QPushButton;
    // m_playButton->setEnabled(false);
    // m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    // connect(m_playButton, &QAbstractButton::clicked,
    //         this, &VideoPlayer::play);

    // m_positionSlider = new QSlider(Qt::Horizontal);
    // m_positionSlider->setRange(0, 0);

    // connect(m_positionSlider, &QAbstractSlider::sliderMoved,
    //         this, &VideoPlayer::setPosition);

    // m_errorLabel = new QLabel;
    // m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    // QHBoxLayout *controlLayout = new QHBoxLayout;
    // controlLayout->setContentsMargins(0, 0, 0, 0);
    // controlLayout->addWidget(openButton);
    // controlLayout->addWidget(m_playButton);
    // controlLayout->addWidget(m_positionSlider);

    QVBoxLayout *layout = ui->verticalLayout;

    layout->addWidget(videoWidget);
    // layout->addLayout(controlLayout);
    // layout->addWidget(m_errorLabel);

    this->setLayout(layout);

    const QSize availableGeometry = this->window()->size();
    // this->resize(availableGeometry.width(), availableGeometry.height());
    this->resize(availableGeometry);

    m_mediaPlayer->setVideoOutput(videoWidget);
//    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged,
//            this, &VideoPlayer::mediaStateChanged);
//    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);
//    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
//     connect(m_mediaPlayer, &QMediaPlayer::errorChanged,
//             this, &VideoPlayer::handleError);
    this->show();
}

VideoPlayer::~VideoPlayer()
{
}

void VideoPlayer::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Movie"));
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
        setUrl(fileDialog.selectedUrls().constFirst());
}

void VideoPlayer::setUrl(const QUrl &url)
{
    // m_errorLabel->setText(QString());
    setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
    m_mediaPlayer->setSource(url);
    m_playButton->setEnabled(true);
}

void VideoPlayer::play()
{
    switch (m_mediaPlayer->playbackState()) {
    case QMediaPlayer::PlayingState:
        m_mediaPlayer->pause();
        break;
    default:
        m_mediaPlayer->play();
        break;
    }
}

void VideoPlayer::receiveFrameBuffer(Util::shared_ptr_uint8_data_t sp_frame)
{
    ; // TODO: Everything
    auto loggerp = Util::UtilLogger::getLoggerPtr();

            const char *data = reinterpret_cast<const char *>(sp_frame->_begin());
            QByteArray ByteArrayData(QByteArray::fromRawData(data, sp_frame->num_items()));
}

#if 0
void VideoPlayer::mediaStateChanged(QMediaPlayer::PlaybackState state)
{
    switch(state) {
    case QMediaPlayer::PlayingState:
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}
void VideoPlayer::positionChanged(qint64 position)
{
    m_positionSlider->setValue(position);
}

void VideoPlayer::durationChanged(qint64 duration)
{
    m_positionSlider->setRange(0, duration);
}

void VideoPlayer::setPosition(int position)
{
    m_mediaPlayer->setPosition(position);
}

void VideoPlayer::handleError()
{
    if (m_mediaPlayer->error() == QMediaPlayer::NoError)
        return;

    m_playButton->setEnabled(false);
    const QString errorString = m_mediaPlayer->errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(m_mediaPlayer->error()));
    else
        message += errorString;
    m_errorLabel->setText(message);
}
#endif // 0
