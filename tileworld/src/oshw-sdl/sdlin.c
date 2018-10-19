/* sdlin.c: Reading the keyboard.
 *
 * Copyright (C) 2001-2006 by Brian Raiter, under the GNU General Public
 * License. No warranty. See COPYING for details.
 */

#include	<string.h>
#include	<SDL.h>
#include	"sdlgen.h"
#include	"../defs.h"

//DKS added
#ifdef PLATFORM_GP2X
/* Structure describing a mapping of a key event to a game command.
 */
typedef	struct buttoncmdmap {
	int		buttoncode;	/* the button number */
	int		cmd;		/* the command */
	int		hold;		/* TRUE for repeating buttons (joystick) */
} buttoncmdmap;

// The possible states of buttons
enum { BS_OFF,           /* button is not currently pressed */
	BS_DOWN,			/* button is being held down */
	BS_STRUCK,		/* button was pressed and released in one tick */
	BS_PRESSED,		/* button was pressed in this tick */
	BS_DOWNBUTOFF1,		/* button has been down since the previous tick */
	BS_DOWNBUTOFF2,		/* button has been down since two ticks ago */
	BS_DOWNBUTOFF3,		/* button has been down since three ticks ago */
	BS_REPEATING,		/* button is down and is now repeating */
	BS_count  // used in below code to see how many total states there are
};

// array of button states 
static char bstates[NUMBUTTONS];

// NOTE: the keycmdmap this is based on uses 0 in the first field to signify the end
// of the map, but 0 is a valid button number.  I am therefore using -1 to signify
// the end of button cmd mapping.
static buttoncmdmap const gamebuttoncmds[] = {
	{ GP2X_BUTTON_UP,               CmdNorth,                   TRUE },
	{ GP2X_BUTTON_LEFT,             CmdWest,                    TRUE },
	{ GP2X_BUTTON_DOWN,             CmdSouth,                   TRUE },
	{ GP2X_BUTTON_RIGHT,            CmdEast,                    TRUE },
	{ GP2X_BUTTON_UPLEFT,           CmdNorth | CmdWest,         TRUE },
	{ GP2X_BUTTON_DOWNLEFT,         CmdSouth | CmdWest,         TRUE },
	{ GP2X_BUTTON_DOWNRIGHT,        CmdSouth | CmdEast,         TRUE },
	{ GP2X_BUTTON_UPRIGHT,          CmdNorth | CmdEast,         TRUE },
	{ GP2X_BUTTON_Y,				CmdNorth,					TRUE },
	{ GP2X_BUTTON_A,				CmdWest,					TRUE },
	{ GP2X_BUTTON_X,				CmdSouth,					TRUE },
	{ GP2X_BUTTON_B,				CmdEast,					TRUE },
	{ GP2X_BUTTON_START,			CmdProceed,					FALSE },
	{ GP2X_BUTTON_SELECT,           CmdQuitLevel,               FALSE },
	{ GP2X_BUTTON_VOLUP,            CmdVolumeUp,                FALSE },
	{ GP2X_BUTTON_VOLDOWN,          CmdVolumeDown,              FALSE },
	{-1, 0, 0}
};

/* The list of button commands recognized when the program is obtaining
 * input from the user.
 */
static buttoncmdmap const inputbuttoncmds[] = {
	{ GP2X_BUTTON_UP,               CmdNorth,                   TRUE },
	{ GP2X_BUTTON_LEFT,             CmdWest,                    TRUE },
	{ GP2X_BUTTON_DOWN,             CmdSouth,                   TRUE },
	{ GP2X_BUTTON_RIGHT,            CmdEast,                    TRUE },
	{ GP2X_BUTTON_A,                CmdProceed,                 FALSE },
	{ GP2X_BUTTON_B,                CmdProceed,                 FALSE },
	{ GP2X_BUTTON_START,            CmdProceed,                 FALSE },
	{ GP2X_BUTTON_SELECT,           CmdQuitLevel,                FALSE },
	{ GP2X_BUTTON_VOLUP,            CmdVolumeUp,                FALSE },
	{ GP2X_BUTTON_VOLDOWN,          CmdVolumeDown,              FALSE },
	{ GP2X_BUTTON_R,				CmdNext10,					TRUE },
	{ GP2X_BUTTON_L,				CmdPrev10,					TRUE },
	{-1, 0, 0}
};


/* The current map of button commands.
 */
static buttoncmdmap const *buttoncmds = gamebuttoncmds;

//DKS - added this my version of the original keyeventcallback to handle
//  GP2X joystick
static void _joybuttoneventcallback(int buttoncode, int down)
{

	if (down) {
		bstates[buttoncode] = bstates[buttoncode] == BS_OFF ?
			BS_PRESSED : BS_REPEATING;
	} else {
		bstates[buttoncode] = bstates[buttoncode] == BS_PRESSED ?
			BS_STRUCK : BS_OFF;
	}
}
#endif //PLATFORM_GP2X


/* Structure describing a mapping of a key event to a game command.
 */
typedef	struct keycmdmap {
	int		scancode;	/* the key's scan code */
	int		shift;		/* the shift key's state */
	int		ctl;		/* the ctrl key's state */
	int		alt;		/* the alt keys' state */
	int		cmd;		/* the command */
	int		hold;		/* TRUE for repeating joystick-mode keys */
} keycmdmap;

/* Structure describing mouse activity.
 */
