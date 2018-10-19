#include <SDL/SDL.h>
#include "apricots.h"
#include "init.h"
#include "menu.h"

MenuContainer *CurrentMenu = NULL;
MenuItem *SelectedItem = NULL;

MenuContainer *MenuMain = NULL;
MenuContainer *MenuMainIntro = NULL;
MenuContainer *MenuOptions = NULL;
MenuContainer *MenuGameOptions = NULL;

int planeArray[6][3];
int cursorX = 0;
int cursorY = 0;
int *p1plane = NULL;
int *p2plane = NULL;
int p1sel = 0;
int p2sel = 0;

MenuContainer *menuCreateNew(MenuContainer *Container, int number, const char *caption, int value, MenuAction Action)
{
	MenuItem *NewItem = NULL;

	if(Container == NULL)
	{
		Container = (MenuContainer *)malloc(sizeof(MenuContainer));
		if(Container == NULL)
		{
			fprintf(stderr, "ERROR: menuCreateNew: Out of memory!\n");
			return NULL;
		}
		Container->Menu = NULL;
		Container->size = 0;
	}

	NewItem = (MenuItem *)malloc(sizeof(MenuItem));
	if(NewItem == NULL)
	{
		fprintf(stderr, "ERROR: menuCreateNew: Out of memory!\n");
		return Container; // TODO: instead of returning destroy the container
	}

	NewItem->number = number;
	NewItem->value = value;
	NewItem->Action = Action;
	strcpy(NewItem->caption, caption);

	NewItem->Next = Container->Menu;
	Container->Menu = NewItem;
	Container->size++;

	return Container;
}

MenuItem *menuSwitchItem(MenuContainer *Container, int number)
{
	MenuItem *NewItem = Container->Menu;

	while(NewItem != NULL)
	{
		if(NewItem->number == number)
		{
			return NewItem;
		}

		NewItem = NewItem->Next;
	}

	return NULL;
}

