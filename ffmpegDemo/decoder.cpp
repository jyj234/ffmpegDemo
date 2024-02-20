#include "decoder.h"
#include "videoitem.h"
//#define HW_DECODE
typedef enum AVPixelFormat (*AVPixelFormatFunc)(AVCodecContext *, const enum AVPixelFormat *);
enum AVPixelFormat Decoder::hw_pix_fmt = AV_PIX_FMT_NONE;
// static FILE* file = fopen("D:/ffmpeg-6.0-essentials_build/bin/audiodata","wb");
#define MAX_AUDIO_FRAME_SIZE 50 * 1024
#define MAX_AVIO_SIZE 1000 * 1024
#define MAX_PACKET_SIZE 800 * 1024
Decoder::Decoder(VideoItem* parent)
    : m_swr_ctx(NULL)
    , m_sws_ctx(NULL)
    , fmt_ctx(NULL)
    , audio_stream(NULL)
    , video_stream(NULL)
    , m_errorCnt(0)
    , m_audioOutBuffer(NULL)
    , m_audioMediumBuffer(NULL)
    , m_dataBufferCurSize(0)
    , m_audioMediumBufferSize(0)
    , m_avioCtx(NULL)
    , m_dataBuffer(NULL)
    , m_isFirstFrame(false)

