#include "backend.h"
#include <SDL.h>
#include "video.h"

int backendInit()
{
	int ret;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK))
		return -1;

	SDL_WM_SetCaption("Shifty Pills", NULL);
	SDL_ShowCursor(SDL_DISABLE);

	ret = videoInit();

	return ret;
}

void backendClean()
{
	videoClean();
	SDL_Quit();
}
