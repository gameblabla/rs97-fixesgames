#include "game.h"

#include <stdlib.h>
#include "audio.h"
#include "board.h"
#include "fileio.h"
#include "font.h"
#include "hiscore.h"
#include "input.h"
#include "main.h"
#include "states.h"
#include "video.h"

int showAnimations = 1;
int showStoneRank;
int practice;
int continueGame;
gameMode newGameMode = GAME_MODE_CLASSIC;
gameMode currentGameMode;
int canMoveX;
int canMoveY;
unsigned long gameTime;
int gameOver;
scoreEntry hiscoreEntry;
int enteringHiscore;
int scoreCursorPos;

void gameUnload()
{
	unloadSfx(&clearSfx);
	storeBoard();
	boardUnload();
	gameTime = 0;
}

void gameLoad()
{
	clearSfx = loadSfx(clearSfx, "data/sfx/clear.wav");
	boardLoad();
	enteringHiscore = 0;
	fadeOutTimer = 0;
}

void gamePrepareHiscore()
{
	int i;
	scoreCursorPos = 0;

	for (i = 0; i < SCORE_NAME_LEN - 1; ++i)
	{
		hiscoreEntry.name[i] = ' ';
	}

	hiscoreEntry.name[scoreCursorPos] = 'A';
	hiscoreEntry.name[SCORE_NAME_LEN-1] = '\0';
	hiscoreEntry.time = gameTime;

	enteringHiscore = (hiscoreCheckScore(&hiscoreEntry, &currentGameMode, &currentAlgorithm) >= 0) ? 1 : 0;
}

void gameLogic()
{
	if (enteringHiscore)
	{
		if (keys[KEY_BACK])
		{
			keys[KEY_BACK] = 0;
		}

		if (keys[KEY_LEFT])
		{
			if (--canMoveX <= 0)
			{
				canMoveX = KEY_DELAY;
				--scoreCursorPos;
			}
		}
		else if (keys[KEY_RIGHT])
		{
			if (--canMoveX <= 0)
			{
				canMoveX = KEY_DELAY;
				++scoreCursorPos;
			}
		}
		else
		{
			canMoveX = 0;
		}

		if (keys[KEY_UP])
		{
			if (--canMoveY <= 0)
			{
				canMoveY = KEY_DELAY;
				++hiscoreEntry.name[scoreCursorPos];
			}

			if ((hiscoreEntry.name[scoreCursorPos] > ' ') && (hiscoreEntry.name[scoreCursorPos] < 'A'))
			{
				hiscoreEntry.name[scoreCursorPos] = 'A';
			}
			else if ((hiscoreEntry.name[scoreCursorPos] > 'Z') && (hiscoreEntry.name[scoreCursorPos] < 'a'))
			{
				hiscoreEntry.name[scoreCursorPos] = 'a';
			}
			else if (hiscoreEntry.name[scoreCursorPos] > 'z')
			{
				hiscoreEntry.name[scoreCursorPos] = ' ';
			}
		}
		else if (keys[KEY_DOWN])
		{
			if (--canMoveY <= 0)
			{
				canMoveY = KEY_DELAY;
				--hiscoreEntry.name[scoreCursorPos];
			}

			if (hiscoreEntry.name[scoreCursorPos] < ' ')
			{
				hiscoreEntry.name[scoreCursorPos] = 'z';
			}
			else if ((hiscoreEntry.name[scoreCursorPos] > ' ') && (hiscoreEntry.name[scoreCursorPos] < 'A'))
			{
				hiscoreEntry.name[scoreCursorPos] = ' ';
			}
			else if ((hiscoreEntry.name[scoreCursorPos] > 'Z') && (hiscoreEntry.name[scoreCursorPos] < 'a'))
			{
				hiscoreEntry.name[scoreCursorPos] = 'Z';
			}
		}
		else
		{
			canMoveY = 0;
		}

		if (keys[KEY_OK])
		{
			int i;

			for (i = 0; i < SCORE_NAME_LEN - 1; ++i)
			{
				if (hiscoreEntry.name[i] != ' ') // Check if hi-score name isn't empty.
				{
					hiscoreAddRecord(&hiscoreEntry, &currentGameMode, &currentAlgorithm);
					storeHiscore();
					programStateNew = STATE_HISCORE;
					break;
				}
			}
		}

		if (keys[KEY_CANCEL])
		{
			keys[KEY_CANCEL] = 0;
		}

		if (scoreCursorPos < 0)
		{
			scoreCursorPos = 0;
		}
		else if (scoreCursorPos >= SCORE_NAME_LEN - 1)
		{
			scoreCursorPos = SCORE_NAME_LEN - 2;
		}
	}
	else
	{
		if (keys[KEY_BACK])
		{
			keys[KEY_BACK] = 0;

			if (!enteringHiscore)
			{
				programStateNew = STATE_TITLE;
			}
		}

		if (keys[KEY_EXTRA])
		{
			keys[KEY_EXTRA] = 0;

			showStoneRank = !showStoneRank;
		}

		if (!fadeOutTimer)
		{
			if (keys[KEY_LEFT])
			{
				if (--canMoveX <= 0)
				{
					canMoveX = KEY_DELAY;
					--cursorX;
				}
			}
			else if (keys[KEY_RIGHT])
			{
				if (--canMoveX <= 0)
				{
					canMoveX = KEY_DELAY;
					++cursorX;
				}
			}
			else
			{
				canMoveX = 0;
			}

			if (keys[KEY_UP])
			{
				if (--canMoveY <= 0)
				{
					canMoveY = KEY_DELAY;
					--cursorY;
				}
			}
			else if (keys[KEY_DOWN])
			{
				if (--canMoveY <= 0)
				{
					canMoveY = KEY_DELAY;
					++cursorY;
				}
			}
			else
			{
				canMoveY = 0;
			}

			if (keys[KEY_OK])
			{
				keys[KEY_OK] = 0;

				if (gameOver)
				{
					if (!enteringHiscore)
					{
						programStateNew = STATE_TITLE;
					}
				}
				else
				{
					boardSelectStone(cursorX, cursorY);
				}
			}

			if (keys[KEY_CANCEL])
			{
				stoneA.type = STONE_EMPTY;
				keys[KEY_CANCEL] = 0;
			}

			if (mouseMoved)
			{
					int mouseX = (mouse[0] / scale - BOARD_OFFSET_X) / (STONE_W - 1);
					int mouseY = (mouse[1] / scale - BOARD_OFFSET_Y) / (STONE_H - 1);

					if ((mouseX > 0 && mouseX < BOARD_W - 1) && (mouseY > 0 && mouseY < BOARD_H - 1))
					{
						cursorX = mouseX;
						cursorY = mouseY;
					}
			}
		}

		if (cursorX < 1)
		{
			cursorX = BOARD_W - 2;
		}
		if (cursorX >= BOARD_W - 1)
		{
			cursorX = 1;
		}
		if (cursorY < 1)
		{
			cursorY = BOARD_H - 2;
		}
		if (cursorY >= BOARD_H - 1)
		{
			cursorY = 1;
		}

	}

	boardFadeOutSelectedStones();

	if (!gameOver && !fadeOutTimer)
	{
		++gameTime;
	}
}

