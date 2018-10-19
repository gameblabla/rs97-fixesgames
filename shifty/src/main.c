#include <stdlib.h>
#include <time.h>
#include "backend/backend.h"
#include "backend/input.h"
#include "backend/video.h"
#include "fileio.h"
#include "font.h"
#include "states.h"
#include "version.h"

static int init()
{
	int ret;
	Font *fontShadow;

	ret = backendInit();

	if (ret)
		return -1;

	if (!(fontShadow = fontLoad("data/fontShadow.bmp", 6, 11, 1, 4, NULL)))
		goto error;
	if (!(fontDefault = fontLoad("data/font.bmp", 6, 11, 1, 4, fontShadow)))
		goto error;

	return 0;

	error:
		return -1;
}

static void clean()
{
	if (fontDefault)
	{
		if (fontDefault->shadow)
			fontUnload(fontDefault->shadow);

		fontUnload(fontDefault);
	}

	backendClean();
}

int main(int argc, char *argv[])
{
	int i;
	int parsedCustomConfig = 0;

	readConfig();

	for (i = 0; i < argc; ++i)
	{
		int debug = 0;

#if defined(DEBUG)
		debug = 1;
#endif

parser:
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
		{
			printf("Shifty Pills v%d.%d.%d%s, Copyright (c) 2016-2017 Artur Rojek.\nThis program is MIT licensed.\n\n", PROGRAM_MAJOR_VERSION, PROGRAM_MINOR_VERSION, PROGRAM_PATCH_VERSION, debug ? " DEBUG" : "");
			printf("Usage:\n");
			printf("%s [options]\n\n", argv[0]);
			printf("Options:\n");
			printf("-h or --help                       Displays this help screen.\n");
			printf("-v or --version                    Displays the program version.\n\n");
			printf("-c <file> or --config <file>       Reads game config from a custom location.\n");
			printf("-s <1..4> or --scale <1..4>        Sets the screen scale.\n");
			printf("-f or --full-screen                Launches game in full screen mode.\n");
			printf("-w or --windowed                   Launches game in windowed mode.\n");
			printf("-r or --frame-rate                 Displays a frame rate counter.\n");
			printf("\n");
			printf("Note: Options passed as parameters will overwrite game config values.\n");

			return 0;
		}
		if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v"))
		{
			printf("%d.%d.%d%s\n", PROGRAM_MAJOR_VERSION, PROGRAM_MINOR_VERSION, PROGRAM_PATCH_VERSION, debug ? " DEBUG" : "");

			return 0;
		}
		if (!parsedCustomConfig && (!strcmp(argv[i], "--config") || !strcmp(argv[i], "-c")))
		{
			if ((argc - 1 < i + 1) || argv[i+1][0] == '-')
			{
				fprintf(stderr, "Not enough parameters for %s.\n1 string parameter required.\n", argv[i]);
				return 1;
			}

			if (snprintf(configFile, FILE_PATH_MAX, "%s", argv[i+1]) >= FILE_PATH_MAX)
			{
				fprintf(stderr, "Path too long.\nConfig file path needs to be at most %d characters.\n", FILE_PATH_MAX);
				return 1;
			}

			/* Reload config at the new location, then reparse all the parameters that the config could have overwritten. */
			readConfig();
			parsedCustomConfig = 1;
			i = 0;
			goto parser;
		}
		if (!strcmp(argv[i], "--scale") || !strcmp(argv[i], "-s"))
		{
			int scale;
			char *endptr;

			if ((argc - 1 < i + 1) || argv[i+1][0] == '-')
			{
				fprintf(stderr, "Not enough parameters for %s.\n1 integer parameter required.\n", argv[i]);
				return 1;
			}

			scale = strtol(argv[i+1], &endptr, 10);

			if (endptr == argv[i+1] || scale < 1 || scale > 4)
			{
				printf("Invalid value '%s' for %s.\nValue must be in range <1..4>.\n", argv[i+1], argv[i]);
				return 1;
			}

			screenScale = scale;
		}
		if (!strcmp(argv[i], "--full-screen") || !strcmp(argv[i], "-f"))
		{
			fullscreen = 1;
		}
		if (!strcmp(argv[i], "--windowed") || !strcmp(argv[i], "-w"))
		{
			fullscreen = 0;
		}
		if (!strcmp(argv[i], "--frame-rate") || !strcmp(argv[i], "-r"))
		{
			showFps = 1;
		}
	}

	// fix for retrogmae
	screenScale = 1;
	srand(time(NULL));

	quit = init();

	while (!quit)
	{
		if (frameLimiter())
			continue;

		frameCounter();

		input();
		logic();
		drawing();
	}

	storeConfig();
	clean();

	return 0;
}