typedef struct mouseaction {
	int		state;		/* state of mouse action (KS_*) */
	int		x, y;		/* position of the mouse */
	int		button;		/* which button generated the event */
} mouseaction;

/* The possible states of keys.
 */
enum { KS_OFF = 0,		/* key is not currently pressed */
	KS_ON = 1,		/* key is down (shift-type keys only) */
	KS_DOWN,			/* key is being held down */
	KS_STRUCK,		/* key was pressed and released in one tick */
	KS_PRESSED,		/* key was pressed in this tick */
	KS_DOWNBUTOFF1,		/* key has been down since the previous tick */
	KS_DOWNBUTOFF2,		/* key has been down since two ticks ago */
	KS_DOWNBUTOFF3,		/* key has been down since three ticks ago */
	KS_REPEATING,		/* key is down and is now repeating */
	KS_count
};


/* The complete array of key states.
 */
static char		keystates[SDLK_LAST];

/* The last mouse action.
 */
static mouseaction	mouseinfo;

/* TRUE if direction keys are to be treated as always repeating.
 */
static int		joystickstyle = FALSE;

/* The complete list of key commands recognized by the game while
 * playing. hold is TRUE for keys that are to be forced to repeat.
 * shift, ctl and alt are positive if the key must be down, zero if
 * the key must be up, or negative if it doesn't matter.
 */
#if 1//PLATFORM_GCW
//DKS - for reference on the GCW:
//    case  SDLK_UP:          /* Dingoo  D-pad up */
//    case  SDLK_DOWN:        /* Dingoo  D-pad down */
//    case  SDLK_LEFT:        /* Dingoo  D-pad left */
//    case  SDLK_RIGHT:       /* Dingoo  D-pad right */
//    case  SDLK_LCTRL:       /* Dingoo  A button */
//    case  SDLK_LALT:        /* Dingoo  B button */
//    case  SDLK_SPACE:       /* Dingoo  Y button */
//    case  SDLK_LSHIFT:      /* Dingoo  X button */
//    case  SDLK_RETURN:      /* Dingoo  START button */
//    case  SDLK_ESCAPE:      /* Dingoo  SELECT button */
//    case  SDLK_TAB:         /* Dingoo  Left shoulder */
//    case  SDLK_BACKSPACE:   /* Dingoo  Right shoulder */
//    case  SDLK_PAUSE:       /* Dingoo  POWER UP button */
static keycmdmap const gamekeycmds[] = {
	//    { SDLK_UP,                    0,  0,  0,   CmdNorth,            TRUE },
	//    { SDLK_LEFT,                  0,  0,  0,   CmdWest,             TRUE },
	//    { SDLK_DOWN,                  0,  0,  0,   CmdSouth,            TRUE },
	//    { SDLK_RIGHT,                 0,  0,  0,   CmdEast,             TRUE },
	//    { SDLK_ESCAPE,                0,  0,  0,   CmdQuitLevel,        FALSE },
	//    { SDLK_RETURN,               -1, -1,  0,   CmdProceed,          FALSE },
	//    { SDLK_LCTRL,                0, -1,  0,   CmdEast, 	           TRUE },
	//    { SDLK_LALT,                 0, -1,  0,   CmdSouth,            TRUE },
	//    { SDLK_LSHIFT,               0, -1,  0,   CmdWest,            TRUE },
	//    { SDLK_SPACE,                0, -1,  0,   CmdNorth,	           TRUE },
	{ SDLK_UP,                    -1, -1, -1,   CmdNorth,            TRUE },
	{ SDLK_LEFT,                  -1, -1, -1,   CmdWest,             TRUE },
	{ SDLK_DOWN,                  -1, -1, -1,   CmdSouth,            TRUE },
	{ SDLK_RIGHT,                 -1, -1, -1,   CmdEast,             TRUE },
	{ SDLK_RETURN,                -1, -1, -1,   CmdQuitLevel,        FALSE },
	{ SDLK_ESCAPE,               -1, -1,  -1,   CmdProceed,          FALSE },
	{ SDLK_LCTRL,                -1, -1,  -1,   CmdEast, 	           TRUE },
	{ SDLK_LALT,                 -1, -1,  -1,   CmdSouth,            TRUE },
	{ SDLK_LSHIFT,               -1, -1,  -1,   CmdWest,            TRUE },
	{ SDLK_SPACE,                -1, -1,  -1,   CmdNorth,	           TRUE },
	{ 0, 0, 0, 0, 0, 0 }
};
/* The list of key commands recognized when the program is obtaining
 * input from the user.
 */
