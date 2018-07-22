/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string.h>
#include <stdio.h>

#include "main.h"
#include "dface.h"
#include "input.h"
#include "config.h"

#include "dingoo-video.h"
#include "dingoo.h"

#include "gui/gui.h"

#include "../common/cheat.h"
#include "../../movie.h"
#include "../../fceu.h"
#include "../../driver.h"
#include "../../utils/xstring.h"
#ifdef _S9XLUA_H
#include "../../fceulua.h"
#endif

#ifdef _GTK_LITE
#include<gtk/gtk.h>
#endif

/** GLOBALS **/
int NoWaiting = 1;
extern Config *g_config;
extern bool bindSavestate, frameAdvanceLagSkip, lagCounterDisplay;

/* UsrInputType[] is user-specified.  InputType[] is current
 (game loading can override user settings)
 */
static int UsrInputType[NUM_INPUT_DEVICES];
static int InputType[NUM_INPUT_DEVICES];
static int cspec = 0;

extern int gametype;

static bool MenuRequested = false;

static int frameAdvanceKey = 0;

/**
 * Necessary for proper GUI functioning (configuring when a game isn't loaded).
 */
void InputUserActiveFix() {
	int x;
	for (x = 0; x < 3; x++) {
		InputType[x] = UsrInputType[x];
	}
}

/**
 * Parse game information and configure the input devices accordingly.
 */
void ParseGIInput(FCEUGI *gi) {
	gametype = gi->type;

	InputType[0] = UsrInputType[0];
	InputType[1] = UsrInputType[1];
	InputType[2] = UsrInputType[2];

	if (gi->input[0] >= 0) {
		InputType[0] = gi->input[0];
	}
	if (gi->input[1] >= 0) {
		InputType[1] = gi->input[1];
	}
	if (gi->inputfc >= 0) {
		InputType[2] = gi->inputfc;
	}
	cspec = gi->cspecial;
}

static uint8 QuizKingData = 0;
static uint8 HyperShotData = 0;
// static uint32 MahjongData   = 0;
// static uint32 FTrainerData  = 0;
static uint8 TopRiderData = 0;
// static uint8  BWorldData[1+13+1];

// static void UpdateFKB(void);
static void UpdateGamepad(void);
static void UpdateQuizKing(void);
static void UpdateHyperShot(void);
// static void UpdateMahjong(void);
//static void UpdateFTrainer(void);
static void UpdateTopRider(void);

static uint32 JSreturn = 0;

/**
 * Configure cheat devices (game genie, etc.).  Restarts the keyboard
 * and video subsystems.
 */
static void DoCheatSeq() {
	SilenceSound(1);
	KillVideo();

	DoConsoleCheatConfig();
	InitVideo(GameInfo);
	SilenceSound(0);
}

#include "keyscan.h"
static uint8 *g_keyState = 0;
static int    DIPS = 0;

static uint8 keyonce[MKK_COUNT];
#define KEY(__a) g_keyState[MKK(__a)]
#define keyonly(__a) _keyonly(MKK(__a))

static int _keyonly(int a)
{
	// check for valid key
	if(a > SDLK_LAST+1 || a < 0)
		return(0);
	#if SDL_VERSION_ATLEAST(1, 3, 0)
    if(g_keyState[SDL_GetScancodeFromKey((SDLKey)a)])
    #else
    if(g_keyState[a])
    #endif
    {
	
        if(!keyonce[a]) {
            keyonce[a] = 1;
            return(1);
        }
    } else {
        keyonce[a] = 0;
    }
    return(0);
}

// a hack to check DINGOO_R hotkey combo
static int ispressed(int sdlk_code)
{
	Uint8 *keystate = SDL_GetKeyState(NULL);

	if(keystate[sdlk_code]) return 1;

	return 0;
}

// mega hack to reset once pressed key
static int resetkey(int sdlk_code)
{
	Uint8 *keystate = SDL_GetKeyState(NULL);
	keystate[sdlk_code] = 0;
}

static int g_fkbEnabled = 0;

// this function loads the sdl hotkeys from the config file into the
// global scope.  this elimates the need for accessing the config file

int Hotkeys[HK_MAX] = { 0 };

// on every cycle of keyboardinput()
void setHotKeys()
{
	std::string prefix = "SDL.Hotkeys.";
	for (int i = 0; i < HK_MAX; i++) {
		g_config->getOption(prefix + HotkeyStrings[i], &Hotkeys[i]);
	}

	//frameAdvanceKey = DINGOO_R | Hotkeys[HK_FRAME_ADVANCE];
	return;
}

/**
 * Lets the user start a new .fm2 movie file
 **/
void FCEUD_MovieRecordTo() {
#if 0
	std::string fname = GetFilename("Save FM2 movie for recording", true, "FM2 movies|*.fm2");
	if (!fname.size())
	return; // no filename selected, quit the whole thing
	std::wstring author = mbstowcs(GetUserText("Author name")); // the author can be empty, so no need to check here

	FCEUI_SaveMovie(fname.c_str(), MOVIE_FLAG_FROM_POWERON, author);
#endif
}

/**
 * Lets the user save a savestate to a specific file
 **/
void FCEUD_SaveStateAs() {
#if 0
	std::string fname = GetFilename("Save savestate as...", true, "Savestates|*.fc0");
	if (!fname.size())
	return; // no filename selected, quit the whole thing

	FCEUI_SaveState(fname.c_str());
#endif
}

/**
 * Lets the user load a savestate from a specific file
 */
void FCEUD_LoadStateFrom() {
#if 0
	std::string fname = GetFilename("Load savestate from...", false, "Savestates|*.fc?");
	if (!fname.size())
	return; // no filename selected, quit the whole thing

	FCEUI_LoadState(fname.c_str());
#endif
}

/**
* Hook for transformer board
*/
unsigned int *GetKeyboard(void)                                                     
{
  int size = 256;
  Uint8* keystate = SDL_GetKeyState(&size);
  return (unsigned int*)(keystate);
}

/**
 * Parse hotkey commands and execute accordingly.
 */
