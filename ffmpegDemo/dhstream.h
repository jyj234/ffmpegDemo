#ifndef DHSTREAM_H
#define DHSTREAM_H
#include "dhnetsdk.h"
#include <QtCore>
#include <QObject>
#include <QThread>
#include "decoder.h"
#include <QVideoSink>
class DHStream : public QObject
{
    Q_OBJECT
public:
    DHStream();
    ~DHStream();
    static DHStream* instance(){
        static DHStream* ins = new DHStream();
        return ins;
    }
    YUVData* getFrame();
    void fillVideoSink(unsigned char* y,unsigned char* u,unsigned char* v);
signals:
    void frameInfoUpdateSig(int width,int height,int format);
    void frameDataUpdateSig();
public slots:
    void setVideoSink(QVideoSink* videoSink);
    void startStream();
    void stopStream();
private:
    LLONG m_loginHandle;
    QThread m_thread;
    LONG m_nPort;
    YUVData* m_yuv;
    QVideoSink* m_videoSink;
    // void CALLBACK realDataCallBack(  LLONG  lRealHandle,DWORD  dwDataType,
    //                                          BYTE  *pBuffer,  DWORD  dwBufSize,  LONG   param,
    //                                          LDWORD dwUser);

};

#endif // DHSTREAM_H
