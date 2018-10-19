#include "title.h"

#include "font.h"
#include "game.h"
#include "input.h"
#include "states.h"
#include "video.h"
#include "objects.h"

int titleTime;
int textIndex;
int textTimer;
size_t textStep[TEXT_LINES];
int showIntro = 1;
int fadeOut;
int fadeOutTimer;
object missile;

void titleUnload()
{
	objTemplateListHead = listElementDeleteAll(objTemplateListHead, objectTemplateItemDelete);
	showIntro = 0;
}

void titleLoad()
{
	titleTime = 0;
	textTimer = 0;
	textIndex = 0;
	memset(&textStep, 0, TEXT_LINES * sizeof(size_t));
	fadeOut = 0;
	fadeOutTimer = 0;


	objectLoad(&missile, OBJ_MISSILE_YELLOW);
	missile.x = showIntro ? -missile.w : SCREEN_W;
	missile.y = 40;
	missile.angle = 270;
}

void titleLogic()
{
	++titleTime;

	if (keys[KEY_BACK])
	{
		keys[KEY_BACK] = 0;
		quit = 1;
	}

	if (keys[KEY_OK])
	{
		keys[KEY_OK] = 0;

		if (showIntro)
		{
			showIntro = 0;
			missile.x = SCREEN_W;
		}
		else
		{
			fadeOut = 1;
		}
	}

	if (keys[KEY_START])
	{
		keys[KEY_START] = 0;

		if (showIntro)
		{
			showIntro = 0;
			missile.x = SCREEN_W;
		}
		else
		{
			fadeOut = 1;
		}
	}

	if (fadeOut)
	{
		if (++fadeOutTimer == FADE_OUT_TIME)
			programStateNew = STATE_GAME;
	}


	if (titleTime >= 60)
		missile.x += 2;
}

void titleDraw()
{
	int i;
	int fontX = SCREEN_W/2 - (strlen("H O M I N G     F E V E R") * (gameFont.w + gameFont.tracking))/2;
	int fontStep = (missile.x < fontX ? 0 : missile.x - fontX);
	char bestTimeStr[10] = "-'--\"--";
	char text[TEXT_PAGES][TEXT_LINES][TEXT_LINE_LEN];

	if (bestTime)
		sprintf(bestTimeStr, "%d'%02d\"%02d", bestTime/60/60, bestTime/60%60, bestTime%60*1000/600);

	memset(text, 0, TEXT_PAGES*TEXT_LINES*TEXT_LINE_LEN);
	sprintf(text[0][1], "Press A or START to play");
	sprintf(text[0][3], "or SELECT to exit");

	sprintf(text[1][0], "Best time");
	sprintf(text[1][1], "%s", bestTimeStr);

	sprintf(text[2][0], "Your mission");
	sprintf(text[2][1], "To avoid missiles");
	sprintf(text[2][3], "and stay alive.");

	sprintf(text[3][0], "Author");
	sprintf(text[3][1], "Artur \"zear\" Rojek");
	sprintf(text[3][3], "(c) 2016");

	sprintf(text[4][0], "Special thanks");
	sprintf(text[4][1], "GhostlyDeath");
	sprintf(text[4][2], "jxv");
	sprintf(text[4][3], "Nebuleon");
	sprintf(text[4][4], "pcercuei");
	sprintf(text[4][5], "Senquack");
	sprintf(text[4][6], "Surkow");

	drawBackground(screen, getColor(0, 0, 128));

	for (i = 0; i < SCREEN_W + 16; i += 16)
	{
		int j;
		for (j = 0; j < SCREEN_H + 16; j += 16)
		{
			int x = i % 16;
			int y = j % 16;

			drawPoint(screen, i - x, j - y, getColor(0, 0, 224));
		}
	}

	if (titleTime >= 60)
		drawImage(missile.tiles->image, &missile.tiles->clip[((360 - missile.angle)/22)%missile.tiles->length], screen, missile.x, missile.y);

	dTextEmerging(&gameFont, "H O M I N G     F E V E R", fontX, 40, fontStep, fadeOutTimer ? 255 - 256/FADE_OUT_TIME * fadeOutTimer : ALPHA_OPAQUE, SHADOW_OUTLINE);

	if (missile.x >= SCREEN_W)
	{
		int i;

		showIntro = 0;
		++textTimer;

		if (!(textTimer % (60*8)))
		{
			textTimer = 0;
			textIndex = (textIndex+1) % TEXT_PAGES;

			for (i = 0; i < TEXT_LINES; ++i)
			{
				textStep[i] = 0;
			}
		}

		for (i = 0; i < TEXT_LINES; ++i)
		{
			if (!i || (textStep[i-1] > strlen(text[textIndex][i-1]) * (gameFont.w + gameFont.tracking)))
				textStep[i] += 2;

			dTextEmerging(&gameFont, text[textIndex][i], (SCREEN_W/2 - (strlen(text[textIndex][i]) * (gameFont.w + gameFont.tracking))/2), SCREEN_H/2 - ((gameFont.h + gameFont.leading)*TEXT_PAGES)/2 + (i ? (gameFont.h + gameFont.leading) * (i + 1): 0), textStep[i], fadeOutTimer ? 255 - 256/FADE_OUT_TIME * fadeOutTimer : ALPHA_OPAQUE, i ? SHADOW_DROP : SHADOW_OUTLINE);
		}

/*		dTextCentered(&gameFont, "<-  Control: D-PAD  ->", SCREEN_H/2 + 50 + (gameFont.h + gameFont.leading) * 3, fadeOutTimer ? 255 - 256/FADE_OUT_TIME * fadeOutTimer : ALPHA_OPAQUE, SHADOW_DROP);*/
	}
}
