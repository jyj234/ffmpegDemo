#ifndef DECODER_H
#define DECODER_H

#include <QObject>
#include <QtCore>
#include <QImage>
#include<QVideoSink>
#include <QVideoFrame>
#include <QThread>
#include <QMutex>
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
typedef struct YUVData{
   YUVData(int w,int h)
   {
       yLineSize = w;
       uLineSize = w;
       vLineSize = w;
       this->h = h;
       Y = new uint8_t[w * h];
       U = new uint8_t[w * h / 2];
       V = new uint8_t[w * h / 2];
   }
   ~YUVData(){
       // if(Y)
       //  delete []Y;
       // if(U)
       //  delete []U;
       // if(V)
       //  delete []V;
   }

   uint8_t* Y;
   uint8_t* U;
   uint8_t* V;
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
    Decoder();
    ~Decoder();
    YUVData* getFrame();
//    static Decoder* instance()
//    {
//        static Decoder* ins = new Decoder();
//        return ins;
//    }
signals:
    void frameInfoUpdateSig(int width,int height,int format);
    void frameDataUpdateSig();
    void audioDataUpdateSig(const char *data, qint64 len);
public slots:
    void startDecode();
    void moveThread(QString url);
    void quitThread();
    void stopRecord();
private:
    int output_video_frame(AVFrame *frame);
    int output_audio_frame(AVFrame *frame);
    int decode_packet(AVCodecContext *dec, const AVPacket *pkt);
    int open_codec_context(int *stream_idx,
                           AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    int get_format_from_sample_fmt(const char **fmt,
                                   enum AVSampleFormat sample_fmt);
    static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                                     const enum AVPixelFormat *pix_fmts);
    int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type);

private:
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx;
    int width, height;
    enum AVPixelFormat pix_fmt;
    AVStream *video_stream = NULL, *audio_stream = NULL;
    const char *src_filename = NULL;
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
    bool isOver = false;

    QThread m_thread;
    QMutex m_mutex;

    char* m_url;

   YUVData* yuv;

   SwsContext   *m_sws_ctx;
   SwrContext   *m_swr_ctx;
   int yuv420p_linesize[3];
   uint8_t* yuv420p_data[3];
   uint8_t * m_audioOutBuffer = NULL;
   AVSampleFormat m_outSampleFormat;
   int m_outSampleRate;
};

#endif // DECODER_H