void menuAction(MenuItem *Item, gamedata &g)
{
	switch(Item->Action)
	{
		case ACTION_NEW_GAME:
			for(int i = 0; i < 6; i++)
			{
				if(p1plane == &planeArray[i][1])
					if((i + 1) > newg.planes)
					{
						*p1plane = 0;
						p1plane = NULL;
						GameState = STATE_MENU_PLANE_SELECT;
						return;
					}

				if(p2plane == &planeArray[i][1])
					if((i + 1) > newg.planes)
					{
						*p2plane = 0;
						p2plane = NULL;
						GameState = STATE_MENU_PLANE_SELECT;
						return;
					}
			}

			if(SDL_NumJoysticks() > 1)
			{
				if(g.playerJoy[0] == -1 && g.playerJoy[1] == -1)
				{
					GameState = STATE_MENU_PLAYER_CONTROL_SELECT;
					return;
				}
			}
			else if(SDL_NumJoysticks() == 1)
			{
				if(g.playerJoy[0] == -1) // select GCW's built-in joystick for P1
				{
					g.playerJoy[0] = 0;
					g.playerJoyBut[0][0] = 0;
					g.playerJoyBut[0][1] = 1;
					g.playerJoyBut[0][2] = 2;
				}
			}

			finish_game(g);
			//init_data(g);

			g.players = newg.players;
			g.planes = newg.planes;
			g.mission = newg.mission;
			g.targetscore = newg.targetscore;
			g.towers = newg.towers;
			g.guns = newg.guns;
			g.buildings = newg.buildings;
			g.trees = newg.trees;
			g.drakoption = newg.drakoption;
			g.scoreBarPos = newg.scoreBarPos;

			for(int i = 1; i < 7; i++)
			{
				g.planeinfo[i].planetype = newg.planeinfo[i].planetype;
				g.planeinfo[i].control = newg.planeinfo[i].control;
				g.planeinfo[i].basetype = newg.planeinfo[i].basetype;
			}

			setup_game(g);
			GameState = STATE_GAME;
			demo = false;
			// save the config
			saveConfig(ap_home, "apricots.cfg", g);
			break;
		case ACTION_OPTIONS:
			CurrentMenu = MenuOptions;
			SelectedItem = menuSwitchItem(CurrentMenu, 0);
			break;
		case ACTION_QUIT:
			quit = true;
			break;
		case ACTION_BACK_TO_GAME:
			GameState = STATE_GAME;
			break;
		case ACTION_OPTIONS_PLANE_SETTINGS:
			GameState = STATE_MENU_PLANE_SELECT;
			break;
		case ACTION_OPTIONS_GAME_SETTINGS:
			CurrentMenu = MenuGameOptions;
			SelectedItem = menuSwitchItem(CurrentMenu, 0);
			break;
		case ACTION_OPTIONS_BACK:
			CurrentMenu = MenuMain;
			SelectedItem = menuSwitchItem(CurrentMenu, 0);
			break;
		case ACTION_GAME_OPTIONS_BACK:
			CurrentMenu = MenuMain;
			SelectedItem = menuSwitchItem(CurrentMenu, 0);
			break;
		case ACTION_SET_NUM_OF_PLANES:
			{
				menuActionChangeValue(SelectedItem, 1, g);
			}
			break;
		case ACTION_SET_MISSION:
			{
				menuActionChangeValue(SelectedItem, 1, g);
			}
			break;
		case ACTION_SET_SCORE:
			{
				menuActionChangeValue(SelectedItem, 1, g);
			}
			break;
		case ACTION_SET_TOWERS:
			{
				menuActionChangeValue(SelectedItem, 1, g);
			}
			break;
		case ACTION_SET_GUNS:
			{
				menuActionChangeValue(SelectedItem, 1, g);
			}
			break;
		case ACTION_SET_BUILDINGS:
			{
				menuActionChangeValue(SelectedItem, 1, g);
			}
			break;
		case ACTION_SET_TREES:
			{
				menuActionChangeValue(SelectedItem, 1, g);
			}
			break;
		case ACTION_SET_DRAK:
			{
				menuActionChangeValue(SelectedItem, 1, g);
			}
			break;
		case ACTION_SET_SCOREBAR_POS:
			{
				menuActionChangeValue(SelectedItem, 1, g);
			}
			break;
		default:
			break;
	}
}

void menuActionChangeValue(MenuItem *Item, int value, gamedata &g)
{
	switch(Item->Action)
	{
		case ACTION_SET_NUM_OF_PLANES:
			{
				Item->value += value;
				if(Item->value < 2)
					Item->value = 6;
				if(Item->value > 6)
					Item->value = 2;

				newg.planes = Item->value;
			}
			break;
		case ACTION_SET_MISSION:
			{
				Item->value += value;
				if(Item->value < 0)
					Item->value = 2;
				if(Item->value > 2)
					Item->value = 0;

				newg.mission = Item->value;
			}
			break;
		case ACTION_SET_SCORE:
			{
				Item->value += value * 100;
				if(Item->value < 100)
					Item->value = 5000;
				if(Item->value > 5000)
					Item->value = 100;

				newg.targetscore = Item->value;
			}
			break;
		case ACTION_SET_TOWERS:
			{
				Item->value += value;
				if(Item->value < 0)
					Item->value = 30;
				if(Item->value > 30)
					Item->value = 0;

				newg.towers = Item->value;
			}
			break;
		case ACTION_SET_GUNS:
			{
				Item->value += value;
				if(Item->value < 0)
					Item->value = 20;
				if(Item->value > 20)
					Item->value = 0;

				newg.guns = Item->value;
			}
			break;
		case ACTION_SET_BUILDINGS:
			{
				Item->value += value;
				if(Item->value < 0)
					Item->value = 50;
				if(Item->value > 50)
					Item->value = 0;

				newg.buildings = Item->value;
			}
			break;
		case ACTION_SET_TREES:
			{
				Item->value += value;
				if(Item->value < 0)
					Item->value = 100;
				if(Item->value > 100)
					Item->value = 0;

				newg.trees = Item->value;
			}
			break;
		case ACTION_SET_DRAK:
			{
				Item->value += value;
				if(Item->value < 0)
					Item->value = 2;
				if(Item->value > 2)
					Item->value = 0;

				newg.drakoption = Item->value;
			}
			break;
		case ACTION_SET_SCOREBAR_POS:
			{
				Item->value += value;
				if(Item->value < 0)
					Item->value = 1;
				if(Item->value > 1)
					Item->value = 0;

				g.scoreBarPos = Item->value;
				newg.scoreBarPos = Item->value;
			}
			break;
		default:
			break;
	}
}