{
    m_dataBuffer = new unsigned char[MAX_PACKET_SIZE];
    m_dataBufferMaxSize = MAX_PACKET_SIZE;
    m_videoItem = parent;
}
Decoder::~Decoder()
{
    qDebug()<<"before stopDecode";
    stopDecode();
    qDebug()<<"after stopDecode";
    if(m_dataBuffer)
        delete []m_dataBuffer;
    // fclose(file);
}
int Decoder::output_video_frame(AVFrame *frame)
{
    // qDebug()<<"start output_video_frame";
    if (frame->width != width || frame->height != height )
    {
        /* To handle this change, one could call av_image_alloc again and
            * decode the following frames into another rawvideo file. */
        qDebug()<<"changed"<<"now: width"<<width<<"height"<<height;
        qDebug()<<"frame:"<<"width"<<frame->width<<"height"<<frame->height;
        return -1;
    }
    if(!m_isFirstFrame && frame->pict_type ==AV_PICTURE_TYPE_I)
    {
        m_isFirstFrame = true;
        emit loadSuccessSig();
    }
    if(!m_isFirstFrame)
        return 0;

    // qDebug()<<"format"<<av_get_pix_fmt_name(pix_fmt)<<"buffer size"<<video_dst_bufsize<<"linesize[0]"<<frame->linesize[0];
    //    qDebug()<<"video_frame n:"<<video_frame_count++;
    // qDebug()<<"width"<<frame->width;
    if (frame->format == hw_pix_fmt) {
        /* retrieve data from GPU to CPU */
        if ((av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
            qDebug()<<"Error transferring the data to system memory";
            return -1;
        }
        tmp_frame = sw_frame;
    } else
    {
        tmp_frame = frame;
    }
    /* copy decoded frame to destination buffer:*/

    sws_scale(m_sws_ctx,
              (const uint8_t* const*) &tmp_frame->data,  // 源数据
              (int*) &tmp_frame->linesize,  // 源数据的行大小
              0,              // 从源数据的第几行开始转换
              height,   // 要转换的行数
              yuv420p_data,   // 目标数据
              yuv420p_linesize); // 目标数据的行大小

    yuv->Y = yuv420p_data[0];
    yuv->U = yuv420p_data[1];
    yuv->V = yuv420p_data[2];
    emit frameDataUpdateSig();
    // qDebug()<<"frameDataUpdateSig";

    return 0;
}

int Decoder::output_audio_frame(AVFrame *frame)
{
    // qDebug()<<"frame->format"<<frame->format;
    // qDebug()<<"unpadded_linesize"<<unpadded_linesize<<"nb_samples"<<frame->nb_samples;
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
    // fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);
    // qDebug()<<"output_audio_frame";
    // swr_convert(sws_ctx_ForAudio, &buf, frame_ForAudio->nb_samples, (const uint8_t**)(frame_ForAudio->data), frame_ForAudio->nb_samples);
    // qDebug()<<"start output_audio_frame";
    int len = swr_convert(m_swr_ctx,
                          &m_audioOutBuffer,
                          MAX_AUDIO_FRAME_SIZE,
                          (const uint8_t**)frame->data,
                          frame->nb_samples);
    // while(m_outsize != 0)
    // {
    //     QThread::usleep(100);
    // }
    m_outsize = av_samples_get_buffer_size(NULL,
                                           frame->ch_layout.nb_channels,
                                           len,
                                           m_outSampleFormat,
                                           1);
    //qDebug("buffer size is: %d.",dst_bufsize);
    // int sleep_time=(m_outSampleRate*16*2/8)/out_size;
    // qDebug()<<"out_size"<<out_size<<"len"<<len<<"linesize[0]"<<frame->linesize[0];
    // qDebug()<<out_size;
    // fwrite((const char*)m_audioOutBuffer,1,out_size,file);
    // m_videoItem->onAudioFrameDataUpdateSig((const char*)m_audioOutBuffer,out_size);
    if(m_outsize == 0)
        return 0;
    m_audioMutex.lock();
    memcpy(m_audioMediumBuffer + m_audioMediumBufferSize,m_audioOutBuffer,m_outsize);
    m_audioMediumBufferSize += m_outsize;
    m_audioMutex.unlock();
    emit audioDataUpdateSig((const char*)m_audioMediumBuffer,(&m_audioMediumBufferSize),&m_audioMutex);
    // m_audioMediumBufferSize += out_size;
    // if(m_audioMediumBufferSize >= 10 * out_size)
    // {
    //     emit audioDataUpdateSig((const char*)m_audioMediumBuffer,m_audioMediumBufferSize);
    //     m_audioMediumBufferSize = 0;
    // }
    // qDebug()<<"emit audioDataUpdateSig((const char*)m_audioOutBuffer,out_size)";

    return 0;
}

int Decoder::decode_packet(AVCodecContext *dec, const AVPacket *pkt)
{
    int ret = 0;
    // qDebug()<<"start decode_packet"<<dec->codec->type;
    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
    // qDebug()<<"after avcodec_send_packet";
    if (ret < 0) {
        qDebug()<<"Error submitting a packet for decoding ";
        //        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0) {
        // qDebug()<<"before avcodec_receive_frame";
        ret = avcodec_receive_frame(dec, frame);
        // qDebug()<<"after avcodec_receive_frame"<<dec->codec->type;
        if(ret == 0)
            m_errorCnt = 0;
        if (ret < 0) {
            ++m_errorCnt;
            if(m_errorCnt >= 100)
            {
                qDebug()<<"m_errorCnt>=100";
                return -1;
            }
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;
            qDebug()<<"Error during decoding";
            //            fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
            return ret;
        }
        // qDebug()<<"before output_video_frame";
        // write the frame data to output file
        if (dec->codec->type == AVMEDIA_TYPE_VIDEO)
            ret = output_video_frame(frame);
        else
            ret = output_audio_frame(frame);

        av_frame_unref(frame);
        if (ret < 0)
        {
            qDebug()<<"output_video_frame fail";
            return ret;
        }
    }

    return 0;
}
int Decoder::hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
{
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
                                      NULL, NULL, 0)) < 0) {
        qDebug()<<"Failed to create specified HW device";
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
        qWarning()<<"Could not find"<<av_get_media_type_string(stream_type)<<"stream in input file"<<m_url;
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            qDebug()<<"Failed to find codec";
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
            qDebug()<<"Failed to allocate the"<<av_get_media_type_string(stream_type)<<"codec context";
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            qDebug()<<"Failed to copy"<<av_get_media_type_string(stream_type)<<"codec parameters to decoder context\n";
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
enum AVPixelFormat Decoder::get_hw_format(AVCodecContext *ctx,
                                          const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }
    qDebug()<<"Failed to get HW surface format";
    return AV_PIX_FMT_NONE;
}
//Callback
static int read_buffer(void *opaque, uint8_t *buf, int buf_size){

    Decoder* decoder = (Decoder*)opaque;
    decoder->m_streamMutex.lock();
    // qDebug()<<"read_buffer";
    // QPair<unsigned char *,DWORD>* tmpData = &(((Decoder*)opaque)->m_streamData);
    QQueue<QPair<unsigned char*,unsigned long>>* tmpData = &(decoder->m_queue);
    int len = 0;
    unsigned char* data = NULL;
    int totallen = 0;
    // if(tmpData->second == 0)
    // {
    //     ((Decoder*)opaque)->m_streamMutex.unlock();
    //     return 0;
    // }
    // memcpy(buf,tmpData->first,tmpData->second);
    // DWORD len = tmpData->second;
    // tmpData->second = 0;
    if(decoder->isOver)
    {
        // while(!tmpData->empty())
        // {
        //     delete tmpData->head().first;
        //     tmpData->pop_front();
        // }
        totallen = -1;
        // if(decoder->m_avioCtx && decoder->m_avioCtx->buffer)
        //     av_freep(decoder->m_avioCtx->buffer);
        qDebug()<<"read buffer end";
        goto out;
    }
    // qDebug()<<"before m_dataAvailableCondition.wait";
    decoder->m_dataAvailableCondition.wait(&decoder->m_streamMutex,100);
    // qDebug()<<"after m_dataAvailableCondition.wait";
    if(decoder->m_dataBufferCurSize == 0)
    {
        totallen = 0;
        goto out;
    }
    memcpy(buf,decoder->m_dataBuffer,decoder->m_dataBufferCurSize);
    totallen = decoder->m_dataBufferCurSize;
    decoder->m_dataBufferCurSize = 0;
    // while(!tmpData->empty())
    // {
    //     data = tmpData->head().first;
    //     len = tmpData->head().second;
    //     // qDebug()<<"maxlen"<<maxlen;
    //     if(totallen + len > buf_size)
    //     {
    //         goto out;
    //     }
    //     memcpy(buf + totallen,data,len);
    //     tmpData->pop_front();
    //     delete data;
    //     totallen += len;
    // }
    // qDebug()<<"read buffer"<<totallen;
    decoder->m_dataNotFullCondition.wakeOne();
out:
    decoder->m_streamMutex.unlock();
    return totallen;

}
void Decoder::pushData(unsigned char* data,unsigned long len)
{
    if(!data)
        return;
    m_streamMutex.lock();
    // m_dataNotFullCondition.wait(&m_streamMutex);


    unsigned char* tmpData = NULL;
    if( isOver)
    {
        goto out;
    }
    // qDebug()<<"before m_dataNotFullCondition.wait";
    if(m_dataBufferCurSize + len > m_dataBufferMaxSize)
    {
        m_dataNotFullCondition.wait(&m_streamMutex,50);
    }
    if(m_dataBufferCurSize + len > m_dataBufferMaxSize)
    {
        goto out;
    }
    // qDebug()<<"after m_dataNotFullCondition.wait";
    memcpy(m_dataBuffer + m_dataBufferCurSize, data,len);
    m_dataBufferCurSize += len;
    // tmpData = new unsigned char[len];
    // memcpy(tmpData,data,len);
    // m_queue.push_back({tmpData,len});
    // qDebug()<<"pushdata"<<len<<m_streamData.second;

// if(m_streamData.second + len > 300 * 1024)
// {
//     qDebug()<<"out of range";
//     m_streamMutex.unlock();
//     return;
// }
// memcpy(m_streamData.first + m_streamData.second, data , len);
// m_streamData.second += len;
// qDebug()<<"pushData"<<len;
out:
    m_dataAvailableCondition.wakeOne();
    m_streamMutex.unlock();
}
void Decoder::startDecode()
{
    m_mutex.lock();
    bool isError = false;
    int ret = 0;
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
    if(m_url.isEmpty())
    {
        fmt_ctx = avformat_alloc_context();
        m_aviobuffer=(unsigned char *)av_malloc(MAX_AVIO_SIZE);
        qDebug("before avio_alloc_context");
        //AVIOContext中的缓存
        m_avioCtx=avio_alloc_context(m_aviobuffer, MAX_AVIO_SIZE,0,this,read_buffer,NULL,NULL);
        m_avioCtx->max_packet_size = MAX_PACKET_SIZE;
        qDebug("after avio_alloc_context");
        fmt_ctx->pb=m_avioCtx;
        fmt_ctx->max_analyze_duration = 1 * AV_TIME_BASE;
    }
    /* open input file, and allocate format context */
    qDebug("before avformat_open_input");
    if (avformat_open_input(&fmt_ctx, m_url.toUtf8().data(), NULL, NULL) < 0) {
        qDebug()<<"Could not open source file"<<m_url;
        goto end;
    }
    qDebug("before open stream");
    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        qDebug()<<"Could not find stream information\n";
        goto end;
    }
    qDebug("before open open_codec_context");
    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        video_stream = fmt_ctx->streams[video_stream_idx];
        /* allocate image where the decoded image will be put */
        width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        if(hw_pix_fmt != AV_PIX_FMT_NONE)
            pix_fmt = AV_PIX_FMT_NV12;
        else
            pix_fmt = video_dec_ctx->pix_fmt;
        emit frameInfoUpdateSig(width,height,AV_PIX_FMT_YUV420P);
        m_sws_ctx = sws_getContext(width, height,
                                   pix_fmt,
                                   width, height,
                                   AV_PIX_FMT_YUV420P,
                                   SWS_BILINEAR, NULL, NULL, NULL);

        if (!m_sws_ctx) {
            fprintf(stderr, "Could not initialize the conversion context\n");
            goto end;
        }
        // 使用av_image_alloc分配空间
        ret = av_image_alloc(yuv420p_data, yuv420p_linesize, width, height, AV_PIX_FMT_YUV420P, 1);
        if(ret <= 0)
        {
            qDebug()<<"av_image_alloc error";
            //goto end;
        }
        qDebug()<<"before new YUVData";
        yuv = new YUVData(width,height);
        qDebug()<<"after new YUVData";
        if (ret < 0) {
            qDebug()<<"Could not allocate raw video buffer";
            goto end;
        }
    }
    else{
        isError = true;
        goto end;
    }
    if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        // qDebug()<<"nb_channels"<<audio_dec_ctx->ch_layout.nb_channels<<"sample_fmt"<<audio_dec_ctx->sample_fmt<<"sample_rate"<<audio_dec_ctx->sample_rate<<"audio_dec_ctx"<<audio_dec_ctx->codec_id;
        audio_stream = fmt_ctx->streams[audio_stream_idx];
        m_outSampleFormat = AV_SAMPLE_FMT_S16;
        m_outSampleRate = 48000;
        swr_alloc_set_opts2(&m_swr_ctx,
                            &audio_dec_ctx->ch_layout,
                            m_outSampleFormat,
                            m_outSampleRate,
                            &audio_dec_ctx->ch_layout,
                            audio_dec_ctx->sample_fmt,
                            audio_dec_ctx->sample_rate,
                            0,NULL);
        swr_init(m_swr_ctx);
        m_audioOutBuffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE);
        m_audioMediumBuffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE);
        qDebug()<<"after av_malloc";
    }
    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, m_url.toUtf8().data(), 0);
    if (!audio_stream && !video_stream) {
        qDebug()<<"Could not find audio or video stream in the input, aborting";
        goto end;
    }

    frame = av_frame_alloc();
    sw_frame = av_frame_alloc();
    if (!frame) {
        qDebug()<<"Could not allocate frame";
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug()<<"Could not allocate packet";
        goto end;
    }

    /* read frames from the file */
    qDebug()<<"before m_mutex.lock();";

    if(m_url.isEmpty())
    {
        avformat_flush(fmt_ctx);
    }
    qDebug()<<"before av_read_frame";
    while ((ret = av_read_frame(fmt_ctx, pkt)) >= 0 ) {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        // qDebug()<<"pts"<<pkt->pts<<"dts"<<pkt->dts;
        if (pkt->stream_index == video_stream_idx)
            ret = decode_packet(video_dec_ctx, pkt);
        else if (pkt->stream_index == audio_stream_idx)
            ret = decode_packet(audio_dec_ctx, pkt);
        av_packet_unref(pkt);
        if (ret < 0 || isOver)
        {
            qDebug()<<"exit";
            break;
        }
        QThread::msleep(1);
    }
    qDebug()<<"after over";

