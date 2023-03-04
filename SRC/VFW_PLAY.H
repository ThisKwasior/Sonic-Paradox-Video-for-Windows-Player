#pragma once

/*
    https://www.binaryworld.net/Main/CodeDetail.aspx?CodeId=3669
    https://web.archive.org/web/20100612090837/https://www.binaryworld.net/Main/CodeDetail.aspx?CodeId=3613
    https://web.archive.org/web/20101029075030/https://www.binaryworld.net/Main/CodeDetail.aspx?CodeId=3669
    
    https://learn.microsoft.com/en-us/windows/win32/multimedia/avifile-reference
    https://learn.microsoft.com/en-us/windows/win32/multimedia/avifile-functions
*/

#include <windows.h>
#include <vfw.h>

#include <stdint.h>
#include <math.h>

#include <SDL/SDL.h>

#define VFW_STATUS_PAUSED   0
#define VFW_STATUS_PLAYING  1
#define VFW_STATUS_STOPPED  2

typedef struct VFW_CONTEXT
{
    PAVIFILE        avi;
    AVIFILEINFO     info;
    LONG            info_size;
    
    PAVISTREAM      video;
    AVISTREAMINFOA  video_info;
    LONG            video_info_size;
    LONG            video_len;
    LONG            video_start_sample;
    int64_t framerate;
    int64_t frame_pos;
    
    PAVISTREAM      audio;
    AVISTREAMINFOA  audio_info;
    LONG            audio_info_size;
    LONG            audio_len;
    LONG            audio_start_sample;
    PCMWAVEFORMAT   audio_format;
    int64_t audio_pos;
    int64_t audio_pos_samples;
    float audio_pos_sec;
    float audio_volume;
    
    float av_sync;
    float av_sync_buf;
    
    SDL_Surface*    frame;
    
    uint8_t status; /* 0 - Paused | 1 - Playing | 2 - Stopped*/
    
    PGETFRAME decompressor;
    BITMAPINFOHEADER* unc_frame;
} vfw_ctx;

void vfw_init();

vfw_ctx* vfw_open_file(const char* file);

void vfw_release(vfw_ctx* ctx);

int64_t vfw_calc_new_audio_to_video_pos(vfw_ctx* ctx);

void vfw_play(vfw_ctx* ctx);
void vfw_pause(vfw_ctx* ctx);
void vfw_stop(vfw_ctx* ctx);

void vfw_close();

void vfw_get_next_frame(vfw_ctx* ctx);
void vfw_get_frame(vfw_ctx* ctx, uint32_t frame);

float vfw_get_pos_float(vfw_ctx* ctx);

void vfw_av_sync(vfw_ctx* ctx, const float delta);

void vfw_seek_absolute(vfw_ctx* ctx, float pos_abs);
void vfw_seek_second_relative(vfw_ctx* ctx, float pos_rel);

void vfw_audio_callback(void* udata, uint8_t* stream, int len);

uint8_t* vfw_bitmap_to_rgb(BITMAPINFOHEADER* bmp, uint32_t* rgb_size);