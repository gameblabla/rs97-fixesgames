#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "homedir.h"

char *home_dir;
char fallback[] = "./";
extern char *replay_dir;
extern char *high_dir;

void get_home()
{
	char *home = getenv("HOME");
	if (home != NULL) {
		home_dir = (char *)malloc(strlen(home) + strlen("/.stransball2") + 1);
		if (home_dir != NULL) {
			sprintf(home_dir, "%s/.stransball2", home);
			mkdir(home_dir, 0755); // Create the directory if it doesn't exist.
		}
	}

	if (home == NULL) {
		home_dir = fallback;
	}

	replay_dir = (char *)malloc(strlen(home_dir) + strlen("/replays") + 1);
	if (replay_dir != NULL) {
		sprintf(replay_dir, "%s/replays", home_dir);
		mkdir(replay_dir, 0755);
	}

	high_dir = (char *)malloc(strlen(home_dir) + strlen("/high") + 1);
	if (high_dir != NULL) {
		sprintf(high_dir, "%s/high", home_dir);
		mkdir(high_dir, 0755);
	}
}

void free_home()
{
	if (home_dir != NULL)
	{
		free(home_dir);
	}
	if (replay_dir != NULL)
	{
		free(replay_dir);
	}
	if (high_dir != NULL)
	{
		free(high_dir);
	}
}
