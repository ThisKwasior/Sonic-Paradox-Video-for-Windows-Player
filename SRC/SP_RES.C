#include "SP_RES.H"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Functions
*/

void SP_read_RECT(const char* data, uint16_t* data_pos, SP_resource_s* res)
{
    res->type = RES_TYPE_RECT;
    
    memcpy(res->info, &data[*data_pos], RES_INFO_SIZE);
    *data_pos += RES_INFO_SIZE;
    
    res->data.rect.x = *(int16_t*)&data[*data_pos]; *data_pos += 2;
    res->data.rect.y = *(int16_t*)&data[*data_pos]; *data_pos += 2;
    res->data.rect.w = *(uint16_t*)&data[*data_pos]; *data_pos += 2;
    res->data.rect.h = *(uint16_t*)&data[*data_pos]; *data_pos += 2;
    
    printf("RECT Info: %*s\n", RES_INFO_SIZE, res->info);
    printf("RECT: %d %d %u %u\n", res->data.rect.x, res->data.rect.y,
                                  res->data.rect.w, res->data.rect.h);
}

void SP_read_POS2(const char* data, uint16_t* data_pos, SP_resource_s* res)
{
    res->type = RES_TYPE_POS2;
    
    memcpy(res->info, &data[*data_pos], RES_INFO_SIZE);
    *data_pos += RES_INFO_SIZE;
    
    res->data.pos.x = *(int32_t*)&data[*data_pos]; *data_pos += 4;
    res->data.pos.y = *(int32_t*)&data[*data_pos]; *data_pos += 4;

    printf("POS2: %d %d\n", res->data.pos.x, res->data.pos.y);
}

void SP_read_BMAP(const char* data, uint16_t* data_pos, SP_resource_s* res)
{
    char filename[RES_FILE_SIZE+1] = {0};
    
    res->type = RES_TYPE_BMAP;
    
    memcpy(res->info, &data[*data_pos], RES_INFO_SIZE);
    *data_pos += RES_INFO_SIZE;
    memcpy(&filename[0], &data[*data_pos], RES_FILE_SIZE);
    *data_pos += RES_FILE_SIZE;
    
    printf("BMAP Info: %*s\n", RES_INFO_SIZE, res->info);
    printf("BMAP Filename: %*s\n", RES_FILE_SIZE, filename);
    
    SDL_Surface* img = SDL_LoadBMP(filename);
    res->data.img = SDL_DisplayFormat(img);
    SDL_FreeSurface(img);
}

void SP_read_BMPA(const char* data, uint16_t* data_pos, SP_resource_s* res)
{
    char filename[RES_FILE_SIZE+1] = {0};
    
    res->type = RES_TYPE_BMPA;
    
    memcpy(res->info, &data[*data_pos], RES_INFO_SIZE);
    *data_pos += RES_INFO_SIZE;
    
    memcpy(&filename[0], &data[*data_pos], RES_FILE_SIZE);
    *data_pos += RES_FILE_SIZE;
    
    const uint8_t r = data[*data_pos]; *data_pos+=1;
    const uint8_t g = data[*data_pos]; *data_pos+=1;
    const uint8_t b = data[*data_pos]; *data_pos+=1;
    
    printf("BMPA Info: %*s\n", RES_INFO_SIZE, res->info);
    printf("BMPA Filename: %*s\n", RES_FILE_SIZE, filename);
    printf("BMPA Color key: %u %u %u\n", r, g, b);
    
    SDL_Surface* img = SDL_LoadBMP(filename);
    SDL_SetColorKey(img, SDL_SRCCOLORKEY, SDL_MapRGB(img->format, r, g, b));
    res->data.img = SDL_DisplayFormatAlpha(img);
    SDL_FreeSurface(img);
}

