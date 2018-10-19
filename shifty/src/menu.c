#include "menu.h"
#include "backend/input.h"
#include "font.h"

#define BUILD_MENU_ITEM_SKIP(name) \
	ITEM_TYPE_SKIP, name, NULL, NULL, NULL, 0, NULL

#define BUILD_MENU_ITEM_ACTION(name, action) \
	ITEM_TYPE_ACTION, name, NULL, action, NULL, 0, NULL

#define BUILD_MENU_ITEM_CHOICE(name, choice, target, nameList) \
	ITEM_TYPE_CHOICE, name, target, NULL, choice, ARRAY_SIZE(nameList), nameList

#define BUILD_MENU_ITEM_SUBMENU(name, target) \
	ITEM_TYPE_SUBMENU, name, target, NULL, NULL, 0, NULL

#define BUILD_MENU_ITEM_PARENT(name, action) \
	ITEM_TYPE_PARENT, name, NULL, action, NULL, 0, NULL

Menu mainMenu;
Menu gameModeMenu;
Menu battleModeMenu;
Menu networkModeMenu;
Menu trainingModeMenu;
Menu optionsMenu;

char *offOnChoices[] =
{
	"OFF", "ON"
};

unsigned int mainMenuGameModeTarget;

char *mainMenuGameModeChoices[] =
{
	"Solo", "Battle", "Networked"
};

MenuItem mainMenuItems[] =
{
	{ BUILD_MENU_ITEM_SUBMENU("Play", &gameModeMenu) },
	{ BUILD_MENU_ITEM_SUBMENU("Options", &optionsMenu) },
	{ BUILD_MENU_ITEM_ACTION("Credits", mainMenuCreditsAction) },
	{ BUILD_MENU_ITEM_SKIP(NULL) },
	{ BUILD_MENU_ITEM_PARENT("Exit", mainMenuExitAction) }
};

Menu mainMenu =
{
	NULL, "S H I F T Y   P I L L S", mainMenuItems, 0, ARRAY_SIZE(mainMenuItems), NULL
};

MenuItem gameModeMenuItems[] =
{
	{ BUILD_MENU_ITEM_ACTION("Solo", gameModeSoloAction) },
	{ BUILD_MENU_ITEM_SUBMENU("Battle", &battleModeMenu) },
	{ BUILD_MENU_ITEM_SUBMENU("Training", &trainingModeMenu) },
	{ BUILD_MENU_ITEM_ACTION("Demo", &gameModeDemoAction) },
	{ BUILD_MENU_ITEM_SKIP(NULL) },
	{ BUILD_MENU_ITEM_PARENT("Back", NULL) }
};

Menu gameModeMenu =
{
	&mainMenu, "Select game mode", gameModeMenuItems, 0, ARRAY_SIZE(gameModeMenuItems), NULL
};

MenuItem battleModeMenuItems[] =
{
	{ BUILD_MENU_ITEM_ACTION("vs AI", battleModeAIAction) },
	{ BUILD_MENU_ITEM_ACTION("Local", battleModeLocalAction) },
	{ BUILD_MENU_ITEM_SUBMENU("Networked", &networkModeMenu) },
	{ BUILD_MENU_ITEM_SKIP(NULL) },
	{ BUILD_MENU_ITEM_PARENT("Back", NULL) }
};

Menu battleModeMenu =
{
	&gameModeMenu, "Select opponent", battleModeMenuItems, 0, ARRAY_SIZE(battleModeMenuItems), NULL
};

MenuItem networkModeMenuItems[] =
{
	{ BUILD_MENU_ITEM_ACTION("Host", networkModeHostAction) },
	{ BUILD_MENU_ITEM_ACTION("Join", networkModeJoinAction) },
	{ BUILD_MENU_ITEM_SKIP(NULL) },
	{ BUILD_MENU_ITEM_PARENT("Back", NULL) }
};

Menu networkModeMenu =
{
	&battleModeMenu, "Select connection mode", networkModeMenuItems, 0, ARRAY_SIZE(networkModeMenuItems), NULL
};

unsigned int trainingModeMenuShiftTarget = 1;
unsigned int trainingModeMenuJunkTarget;
unsigned int trainingModeMenuDebugTarget;


MenuItem trainingModeMenuItems[] =
{
	{ BUILD_MENU_ITEM_ACTION("Begin training", trainingModeStartAction) },
	{ BUILD_MENU_ITEM_SKIP(NULL) },
	{ BUILD_MENU_ITEM_CHOICE("Shift event:", NULL, &trainingModeMenuShiftTarget, offOnChoices) },
	{ BUILD_MENU_ITEM_CHOICE("Junk blocks:", NULL, &trainingModeMenuJunkTarget, offOnChoices) },
	{ BUILD_MENU_ITEM_CHOICE("Debug info:", NULL, &trainingModeMenuDebugTarget, offOnChoices) },
	{ BUILD_MENU_ITEM_SKIP(NULL) },
	{ BUILD_MENU_ITEM_PARENT("Back", NULL) }
};

Menu trainingModeMenu =
{
	&gameModeMenu, "Training options", trainingModeMenuItems, 0, ARRAY_SIZE(trainingModeMenuItems), NULL
};

unsigned int optionsMenuScaleTarget;
unsigned int optionsMenuFullscreenTarget;
unsigned int optionsMenuFPSTarget;

