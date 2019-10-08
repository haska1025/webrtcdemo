#ifndef _SDLWRAPPER_H_
#define _SDLWRAPPER_H_

extern "C"{
#include <SDL2/SDL.h>
}


class SDLWrapper
{
public:
    struct sdlwrapper_frame{
        uint8_t *y_plane;
        int y_pitch;
        uint8_t *u_plane;
        int u_pitch;
        uint8_t *v_plane;
        int v_pitch;
    };

    typedef int (*get_frame_callback)(void *, struct sdlwrapper_frame *frame);

    SDLWrapper(){}
    ~SDLWrapper(){}

    int init(int w, int h);
    int loop();

    int set_callback(void *userdata, get_frame_callback cb)
    {
        userdata_ = userdata;
        sdlwrapper_frame_callback = cb;
    }

private:
    SDL_Window  *window_ = {nullptr};
    SDL_Renderer *render_ = {nullptr};
    SDL_Texture *texture_ = {nullptr};
    SDL_Rect rect_;

    void *userdata_ = {nullptr};
    get_frame_callback sdlwrapper_frame_callback; 
};
#endif//_SDLWRAPPER_H_

