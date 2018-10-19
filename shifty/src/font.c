#include "font.h"
#include <stdlib.h>
#include "tileset.h"

Font *fontDefault;

Font *fontLoad(const char *fileName, unsigned int glyphWidth, unsigned int glyphHeight, unsigned int tracking, unsigned int leading, Font *shadow)
{
	Font *font = NULL;

	if (!fileName)
		goto error;

	font = malloc(sizeof(Font));

	if (!font)
		goto error;

	font->tileset = tilesetLoad(fileName, glyphWidth, glyphHeight, 16, 256);

	if (!font->tileset)
		goto error;

	font->w = glyphWidth;
	font->h = glyphHeight;
	font->tracking = tracking;
	font->leading = leading;
	font->shadow = shadow;

	return font;

	error:
		if (font)
			free(font);

		return NULL;
}

void fontUnload(Font *font)
{
	if (!font)
		return;

	tilesetUnload(font->tileset);
	free(font);
}

void dText(Font *font, ShadowType shadowType, const char *text, int x, int y)
{
	Rect r;
	int letterNum = 0;
	int origX = x;
	int i;

	if (!text)
		return;

	r.x = x;
	r.y = y;

	for (i = 0; text[i] != '\0'; ++i)
	{
		if (text[letterNum] == '\n') /* Line break. */
		{
			r.x = origX - font->w - font->tracking;
			y += font->h + font->leading;
		}

		if (font->shadow)
		{
			switch (shadowType)
			{
				case SHADOW_DROP:
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x + FONT_SHADOW_OFFSET_X, r.y);
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x, r.y + FONT_SHADOW_OFFSET_Y);
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x + FONT_SHADOW_OFFSET_X, r.y + FONT_SHADOW_OFFSET_Y);
				break;
				case SHADOW_OUTLINE:
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x - FONT_SHADOW_OFFSET_X, r.y);
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x + FONT_SHADOW_OFFSET_X, r.y);
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x, r.y - FONT_SHADOW_OFFSET_Y);
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x, r.y + FONT_SHADOW_OFFSET_Y);
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x - FONT_SHADOW_OFFSET_X, r.y - FONT_SHADOW_OFFSET_Y);
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x - FONT_SHADOW_OFFSET_X, r.y + FONT_SHADOW_OFFSET_Y);
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x + FONT_SHADOW_OFFSET_X, r.y - FONT_SHADOW_OFFSET_Y);
					drawImage(font->shadow->tileset->image, &font->shadow->tileset->clip[(signed int)text[letterNum]], r.x + FONT_SHADOW_OFFSET_X, r.y + FONT_SHADOW_OFFSET_Y);
				break;

				default:
				break;
			}
		}

		drawImage(font->tileset->image, &font->tileset->clip[(signed int)text[letterNum]], r.x, r.y);

		++letterNum;
		r.x += font->tracking + font->w;
		r.y = y;
	}
}

void dTextCenteredAtOffset(Font *font, ShadowType shadowType, const char *text, int y, int offset)
{
	int width = 0;
	int len = strlen(text);
	int i;

	for (i = 0; text[i] != '\0'; ++i)
	{
		if (text[i] == '\n') /* Line break. */
			break;

		width += font->w;

		if (i < len - 1)
			width += font->tracking;
	}

	dText(font, shadowType, text, offset - width/2, y);
}

void dTextCentered(Font *font, ShadowType shadowType, const char *text, int y)
{
	dTextCenteredAtOffset(font, shadowType, text, y, SCREEN_W/2);
}