static keycmdmap const inputkeycmds[] = {
	//    { SDLK_UP,                   -1, -1,  0,   CmdNorth,              TRUE },
	//    { SDLK_LEFT,                 -1, -1,  0,   CmdWest,               TRUE },
	//    { SDLK_DOWN,                 -1, -1,  0,   CmdSouth,              TRUE },
	//    { SDLK_RIGHT,                -1, -1,  0,   CmdEast,               TRUE },
	//    { SDLK_RETURN,               -1, -1,  0,   CmdProceed,            FALSE },
	//    { SDLK_LCTRL,                -1, -1,  0,   CmdProceed,            FALSE },
	//    { SDLK_LALT,                 -1, -1,  0,   CmdProceed,            FALSE },
	//    { SDLK_SPACE,                -1, -1,  0,   CmdProceed,            FALSE },
	//    { SDLK_KP_ENTER,             -1, -1,  0,   CmdProceed,            FALSE },
	//    { SDLK_ESCAPE,               -1, -1,  0,   CmdQuitLevel,          FALSE },
	//    { SDLK_TAB,                  -1, -1,  0,   CmdPrev10,             TRUE },
	//    { SDLK_BACKSPACE,            -1, -1,  0,   CmdNext10,             TRUE },
	{ SDLK_UP,                   -1, -1,  -1,   CmdNorth,              TRUE },
	//    { SDLK_LEFT,                 -1, -1,  -1,   CmdWest,               TRUE },
	{ SDLK_LEFT,                 -1, -1,  -1,   CmdPrev,               TRUE },
	{ SDLK_DOWN,                 -1, -1,  -1,   CmdSouth,              TRUE },
	//    { SDLK_RIGHT,                -1, -1,  -1,   CmdEast,               TRUE },
	{ SDLK_RIGHT,                -1, -1,  -1,   CmdNext,               TRUE },
	{ SDLK_RETURN,               -1, -1,  -1,   CmdProceed,            FALSE },
	{ SDLK_LCTRL,                -1, -1,  -1,   CmdProceed,            FALSE },
	{ SDLK_LALT,                 -1, -1,  -1,   CmdProceed,            FALSE },
	{ SDLK_SPACE,                -1, -1,  -1,   CmdProceed,            FALSE },
	{ SDLK_LSHIFT,             -1, -1,  -1,   CmdProceed,            FALSE },
	{ SDLK_ESCAPE,               -1, -1,  -1,   CmdQuitLevel,          FALSE },
	{ SDLK_TAB,                  -1, -1,  -1,   CmdPrev10,             TRUE },
	{ SDLK_BACKSPACE,            -1, -1,  -1,   CmdNext10,             TRUE },
	{ 0, 0, 0, 0, 0, 0 }
};
#else // NOT RUNNING ON GCW:
static keycmdmap const gamekeycmds[] = {
	{ SDLK_UP,                    0,  0,  0,   CmdNorth,              TRUE },
	{ SDLK_LEFT,                  0,  0,  0,   CmdWest,               TRUE },
	{ SDLK_DOWN,                  0,  0,  0,   CmdSouth,              TRUE },
	{ SDLK_RIGHT,                 0,  0,  0,   CmdEast,               TRUE },
	{ SDLK_KP8,                   0,  0,  0,   CmdNorth,              TRUE },
	{ SDLK_KP4,                   0,  0,  0,   CmdWest,               TRUE },
	{ SDLK_KP2,                   0,  0,  0,   CmdSouth,              TRUE },
	{ SDLK_KP6,                   0,  0,  0,   CmdEast,               TRUE },
	{ 'q',                        0,  0,  0,   CmdQuitLevel,          FALSE },
	{ 'p',                        0, +1,  0,   CmdPrevLevel,          FALSE },
	{ 'r',                        0, +1,  0,   CmdSameLevel,          FALSE },
	{ 'n',                        0, +1,  0,   CmdNextLevel,          FALSE },
	{ 'g',                        0, -1,  0,   CmdGotoLevel,          FALSE },
	{ 'q',                       +1,  0,  0,   CmdQuit,               FALSE },
	{ SDLK_PAGEUP,               -1, -1,  0,   CmdPrev10,             FALSE },
	{ 'p',                        0,  0,  0,   CmdPrev,               FALSE },
	{ 'r',                        0,  0,  0,   CmdSame,               FALSE },
	{ 'n',                        0,  0,  0,   CmdNext,               FALSE },
	{ SDLK_PAGEDOWN,             -1, -1,  0,   CmdNext10,             FALSE },
	{ '\b',                      -1, -1,  0,   CmdPauseGame,          FALSE },
	{ '?',			 -1, -1,  0,   CmdHelp,               FALSE },
	{ SDLK_F1,                   -1, -1,  0,   CmdHelp,               FALSE },
	{ 'o',			  0,  0,  0,   CmdStepping,           FALSE },
	{ 'o',			 +1,  0,  0,   CmdSubStepping,        FALSE },
	{ '\t',                       0, -1,  0,   CmdPlayback,           FALSE },
	{ '\t',                      +1, -1,  0,   CmdCheckSolution,      FALSE },
	{ 'x',                        0, +1,  0,   CmdReplSolution,       FALSE },
	{ 'x',                       +1, +1,  0,   CmdKillSolution,       FALSE },
	{ 's',                        0,  0,  0,   CmdSeeScores,          FALSE },
	{ 's',			  0, +1,  0,   CmdSeeSolutionFiles,   FALSE },
	{ 'v',                       +1,  0,  0,   CmdVolumeUp,           FALSE },
	{ 'v',                        0,  0,  0,   CmdVolumeDown,         FALSE },
	{ SDLK_RETURN,               -1, -1,  0,   CmdProceed,            FALSE },
	{ SDLK_KP_ENTER,             -1, -1,  0,   CmdProceed,            FALSE },
	{ ' ',                       -1, -1,  0,   CmdProceed,            FALSE },
	{ 'd',                        0,  0,  0,   CmdDebugCmd1,          FALSE },
	{ 'd',                       +1,  0,  0,   CmdDebugCmd2,          FALSE },
	{ SDLK_UP,                   +1,  0,  0,   CmdCheatNorth,         TRUE },
	{ SDLK_LEFT,                 +1,  0,  0,   CmdCheatWest,          TRUE },
	{ SDLK_DOWN,                 +1,  0,  0,   CmdCheatSouth,         TRUE },
	{ SDLK_RIGHT,                +1,  0,  0,   CmdCheatEast,          TRUE },
	{ SDLK_HOME,                 +1,  0,  0,   CmdCheatHome,          FALSE },
	{ SDLK_F2,                    0,  0,  0,   CmdCheatICChip,        FALSE },
	{ SDLK_F3,                    0,  0,  0,   CmdCheatKeyRed,        FALSE },
	{ SDLK_F4,                    0,  0,  0,   CmdCheatKeyBlue,       FALSE },
	{ SDLK_F5,                    0,  0,  0,   CmdCheatKeyYellow,     FALSE },
	{ SDLK_F6,                    0,  0,  0,   CmdCheatKeyGreen,      FALSE },
	{ SDLK_F7,                    0,  0,  0,   CmdCheatBootsIce,      FALSE },
	{ SDLK_F8,                    0,  0,  0,   CmdCheatBootsSlide,    FALSE },
	{ SDLK_F9,                    0,  0,  0,   CmdCheatBootsFire,     FALSE },
	{ SDLK_F10,                   0,  0,  0,   CmdCheatBootsWater,    FALSE },
	{ '\003',                    -1, -1,  0,   CmdQuit,               FALSE },
	{ SDLK_F4,                    0,  0, +1,   CmdQuit,               FALSE },
	{ 0, 0, 0, 0, 0, 0 }
};
/* The list of key commands recognized when the program is obtaining
 * input from the user.
 */