void menuDeleteSingle(MenuContainer *Container)
{
	MenuItem *CurrentItem;

	if(Container == NULL)
	{
		return;
	}

	CurrentItem = Container->Menu;
	while(CurrentItem != NULL)
	{
		Container->Menu = CurrentItem->Next;
		free(CurrentItem);
		CurrentItem = Container->Menu;
	}
}

void menuDeleteAll()
{
	menuDeleteSingle(MenuMain);
}

void menuLoadAll(gamedata &g)
{
	MenuMain = menuCreateNew(MenuMain, 0, "New game", -1, ACTION_NEW_GAME);
	MenuMain = menuCreateNew(MenuMain, 1, "Options", -1, ACTION_OPTIONS);
	MenuMain = menuCreateNew(MenuMain, 2, "Quit", -1, ACTION_QUIT);
	MenuMain = menuCreateNew(MenuMain, 3, "", -1, ACTION_NONE);
	MenuMain = menuCreateNew(MenuMain, 4, "Back to game", -1, ACTION_BACK_TO_GAME);

	MenuMainIntro = menuCreateNew(MenuMainIntro, 0, "New game", -1, ACTION_NEW_GAME);
	MenuMainIntro = menuCreateNew(MenuMainIntro, 1, "Options", -1, ACTION_OPTIONS);
	MenuMainIntro = menuCreateNew(MenuMainIntro, 2, "Quit", -1, ACTION_QUIT);

	MenuOptions = menuCreateNew(MenuOptions, 0, "Plane settings", -1, ACTION_OPTIONS_PLANE_SETTINGS);
	MenuOptions = menuCreateNew(MenuOptions, 1, "Game settings", -1, ACTION_OPTIONS_GAME_SETTINGS);
	MenuOptions = menuCreateNew(MenuOptions, 2, "", -1, ACTION_NONE);
	MenuOptions = menuCreateNew(MenuOptions, 3, "Back", -1, ACTION_OPTIONS_BACK);

	MenuGameOptions = menuCreateNew(MenuGameOptions, 0, "Number of planes:", newg.planes, ACTION_SET_NUM_OF_PLANES);
	MenuGameOptions = menuCreateNew(MenuGameOptions, 1, "Mission:", newg.mission, ACTION_SET_MISSION);
	MenuGameOptions = menuCreateNew(MenuGameOptions, 2, "Target score (missions 0-1):", newg.targetscore, ACTION_SET_SCORE);
	MenuGameOptions = menuCreateNew(MenuGameOptions, 3, "Number of towerblocks:", newg.towers, ACTION_SET_TOWERS);
	MenuGameOptions = menuCreateNew(MenuGameOptions, 4, "Number of neutral guns:", newg.guns, ACTION_SET_GUNS);
	MenuGameOptions = menuCreateNew(MenuGameOptions, 5, "Number of buildings:", newg.buildings, ACTION_SET_BUILDINGS);
	MenuGameOptions = menuCreateNew(MenuGameOptions, 6, "Maximum number of trees:", newg.trees, ACTION_SET_TREES);
	MenuGameOptions = menuCreateNew(MenuGameOptions, 7, "Does the Drak show up:", newg.drakoption, ACTION_SET_DRAK);
	MenuGameOptions = menuCreateNew(MenuGameOptions, 8, "Score Bar position:", newg.scoreBarPos, ACTION_SET_SCOREBAR_POS);
	MenuGameOptions = menuCreateNew(MenuGameOptions, 9, "", -1, ACTION_NONE);
	MenuGameOptions = menuCreateNew(MenuGameOptions, 10, "Back", -1, ACTION_GAME_OPTIONS_BACK);

	CurrentMenu = MenuMainIntro;
	SelectedItem = menuSwitchItem(CurrentMenu, 0);
}

