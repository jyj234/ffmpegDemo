#include "frameprovider.h"
#include <QVideoFrame>
static const int nb_channels = 1;
FrameProvider::FrameProvider(QObject *parent)
    : QObject{parent}
    , ishasAudio(false)
    , ishasVideo(false)
{
    // 选择默认音频输入设备
    QAudioDevice defaultDevice = QMediaDevices::defaultAudioInput();
    //    qDebug() << "Default audio input device:" << defaultDevice.description();
    //    qDebug()<<"minimumSampleRate"<<defaultDevice.minimumSampleRate()<<"maximumSampleRate"<<defaultDevice.maximumSampleRate();
    //    for(int i = 0;i < defaultDevice.supportedSampleFormats().size();++i)
    //    {
    //        qDebug()<<defaultDevice.supportedSampleFormats()[i];
    //    }
    qDebug()<<"minimumChannelCount"<<defaultDevice.minimumChannelCount()<<"maximumChannelCount"<<defaultDevice.maximumChannelCount();
    // 设置音频格式
    QAudioFormat format;
    format.setSampleRate(44100); // 采样率
    format.setChannelCount(nb_channels);   // 声道数
    format.setSampleFormat(QAudioFormat::Float); // 格式

    // 创建音频源
    m_audioSource = new QAudioSource(defaultDevice, format);
    ishasAudio = false;
    ishasVideo = false;

}
FrameProvider::~FrameProvider()
{
    m_audioSource->stop();
}
void FrameProvider::setVideoSink(QVideoSink* videoSink,QCamera* camera)
{
    m_videoSink = videoSink;
    for(int i = 0;i<camera->cameraDevice().videoFormats().count();++i)
    {
        if(camera->cameraDevice().videoFormats()[i].resolution() == QSize(1280,720)
            && camera->cameraDevice().videoFormats()[i].pixelFormat() == QVideoFrameFormat::Format_YUYV
            && camera->cameraDevice().videoFormats()[i].minFrameRate() == 30)
        {
            camera->setCameraFormat(camera->cameraDevice().videoFormats()[i]);
            qDebug()<<"set Camera succeed";
        }
//        qDebug()<<camera->cameraDevice().videoFormats()[i].resolution()<<camera->cameraDevice().videoFormats()[i].pixelFormat()<<camera->cameraDevice().videoFormats()[i].minFrameRate();
    }
}

void FrameProvider::startRecord()
{
    m_mux = new Mux();

    connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &FrameProvider::writeVideoFrame);



    // 开始录制
    m_device = m_audioSource->start();
    if (m_device) {
        // 在这里读取从 QIODevice 中的数据
        connect(m_device, &QIODevice::readyRead,this, &FrameProvider::writeAudioFrame);
    }

}
void FrameProvider::writeAudioFrame()
{
//    m_device->readAll();
//    qDebug()<<"all size"<<m_device->bytesAvailable();
    if(m_device->bytesAvailable() >= 4096 * nb_channels)
    {
        ishasAudio = true;
        QByteArray data = m_device->read(4096 * nb_channels);
//        qDebug()<<"size"<<data.size();
        if(!ishasVideo)
            return;
        ishasAudio = true;
        qDebug()<<ishasVideo<<"audio send";
        m_mux->writeAudioFrame(data.data());
    }
}
void FrameProvider::writeVideoFrame()
{

    QVideoFrame m_videoFrame = m_videoSink->videoFrame();
    if(!m_videoFrame.isValid() || !m_videoFrame.map(QVideoFrame::ReadOnly)){
        qWarning() << "QVideoFrame is not valid or not Readable";
        return;
    }
    uchar* p1 = m_videoFrame.bits(0);
    ishasVideo = true;
    if(!ishasAudio)
    {
        goto out;
    }

    qDebug()<<ishasAudio<<"video send";
    m_mux->writeVideoFrame(p1);
out:
    m_videoFrame.unmap();
}
void FrameProvider::stopRecord()
{
    disconnect(m_videoSink, &QVideoSink::videoFrameChanged, this, &FrameProvider::writeVideoFrame);
    disconnect(m_device, &QIODevice::readyRead,this, &FrameProvider::writeAudioFrame);
    m_mux->stopRecord();
}