static void KeyboardCommands() {
    int is_shift, is_alt;

    // get the keyboard input
    #if SDL_VERSION_ATLEAST(1, 3, 0)	 
    g_keyState = SDL_GetKeyboardState(NULL);	 
    #else
    g_keyState = SDL_GetKeyState(NULL);
	#endif

	/* NOT SUPPORTED
	 // check if the family keyboard is enabled
    if(InputType[2] == SIFC_FKB) {
        if(keyonly(SCROLLLOCK)) {
            g_fkbEnabled ^= 1;
            FCEUI_DispMessage("Family Keyboard %sabled.", 0,
                              g_fkbEnabled ? "en" : "dis");
        }
        SDL_WM_GrabInput(g_fkbEnabled ? SDL_GRAB_ON : SDL_GRAB_OFF);
        if(g_fkbEnabled) {
            return;
        }
    }
	 }*/

	#if SDL_VERSION_ATLEAST(1, 3, 0)
	if(g_keyState[SDL_GetScancodeFromKey(SDLK_LSHIFT)] || g_keyState[SDL_GetScancodeFromKey(SDLK_RSHIFT)])
	#else
	if(g_keyState[SDLK_LSHIFT] || g_keyState[SDLK_RSHIFT])
	#endif
		is_shift = 1;
	else
		is_shift = 0;
	#if SDL_VERSION_ATLEAST(1, 3, 0)
	if(g_keyState[SDL_GetScancodeFromKey(SDLK_LALT)] || g_keyState[SDL_GetScancodeFromKey(SDLK_RALT)])
	#else
	if(g_keyState[SDLK_LALT] || g_keyState[SDLK_RALT])
	#endif
		is_alt = 1;
	else
		is_alt = 0;

	if (_keyonly(Hotkeys[HK_QUIT])) {
		CloseGame();
		exit(0);
	}

	if (_keyonly(Hotkeys[HK_TOGGLE_BG])) {
		static int state = 1;
		state = !state;
		FCEUI_SetRenderPlanes(true, state);
	}

	// L (SDLK_TAB), Start+Select or Power flick (SDLK_HOME) - enter GUI
	if (_keyonly(DINGOO_L)
	 || _keyonly(DINGOO_MENU)
	 || MenuRequested
	 || (ispressed(DINGOO_START) && ispressed(DINGOO_SELECT))) {
		SilenceSound(1);
		MenuRequested = false;
		FCEUGUI_Run();
		SilenceSound(0);
		return;
	}

	// R shift + combokeys
	if(ispressed(DINGOO_R)) {
		extern int g_slot; // import from gui.cpp
		void save_preview(); // import from gui.cpp
		if(_keyonly(DINGOO_A)) { // R + A  save state
			FCEUI_SelectState(g_slot, 0);
			FCEUI_SaveState(NULL);
			save_preview();
			resetkey(DINGOO_A);
		}
		if(_keyonly(DINGOO_B)) { // R + B  load state
			FCEUI_SelectState(g_slot, 0);
			FCEUI_LoadState(NULL);
			resetkey(DINGOO_B);
		}
		if(_keyonly(DINGOO_X)) { // R + X  toggle fullscreen
			extern int s_fullscreen; // from dingoo_video.cpp
			s_fullscreen = (s_fullscreen + 1) % 4;
			g_config->setOption("SDL.Fullscreen", s_fullscreen);
			dingoo_clear_video();
			resetkey(DINGOO_X);
		}
		if(_keyonly(DINGOO_Y)) { // R + Y  flip fds disk
			if(gametype == GIT_FDS) FCEUI_FDSFlip();
			resetkey(DINGOO_Y);
		}
		if(_keyonly(DINGOO_UP)) { // R + UP tooggle fps show
			extern int showfps; // from dingoo.cpp
			showfps ^= 1;
			g_config->setOption("SDL.ShowFPS", showfps);
			resetkey(DINGOO_UP);
		}
		if(_keyonly(DINGOO_DOWN)) {// R + DOWN activate subtitle display (??) is this really needed
			resetkey(DINGOO_DOWN);
		}
		if(_keyonly(DINGOO_LEFT)) { // R + LEFT  insert vsuini coin
			if (gametype == GIT_VSUNI) FCEUI_VSUniCoin();
			resetkey(DINGOO_LEFT);
		}
		if(_keyonly(DINGOO_RIGHT)) { // R + RIGHT  frame advancing (??)
			resetkey(DINGOO_RIGHT);
		}
		if(_keyonly(DINGOO_SELECT)) { // R + SELECT
			FCEUI_SaveSnapshot();
			resetkey(DINGOO_SELECT);
		}
		if(_keyonly(DINGOO_START)) { // R + START  pause emulation
			FCEUI_ToggleEmulationPause();
			resetkey(DINGOO_START);
		}
	}

	/*
	 // Toggle Movie auto-backup
	 if(keyonly(M) && is_shift) {
	 autoMovieBackup ^= 1;
	 FCEUI_DispMessage("Automatic movie backup %sabled.", 0,
	 autoMovieBackup ? "en" : "dis");
	 }
	 */

	/* NOT SUPPORTED
	 // Start recording an FM2 movie on Alt+R
	 if(keyonly(R) && is_alt) {
	 FCEUD_MovieRecordTo();
	 }
	 */

	// Famicom disk-system games
	if (gametype == GIT_FDS) {
		if (_keyonly(Hotkeys[HK_FDS_FLIP]))
			FCEUI_FDSFlip();
		if (_keyonly(Hotkeys[HK_FDS_SELECT]))
			FCEUI_FDSSelect();
		if (_keyonly(Hotkeys[HK_FDS_EJECT]))
			FCEUI_FDSInsert();
	}

	// if not NES Sound Format
	if (gametype != GIT_NSF) {
		if (_keyonly(Hotkeys[HK_CHEAT_MENU]))
			DoCheatSeq();

		if (_keyonly(Hotkeys[HK_SAVE_STATE])) {
			/*if(is_shift) {
			 movie_fname = const_cast<char*>(FCEU_MakeFName(FCEUMKF_MOVIE, 0, 0).c_str());
			 FCEUI_printf("Recording movie to %s\n", movie_fname);
			 FCEUI_SaveMovie(movie_fname, MOVIE_FLAG_NONE, "");
			 } else */
			FCEUI_SaveState(NULL);
		}

		// f7 to load state, Shift-f7 to load movie
		if (_keyonly(Hotkeys[HK_LOAD_STATE])) {
			/*
			 if(is_shift) {
			 FCEUI_StopMovie();
			 std::string fname;
			 fname = GetFilename("Open FM2 movie for playback...", false, "FM2 movies|*.fm2");
			 if(fname != "") {
			 if(fname.find(".fm2") != std::string::npos) {
			 FCEUI_printf("Playing back movie located at %s\n", fname.c_str());
			 FCEUI_LoadMovie(fname.c_str(), false, false, false);
			 } else
			 FCEUI_printf("Only FM2 movies are supported.\n");
			 }
			 } else */
			FCEUI_LoadState(NULL);
		}
	}

	if (_keyonly(Hotkeys[HK_DECREASE_SPEED]))
		DecreaseEmulationSpeed();

	if (_keyonly(Hotkeys[HK_INCREASE_SPEED]))
		IncreaseEmulationSpeed();

	if (_keyonly(Hotkeys[HK_TOGGLE_FRAME_DISPLAY]))
		FCEUI_MovieToggleFrameDisplay();

	if (_keyonly(Hotkeys[HK_TOGGLE_INPUT_DISPLAY])) {
		FCEUI_ToggleInputDisplay();
		extern int input_display;
		g_config->setOption("SDL.InputDisplay", input_display);
	}

	if (_keyonly(Hotkeys[HK_MOVIE_TOGGLE_RW]))
		FCEUI_SetMovieToggleReadOnly(!FCEUI_GetMovieToggleReadOnly());

#ifdef CREATE_AVI
	if(_keyonly(Hotkeys[HK_MUTE_CAPTURE])) {
		extern int mutecapture;
		mutecapture ^= 1;
	}
#endif

	if (_keyonly(Hotkeys[HK_PAUSE]))
		FCEUI_ToggleEmulationPause();

	// Toggle throttling
	NoWaiting &= ~1;
	//if (dingoo_key & 1 << Hotkeys[HK_TURBO])
	//	NoWaiting |= 1;
/*
	static bool frameAdvancing = false;
	if ((dingoo_key & frameAdvanceKey) == frameAdvanceKey) {
		if (frameAdvancing == false) {
			FCEUI_FrameAdvance();
			frameAdvancing = true;
		}
	} else {
		if (frameAdvancing) {
			FCEUI_FrameAdvanceEnd();
			frameAdvancing = false;
		}
	}
*/
	if (_keyonly(Hotkeys[HK_RESET]))
		FCEUI_ResetNES();

	//if(_keyonly(Hotkeys[HK_POWER]))
	//	FCEUI_PowerNES();

	if (_keyonly(Hotkeys[HK_QUIT]))
		CloseGame();

#ifdef _S9XLUA_H
	if(_keyonly(Hotkeys[HK_LOAD_LUA])) {
		std::string fname;
		fname = GetFilename("Open LUA script...", false, "Lua scripts|*.lua");
		if(fname != "")
		FCEU_LoadLuaCode(fname.c_str());
	}
#endif

	for (int i = 0; i < 10; i++)
		if (_keyonly(Hotkeys[HK_SELECT_STATE_0 + i]))
			FCEUI_SelectState(i, 1);

	if (_keyonly(Hotkeys[HK_BIND_STATE])) {
		bindSavestate ^= 1;
		FCEUI_DispMessage("Savestate binding to movie %sabled.", 0,
				bindSavestate ? "en" : "dis");
	}

	if (_keyonly(Hotkeys[HK_FA_LAG_SKIP])) {
		frameAdvanceLagSkip ^= 1;
		FCEUI_DispMessage("Skipping lag in Frame Advance %sabled.", 0,
				frameAdvanceLagSkip ? "en" : "dis");
	}

	if (_keyonly(Hotkeys[HK_LAG_COUNTER_DISPLAY]))
		lagCounterDisplay ^= 1;
#if 0
	if (_keyonly(Hotkeys[HK_TOGGLE_SUBTITLE])) {
		extern int movieSubtitles;
		movieSubtitles ^= 1;
		FCEUI_DispMessage("Movie subtitles o%s.", 0, movieSubtitles ? "n" : "ff");
	}
#endif
	// VS Unisystem games
	if (gametype == GIT_VSUNI) {
		// insert coin
		if (_keyonly(Hotkeys[HK_VS_INSERT_COIN]))
			FCEUI_VSUniCoin();

		/* NOT SUPPORTED
		 // toggle dipswitch display
		 if(_keyonly(Hotkeys[HK_VS_TOGGLE_DIPSWITCH])) {
		 DIPS^=1;
		 FCEUI_VSUniToggleDIPView();
		 }
		 if(!(DIPS&1))
		 goto DIPSless;

		 // toggle the various dipswitches
		 if(keyonly(1)) FCEUI_VSUniToggleDIP(0);
		 if(keyonly(2)) FCEUI_VSUniToggleDIP(1);
		 if(keyonly(3)) FCEUI_VSUniToggleDIP(2);
		 if(keyonly(4)) FCEUI_VSUniToggleDIP(3);
		 if(keyonly(5)) FCEUI_VSUniToggleDIP(4);
		 if(keyonly(6)) FCEUI_VSUniToggleDIP(5);
		 if(keyonly(7)) FCEUI_VSUniToggleDIP(6);
		 if(keyonly(8)) FCEUI_VSUniToggleDIP(7);
		 */
	} else {
		/* NOT SUPPORTED
		 static uint8 bbuf[32];
		 static int bbuft;
		 static int barcoder = 0;
		 */

		/* NOT SUPPORTED
		 if(keyonly(H)) FCEUI_NTSCSELHUE();
		 if(keyonly(T)) FCEUI_NTSCSELTINT();
		 */

		if (_keyonly(Hotkeys[HK_DECREASE_SPEED]))
			FCEUI_NTSCDEC();
		if (_keyonly(Hotkeys[HK_INCREASE_SPEED]))
			FCEUI_NTSCINC();

		/* NOT SUPPORTED 
		 if((InputType[2] == SIFC_BWORLD) || (cspec == SIS_DATACH)) {
		 if(keyonly(F8)) {
		 barcoder ^= 1;
		 if(!barcoder) {
		 if(InputType[2] == SIFC_BWORLD) {
		 strcpy((char *)&BWorldData[1], (char *)bbuf);
		 BWorldData[0] = 1;
		 } else {
		 FCEUI_GetDatachSet(bbuf);
		 }
		 FCEUI_DispMessage("Barcode Entered", 0);
		 } else {
		 bbuft = 0;
		 FCEUI_DispMessage("Enter Barcode", 0);
		 }
		 }
		 } else {
		 barcoder = 0;
		 }

		 #define SSM(x)                                    \
		do {                                              \
		    if(barcoder) {                                \
			if(bbuft < 13) {                          \
			    bbuf[bbuft++] = '0' + x;              \
			    bbuf[bbuft] = 0;                      \
			}                                         \
			FCEUI_DispMessage("Barcode: %s", 0, bbuf);   \
			}                                             \
		} while(0)

		 DIPSless:
		 if(keyonly(0)) SSM(0);
		 if(keyonly(1)) SSM(1);
		 if(keyonly(2)) SSM(2);
		 if(keyonly(3)) SSM(3);
		 if(keyonly(4)) SSM(4);
		 if(keyonly(5)) SSM(5);
		 if(keyonly(6)) SSM(6);
		 if(keyonly(7)) SSM(7);
		 if(keyonly(8)) SSM(8);
		 if(keyonly(9)) SSM(9);
		 #undef SSM */
	}
}

