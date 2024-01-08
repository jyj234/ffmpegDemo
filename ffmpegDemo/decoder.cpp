#include "decoder.h"
#define HW_DECODE
typedef enum AVPixelFormat (*AVPixelFormatFunc)(AVCodecContext *, const enum AVPixelFormat *);
enum AVPixelFormat Decoder::hw_pix_fmt = AV_PIX_FMT_NONE;
Decoder::Decoder()
{
}
Decoder::~Decoder()
{

}
int Decoder::output_video_frame(AVFrame *frame)
{
//    qDebug()<<"start output_video_frame";
//    if (frame->width != width || frame->height != height ||
//        frame->format != pix_fmt) {
//        /* To handle this change, one could call av_image_alloc again and
//         * decode the following frames into another rawvideo file. */
//        fprintf(stderr, "Error: Width, height and pixel format have to be "
//                        "constant in a rawvideo file, but the width, height or "
//                        "pixel format of the input video changed:\n"
//                        "old: width = %d, height = %d, format = %s\n"
//                        "new: width = %d, height = %d, format = %s\n",
//                width, height, av_get_pix_fmt_name(pix_fmt),
//                frame->width, frame->height,
//                av_get_pix_fmt_name((AVPixelFormat)frame->format));
//        return -1;
//    }
//    qDebug()<<"format"<<av_get_pix_fmt_name(pix_fmt)<<"buffer size"<<video_dst_bufsize<<"linesize[0]"<<frame->linesize[0];
//    qDebug()<<"video_frame n:"<<video_frame_count++;
//    qDebug()<<"width"<<frame->width;
    if (frame->format == hw_pix_fmt) {
        /* retrieve data from GPU to CPU */
        if ((av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
            qDebug()<<"Error transferring the data to system memory";
            return -1;
        }
        tmp_frame = sw_frame;
//        qDebug()<<"hw frame";
    } else
        tmp_frame = frame;

    /* copy decoded frame to destination buffer:
     * this is required since rawvideo expects non aligned data */
//    av_image_copy2(video_dst_data, video_dst_linesize,
//                   frame->data, frame->linesize,
//                   pix_fmt, width, height);
//    int size = av_image_get_buffer_size((AVPixelFormat)tmp_frame->format, tmp_frame->width,
//                                    tmp_frame->height, 1);
//    qDebug()<<"format"<<tmp_frame->format;
    av_image_copy_to_buffer(video_dst_data, video_dst_bufsize,
                                  (const uint8_t * const *)tmp_frame->data,
                            (const int *)tmp_frame->linesize, (AVPixelFormat)tmp_frame->format,
                                  tmp_frame->width, tmp_frame->height,1);
    /* write to rawvideo file */
//    fwrite(video_dst_data[0], 1, video_dst_bufsize, video_dst_file);
    enum QVideoFrameFormat::PixelFormat pixelFormat;
#ifdef HW_DECODE
    pixelFormat = QVideoFrameFormat::Format_NV12;
#else
    pixelFormat = QVideoFrameFormat::Format_YUV420P;
#endif
    QVideoFrameFormat format(QSize(frame->width,frame->height),pixelFormat);
    QVideoFrame videoFrame(format);
    if( !videoFrame.map(QVideoFrame::WriteOnly)){
        qWarning() << "QVideoFrame is not valid or not Writeable";
        return 0;
    }
    uint8_t* p1 = videoFrame.bits(0);
    memcpy(p1,video_dst_data,video_dst_bufsize);
    m_videoSink->setVideoFrame(videoFrame);
    videoFrame.unmap();
    return 0;
}

int Decoder::output_audio_frame(AVFrame *frame)
{
    size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
//    printf("audio_frame n:%d nb_samples:%d pts:%s\n",
//           audio_frame_count++, frame->nb_samples,
//           av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));

    /* Write the raw audio data samples of the first plane. This works
     * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
     * most audio decoders output planar audio, which uses a separate
     * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
     * In other words, this code will write only the first audio channel
     * in these cases.
     * You should use libswresample or libavfilter to convert the frame
     * to packed data. */
//    fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);

    return 0;
}

int Decoder::decode_packet(AVCodecContext *dec, const AVPacket *pkt)
{
    int ret = 0;
//    qDebug()<<"start decode_packet"<<dec->codec->type;
    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
//    qDebug()<<"after avcodec_send_packet";
    if (ret < 0) {
        qDebug()<<"Error submitting a packet for decoding ";
//        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec, frame);
        if (ret < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;
            qDebug()<<"Error during decoding";
            //            fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
            return ret;
        }
//        qDebug()<<"before output_video_frame";
        // write the frame data to output file
        if (dec->codec->type == AVMEDIA_TYPE_VIDEO)
            ret = output_video_frame(frame);
        else
            ret = output_audio_frame(frame);

        av_frame_unref(frame);
        if (ret < 0)
            return ret;
    }

    return 0;
}
int Decoder::hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
{
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
                                      NULL, NULL, 0)) < 0) {
        qFatal()<<"Failed to create specified HW device";
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    return err;
}
int Decoder::open_codec_context(int *stream_idx,
                                AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType stream_type)
{
    int ret, stream_index;
    AVStream *st;
    const AVCodec *dec = NULL;

    ret = av_find_best_stream(fmt_ctx, stream_type, -1, -1, NULL, 0);
    if (ret < 0) {
        qWarning()<<"Could not find"<<av_get_media_type_string(stream_type)<<"stream in input file"<<src_filename;
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            qFatal()<<"Failed to find %s codec";
            return AVERROR(EINVAL);
        }
#ifdef HW_DECODE
        if(stream_type == AVMEDIA_TYPE_VIDEO)
        {
            for (int i = 0;; i++) {
                const AVCodecHWConfig *config = avcodec_get_hw_config(dec, i);
                if (!config) {
                    qDebug()<<"Decoder"<<dec->name<<"does not support device type"<<av_hwdevice_get_type_name(AVHWDeviceType(type));
                    return -1;
                }
                if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                    config->device_type == type) {
                    hw_pix_fmt = config->pix_fmt;
                    qDebug()<<"hw_pix_fmt"<<hw_pix_fmt;
                    break;
                }
            }
        }