void gameGuiDraw()
{
	char txtTime[50];

	snprintf(txtTime, 10, "%lu:%02lu:%02lu", gameTime/FPS/3600 > 99 ? 99 : gameTime/FPS/3600, gameTime/FPS/60%60, gameTime/FPS%60);

	if (gameOver && !stonesLeft)
	{
		if (enteringHiscore)
		{
			int i;
			int txtPositionY = SCREEN_H/2 - (gameFontRegular.h + gameFontRegular.leading)/2 * 8;
			int place;
			char txtHiscore[SCORE_NAME_LEN];
			char txtMessage[50];
			char txtMessage2[50];

			place = hiscoreCheckScore(&hiscoreEntry, &currentGameMode, &currentAlgorithm) + 1;

			strcpy(txtHiscore, hiscoreEntry.name);
			sprintf(txtMessage, "Your time of %s", txtTime);
			sprintf(txtMessage2, "gives you the %d%s place.", place, place == 1 ? "st" : (place == 2 ? "nd" : (place == 3 ? "rd" : "th")));

			dTextCentered(&gameFontRegular, "Congratulations!", txtPositionY, SHADOW_OUTLINE);
			dTextCentered(&gameFontRegular, txtMessage, txtPositionY + (gameFontRegular.h + gameFontRegular.leading) * 2, SHADOW_OUTLINE);
			dTextCentered(&gameFontRegular, txtMessage2, txtPositionY + (gameFontRegular.h + gameFontRegular.leading) * 3, SHADOW_OUTLINE);
			dTextCentered(&gameFontRegular, "Enter your initials:", txtPositionY + (gameFontRegular.h + gameFontRegular.leading) * 5, SHADOW_OUTLINE);

			dTextCentered(&gameFontRegular, txtHiscore, txtPositionY + (gameFontRegular.h + gameFontRegular.leading) * 7, SHADOW_OUTLINE);

			for (i = 0; i < SCORE_NAME_LEN - 1; ++i)
			{
				txtHiscore[i] = (i == scoreCursorPos) ? (hiscoreEntry.name[scoreCursorPos] == ' ' ? '_' : hiscoreEntry.name[scoreCursorPos]) : ' ';
			}
			txtHiscore[SCORE_NAME_LEN-1] = '\0';

			dTextCentered(&gameFontSelected, txtHiscore, txtPositionY + (gameFontSelected.h + gameFontSelected.leading) * 7, SHADOW_OUTLINE);
		}
		else
		{
			int txtPositionY = SCREEN_H/2 - (gameFontRegular.h + gameFontRegular.leading)/2 - (gameFontRegular.h + gameFontRegular.leading);
			char txtMessage[50];

			sprintf(txtMessage, "Your time: %s\n", txtTime);

			dTextCentered(&gameFontRegular, "Well done!", txtPositionY, SHADOW_OUTLINE);
			dTextCentered(&gameFontRegular, txtMessage, txtPositionY + (gameFontRegular.h + gameFontRegular.leading) * 2, SHADOW_OUTLINE);

			if (practice)
			{
				dTextCentered(&gameFontSelected, "Games in practice mode", txtPositionY + (gameFontSelected.h + gameFontSelected.leading) * 4, SHADOW_OUTLINE);
				dTextCentered(&gameFontSelected, "are excluded from score table!", txtPositionY + (gameFontSelected.h + gameFontSelected.leading) * 5, SHADOW_OUTLINE);
			}
		}
	}
	else
	{
		char txtStones[50];
		char txtGameOver[50];
		char txtBottomBar[100];

		sprintf(txtStones, "Stones Left: %d", stonesLeft);
		sprintf(txtBottomBar, "Time: %s  Stones Left: %d", txtTime, stonesLeft);

		strcpy(txtGameOver, "No moves left!");
		dTextCentered(&gameFontShadow, (gameOver && stonesLeft) ? txtGameOver : txtBottomBar, SCREEN_H - (gameFontShadow.h + gameFontShadow.leading), SHADOW_NONE);
	}
}

void gameDraw()
{
	boardDraw();
	gameGuiDraw();
}
