#include "fileio.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "hiscore.h"

char *configDir;

void getConfigDir()
{
	char *homeDir = getenv("HOME");

	if(homeDir != NULL)
	{
		configDir = (char *)malloc(strlen(homeDir) + strlen("/.tripleTrapled") + 1);
		if(configDir != NULL)
		{
			sprintf(configDir, "%s/.tripleTrapled", homeDir);
			mkdir(configDir, 0755); // create the directory if doesn't exist
		}
	}
}

void getConfig()
{
	FILE *f;
	char *config;

	if(!configDir)
	{
		printf("Config directory doesn't exist.\n");
		return;
	}

	config = (char *)malloc(strlen(configDir) + strlen("/tt.cfg") + 1);

	if(!config)
	{
		return;
	}

	sprintf(config, "%s/tt.cfg", configDir);

	f = fopen(config, "r");

	if(f == NULL)
	{
		printf("Failed to open config file: \"%s\" for reading.\n", config);
		return;
	}

	char line[15];

	while(fgets(line, sizeof(line), f))
	{
		char *arg = strchr(line, ' ');

		if(!arg)
		{
			continue;
		}
		*arg = '\0';
		arg++;

		if(!strcmp(line, "HI"))
		{
			sscanf(arg, "%d", &hiscore[0]);
		}
		else if(!strcmp(line, "HI2"))
		{
			sscanf(arg, "%d", &hiscore[1]);
		}
		else if(!strcmp(line, "HI3"))
		{
			sscanf(arg, "%d", &hiscore[2]);
		}
		else if(!strcmp(line, "HI4"))
		{
			sscanf(arg, "%d", &hiscore[3]);
		}
		else if(!strcmp(line, "HI5"))
		{
			sscanf(arg, "%d", &hiscore[4]);
		}
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

	config = (char *)malloc(strlen(configDir) + strlen("/tt.cfg") + 1);

	if(!config)
	{
		return;
	}

	sprintf(config, "%s/tt.cfg", configDir);

	f = fopen(config, "w");

	if(f == NULL)
	{
		printf("Failed to open config file: \"%s\" for writing.\n", config);
		return;
	}

	fprintf(f, "HI %d\nHI2 %d\nHI3 %d\nHI4 %d\nHI5 %d\n", hiscore[0], hiscore[1], hiscore[2], hiscore[3], hiscore[4]);

	fclose(f);
	free(config);
}
