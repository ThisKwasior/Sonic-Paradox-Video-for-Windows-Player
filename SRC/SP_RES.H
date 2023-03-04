#pragma once

#include <stdint.h>

#include <SDL/SDL.h>

#define SP_RES_UNAVAILABLE -1

/*
    Resource types

    RECT RES_INFO[RES_INFO_SIZE] X Y W H
    POS2 RES_INFO[RES_INFO_SIZE] X Y
    BMAP RES_INFO[RES_INFO_SIZE] FILE[RES_FILE_SIZE]
    BMPA RES_INFO[RES_INFO_SIZE] FILE[RES_FILE_SIZE] R G B
    AUDO RES_INFO[RES_INFO_SIZE] FILE[RES_FILE_SIZE] LOOP_START LOOP_END
*/

#define RES_INFO_SIZE 24
#define RES_FILE_SIZE 32

#define RES_TYPE_RECT 0x54434552
#define RES_TYPE_POS2 0x32534F50
#define RES_TYPE_BMAP 0x50414D42
#define RES_TYPE_BMPA 0x41504D42
#define RES_TYPE_AUDO 0x4F445541

typedef struct SP_AUDIO
{
    SDL_AudioSpec spec;
    uint8_t* buffer;
    uint32_t len;
    uint32_t loop_start;
    uint32_t loop_end;
    uint32_t pos;
} SP_audio;

typedef struct SP_VEC2
{
    int32_t x;
    int32_t y;
} SP_vec_i2;

typedef struct SP_RESOURCE
{
    uint32_t type;
    char info[RES_INFO_SIZE];
    
    union
    {
        SDL_Rect rect;
        SP_vec_i2 pos;
        SDL_Surface* img;
        SP_audio audio;
    } data;
} SP_resource_s;

typedef struct SP_RESOURCE_COLLECTION
{
    uint16_t count;
    SP_resource_s* res;
} SP_res_col_s;

/*
    Functions
*/

SP_res_col_s* SP_load_res_from_file(const char* file);

void SP_free_res_col(SP_res_col_s* col);

uint16_t SP_get_res_id(const char* info, SP_res_col_s* col);