static keycmdmap const inputkeycmds[] = {
	{ SDLK_UP,                   -1, -1,  0,   CmdNorth,              FALSE },
	{ SDLK_LEFT,                 -1, -1,  0,   CmdWest,               FALSE },
	{ SDLK_DOWN,                 -1, -1,  0,   CmdSouth,              FALSE },
	{ SDLK_RIGHT,                -1, -1,  0,   CmdEast,               FALSE },
	{ '\b',                      -1, -1,  0,   CmdWest,               FALSE },
	{ ' ',                       -1, -1,  0,   CmdEast,               FALSE },
	{ SDLK_RETURN,               -1, -1,  0,   CmdProceed,            FALSE },
	{ SDLK_KP_ENTER,             -1, -1,  0,   CmdProceed,            FALSE },
	{ SDLK_ESCAPE,               -1, -1,  0,   CmdQuitLevel,          FALSE },
	{ 'a',                       -1,  0,  0,   'a',                   FALSE },
	{ 'b',                       -1,  0,  0,   'b',                   FALSE },
	{ 'c',                       -1,  0,  0,   'c',                   FALSE },
	{ 'd',                       -1,  0,  0,   'd',                   FALSE },
	{ 'e',                       -1,  0,  0,   'e',                   FALSE },
	{ 'f',                       -1,  0,  0,   'f',                   FALSE },
	{ 'g',                       -1,  0,  0,   'g',                   FALSE },
	{ 'h',                       -1,  0,  0,   'h',                   FALSE },
	{ 'i',                       -1,  0,  0,   'i',                   FALSE },
	{ 'j',                       -1,  0,  0,   'j',                   FALSE },
	{ 'k',                       -1,  0,  0,   'k',                   FALSE },
	{ 'l',                       -1,  0,  0,   'l',                   FALSE },
	{ 'm',                       -1,  0,  0,   'm',                   FALSE },
	{ 'n',                       -1,  0,  0,   'n',                   FALSE },
	{ 'o',                       -1,  0,  0,   'o',                   FALSE },
	{ 'p',                       -1,  0,  0,   'p',                   FALSE },
	{ 'q',                       -1,  0,  0,   'q',                   FALSE },
	{ 'r',                       -1,  0,  0,   'r',                   FALSE },
	{ 's',                       -1,  0,  0,   's',                   FALSE },
	{ 't',                       -1,  0,  0,   't',                   FALSE },
	{ 'u',                       -1,  0,  0,   'u',                   FALSE },
	{ 'v',                       -1,  0,  0,   'v',                   FALSE },
	{ 'w',                       -1,  0,  0,   'w',                   FALSE },
	{ 'x',                       -1,  0,  0,   'x',                   FALSE },
	{ 'y',                       -1,  0,  0,   'y',                   FALSE },
	{ 'z',                       -1,  0,  0,   'z',                   FALSE },
	{ '\003',                    -1, -1,  0,   CmdQuit,               FALSE },
	{ SDLK_F4,                    0,  0, +1,   CmdQuit,               FALSE },
	{ 0, 0, 0, 0, 0, 0 }
};
#endif 


/* The current map of key commands.
 */
static keycmdmap const *keycmds = gamekeycmds;


/* A map of keys that can be held down simultaneously to produce
 * multiple commands.
 */
static int mergeable[CmdKeyMoveLast + 1];

/*
 * Running the keyboard's state machine.
 */

//DKS - added this hack function to fix power slider issues on GCW
//	Details: slider issues SDLK_PAUSE keypress before slider daemon
//	handles volume/lcd brightness adjustment, this somehow can interfere
//	with game's directional keystates after it is released.
//	Intended to be called from _keyeventcallback()
static void blankkeystates()
{
	memset(keystates, KS_OFF, sizeof keystates);
}


