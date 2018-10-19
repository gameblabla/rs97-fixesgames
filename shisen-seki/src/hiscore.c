#include "hiscore.h"

#include <stdio.h>
#include "fileio.h"
#include "font.h"
#include "input.h"
#include "states.h"
#include "video.h"

scoreEntry scoreTable[MAX_MODES][MAX_SCORES];
int hiscorePage;
int highlightPage = -1;
int highlightEntry = -1;

void hiscoreUnload()
{
	SDL_FreeSurface(hiscoreBackgroundIMG);
	hiscoreBackgroundIMG = NULL;
	hiscorePage = 0;
	highlightPage = -1;
	highlightEntry = -1;
}

void hiscoreLoad()
{
	if (!hiscoreBackgroundIMG)
	{
		hiscoreBackgroundIMG = loadImage("data/gfx/background.bmp");
	}
}

int hiscoreCheckScore(scoreEntry *entry, gameMode *mode, algorithm *algo)
{
	int i;
	int place = MAX_SCORES;
	int modeId = -1;

	if (entry->time == 0) // Invalid record.
	{
		return -1;
	}

	if (*mode == GAME_MODE_CLASSIC)
	{
		if (*algo == ALGO_RANDOM)
		{
			modeId = 0;
		}
		else if (*algo == ALGO_REVERSE)
		{
			modeId = 1;
		}
	}
	else if (*mode == GAME_MODE_GRAVITY)
	{
		if (*algo == ALGO_RANDOM)
		{
			modeId = 2;
		}
		else if (*algo == ALGO_REVERSE)
		{
			modeId = 3;
		}
	}

	if (modeId < 0 || modeId >= MAX_MODES)
	{
		return -1;
	}

	for (i = MAX_SCORES - 1; i >= 0; --i)
	{
		if ((entry->time < scoreTable[modeId][i].time) || scoreTable[modeId][i].time <= 0)
		{
			place = i;
			continue;
		}
	}

	if (place >= MAX_SCORES) // Score not high enough for hi-score entry.
	{
		return -1;
	}

	return place;
}

void hiscoreAddRecord(scoreEntry *entry, gameMode *mode, algorithm *algo)
{
	int i;
	int place = MAX_SCORES;
	int modeId = -1;

	if (entry->time == 0) // Invalid record.
	{
		return;
	}

	if (*mode == GAME_MODE_CLASSIC)
	{
		if (*algo == ALGO_RANDOM)
		{
			modeId = 0;
		}
		else if (*algo == ALGO_REVERSE)
		{
			modeId = 1;
		}
	}
	else if (*mode == GAME_MODE_GRAVITY)
	{
		if (*algo == ALGO_RANDOM)
		{
			modeId = 2;
		}
		else if (*algo == ALGO_REVERSE)
		{
			modeId = 3;
		}
	}

	if (modeId < 0 || modeId >= MAX_MODES)
	{
		return;
	}

	for (i = MAX_SCORES - 1; i >= 0; --i)
	{
		if ((entry->time < scoreTable[modeId][i].time) || scoreTable[modeId][i].time <= 0)
		{
			place = i;
			continue;
		}
	}

	if (place >= MAX_SCORES) // Score not high enough for hi-score entry.
	{
		return;
	}

	for (i = MAX_SCORES - 2; i >= place; --i)
	{
		scoreTable[modeId][i+1] = scoreTable[modeId][i];
	}
	strcpy(scoreTable[modeId][place].name, entry->name);
	scoreTable[modeId][place].time = entry->time;

	hiscorePage = modeId;
	highlightPage = modeId;
	highlightEntry = place;
}

void hiscoreReset()
{
	int i;
	int j;

	for (i = 0; i < MAX_MODES; ++i)
	{
		for (j = 0; j < MAX_SCORES; ++j)
		{
			scoreTable[i][j].name[0] = '\n';
			scoreTable[i][j].time = 0;
		}
	}

	storeHiscore();
}

void hiscoreLogic()
{
	if (keys[KEY_BACK])
	{
		keys[KEY_BACK] = 0;
		programStateNew = STATE_TITLE;
	}
	if (keys[KEY_CANCEL])
	{
		keys[KEY_CANCEL] = 0;
		programStateNew = STATE_TITLE;
	}

	if(keys[KEY_LEFT])
	{
		keys[KEY_LEFT] = 0;

		if (--hiscorePage < 0)
		{
			hiscorePage = MAX_MODES - 1;
		}
	}

	if(keys[KEY_RIGHT])
	{
		keys[KEY_RIGHT] = 0;

		if (++hiscorePage >= MAX_MODES)
		{
			hiscorePage = 0;
		}
	}
}

void hiscoreDraw()
{
	int i;
	char txtTopBar[100];

	drawImage(hiscoreBackgroundIMG, NULL, screen, 0, 0);
	sprintf(txtTopBar, "<  %d. Mode: %-7.7s  Algo: %-7.7s  >", hiscorePage + 1, hiscorePage < 2 ? "classic" : "gravity", (hiscorePage == 0 || hiscorePage == 2) ? "random" : "reverse");

	dTextCentered(&gameFontRegular, txtTopBar, 20, SHADOW_OUTLINE);

	for (i = 0; i < MAX_SCORES; ++i)
	{
		char txtEntry[100];
		font *curFont = &gameFontRegular;

		if (scoreTable[hiscorePage][i].time > 0)
		{
			sprintf(txtEntry, "%d.  %.3s        %02lu:%02lu:%02lu", i+1, scoreTable[hiscorePage][i].name, scoreTable[hiscorePage][i].time/FPS/3600 > 99 ? 99 : scoreTable[hiscorePage][i].time/FPS/3600, scoreTable[hiscorePage][i].time/FPS/60%60, scoreTable[hiscorePage][i].time/FPS%60);
		}
		else
		{
			sprintf(txtEntry, "%d.  n/a        --:--:--", i+1);
		}

		if (hiscorePage == highlightPage && i == highlightEntry)
		{
			curFont = &gameFontSelected;
		}

		dTextCentered(curFont, txtEntry, 80 + (curFont->h + curFont->leading) * i, SHADOW_DROP);
	}
}
