#include "simpleplayer.h"
#include <sstream>

int SimplePlayer::init(const char *url)
{
    int rc = 0;
	//av_register_all();
	avformat_network_init();

	//format_ctx_ = avformat_alloc_context();

    rc = avformat_open_input(&format_ctx_, url, NULL, NULL); 
    if (rc != 0){
        printf("Open avformat failed! rc(%d)\n", rc);
        return rc;
    }

    rc = avformat_find_stream_info(format_ctx_, NULL);
    if (rc < 0){
        printf("Find stream info faield! rc(%d)\n", rc);
        return rc;
    }

    for (int i=0; i < format_ctx_->nb_streams; i++){
        if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            first_video = i;
            break;
        }
    }

    if (first_video == -1){
        printf("Can't find video stream!\n");
        return -1;
    }

    AVCodecParameters *codec_para = format_ctx_->streams[first_video]->codecpar;
    AVCodec *codec = avcodec_find_decoder(codec_para->codec_id); 
    if (!codec){
        printf("codec is NULL");
        return -1;
    }

    codec_ctx_ = avcodec_alloc_context3(codec);
    if(!codec_ctx_){
        printf("Alloc avcodec failed!\n");
        return -1;
    }

    rc = avcodec_parameters_to_context(codec_ctx_, codec_para);
    if (rc < 0){
        printf("Copy para to avcodec context failed!rc(%d)\n", rc);
        return -1;
    }

    rc = avcodec_open2(codec_ctx_, codec, NULL);
    if (rc < 0){
        printf("Init codec ctx by codec failed! rc(%d)\n", rc);
        return -1;
    }

    frame_yuv = av_frame_alloc();
    frame_yuv->width = codec_ctx_->width;
    frame_yuv->height = codec_ctx_->height;
    frame_yuv->format = AV_PIX_FMT_YUV420P;
    width_ = codec_ctx_->width;
    height_ = codec_ctx_->height;
    /*
    int buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, codec_ctx_->width, codec_ctx_->height, 1);

    uint8_t *buffer = (uint8_t*)av_malloc(buf_size);

    av_image_fill_arrays(frame_yuv->data, frame_yuv->linesize, buffer, AV_PIX_FMT_YUV420P, codec_ctx_->width, codec_ctx_->height, 1);
    */
    av_image_alloc(frame_yuv->data, frame_yuv->linesize, codec_ctx_->width, codec_ctx_->height, AV_PIX_FMT_YUV420P, 1);

    sws_ctx_ = sws_getContext(codec_ctx_->width,
            codec_ctx_->height,
            codec_ctx_->pix_fmt,
            codec_ctx_->width,
            codec_ctx_->height,
            AV_PIX_FMT_YUV420P,
            SWS_BICUBIC,
            NULL,
            NULL,
            NULL);

	return 0;
}

int SimplePlayer::get_frame(void *userdata, struct SDLWrapper::sdlwrapper_frame *frame)
{
    int rc = 0;
    AVPacket av_packet;
    AVFrame *frame_recv = av_frame_alloc();
    int file_index = 0;

    SimplePlayer *myself = (SimplePlayer*)userdata;
    rc = av_read_frame(myself->format_ctx_, &av_packet);
    if (rc != 0){
        printf("read frame failed. rc(%d)\n", rc);
        return rc;
    }

    if (av_packet.stream_index == myself->first_video){
        rc = avcodec_send_packet(myself->codec_ctx_, &av_packet);
        if (rc != 0){
            printf("avcodec decode failed!rc(%d)\n", rc);
            return rc;
        }

        rc = avcodec_receive_frame(myself->codec_ctx_, frame_recv); 
        if (rc != 0){
            printf("receive frame failed! rc(%d)\n", rc);
            return rc;
        }

        if (frame_recv->format != AV_PIX_FMT_YUV420P){
            sws_scale(myself->sws_ctx_,
                    frame_recv->data,
                    frame_recv->linesize,
                    0,
                    myself->codec_ctx_->height,
                    myself->frame_yuv->data,
                    myself->frame_yuv->linesize);
            frame->y_plane = myself->frame_yuv->data[0];
            frame->y_pitch = myself->frame_yuv->linesize[0];
            frame->u_plane = myself->frame_yuv->data[1];
            frame->u_pitch = myself->frame_yuv->linesize[1];
            frame->v_plane = myself->frame_yuv->data[2];
            frame->v_pitch = myself->frame_yuv->linesize[2];

        }else{
            frame->y_plane = frame_recv->data[0];
            frame->y_pitch = frame_recv->linesize[0];
            frame->u_plane = frame_recv->data[1];
            frame->u_pitch = frame_recv->linesize[1];
            frame->v_plane = frame_recv->data[2];
            frame->v_pitch = frame_recv->linesize[2];
        }
                /*
           std::ostringstream filename;
           filename << "yuv_to_jpeg_" << file_index++;
           filename << ".jpeg";

           yuv_to_jpeg(frame_yuv, filename.str().c_str(), codec_ctx_->width, codec_ctx_->height); 
           */

    }
    av_packet_unref(&av_packet);
    av_frame_free(&frame_recv);
    frame_recv = nullptr;
}

int SimplePlayer::yuv_to_jpeg(AVFrame *yuv, const char *ofile, int w, int h)
{
    int rc = -1;
    AVFormatContext *o_fmt_ctx = NULL;
    AVCodecContext *encoder_ctx = NULL;
    AVPacket packet;

    rc = avformat_alloc_output_context2(&o_fmt_ctx, NULL, NULL, ofile);
    if (rc < 0){
        printf("alloc output context failed! rc(%d)\n", rc);
        return rc;
    }

    AVCodec *codec = avcodec_find_encoder(o_fmt_ctx->oformat->video_codec);
    if (!codec){
        printf("Can't find codec!\n");
        return -1;
    }

    encoder_ctx = avcodec_alloc_context3(codec);
    encoder_ctx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    encoder_ctx->width = w;
    encoder_ctx->height = h;
    encoder_ctx->time_base.num = 1;
    encoder_ctx->time_base.den = 25;

    rc = avcodec_open2(encoder_ctx, codec, NULL);
    if (rc < 0){
        printf("Can't open codec!\n");
        return -1;
    }

    rc = avcodec_send_frame(encoder_ctx, yuv);
    if (rc < 0){
        printf("Send frame to encoder failed! rc(%d)\n", rc);
        return rc;
    }

    av_init_packet(&packet);
    rc = avcodec_receive_packet(encoder_ctx, &packet);
    if (rc < 0){
        printf("Recv packet from encoder failed! rc(%d)\n", rc);
        return rc;
    }

    avformat_new_stream(o_fmt_ctx, codec);

    avformat_write_header(o_fmt_ctx, NULL);
    av_write_frame(o_fmt_ctx, &packet);
    av_write_trailer(o_fmt_ctx);

    avcodec_free_context(&encoder_ctx);
    avformat_free_context(o_fmt_ctx);
}


