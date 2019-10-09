#include <iostream>
//#include <libavutil/log.h>

#include "simpleplayer.h"
#include "sdlwrapper.h"

int main(int argc, char *argv[])
{
	SimplePlayer sp;

    if (argc != 2){
        std::cout << "Please Usage:simpleplayer xxx.mp4" << std::endl;
        return -1;
    }

    std::cout << "The media filename is : " << argv[1] << std::endl;

    av_log_set_level(AV_LOG_DEBUG);

	if (sp.init(argv[1]) != 0) {
		return -1;
	}

	std::cout << "simpleplayer!" << std::endl;

    SDLWrapper sdlwrapper;

    SDLWrapper::init_param param;
    param.width = sp.width();
    param.height = sp.height();
    param.video_userdata = &sp;
    param.video_cb = SimplePlayer::get_frame;

    param.freq = sp.freq();
    param.channels = sp.channels();
    param.audio_userdata = &sp;
    param.audio_cb = SimplePlayer::sdl_audio_callback;

    sdlwrapper.init(&param);

    sp.set_audio_params(sdlwrapper.freq(), sdlwrapper.channels());

    sdlwrapper.loop();

	std::cout << "simpleplayer exit!" << std::endl;
}
