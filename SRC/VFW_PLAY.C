#include "VFW_PLAY.H"

#include <stdlib.h>
#include <math.h>

#include <SDL/SDL_audio.h>

#include "SDL_MISC.H"
#include "MISC.H"

void vfw_init()
{
    AVIFileInit();
}

vfw_ctx* vfw_open_file(const char* file)
{
    vfw_ctx* ctx = (vfw_ctx*)calloc(sizeof(vfw_ctx), 1);
    
    AVIFileOpen(&ctx->avi, file, OF_READ, NULL);
    
    ctx->info_size = sizeof(AVIFILEINFO);
    ctx->video_info_size = sizeof(AVISTREAMINFOA);
    ctx->audio_info_size = sizeof(AVISTREAMINFOA);
    
    AVIFileInfo(ctx->avi, &ctx->info, ctx->info_size);
    
    HRESULT getstream_video = AVIFileGetStream(ctx->avi, &ctx->video, streamtypeVIDEO, 0);
    HRESULT getstream_audio = AVIFileGetStream(ctx->avi, &ctx->audio, streamtypeAUDIO, 0);
    
    if(getstream_video != 0) return NULL;
    if(getstream_audio != 0) return NULL;

    AVIStreamInfoA(ctx->video, &ctx->video_info, ctx->video_info_size);
    ctx->framerate = ctx->video_info.dwRate/ctx->video_info.dwScale;
    ctx->video_len = ctx->video_info.dwLength;

    AVIStreamInfoA(ctx->audio, &ctx->audio_info, ctx->audio_info_size);
    ctx->audio_len = ctx->audio_info.dwLength;
    ctx->audio_start_sample = AVIStreamStart(ctx->audio);
    
    LONG ocnwfsize = sizeof(PCMWAVEFORMAT);
    AVIStreamReadFormat(ctx->audio, 1, &ctx->audio_format, &ocnwfsize);
    
    ctx->decompressor = AVIStreamGetFrameOpen(ctx->video, NULL);
    
    ctx->audio_volume = 1.f;
    
    return ctx;
}

void vfw_release(vfw_ctx* ctx)
{
    AVIStreamRelease(ctx->video);
    AVIStreamRelease(ctx->audio);
    AVIFileRelease(ctx->avi);
    AVIStreamGetFrameClose(ctx->decompressor);
    SDL_FreeSurface(ctx->frame);
    free(ctx);
}

int64_t vfw_calc_new_audio_to_video_pos(vfw_ctx* ctx)
{
    return ctx->av_sync*ctx->audio_format.wf.nSamplesPerSec;
}

void vfw_play(vfw_ctx* ctx)
{
    if(ctx->status == VFW_STATUS_STOPPED)
    {
        ctx->status = VFW_STATUS_PLAYING;
        vfw_seek_absolute(ctx, 0.f);
    }
    
    if(ctx->status == VFW_STATUS_PAUSED)
    {
        ctx->status = VFW_STATUS_PLAYING;
        ctx->audio_pos = vfw_calc_new_audio_to_video_pos(ctx);
    }
}

void vfw_pause(vfw_ctx* ctx)
{
    if(ctx->status == VFW_STATUS_PLAYING)
    {
        ctx->status = VFW_STATUS_PAUSED;
    }
}

void vfw_stop(vfw_ctx* ctx)
{
    ctx->status = VFW_STATUS_STOPPED;
    vfw_seek_absolute(ctx, 0.f);
}

void vfw_close()
{
    AVIFileExit();
}

void vfw_get_next_frame(vfw_ctx* ctx)
{
    vfw_get_frame(ctx, ctx->frame_pos);
    
    if(ctx->status == VFW_STATUS_PLAYING)
    {
        ctx->frame_pos += 1;
    }
}

