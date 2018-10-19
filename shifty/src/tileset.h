#ifndef _TILESET_H_
#define _TILESET_H_

#include "backend/video.h"

typedef struct Tileset
{
	Image *image;
	Rect *clip;
	int totalTiles;
} Tileset;

Tileset *tilesetLoad(const char *fileName, unsigned int tileWidth, unsigned int tileHeight, unsigned int rowWidth, unsigned int totalTiles);
void tilesetUnload(Tileset *tileset);

#endif /* _TILESET_H_ */
