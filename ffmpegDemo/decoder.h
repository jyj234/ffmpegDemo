#ifndef DECODER_H
#define DECODER_H

#include <QObject>
#include <QtCore>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QPair>
extern "C"{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include<libavutil/time.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

class VideoItem;
typedef struct YUVData{
    YUVData(int w,int h)
    {
        yLineSize = w;
        uLineSize = w / 2;
        vLineSize = w / 2;
        this->h = h;
    }
    ~YUVData(){
    }

    uint8_t* Y = NULL;
    uint8_t* U = NULL;
    uint8_t* V = NULL;
    uint8_t* data;
    int yLineSize;
    int uLineSize;
    int vLineSize;
    int h;
}YUVData;
class Decoder :public QObject
{
    Q_OBJECT
public:
    Decoder(VideoItem* parent);
    ~Decoder();
    YUVData* getFrame();
    void pushData(unsigned char* data,unsigned long len);
    QPair<unsigned char *,unsigned long> m_streamData;
    QMutex m_streamMutex;
    QQueue<QPair<unsigned char*,unsigned long>> m_queue;
    bool isOver = false;
    unsigned char *m_aviobuffer;
    AVIOContext *m_avioCtx;
    void setIsOver();
    QWaitCondition m_dataAvailableCondition;   // 条件变量，队列中数据可用时等待/通知
    QWaitCondition m_dataNotFullCondition;     // 条件变量，队列非满时等待/通知
    unsigned char* m_dataBuffer;
    unsigned long m_dataBufferMaxSize;
    unsigned long m_dataBufferCurSize;
    // void freeDecoder();
    //    static Decoder* instance()
    //    {
    //        static Decoder* ins = new Decoder();
    //        return ins;
    //    }
signals:
    void frameInfoUpdateSig(int width,int height,int format);
    void frameDataUpdateSig();
    void audioDataUpdateSig(const char *data, qint64* len,QMutex* audioMutex);
    void frameInfoChangedSig();
    void videoErrorSig();
    void loadSuccessSig();
public slots:
    void startDecode();
    void setUrl(QString url);
    void stopDecode();

private:
    int output_video_frame(AVFrame *frame);
    int output_audio_frame(AVFrame *frame);
    int decode_packet(AVCodecContext *dec, const AVPacket *pkt);
    int open_codec_context(int *stream_idx,
                           AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                            const enum AVPixelFormat *pix_fmts);
    int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type);

private:
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx;
    int width, height;
    enum AVPixelFormat pix_fmt;
    AVStream *video_stream = NULL, *audio_stream = NULL;
    FILE *video_dst_file = NULL;
    FILE *audio_dst_file = NULL;

    uint8_t *video_dst_data = {NULL};

    int      video_dst_linesize[4];
    int video_dst_bufsize;

    int video_stream_idx = -1, audio_stream_idx = -1;
    AVFrame *frame = NULL;
    AVFrame *sw_frame = NULL;
    AVFrame *tmp_frame = NULL;
    AVPacket *pkt = NULL;
    int video_frame_count = 0;
    int audio_frame_count = 0;

    enum AVHWDeviceType type;
    static enum AVPixelFormat hw_pix_fmt;
    AVBufferRef *hw_device_ctx = NULL;


    // QThread m_thread;
    QMutex m_mutex;

    QString m_url;

    YUVData* yuv;

    SwsContext   *m_sws_ctx;
    SwrContext   *m_swr_ctx;
    int yuv420p_linesize[3];
    uint8_t* yuv420p_data[3];
    uint8_t * m_audioOutBuffer = NULL;
    uint8_t * m_audioMediumBuffer = NULL;
    qint64 m_audioMediumBufferSize;
    AVSampleFormat m_outSampleFormat;
    int m_outSampleRate;
    int m_errorCnt;
    VideoItem* m_videoItem;
    qint64 m_outsize = 0;
    QMutex m_audioMutex;
    bool m_isFirstFrame;


};

#endif // DECODER_H
