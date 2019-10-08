#include <iostream>
#include <libavutil/log.h>

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

    //av_log_set_level(AV_LOG_DEBUG);

	if (sp.init(argv[1]) != 0) {
		return -1;
	}

	std::cout << "simpleplayer!\n";

    SDLWrapper sdlwrapper;
    sdlwrapper.init(sp.width(), sp.height());
    sdlwrapper.set_callback(&sp, SimplePlayer::get_frame);

    sdlwrapper.loop();

	std::cout << "simpleplayer exit!\n";
}