/* This callback is called whenever the state of any keyboard key
 * changes. It records this change in the keystates array. The key can
 * be recorded as being struck, pressed, repeating, held down, or down
 * but ignored, as appropriate to when they were first pressed and the
 * current behavior settings. Shift-type keys are always either on or
 * off.
 */
static void _keyeventcallback(int scancode, int down)
{
	//    switch (scancode) {
	//      case SDLK_LSHIFT:
	//      case SDLK_RSHIFT:
	//      case SDLK_LCTRL:
	//      case SDLK_RCTRL:
	//      case SDLK_LALT:
	//      case SDLK_RALT:
	//      case SDLK_LMETA:
	//      case SDLK_RMETA:
	//      case SDLK_NUMLOCK:
	//      case SDLK_CAPSLOCK:
	//      case SDLK_MODE:
	//	keystates[scancode] = down ? KS_ON : KS_OFF;
	//	break;
	//      default:

#ifdef PLATFORM_GCW
	//DKS - GCW hack to fix power slider interfering with keystates
	if (scancode == 0) // 0 is power slider scancode
	{
		blankkeystates();
	}
#endif

	if (scancode < SDLK_LAST) {
		if (down) {
			keystates[scancode] = keystates[scancode] == KS_OFF ?
				KS_PRESSED : KS_REPEATING;
		} else {
			keystates[scancode] = keystates[scancode] == KS_PRESSED ?
				KS_STRUCK : KS_OFF;
		}
	}
	//	break;
	//    }
}

/* Initialize (or re-initialize) all key states.
 */
static void restartkeystates(void)
{
	Uint8      *keyboard;
	int		count, n;

	memset(keystates, KS_OFF, sizeof keystates);
	keyboard = SDL_GetKeyState(&count);
	if (count > SDLK_LAST)
		count = SDLK_LAST;
	for (n = 0 ; n < count ; ++n)
		if (keyboard[n])
			_keyeventcallback(n, TRUE);
}

//DKS - my version of above
#ifdef PLATFORM_GP2X
/* Initialize (or re-initialize) all button states.
 */
static void restartbuttonstates(void)
{

	if (SDL_NumJoysticks()>0) {
		memset(bstates, BS_OFF, sizeof bstates);

		//DKS - might need this here despite events being enabled
		SDL_JoystickUpdate();

		int n;
		for (n = 0; n < NUMBUTTONS; ++n)
			if (SDL_JoystickGetButton(sdlg.joy, n)) {
				_joybuttoneventcallback(n, TRUE);
			}

	}
}
#endif

/* Update the key states. This is done at the start of each polling
 * cycle. The state changes that occur depend on the current behavior
 * settings.
 */
static void resetkeystates(void)
{
	/* The transition table for keys in joystick behavior mode.
	 */
	static char const joystick_trans[KS_count] = {
		/* KS_OFF         => */	KS_OFF,
		/* KS_ON          => */	KS_ON,
		/* KS_DOWN        => */	KS_DOWN,
		/* KS_STRUCK      => */	KS_OFF,
		/* KS_PRESSED     => */	KS_DOWN,
		/* KS_DOWNBUTOFF1 => */	KS_DOWN,
		/* KS_DOWNBUTOFF2 => */	KS_DOWN,
		/* KS_DOWNBUTOFF3 => */	KS_DOWN,
		/* KS_REPEATING   => */	KS_DOWN
	};
	/* The transition table for keys in keyboard behavior mode.
	 */
	static char const keyboard_trans[KS_count] = {
		/* KS_OFF         => */	KS_OFF,
		/* KS_ON          => */	KS_ON,
		/* KS_DOWN        => */	KS_DOWN,
		/* KS_STRUCK      => */	KS_OFF,
		/* KS_PRESSED     => */	KS_DOWNBUTOFF1,
		/* KS_DOWNBUTOFF1 => */	KS_DOWNBUTOFF2,
		/* KS_DOWNBUTOFF2 => */	KS_DOWN,
		/* KS_DOWNBUTOFF3 => */	KS_DOWN,
		/* KS_REPEATING   => */	KS_DOWN
	};

	char const *newstate;
	int		n;

	newstate = joystickstyle ? joystick_trans : keyboard_trans;
	for (n = 0 ; n < SDLK_LAST ; ++n)
		keystates[n] = newstate[(int)keystates[n]];

}

//DKS - my version of above for joypad buttons
#ifdef PLATFORM_GP2X
/* Update the button states. This is done at the start of each polling
 * cycle. The state changes that occur depend on the current behavior
 * settings.
 */
