#include <stdio.h>
#include <SDL.h>

#ifndef DREAMCAST
#include "icon.h"
#else
#include <kos.h>
#include "vmu_icon.h"
#endif

#ifndef NAME_CAPTION
#define NAME_CAPTION "DCaSTaway"
#endif

void show_icon(void)
{
#ifndef DREAMCAST
	SDL_RWops	*iconfp;
	SDL_Surface *icon;

	SDL_WM_SetCaption(NAME_CAPTION,NAME_CAPTION);
	iconfp=SDL_RWFromMem((void*)rawicon, sizeof(rawicon));
	icon=SDL_LoadBMP_RW(iconfp,0);
	SDL_WM_SetIcon(icon,NULL);
#else
	vmu_set_icon(vmu_icon);
#endif
	SDL_ShowCursor(SDL_DISABLE);
}