/**
 * Return the state of the mouse buttons.  Input 'd' is an array of 3
 * integers that store <x, y, button state>.
 */
void // removed static for a call in lua-engine.cpp
GetMouseData(uint32(&d)[3])
{
    int x,y;
    uint32 t;

    // Don't get input when a movie is playing back
    if(FCEUMOV_Mode(MOVIEMODE_PLAY))
        return;

    // retrieve the state of the mouse from SDL
    t = SDL_GetMouseState(&x, &y);

    d[2] = 0;
    if(t & SDL_BUTTON(1)) {
        d[2] |= 0x1;
    }
    if(t & SDL_BUTTON(3)) {
        d[2] |= 0x2;
    }

    // get the mouse position from the SDL video driver
    t = PtoV(x, y);
    d[0] = t & 0xFFFF;
    d[1] = (t >> 16) & 0xFFFF;
}

/**
 * Handles outstanding SDL events.
 */
static void UpdatePhysicalInput()
{
    SDL_Event event;

    // loop, handling all pending events
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
        case SDL_QUIT:
            CloseGame();
            puts("Quit");
            break;
        case SDL_FCEU_HOTKEY_EVENT:
            switch(event.user.code) {
                case HK_PAUSE:
                    FCEUI_ToggleEmulationPause();
                    break;
                default:
                    FCEU_printf("Warning: unknown hotkey event %d\n", event.user.code);
            }
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == DINGOO_MENU)
                // Because a KEYUP is sent straight after the KEYDOWN for the
                // Power switch, SDL_GetKeyState will not ever see this.
                // Keep a record of it.
                MenuRequested = true;
            break;
        default:
            // do nothing
            break;
        }
    }
    //SDL_PumpEvents();
}