static void resetbuttonstates(void)
{
	/* The transition table for buttons in joystick behavior mode.
	 */
	static char const joystick_trans[BS_count] = {
		/* BS_OFF           => */ BS_OFF,
		/* BS_DOWN          => */ BS_DOWN,
		/* BS_STRUCK,		=> */ BS_OFF,
		/* BS_PRESSED,		=> */ BS_DOWN,
		/* BS_DOWNBUTOFF1,	=> */ BS_DOWN,
		/* BS_DOWNBUTOFF2,	=> */ BS_DOWN,
		/* BS_DOWNBUTOFF3,	=> */ BS_DOWN,
		/* BS_REPEATING     => */ BS_DOWN
	};

	/* The transition table for buttons in button behavior mode.
	 */
	static char const keyboard_trans[BS_count] = {
		/* BS_OFF         => */	BS_OFF,
		/* BS_DOWN        => */	BS_DOWN,
		/* BS_STRUCK      => */	BS_OFF,
		/* BS_PRESSED     => */	BS_DOWNBUTOFF1,
		/* BS_DOWNBUTOFF1 => */	BS_DOWNBUTOFF2,
		/* BS_DOWNBUTOFF2 => */	BS_DOWN,
		/* BS_DOWNBUTOFF3 => */	BS_DOWN,
		/* BS_REPEATING   => */ BS_DOWN
	};

	if (SDL_NumJoysticks()>0) {
		char const *newstate;
		int		n;

		newstate = joystickstyle ? joystick_trans : keyboard_trans;
		for (n = 0 ; n < NUMBUTTONS ; ++n)
			bstates[n] = newstate[(int)bstates[n]];
	}

}
#endif

//DKS
#ifdef PLATFORM_GP2X
//Query each button's state and update bstates array for either 1 or 0.
//Return 0 if nothing was pressed, 1 if any buttons pressed.
static int updatebuttons(void)
{

	int buttonpressed = 0;
	if (SDL_NumJoysticks()>0) {
		SDL_JoystickUpdate();
		int		n;
		for (n = 0 ; n < NUMBUTTONS ; ++n) {
			bstates[n] = SDL_JoystickGetButton(sdlg.joy, n);
			if ( bstates[n] )
				buttonpressed = 1;
		}

	}
	return buttonpressed;
}
#endif

/*
 * Mouse event functions.
 */

//DKS - modified
/* This callback is called whenever there is a state change in the
 * mouse buttons. Up events are ignored. Down events are stored to
 * be examined later.
 */
static void _mouseeventcallback(int xpos, int ypos, int button, int down)
{
#if !defined(PLATFORM_GP2X) && !defined(PLATFORM_GCW)
	if (down) {
		mouseinfo.state = KS_PRESSED;
		mouseinfo.x = xpos;
		mouseinfo.y = ypos;
		mouseinfo.button = button;
	}
#endif //PLATFORM_GP2X/GCW
}

//DKS - modified
/* Return the command appropriate to the most recent mouse activity.
 */
static int retrievemousecommand(void)
{
#if !defined(PLATFORM_GP2X) && !defined(PLATFORM_GCW)
	int	n;

	switch (mouseinfo.state) {
		case KS_PRESSED:
			mouseinfo.state = KS_OFF;
			if (mouseinfo.button == SDL_BUTTON_WHEELDOWN)
				return CmdNext;
			if (mouseinfo.button == SDL_BUTTON_WHEELUP)
				return CmdPrev;
			if (mouseinfo.button == SDL_BUTTON_LEFT) {
				n = windowmappos(mouseinfo.x, mouseinfo.y);
				if (n >= 0) {
					mouseinfo.state = KS_DOWNBUTOFF1;
					return CmdAbsMouseMoveFirst + n;
				}
			}
			break;
		case KS_DOWNBUTOFF1:
			mouseinfo.state = KS_DOWNBUTOFF2;
			return CmdPreserve;
		case KS_DOWNBUTOFF2:
			mouseinfo.state = KS_DOWNBUTOFF3;
			return CmdPreserve;
		case KS_DOWNBUTOFF3:
			mouseinfo.state = KS_OFF;
			return CmdPreserve;
	}
#endif //PLATFORM_GP2X/GCW
	return 0;
}

/*
 * Exported functions.
 */

//DKS - modified
/* Wait for any non-shift key to be pressed down, ignoring any keys
 * that may be down at the time the function is called. Return FALSE
 * if the key pressed is suggestive of a desire to quit.
 */
int anykey(void)
{
	int	n;

	resetkeystates();

#ifdef PLATFORM_GP2X
	resetbuttonstates();
#endif

	eventupdate(FALSE);
	for (;;) {
		resetkeystates();

#ifdef PLATFORM_GP2X
		resetbuttonstates();
		updatebuttons();
		if (bstates[GP2X_BUTTON_VOLUP]) {
			changevolume(+2, TRUE);
			controlleddelay(150);
			continue;
		} else if (bstates[GP2X_BUTTON_VOLDOWN]) {
			changevolume(-2, TRUE);
			controlleddelay(150);
			continue;
		}
#endif

		eventupdate(TRUE);

#ifdef PLATFORM_GP2X
		if (SDL_NumJoysticks()>0) {
			for (n = 0 ; n < NUMBUTTONS ; ++n) {
				if ( bstates[n] == BS_STRUCK || bstates[n] == BS_PRESSED
						|| bstates[n] == BS_REPEATING) {
					return n;
				}
			}
		}
#endif

		for (n = 0 ; n < SDLK_LAST ; ++n)
			if (keystates[n] == KS_STRUCK || keystates[n] == KS_PRESSED
					|| keystates[n] == KS_REPEATING)
				return n != 'q' && n != SDLK_ESCAPE;
	}
}

//DKS - modified
/* Poll the keyboard and return the command associated with the
 * selected key, if any. If no key is selected and wait is TRUE, block
 * until a key with an associated command is selected. In keyboard behavior
 * mode, the function can return CmdPreserve, indicating that if the key
 * command from the previous poll has not been processed, it should still
 * be considered active. If two mergeable keys are selected, the return
 * value will be the bitwise-or of their command values.
 */
