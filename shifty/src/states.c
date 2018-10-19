#include "states.h"
#include "game.h"
#include "title.h"

int quit;

State stateActive = STATE_NONE;
State stateNew = STATE_TITLE;

static void checkState()
{
	if (stateActive != stateNew)
	{
		/* Unload active state. */
		switch (stateActive)
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

		/* Load a new state. */
		switch (stateNew)
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

		stateActive = stateNew;
	}
}

void logic()
{
	checkState();

	switch (stateActive)
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

void drawing()
{
	switch (stateActive)
	{
		case STATE_TITLE:
			titleDrawing();
		break;

		case STATE_GAME:
			gameDrawing();
		break;

		default:
		break;
	}
}
