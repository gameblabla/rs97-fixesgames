#ifndef _MENU_H_
#define _MENU_H_

#include "font.h"

typedef enum menuItems
{
	MENU_UNSET = 0,
	MENU_SEPARATOR,
	MENU_BACK,
	MENU_YES,
	MENU_NO,
	MENU_CONTINUE,
	MENU_NEW_GAME,
	MENU_HISCORE,
	MENU_HELP,
	MENU_OPTIONS,
	MENU_CREDITS,
	MENU_QUIT,
	MENU_START_GAME,
	MENU_PRACTICE,
	MENU_GAME_TYPE,
	MENU_ALGORITHM,
	MENU_HAVE_JOYSTICK,
	MENU_HAVE_MUSIC,
	MENU_HAVE_SFX,
	MENU_ANIMATIONS,
	MENU_SCALE,
	MENU_RESET_HISCORE,
	MENU_VERSION_INFO,
	MENU_COUNT
	
} menuItems;

typedef struct menuContainer
{
	menuItems *items;
	int length;
	int y;
} menuContainer;

extern menuContainer menuMain;
extern menuContainer menuNewGame;
extern menuContainer menuOptions;
extern menuContainer menuResetScore;
extern char menuText[][20];

void menuLoad();
void menuDraw(menuContainer *curMenu, font *fontRegular, font *fontSelected, int selection, int offset, int y);

#endif /* _MENU_H_ */