/**
 * Tests to see if a specified button is currently pressed.
 */
static int DTestButton(ButtConfig *bc){
    int x;

    for(x = 0; x < bc->NumC; x++) {
        if(bc->ButtType[x] == BUTTC_KEYBOARD) {
			#if SDL_VERSION_ATLEAST(1,3,0)
			if(g_keyState[SDL_GetScancodeFromKey((SDLKey)bc->ButtonNum[x])]) {
			#else
            if(g_keyState[bc->ButtonNum[x]]) {
			#endif
                return(1);
            }
        } else if(bc->ButtType[x] == BUTTC_JOYSTICK) {
            if(DTestButtonJoy(bc)) {
                return(1);
            }
        }
    }
    return(0);
}

#define MK(x)       {{BUTTC_KEYBOARD},{0},{MKK(x)},1}
#define MK2(x1,x2)  {{BUTTC_KEYBOARD},{0},{MKK(x1),MKK(x2)},2}
#define MKZ()       {{0},{0},{0},0}
#define GPZ()       {MKZ(), MKZ(), MKZ(), MKZ()}

//#define MK(x)       {{BUTTC_JOYSTICK},{0},{x},1}
//#define MK2(x1,x2)  {{BUTTC_JOYSTICK},{0},{x1,x2},2}
//#define MKZ()       {{-1},{-1},{-1},-1}
//#define GPZ()       {MKZ(), MKZ(), MKZ(), MKZ()}

ButtConfig GamePadConfig[4][10] = {
		/* Gamepad 1 */
		{ MK(LCTRL), MK(LALT), MK(ESCAPE), MK(RETURN), MK(UP), MK(DOWN), MK(LEFT), MK(RIGHT),
				MK(SPACE), MK(LSHIFT) },

		/* Gamepad 2 */
		GPZ(),

		/* Gamepad 3 */
		GPZ(),

		/* Gamepad 4 */
		GPZ() };

/**
 * Update the status of the gamepad input devices.
 */
static void UpdateGamepad(void) {
	// don't update during movie playback
	if (FCEUMOV_Mode(MOVIEMODE_PLAY)) {
		return;
	}

	static int rapid = 0;
	uint32 JS = 0;
	int x;
	int wg;
	int merge;

	rapid ^= 1;

	g_config->getOption("SDL.MergeControls", &merge);

	// go through just one device for the little dingoo :)
	for (wg = 0; wg < 1; wg++) {
		// the 4 directional buttons, start, select, a, b
		for (x = 0; x < 8; x++) {
			if (DTestButton(&GamePadConfig[wg][x])) {
				JS |= (1 << x) << (wg << 3);
				// Inject gamepad 1 input into gamepad 2 if enabled
				if (merge) {
					JS |= (1 << x) << ((wg + 1) << 3);
				}
			}
		}

		// rapid-fire a, rapid-fire b
		if (rapid) {
			for (x = 0; x < 2; x++) {
				if (DTestButton(&GamePadConfig[wg][8 + x])) {
					JS |= (1 << x) << (wg << 3);
					// Inject gamepad 1 input into gamepad 2 if enabled
					if (merge) {
						JS |= (1 << x) << ((wg + 1) << 3);
					}
				}
			}
		}
	}

	//  for(x=0;x<32;x+=8) {	/* Now, test to see if anything weird(up+down at same time)
	//			   is happening, and correct */
	//		if((JS & (0xC0<<x) ) == (0xC0<<x) ) JS&=~(0xC0<<x);
	//		if((JS & (0x30<<x) ) == (0x30<<x) ) JS&=~(0x30<<x);
	//  }

	JSreturn = JS;
}

/* NOT SUPPORTED */
#if 0
static ButtConfig powerpadsc[2][12]= {
	{
		{
			MK(-1),MK(-1),MK(-1),MK(-1),
			MK(-1),MK(-1),MK(-1),MK(-1),
			MK(-1),MK(-1),MK(-1),MK(-1)
		},
		{
			MK(-1),MK(-1),MK(-1),MK(-1),
			MK(-1),MK(-1),MK(-1),MK(-1),
			MK(-1),MK(-1),MK(-1),MK(-1)
		}
	};

	static uint32 powerpadbuf[2]= {0,0};

	/**
	 * Update the status of the power pad input device.
	 */
	static uint32
	UpdatePPadData(int w)
	{
		// don't update if a movie is playing
		if(FCEUMOV_Mode(MOVIEMODE_PLAY)) {
			return 0;
		}

		uint32 r = 0;
		ButtConfig *ppadtsc = powerpadsc[w];
		int x;

		// update each of the 12 buttons
		for(x = 0; x < 12; x++) {
			if(DTestButton(&ppadtsc[x])) {
				r |= 1 << x;
			}
		}

		return r;
	}
#endif

static uint32 MouseData[3] = { 0, 0, 0 };
static uint8 fkbkeys[0x48];

/**
 * Update all of the input devices required for the active game.
 */
void FCEUD_UpdateInput() {
	int x;
	int t = 0;

	UpdatePhysicalInput();
	KeyboardCommands();

	for (x = 0; x < 2; x++) {
		switch (InputType[x]) {
		case SI_GAMEPAD:
			t |= 1;
			break;
		case SI_ARKANOID:
			t |= 2;
			break;
		case SI_ZAPPER:
			t |= 2;
			break;
#if 0
			case SI_POWERPADA:
			case SI_POWERPADB:
			powerpadbuf[x] = UpdatePPadData(x);
			break;
#endif
		}
	}

	switch (InputType[2]) {
	case SIFC_ARKANOID:
		t |= 2;
		break;
	case SIFC_SHADOW:
		t |= 2;
		break;
#if 0
		case SIFC_FKB:
		if(g_fkbEnabled)
		UpdateFKB();
		break;
#endif
	case SIFC_HYPERSHOT:
		UpdateHyperShot();
		break;
#if 0
		case SIFC_MAHJONG:
		UpdateMahjong();
		break;
#endif

	case SIFC_QUIZKING:
		UpdateQuizKing();
		break;
#if 0
		case SIFC_FTRAINERB:
		case SIFC_FTRAINERA:
		UpdateFTrainer();
		break;
#endif
	case SIFC_TOPRIDER:
		UpdateTopRider();
		break;
	case SIFC_OEKAKIDS:
		t |= 2;
		break;
	}

	if (t & 1)
		UpdateGamepad();

	if (t & 2)
		GetMouseData(MouseData);
}

void FCEUD_SetInput(bool fourscore, bool microphone, ESI port0, ESI port1, ESIFC fcexp) {
	eoptions &= ~EO_FOURSCORE;
	if (fourscore) { // Four Score emulation, only support gamepads, nothing else
		eoptions |= EO_FOURSCORE;
		InputType[0] = SI_GAMEPAD; // Controllers 1 and 3
		InputType[1] = SI_GAMEPAD; // Controllers 2 and 4
		InputType[2] = SIFC_NONE; // No extension
	} else { // no Four Core emulation, check the config/movie file for controller types
		InputType[0] = port0;
		InputType[1] = port1;
		InputType[2] = fcexp;
	}

	//replaceP2StartWithMicrophone = microphone;

	InitInputInterface();
}


/**
 * Initialize the input device interface between the emulation and the driver.
 */
void InitInputInterface() {
	void *InputDPtr;

	int t;
	int x;
	int attrib;

	for (t = 0, x = 0; x < 2; x++) {
		attrib = 0;
		InputDPtr = 0;

		switch (InputType[x]) {
#if 0
		case SI_POWERPADA:
		case SI_POWERPADB:
		InputDPtr = &powerpadbuf[x];
		break;
#endif
		case SI_GAMEPAD:
			InputDPtr = &JSreturn;
			break;
		case SI_ARKANOID:
			InputDPtr = MouseData;
			t |= 1;
			break;
		case SI_ZAPPER:
			InputDPtr = MouseData;
			t |= 1;
			attrib = 1;
			break;
		}
		FCEUI_SetInput(x, (ESI) InputType[x], InputDPtr, attrib);
	}

	attrib = 0;
	InputDPtr = 0;
	switch (InputType[2]) {
	case SIFC_SHADOW:
		InputDPtr = MouseData;
		t |= 1;
		attrib = 1;
		break;
	case SIFC_OEKAKIDS:
		InputDPtr = MouseData;
		t |= 1;
		attrib = 1;
		break;
	case SIFC_ARKANOID:
		InputDPtr = MouseData;
		t |= 1;
		break;
#if 0
		case SIFC_FKB:
		InputDPtr = fkbkeys;
		break;
#endif
	case SIFC_HYPERSHOT:
		InputDPtr = &HyperShotData;
		break;
#if 0
		case SIFC_MAHJONG:
		InputDPtr = &MahjongData;
		break;
#endif
	case SIFC_QUIZKING:
		InputDPtr = &QuizKingData;
		break;
	case SIFC_TOPRIDER:
		InputDPtr = &TopRiderData;
		break;
#if 0
		case SIFC_BWORLD:
		InputDPtr = BWorldData;
		break;
		case SIFC_FTRAINERA:
		case SIFC_FTRAINERB:
		InputDPtr = &FTrainerData;
		break;
#endif
	}

	FCEUI_SetInputFC((ESIFC) InputType[2], InputDPtr, attrib);
	FCEUI_SetInputFourscore((eoptions & EO_FOURSCORE) != 0);
}

/* NOT SUPPORTED */
#if 0
static ButtConfig fkbmap[0x48]=
{
	MK(F1),MK(F2),MK(F3),MK(F4),MK(F5),MK(F6),MK(F7),MK(F8),
	MK(1),MK(2),MK(3),MK(4),MK(5),MK(6),MK(7),MK(8),MK(9),MK(0),
	MK(MINUS),MK(EQUAL),MK(BACKSLASH),MK(BACKSPACE),
	MK(ESCAPE),MK(Q),MK(W),MK(E),MK(R),MK(T),MK(Y),MK(U),MK(I),MK(O),
	MK(P),MK(GRAVE),MK(BRACKET_LEFT),MK(ENTER),
	MK(LEFTCONTROL),MK(A),MK(S),MK(D),MK(F),MK(G),MK(H),MK(J),MK(K),
	MK(L),MK(SEMICOLON),MK(APOSTROPHE),MK(BRACKET_RIGHT),MK(INSERT),
	MK(LEFTSHIFT),MK(Z),MK(X),MK(C),MK(V),MK(B),MK(N),MK(M),MK(COMMA),
	MK(PERIOD),MK(SLASH),MK(RIGHTALT),MK(RIGHTSHIFT),MK(LEFTALT),MK(SPACE),
	MK(DELETE),MK(END),MK(PAGEDOWN),
	MK(CURSORUP),MK(CURSORLEFT),MK(CURSORRIGHT),MK(CURSORDOWN)
};

/**
 * Update the status of the Family KeyBoard.
 */
static void
UpdateFKB()
{
	int x;

	for(x = 0; x < 0x48; x++) {
		fkbkeys[x] = 0;

		if(DTestButton(&fkbmap[x])) {
			fkbkeys[x] = 1;
		}
	}
}
#endif

static ButtConfig HyperShotButtons[4] = {  MK(Q),MK(W),MK(E),MK(R) };

/**
 * Update the status of the HyperShot input device.
 */
static void UpdateHyperShot() {
	int x;

	HyperShotData = 0;
	for (x = 0; x < 0x4; x++) {
		if (DTestButton(&HyperShotButtons[x])) {
			HyperShotData |= 1 << x;
		}
	}
}

/* NOT SUPPORTED */
#if 0
static ButtConfig MahjongButtons[21]=
{
	MK(Q),MK(W),MK(E),MK(R),MK(T),
	MK(A),MK(S),MK(D),MK(F),MK(G),MK(H),MK(J),MK(K),MK(L),
	MK(Z),MK(X),MK(C),MK(V),MK(B),MK(N),MK(M)
};

/**
 * Update the status of for the Mahjong input device.
 */
static void
UpdateMahjong()
{
	int x;

	MahjongData = 0;
	for(x = 0; x < 21; x++) {
		if(DTestButton(&MahjongButtons[x])) {
			MahjongData |= 1 << x;
		}
	}
}
#endif

static ButtConfig QuizKingButtons[6] =
{
	MK(Q),MK(W),MK(E),MK(R),MK(T),MK(Y)
};

/**
 * Update the status of the QuizKing input device.
 */
static void UpdateQuizKing() {
	int x;

	QuizKingData = 0;

	for (x = 0; x < 6; x++) {
		if (DTestButton(&QuizKingButtons[x])) {
			QuizKingData |= 1 << x;
		}
	}
}

static ButtConfig TopRiderButtons[8]=
{
 MK(Q),MK(W),MK(E),MK(R),MK(T),MK(Y),MK(U),MK(I)
};


/**
 * Update the status of the TopRider input device.
 */
static void UpdateTopRider() {
	int x;
	TopRiderData = 0;
	for (x = 0; x < 8; x++) {
		if (DTestButton(&TopRiderButtons[x])) {
			TopRiderData |= (1 << x);
		}
	}
}

/* NOT SUPPORTED */
#if 0
static ButtConfig FTrainerButtons[12]=
{
	MK(O),MK(P),MK(BRACKET_LEFT),
	MK(BRACKET_RIGHT),MK(K),MK(L),MK(SEMICOLON),
	MK(APOSTROPHE),
	MK(M),MK(COMMA),MK(PERIOD),MK(SLASH)
};

/**
 * Update the status of the FTrainer input device.
 */
static void
UpdateFTrainer()
{
	int x;
	FTrainerData = 0;

	for(x = 0; x < 12; x++) {
		if(DTestButton(&FTrainerButtons[x])) {
			FTrainerData |= (1 << x);
		}
	}
}
#endif

/**
 * Hack to map the new configuration onto the existing button
 * configuration management.  Will probably want to change this in the
 * future - soules.
 */
void UpdateInput(Config *config) {
	char buf[64];
	std::string device, prefix;

	for (unsigned int i = 0; i < 3; i++) {
		snprintf(buf, 64, "SDL.Input.%d", i);
		config->getOption(buf, &device);

		if (device == "None") {
			UsrInputType[i] = SI_NONE;
		} else if (device.find("GamePad") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int) SI_GAMEPAD : (int) SIFC_NONE;
#if 0
		} else if(device.find("PowerPad.0") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int)SI_POWERPADA : (int)SIFC_NONE;
		} else if(device.find("PowerPad.1") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int)SI_POWERPADB : (int)SIFC_NONE;
#endif
		} else if (device.find("QuizKing") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int) SI_NONE : (int) SIFC_QUIZKING;
		} else if (device.find("HyperShot") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int) SI_NONE : (int) SIFC_HYPERSHOT;