void menuInput(gamedata &g)
{
	Uint8 *keys = SDL_GetKeyState(NULL);
	MenuItem *NewItem = NULL;
	int newItemNumber = SelectedItem->number;

	    if (keys[SDLK_ESCAPE] == SDL_PRESSED){
		keys[SDLK_ESCAPE] = SDL_RELEASED;
	      //quit = true;
	    }

	if(keys[SDLK_UP] == SDL_PRESSED)
	{
		keys[SDLK_UP] = SDL_RELEASED;
		newItemNumber--;
	}
	else if(keys[SDLK_DOWN] == SDL_PRESSED)
	{
		keys[SDLK_DOWN] = SDL_RELEASED;
		newItemNumber++;
	}

	if(newItemNumber < 0)
	{
		newItemNumber = CurrentMenu->size - 1;
	}

	if(newItemNumber >= CurrentMenu->size)
	{
		newItemNumber = 0;
	}

	if(newItemNumber != SelectedItem->number)
	{
		NewItem = menuSwitchItem(CurrentMenu, newItemNumber);
		if(NewItem != NULL)
		{
			while(NewItem->Action == ACTION_NONE)
			{
				if(NewItem->number < SelectedItem->number)
				{
					newItemNumber--;
				}
				else if(NewItem->number > SelectedItem->number)
				{
					newItemNumber++;
				}
				else
				{
					return;
				}

				NewItem = menuSwitchItem(CurrentMenu, newItemNumber);
			}

			SelectedItem = NewItem;
		}
	}

	if(keys[SDLK_LEFT] == SDL_PRESSED)
	{
		keys[SDLK_LEFT] = SDL_RELEASED;
		menuActionChangeValue(SelectedItem, -1, g);
	}
	else if(keys[SDLK_RIGHT] == SDL_PRESSED)
	{
		keys[SDLK_RIGHT] = SDL_RELEASED;
		menuActionChangeValue(SelectedItem, 1, g);
	}
	else if(keys[SDLK_RETURN] == SDL_PRESSED)
	{
		keys[SDLK_RETURN] = SDL_RELEASED;
		menuAction(SelectedItem, g);
	}
	else if(keys[SDLK_LCTRL] == SDL_PRESSED)
	{
		keys[SDLK_LCTRL] = SDL_RELEASED;
		menuAction(SelectedItem, g);
	}
}

void menuDrawSingle(MenuContainer *Container, int number, int x, int y, gamedata &g)
{
	MenuItem *CurrentItem = Container->Menu;

	if(number < 0 || number > Container->size)
	{
		fprintf(stderr, "ERROR (menuDraw): No such menu: %d\n", number);
		return;
	}

	while(CurrentItem != NULL)
	{
		if(CurrentItem->number == number)
		{
			g.whitefont.write(g.virtualscreen, x, y, CurrentItem->caption);
			if(CurrentItem->value != -1)
			{
				char valueText[10];
				if(Container == MenuGameOptions && CurrentItem->number == 7) // drak item
				{
					if(CurrentItem->value == 2)
					{
						sprintf(valueText, "Always");
						g.whitefont.write(g.virtualscreen, x + 250, y, valueText);
					}
					else if(CurrentItem->value == 1)
					{
						sprintf(valueText, "Sometimes");
						g.whitefont.write(g.virtualscreen, x + 225, y, valueText);
					}
					else
					{
						sprintf(valueText, "Never");
						g.whitefont.write(g.virtualscreen, x + 250, y, valueText);
					}
				}
				else if(Container == MenuGameOptions && CurrentItem->number == 8) // ScoreBar item
				{
					if(CurrentItem->value)
						sprintf(valueText, "Bottom");
					else
						sprintf(valueText, "Top");

					g.whitefont.write(g.virtualscreen, x + 250, y, valueText);
				}
				else
				{
					sprintf(valueText, "%d", CurrentItem->value);
					g.whitefont.write(g.virtualscreen, x + 250, y, valueText);
				}
			}

			if(CurrentItem == SelectedItem)
			{
				g.whitefont.write(g.virtualscreen, x - 20, y, "*");
			}
			break;
		}

		CurrentItem = CurrentItem->Next;
	}
}

