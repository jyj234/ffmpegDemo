#include <QOpenGLFramebufferObject>
#include <QQuickWindow>
#include "videoitem.h"
//************TaoItemRender************//
class ItemRender  : public QQuickFramebufferObject::Renderer
{
public:
    ItemRender();

    void render() override;
    QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override;
    void synchronize(QQuickFramebufferObject*) override;

private:
    I420Render m_render;
    QQuickWindow* m_window = nullptr;
};

ItemRender::ItemRender()
{
    m_render.init();
}

void ItemRender::render()
{
    m_render.render();
    QQuickOpenGLUtils::resetOpenGLState();
}

QOpenGLFramebufferObject* ItemRender::createFramebufferObject(const QSize& size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    m_render.resize(size.width(), size.height());
    return new QOpenGLFramebufferObject(size, format);
}

void ItemRender::synchronize(QQuickFramebufferObject* item)
{
    // qDebug()<<"ItemRender::synchronize";
    VideoItem* pItem = qobject_cast<VideoItem*>(item);
    if (pItem)
    {
        if (!m_window)
        {
            m_window = pItem->window();
        }
        if (pItem->infoDirty())
        {
            m_render.updateTextureInfo(pItem->videoWidth(), pItem->videoHeght(), pItem->videoFormat());
            pItem->makeInfoDirty(false);
        }
        if(pItem->m_decoder && pItem->m_decoder->getFrame())
            m_render.updateTextureData(pItem->m_decoder->getFrame());
    }
}

//************TaoItem************//
VideoItem::VideoItem(QQuickItem* parent)
    : QQuickFramebufferObject(parent)
{
    m_decoder = new Decoder();
    QAudioFormat format;
    // Set up the format, eg.
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    qDebug()<<"here1";
    QAudioDevice info(QMediaDevices::defaultAudioOutput());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return;
    }
    m_audioSink = new QAudioSink(info,format, this);
    m_ioDevice = m_audioSink->start();
     qDebug()<<"here2";
}
void VideoItem::setSource(QString url)
{
    connect(m_decoder,&Decoder::frameInfoUpdateSig,this,&VideoItem::onVideoInfoReady);
    connect(m_decoder,&Decoder::frameDataUpdateSig,this,&VideoItem::update);
    connect(m_decoder,&Decoder::audioFrameDataUpdateSig,this,&VideoItem::onReadyForMoreDataSig);
    m_decoder->moveThread(url);
    // m_audioSink->start(m_audioDevice);
    //按每秒60帧的帧率更新界面
    // m_timerId = startTimer(1000 / 25);
}
void VideoItem::onReadyForMoreDataSig(const char *data, qint64 len)
{
    qDebug()<<"before m_ioDevice->write";
    m_ioDevice->write(data,len);
    qDebug()<<"after m_ioDevice->write";
    // if (!m_audioDevice->isOpen()) {
    //     qDebug()<<"onReadyForMoreDataSig";
    //     m_audioDevice->open(QIODevice::ReadWrite);
    //     m_audioSink->start(m_audioDevice);
    // }
}
void VideoItem::timerEvent(QTimerEvent* event)
{
    // qDebug()<<"timerEvent";
    Q_UNUSED(event);
    update();
}
void VideoItem::stopDecoder()
{
    qDebug()<<"VideoItem::stopDecoder";
    disconnect(m_decoder,&Decoder::frameInfoUpdateSig,this,&VideoItem::onVideoInfoReady);
    disconnect(m_decoder,&Decoder::frameDataUpdateSig,this,&VideoItem::update);
    disconnect(m_decoder,&Decoder::audioFrameDataUpdateSig,this,&VideoItem::onReadyForMoreDataSig);
    m_ioDevice->waitForBytesWritten(1000);
    qDebug()<<"before m_audioSink->stop()"<<m_ioDevice->bytesToWrite();
    m_audioSink->stop();
    m_ioDevice->close();
    qDebug()<<"after m_audioSink->stop()";
    if(m_decoder)
        delete m_decoder;
    qDebug()<<"before m_decoder = NULL";
    m_decoder = NULL;
    qDebug()<<"stopDecoder over";
}
void VideoItem::onVideoInfoReady(int width, int height, int format)
{
    qDebug()<<"VideoItem::onVideoInfoReady";
    if (m_videoWidth != width)
    {
        m_videoWidth = width;
        makeInfoDirty(true);
    }
    if (m_videoHeight != height)
    {
        m_videoHeight = height;
        makeInfoDirty(true);
    }
    if (m_videoFormat != format)
    {
        m_videoFormat = format;
        makeInfoDirty(true);
    }
}

QQuickFramebufferObject::Renderer* VideoItem::createRenderer() const
{
    qDebug()<<"VideoItem::createRenderer()";
    return new ItemRender;
}
