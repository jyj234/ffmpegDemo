#ifndef MUX_H
#define MUX_H
extern "C"{
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#include <mutex>
#include <QMutex>
#define STREAM_DURATION   10.0
#define STREAM_FRAME_RATE 30 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV422P /* default pix_fmt */

#define SCALE_FLAGS SWS_BICUBIC
typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    AVPacket *tmp_pkt;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;
class Mux
{
public:
    Mux();
    void writeVideoFrame(const unsigned char* p);
    void writeAudioFrame(const char* p);
    void stopRecord();
private:
    void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt);
    int write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c,
                    AVStream *st, AVFrame *frame, AVPacket *pkt);
    void add_stream(OutputStream *ost, AVFormatContext *oc,
                           const AVCodec **codec,
                           enum AVCodecID codec_id);
    AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                      const AVChannelLayout *channel_layout,
                                      int sample_rate, int nb_samples);
    void open_audio(AVFormatContext *oc, const AVCodec *codec,
                           OutputStream *ost, AVDictionary *opt_arg);
    AVFrame *get_audio_frame(OutputStream *ost,const char* p);
    int write_audio_frame(AVFormatContext *oc, OutputStream *ost,const char * p);
    AVFrame *alloc_frame(enum AVPixelFormat pix_fmt, int width, int height);
    void open_video(AVFormatContext *oc, const AVCodec *codec,
                    OutputStream *ost, AVDictionary *opt_arg);
    void fill_yuv_image(AVFrame *pict, const unsigned char* p,
                        int width, int height);
    AVFrame *get_video_frame(OutputStream *ost,const unsigned char* p);
    int write_video_frame(AVFormatContext *oc, OutputStream *ost,const unsigned char* p);
    void close_stream(AVFormatContext *oc, OutputStream *ost);
private:
    AVFormatContext *m_oc;
    int m_haveVideo = 0;
    int m_haveAudio = 0;
    int m_encodeVideo = 0;
    int m_encodeAudio = 0;
    const AVOutputFormat * m_fmt;
    const AVCodec *m_audioCodec;
    const AVCodec *m_videoCodec;
    OutputStream m_videoSt = { 0 };
    OutputStream m_audioSt = { 0 };
    std::mutex m_mutex;
};

#endif // MUX_H
