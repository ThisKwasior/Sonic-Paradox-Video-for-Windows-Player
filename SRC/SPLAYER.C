/*
    SPLAYER - Sonic Paradox Video for Windows player.
    
    Written by Kwasior/@ThisKwasior
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <winuser.h>

#include <SDL/SDL.h>
#include <SDL/SDL_keyboard.h>

/*
    SPlayer internals
*/

#include "SP_RES.H"

/*
    Core of audio/video reading
*/
#include "VFW_PLAY.H"

/*
    ICON File in BMP format
*/
#include "SP_ICON.H"

/*
    Other includes
*/
#include "AABB.H"
#include "SDL_MISC.H"
#include "MISC.H"

/*
    Structures
*/
typedef struct SP_VIDEO_LOOP
{
    vfw_ctx* avi;
    SP_audio* mus;
} SP_video_loop_s;

/*
    Globals
*/
SDL_Surface* screen = NULL;
SDL_Surface* img_background = NULL;
SDL_Surface* img_controls = NULL;

int32_t mouse_x = 0;
int32_t mouse_y = 0;

/*
    Audio
*/
SDL_AudioSpec sp_audio_spec;

/*
    Application
*/

uint8_t SP_init();
uint8_t SP_init_sdl();
void SP_close();
void SP_poll_event(SDL_Event* event);
void SP_draw();
void SP_setup_avi_audio(SDL_AudioSpec* spec, PCMWAVEFORMAT* wf, SP_video_loop_s* vl);
void SP_audio_callback(void* udata, uint8_t* stream, int len);
float SP_lnr_to_log_approx_nrm(float value);

uint8_t is_running = 1;
uint8_t video_current = 0;
float music_volume = 0.65f;
float delta_time = 0.f;


/*
    Resource collections
*/
SP_res_col_s* res_col_bmp = NULL;
SP_res_col_s* res_col_audio = NULL;
SP_res_col_s* res_col_videos = NULL;
SP_res_col_s* res_col_ctrl = NULL;
SP_res_col_s* res_col_ui = NULL;

SP_video_loop_s vl = {0};

/*
    UI Elements
*/
SDL_Rect* ui_video_screen = NULL;
SDL_Rect* ui_video_pos = NULL;
uint16_t ui_video_pos_id = SP_RES_UNAVAILABLE;
uint16_t ui_play_id = SP_RES_UNAVAILABLE;
uint16_t ui_pause_id = SP_RES_UNAVAILABLE;
uint16_t ui_stop_id = SP_RES_UNAVAILABLE;
uint16_t ui_slider_music_id = SP_RES_UNAVAILABLE;
SDL_Rect* ui_slider_music = NULL;
uint16_t ui_slider_video_id = SP_RES_UNAVAILABLE;
SDL_Rect* ui_slider_video = NULL;
uint16_t ui_message_id = SP_RES_UNAVAILABLE;

uint16_t ctrl_video_on_id = SP_RES_UNAVAILABLE;
uint16_t ctrl_video_pos_id = SP_RES_UNAVAILABLE;
uint16_t ctrl_video_volume_id = SP_RES_UNAVAILABLE;
uint16_t ctrl_music_volume_id = SP_RES_UNAVAILABLE;


int main(int argc, char** argv)
{
    /*
        Init
    */
    if(!SP_init()) return 0;

    /* Loop */
    SDL_Event event;
    
    const float start_time = SDL_GetTicks()/1000.f;
    float prev_delta_time = 0.f;
    
    while(is_running)
    {
        prev_delta_time = SDL_GetTicks()/1000.f - start_time;
        
        SP_poll_event(&event);
        
        SP_draw();
        
        /* Delta time */
        delta_time = (SDL_GetTicks()/1000.f) - prev_delta_time - start_time;
    }
    
    /* Close everything */
    SP_close();
    
    return 0;
}