#if 0
		} else if(device.find("Mahjong") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int)SI_NONE : (int)SIFC_MAHJONG;
#endif
		} else if (device.find("TopRider") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int) SI_NONE : (int) SIFC_TOPRIDER;
#if 0
		} else if(device.find("FTrainer") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int)SI_NONE : (int)SIFC_FTRAINERA;
		} else if(device.find("FamilyKeyBoard") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int)SI_NONE : (int)SIFC_FKB;
#endif
		} else if (device.find("OekaKids") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int) SI_NONE : (int) SIFC_OEKAKIDS;
		} else if (device.find("Arkanoid") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int) SI_ARKANOID : (int) SIFC_ARKANOID;
		} else if (device.find("Shadow") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int) SI_NONE : (int) SIFC_SHADOW;
		} else if (device.find("Zapper") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int) SI_ZAPPER : (int) SIFC_NONE;
#if 0
		} else if(device.find("BWorld") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int)SI_NONE : (int)SIFC_BWORLD;
#endif
		} else if (device.find("4Player") != std::string::npos) {
			UsrInputType[i] = (i < 2) ? (int) SI_NONE : (int) SIFC_4PLAYER;
		} else {
			// Unknown device
			UsrInputType[i] = SI_NONE;
		}
	}

	// update each of the devices' configuration structure
	// XXX soules - this is temporary until this file is cleaned up to
	//              simplify the interface between configuration and
	//              structure data.  This will likely include the
	//              removal of multiple input buttons for a single
	//              input device key.
	int type, devnum, button;

	// gamepad 0 - 3
	for (unsigned int i = 0; i < GAMEPAD_NUM_DEVICES; i++) {
		char buf[64];
		snprintf(buf, 20, "SDL.Input.GamePad.%d.", i);
		prefix = buf;

		config->getOption(prefix + "DeviceType", &device);
		if (device.find("Keyboard") != std::string::npos) {
			type = BUTTC_KEYBOARD;
		} else if (device.find("Joystick") != std::string::npos) {
			type = BUTTC_JOYSTICK;
		} else {
			type = 0;
		}

		config->getOption(prefix + "DeviceNum", &devnum);
		for (unsigned int j = 0; j < GAMEPAD_NUM_BUTTONS; j++) {
			config->getOption(prefix + GamePadNames[j], &button);

			GamePadConfig[i][j].ButtType[0] = type;
			GamePadConfig[i][j].DeviceNum[0] = devnum;
			GamePadConfig[i][j].ButtonNum[0] = button;
			GamePadConfig[i][j].NumC = 1;
		}
	}

