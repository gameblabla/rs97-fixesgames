#include "tileset.h"
#include <stdlib.h>
#include "backend/backend.h"

Tileset *tilesetLoad(const char *fileName, unsigned int tileWidth, unsigned int tileHeight, unsigned int rowWidth, unsigned int totalTiles)
{
	Tileset *tileset = NULL;
	int i;
	
	if (!fileName)
		goto error;

	tileset = malloc(sizeof(Tileset));

	if (!tileset)
		goto error;

	tileset->image = loadImage(fileName);

	if (!tileset->image)
		goto error;

	tileset->totalTiles = totalTiles;
	tileset->clip = malloc(tileset->totalTiles * sizeof(Rect));

	if (!tileset->clip)
		goto error;

	for (i = 0; i < tileset->totalTiles; ++i)
	{
		tileset->clip[i].x = i%rowWidth * tileWidth;
		tileset->clip[i].y = i/rowWidth * tileHeight;
		tileset->clip[i].w = tileWidth;
		tileset->clip[i].h = tileHeight;
	}

	return tileset;

	error:
		if (tileset)
			free(tileset);
		return NULL;
}

void tilesetUnload(Tileset *tileset)
{
	if (!tileset)
		return;

	unloadImage(tileset->image);
	free(tileset);
}
