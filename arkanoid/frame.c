
#include "includes.h"

#define FPS_Default 1000 / 70
u32 gnTimer1, gnTimer2;

// Init timers.
void FrameInit(void)
{
	gnTimer1 = SDL_GetTicks();
}

// Attente de la frame.
void FrameWait(void)
{
	// S'assurer que l'on ne va pas trop vite...
	while (1)
	{
		gnTimer2 = SDL_GetTicks() - gnTimer1;
		if (gnTimer2 >= FPS_Default) break;
		SDL_Delay(3);
	}
	gnTimer1 = SDL_GetTicks();
}




