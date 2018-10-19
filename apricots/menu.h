#ifndef _MENU_H_
#define _MENU_H_

#include "apricots.h"

typedef enum MenuActionEnum
{
	ACTION_NONE,
	ACTION_NEW_GAME,
	ACTION_OPTIONS,
	ACTION_QUIT,
	ACTION_BACK_TO_GAME,
	ACTION_OPTIONS_PLANE_SETTINGS,
	ACTION_OPTIONS_GAME_SETTINGS,
	ACTION_OPTIONS_BACK,
	ACTION_SET_NUM_OF_PLANES,
	ACTION_SET_MISSION,
	ACTION_SET_SCORE,
	ACTION_SET_TOWERS,
	ACTION_SET_GUNS,
	ACTION_SET_BUILDINGS,
	ACTION_SET_TREES,
	ACTION_SET_DRAK,
	ACTION_SET_SCOREBAR_POS,
	ACTION_GAME_OPTIONS_BACK
} MenuAction;

typedef struct MenuItemStruct
{
	int number;
	MenuAction Action;
	char caption[101];
	int value;
	struct MenuItemStruct *Next;
} MenuItem;

typedef struct MenuContainerStruct
{
	MenuItem *Menu;
	int size;
} MenuContainer;

MenuContainer *menuCreateNew(MenuContainer *Container, int number, const char *caption, MenuAction Action);
MenuItem *menuSwitchItem(MenuContainer *Container, int number);

extern int planeArray[6][3];
extern int cursorX;
extern int cursorY;
extern int *p1plane;
extern int *p2plane;

void menuAction(MenuItem *Item, gamedata &g);
void menuActionChangeValue(MenuItem *Item, int value, gamedata &g);
void menuDeleteSingle(MenuContainer *Container);
void menuDeleteAll();
void menuLoadAll(gamedata &g);
void menuInput(gamedata &g);
void menuDrawSingle(MenuContainer *Container, int number, int x, int y, gamedata &g);
void menuDraw(MenuContainer *Container, int x, int y, gamedata &g);
void menuPlanes(gamedata &g);
void menuPlanesDraw(int x, int y, gamedata &g);
void loadPlaneInfo(gamedata &g);
void menuSelectPlayer(gamedata &g);
void menuSelectPlayerDraw(gamedata &g);

extern MenuContainer *CurrentMenu;
extern MenuItem *SelectedItem;

extern MenuContainer *MenuMain;
extern MenuContainer *MenuMainIntro;
extern MenuContainer *MenuOptions;
extern MenuContainer *MenuGameOptions;

#endif /* _MENU_H_ */