uint8_t SP_init()
{
    /*
        Init SDL
    */
    const uint8_t init_status = SP_init_sdl();
    if(init_status == 0)
        return 0;
    
    /*
        Load resource collections
    */
    res_col_bmp = SP_load_res_from_file("DAT/BMP.DAT");
    res_col_audio = SP_load_res_from_file("DAT/AUDIO.DAT");
    res_col_videos = SP_load_res_from_file("DAT/MOVIES.DAT");
    res_col_ctrl = SP_load_res_from_file("DAT/CTRL.DAT"); 
    res_col_ui = SP_load_res_from_file("DAT/UI.DAT");
    
    /*
        Graphics setup
    */
    img_background = res_col_bmp->res[SP_get_res_id("BACKGROUND", res_col_bmp)].data.img;
    img_controls = res_col_bmp->res[SP_get_res_id("CONTROLS", res_col_bmp)].data.img;
    
    /*
        Video setup
    */
    vfw_init();
    vl.avi = vfw_open_file(res_col_videos->res[0].info);
    
    /*
        Audio setup
    */
    vl.mus = &res_col_audio->res[SP_get_res_id("LOOP_MUSIC", res_col_audio)].data.audio;
    
    SP_setup_avi_audio(&sp_audio_spec, &vl.avi->audio_format, &vl);
    SDL_PauseAudio(0);
    
    /*
        UI Elements
    */
    ui_video_screen = &res_col_ui->res[SP_get_res_id("VIDEO_SCREEN", res_col_ui)].data.rect;
    ui_video_pos_id = SP_get_res_id("VIDEO_POS", res_col_ui);
    ui_video_pos = &res_col_ui->res[ui_video_pos_id].data.rect;
    ui_play_id = SP_get_res_id("PLAY_BUTTON", res_col_ui);
    ui_pause_id = SP_get_res_id("PAUSE_BUTTON", res_col_ui);
    ui_stop_id = SP_get_res_id("STOP_BUTTON", res_col_ui);
    ui_message_id = SP_get_res_id("MESSAGE", res_col_ui);
    
    ui_slider_video_id = SP_get_res_id("VOL_VIDEO", res_col_ui);
    ui_slider_video = &res_col_ui->res[ui_slider_video_id].data.rect;
    ui_slider_music_id = SP_get_res_id("VOL_MUSIC", res_col_ui);
    ui_slider_music = &res_col_ui->res[ui_slider_music_id].data.rect;
    
    ctrl_video_on_id = SP_get_res_id("VID_ON", res_col_ctrl);
    ctrl_video_pos_id = SP_get_res_id("TRK_VID", res_col_ctrl);
    ctrl_video_volume_id = SP_get_res_id("BAR_VOL_VID", res_col_ctrl);
    ctrl_music_volume_id = SP_get_res_id("BAR_VOL_MUS", res_col_ctrl);
    
    return 1;
}

uint8_t SP_init_sdl()
{
    /* Initialize the SDL library */
    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return 0;
    }
    
    /* Set window title */
    SDL_WM_SetCaption("SPLAYER", NULL);
    
    /* Set Icon */   
    SDL_Surface* icon = SDL_LoadBMPFromMem(SP_ICON_BMP, SP_ICON_BMP_len, SDL_FALSE);
    SDL_WM_SetIcon(icon, NULL);
    SDL_FreeSurface(icon);
    
    /* Create window surface */
    screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if(screen == NULL)
    {
        printf("VideoMode 640x480x16 fail: %s\n", SDL_GetError());
        return 0;
    }

    return 1;
}

void SP_close()
{
    /* Shutdown all subsystems */
    SDL_CloseAudio();
    SDL_Quit();
    
    /* Close AVI stuff */
    vfw_release(vl.avi);
    vfw_close();
    
    /*  Free collections */
    SP_free_res_col(res_col_bmp);
    SP_free_res_col(res_col_audio);
    SP_free_res_col(res_col_videos);
    SP_free_res_col(res_col_ctrl);
    SP_free_res_col(res_col_ui);
}

void SP_poll_event(SDL_Event* event)
{
    while(SDL_PollEvent(event))
    {
        uint8_t test = 0;
        
        switch (event->type) 
        {
            case SDL_QUIT: // Exiting the application
                printf("Requested exit\n");
                is_running = 0;
                break;
            case SDL_MOUSEMOTION:
                mouse_x = event->motion.x;
                mouse_y = event->motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                /* UI Element test */
                for(uint16_t i = 0; i != res_col_ui->count; ++i)
                {
                    SP_resource_s* res = &res_col_ui->res[i];
                    test = AABB_test_point(mouse_x, mouse_y, 
                                           res->data.rect.x, res->data.rect.y,
                                           res->data.rect.w, res->data.rect.h);
                    if(test) 
                    {
                        if(i == ui_play_id) 
                        {
                            vfw_play(vl.avi);
                            break;
                        }
                        
                        if(i == ui_pause_id)
                        {
                            vfw_pause(vl.avi);
                            break;
                        }
                        
                        if(i == ui_stop_id)
                        {
                            vfw_stop(vl.avi);
                            break;
                        }
                        
                        if(i == ui_video_pos_id)
                        {
                            const float new_pos = (mouse_x - res->data.rect.x)/(float)res->data.rect.w;
                            vfw_seek_absolute(vl.avi, new_pos);
                            break;
                        }
                        
                        if(i == ui_slider_video_id)
                        {
                            vl.avi->audio_volume = (mouse_x - res->data.rect.x)/(float)res->data.rect.w;
                            break;
                        }
                        
                        if(i == ui_slider_music_id)
                        {
                            music_volume = (mouse_x - res->data.rect.x)/(float)res->data.rect.w;
                            break;
                        }
                       
                        if(i == ui_message_id)
                        {
                            MessageBox(NULL, "-Video and Audio Encoding, Project Director-\nDr. Mack Foxx\n\n-Programming Wizard, Graphic Design-\nKwasior\n\n-Graphic Design-\nPiggybank\n\n-Graphic Design-\nTheWax\n\n\nSource Code available at:\ngithub.com/ThisKwasior/Sonic-Paradox-Video-for-Windows-Player\nand in the LEGAL\\SP-SRC.ZIP compressed file on the CD.",
                                       "**Sonic Paradox for Windows 95 Staff**", MB_OK);
                            break;
                        }
                        
                        continue;
                    }
                }
              
                /* Video test */
                for(uint16_t i = 0; i != res_col_videos->count; ++i)
                {
                    SP_resource_s* res = &res_col_videos->res[i]; 
                    test = AABB_test_point(mouse_x, mouse_y, 
                                           res->data.rect.x, res->data.rect.y,
                                           res->data.rect.w, res->data.rect.h);
                    if(test) 
                    {
                        /* Load new video */
                        float old_volume = vl.avi->audio_volume;
                        SDL_PauseAudio(1);
                        vfw_release(vl.avi);
                        vl.avi = vfw_open_file(res->info);
                        SP_setup_avi_audio(&sp_audio_spec, &vl.avi->audio_format, &vl);
                        SDL_PauseAudio(0);
                        vl.avi->audio_volume = old_volume;
                        vfw_play(vl.avi);
                        video_current = i;
                        printf("Selected video: %s\n", res->info);
                        break;
                    }
                }
                break;
        }
    }
}

