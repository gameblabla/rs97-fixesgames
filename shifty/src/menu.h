#ifndef _MENU_H_
#define _MENU_H_

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

typedef enum MenuItemType
{
	ITEM_TYPE_SKIP,		/* Skip over. */
	ITEM_TYPE_ACTION,	/* Trigger an action. */
	ITEM_TYPE_CHOICE,	/* Select between multiple options. */
	ITEM_TYPE_SUBMENU,	/* Go to submenu. */
	ITEM_TYPE_PARENT,	/* Go to parent. */
	ITEM_TYPE_CUSTOM	/* Mix of the above. */
} MenuItemType;

typedef struct MenuItem
{
	/* Common fields. */
	MenuItemType type;
	char *name;
	void *target;

	/* Action type fields. */
	void (*action)(void);

	/* Choice type fields. */
	void (*choice)(unsigned int *);
	unsigned int choiceCount;
	char **choiceNames;
} MenuItem;

typedef struct Menu
{
	struct Menu *parent;
	char *name;
	MenuItem *items;
	unsigned int itemSel;
	unsigned int itemCount;
	void (*init)(void);
} Menu;

extern Menu mainMenu;

extern unsigned int mainMenuGameModeTarget;
extern unsigned int optionsMenuScaleTarget;
extern unsigned int optionsMenuFullscreenTarget;
extern unsigned int optionsMenuFPSTarget;
extern unsigned int trainingModeMenuShiftTarget;
extern unsigned int trainingModeMenuJunkTarget;
extern unsigned int trainingModeMenuDebugTarget;

void mainMenuCreditsAction();
void mainMenuExitAction();

void gameModeSoloAction();
void gameModeDemoAction();

void battleModeAIAction();
void battleModeLocalAction();

void networkModeHostAction();
void networkModeJoinAction();

void trainingModeStartAction();

void optionsMenuScaleChoice(unsigned int *choiceSel);
void optionsMenuFullscreenChoice(unsigned int *choiceSel);
void optionsMenuFPSChoice(unsigned int *choiceSel);


void menuLogic(Menu **menuPptr, int *keyMap[]);
void menuDisplay(Menu *menu, int x, int y);

#endif /* _MENU_H_ */
