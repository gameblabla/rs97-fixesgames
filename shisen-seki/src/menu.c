#include "menu.h"

#include "font.h"
#include "input.h"

menuItems menuMainItems[] =
{
	MENU_CONTINUE,
	MENU_NEW_GAME,
	MENU_HISCORE,
	MENU_HELP,
	MENU_OPTIONS,
	MENU_CREDITS,
	MENU_QUIT
};

menuItems menuNewGameItems[] =
{
	MENU_START_GAME,
	MENU_PRACTICE,
	MENU_BACK,
	MENU_SEPARATOR,
	MENU_GAME_TYPE,
	MENU_ALGORITHM
};

menuItems menuOptionsItems[] =
{
#if !defined(PLATFORM_A320)
	MENU_HAVE_JOYSTICK,
#endif
	MENU_HAVE_MUSIC,
	MENU_HAVE_SFX,
	MENU_ANIMATIONS,
#if !defined(NO_SCALING)
	MENU_SCALE,
#endif
	MENU_SEPARATOR,
	MENU_RESET_HISCORE,
	MENU_VERSION_INFO,
	MENU_SEPARATOR,
	MENU_BACK
};

menuItems menuResetScoreItems[] =
{
	MENU_NO,
	MENU_YES
};

char menuText[][20] =
{
	"",
	"Back",
	"Yes",
	"No",
	"Continue game",
	"New game",
	"Score table",
	"Help",
	"Options",
	"Credits",
	"Exit",
	"Start game",
	"Practice",
	"Game type",
	"Algorithm",
	"Use joystick",
	"Play music",
	"Play sfx",
	"Animations",
	"Screen scale",
	"Clear score table",
	"Version info",
};

menuContainer menuMain;
menuContainer menuNewGame;
menuContainer menuOptions;
menuContainer menuResetScore;

void menuLoad()
{
	menuMain.items = menuMainItems;
	menuMain.length = sizeof(menuMainItems)/sizeof(menuMainItems[0]);
	menuMain.y = 80;
	menuNewGame.items = menuNewGameItems;
	menuNewGame.length = sizeof(menuNewGameItems)/sizeof(menuNewGameItems[0]);
	menuNewGame.y = 80;
	menuOptions.items = menuOptionsItems;
	menuOptions.length = sizeof(menuOptionsItems)/sizeof(menuOptionsItems[0]);
	menuOptions.y = 80;
	menuResetScore.items = menuResetScoreItems;
	menuResetScore.length = sizeof(menuResetScoreItems)/sizeof(menuResetScoreItems[0]);
	menuResetScore.y = 120;
}

void menuDraw(menuContainer *curMenu, font *fontRegular, font *fontSelected, int selection, int offset, int y)
{
	int i;

	if (!curMenu)
	{
		return;
	}

	for (i = offset; i < curMenu->length; ++i)
	{
		font *curFont = i == selection ? fontSelected : fontRegular;

		dTextCentered(curFont, menuText[curMenu->items[i]-1], y + (curFont->h + curFont->leading) * i, SHADOW_DROP);
	}
}
