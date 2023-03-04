#pragma once

#include <SDL/SDL.h>

inline SDL_Surface* SDL_LoadBMPFromMem(unsigned char* data, const unsigned int len, int freesrc)
{
    return SDL_LoadBMP_RW(SDL_RWFromMem(data, len), freesrc);
}