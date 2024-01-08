#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <QTimer>
#include <QImage>
#include <QDebug>
#include <QVideoSink>
#include <QVideoFrame>
#include <QCamera>
#include <QMediaDevices>
#include "mux.h"
#include <QAudioFormat>
#include <QAudioDevice>
#include <QAudioSource>
#include <QAudioBuffer>
class FrameProvider : public QObject
{
    Q_OBJECT
public:
    explicit FrameProvider(QObject *parent = nullptr);
    ~FrameProvider();
    static FrameProvider* instance()
    {
        static FrameProvider* ins = new FrameProvider();
        return ins;
    }
    void setVideoOutput(uint8_t *data,int len);
public slots:
    void startRecord();
    void stopRecord();
    void setVideoSink(QVideoSink* videoSink,QCamera* camera);
private:
    cv::VideoCapture cap;
private:
    QVideoSink* m_videoSink;
    void writeVideoFrame();
    void writeAudioFrame();
    Mux* m_mux;
    QIODevice *m_device;
    QAudioSource *m_audioSource;
    bool ishasVideo;
    bool ishasAudio;
};

#endif // FRAMEPROVIDER_H
