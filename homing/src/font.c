#include "font.h"

#include <SDL.h>
#include "game.h"
#include "video.h"

font gameFont;
font gameFontShadow;

void fontLoad(font *fontObj, char *filename, int glyphWidth, int glyphHeight, int tracking, int leading, font *shadow)
{
	tilesetLoad(&fontObj->tiles, filename, glyphWidth, glyphHeight, 16, 256);

	fontObj->w = glyphWidth;
	fontObj->h = glyphHeight;
	fontObj->tracking = tracking;
	fontObj->leading = leading;
	fontObj->shadow = shadow;
}

void fontUnload(font *fontObj)
{
	tilesetUnload(&fontObj->tiles);
}

void dTextCentered(font *fontObj, char *string, int y, int alpha, shadowType withShadow)
{
	int width = 0;
	int len = strlen(string);
	int i;

	for (i = 0; string[i] != '\0'; ++i)
	{
		if (string[i] == '\n') /* Line break. */
			break;

		width += fontObj->w;
		if (i < len - 1)
			width += fontObj->tracking;
	}

	dText(fontObj, string, SCREEN_W/2 - width/2, y, alpha, withShadow);
}

void dText(font *fontObj, char *string, int x, int y, int alpha, shadowType withShadow)
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
		if (string[letterNum] == '\n') /* Line break. */
		{
			r.x = origX - fontObj->w - fontObj->tracking;
			y += fontObj->h + fontObj->leading;
		}

		if (fontObj->shadow)
		{
			SDL_SetAlpha(fontObj->shadow->tiles.image, SDL_SRCALPHA, alpha%256);

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

		SDL_SetAlpha(fontObj->tiles.image, SDL_SRCALPHA, alpha%256);
		drawImage(fontObj->tiles.image, &fontObj->tiles.clip[(signed int)string[letterNum]], screen, r.x, r.y);

		letterNum++;
		r.x += fontObj->tracking + fontObj->w;
		r.y = y;
	}
}

void dTextEmerging(font *fontObj, char *string, int x, int y, int step, int alpha, shadowType withShadow)
{
	SDL_Rect r;
	SDL_Rect clip;
	int letterNum = 0;
	int origX = x;
	int i;
	int drawAreaLen;

	if (!string)
	{
		return;
	}

	if ((unsigned int)step > strlen(string) * (fontObj->w + fontObj->tracking))
		step = strlen(string) * (fontObj->w + fontObj->tracking);

	drawAreaLen = x + step;

	r.x = x;
	r.y = y;

	for (i = 0; string[i] != '\0'; ++i)
	{
		if (string[letterNum] == '\n') /* Line break. */
		{
			r.x = origX - fontObj->w - fontObj->tracking;
			y += fontObj->h + fontObj->leading;
		}

		if (drawAreaLen > r.x)
		{
			if (fontObj->shadow)
			{
				SDL_Rect clip;

				SDL_SetAlpha(fontObj->shadow->tiles.image, SDL_SRCALPHA, alpha%256);

				clip = fontObj->shadow->tiles.clip[(signed int)string[letterNum]];
				clip.w = (drawAreaLen - r.x > clip.w ? clip.w : drawAreaLen - r.x);

				switch (withShadow)
				{
					case SHADOW_DROP:
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x + FONT_SHADOW_OFFSET_X, r.y);
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x, r.y + FONT_SHADOW_OFFSET_Y);
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x + FONT_SHADOW_OFFSET_X, r.y + FONT_SHADOW_OFFSET_Y);
					break;
					case SHADOW_OUTLINE:
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x - FONT_SHADOW_OFFSET_X, r.y);
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x + FONT_SHADOW_OFFSET_X, r.y);
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x, r.y - FONT_SHADOW_OFFSET_Y);
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x, r.y + FONT_SHADOW_OFFSET_Y);
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x - FONT_SHADOW_OFFSET_X, r.y - FONT_SHADOW_OFFSET_Y);
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x - FONT_SHADOW_OFFSET_X, r.y + FONT_SHADOW_OFFSET_Y);
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x + FONT_SHADOW_OFFSET_X, r.y - FONT_SHADOW_OFFSET_Y);
						drawImage(fontObj->shadow->tiles.image, &clip, screen, r.x + FONT_SHADOW_OFFSET_X, r.y + FONT_SHADOW_OFFSET_Y);
					break;

					default:
					break;
				}
			}

			SDL_SetAlpha(fontObj->tiles.image, SDL_SRCALPHA, alpha%256);

			clip = fontObj->tiles.clip[(signed int)string[letterNum]];
			clip.w = (drawAreaLen - r.x > clip.w ? clip.w : drawAreaLen - r.x);

			drawImage(fontObj->tiles.image, &clip, screen, r.x, r.y);
		}

		letterNum++;
		r.x += fontObj->tracking + fontObj->w;
		r.y = y;
	}
}
