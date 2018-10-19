#include "title.h"
#include "backend/input.h"
#include "backend/video.h"
#include "font.h"
#include "game.h"
#include "menu.h"
#include "network.h"
#include "player.h"
#include "states.h"

static int *keyMap[KEY_COUNT];
static Menu *menu;
static Image *backgroundGfx;
static int showCredits;

void mainMenuCreditsAction()
{
	showCredits = !showCredits;
}

void mainMenuExitAction()
{
	quit = 1;
}

void gameModeSoloAction()
{
	game.mode = GAME_MODE_SOLO;
	game.modifiers.shiftEvent = 1;
	game.modifiers.junkBlocks = 0;
	game.modifiers.debugInfo = 0;
	player1.ai.enabled = 0;

	stateNew = STATE_GAME;
}

void gameModeDemoAction()
{
	game.mode = GAME_MODE_LOCAL;
	game.modifiers.shiftEvent = 1;
	game.modifiers.junkBlocks = 1;
	game.modifiers.debugInfo = 0;
	player1.ai.enabled = 1;
	player2.ai.enabled = 1;

	stateNew = STATE_GAME;
}

void battleModeAIAction()
{
	game.mode = GAME_MODE_LOCAL;
	game.modifiers.shiftEvent = 1;
	game.modifiers.junkBlocks = 1;
	game.modifiers.debugInfo = 0;
	player1.ai.enabled = 0;
	player2.ai.enabled = 1;

	stateNew = STATE_GAME;
}

void battleModeLocalAction()
{
	game.mode = GAME_MODE_LOCAL;
	game.modifiers.shiftEvent = 1;
	game.modifiers.junkBlocks = 1;
	game.modifiers.debugInfo = 0;
	player1.ai.enabled = 0;
	player2.ai.enabled = 0;

	stateNew = STATE_GAME;
}

void networkModeHostAction()
{
	netConnection.type = NET_CONNECTION_HOST;
	networkListen();
}

void networkModeJoinAction()
{
	netConnection.type = NET_CONNECTION_CLIENT;
	networkConnect();
}

void trainingModeStartAction()
{
	game.mode = GAME_MODE_SOLO;

	game.modifiers.shiftEvent = trainingModeMenuShiftTarget;
	game.modifiers.junkBlocks = trainingModeMenuJunkTarget;
	game.modifiers.debugInfo = trainingModeMenuDebugTarget;
	player1.ai.enabled = 0;

	stateNew = STATE_GAME;
}

void optionsMenuScaleChoice(unsigned int *choiceSel)
{
	screenScale = *choiceSel + 1;
	updateScale(screenScale);
}

void optionsMenuFullscreenChoice(unsigned int *choiceSel)
{
	fullscreen = *choiceSel;
	updateScale(screenScale);
}

void optionsMenuFPSChoice(unsigned int *choiceSel)
{
	showFps = *choiceSel;
}

void titleLoad()
{
	if (!(backgroundGfx = loadImage("data/bg02.bmp")))
		goto error;

	defaultKeymap(keyMap, 1);

	menu = &mainMenu;

	optionsMenuFullscreenTarget = fullscreen;
	optionsMenuScaleTarget = screenScale - 1;
	optionsMenuFPSTarget = showFps;
	return;

	error:
		titleUnload();
		fprintf(stderr, "Missing game assets.\n");
		exit(1);
}

void titleUnload()
{
	unloadImage(backgroundGfx);
}

void titleLogic()
{
	if (showCredits)
	{
		if (*keyMap[KEY_BACK])
		{
			*keyMap[KEY_BACK] = 0;

			showCredits = 0;
		}
	}
	else if (netConnection.status == NET_STATUS_LISTENING || netConnection.status == NET_STATUS_CONNECTING)
	{
		if (*keyMap[KEY_BACK])
		{
			*keyMap[KEY_BACK] = 0;

			networkCloseConnection();
		}
	}
	else
	{
		menuLogic(&menu, keyMap);
	}

	switch (netConnection.status)
	{
		case NET_STATUS_CONNECTED:
			networkSendPacket(NET_PACKET_VERSION);
		
			game.mode = GAME_MODE_NETWORKED;
			game.modifiers.shiftEvent = 1;
			game.modifiers.junkBlocks = 1;
			game.modifiers.debugInfo = 0;
			player1.ai.enabled = 0;
			player2.ai.enabled = 0;

			stateNew = STATE_GAME;
		break;
		case NET_STATUS_LISTENING:
		case NET_STATUS_CONNECTING:
			if (++netConnection.timer > NET_CONNECTION_TIMEOUT)
			{
				netConnection.timer = 0;
				++netConnection.attempt;

				if (netConnection.attempt <= NET_CONNECTION_ATTEMPTS_MAX)
				{
					if (netConnection.type == NET_CONNECTION_HOST)
					{
						networkListen();
					}
					else
					{
						networkConnect();
					}
				}
				else
				{
					memset(&netConnection, 0, sizeof(NetConnection));
					netConnection.status = NET_STATUS_DISCONNECTED;
				}
			}
		break;
	
		default:
		break;
	}
}

void titleDrawing()
{
	clearScreen();

	drawImage(backgroundGfx, NULL, 0, 0);

	if (showCredits)
	{
		int y = 2;

		dTextCentered(fontDefault, SHADOW_OUTLINE, "Credits", 50);
		dTextCentered(fontDefault, SHADOW_OUTLINE, "Game concept", 50 + (fontDefault->h + fontDefault->leading)*y++);
		dTextCentered(fontDefault, SHADOW_OUTLINE, "Programming", 50 + (fontDefault->h + fontDefault->leading)*y++);
		dTextCentered(fontDefault, SHADOW_DROP, "Artur Rojek (zear)", 50 + (fontDefault->h + fontDefault->leading)*y++);
		++y;
		dTextCentered(fontDefault, SHADOW_OUTLINE, "Graphics", 50 + (fontDefault->h + fontDefault->leading)*y++);
		dTextCentered(fontDefault, SHADOW_DROP, "vergeofapathy", 50 + (fontDefault->h + fontDefault->leading)*y++);
	}
	else if (netConnection.status == NET_STATUS_LISTENING || netConnection.status == NET_STATUS_CONNECTING)
	{
		char connectionTxt[50];
		char attemptsTxt[50];

		if (netConnection.status == NET_STATUS_CONNECTING)
		{
			snprintf(connectionTxt, sizeof(connectionTxt), "Connecting to %s:%d", hostAddress, NET_PORT);
			snprintf(attemptsTxt, sizeof(attemptsTxt), "Attempt: %d/%d\n", netConnection.attempt, NET_CONNECTION_ATTEMPTS_MAX);
		}
		else
		{
			snprintf(connectionTxt, sizeof(connectionTxt), "Listening at %s:%d", hostAddress, NET_PORT);
			snprintf(attemptsTxt, sizeof(attemptsTxt), "Attempt: %d/%d\n", netConnection.attempt, NET_CONNECTION_ATTEMPTS_MAX);
		}

		dTextCentered(fontDefault, SHADOW_OUTLINE, connectionTxt, 50);
		dTextCentered(fontDefault, SHADOW_OUTLINE, attemptsTxt, 50 + (fontDefault->h + fontDefault->leading)*2);
	}
	else
	{
		menuDisplay(menu, 120, 80);
	}

	flipScreen();
}
