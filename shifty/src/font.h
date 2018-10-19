#ifndef _FONT_H_
#define _FONT_H_

#include "tileset.h"

#define FONT_SHADOW_OFFSET_X	1
#define FONT_SHADOW_OFFSET_Y	1

typedef struct Font
{
	Tileset *tileset;
	int w;
	int h;
	int tracking;
	int leading;
	struct Font *shadow;
} Font;

typedef enum ShadowType
{
	SHADOW_NONE,
	SHADOW_DROP,
	SHADOW_OUTLINE
} ShadowType;

extern Font *fontDefault;

Font *fontLoad(const char *fileName, unsigned int glyphWidth, unsigned int glyphHeight, unsigned int tracking, unsigned int leading, Font *shadow);
void fontUnload(Font *font);
void dText(Font *font, ShadowType shadowType, const char *text, int x, int y);
void dTextCenteredAtOffset(Font *font, ShadowType shadowType, const char *text, int y, int offset);
void dTextCentered(Font *font, ShadowType shadowType, const char *text, int y);

#endif /* _FONT_H_ */