#if 0
	// PowerPad 0 - 1
	for(unsigned int i = 0; i < POWERPAD_NUM_DEVICES; i++) {
		char buf[64];
		snprintf(buf, 20, "SDL.Input.PowerPad.%d.", i);
		prefix = buf;

		config->getOption(prefix + "DeviceType", &device);
		if(device.find("Keyboard") != std::string::npos) {
			type = BUTTC_KEYBOARD;
		} else if(device.find("Joystick") != std::string::npos) {
			type = BUTTC_JOYSTICK;
		} else {
			type = 0;
		}

		config->getOption(prefix + "DeviceNum", &devnum);
		for(unsigned int j = 0; j < POWERPAD_NUM_BUTTONS; j++) {
			config->getOption(prefix + PowerPadNames[j], &button);

			powerpadsc[i][j].ButtType[0] = type;
			powerpadsc[i][j].DeviceNum[0] = devnum;
			powerpadsc[i][j].ButtonNum[0] = button;
			powerpadsc[i][j].NumC = 1;
		}
	}
#endif

	// QuizKing
	prefix = "SDL.Input.QuizKing.";
	config->getOption(prefix + "DeviceType", &device);
	if (device.find("Keyboard") != std::string::npos) {
		type = BUTTC_KEYBOARD;
	} else if (device.find("Joystick") != std::string::npos) {
		type = BUTTC_JOYSTICK;
	} else {
		type = 0;
	}
	config->getOption(prefix + "DeviceNum", &devnum);
	for (unsigned int j = 0; j < QUIZKING_NUM_BUTTONS; j++) {
		config->getOption(prefix + QuizKingNames[j], &button);

		QuizKingButtons[j].ButtType[0] = type;
		QuizKingButtons[j].DeviceNum[0] = devnum;
		QuizKingButtons[j].ButtonNum[0] = button;
		QuizKingButtons[j].NumC = 1;
	}

	// HyperShot
	prefix = "SDL.Input.HyperShot.";
	config->getOption(prefix + "DeviceType", &device);
	if (device.find("Keyboard") != std::string::npos) {
		type = BUTTC_KEYBOARD;
	} else if (device.find("Joystick") != std::string::npos) {
		type = BUTTC_JOYSTICK;
	} else {
		type = 0;
	}
	config->getOption(prefix + "DeviceNum", &devnum);
	for (unsigned int j = 0; j < HYPERSHOT_NUM_BUTTONS; j++) {
		config->getOption(prefix + HyperShotNames[j], &button);

		HyperShotButtons[j].ButtType[0] = type;
		HyperShotButtons[j].DeviceNum[0] = devnum;
		HyperShotButtons[j].ButtonNum[0] = button;
		HyperShotButtons[j].NumC = 1;
	}

