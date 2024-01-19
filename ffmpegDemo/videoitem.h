#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QQuickFramebufferObject>
#include <QQuickItem>
#include <memory>
#include <QQuickOpenGLUtils>
#include "decoder.h"
#include "i420render.h"
#include "decoder.h"
#include "nv12render.h"
#include <QTimer>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QAudioSink>
#include <QMediaDevices>
class VideoItem : public QQuickFramebufferObject
{
    Q_OBJECT
public:
    VideoItem(QQuickItem* parent = nullptr);
    void timerEvent(QTimerEvent* event) override;

    // YUVData getFrame(bool& got);
    bool infoDirty() const
    {
        return m_infoChanged;
    }
    void makeInfoDirty(bool dirty)
    {
        m_infoChanged = dirty;
    }
    int videoWidth() const
    {
        return m_videoWidth;
    }
    int videoHeght() const
    {
        return m_videoHeight;
    }
    int videoFormat() const
    {
        return m_videoFormat;
    }
public slots:

    void setSource(QString url);
    void stopDecoder();
protected slots:
    void onVideoInfoReady(int width, int height, int format);


public:
    Renderer* createRenderer() const override;
    int m_videoWidth;
    int m_videoHeight;
    int m_videoFormat;
    bool m_infoChanged = false;
    YUVData* m_yuv;
    Decoder* m_decoder;
    // int m_timerId;
    // QTimer m_timer;
private:
    void handleStateChanged(QAudio::State newState);
    QAudioSink* m_audioSink;
    void OnAudioFrameDataUpdateSig();
    void onReadyForMoreDataSig(const char *data, qint64 len);
    QIODevice* m_ioDevice;
};
#endif // VIDEOITEM_H