void SP_read_AUDO(const char* data, uint16_t* data_pos, SP_resource_s* res)
{
    char filename[RES_FILE_SIZE+1] = {0};

    res->type = RES_TYPE_AUDO;
    
    memcpy(res->info, &data[*data_pos], RES_INFO_SIZE);
    *data_pos += RES_INFO_SIZE;
    
    memcpy(&filename[0], &data[*data_pos], RES_FILE_SIZE);
    *data_pos += RES_FILE_SIZE;
    
    printf("AUDO Info: %*s\n", RES_INFO_SIZE, res->info);
    printf("AUDO Filename: %*s\n", RES_FILE_SIZE, filename);
    
    res->data.audio.loop_start = *(int32_t*)&data[*data_pos]; *data_pos += 4;
    res->data.audio.loop_end = *(int32_t*)&data[*data_pos]; *data_pos += 4;
    
    SDL_LoadWAV(filename, &res->data.audio.spec,
                &res->data.audio.buffer, &res->data.audio.len);
                
    printf("AUDO Buffer: %p\n", res->data.audio.buffer);
    printf("AUDO Len: %u\n", res->data.audio.len);
    printf("AUDO Loop: %u %u\n", res->data.audio.loop_start, res->data.audio.loop_end);
}

SP_res_col_s* SP_load_res_from_file(const char* file)
{
    char* buf = NULL;
    uint16_t dat_size = 0;

    FILE* dat = fopen(file, "rb");
    fread(&dat_size, 2, 1, dat);
    buf = (char*)calloc(dat_size-2, 1);
    fread(buf, dat_size-2, 1, dat);
    fclose(dat);
    
    SP_res_col_s* col = (SP_res_col_s*)calloc(sizeof(SP_res_col_s), 1);
    uint16_t buf_pos = 0;
    
    col->count = *(uint16_t*)&buf[buf_pos];
    buf_pos += 2;
    printf("Resource count: %u\n", col->count);
    
    col->res = (SP_resource_s*)calloc(sizeof(SP_resource_s), col->count);
    
    for(uint16_t i = 0; i != col->count; ++i)
    {
        uint32_t type = 0;
        memcpy(&type, &buf[buf_pos], 4);
        buf_pos += 4;
        
        printf("ID: %u\n", i);
        
        switch(type)
        {
            case RES_TYPE_RECT:
                SP_read_RECT(buf, &buf_pos, &col->res[i]);
                break;
            case RES_TYPE_POS2:
                SP_read_POS2(buf, &buf_pos, &col->res[i]);
                break;
            case RES_TYPE_BMAP:
                SP_read_BMAP(buf, &buf_pos, &col->res[i]);
                break;
            case RES_TYPE_BMPA:
                SP_read_BMPA(buf, &buf_pos, &col->res[i]);
                break;
            case RES_TYPE_AUDO:
                SP_read_AUDO(buf, &buf_pos, &col->res[i]);
                break;
            default:
                // ¯\_(ツ)_/¯
                break;
        }
    }
    
    return col;
}

void SP_free_res_col(SP_res_col_s* col)
{
    /*
        Free SDL_Surfaces and audio buffers inside of resources
    */
    for(uint16_t i = 0; i != col->count; ++i)
    {
        switch(col->res[i].type)
        {
            case RES_TYPE_BMAP:
            case RES_TYPE_BMPA:
                SDL_FreeSurface(col->res[i].data.img);
                break;
                
            case RES_TYPE_AUDO:
                SDL_FreeWAV(col->res[i].data.audio.buffer);
                break;
        }
    }
    
    /*
        Free resources
    */
    free(col->res);
    free(col);
}

uint16_t SP_get_res_id(const char* info, SP_res_col_s* col)
{
    uint32_t info_len = strlen(info);

    if(info_len > RES_INFO_SIZE) 
        info_len = RES_INFO_SIZE;

    uint16_t it = 0;
    for(it = 0; it != col->count; ++it)
    {
        if(strncmp(col->res[it].info, info, info_len) == 0)
            break;
    }
    
    // Resource available
    if(it == col->count) it = SP_RES_UNAVAILABLE;
    
    return it;
}