#if 0
	// Mahjong
	prefix = "SDL.Input.Mahjong.";
	config->getOption(prefix + "DeviceType", &device);
	if(device.find("Keyboard") != std::string::npos) {
		type = BUTTC_KEYBOARD;
	} else if(device.find("Joystick") != std::string::npos) {
		type = BUTTC_JOYSTICK;
	} else {
		type = 0;
	}
	config->getOption(prefix + "DeviceNum", &devnum);
	for(unsigned int j = 0; j < MAHJONG_NUM_BUTTONS; j++) {
		config->getOption(prefix + MahjongNames[j], &button);

		MahjongButtons[j].ButtType[0] = type;
		MahjongButtons[j].DeviceNum[0] = devnum;
		MahjongButtons[j].ButtonNum[0] = button;
		MahjongButtons[j].NumC = 1;
	}
#endif

	// TopRider
	prefix = "SDL.Input.TopRider.";
	config->getOption(prefix + "DeviceType", &device);
	if (device.find("Keyboard") != std::string::npos) {
		type = BUTTC_KEYBOARD;
	} else if (device.find("Joystick") != std::string::npos) {
		type = BUTTC_JOYSTICK;
	} else {
		type = 0;
	}
	config->getOption(prefix + "DeviceNum", &devnum);
	for (unsigned int j = 0; j < TOPRIDER_NUM_BUTTONS; j++) {
		config->getOption(prefix + TopRiderNames[j], &button);

		TopRiderButtons[j].ButtType[0] = type;
		TopRiderButtons[j].DeviceNum[0] = devnum;
		TopRiderButtons[j].ButtonNum[0] = button;
		TopRiderButtons[j].NumC = 1;
	}

#if 0
	// FTrainer
	prefix = "SDL.Input.FTrainer.";
	config->getOption(prefix + "DeviceType", &device);
	if(device.find("Keyboard") != std::string::npos) {
		type = BUTTC_KEYBOARD;
	} else if(device.find("Joystick") != std::string::npos) {
		type = BUTTC_JOYSTICK;
	} else {
		type = 0;
	}
	config->getOption(prefix + "DeviceNum", &devnum);
	for(unsigned int j = 0; j < FTRAINER_NUM_BUTTONS; j++) {
		config->getOption(prefix + FTrainerNames[j], &button);

		FTrainerButtons[j].ButtType[0] = type;
		FTrainerButtons[j].DeviceNum[0] = devnum;
		FTrainerButtons[j].ButtonNum[0] = button;
		FTrainerButtons[j].NumC = 1;
	}

	// FamilyKeyBoard
	prefix = "SDL.Input.FamilyKeyBoard.";
	config->getOption(prefix + "DeviceType", &device);
	if(device.find("Keyboard") != std::string::npos) {
		type = BUTTC_KEYBOARD;
	} else if(device.find("Joystick") != std::string::npos) {
		type = BUTTC_JOYSTICK;
	} else {
		type = 0;
	}
	config->getOption(prefix + "DeviceNum", &devnum);
	for(unsigned int j = 0; j < FAMILYKEYBOARD_NUM_BUTTONS; j++) {
		config->getOption(prefix + FamilyKeyBoardNames[j], &button);

		fkbmap[j].ButtType[0] = type;
		fkbmap[j].DeviceNum[0] = devnum;
		fkbmap[j].ButtonNum[0] = button;
		fkbmap[j].NumC = 1;
	}
