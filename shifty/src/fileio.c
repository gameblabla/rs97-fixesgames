#include "fileio.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(HOME_DIR)
#include <sys/stat.h>
#endif

#include "backend/video.h"

static char configDir[FILE_PATH_MAX] = "\0";
char configFile[FILE_PATH_MAX] = "\0";

static int getConfigDir()
{
#if defined(HOME_DIR)
	char *homeDir = getenv("HOME");

	if (!homeDir)
		goto error;
#endif

	if (configDir[0] != '\0')
		return 0;

#if defined(HOME_DIR)
	if (snprintf(configDir, FILE_PATH_MAX, "%s/.shiftyPills", homeDir) >= FILE_PATH_MAX)
		goto error;

	mkdir(configDir, 0755); /* Create the directory if it doesn't exist. */
#else
	strcpy(configDir, ".");
#endif

	return 0;

error:
	configDir[0] = '\0';
	return -1;
}

int readConfig()
{
	FILE *f;
	char line[100];

	if (configFile[0] == '\0')
	{
		if (getConfigDir() || snprintf(configFile, FILE_PATH_MAX, "%s/game.cfg", configDir) >= FILE_PATH_MAX)
			return -1;
	}

	f = fopen(configFile, "r");

	if (!f)
		return -1;

	while (fgets(line, sizeof(line), f))
	{
		char *arg = strchr(line, ' ');

		if (!arg)
			continue;

		*arg = '\0';
		++arg;

		if (line[0] == '#')
			continue;

		if (!strcmp(line, "SCALE"))
			sscanf(arg, "%d", (int *)&screenScale);
		else if (!strcmp(line, "FULLSCREEN"))
			sscanf(arg, "%d", (int *)&fullscreen);
		else if (!strcmp(line, "SHOW_FPS"))
			sscanf(arg, "%d", (int *)&showFps);
	}

	fclose(f);

	return 0;
}

int storeConfig()
{
	FILE *f;

	if (configFile[0] == '\0')
		return -1;

	f = fopen(configFile, "w");

	if (!f)
		return -1;

	fprintf(f, "# Screen scale. Value range: 1-4\n");
	fprintf(f, "SCALE %d\n\n", screenScale);
	fprintf(f, "# Display mode. Value: 0 - windowed, 1 - fullscreen\n");
	fprintf(f, "FULLSCREEN %d\n", fullscreen);
	fprintf(f, "SHOW_FPS %d\n", showFps);

	fclose(f);

	return 0;
}