int input(int wait)
{
	//this is all-new joystick stuff, but basically my version of what's below this
#ifdef PLATFORM_GP2X	
	if (SDL_NumJoysticks()>0) {
		buttoncmdmap	const	*bc;
		int			lingerflag = FALSE;
		int			cmd1, cmd, n;

		for (;;) {
			resetbuttonstates();
			eventupdate(wait);
			//add volume control stuff:
			if ((bstates[GP2X_BUTTON_VOLUP] == BS_PRESSED) || 
					(bstates[GP2X_BUTTON_VOLUP] == BS_REPEATING)  ||
					(bstates[GP2X_BUTTON_VOLUP] == BS_DOWN) ||
					(bstates[GP2X_BUTTON_VOLUP] == BS_DOWNBUTOFF1) ||
					(bstates[GP2X_BUTTON_VOLUP] == BS_DOWNBUTOFF2) || 
					(bstates[GP2X_BUTTON_VOLUP] == BS_DOWNBUTOFF3)) {
				changevolume(1, TRUE);
			} 
			if ((bstates[GP2X_BUTTON_VOLDOWN] == BS_PRESSED) || 
					(bstates[GP2X_BUTTON_VOLDOWN] == BS_REPEATING)  ||
					(bstates[GP2X_BUTTON_VOLDOWN] == BS_DOWN) ||
					(bstates[GP2X_BUTTON_VOLDOWN] == BS_DOWNBUTOFF1) ||
					(bstates[GP2X_BUTTON_VOLDOWN] == BS_DOWNBUTOFF2) || 
					(bstates[GP2X_BUTTON_VOLDOWN] == BS_DOWNBUTOFF3)) {
				changevolume(-1, TRUE);
			}
			cmd1 = cmd = 0;
			for (bc = buttoncmds ; (bc->buttoncode != -1); ++bc) {
				n = bstates[bc->buttoncode];
				if (!n)
					continue;
				if (n == BS_PRESSED || (bc->hold && n == BS_DOWN)) {
					if (!cmd1) {
						cmd1 = bc->cmd;
						if (!joystickstyle || cmd1 > CmdKeyMoveLast
								|| !mergeable[cmd1])
							return cmd1;
					} else {
						if (cmd1 <= CmdKeyMoveLast
								&& (mergeable[cmd1] & bc->cmd) == bc->cmd)
							return cmd1 | bc->cmd;
					}
				} else if (n == BS_STRUCK || n == BS_REPEATING) {
					cmd = bc->cmd;
				} else if (n == BS_DOWNBUTOFF1 || n == BS_DOWNBUTOFF2) {
					lingerflag = TRUE;
				}
			}
			if (cmd1)
				return cmd1;
			if (cmd)
				return cmd;

			if (!wait)
				break;
		}
		if (!cmd && lingerflag)
			cmd = CmdPreserve;
		return cmd;
	} 
#endif //PLATFORM_GP2X	


	keycmdmap const    *kc;
	int			lingerflag = FALSE;
	int			cmd1, cmd, n;

	for (;;) {

		resetkeystates();
		eventupdate(wait);

		cmd1 = cmd = 0;
		for (kc = keycmds ; kc->scancode ; ++kc) {
			n = keystates[kc->scancode];
			if (!n)
				continue;
#ifndef PLATFORM_GCW
//DKS - disabled for GCW ZERO:
	    if (kc->shift != -1)
		if (kc->shift !=
			(keystates[SDLK_LSHIFT] || keystates[SDLK_RSHIFT]))
		    continue;
	    if (kc->ctl != -1)
		if (kc->ctl !=
			(keystates[SDLK_LCTRL] || keystates[SDLK_RCTRL]))
		    continue;
	    if (kc->alt != -1)
		if (kc->alt != (keystates[SDLK_LALT] || keystates[SDLK_RALT]))
		    continue;
#endif

			if (n == KS_PRESSED || (kc->hold && n == KS_DOWN)) {
				if (!cmd1) {
					cmd1 = kc->cmd;
					if (!joystickstyle || cmd1 > CmdKeyMoveLast
							|| !mergeable[cmd1])
						return cmd1;
				} else {
					if (cmd1 <= CmdKeyMoveLast
							&& (mergeable[cmd1] & kc->cmd) == kc->cmd)
						return cmd1 | kc->cmd;
				}
			} else if (n == KS_STRUCK || n == KS_REPEATING) {
				cmd = kc->cmd;
			} else if (n == KS_DOWNBUTOFF1 || n == KS_DOWNBUTOFF2) {
				lingerflag = TRUE;
			}
		}
		if (cmd1)
			return cmd1;
		if (cmd)
			return cmd;
		cmd = retrievemousecommand();
		if (cmd)
			return cmd;
		if (!wait)
			break;
	}
	if (!cmd && lingerflag)
		cmd = CmdPreserve;
	return cmd;
}

/* Turn key-repeating on and off.
 */
int setkeyboardrepeat(int enable)
{
	if (enable)
		return SDL_EnableKeyRepeat(500, 75) == 0;
	else
		return SDL_EnableKeyRepeat(0, 0) == 0;
}

//DKS - modified
/* Turn joystick behavior mode on or off. In joystick-behavior mode,
 * the arrow keys are always returned from input() if they are down at
 * the time of the polling cycle. Other keys are only returned if they
 * are pressed during a polling cycle (or if they repeat, if keyboard
 * repeating is on). In keyboard-behavior mode, the arrow keys have a
 * special repeating behavior that is kept synchronized with the
 * polling cycle.
 */
