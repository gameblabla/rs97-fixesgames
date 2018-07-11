#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <ctype.h>
#include <math.h>

#if !defined(__GP32__) && !defined(TARGET_PSP) && !defined(TARGET_OD)
#include "GP2X.h"
extern SDL_Surface* screen;
extern SDL_Surface* actualScreen;
#endif

#ifdef TARGET_OD
#include <SDL/SDL.h>
extern SDL_Surface* screen;
extern SDL_Surface* actualScreen;
#endif

typedef struct {
	int		hSize;
	int		vSize;
	int		Ticks;
#ifdef TARGET_OD
	int		InputKeys[16];
#else
	int		InputKeys[12];
#endif
	//MachineSound	sound[4];
	// Dynamic System Info
	BOOL	Back0;
	BOOL	Back1;
	BOOL	Sprites;
} SYSTEMINFO;

typedef struct {
/*	char 	ProgramFolder[_MAX_PATH];	// place holders for filenames
	char 	SavePath[_MAX_PATH];
	char 	DebugPath[_MAX_PATH];
	char 	RomPath[_MAX_PATH];
	char  ScreenPath[_MAX_PATH];

	char  OpenFileName[_MAX_PATH];	// place holders for filenames
	char 	SaveFileName[_MAX_PATH];*/
	char 	RomFileName[_MAX_PATH];

	int		machine;		// what kind of machine should we emulate
	int		romSize;		// what is the size of the currently loaded file
	int		sample_rate;	// what is the current sample rate
	int		stereo;			// play in stereo?
	//unsigned int		fps;
	int		samples;
	SYSTEMINFO	*drv;
} EMUINFO;

#define KEY_UP			0
#define KEY_DOWN		1
#define KEY_LEFT		2
#define KEY_RIGHT		3
#define KEY_START		4
#define KEY_BUTTON_A	5
#define KEY_BUTTON_B	6
#define KEY_SELECT		7
#define KEY_UP_2		8
#define KEY_DOWN_2		9
#define KEY_LEFT_2		10
#define KEY_RIGHT_2		11

#ifdef TARGET_OD
#define KEY_BUTTON_X	12
#define KEY_BUTTON_Y	13
#define KEY_BUTTON_R	14
#define KEY_BUTTON_L	15
#endif

// Possible Neogeo Pocket versions
#define NGP				0x01
#define NGPC			0x02

#define NR_OF_SYSTEMS	2


extern BOOL		m_bIsActive;
extern EMUINFO		m_emuInfo;
extern SYSTEMINFO	m_sysInfo[NR_OF_SYSTEMS];
extern int romSize;

int handleInputFile(char *romName);
void mainemuinit();

#ifdef __GP32__
#define HOST_FPS 60  //100 was what it was, originally
#else
#ifdef TARGET_PSP   //to call these FPS is a bit of a misnomer
#define HOST_FPS 60  //the number of frames we want to draw to the host's screen every second
#else
#define HOST_FPS 60  //100 was what it was, originally
#endif
#endif

#endif
