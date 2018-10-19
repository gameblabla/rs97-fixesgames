#include "main.h"

#include <stdlib.h>
#include <time.h>
#include "audio.h"
#include "fileio.h"
#include "font.h"
#include "input.h"
#include "board.h"
#include "states.h"
#include "video.h"

int quit;

int init()
{
	getConfigDir();
	getConfig();
	getHiscore(0);


	if (!scale)
	{
		scale = DEFAULT_SCALE;
	}

	if (initSDL())
	{
		return -1;
	}

	hasAudio = !initAudio();

	if (fontLoad(&gameFontShadow, "data/gfx/fontGray.bmp", 6, 11, 1, 4, NULL))
	{
		return -1;
	}

	if (fontLoad(&gameFontRegular, "data/gfx/fontWhite.bmp", 6, 11, 1, 4, &gameFontShadow))
	{
		return -1;
	}

	if (fontLoad(&gameFontSelected, "data/gfx/fontLightOrange.bmp", 6, 11, 1, 4, &gameFontShadow))
	{
		return -1;
	}

	if (fontLoad(&gameFontBlack, "data/gfx/fontBlack.bmp", 6, 11, 1, 4, &gameFontRegular))
	{
		return -1;
	}

	return 0;
}

void deinit()
{
	if (configDir)
	{
		free(configDir);
	}

	fontUnload(&gameFontShadow);
	fontUnload(&gameFontRegular);
	fontUnload(&gameFontSelected);
	fontUnload(&gameFontBlack);

	deinitAudio();
	deinitSDL();
}

int main(int argc, char *argv[])
{
	int i;
	quit = 0;

	for (i = 1; i < argc; i++)
	{
		if ((!strcmp(argv[i], "--help")) || (!strcmp(argv[i], "-help")) || (!strcmp(argv[i], "-h")))
		{
			printf("Shisen-Seki v%d.%d.%d, Copyright (c) 2015 Artur Rojek.\nThis program is MIT licensed.\n\n", PROGRAM_MAJOR_VERSION, PROGRAM_MINOR_VERSION, PROGRAM_PATCH_VERSION);
			printf("Options:\n");
			printf("-s or -scale [1-2]\t\tScales the screen resolution\n");
			printf("-v or -version\t\t\tDisplay the program version\n");
			printf("-h or -help\t\t\tDisplay this help screen\n");
			return 0;
		}
		if ((!strcmp(argv[i], "-version")) || (!strcmp(argv[i], "-v")))
		{
			printf("%d.%d.%d\n", PROGRAM_MAJOR_VERSION, PROGRAM_MINOR_VERSION, PROGRAM_PATCH_VERSION);
			return 0;
		}
		if ((!strcmp(argv[i], "-scale")) || (!strcmp(argv[i], "-s")))
		{
			if (argc - 1 < i + 1)
			{
				printf("Not enough arguments for %s.\n1 integer parameter required.\n", argv[i]);
				return 1;
			}
			else
			{
				scale = atoi(argv[i+1]);
				if (scale < 1 || scale > 2)
				{
					printf("Invalid value '%s' for %s.\nValue must be between [1-2].\n", argv[i+1], argv[i]);
					return 1;
				}
			}

			++i;
		}
	}

	if (init())
	{
		quit = 1;
	}

	srand(time(NULL));

	while (!quit)
	{
		if (!frameLimiter())
		{
			input();
			logic();
			draw();
		}
	}

	storeConfig();
	deinit();

	return 0;
}
