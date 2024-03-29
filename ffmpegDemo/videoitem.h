#ifndef VIDEOITEM_H
#define VIDEOITEM_H
#include "decoder.h"
#include <QQuickFramebufferObject>
#include <QQuickItem>
#include <QQuickOpenGLUtils>
#include <QTimer>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QAudioSink>
#include <QMediaDevices>
#include <QOpenGLFramebufferObject>
#include <QTimer>
#include "i420render.h"
class VideoItem : public QQuickFramebufferObject
{
    Q_OBJECT
public:
    enum FillModeType{
        Stretch,
        PreserveAspectFit
    };
    enum ZNLoadingStatus{
        LoadingMedia,
        BufferedMedia
    };

    Q_ENUM(FillModeType)
    Q_ENUM(ZNLoadingStatus)
    VideoItem(QQuickItem* parent = nullptr);
    ~VideoItem();
    Q_PROPERTY(bool isAudioOutput READ isAudioOutput WRITE setIsAudioOutput NOTIFY isAudioOutputChanged);
    Q_PROPERTY(FillModeType fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged);
    Q_PROPERTY(int channelId READ channelId WRITE setChannelId NOTIFY channelIdChanged);
    Q_PROPERTY(QString videoSource READ videoSource WRITE setVideoSource NOTIFY videoSourceChanged);
    Q_PROPERTY(ZNLoadingStatus mediaStatus READ mediaStatus WRITE setMediaStatus NOTIFY mediaStatusChanged);
    Q_PROPERTY(int timeoutDuration READ timeoutDuration WRITE setTimeoutDuration NOTIFY timeoutDurationChanged);

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
    bool isAudioOutput() const
    {
        return m_isAudioOutput;
    }
    int timeoutDuration() const
    {
        return m_timeoutDuration;
    }
    void onAudioFrameDataUpdateSig(const char *data, qint64* len,QMutex* audioMutex);
signals:
    void isAudioOutputChanged();
    void videoInfoChangedSig();
    void fillModeChanged();
    void channelIdChanged();
    void videoErrorSig();
    void videoSourceChanged();
    void mediaStatusChanged();
    void timeoutDurationChanged();
public slots:
    void start();
    void stop();
    void onVideoInfoReady(int width, int height, int format);
    void onLoadSuccessSig();
    void myrotate();
    void resize(int flag);
    void drag(float deltx,float delty);

public:
    Renderer* createRenderer() const override;
    int m_videoWidth;
    int m_videoHeight;
    int m_videoFormat;
    bool m_infoChanged = false;
    YUVData* m_yuv;
    Decoder* m_decoder = NULL;
    // int m_timerId;
    // QTimer m_timer;
private:
    FillModeType fillMode() const
    {
        return m_fillMode;
    }
    int channelId() const
    {
        return m_channelId;
    }
    ZNLoadingStatus mediaStatus() const
    {
        return m_mediaStatus;
    }
    QString videoSource() const
    {
        return m_source;
    }
private:
    void setIsAudioOutput(const bool &isAudioOutput);
    void setFillMode(FillModeType fillMode);
    void setChannelId(int channelId);
    void setVideoSource(QString source);
    void setMediaStatus(ZNLoadingStatus mediaStatus);
    void setTimeoutDuration(int timeoutDuration)
    {
        m_timeoutDuration = timeoutDuration;
        qDebug()<<"setTimeoutDuration"<<timeoutDuration;
        emit timeoutDurationChanged();
    }

    void handleStateChanged(QAudio::State newState);
    void onFrameDataUpdateSig();
    QAudioSink* m_audioSink;

    QIODevice* m_ioDevice;
    bool m_isAudioOutput;
    QMetaObject::Connection m_videoDataUpdateCon;
    QMetaObject::Connection m_videoInfoUpdateCon;
    QMetaObject::Connection m_audioDataUpdateCon;

    FillModeType m_fillMode;
    int m_channelId;
    QString m_source;
    QThread* m_thread;
    ZNLoadingStatus m_mediaStatus;
    QMutex m_audioMutex;
    int m_timeoutDuration;
};
#endif // VIDEOITEM_H
