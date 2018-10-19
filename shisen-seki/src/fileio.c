#include "fileio.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "audio.h"
#include "board.h"
#include "game.h"
#include "hiscore.h"
#include "input.h"
#include "video.h"

char *configDir;

void getConfigDir()
{
	char *homeDir = getenv("HOME");

	if(homeDir != NULL)
	{
		configDir = (char *)malloc(strlen(homeDir) + strlen("/.shisen-seki") + 1);
		if(configDir != NULL)
		{
			sprintf(configDir, "%s/.shisen-seki", homeDir);
			mkdir(configDir, 0755); // create the directory if doesn't exist
		}
	}
}

void getConfig()
{
	FILE *f;
	char *config;
	char line[15];

	if(!configDir)
	{
		printf("Config directory doesn't exist.\n");
		return;
	}

	config = (char *)malloc(strlen(configDir) + strlen("/game.cfg") + 1);

	if(!config)
	{
		return;
	}

	sprintf(config, "%s/game.cfg", configDir);

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

	if (!strcmp(line, "USE_JOYSTICK"))
		sscanf(arg, "%d", (int *)&enableJoystick);
	else if (!strcmp(line, "MUSIC"))
		sscanf(arg, "%d", (int *)&enableMusic);
	else if (!strcmp(line, "SFX"))
		sscanf(arg, "%d", (int *)&enableSfx);
	else if (!strcmp(line, "SCALE") && !scale)
		sscanf(arg, "%d", (int *)&scale);
	else if (!strcmp(line, "GAME_MODE"))
		sscanf(arg, "%d", (int *)&newGameMode);
	else if (!strcmp(line, "ALGORITHM"))
		sscanf(arg, "%d", (int *)&currentAlgorithm);
	else if (!strcmp(line, "ANIMATIONS"))
		sscanf(arg, "%d", &showAnimations);
	}

	fclose(f);
	free(config);
}

void storeConfig()
{
	FILE *f;
	char *config;

	if(!configDir)
	{
		printf("Config directory doesn't exist.\n");
		return;
	}

	config = (char *)malloc(strlen(configDir) + strlen("/game.cfg") + 1);

	if(!config)
	{
		return;
	}

	sprintf(config, "%s/game.cfg", configDir);

	f = fopen(config, "w");

	if(f == NULL)
	{
		printf("Failed to open config file: \"%s\" for writing.\n", config);
		return;
	}

	fprintf(f, "USE_JOYSTICK %d\nMUSIC %d\nSFX %d\nSCALE %d\nGAME_MODE %d\nALGORITHM %d\nANIMATIONS %d\n", enableJoystick, enableMusic, enableSfx, scale, newGameMode, currentAlgorithm, showAnimations);

	fclose(f);
	free(config);
}

int getBoard(int probe)
{
	FILE *f;
	char *save;
	int x;
	int y;
	uint8_t version;

	if(!configDir)
	{
		printf("Config directory doesn't exist.\n");
		return 0;
	}

	save = (char *)malloc(strlen(configDir) + strlen("/game.sav") + 1);

	if(!save)
	{
		return 0;
	}

	sprintf(save, "%s/game.sav", configDir);

	if (!stones && !probe)
	{
		free(save);
		return 0;
	}

	f = fopen(save, "rb");

	if(f == NULL)
	{
		printf("Failed to open save file: \"%s\" for writing.\n", save);
		return 0;
	}

	fread(&version, sizeof(uint8_t), 1, f);

	if (version != SAVE_FORMAT_VERSION)
	{
		fclose(f);
		free(save);
		printf("Incompatible save file in version %d. Required version: %d.\n", version, SAVE_FORMAT_VERSION);
		return 0;
	}

	if (probe)
	{
		fclose(f);
		free(save);
		return 1;
	}

	for (x = 0; x < BOARD_W; ++x)
	{
		for (y = 0; y < BOARD_H; ++y)
		{
			int index = y * BOARD_W + x;
			uint8_t value;

			fread(&value, 1, sizeof(uint8_t), f);
			stones[x][y].type = value ^ index;

			if (stones[x][y].type > 0)
			{
				++stonesLeft;
			}
		}
	}
	fread(&currentGameMode, sizeof(currentGameMode), 1, f);
	fread(&gameTime, sizeof(gameTime), 1, f);
	fread(&practice, sizeof(practice), 1, f);

	fclose(f);
	free(save);

	return 1;
}

