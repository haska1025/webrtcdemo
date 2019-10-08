#include <stdio.h>

#include "sdlwrapper.h"

int SDLWrapper::init(int w, int h)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)){
        printf("Init SDL2 failed!\n");
        return -1;
    }

    window_ = SDL_CreateWindow("Simple palyer by ffmpeg",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            w,
            h,
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
            w,
            h);

    if (!texture_){
        printf("SDL Create texture failed.err(%s)\n", SDL_GetError());
        return -1;
    }

    rect_.x = 0;
    rect_.y = 0;
    rect_.w = w;
    rect_.h = h;

    return 0;
}

int SDLWrapper::loop()
{
    SDL_Event event;

    if (!sdlwrapper_frame_callback){
        printf("You must set the sdlwrapper frame callback!\n");
        return -1;
    }

    while (1) {
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
            break;

        struct sdlwrapper_frame frame;

        int rc = sdlwrapper_frame_callback(userdata_, &frame); 
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