void menuDraw(MenuContainer *Container, int x, int y, gamedata &g)
{
	int i;

	for(i = 0; i < CurrentMenu->size; i++)
	{
		menuDrawSingle(CurrentMenu, i, x, y + i * 10, g);
	}
}

void menuPlanes(gamedata &g)
{
	Uint8 *keys = SDL_GetKeyState(NULL);

	if(keys[SDLK_UP] == SDL_PRESSED)
	{
		keys[SDLK_UP] = SDL_RELEASED;
		cursorY--;
	}
	else if(keys[SDLK_DOWN] == SDL_PRESSED)
	{
		keys[SDLK_DOWN] = SDL_RELEASED;
		cursorY++;
	}
	else if(keys[SDLK_LEFT] == SDL_PRESSED)
	{
		keys[SDLK_LEFT] = SDL_RELEASED;
		cursorX--;
	}
	else if(keys[SDLK_RIGHT] == SDL_PRESSED)
	{
		keys[SDLK_RIGHT] = SDL_RELEASED;
		cursorX++;
	}
	else if(keys[SDLK_ESCAPE] == SDL_PRESSED || keys[SDLK_LALT] == SDL_PRESSED)
	{
		if(keys[SDLK_ESCAPE] == SDL_PRESSED)
			keys[SDLK_ESCAPE] = SDL_RELEASED;
		if(keys[SDLK_LALT] == SDL_PRESSED)
			keys[SDLK_LALT] = SDL_RELEASED;

		if(p1plane != NULL)
		{
			int leave = 1;

			for(int i = 0; i < 6; i++)
			{	
				if(p1plane == &planeArray[i][1])
					if((i + 1) > newg.planes)
					{
						*p1plane = 0;
						p1plane = NULL;
						leave = 0;
					}

				if(p2plane == &planeArray[i][1])
					if((i + 1) > newg.planes)
					{
						*p2plane = 0;
						p2plane = NULL;
					}

				newg.planeinfo[i + 1].control = planeArray[i][1];
			}

			if(leave)
			{
				if(p2plane != NULL)
					newg.players = 2;
				else
					newg.players = 1;

				if(demo)
					GameState = STATE_INTRO;
				else
					GameState = STATE_MENU;
				return;
			}
		}
	}
	else if(keys[SDLK_RETURN] == SDL_PRESSED || keys[SDLK_LCTRL] == SDL_PRESSED)
	{
		if(keys[SDLK_RETURN] == SDL_PRESSED)
			keys[SDLK_RETURN] = SDL_RELEASED;
		if(keys[SDLK_LCTRL] == SDL_PRESSED)
			keys[SDLK_LCTRL] = SDL_RELEASED;

		planeArray[cursorY][cursorX]++;

		if(cursorX == 0 || cursorX == 1)
			if(planeArray[cursorY][cursorX] > 2)
				planeArray[cursorY][cursorX] = 0;
		if(cursorX == 2)
			if(planeArray[cursorY][cursorX] > 6)
				planeArray[cursorY][cursorX] = 0;

		switch(cursorX)
		{
			case 0: // plane select
				newg.planeinfo[cursorY + 1].planetype = planeArray[cursorY][cursorX] + 1;
			break;
			case 1: // control select
				if(planeArray[cursorY][cursorX] == 0)
				{
					if(p1plane == &planeArray[cursorY][cursorX])
						p1plane = NULL;
					if(p2plane == &planeArray[cursorY][cursorX])
						p2plane = NULL;
				}
				else if(planeArray[cursorY][cursorX] == 1)
				{
					if(p1plane != NULL && p1plane != &planeArray[cursorY][cursorX])
						*p1plane = 0;
					p1plane = &planeArray[cursorY][cursorX];
				}
				else if(planeArray[cursorY][cursorX] == 2)
				{
					if(p2plane != NULL && p2plane != &planeArray[cursorY][cursorX])
						*p2plane = 0;

					p2plane = &planeArray[cursorY][cursorX];
					if(p2plane == p1plane)
						p1plane = NULL;
				}

				newg.planeinfo[cursorY + 1].control = planeArray[cursorY][cursorX];
			break;
			case 2: // base select
				newg.planeinfo[cursorY + 1].basetype = planeArray[cursorY][cursorX] + 1;

			break;

			default:
			break;
		}
	}

	if(cursorX < 0)
		cursorX = 2;
	if(cursorX > 2)
		cursorX = 0;

	if(cursorY < 0)
		cursorY = newg.planes - 1;

	if(cursorY >= newg.planes)
		cursorY = 0;
}

