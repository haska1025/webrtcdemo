#ifndef SIMPLEPLAYER_H
#define SIMPLEPLAYER_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

#include "sdlwrapper.h"

class SimplePlayer
{
public:
	SimplePlayer(){}
	~SimplePlayer(){}

	int init(const char *url);
    int close();

    int yuv_to_jpeg(AVFrame *yuv, const char *ofile, int w, int h);

    int width(){return width_;}
    int height() {return height_;}

    static int get_frame(void *userdata, struct SDLWrapper::sdlwrapper_frame *frame);
private:
	AVFormatContext	*format_ctx_ = {nullptr};
	AVCodecContext	*codec_ctx_ = {nullptr};
    SwsContext *sws_ctx_ = {nullptr};
    AVFrame *frame_yuv = {nullptr};
    int first_video = -1;
    int width_ = 0;
    int height_ = 0;
};

#endif //SIMPLEPLAYER_H

