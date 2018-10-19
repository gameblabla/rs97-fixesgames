#include "tileset.h"

#include <stdlib.h>
#include "video.h"

void tilesetLoad(tileset *tSet, char *fileName, int width, int height, int rowLen, int length)
{
	if (!tSet)
	{
		return;
	}

	if (!fileName)
	{
		return;
	}

	tSet->image = loadImage(fileName);
	if (!tSet->image)
	{
		fprintf(stderr, "ERROR: Failed to load file: %s\n", fileName);
		return;
	}

	if (!tSet->clip)
	{
		tSet->clip = malloc(sizeof(SDL_Rect) * length);
		if (!tSet->clip)
		{
			fprintf(stderr, "ERROR: Not enough memory for allocation.\n");
			return;
		}
	}
	clipImage(tSet->clip, width, height, rowLen, length);
	tSet->rowLen = rowLen;
	tSet->length = length;
}

void tilesetUnload(tileset *tSet)
{
	if (!tSet)
	{
		return;
	}

	SDL_FreeSurface(tSet->image);
	tSet->image = NULL;
	free(tSet->clip);
	tSet->clip = NULL;
}
