#include "fileio.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(HOME_DIR)
#include <sys/stat.h>
#endif
#include "game.h"
#include "input.h"
#include "video.h"

char configDir[FILE_MAX_PATH];

int getConfigDir()
{
#if defined(HOME_DIR)
	char *homeDir = getenv("HOME");

	if (!homeDir)
		return -1;

	if (snprintf(configDir, FILE_MAX_PATH, "%s/.homingFever", homeDir) >= FILE_MAX_PATH)
		return -1;

	mkdir(configDir, 0755); /* Create the directory if it doesn't exist. */
#else
	strcpy(configDir, ".");
#endif
	return 0;
}

void getConfig()
{
	FILE *f;
	char config[FILE_MAX_PATH];
	char line[100];

	if (snprintf(config, FILE_MAX_PATH, "%s/game.cfg", configDir) >= FILE_MAX_PATH)
	{
		printf("Failed to retrieve config file path.\n");
		return;
	}

	f = fopen(config, "r");

	if(f == NULL)
	{
		printf("Failed to open config file: \"%s\" for reading.\n", config);
		return;
	}

	while(fgets(line, sizeof(line), f))
	{
		char *arg = strchr(line, ' ');

		if(!arg)
		{
			continue;
		}
		*arg = '\0';
		arg++;

		if (line[0] == '#')
			continue;

		if (!strcmp(line, "JOY_MODE"))
			sscanf(arg, "%d", (int *)&joyMode);
		else if (!strcmp(line, "JOY_NUM"))
			sscanf(arg, "%d", (int *)&joyNum);
		else if (!strcmp(line, "JOY_DEADZONE"))
			sscanf(arg, "%d", (int *)&joyDeadzone);
		else if (!strcmp(line, "SCALE") && !screenScale)
			sscanf(arg, "%d", (int *)&screenScale);
		else if (!strcmp(line, "FULLSCREEN"))
			sscanf(arg, "%d", (int *)&fullscreen);
	}

	fclose(f);
}

void storeConfig()
{
	FILE *f;
	char config[FILE_MAX_PATH];

	if (snprintf(config, FILE_MAX_PATH, "%s/game.cfg", configDir) >= FILE_MAX_PATH)
	{
		printf("Failed to retrieve config file path.\n");
		return;
	}

	f = fopen(config, "w");

	if(f == NULL)
	{
		printf("Failed to open config file: \"%s\" for writing.\n", config);
		return;
	}

	fprintf(f, "# Joystick control mode. Value: 0 - off, 1 - digital, 2 - analog\n");
	fprintf(f, "JOY_MODE %d\n\n", joyMode);
	fprintf(f, "# Joystick device number. Value: 0 - first joystick, 1 - second joystick, etc.\n");
	fprintf(f, "JOY_NUM %d\n\n", joyNum);
	fprintf(f, "# Joystick deadzone. Value range: 0 - 65535\n");
	fprintf(f, "JOY_DEADZONE %d\n\n", joyDeadzone);
	fprintf(f, "# Screen scale. Value: 1 - original, 2 - double\n");
	fprintf(f, "SCALE %d\n\n", screenScale);
	fprintf(f, "# Display mode. Value: 0 - windowed, 1 - fullscreen\n");
	fprintf(f, "FULLSCREEN %d\n", fullscreen);

	fclose(f);
}


void getHiscore()
{
	FILE *f;
	char save[FILE_MAX_PATH];
	char header[] = HISCORE_HEADER;
	uint8_t version;

	if (snprintf(save, FILE_MAX_PATH, "%s/score.dat", configDir) >= FILE_MAX_PATH)
	{
		printf("Failed to retrieve save file path.\n");
		return;
	}

	f = fopen(save, "rb");

	if(f == NULL)
	{
		printf("Failed to open score file: \"%s\" for writing.\n", save);
		return;
	}

	fread(&header, sizeof(char), strlen(header), f);

	if (strcmp(header, HISCORE_HEADER))
	{
		printf("File \"%s\" is not a valid score file.\n", save);
		fclose(f);
		return;
	}

	fread(&version, sizeof(uint8_t), 1, f);

	if (version != HISCORE_FORMAT_VERSION)
	{
		fclose(f);
		printf("Incompatible score file in version %d. Required version: %d.\n", version, SAVE_FORMAT_VERSION);
		return;
	}

	fread(&bestTime, sizeof(uint16_t), 1, f);

	fclose(f);
}

void storeHiscore()
{
	FILE *f;
	char save[FILE_MAX_PATH];
	char header[] = HISCORE_HEADER;
	uint8_t version = HISCORE_FORMAT_VERSION;

	if (snprintf(save, FILE_MAX_PATH, "%s/score.dat", configDir) >= FILE_MAX_PATH)
	{
		printf("Failed to retrieve save file path.\n");
		return;
	}

	f = fopen(save, "wb");

	if(f == NULL)
	{
		printf("Failed to open score file: \"%s\" for writing.\n", save);
		return;
	}

	fwrite(&header, sizeof(char), strlen(header), f);
	fwrite(&version, sizeof(uint8_t), 1, f);
	fwrite(&bestTime, sizeof(uint16_t), 1, f);

	fclose(f);
}
