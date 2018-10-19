#pragma once

#ifdef SCALING
#include "SDL_rotozoom.h"
SDL_Surface *real_screen;
SDL_Rect screen_position;
struct scaling
{
	unsigned short w_display;
	unsigned short h_display;
	unsigned char w_scale;
	unsigned char h_scale;
	unsigned short w_scale_size;
	unsigned short h_scale_size;
} screen_scale;
#endif

void Get_Resolution(void);
