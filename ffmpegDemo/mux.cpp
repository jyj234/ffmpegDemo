#include "mux.h"
#include <QtCore>
Mux::Mux()
{
    const char *filename;

    int ret;
    AVDictionary *opt = NULL;
    int i;


    filename = "test2.mp4";

    /* allocate the output media context */
    avformat_alloc_output_context2(&m_oc, NULL, NULL, filename);
    if (!m_oc) {
        qCritical()<<"ffmpeg::Could not deduce output format from file extension: using MPEG.";
        avformat_alloc_output_context2(&m_oc, NULL, "mpeg", filename);
    }
    if (!m_oc)
    {
        qCritical()<<"ffmpeg::Could not deduce output format.";
        return;
     }

    m_fmt = m_oc->oformat;
    /* Add the audio and video streams using the default format codecs
      * and initialize the codecs. */
    if (m_fmt->video_codec != AV_CODEC_ID_NONE) {
        add_stream(&m_videoSt, m_oc, &m_videoCodec, m_fmt->video_codec);
        m_haveVideo = 1;
        m_encodeVideo = 1;
    }
    if (m_fmt->audio_codec != AV_CODEC_ID_NONE) {
        add_stream(&m_audioSt, m_oc, &m_audioCodec, m_fmt->audio_codec);
        m_haveAudio = 1;
        m_encodeAudio = 1;
    }

    /* Now that all the parameters are set, we can open the audio and
      * video codecs and allocate the necessary encode buffers. */
    if (m_haveVideo)
        open_video(m_oc, m_videoCodec, &m_videoSt, opt);

    if (m_haveAudio)
        open_audio(m_oc, m_audioCodec, &m_audioSt, opt);

    av_dump_format(m_oc, 0, filename, 1);

    /* open the output file, if needed */
    if (!(m_fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&m_oc->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            return;
        }
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(m_oc, &opt);
    if (ret < 0) {

        return;
    }

//    while (m_encodeVideo || m_encodeAudio) {
//        /* select the stream to encode */
//        if (m_encodeVideo &&
//            (!m_encodeAudio || av_compare_ts(m_videoSt.next_pts, m_videoSt.enc->time_base,
//                                            m_audioSt.next_pts, m_audioSt.enc->time_base) <= 0)) {
//            m_encodeVideo = !write_video_frame(m_oc, &m_videoSt);
//        } else {
//            m_encodeAudio = !write_audio_frame(m_oc, &m_audioSt);
//        }
//    }


}
void Mux::writeVideoFrame(const unsigned char* p)
{
    write_video_frame(m_oc, &m_videoSt,p);
}
void Mux::writeAudioFrame(const char* p)
{
    write_audio_frame(m_oc, &m_audioSt,p);
}
void Mux::stopRecord()
{
    if(m_haveVideo)
    {
        write_frame(m_oc, m_videoSt.enc, m_videoSt.st, NULL, m_videoSt.tmp_pkt);
    }
    if(m_haveAudio)
    {
        write_frame(m_oc, m_audioSt.enc, m_audioSt.st, NULL, m_audioSt.tmp_pkt);
    }
    av_write_trailer(m_oc);

    /* Close each codec. */
    if (m_haveVideo)
        close_stream(m_oc, &m_videoSt);
    if (m_haveAudio)
    {
        close_stream(m_oc, &m_audioSt);
    }

    if (!(m_fmt->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_closep(&m_oc->pb);

    /* free the stream */
    avformat_free_context(m_oc);
}
void Mux::log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
//    char s[512];
//    sprintf(s,"pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
//           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
//           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
//           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
//           pkt->stream_index);
//    qDebug()<<s;
}

int Mux::write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c,
                       AVStream *st, AVFrame *frame, AVPacket *pkt)
{
    int ret;
    m_mutex.lock();
    // send the frame to the encoder
    ret = avcodec_send_frame(c, frame);
//    if(c->sample_rate == 44100)
//    {
//        qDebug()<<"audio send";

//    }
//    else {
//        qDebug()<<"video send";
//    }
    if (ret < 0) {
        if(ret == AVERROR(EAGAIN))
            qDebug()<<"input is not accepted in the current state";
        else if(ret == AVERROR_EOF)
            qDebug()<<"the encoder has been flushed";
        else if(ret == AVERROR(EINVAL))
            qDebug()<<"codec not opened, it is a decoder, or requires flush";
        else if(ret == AVERROR(ENOMEM))
            qDebug()<<"failed to add packet to internal queue, or similar";
        else
            qDebug()<<"legitimate encoding errors";
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(c, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            if(c->sample_rate == 44100)
            {
                static int cnt = 0;
                qDebug()<<++cnt;
                qDebug()<<"audio empty";
            }
            else
            {
                qDebug()<<"video empty";
            }
            break;
        }
        else if (ret < 0) {
            qFatal()<<"avcodec_receive_packet failed";
            break;
        }
        qDebug()<<c->sample_rate<<pkt->pts;
        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(pkt, c->time_base, st->time_base);
        pkt->stream_index = st->index;

        /* Write the compressed frame to the media file. */
//        log_packet(fmt_ctx, pkt);
        ret = av_interleaved_write_frame(fmt_ctx, pkt);
        /* pkt is now blank (av_interleaved_write_frame() takes ownership of
          * its contents and resets pkt), so that no unreferencing is necessary.
          * This would be different if one used av_write_frame(). */
        if (ret < 0) {
            qFatal()<<"av_interleaved_write_frame failed";
            break;
        }
    }
     m_mutex.unlock();
    return ret == AVERROR_EOF ? 1 : 0;

}

/* Add an output stream. */
void Mux::add_stream(OutputStream *ost, AVFormatContext *oc,
                       const AVCodec **codec,
                       enum AVCodecID codec_id)
{
    AVCodecContext *c;
    int i;

    /* find the encoder */

    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        qFatal()<<"ffmpeg::Could not find encoder for "<<
                avcodec_get_name(codec_id);
        exit(1);
    }

    ost->tmp_pkt = av_packet_alloc();
    if (!ost->tmp_pkt) {
        qFatal()<<"ffmpeg::Could not allocate AVPacket";
        exit(1);
    }

    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        qFatal()<<"ffmpeg::Could not allocate stream";
        exit(1);
    }
    ost->st->id = oc->nb_streams-1;
    c = avcodec_alloc_context3(*codec);
    if (!c) {
        qFatal()<<"ffmpeg::Could not alloc an encoding context";
        exit(1);
    }
    ost->enc = c;

    AVChannelLayout tmp(AV_CHANNEL_LAYOUT_MASK(1,  AV_CH_FRONT_CENTER));
    switch ((*codec)->type) {
    case AVMEDIA_TYPE_AUDIO:
        c->sample_fmt  = AV_SAMPLE_FMT_FLTP;
        c->bit_rate    = 64000;
        c->sample_rate = 44100;

        av_channel_layout_copy(&c->ch_layout, &tmp);
        ost->st->time_base = AVRational({ 1, c->sample_rate });
        break;

    case AVMEDIA_TYPE_VIDEO:
        c->codec_id = codec_id;
        c->bit_rate = 800000;
        /* Resolution must be a multiple of two. */
        c->width    = 1280;
        c->height   = 720;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
          * of which frame timestamps are represented. For fixed-fps content,
          * timebase should be 1/framerate and timestamp increments should be
          * identical to 1. */
        ost->st->time_base = AVRational({ 1, 30 });
        c->framerate = AVRational({ 30, 1 });
        c->max_b_frames = 0;
        c->time_base       = ost->st->time_base;
        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = STREAM_PIX_FMT;
        c->codec_type = AVMEDIA_TYPE_VIDEO;

        break;

    default:
        break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}

