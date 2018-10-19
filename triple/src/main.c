#include "main.h"

#include <stdlib.h>
#include <time.h>
#include "fileio.h"
#include "input.h"
#include "states.h"
#include "video.h"

int quit;

int init()
{
	getConfigDir();
	getConfig();

	if(initSDL())
	{
		return -1;
	}

	return 0;
}

void deinit()
{
	if(configDir)
	{
		free(configDir);
	}
}

int main(int argc, char *argv[])
{
	quit = 0;

	if(init())
	{
		quit = 1;
	}

	srand(time(NULL));

	while(!quit)
	{
		if(!frameLimiter())
		{
			input();
			logic();
			draw();
		}
	}

	storeConfig();
	deinit();

	SDL_Quit();
	return 0;
}