void vfw_get_frame(vfw_ctx* ctx, uint32_t frame)
{
    if(frame >= ctx->video_len)
    {
        ctx->status = VFW_STATUS_STOPPED;
        return;
    }

    ctx->unc_frame = (BITMAPINFOHEADER*)AVIStreamGetFrame(ctx->decompressor, frame);
    
    if(ctx->unc_frame)
    {
        /*
            Adding BMP header to Packed DIB so it becomes a valid BMP (according to SDL)
        */
        BITMAPFILEHEADER header = {0};
        header.bfType = 0x4D42; // 'BM'
        
        const uint32_t size_og = ctx->unc_frame->biWidth * ctx->unc_frame->biHeight;
        uint32_t color_lut_entries = 0;
        uint32_t color_lut_size = 0;
        uint32_t bmp_size = 0;
        
        if(ctx->unc_frame->biBitCount <= 8)
        {
            color_lut_entries = pow(2.f, ctx->unc_frame->biBitCount*1.f);
            color_lut_size = color_lut_entries*sizeof(RGBQUAD);
            bmp_size = size_og + color_lut_size + ctx->unc_frame->biSize;
        }
        else
        {
            bmp_size = size_og*(ctx->unc_frame->biBitCount/8) + ctx->unc_frame->biSize;
        }
        
        header.bfOffBits = sizeof(BITMAPFILEHEADER) + ctx->unc_frame->biSize + color_lut_size;
        header.bfSize = sizeof(BITMAPFILEHEADER) + bmp_size;
        
        uint8_t* bmp = (uint8_t*)calloc(header.bfSize, 1);
        memcpy(bmp, &header, sizeof(BITMAPFILEHEADER));
        memcpy(&bmp[sizeof(BITMAPFILEHEADER)], ctx->unc_frame, bmp_size);
        
        /*
            Loading this Frankenstein of a BMP with SDL from memory
        */
        SDL_FreeSurface(ctx->frame);
        ctx->frame = SDL_LoadBMPFromMem(bmp, header.bfSize, SDL_TRUE);
        
        free(bmp);
    }
    else
    {
        ctx->status = VFW_STATUS_PAUSED;
    }
}

float vfw_get_pos_float(vfw_ctx* ctx)
{
    return (ctx->frame_pos)/(float)ctx->video_len;
}

void vfw_av_sync(vfw_ctx* ctx, const float delta)
{
    if(ctx->status == VFW_STATUS_PLAYING)
    {
        ctx->frame_pos = ctx->av_sync * ctx->framerate;
        ctx->av_sync += delta;
        
        if(ctx->av_sync_buf > 0.4f)
        {
            const float audio_progress = ctx->audio_pos/(float)ctx->audio_format.wf.nSamplesPerSec;
            ctx->av_sync = audio_progress;
            printf("AV DESYNC - %f\n", ctx->av_sync_buf);
        }
    }
}

void vfw_seek_absolute(vfw_ctx* ctx, float pos_abs)
{
    if(ctx->status == VFW_STATUS_STOPPED) ctx->status = VFW_STATUS_PAUSED;
    
    if(pos_abs < 0.0f) pos_abs = 0.f;
    if(pos_abs >= 1.f) pos_abs = 1.f;
    
    ctx->av_sync = (ctx->video_len * pos_abs)/(float)ctx->framerate;
    ctx->frame_pos = ctx->av_sync * ctx->framerate;
    ctx->audio_pos = vfw_calc_new_audio_to_video_pos(ctx);
}

void vfw_seek_second_relative(vfw_ctx* ctx, float pos_rel)
{
    if(ctx->status == VFW_STATUS_STOPPED) ctx->status = VFW_STATUS_PAUSED;
    
    ctx->av_sync += pos_rel;

    const float len_sec = (ctx->video_len/(float)ctx->framerate);

    if(ctx->av_sync < 0.f) ctx->av_sync = 0.f;
    if(ctx->av_sync >= len_sec) ctx->av_sync = len_sec;
    
    ctx->frame_pos = ctx->av_sync * ctx->framerate;
    ctx->audio_pos = vfw_calc_new_audio_to_video_pos(ctx);
}

void vfw_audio_callback(void* udata, uint8_t* stream, int len)
{
    vfw_ctx* ctx = (vfw_ctx*)udata;
    
    uint8_t* to_mix = (uint8_t*)malloc(len);
    
    if(ctx->status == VFW_STATUS_PLAYING)
    {
        // I want the current delay between audio and video
        const float audio_progress = ctx->audio_pos/(float)ctx->audio_format.wf.nSamplesPerSec;
        const float video_progress = (ctx->frame_pos/(float)ctx->framerate);
        ctx->av_sync_buf = fabs(audio_progress - video_progress);
        
        //printf("audiovideo: %f %f %f\n", ctx->av_sync_buf, audio_progress, video_progress);
        
        LONG samples = 0;
        //AVIStreamRead(ctx->audio, ctx->audio_pos, len, stream, len, NULL, &samples);
        AVIStreamRead(ctx->audio, ctx->audio_pos, len, to_mix, len, NULL, &samples);
        ctx->audio_pos += len/ctx->audio_format.wf.nChannels;
        
        //printf("callback: %u %u %u\n", len, samples, ctx->audio_pos);
        
        SDL_MixAudio(stream, to_mix, len, lnr_to_log_approx_nrm(ctx->audio_volume)*SDL_MIX_MAXVOLUME);
    }
    
    free(to_mix);
}