void menuPlanesDraw(int x, int y, gamedata &g)
{
	int selPos = 0;
	int vertDist = 20;
	int horizDist = 60;
	int horizDist2 = 140;

	// draw the menu

	g.whitefont.write(g.virtualscreen, x, y, "Plane:");
	g.whitefont.write(g.virtualscreen, x + horizDist, y, "Player:");
	g.whitefont.write(g.virtualscreen, x + horizDist2, y, "Base:");

	for(int j = 0; j < newg.planes; j++)
	{
		for(int i = 0; i < 3; i++)
		{
			if(i == 0)
			{
				int plane = 135;
				if(planeArray[j][i] == 0)
					plane = 135;
				else if(planeArray[j][i] == 1)
					plane = 89;
				else if(planeArray[j][i] == 2)
					plane = 187;

				g.images[plane].blit(g.virtualscreen, x, y + j * 30 + vertDist);
			}
			else if(i == 1)
			{
				char text[5];
				if(planeArray[j][i] == 0)
					sprintf(text, "CPU");
				else if(planeArray[j][i] == 1)
					sprintf(text, "P1");
				else if(planeArray[j][i] == 2)
					sprintf(text, "P2");

				g.whitefont.write(g.virtualscreen, x + horizDist, y + j * 30 + vertDist, text);
			}
			else if(i == 2)
			{
				char text[15];
				if(planeArray[j][i] == 0)
					sprintf(text, "1) Standard");
				else if(planeArray[j][i] == 1)
					sprintf(text, "2) Reversed");
				else if(planeArray[j][i] == 2)
					sprintf(text, "3) Little");
				else if(planeArray[j][i] == 3)
					sprintf(text, "4) Long");
				else if(planeArray[j][i] == 4)
					sprintf(text, "5) Original");
				else if(planeArray[j][i] == 5)
					sprintf(text, "6) Shooty");
				else if(planeArray[j][i] == 6)
					sprintf(text, "7) Twogun");

				g.whitefont.write(g.virtualscreen, x + horizDist2, y + j * 30 + vertDist, text);
			}
		}
	}

	if(cursorX == 0)
		selPos = 0;
	else if(cursorX == 1)
		selPos = horizDist;
	else if(cursorX == 2)
		selPos = horizDist2;

	g.whitefont.write(g.virtualscreen, x + selPos - 10, y + cursorY * 30 + vertDist, "*");
}

void loadPlaneInfo(gamedata &g)
{
	for(int j = 0; j < 6; j++)
	{
		for(int i = 0; i < 3; i++)
		{
			switch(i)
			{
				case 0: // plane select
					planeArray[j][i] = newg.planeinfo[j + 1].planetype - 1;
				break;
				case 1: // control select
					planeArray[j][i] = newg.planeinfo[j + 1].control;
					if(planeArray[j][i] == 1)
						p1plane = &planeArray[j][i];
					else if(planeArray[j][i] == 2)
						p2plane = &planeArray[j][i];
				break;
				case 2: // base select
					planeArray[j][i] = newg.planeinfo[j + 1].basetype - 1;
				break;

				default:
				break;
			}
		}
	}
}

