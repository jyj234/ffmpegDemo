#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#pragma pack(push, 8)   // 保存并设置到8字节对齐
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
#include "i420render.h"

// #include <QQuickWindow>
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
    void onAudioFrameDataUpdateSig(const char *data, qint64* len,QMutex* audioMutex);
signals:
    void isAudioOutputChanged();
    void videoInfoChangedSig();
    void fillModeChanged();
    void channelIdChanged();
    void videoErrorSig();
    void videoSourceChanged();
    void mediaStatusChanged();
public slots:
    void start();
    void stop();
    void onVideoInfoReady(int width, int height, int format);
    void onLoadSuccessSig();

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
    bool isAudioOutput() const
    {
        return m_isAudioOutput;
    }
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

    void handleStateChanged(QAudio::State newState);
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
};
#pragma pack(pop)       // 恢复以前的打包对齐
#endif // VIDEOITEM_H
