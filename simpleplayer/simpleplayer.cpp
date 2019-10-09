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
        if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO 
                && video_stream_id_ == -1){
            video_stream_id_ = i;
        }else if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO
                && audio_stream_id_ == -1){
            audio_stream_id_ = i;
        }

        if (video_stream_id_ != -1 && audio_stream_id_ != -1){
            break;
        }
    }

    if (video_stream_id_ == -1 && audio_stream_id_ == -1){
        printf("Can't find video and audio stream!\n");
        return -1;
    }

    video_codec_ctx_ = create_codec_context(video_stream_id_);
    if (!video_codec_ctx_)
        return -1;

    audio_codec_ctx_ = create_codec_context(audio_stream_id_);
    if (!audio_codec_ctx_)
        return -1;

    return init_video();
}

int SimplePlayer::get_frame(void *userdata, struct SDLWrapper::sdlwrapper_frame *frame)
{
    int rc = 0;
    AVPacket av_packet;
    AVFrame *frame_recv = av_frame_alloc();
    int file_index = 0;

    SimplePlayer *myself = (SimplePlayer*)userdata;

read_again:
    rc = av_read_frame(myself->format_ctx_, &av_packet);
    if (rc != 0){
        printf("read frame failed. rc(%d)\n", rc);
        return rc;
    }

    if (av_packet.stream_index == myself->video_stream_id_){
        rc = avcodec_send_packet(myself->video_codec_ctx_, &av_packet);
        if (rc != 0){
            printf("avcodec decode failed!rc(%d)\n", rc);
            return rc;
        }

        rc = avcodec_receive_frame(myself->video_codec_ctx_, frame_recv); 
        if (rc != 0){
            printf("receive frame failed! rc(%d)\n", rc);
            return rc;
        }

        if (frame_recv->format != AV_PIX_FMT_YUV420P){
            sws_scale(myself->sws_ctx_,
                    frame_recv->data,
                    frame_recv->linesize,
                    0,
                    myself->video_codec_ctx_->height,
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

           yuv_to_jpeg(frame_yuv, filename.str().c_str(), video_codec_ctx_->width, video_codec_ctx_->height); 
           */

    }else{
        goto read_again;
    }

    av_packet_unref(&av_packet);
    av_frame_free(&frame_recv);
    frame_recv = nullptr;
}

void SimplePlayer::sdl_audio_callback(void *userdata, uint8_t *stream, int len)
{
    printf("Get audio data len(%d)\n", len);

    int rc = 0;
    int total_copy_len = 0;
    int resampled_data_size = 0;
    uint8_t *data_buf = nullptr;

    AVPacket av_packet;
    AVFrame *frame_recv = av_frame_alloc();

    SimplePlayer *myself = (SimplePlayer*)userdata;

    if (len <= 0 || !stream){
        printf("Invalid param!\n");
        return ;
    }

    if (myself->left_resampled_data_size_ > 0){
        if (myself->left_resampled_data_size_ < len){
            memcpy(stream, myself->audio_buf_ + myself->consume_resampled_data_size_, myself->left_resampled_data_size_);
            myself->left_resampled_data_size_ = 0;
            total_copy_len += myself->left_resampled_data_size_;
        }else{
            memcpy(stream, myself->audio_buf_ + myself->consume_resampled_data_size_, len);
            myself->left_resampled_data_size_ -= len;
            total_copy_len += len;

            return;
        }
    }

    while( total_copy_len < len){
        rc = av_read_frame(myself->format_ctx_, &av_packet);
        if (rc != 0){
            printf("read audio packet failed. rc(%d)\n", rc);
            break;
        }

        if (av_packet.stream_index != myself->audio_stream_id_){
            continue;
        }

        rc = avcodec_send_packet(myself->audio_codec_ctx_, &av_packet);
        if (rc != 0){
            printf("avcodec decode audio failed!rc(%d)\n", rc);
            break;
        }

        rc = avcodec_receive_frame(myself->audio_codec_ctx_, frame_recv); 
        if (rc != 0){
            printf("receive audio frame failed! rc(%d)\n", rc);
            break;
        }

        if (myself->audio_params_.freq != frame_recv->sample_rate
                || myself->audio_params_.channel_layout != frame_recv->channel_layout
                || myself->audio_params_.fmt != frame_recv->format){
            if (!myself->swr_ctx_){
                myself->swr_ctx_ = swr_alloc_set_opts(NULL,
                        myself->audio_params_.channel_layout,
                        myself->audio_params_.fmt,
                        myself->audio_params_.freq,
                        frame_recv->channel_layout,
                        (enum AVSampleFormat)frame_recv->format,
                        frame_recv->sample_rate,
                        0,
                        NULL);

                if (!myself->sws_ctx_){
                    printf("Create swr failed!\n");
                    rc = -1;
                    break;
                }

                rc = swr_init(myself->swr_ctx_);
                if (rc < 0){
                    printf("Init swr failed!rc(%d)\n", rc);
                    swr_free(&myself->swr_ctx_);
                    myself->swr_ctx_ = nullptr;
                    break;
                }
            }

            const uint8_t **in = (const uint8_t **)frame_recv->extended_data;
            int out_count = (int64_t)frame_recv->nb_samples * myself->audio_params_.freq / frame_recv->sample_rate + 256;
            int out_size  = av_samples_get_buffer_size(NULL, myself->audio_params_.channels, out_count, myself->audio_params_.fmt, 0);
            if (out_size < 0) {
                printf("av_samples_get_buffer_size() failed\n");
                rc = -1;
                break;
            }
            /*
               if (wanted_nb_samples != af->frame->nb_samples) {
               if (swr_set_compensation(is->swr_ctx, (wanted_nb_samples - af->frame->nb_samples) * is->audio_tgt.freq / af->frame->sample_rate,
               wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate) < 0) {
               av_log(NULL, AV_LOG_ERROR, "swr_set_compensation() failed\n");
               return -1;
               }
               }
               */
            av_fast_malloc(&myself->audio_buf_, &myself->audio_buf_len_, out_size);
            if (!myself->audio_buf_){
                printf("fast alloc failed!\n");
                rc = -1;
                break;
            }

            int len2 = swr_convert(myself->swr_ctx_, &myself->audio_buf_, out_count, in, frame_recv->nb_samples);
            if (len2 < 0) {
                printf(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
                rc = -1;
                break;
            }
            if (len2 == out_count) {
                printf("audio buffer is probably too small\n");
                if (swr_init(myself->swr_ctx_) < 0)
                    swr_free(&myself->swr_ctx_);
            }

            resampled_data_size = len2 * myself->audio_params_.channels * av_get_bytes_per_sample(myself->audio_params_.fmt);
            data_buf = myself->audio_buf_;
        }else{
            resampled_data_size = av_samples_get_buffer_size(
                    NULL,
                    frame_recv->channels,
                    frame_recv->nb_samples,
                    (enum AVSampleFormat)frame_recv->format,
                    1);

            data_buf = frame_recv->data[0];
        }

        int left_len = len - total_copy_len;
        int copy_len = (resampled_data_size <= left_len)? resampled_data_size : left_len;

        memcpy(stream + total_copy_len, myself->audio_buf_, copy_len); 

        total_copy_len += copy_len;

        myself->consume_resampled_data_size_ = copy_len;
        myself->left_resampled_data_size_ = resampled_data_size - copy_len;
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

AVCodecContext *SimplePlayer::create_codec_context(int stream_id)
{
    int rc = 0;
    AVCodecParameters *codec_para = format_ctx_->streams[stream_id]->codecpar;
    AVCodec *codec = avcodec_find_decoder(codec_para->codec_id); 
    if (!codec){
        printf("codec is NULL");
        return nullptr;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if(!codec_ctx){
        printf("Alloc avcodec failed!\n");
        return nullptr;
    }

    rc = avcodec_parameters_to_context(codec_ctx, codec_para);
    if (rc < 0){
        printf("Copy para to avcodec context failed!rc(%d)\n", rc);
        return nullptr;
    }

    rc = avcodec_open2(codec_ctx, codec, NULL);
    if (rc < 0){
        printf("Init codec ctx by codec failed! rc(%d)\n", rc);
        return nullptr;
    }

    return codec_ctx;
}

int SimplePlayer::init_video()
{
    frame_yuv = av_frame_alloc();
    frame_yuv->width = video_codec_ctx_->width;
    frame_yuv->height = video_codec_ctx_->height;
    frame_yuv->format = AV_PIX_FMT_YUV420P;
    width_ = video_codec_ctx_->width;
    height_ = video_codec_ctx_->height;
    /*
    int buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, video_codec_ctx_->width, video_codec_ctx_->height, 1);

    uint8_t *buffer = (uint8_t*)av_malloc(buf_size);

    av_image_fill_arrays(frame_yuv->data, frame_yuv->linesize, buffer, AV_PIX_FMT_YUV420P, video_codec_ctx_->width, video_codec_ctx_->height, 1);
    */
    av_image_alloc(frame_yuv->data, frame_yuv->linesize, video_codec_ctx_->width, video_codec_ctx_->height, AV_PIX_FMT_YUV420P, 1);

    sws_ctx_ = sws_getContext(video_codec_ctx_->width,
            video_codec_ctx_->height,
            video_codec_ctx_->pix_fmt,
            video_codec_ctx_->width,
            video_codec_ctx_->height,
            AV_PIX_FMT_YUV420P,
            SWS_BICUBIC,
            NULL,
            NULL,
            NULL);


    return 0;
}

void SimplePlayer::set_audio_params(int freq, int channels)
{
    audio_params_.fmt = AV_SAMPLE_FMT_S16;
    audio_params_.freq = freq;
    audio_params_.channel_layout = av_get_default_channel_layout(channels);
    audio_params_.channels =  channels;
    audio_params_.frame_size = av_samples_get_buffer_size(NULL, audio_params_.channels, 1, audio_params_.fmt, 1);
    audio_params_.bytes_per_sec = av_samples_get_buffer_size(NULL, audio_params_.channels, audio_params_.freq, audio_params_.fmt, 1);
    if (audio_params_.bytes_per_sec <= 0 || audio_params_.frame_size <= 0) {
        printf("av_samples_get_buffer_size failed\n");
    }
}

