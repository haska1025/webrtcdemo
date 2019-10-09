#ifndef SIMPLEPLAYER_H
#define SIMPLEPLAYER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

#include "sdlwrapper.h"

class SimplePlayer
{
public:
    typedef struct AudioParams {
        int freq;
        int channels;
        int64_t channel_layout;
        enum AVSampleFormat fmt;
        int frame_size;
        int bytes_per_sec;
    } AudioParams;

public:
	SimplePlayer(){}
	~SimplePlayer(){}

	int init(const char *url);
    int close();

    int yuv_to_jpeg(AVFrame *yuv, const char *ofile, int w, int h);

    int width(){return width_;}
    int height() {return height_;}

    int freq(){return audio_codec_ctx_->sample_rate;}
    int channels(){return audio_codec_ctx_->channels;}

    void set_audio_params(int freq, int channels);

    static int get_frame(void *userdata, struct SDLWrapper::sdlwrapper_frame *frame);
    static void sdl_audio_callback(void *userdata, uint8_t *stream, int len);
private:
    AVCodecContext *create_codec_context(int stream_id);
    int init_video();

private:
	AVFormatContext	*format_ctx_ = {nullptr};
	AVCodecContext	*video_codec_ctx_ = {nullptr};
	AVCodecContext	*audio_codec_ctx_ = {nullptr};

    SwsContext *sws_ctx_ = {nullptr};
    SwrContext *swr_ctx_ = {nullptr};


    AVFrame *frame_yuv = {nullptr};
    int video_stream_id_ = -1;
    int audio_stream_id_ = -1;
    int width_ = 0;
    int height_ = 0;

    AudioParams audio_params_;
    uint8_t *audio_buf_ = nullptr; 
    unsigned int audio_buf_len_ = 0; 
    int consume_resampled_data_size_ = 0;
    int left_resampled_data_size_ = 0;
};

#endif //SIMPLEPLAYER_H