#endif
        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            qFatal()<<"Failed to allocate the"<<av_get_media_type_string(stream_type)<<"codec context";
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            qFatal()<<"Failed to copy"<<av_get_media_type_string(stream_type)<<"codec parameters to decoder context\n";
            return ret;
        }
#ifdef HW_DECODE
        if(stream_type == AVMEDIA_TYPE_VIDEO)
        {
            (*dec_ctx)->get_format  = (AVPixelFormatFunc) &Decoder::get_hw_format;
            if (hw_decoder_init(*dec_ctx, (AVHWDeviceType)type) < 0)
                return -1;
        }
#endif
        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
            qWarning()<<"Failed to open"<<av_get_media_type_string(stream_type)<<"codec";
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

int Decoder::get_format_from_sample_fmt(const char **fmt,
                                        enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
                              { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
                              { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
                              { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
                              { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
                              { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
                              };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }
    qFatal()<<"sample format"<<av_get_sample_fmt_name(sample_fmt)<<"is not supported as output format";
    return -1;
}
enum AVPixelFormat Decoder::get_hw_format(AVCodecContext *ctx,
                                          const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }
    qFatal()<<"Failed to get HW surface format";
    return AV_PIX_FMT_NONE;
}
void Decoder::startDecode()
{
    int ret = 0;
    src_filename = "rtsp://admin:123456@10.12.13.237:554/H264?ch=1&subtype=0";
    //    src_filename = "record.dat";
    video_dst_filename = "videoDes";
    audio_dst_filename = "audioDes";
    while((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
        qDebug()<<av_hwdevice_get_type_name(type);
#ifdef HW_DECODE
    char hwDevice[] ="dxva2";
    type = av_hwdevice_find_type_by_name(hwDevice);
    if (type == AV_HWDEVICE_TYPE_NONE) {
        qDebug()<<"Device type"<<hwDevice<<"is not supported";
        qDebug()<<"Available device types:";
        while((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
            qDebug()<<av_hwdevice_get_type_name(type);
        return;
    }
#endif
    qDebug()<<"here1";
    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
        qDebug()<<"Could not open source file"<<src_filename;
        return;
    }
    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        qDebug()<<"Could not find stream information\n";
        return;
    }
    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        video_stream = fmt_ctx->streams[video_stream_idx];
        /* allocate image where the decoded image will be put */
        width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        if(hw_pix_fmt != AV_PIX_FMT_NONE)
            pix_fmt = AV_PIX_FMT_NV12;
        else
            pix_fmt = video_dec_ctx->pix_fmt;
        video_dst_bufsize = av_image_get_buffer_size(pix_fmt,width,
                                        height, 1);
        video_dst_data = (uint8_t*)av_malloc(video_dst_bufsize);
        if (ret < 0) {
            qFatal()<<"Could not allocate raw video buffer";
            goto end;
        }
        qDebug()<<"video_dst_bufsize"<<video_dst_bufsize;
    }
    if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        audio_stream = fmt_ctx->streams[audio_stream_idx];
    }
    qDebug()<<"here2";
    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, src_filename, 0);
    if (!audio_stream && !video_stream) {
        qFatal()<<"Could not find audio or video stream in the input, aborting";
        ret = 1;
        goto end;
    }

    frame = av_frame_alloc();
    sw_frame = av_frame_alloc();
    if (!frame) {
        qFatal()<<"Could not allocate frame";
        ret = AVERROR(ENOMEM);
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        qFatal()<<"Could not allocate packet";
        ret = AVERROR(ENOMEM);
        goto end;
    }
    qDebug()<<"here3";

    /* read frames from the file */

    m_mutex.lock();
    while (av_read_frame(fmt_ctx, pkt) >= 0 ) {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if (pkt->stream_index == video_stream_idx)
            ret = decode_packet(video_dec_ctx, pkt);
        else if (pkt->stream_index == audio_stream_idx)
            ret = decode_packet(audio_dec_ctx, pkt);
        av_packet_unref(pkt);
        if (ret < 0 || isOver)
        {
            break;
        }


    }
    m_mutex.unlock();
    qDebug()<<"after over";

end:

    return;
}
void Decoder::setVideoSink(QVideoSink* videoSink)
{
    m_videoSink = videoSink;
}
void Decoder::moveThread()
{
    qDebug()<<"start thread";
    Decoder::instance()->moveToThread(&m_thread);
    QObject::connect(&m_thread, &QThread::started, Decoder::instance(), &Decoder::startDecode);
    m_thread.start();
}
void Decoder::quitThread()
{
    qDebug()<<"quit thread";
    m_thread.quit();
}
void Decoder::stopRecord()
{
    isOver = true;
    m_mutex.lock();
    if (video_dec_ctx)
        decode_packet(video_dec_ctx, NULL);
    if (audio_dec_ctx)
        decode_packet(audio_dec_ctx, NULL);

    printf("Demuxing succeeded.\n");

    if (video_stream) {
        qDebug()<<"Play the output video file with the command:";
        qDebug()<<"ffplay -f rawvideo -pix_fmt"<<av_get_pix_fmt_name(pix_fmt)<<"-video_size"<<width<<"x"<<height<<video_dst_filename;
    }

    if (audio_stream) {
        enum AVSampleFormat sfmt = audio_dec_ctx->sample_fmt;
        int n_channels = audio_dec_ctx->ch_layout.nb_channels;
        const char *fmt;

        if (av_sample_fmt_is_planar(sfmt)) {
            const char *packed = av_get_sample_fmt_name(sfmt);
            qDebug()<<"Warning: the sample format the decoder produced is planar "<<packed<<"This example will output the first channel only.\n";
            sfmt = av_get_packed_sample_fmt(sfmt);
            n_channels = 1;
        }

        if ((get_format_from_sample_fmt(&fmt, sfmt)) < 0)
            goto out;

        qDebug()<<"Play the output audio file with the command:";
        qDebug()<<"ffplay -f"<<fmt<<"-ac"<<n_channels<<"-ar"<<audio_dec_ctx->sample_rate<<audio_dst_filename;

    }
    //    qDebug()<<"before avcodec_free_context";
    avcodec_free_context(&video_dec_ctx);
    //    qDebug()<<"before avcodec_free_context";
    avcodec_free_context(&audio_dec_ctx);
    //    qDebug()<<"before avformat_close_input";
    avformat_close_input(&fmt_ctx);
    //    qDebug()<<"before av_packet_free";
    av_packet_free(&pkt);
    //    qDebug()<<"before av_frame_free";
    av_frame_free(&frame);
    av_frame_free(&sw_frame);
    //    qDebug()<<"before av_free";
    av_free(video_dst_data);
out:
    m_mutex.unlock();

}
