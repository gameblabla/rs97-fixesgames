#include "states.h"

#include "game.h"
#include "title.h"
#include "video.h"

int quit;

State programStateActive = STATE_NONE;
State programStateNew = STATE_TITLE;

void checkState()
{
	if (programStateActive != programStateNew)
	{
		/* Unload current state. */
		switch (programStateActive)
		{
			case STATE_TITLE:
				titleUnload();
			break;
			case STATE_GAME:
				gameUnload();
			break;

			default:
			break;
		}
		/* Load new state. */
		switch (programStateNew)
		{
			case STATE_TITLE:
				titleLoad();
			break;
			case STATE_GAME:
				gameLoad();
			break;

			default:
			break;
		}

		programStateActive = programStateNew;
	}
}

void logic()
{
	checkState();

	switch (programStateActive)
	{
		case STATE_TITLE:
			titleLogic();
		break;
		case STATE_GAME:
			gameLogic();
		break;

		default:
		break;
	}
}

void draw()
{
	clearScreen();

	switch (programStateActive)
	{
		case STATE_TITLE:
			titleDraw();
		break;
		case STATE_GAME:
			gameDraw();
		break;

		default:
		break;
	}

	flipScreen();
}
