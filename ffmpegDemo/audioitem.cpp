#include "audioitem.h"

AudioItem::AudioItem() {

}
void AudioItem::start()
{
    QAudioFormat format;
    // Set up the format, eg.
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    qDebug()<<"format.bytesPerFrame()"<<format.bytesPerFrame();
    QAudioDevice info(QMediaDevices::defaultAudioOutput());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return;
    }
    m_audioSink = new QAudioSink(info,format);
    m_audioSink->setBufferSize(8 * 3072);
    m_ioDevice = m_audioSink->start();
}
void AudioItem::onAudioFrameDataUpdateSig(const char *data, qint64 len)
{
    // m_audioMutex.lock();
    qDebug()<<"m_audioSink->bytesFree()"<<m_audioSink->bytesFree()<<len;
    if(m_audioSink->bytesFree() < len)
    {
        qWarning()<<"m_audioSink->bytesFree() < len";
        // QThread::msleep(len * 1000 / 44100 / 2);
    }
    if(len != 3072)
        qDebug()<<"abnormal len"<<len;
    // fread(m_auioBuffer,1,3072,file);
    // m_ioDevice->write(m_auioBuffer,3072);
    int writelen = m_ioDevice->write(data,len);
    if(writelen != len)
        qDebug()<<"real write "<<writelen<<"len"<<len;
    // m_ioDevice->waitForBytesWritten(100);
    // m_ioDevice->waitForReadyRead(100);
    // m_audioMutex.unlock();
}
void AudioItem::stop()
{
    if(m_ioDevice && m_audioSink)
    {
        m_audioSink->stop();
        m_ioDevice->close();
        delete m_audioSink;
        m_audioSink = NULL;
        m_ioDevice = NULL;
    }
}
