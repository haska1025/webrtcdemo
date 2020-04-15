#include <stdio.h>
#include <algorithm>

#include "sdlwrapper.h"

int SDLWrapper::init(struct init_param *param)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)){
        printf("Init SDL2 failed!\n");
        return -1;
    }

    window_ = SDL_CreateWindow("Simple palyer by ffmpeg",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            param->width,
            param->height,
            SDL_WINDOW_OPENGL);

    if (!window_){
        printf("SDL Create Window failed.err(%s)\n", SDL_GetError());
        return -1;
    }

    render_ = SDL_CreateRenderer(window_,
            0,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!render_){
        printf("SDL Create render failed.err(%s)\n", SDL_GetError());
        return -1;
    }

    texture_ = SDL_CreateTexture(render_,
            SDL_PIXELFORMAT_IYUV,
            SDL_TEXTUREACCESS_STREAMING,
            param->width,
            param->height);

    if (!texture_){
        printf("SDL Create texture failed.err(%s)\n", SDL_GetError());
        return -1;
    }

    rect_.x = 0;
    rect_.y = 0;
    rect_.w = param->width;
    rect_.h = param->height;

    video_userdata_ = param->video_userdata;
    video_frame_cb = param->video_cb;

    audio_userdata_ = param->audio_userdata;
    audio_frame_cb = param->audio_cb;

    if (!audio_frame_cb){
        printf("You must set audio callback!\n");
        return -1;
    }

    // init audio
    SDL_AudioSpec wanted_spec, spec;

    wanted_spec.freq = param->freq; 
    wanted_spec.channels = param->channels;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.silence = 0;
    wanted_spec.samples = std::max(512, 2 *( wanted_spec.freq / 30));
    wanted_spec.callback = sdl_easy_audio_callback;
    wanted_spec.userdata = this;

    audio_devid_ = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
    if (audio_devid_ == 0){
        printf("Open audio device failed! rc(%d)\n", audio_devid_);
        return -1;
    }

    freq_ = spec.freq;
    channels_ = spec.channels;

    SDL_PauseAudioDevice(audio_devid_, 0);

    printf("Init sdlwrapper success!freq(%d) channels(%d)\n", freq_, channels_);

    return 0;
}

void SDLWrapper::sdl_easy_audio_callback(void *userdata, uint8_t *stream, int len)
{
    SDLWrapper *myself = (SDLWrapper*)userdata;

    myself->audio_frame_cb(myself->audio_userdata_, stream, len); 
}

int SDLWrapper::loop()
{
    SDL_Event event;

    printf("Enter loop......\n");

    if (!video_frame_cb){
        printf("You must set the sdlwrapper frame callback!\n");
        return -1;
    }

    while (1) {
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
            break;

        struct sdlwrapper_frame frame;

        int rc = video_frame_cb(video_userdata_, &frame); 
        if (rc != 0){
            printf("Get sdlwrapper frame failed! rc(%d)\n", rc);
            break;
        }

        SDL_UpdateYUVTexture(texture_,
                &rect_,
                frame.y_plane,
                frame.y_pitch,
                frame.u_plane,
                frame.u_pitch,
                frame.v_plane,
                frame.v_pitch);

        SDL_RenderClear(render_);

        SDL_RenderCopy(render_,
                texture_,
                NULL,
                &rect_);

        SDL_RenderPresent(render_);

        SDL_Delay(40);
    }
}