int setkeyboardarrowsrepeat(int enable)
{
	joystickstyle = enable;
	restartkeystates();
	return TRUE;
}


//DKS - modified
/* Turn input mode on or off. When input mode is on, the input key
 * command map is used instead of the game key command map.
 */
int setkeyboardinputmode(int enable)
{
	keycmds = enable ? inputkeycmds : gamekeycmds;
#ifdef PLATFORM_GP2X
	//DKS - added for GP2X
	buttoncmds = enable ? inputbuttoncmds : gamebuttoncmds;
#endif
	return TRUE;
}

//DKS - modified
/* Initialization.
 */
int _sdlinputinitialize(void)
{
	sdlg.keyeventcallbackfunc = _keyeventcallback;
	sdlg.mouseeventcallbackfunc = _mouseeventcallback;

	mergeable[CmdNorth] = mergeable[CmdSouth] = CmdWest | CmdEast;
	mergeable[CmdWest] = mergeable[CmdEast] = CmdNorth | CmdSouth;

	//dks - modifying for GCW:
#ifdef PLATFORM_GCW
	setkeyboardrepeat(FALSE);
#else
	setkeyboardrepeat(TRUE);
#endif
	SDL_EnableUNICODE(TRUE);

#ifdef PLATFORM_GP2X
	//DKS added for joystick
	sdlg.joybuttoneventcallbackfunc = _joybuttoneventcallback;
#endif

	//DKS
	sdlg.joy = NULL;
	sdlg.deadzone = 15000; // sensible value for GCW 
#ifdef PLATFORM_GP2X 
	/* Initialize the joystick subsystem */
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);

	// Check for joystick
	if(SDL_NumJoysticks() > 0){
		// Open joystick
		sdlg.joy=SDL_JoystickOpen(0);
	}
#elif defined (PLATFORM_GCW)
	/* Initialize the joystick subsystem */
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);

	// Check for joystick
	if(SDL_NumJoysticks() > 0){
		// Open joystick
		sdlg.joy=SDL_JoystickOpen(0);
	}
	if (sdlg.joy) printf("OPENED JOY1\n");
#endif

	return TRUE;
}

/* Online help texts for the keyboard commands.
 */
tablespec const *keyboardhelp(int which)
{
	static char *ingame_items[] = {
		"1-arrows", "1-move Chip",
		"1-2 4 6 8 (keypad)", "1-also move Chip",
		"1-Q", "1-quit the current game",
		"1-Bkspc", "1-pause the game",
		"1-Ctrl-R", "1-restart the current level",
		"1-Ctrl-P", "1-jump to the previous level",
		"1-Ctrl-N", "1-jump to the next level",
		"1-V", "1-decrease volume",
		"1-Shift-V", "1-increase volume",
		"1-Ctrl-C", "1-exit the program",
		"1-Alt-F4", "1-exit the program"
	};
	static tablespec const keyhelp_ingame = { 11, 2, 4, 1, ingame_items };

	static char *twixtgame_items[] = {
		"1-P", "1-jump to the previous level",
		"1-N", "1-jump to the next level",
		"1-PgUp", "1-skip back ten levels",
		"1-PgDn", "1-skip ahead ten levels",
		"1-G", "1-go to a level using a password",
		"1-S", "1-see the scores for each level",
		"1-Tab", "1-playback saved solution",
		"1-Shift-Tab", "1-verify saved solution",
		"1-Ctrl-X", "1-replace existing solution",
		"1-Shift-Ctrl-X", "1-delete existing solution",
		"1-Ctrl-S", "1-see the available solution files",
		"1-O", "1-toggle between even-step and odd-step offset",
		"1-Shift-O", "1-increment stepping offset (Lynx only)",
		"1-V", "1-decrease volume",
		"1-Shift-V", "1-increase volume",
		"1-Q", "1-return to the file list",
		"1-Ctrl-C", "1-exit the program",
		"1-Alt-F4", "1-exit the program"
	};
	static tablespec const keyhelp_twixtgame = { 18, 2, 4, 1,
		twixtgame_items };

	static char *scorelist_items[] = {
		"1-up down", "1-move selection",
		"1-PgUp PgDn", "1-scroll selection",
		"1-Enter Space", "1-select level",
		"1-Ctrl-S", "1-change solution file",
		"1-Q", "1-return to the last level",
		"1-Ctrl-C", "1-exit the program",
		"1-Alt-F4", "1-exit the program"
	};
	static tablespec const keyhelp_scorelist = { 7, 2, 4, 1, scorelist_items };

	static char *scroll_items[] = {
		"1-up down", "1-move selection",
		"1-PgUp PgDn", "1-scroll selection",
		"1-Enter Space", "1-select",
		"1-Q", "1-cancel",
		"1-Ctrl-C", "1-exit the program",
		"1-Alt-F4", "1-exit the program"
	};
	static tablespec const keyhelp_scroll = { 6, 2, 4, 1, scroll_items };

	switch (which) {
		case KEYHELP_INGAME:	return &keyhelp_ingame;
		case KEYHELP_TWIXTGAMES:	return &keyhelp_twixtgame;
		case KEYHELP_SCORELIST:	return &keyhelp_scorelist;
		case KEYHELP_FILELIST:	return &keyhelp_scroll;
	}

	return NULL;
}
