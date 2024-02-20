#ifndef AUDIOITEM_H
#define AUDIOITEM_H
#include <QAudioFormat>
#include <QAudioDevice>
#include <QAudioSink>
#include <QObject>
#include <QtCore>
#include <QMediaDevices>
class AudioItem
{
public:
    AudioItem();
    void start();
    void stop();
public slots:
    void onAudioFrameDataUpdateSig(const char *data, qint64 len);
private:
    QAudioSink* m_audioSink;
    QIODevice* m_ioDevice;
};

#endif // AUDIOITEM_H