void menuSelectPlayer(gamedata &g)
{
	int numOfJoys = SDL_NumJoysticks();

	if(numOfJoys > 1)
	{
		static int allJoys[10];
		static int allowMove[10];
		static int selectInput[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		static int done[2] = {0, 0};

		if(numOfJoys > 9)
			numOfJoys = 9;

		if(done[0])
		{
			if(done[1] || newg.players == 1)
			{
				done[0] = 0;
				done[1] = 0;
				for(int i = 0; i < 10; i++)
				{
					selectInput[i] = 0;
				}

				GameState = STATE_MENU;
				menuAction(menuSwitchItem(MenuMain, 0), g);
			}
		}
			

		for(int i = 0; i < numOfJoys; i++)
		{
			SDL_Joystick *joy = SDL_JoystickOpen(i);
			int deadzone = 250;
			int y;

			if(selectInput[i])
			{
				int pnum;
				if(allJoys[i] == -1)
					pnum = 0;
				else
					pnum = 1;

				if(!allowMove[i])
				{
					int change = 1;

					for(int j = 0; j < SDL_JoystickNumButtons(joy); j++)
					{
						if(SDL_JoystickGetButton(joy, j))
							change = 0;
					}
					if(change)
						allowMove[i] = 1;
				}

				if(i == 0) // joystick is GCW0, skip button mapping
				{
					if(pnum == 0)
						done[0] = 1;
					else if(pnum == 1)
						done[1] = 1;
				}
				else
				{
					if(g.playerJoyBut[pnum][0] == -1 && allowMove[i])
					{
						for(int j = 0; j < SDL_JoystickNumButtons(joy); j++)
						{
							if(SDL_JoystickGetButton(joy, j))
							{
								g.playerJoyBut[pnum][0] = j;
								allowMove[i] = 0;
							}
						}
					}
					else if(g.playerJoyBut[pnum][1] == -1 && allowMove[i])
					{
						for(int j = 0; j < SDL_JoystickNumButtons(joy); j++)
						{
							if(SDL_JoystickGetButton(joy, j))
							{
								g.playerJoyBut[pnum][1] = j;
								allowMove[i] = 0;
								if(pnum == 1)
									done[1] = 1;
							}
						}
					}
					else if(allJoys[i] == -1 && allowMove[i])
					{
						if(g.playerJoyBut[0][2] == -1)
						{
							for(int j = 0; j < SDL_JoystickNumButtons(joy); j++)
							{
								if(SDL_JoystickGetButton(joy, j))
								{
									g.playerJoyBut[0][2] = j;
									allowMove[i] = 0;
									done[0] = 1;
								}
							}
						}
					}
				}
			}
			else
			{
				y = SDL_JoystickGetAxis(joy, 1);

				if(y < -deadzone)
				{
					if(allowMove[i])
					{
						allJoys[i]--;
						allowMove[i] = 0;
					}
				}
				else if(y > deadzone)
				{
					if(allowMove[i])
					{
						allJoys[i]++;
						allowMove[i] = 0;
					}
				}
				else
					if(i == 0) // joystick is GCW0, allow selection with keyboard 
					{
						Uint8 *key = SDL_GetKeyState(NULL);

						if(key[SDLK_UP])
						{
							if(allowMove[i])
							{
								allJoys[i]--;
								allowMove[i] = 0;
							}
						}
						else if(key[SDLK_DOWN])
						{
							if(allowMove[i])
							{
								allJoys[i]++;
								allowMove[i] = 0;
							}
						}
						else
							allowMove[i] = 1;
					}
					else
						allowMove[i] = 1;

				if(allJoys[i] < 0)
					allJoys[i] = -1;
				else if(allJoys[i] > 0)
				{
					if(newg.players == 2)
						allJoys[i] = 1;
					else
						allJoys[i] = 0;
				}

				if(allJoys[i] < 0)
				{
					if(g.playerJoy[0] == -1)
						g.playerJoy[0] = i; // select P1
				}
				else if(allJoys[i] > 0)
				{
					if(g.playerJoy[1] == -1)
						g.playerJoy[1] = i; // select P2
				}
				else  // withdraw selection
				{
					if(g.playerJoy[0] == i)
						g.playerJoy[0] = -1;
					else if(g.playerJoy[1] == i)
						g.playerJoy[1] = -1;
				}

				if(allJoys[i] != 0)
				{
					for(int j = 0; j < SDL_JoystickNumButtons(joy); j++)
					{
						if(SDL_JoystickGetButton(joy, j))
						{
							selectInput[i] = 1;
							allowMove[i] = 0;
							if(allJoys[i] == -1)
								p1sel = 1;
							else if(allJoys[i] == 1)
								p2sel = 1;
						}
					}

					if(i == 0) // joystick is GCW0, allow selection with keyboard
					{
						Uint8 *key = SDL_GetKeyState(NULL);

						if(key[SDLK_LCTRL])
						{
							selectInput[i] = 1;
							allowMove[i] = 0;
							if(allJoys[i] == -1)
								p1sel = 1;
							else if(allJoys[i] == 1)
								p2sel = 1;
						}
					}
				}
			}
		}
	}
}

void menuSelectPlayerDraw(gamedata &g)
{
	int numOfJoys = SDL_NumJoysticks();

	char textJoys[numOfJoys][5];

	for(int i = 0; i < numOfJoys; i++)
	{
		if(i == 0)
			sprintf(textJoys[i], "GCW0");
		else
			sprintf(textJoys[i], "JOY%d", i);
	}

	if(numOfJoys > 1)
	{
		for(int i = 0; i < numOfJoys; i++)
		{
			if(g.playerJoy[0] == i)
			{
				if(!p1sel)
					g.whitefont.write(g.virtualscreen, 120 + 50*i, 30, textJoys[i]);
				else
				{
					if(g.playerJoyBut[0][0] == -1 && i != 0)
					{
						g.whitefont.write(g.virtualscreen, 120, 30, textJoys[i]);
						g.whitefont.write(g.virtualscreen, 120 + 50, 30, "Press Fire key");
					}
					else if(g.playerJoyBut[0][1] == -1 && i != 0)
					{
						g.whitefont.write(g.virtualscreen, 120, 30, textJoys[i]);
						g.whitefont.write(g.virtualscreen, 120 + 50, 30, "Press Bomb key");
					}
					else if(g.playerJoyBut[0][2] == -1 && i != 0)
					{
						g.whitefont.write(g.virtualscreen, 120, 30, textJoys[i]);
						g.whitefont.write(g.virtualscreen, 120 + 50, 30, "Press Menu key");
					}
					else
					{
						g.whitefont.write(g.virtualscreen, 120, 30, textJoys[i]);
						if(newg.players == 2)
						{
							g.whitefont.write(g.virtualscreen, 120 + 50, 20, "Done! Wait for");
							g.whitefont.write(g.virtualscreen, 120 + 50, 30, "the other player...");
						}
					}
				}
			}
			else if(g.playerJoy[1] == i)
			{
				if(!p2sel)
					g.whitefont.write(g.virtualscreen, 120 + 50*i, 70, textJoys[i]);
				else
				{
					if(g.playerJoyBut[1][0] == -1 && i != 0)
					{
						g.whitefont.write(g.virtualscreen, 120, 70, textJoys[i]);
						g.whitefont.write(g.virtualscreen, 120 + 50, 70, "Press Fire key");
					}
					else if(g.playerJoyBut[1][1] == -1 && i != 0)
					{
						g.whitefont.write(g.virtualscreen, 120, 70, textJoys[i]);
						g.whitefont.write(g.virtualscreen, 120 + 50, 70, "Press Bomb key");
					}
					else
					{
						g.whitefont.write(g.virtualscreen, 120, 70, textJoys[i]);
						if(newg.players == 2)
						{
							g.whitefont.write(g.virtualscreen, 120 + 50, 70, "Done! Wait for");
							g.whitefont.write(g.virtualscreen, 120 + 50, 80, "the other player...");
						}
					}
				}
			}
			else
				g.whitefont.write(g.virtualscreen, 120 + 50*i, 50, textJoys[i]);
		}
	}

	g.whitefont.write(g.virtualscreen, 40, 30, "Player 1");
	if(newg.players == 2)
		g.whitefont.write(g.virtualscreen, 40, 70, "Player 2");	
}