end:
    isOver = true;
    if(isError)
    {
        emit videoErrorSig();
        qDebug()<<"emit videoErrorSig();";
    }
    else if(!isOver)
    {
        emit frameInfoChangedSig();
        qDebug()<<"emit frameInfoChangedSig();";
    }
    m_mutex.unlock();
    return;
}
void Decoder::setUrl(QString url)
{
    m_url = url;
}
void Decoder::setIsOver()
{
    isOver = true;
}
void Decoder::stopDecode()
{
    qDebug()<<"before m_mutex.lock();";
    m_mutex.lock();
    qDebug()<<"after m_mutex.lock();";
    if(m_avioCtx)
    {
        av_free(m_avioCtx->buffer);
        avio_context_free(&m_avioCtx);
    }
    if (video_dec_ctx)
    {
        m_dataBufferMaxSize = 0;
        qDebug()<<"before decode_packet NULL";
        decode_packet(video_dec_ctx, NULL);
        qDebug()<<"before avformat_close_input";
        avformat_close_input(&fmt_ctx);
        qDebug()<<"before avcodec_free_context";
        avcodec_free_context(&video_dec_ctx);
        qDebug()<<"before delete yuv";
        delete yuv;

    }
    if (audio_dec_ctx)
    {
        decode_packet(audio_dec_ctx, NULL);
        qDebug()<<"before avcodec_free_context";
        avcodec_free_context(&audio_dec_ctx);
        qDebug()<<"before swr_free(&m_swr_ctx)";
        if(m_swr_ctx)
        {
            swr_close(m_swr_ctx);//在free前需要close，否则当视频开启时关闭程序会崩溃
            swr_free(&m_swr_ctx);
        }
        qDebug()<<"before av_free(m_audioOutBuffer)";
        if(m_audioOutBuffer)
            av_free(m_audioOutBuffer);
        qDebug()<<"before av_free(m_audioMediumBuffer)";
        if(m_audioMediumBuffer)
            av_free(m_audioMediumBuffer);
    }
    qDebug()<<"before av_packet_free";
    if(pkt)
        av_packet_free(&pkt);
    qDebug()<<"before av_frame_free";
    if(frame)
        av_frame_free(&frame);
    sw_frame = NULL;
    qDebug()<<"before sws_freeContext";
    if(m_sws_ctx)
    {
        sws_freeContext(m_sws_ctx);
        // 释放YUV420P数据
        av_freep(&yuv420p_data[0]);
    }


out:
    m_mutex.unlock();
    qDebug()<<"stop over";
}
YUVData* Decoder::getFrame()
{
    if(sw_frame && yuv && yuv->Y)
        return yuv;
    else
        return NULL;
}