#endif
}
// Definitions from main.h:
// GamePad defaults
const char *GamePadNames[GAMEPAD_NUM_BUTTONS] = 
{ 
	"A", "B", "Select", "Start", "Up", "Down", "Left", "Right", "TurboA", "TurboB"
};

const char *DefaultGamePadDevice[GAMEPAD_NUM_DEVICES] = 
{
	"Keyboard", "None", "None", "None"
};

const int DefaultGamePad[GAMEPAD_NUM_DEVICES][GAMEPAD_NUM_BUTTONS] = 
{ 
	{  SDLK_LCTRL, SDLK_LALT, SDLK_ESCAPE, SDLK_RETURN, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_SPACE, SDLK_LSHIFT },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

#if 0
// PowerPad defaults
const char *PowerPadNames[POWERPAD_NUM_BUTTONS] =
{	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B"};
const char *DefaultPowerPadDevice[POWERPAD_NUM_DEVICES] =
{	"Keyboard", "None"};
const int DefaultPowerPad[POWERPAD_NUM_DEVICES][POWERPAD_NUM_BUTTONS] =
{	{	SDLK_o, SDLK_p, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET,
		SDLK_k, SDLK_l, SDLK_SEMICOLON, SDLK_QUOTE,
		SDLK_m, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH},
	{	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
#endif
// QuizKing defaults
const char *QuizKingNames[QUIZKING_NUM_BUTTONS] = { "0", "1", "2", "3", "4",
		"5" };
const char *DefaultQuizKingDevice = "Keyboard";
const int DefaultQuizKing[QUIZKING_NUM_BUTTONS] = { 0, 1, 19, 2, 17, 16 };

// HyperShot defaults
const char *HyperShotNames[HYPERSHOT_NUM_BUTTONS] = { "0", "1", "2", "3" };
const char *DefaultHyperShotDevice = "Keyboard";
const int DefaultHyperShot[HYPERSHOT_NUM_BUTTONS] = { 0, 1, 19, 2 };

#if 0
// Mahjong defaults
const char *MahjongNames[MAHJONG_NUM_BUTTONS] =
{	"00", "01", "02", "03", "04", "05", "06", "07",
	"08", "09", "10", "11", "12", "13", "14", "15",
	"16", "17", "18", "19", "20"};
const char *DefaultMahjongDevice = "Keyboard";
const int DefaultMahjong[MAHJONG_NUM_BUTTONS] =
{	SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_a, SDLK_s, SDLK_d,
	SDLK_f, SDLK_g, SDLK_h, SDLK_j, SDLK_k, SDLK_l, SDLK_z, SDLK_x,
	SDLK_c, SDLK_v, SDLK_b, SDLK_n, SDLK_m};
#endif

// TopRider defaults
const char *TopRiderNames[TOPRIDER_NUM_BUTTONS] = { "0", "1", "2", "3", "4",
		"5", "6", "7" };
const char *DefaultTopRiderDevice = "Keyboard";
const int DefaultTopRider[TOPRIDER_NUM_BUTTONS] = { 6, 27, 5, 18, 0, 1, 19, 2 };

#if 0
// FTrainer defaults
const char *FTrainerNames[FTRAINER_NUM_BUTTONS] =
{	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B"};
const char *DefaultFTrainerDevice = "Keyboard";
const int DefaultFTrainer[FTRAINER_NUM_BUTTONS] =
{	SDLK_o, SDLK_p, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET,
	SDLK_k, SDLK_l, SDLK_SEMICOLON, SDLK_QUOTE,
	SDLK_m, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH};

// FamilyKeyBoard defaults
const char *FamilyKeyBoardNames[FAMILYKEYBOARD_NUM_BUTTONS] =
{	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"MINUS", "EQUAL", "BACKSLASH", "BACKSPACE",
	"ESCAPE", "Q", "W", "E", "R", "T", "Y", "U", "I", "O",
	"P", "GRAVE", "BRACKET_LEFT", "ENTER",
	"LEFTCONTROL", "A", "S", "D", "F", "G", "H", "J", "K",
	"L", "SEMICOLON", "APOSTROPHE", "BRACKET_RIGHT", "INSERT",
	"LEFTSHIFT", "Z", "X", "C", "V", "B", "N", "M", "COMMA",
	"PERIOD", "SLASH", "RIGHTALT", "RIGHTSHIFT", "LEFTALT", "SPACE",
	"DELETE", "END", "PAGEDOWN",
	"CURSORUP", "CURSORLEFT", "CURSORRIGHT", "CURSORDOWN"};
const char *DefaultFamilyKeyBoardDevice = "Keyboard";
const int DefaultFamilyKeyBoard[FAMILYKEYBOARD_NUM_BUTTONS] =
{	SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6, SDLK_F7, SDLK_F8,
	SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5,
	SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_0,
	SDLK_MINUS, SDLK_EQUALS, SDLK_BACKSLASH, SDLK_BACKSPACE,
	SDLK_ESCAPE, SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u,
	SDLK_i, SDLK_o, SDLK_p, SDLK_BACKQUOTE, SDLK_LEFTBRACKET, SDLK_RETURN,
	SDLK_LCTRL, SDLK_a, SDLK_s, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j,
	SDLK_k, SDLK_l, SDLK_SEMICOLON, SDLK_QUOTE, SDLK_RIGHTBRACKET,
	SDLK_INSERT, SDLK_LSHIFT, SDLK_z, SDLK_x, SDLK_c, SDLK_v, SDLK_b,
	SDLK_n, SDLK_m, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_RALT,
	SDLK_RSHIFT, SDLK_LALT, SDLK_SPACE, SDLK_DELETE, SDLK_END, SDLK_PAGEDOWN,
	SDLK_UP, SDLK_LEFT, SDLK_RIGHT, SDLK_DOWN};
#endif