void SP_draw()
{
        /* Drawing background and layout */
        SDL_BlitSurface(img_background, NULL, screen, NULL);
        
        /* Video playback */
        vfw_av_sync(vl.avi, delta_time);
        vfw_get_next_frame(vl.avi);
        if(vl.avi->frame)
        {
            SDL_BlitSurface(vl.avi->frame, NULL, screen, ui_video_screen);
        }
        
        /*
            Draw UI elements
        */
        
        /* Current video border */
        /* Do not display border for a robotnik sign */
        if(video_current != (res_col_videos->count-1)) 
        {
            SDL_BlitSurface(img_controls, &res_col_ctrl->res[ctrl_video_on_id].data.rect,
                                screen, &res_col_videos->res[video_current].data.rect);
        }
                        
        /* Video pos */
        SDL_Rect vp_rect = {(int16_t)(vfw_get_pos_float(vl.avi)*(ui_video_pos->w-4) + ui_video_pos->x - 2),
                            ui_video_pos->y, ui_video_pos->w, ui_video_pos->h};
        SDL_BlitSurface(img_controls, &res_col_ctrl->res[ctrl_video_pos_id].data.rect, screen, &vp_rect);
        
        /* Music volume */
        SDL_Rect vv_rect = {(int16_t)(vl.avi->audio_volume*(ui_slider_video->w-4) + ui_slider_video->x - 2),
                            ui_slider_video->y, ui_slider_video->w, ui_slider_video->h};
        SDL_BlitSurface(img_controls, &res_col_ctrl->res[ctrl_video_volume_id].data.rect, screen, &vv_rect);
        
        /* Music volume */
        SDL_Rect mv_rect = {(int16_t)(music_volume*(ui_slider_music->w-4) + ui_slider_music->x - 2),
                            ui_slider_music->y, ui_slider_music->w, ui_slider_music->h};
        SDL_BlitSurface(img_controls, &res_col_ctrl->res[ctrl_music_volume_id].data.rect, screen, &mv_rect);

        /*
            Changing currently displayed frame to new one
        */
        SDL_Flip(screen);
}

void SP_setup_avi_audio(SDL_AudioSpec* spec, PCMWAVEFORMAT* pwf, SP_video_loop_s* vl)
{
    // Who knows if it was open
    SDL_CloseAudio();
    
    vl->avi->audio_pos = 0;
    vl->avi->audio_pos_samples = 0;
    vl->avi->audio_pos_sec = 0;
    
    /* Set the audio format */
    spec->freq = pwf->wf.nSamplesPerSec;
    
    if(pwf->wBitsPerSample == 16) spec->format = AUDIO_S16;
    else if(pwf->wBitsPerSample == 8) spec->format = AUDIO_U8;
    
    spec->channels = pwf->wf.nChannels;
    spec->samples = 1024;
    spec->callback = SP_audio_callback;
    spec->userdata = vl;
    
    vl->avi->audio_pos = 0;

    /* Open the audio device, forcing the desired format */
    SDL_OpenAudio(spec, NULL);
}

void SP_audio_callback(void* udata, uint8_t* stream, int len)
{
    SP_video_loop_s* vl = (SP_video_loop_s*)udata;
    vfw_ctx* ctx = vl->avi;
    SP_audio* mus = vl->mus;
    
    if(ctx->status == VFW_STATUS_PLAYING)
    {
        vfw_audio_callback((void*)ctx, stream, len);
    }
    else
    {
        const int32_t len_check = mus->loop_end - mus->pos;
        
        const float volume = lnr_to_log_approx_nrm(music_volume);
        
        if(len_check <= len)
        {
            SDL_MixAudio(stream, &mus->buffer[mus->pos], len_check, volume*SDL_MIX_MAXVOLUME);
            mus->pos = mus->loop_start;
            SDL_MixAudio(&stream[len_check], &mus->buffer[mus->pos], len-len_check, volume*SDL_MIX_MAXVOLUME);
            mus->pos += len-len_check;
            
        }
        else
        {
            SDL_MixAudio(stream, &mus->buffer[mus->pos], len, volume*SDL_MIX_MAXVOLUME);
            mus->pos += len/mus->spec.channels;
        }
    }
}