void storeBoard()
{
	FILE *f;
	char *save;
	int x;
	int y;
	uint8_t version = SAVE_FORMAT_VERSION;

	if(!configDir)
	{
		printf("Config directory doesn't exist.\n");
		return;
	}

	save = (char *)malloc(strlen(configDir) + strlen("/game.sav") + 1);

	if(!save)
	{
		return;
	}

	sprintf(save, "%s/game.sav", configDir);

	if (!stones)
	{
		free(save);
		return;
	}

	f = fopen(save, "wb");

	if(f == NULL)
	{
		printf("Failed to open save file: \"%s\" for writing.\n", save);
		return;
	}

	fwrite(&version, sizeof(uint8_t), 1, f);
	for (x = 0; x < BOARD_W; ++x)
	{
		for (y = 0; y < BOARD_H; ++y)
		{
			int index = y * BOARD_W + x;
			uint8_t value = stones[x][y].type ^ index;
			fwrite(&value, 1, sizeof(uint8_t), f);
		}
	}
	fwrite(&currentGameMode, sizeof(currentGameMode), 1, f);
	fwrite(&gameTime, sizeof(gameTime), 1, f);
	fwrite(&practice, sizeof(practice), 1, f);

	fclose(f);
	free(save);
}

int getHiscore(int probe)
{
	FILE *f;
	char *save;
	int i;
	int j;
	uint8_t version;
	uint8_t scoreLen;

	if(!configDir)
	{
		printf("Config directory doesn't exist.\n");
		return 0;
	}

	save = (char *)malloc(strlen(configDir) + strlen("/score.dat") + 1);

	if(!save)
	{
		return 0;
	}

	sprintf(save, "%s/score.dat", configDir);

	f = fopen(save, "rb");

	if(f == NULL)
	{
		printf("Failed to open score file: \"%s\" for writing.\n", save);
		return 0;
	}

	fread(&version, sizeof(uint8_t), 1, f);

	if (version != HISCORE_FORMAT_VERSION)
	{
		fclose(f);
		free(save);
		printf("Incompatible score file in version %d. Required version: %d.\n", version, SAVE_FORMAT_VERSION);
		return 0;
	}

	if (probe)
	{
		fclose(f);
		free(save);
		return 1;
	}

	fread(&scoreLen, sizeof(uint8_t), 1, f);

	for (i = 0; i < MAX_MODES; ++i)
	{
		for (j = 0; j < scoreLen; ++j)
		{
			int n;

			for (n = 0; n < SCORE_NAME_LEN-1; ++n)
			{
				fread(&scoreTable[i][j].name[n], sizeof(char), 1, f);
			}
			scoreTable[i][j].name[SCORE_NAME_LEN-1] = '\n';
			fread(&scoreTable[i][j].time, sizeof(scoreTable[i][j].time), 1, f);
		}
	}

	fclose(f);
	free(save);

	return 1;
}

void storeHiscore()
{
	FILE *f;
	char *save;
	int i;
	int j;
	uint8_t version = HISCORE_FORMAT_VERSION;
	uint8_t scoreLen = MAX_SCORES;

	if(!configDir)
	{
		printf("Config directory doesn't exist.\n");
		return;
	}

	save = (char *)malloc(strlen(configDir) + strlen("/score.dat") + 1);

	if(!save)
	{
		return;
	}

	sprintf(save, "%s/score.dat", configDir);

	f = fopen(save, "wb");

	if(f == NULL)
	{
		printf("Failed to open score file: \"%s\" for writing.\n", save);
		return;
	}

	fwrite(&version, sizeof(uint8_t), 1, f);
	fwrite(&scoreLen, sizeof(uint8_t), 1, f);

	for (i = 0; i < MAX_MODES; ++i)
	{
		for (j = 0; j < scoreLen; ++j)
		{
			int n;

			for (n = 0; n < SCORE_NAME_LEN-1; ++n)
			{
				fwrite(&scoreTable[i][j].name[n], sizeof(char), 1, f);
			}
			fwrite(&scoreTable[i][j].time, sizeof(scoreTable[i][j].time), 1, f);
		}
	}

	fclose(f);
	free(save);
}