/**************************************************************/
/* audio output */

AVFrame *Mux::alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  const AVChannelLayout *channel_layout,
                                  int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    av_channel_layout_copy(&frame->ch_layout, channel_layout);
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        if (av_frame_get_buffer(frame, 0) < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }

    return frame;
}

void Mux::open_audio(AVFormatContext *oc, const AVCodec *codec,
                       OutputStream *ost, AVDictionary *opt_arg)
{
    AVCodecContext *c;
    int nb_samples;
    int ret;
    AVDictionary *opt = NULL;

    c = ost->enc;

    /* open it */
    av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        exit(1);
    }

    nb_samples = 1024;

    ost->frame     = alloc_audio_frame(c->sample_fmt, &c->ch_layout,
                                   c->sample_rate, nb_samples);
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_FLTP, &c->ch_layout,
                                       c->sample_rate, nb_samples);

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

    /* create resampler context */
    ost->swr_ctx = swr_alloc();
    if (!ost->swr_ctx) {
        fprintf(stderr, "Could not allocate resampler context\n");
        exit(1);
    }

    /* set options */
    av_opt_set_chlayout  (ost->swr_ctx, "in_chlayout",       &c->ch_layout,      0);
    av_opt_set_int       (ost->swr_ctx, "in_sample_rate",     c->sample_rate,    0);
    av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",      AV_SAMPLE_FMT_FLTP, 0);
    av_opt_set_chlayout  (ost->swr_ctx, "out_chlayout",      &c->ch_layout,      0);
    av_opt_set_int       (ost->swr_ctx, "out_sample_rate",    c->sample_rate,    0);
    av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",     c->sample_fmt,     0);

    /* initialize the resampling context */
    if ((ret = swr_init(ost->swr_ctx)) < 0) {
        fprintf(stderr, "Failed to initialize the resampling context\n");
        exit(1);
    }
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
  * 'nb_channels' channels. */
