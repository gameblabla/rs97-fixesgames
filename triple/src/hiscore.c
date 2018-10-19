#include "hiscore.h"

#include <SDL.h>
#include "board.h"
#include "font.h"
#include "game.h"
#include "input.h"
#include "states.h"
#include "video.h"

int hiscore[5] = {100,80,60,40,20};
int topHiscore;
int toTitleTimer;
int scoreUncoverTimer;

void hiscoreUnload()
{
}

void hiscoreLoad()
{
	toTitleTimer = 300;
	scoreUncoverTimer = 0;
}

void hiscoreUpdate(int curScore)
{
	int i;
	int n;
	int place = 5;

	for(n = 4; n >= 0; n--, place--)
	{
		if(hiscore[n] > curScore)
		{
			break;
		}
	}

	if(place == 5) // Score not high enough for hi-score place.
	{
		return;
	}

	for(i = 3; i > place - 1; i--)
	{
		hiscore[i+1] = hiscore[i];
	}

	hiscore[place] = curScore;
}

void hiscoreLogic()
{
	scoreUncoverTimer++;

	if(!score && !toTitleTimer--)
	{
		programStateNew = STATE_TITLE;
	}

	if(keys[KEY_BACK])
	{
		keys[KEY_BACK] = 0;
		programStateNew = STATE_TITLE;
	}

	if(keys[KEY_START])
	{
		keys[KEY_START] = 0;
		programStateNew = STATE_TITLE;
	}

	if(keys[KEY_OK])
	{
		keys[KEY_OK] = 0;
		programStateNew = STATE_TITLE;
	}
}

void hiscoreDraw()
{
	char tableStr[15];
	int rcolor = SDL_MapRGB(screen->format, 255, 255, 0);
	int blinking = 0;
	int i;

	blinkTimersTick();

	clearScreen();

	for(i = 0; i < 5; i++)
	{
		if(!blinking && score == hiscore[i])
		{
			blinking = 1;

			if(blinkTimer > 10 && ((scoreUncoverTimer / 30) >= (5 - i)))
			{
				sprintf(tableStr, "%d. %7d", i+1, hiscore[i]);
				dTextCentered(tableStr, SCREEN_H/2 - 27 + i*13, rcolor);
			}
		}
		else if((scoreUncoverTimer / 30) >= (5 - i))
		{
			sprintf(tableStr, "%d. %7d", i+1, hiscore[i]);
			dTextCentered(tableStr, SCREEN_H/2 - 27 + i*13, rcolor);
		}
	}
}
