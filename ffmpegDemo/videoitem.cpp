#include "videoitem.h"

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

VideoItem::VideoItem(QQuickItem* parent)
    : QQuickFramebufferObject(parent)
    , m_ioDevice(NULL)
    , m_audioSink(NULL)
    , m_isAudioOutput(false)
    , m_fillMode(PreserveAspectFit)
    , m_channelId(0)
    , m_source("")
    , m_mediaStatus(LoadingMedia)
{
    this->setVisible(false);
    // start();
}
VideoItem::~VideoItem()
{
    // stop();
}
void VideoItem::onFrameDataUpdateSig()
{
    update();
}
void VideoItem::setIsAudioOutput(const bool &isAudioOutput)
{
    m_isAudioOutput = isAudioOutput;
    emit isAudioOutputChanged();
}
void VideoItem::resize(int flag)
{
    m_decoder->resize(flag);
}
void VideoItem::drag(float deltx,float delty)
{
    m_decoder->drag(deltx,delty);
}
void VideoItem::start()
{
    qDebug()<<"VideoItem::start()";
    emit mediaStatusChanged();
    if(!m_decoder)
    {
        m_decoder = new Decoder(this);
    }
    else
        return;
    if(m_isAudioOutput)
    {
        QAudioFormat format;
        // Set up the format, eg.
        format.setSampleRate(48000);
        format.setChannelCount(1);
        format.setSampleFormat(QAudioFormat::Int16);
        QAudioDevice info(QMediaDevices::defaultAudioOutput());
        if (!info.isFormatSupported(format)) {
            qWarning() << "Raw audio format not supported by backend, cannot play audio.";
            return;
        }
        m_audioSink = new QAudioSink(info,format, this);
        // m_audioSink->setBufferSize(10 * 3072);
        m_ioDevice = m_audioSink->start();
        connect(m_decoder,&Decoder::audioDataUpdateSig,this,&VideoItem::onAudioFrameDataUpdateSig);
    }
    connect(m_decoder,&Decoder::frameInfoUpdateSig,this,&VideoItem::onVideoInfoReady);
    connect(m_decoder,&Decoder::frameDataUpdateSig,this,&VideoItem::onFrameDataUpdateSig);
    connect(m_decoder,&Decoder::frameInfoChangedSig,this,&VideoItem::videoInfoChangedSig);
    connect(m_decoder,&Decoder::videoErrorSig,this,&VideoItem::videoErrorSig);
    connect(m_decoder,&Decoder::loadSuccessSig,this,&VideoItem::onLoadSuccessSig);
    m_thread = new QThread();
    connect(m_thread, &QThread::started, m_decoder, &Decoder::startDecode);
    connect(m_thread, &QThread::finished, m_decoder, &Decoder::deleteLater);
    m_decoder->setUrl(m_source);
    m_decoder->moveToThread(m_thread);
    m_thread->start();

    // m_url = "rtsp://admin:123456@10.12.13.237:554/H264?ch=1&subtype=0";
    // m_url = "rtsp://admin:123456@10.12.13.237:554/H264?ch=1&subtype=0";
    // m_url = NULL;
    // m_decoder->moveThread(m_url);
    // m_audioSink->start(m_audioDevice);
    //按每秒60帧的帧率更新界面
    // m_timerId = startTimer(1000 / 25);
}
void VideoItem::onLoadSuccessSig()
{
    qDebug()<<"VideoItem::onLoadSuccessSig()";
    m_mediaStatus = BufferedMedia;
    emit mediaStatusChanged();
}
void VideoItem::onAudioFrameDataUpdateSig(const char *data, qint64* len,QMutex* audioMutex)
{
    audioMutex->lock();
    // qDebug()<<"m_audioSink->bytesFree()"<<m_audioSink->bytesFree()<<*len;
    if(m_audioSink->bytesFree() < *len)
    {
        // qWarning()<<"m_audioSink->bytesFree() < len";
    }
    m_ioDevice->write(data,*len);
    *len = 0;
out:
    audioMutex->unlock();
    // if (!m_audioDevice->isOpen()) {
    //     qDebug()<<"onReadyForMoreDataSig";
    //     m_audioDevice->open(QIODevice::ReadWrite);
    //     m_audioSink->start(m_audioDevice);
    // }
}
void VideoItem::stop()
{
    qDebug()<<"start VideoItem::stop()";
    if(m_decoder)
    {
        qDebug()<<"after closeDHStream()";
        disconnect(m_decoder,&Decoder::frameInfoUpdateSig,this,&VideoItem::onVideoInfoReady);
        disconnect(m_decoder,&Decoder::frameDataUpdateSig,this,&VideoItem::update);
        disconnect(m_decoder,&Decoder::frameInfoChangedSig,this,&VideoItem::videoInfoChangedSig);
        disconnect(m_decoder,&Decoder::videoErrorSig,this,&VideoItem::videoErrorSig);
        if(m_ioDevice && m_audioSink)
        {
            disconnect(m_decoder,&Decoder::audioDataUpdateSig,this,&VideoItem::onAudioFrameDataUpdateSig);
            m_audioSink->stop();
            m_ioDevice->close();
            delete m_audioSink;
            m_audioSink = NULL;
            m_ioDevice = NULL;
        }
        qDebug()<<"before setIsOver()";
        m_decoder->setIsOver();
        m_thread->quit();
        qDebug()<<"before wait()";
        m_thread->wait();
        qDebug()<<"after wait()";
        m_decoder = NULL;
        if(m_thread)
        {
            delete m_thread;
            m_thread = NULL;
        }
    }
    qDebug()<<"after VideoItem::stop()";

}
void VideoItem::setFillMode(FillModeType fillMode)
{
    m_fillMode = fillMode;
}
void VideoItem::setChannelId(int channelId)
{
    m_channelId = channelId;
}
void VideoItem::setVideoSource(QString source)
{
    m_source = source;
}
void VideoItem::setMediaStatus(ZNLoadingStatus mediaStatus)
{
    qDebug()<<"mediaStatus is read-only";
    return;
}
void VideoItem::myrotate()
{
    static int cnt = 0;
    cnt++;
    m_decoder->rotate(cnt*M_PI/6);
}
void VideoItem::onVideoInfoReady(int width, int height, int format)
{
    if(m_fillMode == PreserveAspectFit)
    {
        if(this->width() * (1.0 * height / width) < this->height())
        {
            qDebug()<<"height"<<height<<"width"<<width;
            qDebug()<<"this->height"<<this->height()<<"this->width"<<this->width();
            this->setHeight(this->width() * (1.0 * height / width));
        }
        else
        {
            this->setWidth(this->height() * (1.0 * width / height));
        }
    }
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
    return new ItemRender;
}

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
        {
            if(!pItem->isVisible())
                pItem->setVisible(true);
            m_render.updateTextureData(pItem->m_decoder->getFrame());
        }
    }
}