AVFrame *Mux::get_audio_frame(OutputStream *ost,const char* p)
{
    if(p == NULL)
        return NULL;
    AVFrame *frame = ost->tmp_frame;
    int j, i, v;
    int16_t *q = (int16_t*)frame->data[0];
    memcpy(q,p,frame->nb_samples * 4 * 1);
    frame->pts = ost->next_pts;
    ost->next_pts  += frame->nb_samples;

    return frame;
}

/*
  * encode one audio frame and send it to the muxer
  * return 1 when encoding is finished, 0 otherwise
  */
int Mux::write_audio_frame(AVFormatContext *oc, OutputStream *ost,const char* p)
{
    AVCodecContext *c;
    AVFrame *frame;
    int ret;
    int dst_nb_samples;

    c = ost->enc;

    frame = get_audio_frame(ost,p);

    if (frame) {
        /* convert samples from native format to destination codec format, using the resampler */
        /* compute destination number of samples */
        dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples,
                                        c->sample_rate, c->sample_rate, AV_ROUND_UP);
        av_assert0(dst_nb_samples == frame->nb_samples);

        /* when we pass a frame to the encoder, it may keep a reference to it
          * internally;
          * make sure we do not overwrite it here
          */
        ret = av_frame_make_writable(ost->frame);
        if (ret < 0)
            exit(1);

        /* convert to destination format */
        ret = swr_convert(ost->swr_ctx,
                          ost->frame->data, dst_nb_samples,
                          (const uint8_t **)frame->data, frame->nb_samples);
        if (ret < 0) {
            qFatal()<< "Error while converting\n";
            exit(1);
        }
        frame = ost->frame;

        frame->pts = av_rescale_q(ost->samples_count, AVRational({1, c->sample_rate}), c->time_base);
        ost->samples_count += dst_nb_samples;
    }

    return write_frame(oc, c, ost->st, frame, ost->tmp_pkt);
}

/**************************************************************/
/* video output */

AVFrame *Mux::alloc_frame(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *frame;
    int ret;

    frame = av_frame_alloc();
    if (!frame)
        return NULL;

    frame->format = pix_fmt;
    frame->width  = width;
    frame->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return frame;
}

void Mux::open_video(AVFormatContext *oc, const AVCodec *codec,
                       OutputStream *ost, AVDictionary *opt_arg)
{
    int ret;
    AVCodecContext *c = ost->enc;
    AVDictionary *opt = NULL;

    av_dict_copy(&opt, opt_arg, 0);

    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        exit(1);
    }

    /* allocate and init a re-usable frame */
    ost->frame = alloc_frame(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        exit(1);
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        qFatal()<< "Could not copy the stream parameters\n";
        exit(1);
    }
}

/* Prepare a dummy image. */
void Mux::fill_yuv_image(AVFrame *pict, const unsigned char* p,
                           int width, int height)
{
    int x, y, i;
    for(int i = 0;i<width * height * 2;i+=4)
    {
        pict->data[0][i / 2] = p[i];
        pict->data[1][i / 4] = p[i + 1];
        pict->data[0][i / 2 + 1] = p[i + 2];
        pict->data[2][i / 4] = p[i + 3];
    }
}

AVFrame *Mux::get_video_frame(OutputStream *ost,const unsigned char* p)
{
    AVCodecContext *c = ost->enc;

    /* when we pass a frame to the encoder, it may keep a reference to it
      * internally; make sure we do not overwrite it here */
    if (av_frame_make_writable(ost->frame) < 0)
    {
        qFatal()<<"av_frame_make_writable failed";
        exit(1);
    }

    fill_yuv_image(ost->frame, p, c->width, c->height);
    ost->frame->pts = ost->next_pts++;

    return ost->frame;
}

/*
  * encode one video frame and send it to the muxer
  * return 1 when encoding is finished, 0 otherwise
  */
int Mux::write_video_frame(AVFormatContext *oc, OutputStream *ost,const unsigned char* p)
{
    return write_frame(oc, ost->enc, ost->st, get_video_frame(ost,p), ost->tmp_pkt);
}

void Mux::close_stream(AVFormatContext *oc, OutputStream *ost)
{

//    av_interleaved_write_frame(m_oc,NULL);
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    av_packet_free(&ost->tmp_pkt);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}
