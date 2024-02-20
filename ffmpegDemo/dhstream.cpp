#include "dhstream.h"
static FILE* file = fopen("D:/ffmpeg-6.0-essentials_build/bin/dhyuv","wb");
static unsigned char* streamData = new unsigned char[100* 1024];
DHStream::DHStream()
    : m_yuv(NULL)
{
}
DHStream::~DHStream()
{
    stopStream();
}
static void CALLBACK deviceDisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{

}
static void CALLBACK decodeRecv(LONG nPort, FRAME_DECODE_INFO* pFrameDecodeInfo, FRAME_INFO_EX* pFrameInfo, void* pUser)
{
    // if(!pFrameDecodeInfo->nHeight[0])
    //     return;
    fwrite((unsigned char*)pFrameDecodeInfo->pVideoData[0],1,pFrameDecodeInfo->nHeight[0] * pFrameDecodeInfo->nWidth[0],file);
    fwrite((unsigned char*)pFrameDecodeInfo->pVideoData[1],1,pFrameDecodeInfo->nHeight[1] * pFrameDecodeInfo->nWidth[1],file);
    fwrite((unsigned char*)pFrameDecodeInfo->pVideoData[2],1,pFrameDecodeInfo->nHeight[2] * pFrameDecodeInfo->nWidth[2],file);

    // qDebug()<<"decodeRecv"<<pFrameDecodeInfo->nType<<pFrameDecodeInfo->nDataTime.wMinute<<pFrameDecodeInfo->nDataTime.wSecond;
    // DHStream* stream = ((DHStream*)pUser);
    // YUVData* tmpYUV = stream->getFrame();
    // if(!std::equal(std::begin(tmpYUV->height), std::end(tmpYUV->height), std::begin(pFrameDecodeInfo->nHeight), std::end(pFrameDecodeInfo->nHeight))|| tmpYUV->yLineSize != pFrameDecodeInfo->nWidth[0]
    //     || tmpYUV->uLineSize != pFrameDecodeInfo->nWidth[1] ||tmpYUV->vLineSize != pFrameDecodeInfo->nWidth[2])
    // {
    //     // qDebug()<<"tmpYUV->h"<<tmpYUV->h<<"tmpYUV->yLineSize"<<tmpYUV->yLineSize<<"tmpYUV->uLineSize"<<tmpYUV->uLineSize<<"tmpYUV->vLineSize"<<tmpYUV->vLineSize;
    //     // qDebug()<<"tmpYUV->h"<<pFrameDecodeInfo->nHeight[0]<<"tmpYUV->yLineSize"<<pFrameDecodeInfo->nWidth[0]<<"tmpYUV->uLineSize"<<pFrameDecodeInfo->nWidth[1]<<"tmpYUV->vLineSize"<<pFrameDecodeInfo->nWidth[2];
    //     qDebug()<<"tmpYUV->setFormat";
    //     tmpYUV->setFormat(pFrameDecodeInfo->nHeight,pFrameDecodeInfo->nWidth,0);
    // }
    // qDebug()<<"before set Y";
    // tmpYUV->Y = (unsigned char*)pFrameDecodeInfo->pVideoData[0];
    // tmpYUV->U = (unsigned char*)pFrameDecodeInfo->pVideoData[1];
    // tmpYUV->V = (unsigned char*)pFrameDecodeInfo->pVideoData[2];

    // qDebug()<<"tmpYUV->yLineSize"<<tmpYUV->yLineSize<<"tmpYUV->uLineSize"<<tmpYUV->uLineSize<<"tmpYUV->vLineSize"<<tmpYUV->vLineSize;
    // stream->fillVideoSink(tmpYUV->Y,(unsigned char*)pFrameDecodeInfo->pVideoData[1],(unsigned char*)pFrameDecodeInfo->pVideoData[2]);
    // qDebug()<<"pFrameDecodeInfo->nHeight[1]"<<pFrameDecodeInfo->nHeight[1]<<"pFrameDecodeInfo->nHeight[2]"<<pFrameDecodeInfo->nHeight[2];
    // if(tmpYUV->h)
    // {
    //     emit stream->frameInfoUpdateSig(pFrameDecodeInfo->nWidth[0],pFrameDecodeInfo->nHeight[0],0);
    //     emit stream->frameDataUpdateSig();
    // }
}
static void CALLBACK realDataCallBack(
    LLONG  lRealHandle,
    DWORD  dwDataType,
    BYTE  *pBuffer,
    DWORD  dwBufSize,
    LONG   param,
    LDWORD dwUser
    )
{
    PLAY_InputData(dwUser,pBuffer,dwBufSize);
    // qDebug()<<"realDataCallBack";
}
void DHStream::startStream()
{
    // this->moveToThread(&m_thread);
    // m_thread.start();
    CLIENT_Init(deviceDisConnect,0);
    NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY sLoginIn = {0};
    NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY sLoginOut = {0};
    sLoginIn.dwSize = sizeof(NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY);
    sLoginOut.dwSize = sizeof(NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY);
    memcpy(sLoginIn.szIP,"10.12.13.108",sizeof("10.12.13.108"));
    sLoginIn.nPort = 37777;
    memcpy(sLoginIn.szUserName,"admin",sizeof("admin"));
    memcpy(sLoginIn.szPassword,"admin",sizeof("admin"));

    m_loginHandle = CLIENT_LoginWithHighLevelSecurity(&sLoginIn, &sLoginOut);
    if(m_loginHandle <= 0)
    {
        qDebug()<<"login fail"<<sLoginOut.nError;
    }
    PLAY_GetFreePort(&m_nPort);
    qDebug()<<"m_nPort"<<m_nPort;
    qDebug()<<"PLAY_SetStreamOpenMode"<<PLAY_SetStreamOpenMode(m_nPort,0);
    qDebug()<<"PLAY_OpenStream"<<PLAY_OpenStream(m_nPort,NULL,0,1024 * 10240);
    qDebug()<<"PLAY_SetDecodeCallBack"<<PLAY_SetDecodeCallBack(m_nPort,decodeRecv,this);
    qDebug()<<"play"<<PLAY_Play(m_nPort,NULL);
    LLONG lRealHandle = CLIENT_RealPlay(m_loginHandle,0,0);
    if(!lRealHandle)
    {
        qDebug()<<"open stream fail";
    }
    qDebug()<<"lRealHandle"<<lRealHandle;
    if(!CLIENT_SetRealDataCallBackEx(lRealHandle,realDataCallBack,m_nPort,REALDATA_FLAG_RAW_DATA))
    {
        qDebug()<<"open failed";
    }
}
void DHStream::stopStream()
{
    PLAY_Stop(m_nPort);
    CLIENT_StopRealPlay(m_loginHandle);
    CLIENT_Logout(m_loginHandle);
    PLAY_ReleasePort(m_nPort);
    m_thread.quit();
}
YUVData* DHStream::getFrame()
{
    return m_yuv;
}
void DHStream::setVideoSink(QVideoSink* videoSink)
{
    m_videoSink = videoSink;
}
void DHStream::fillVideoSink(unsigned char* y,unsigned char* u,unsigned char* v)
{
//     qDebug()<<"h[0]"<<m_yuv->height[0]<<"h[1]"<<m_yuv->height[1]<<"h[2]"<<m_yuv->height[2];
//     enum QVideoFrameFormat::PixelFormat pixelFormat;
// #ifdef HW_DECODE
//     pixelFormat = QVideoFrameFormat::Format_NV12;
// #else
//     pixelFormat = QVideoFrameFormat::Format_YUV420P;
// #endif
//     QVideoFrameFormat format(QSize(m_yuv->yLineSize,m_yuv->h),pixelFormat);
//     QVideoFrame videoFrame(format);
//     if( !videoFrame.map(QVideoFrame::WriteOnly)){
//         qWarning() << "QVideoFrame is not valid or not Writeable";
//     }
//     unsigned char* p0 = videoFrame.bits(0);
//     unsigned char* p1 = videoFrame.bits(1);
//     unsigned char* p2 = videoFrame.bits(2);
//     memcpy(p0,y,m_yuv->yLineSize * m_yuv->height[0]);
//     memcpy(p1,u,m_yuv->uLineSize * m_yuv->height[1]);
//     memcpy(p2,v,m_yuv->vLineSize * m_yuv->height[2]);
//     videoFrame.unmap();
//     m_videoSink->setVideoFrame(videoFrame);
}
