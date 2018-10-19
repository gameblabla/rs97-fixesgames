#ifndef _FONT_H_
#define _FONT_H_

#include "tileset.h"

#define FONT_SHADOW_OFFSET_X	1
#define FONT_SHADOW_OFFSET_Y	1

typedef struct font
{
	tileset tiles;
	int w;
	int h;
	int tracking;
	int leading;
	struct font *shadow;
} font;

typedef enum shadowType
{
	SHADOW_NONE = 0,
	SHADOW_DROP,
	SHADOW_OUTLINE
} shadowType;

extern font gameFontShadow;
extern font gameFontRegular;
extern font gameFontSelected;
extern font gameFontBlack;

int fontLoad(font *fontObj, char *filename, int glyphWidth, int glyphHeight, int tracking, int leading, font *shadow);
void fontUnload(font *fontObj);
void dTextCentered(font *fontObj, char *string, int y, shadowType withShadow);
void dText(font *fontObj, char *string, int x, int y, shadowType withShadow);

#endif /* _FONT_H_ */
