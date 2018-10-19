#include "tileset.h"

#include <stdlib.h>
#include "video.h"

int tilesetLoad(tileset *tSet, char *fileName, int width, int height, int rowLen, int length)
{
	if (!tSet)
	{
		return -1;
	}

	if (!fileName)
	{
		return -1;
	}

	tSet->image = loadImage(fileName);
	if (!tSet->image)
	{
		fprintf(stderr, "ERROR: Failed to load file: %s\n", fileName);
		return -1;
	}

	if (!tSet->clip)
	{
		tSet->clip = malloc(sizeof(SDL_Rect) * length);
		if (!tSet->clip)
		{
			fprintf(stderr, "ERROR: Not enough memory for allocation.\n");
			return -1;
		}
	}
	clipImage(tSet->clip, width, height, rowLen, length);

	return 0;
}

void tilesetUnload(tileset *tSet)
{
	if (!tSet)
	{
		return;
	}

	if (tSet->image)
	{
		SDL_FreeSurface(tSet->image);
		tSet->image = NULL;
	}
	if (tSet->clip)
	{
		free(tSet->clip);
		tSet->clip = NULL;
	}
}
