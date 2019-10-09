#ifndef _SDLWRAPPER_H_
#define _SDLWRAPPER_H_

extern "C"{
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
}


class SDLWrapper
{
public:
    struct sdlwrapper_frame
    {
        uint8_t *y_plane;
        int y_pitch;
        uint8_t *u_plane;
        int u_pitch;
        uint8_t *v_plane;
        int v_pitch;
    };

    typedef int (*video_frame_callback)(void *, struct sdlwrapper_frame *frame);
    typedef void (*audio_frame_callback)(void *, uint8_t *stream, int len);

    struct init_param
    {
        // video
        int width;
        int height;
        void *video_userdata;
        video_frame_callback video_cb;

        // audio
        int freq;
        int channels; 
        void *audio_userdata;
        audio_frame_callback audio_cb;
    };

    SDLWrapper(){}
    ~SDLWrapper(){}

    int init(struct init_param *param);
    int loop();

    int freq(){return freq_;}
    int channels(){return channels_;}

    static void sdl_easy_audio_callback(void *userdata, uint8_t *stream, int len);
private:
    SDL_Window  *window_ = {nullptr};
    SDL_Renderer *render_ = {nullptr};
    SDL_Texture *texture_ = {nullptr};
    SDL_Rect rect_;

    void *video_userdata_ = {nullptr};
    video_frame_callback video_frame_cb = {nullptr}; 

    void *audio_userdata_ = {nullptr};
    audio_frame_callback audio_frame_cb = {nullptr}; 

    SDL_AudioDeviceID audio_devid_;
    int freq_;
    int channels_;
};
#endif//_SDLWRAPPER_H_