char *optionsMenuScaleChoices[] =
{
	"1x", "2x", "3x", "4x"
};

MenuItem optionsMenuItems[] =
{
	{ BUILD_MENU_ITEM_CHOICE("Scale:", optionsMenuScaleChoice, &optionsMenuScaleTarget, optionsMenuScaleChoices) },
	{ BUILD_MENU_ITEM_CHOICE("Fullscreen:", optionsMenuFullscreenChoice, &optionsMenuFullscreenTarget, offOnChoices) },
	{ BUILD_MENU_ITEM_CHOICE("Show FPS:", optionsMenuFPSChoice, &optionsMenuFPSTarget, offOnChoices) },
	{ BUILD_MENU_ITEM_SKIP(NULL) },
	{ BUILD_MENU_ITEM_PARENT("Back", NULL) }
};

Menu optionsMenu =
{
	&mainMenu, "Game options", optionsMenuItems, 0, ARRAY_SIZE(optionsMenuItems), NULL
};

void menuLogic(Menu **menuPptr, int *keyMap[])
{
	Menu *menu = *menuPptr;
	MenuItem *item;

	if (!menu || !keyMap || !menu->itemCount)
		return;

	item = &menu->items[menu->itemSel];

	if (*keyMap[KEY_UP])
	{
		*keyMap[KEY_UP] = 0;

		do
		{
			if (menu->itemSel == 0)
				menu->itemSel = menu->itemCount;

			--menu->itemSel;
		}
		while (menu->items[menu->itemSel].type == ITEM_TYPE_SKIP);
	}

	if (*keyMap[KEY_DOWN])
	{
		*keyMap[KEY_DOWN] = 0;

		do
		{
			if (++menu->itemSel >= menu->itemCount)
				menu->itemSel = 0;
		}
		while (menu->items[menu->itemSel].type == ITEM_TYPE_SKIP);
	}

	if (*keyMap[KEY_LEFT])
	{
		*keyMap[KEY_LEFT] = 0;

		if (item->type == ITEM_TYPE_CHOICE && item->choiceCount && item->target)
		{
			unsigned int *choiceSel = (unsigned int *)item->target;

			if (*choiceSel == 0)
				*choiceSel = item->choiceCount;

			--(*choiceSel);

			if (item->choice)
				item->choice(choiceSel);
		}
	}

	if (*keyMap[KEY_RIGHT])
	{
		*keyMap[KEY_RIGHT] = 0;

		if (item->type == ITEM_TYPE_CHOICE && item->choiceCount && item->target)
		{
			unsigned int *choiceSel = (unsigned int *)item->target;

			if (++(*choiceSel) >= item->choiceCount)
				*choiceSel = 0;

			if (item->choice)
				item->choice(choiceSel);
		}
	}

	if (*keyMap[KEY_OK])
	{
		*keyMap[KEY_OK] = 0;

		switch (item->type)
		{
			case ITEM_TYPE_SUBMENU:
				if (item->target)
				{
					Menu *submenu = (Menu *)item->target;

					if (submenu->init)
						submenu->init();

					*menuPptr = submenu;
				}
			break;
			case ITEM_TYPE_PARENT:
				if (menu->parent)
				{
					menu->itemSel = 0;
					
					if (menu->parent->init)
						menu->parent->init();

					*menuPptr = menu->parent;
				}

			/* Fall-through. */
			case ITEM_TYPE_ACTION:
				if (item->action)
					item->action();
			break;
			case ITEM_TYPE_CHOICE:
				if (item->choiceCount && item->target)
				{
					unsigned int *choiceSel = (unsigned int *)item->target;

					if (++(*choiceSel) >= item->choiceCount)
						*choiceSel = 0;

					if (item->choice)
						item->choice(choiceSel);
				}
			break;

			default:
			break;
		}
	}

	if (*keyMap[KEY_BACK])
	{
		unsigned int i;

		*keyMap[KEY_BACK] = 0;

		for (i = 0; i < menu->itemCount; ++i)
		{
			if (menu->items[i].type == ITEM_TYPE_PARENT)
			{
				menu->itemSel = i;
				break;
			}
		}
	}
}

void menuDisplay(Menu *menu, int x, int y)
{
	unsigned int i;

	if (!menu || !menu->items)
		return;

	if (menu->name)
		dTextCentered(fontDefault, SHADOW_OUTLINE, menu->name, 50);

	for (i = 0; i < menu->itemCount; ++i)
	{
		if (menu->items[i].name)
			dText(fontDefault, i == menu->itemSel ? SHADOW_OUTLINE : SHADOW_DROP, menu->items[i].name, x, y + i * (fontDefault->h + fontDefault->leading));

		if (menu->items[i].type == ITEM_TYPE_CHOICE && menu->items[i].choiceCount && menu->items[i].target)
		{
			unsigned int *choiceSel = (unsigned int *)menu->items[i].target;

			dText(fontDefault, SHADOW_DROP, menu->items[i].choiceNames[*choiceSel], x + 90, y + i * (fontDefault->h + fontDefault->leading));
		}

		if (i == menu->itemSel) /* Temporary. */
			dText(fontDefault, SHADOW_OUTLINE, "*", x - 15, y + i * (fontDefault->h + fontDefault->leading));
	}
}
