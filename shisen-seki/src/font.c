#include "font.h"

#include <SDL.h>
#include "game.h"
#include "main.h"
#include "video.h"

font gameFontShadow;
font gameFontRegular;
font gameFontSelected;
font gameFontBlack;

int fontLoad(font *fontObj, char *filename, int glyphWidth, int glyphHeight, int tracking, int leading, font *shadow)
{
	if (tilesetLoad(&fontObj->tiles, filename, glyphWidth, glyphHeight, 16, 256))
	{
		return -1;
	}

	fontObj->w = glyphWidth;
	fontObj->h = glyphHeight;
	fontObj->tracking = tracking;
	fontObj->leading = leading;
	fontObj->shadow = shadow;

	return 0;
}

void fontUnload(font *fontObj)
{
	tilesetUnload(&fontObj->tiles);
}

void dTextCentered(font *fontObj, char *string, int y, shadowType withShadow)
{
	int width = 0;
	int len = strlen(string);
	int i;

	for (i = 0; string[i] != '\0'; ++i)
	{
		if (string[i] == '\n') // line break
			break;

		width += fontObj->w;
		if (i < len - 1)
			width += fontObj->tracking;
	}

	dText(fontObj, string, SCREEN_W/2 - width/2, y, withShadow);
}

void dText(font *fontObj, char *string, int x, int y, shadowType withShadow)
{
	SDL_Rect r;
	int letterNum = 0;
	int origX = x;
	int i;

	if (!string)
	{
		return;
	}

	r.x = x;
	r.y = y;

	for (i = 0; string[i] != '\0'; ++i)
	{
		if (string[letterNum] == '\n') // line break
		{
			r.x = origX - fontObj->w - fontObj->tracking;
			y += fontObj->h + fontObj->leading;
		}

		if (fontObj->shadow)
		{
			switch (withShadow)
			{
				case SHADOW_DROP:
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x + FONT_SHADOW_OFFSET_X, r.y);
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x, r.y + FONT_SHADOW_OFFSET_Y);
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x + FONT_SHADOW_OFFSET_X, r.y + FONT_SHADOW_OFFSET_Y);
				break;
				case SHADOW_OUTLINE:
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x - FONT_SHADOW_OFFSET_X, r.y);
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x + FONT_SHADOW_OFFSET_X, r.y);
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x, r.y - FONT_SHADOW_OFFSET_Y);
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x, r.y + FONT_SHADOW_OFFSET_Y);
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x - FONT_SHADOW_OFFSET_X, r.y - FONT_SHADOW_OFFSET_Y);
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x - FONT_SHADOW_OFFSET_X, r.y + FONT_SHADOW_OFFSET_Y);
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x + FONT_SHADOW_OFFSET_X, r.y - FONT_SHADOW_OFFSET_Y);
					drawImage(fontObj->shadow->tiles.image, &fontObj->shadow->tiles.clip[(signed int)string[letterNum]], screen, r.x + FONT_SHADOW_OFFSET_X, r.y + FONT_SHADOW_OFFSET_Y);
				break;

				default:
				break;
			}
		}
		drawImage(fontObj->tiles.image, &fontObj->tiles.clip[(signed int)string[letterNum]], screen, r.x, r.y);

		letterNum++;
		r.x += fontObj->tracking + fontObj->w;
		r.y = y;
	}
}
