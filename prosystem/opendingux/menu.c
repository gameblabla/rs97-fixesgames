/*  prosystem 
    Copyright (C) 2012 Alekmaul

    ProSystem Emulator (original version)
	Copyright(C) 2005 Greg Stanton (see http://home.comcast.net/~gscottstanton/) 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include <dirent.h>

#include "shared.h"

#include "./data/prosystem_background.h"
#include "./data/prosystem_load.h"

#include "../emu/ProSystem.h"

extern unsigned int m_Flag;

bool gameMenu;

#define COLOR_BG            PIX_TO_RGB(layer->format,05, 03, 02)
#define COLOR_HELP_TEXT		PIX_TO_RGB(layer->format,64, 240, 96)
#define COLOR_OK			PIX_TO_RGB(layer->format,0,0,255)
#define COLOR_KO			PIX_TO_RGB(layer->format,255,0,0)
#define COLOR_INFO			PIX_TO_RGB(layer->format,0,255,0)
#define COLOR_LIGHT			PIX_TO_RGB(layer->format,255,255,0)
#define COLOR_ACTIVE_ITEM   PIX_TO_RGB(layer->format,232, 253, 77)
#define COLOR_INACTIVE_ITEM PIX_TO_RGB(layer->format,67,89,153)

// defines and macros
#define MAX__PATH 1024
#define FILE_LIST_ROWS 19

// Font: THIN8X8.pf : Exported from PixelFontEdit 2.7.0
static const unsigned char fontdata8x8[2048] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 000 (.)
	0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E,	// Char 001 (.)
	0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E,	// Char 002 (.)
	0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00,	// Char 003 (.)
	0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00,	// Char 004 (.)
	0x38, 0x7C, 0x38, 0xFE, 0xFE, 0x7C, 0x38, 0x7C,	// Char 005 (.)
	0x10, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x7C,	// Char 006 (.)
	0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00,	// Char 007 (.)
	0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF,	// Char 008 (.)
	0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00,	// Char 009 (.)
	0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF,	// Char 010 (.)
	0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,	// Char 011 (.)
	0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18,	// Char 012 (.)
	0x3F, 0x33, 0x3F, 0x30, 0x30, 0x70, 0xF0, 0xE0,	// Char 013 (.)
	0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0,	// Char 014 (.)
	0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99,	// Char 015 (.)
	0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00,	// Char 016 (.)
	0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00,	// Char 017 (.)
	0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18,	// Char 018 (.)
	0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00,	// Char 019 (.)
	0x7F, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x1B, 0x00,	// Char 020 (.)
	0x3E, 0x63, 0x38, 0x6C, 0x6C, 0x38, 0xCC, 0x78,	// Char 021 (.)
	0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x00,	// Char 022 (.)
	0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF,	// Char 023 (.)
	0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00,	// Char 024 (.)
	0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00,	// Char 025 (.)
	0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00,	// Char 026 (.) right arrow
	0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00,	// Char 027 (.)
	0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xFE, 0x00, 0x00,	// Char 028 (.)
	0x00, 0x24, 0x66, 0xFF, 0x66, 0x24, 0x00, 0x00,	// Char 029 (.)
	0x00, 0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x00, 0x00,	// Char 030 (.)
	0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00,	// Char 031 (.)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 032 ( )
	0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x40, 0x00,	// Char 033 (!)
	0x90, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 034 (")
	0x50, 0x50, 0xF8, 0x50, 0xF8, 0x50, 0x50, 0x00,	// Char 035 (#)
	0x20, 0x78, 0xA0, 0x70, 0x28, 0xF0, 0x20, 0x00,	// Char 036 ($)
	0xC8, 0xC8, 0x10, 0x20, 0x40, 0x98, 0x98, 0x00,	// Char 037 (%)
	0x70, 0x88, 0x50, 0x20, 0x54, 0x88, 0x74, 0x00,	// Char 038 (&)
	0x60, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 039 (')
	0x20, 0x40, 0x80, 0x80, 0x80, 0x40, 0x20, 0x00,	// Char 040 (()
	0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00,	// Char 041 ())
	0x00, 0x20, 0xA8, 0x70, 0x70, 0xA8, 0x20, 0x00,	// Char 042 (*)
	0x00, 0x00, 0x20, 0x20, 0xF8, 0x20, 0x20, 0x00,	// Char 043 (+)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x20, 0x40,	// Char 044 (,)
	0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00,	// Char 045 (-)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00,	// Char 046 (.)
	0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00,	// Char 047 (/)
	0x70, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x70, 0x00,	// Char 048 (0)
	0x40, 0xC0, 0x40, 0x40, 0x40, 0x40, 0xE0, 0x00,	// Char 049 (1)
	0x70, 0x88, 0x08, 0x10, 0x20, 0x40, 0xF8, 0x00,	// Char 050 (2)
	0x70, 0x88, 0x08, 0x10, 0x08, 0x88, 0x70, 0x00,	// Char 051 (3)
	0x08, 0x18, 0x28, 0x48, 0xFC, 0x08, 0x08, 0x00,	// Char 052 (4)
	0xF8, 0x80, 0x80, 0xF0, 0x08, 0x88, 0x70, 0x00,	// Char 053 (5)
	0x20, 0x40, 0x80, 0xF0, 0x88, 0x88, 0x70, 0x00,	// Char 054 (6)
	0xF8, 0x08, 0x10, 0x20, 0x40, 0x40, 0x40, 0x00,	// Char 055 (7)
	0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70, 0x00,	// Char 056 (8)
	0x70, 0x88, 0x88, 0x78, 0x08, 0x08, 0x70, 0x00,	// Char 057 (9)
	0x00, 0x00, 0x60, 0x60, 0x00, 0x60, 0x60, 0x00,	// Char 058 (:)
	0x00, 0x00, 0x60, 0x60, 0x00, 0x60, 0x60, 0x20,	// Char 059 (;)
	0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x00,	// Char 060 (<)
	0x00, 0x00, 0xF8, 0x00, 0xF8, 0x00, 0x00, 0x00,	// Char 061 (=)
	0x80, 0x40, 0x20, 0x10, 0x20, 0x40, 0x80, 0x00,	// Char 062 (>)
	0x78, 0x84, 0x04, 0x08, 0x10, 0x00, 0x10, 0x00,	// Char 063 (?)
	0x70, 0x88, 0x88, 0xA8, 0xB8, 0x80, 0x78, 0x00,	// Char 064 (@)
	0x20, 0x50, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x00,	// Char 065 (A)
	0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0, 0x00,	// Char 066 (B)
	0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00,	// Char 067 (C)
	0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00,	// Char 068 (D)
	0xF8, 0x80, 0x80, 0xE0, 0x80, 0x80, 0xF8, 0x00,	// Char 069 (E)
	0xF8, 0x80, 0x80, 0xE0, 0x80, 0x80, 0x80, 0x00,	// Char 070 (F)
	0x70, 0x88, 0x80, 0x80, 0x98, 0x88, 0x78, 0x00,	// Char 071 (G)
	0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0x00,	// Char 072 (H)
	0xE0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xE0, 0x00,	// Char 073 (I)
	0x38, 0x10, 0x10, 0x10, 0x10, 0x90, 0x60, 0x00,	// Char 074 (J)
	0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88, 0x00,	// Char 075 (K)
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF8, 0x00,	// Char 076 (L)
	0x82, 0xC6, 0xAA, 0x92, 0x82, 0x82, 0x82, 0x00,	// Char 077 (M)
	0x84, 0xC4, 0xA4, 0x94, 0x8C, 0x84, 0x84, 0x00,	// Char 078 (N)
	0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00,	// Char 079 (O)
	0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80, 0x80, 0x00,	// Char 080 (P)
	0x70, 0x88, 0x88, 0x88, 0xA8, 0x90, 0x68, 0x00,	// Char 081 (Q)
	0xF0, 0x88, 0x88, 0xF0, 0xA0, 0x90, 0x88, 0x00,	// Char 082 (R)
	0x70, 0x88, 0x80, 0x70, 0x08, 0x88, 0x70, 0x00,	// Char 083 (S)
	0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,	// Char 084 (T)
	0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00,	// Char 085 (U)
	0x88, 0x88, 0x88, 0x50, 0x50, 0x20, 0x20, 0x00,	// Char 086 (V)
	0x82, 0x82, 0x82, 0x82, 0x92, 0x92, 0x6C, 0x00,	// Char 087 (W)
	0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88, 0x00,	// Char 088 (X)
	0x88, 0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x00,	// Char 089 (Y)
	0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xF8, 0x00,	// Char 090 (Z)
	0xE0, 0x80, 0x80, 0x80, 0x80, 0x80, 0xE0, 0x00,	// Char 091 ([)
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00,	// Char 092 (\)
	0xE0, 0x20, 0x20, 0x20, 0x20, 0x20, 0xE0, 0x00,	// Char 093 (])
	0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 094 (^)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00,	// Char 095 (_)
	0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 096 (`)
	0x00, 0x00, 0x70, 0x08, 0x78, 0x88, 0x74, 0x00,	// Char 097 (a)
	0x80, 0x80, 0xB0, 0xC8, 0x88, 0xC8, 0xB0, 0x00,	// Char 098 (b)
	0x00, 0x00, 0x70, 0x88, 0x80, 0x88, 0x70, 0x00,	// Char 099 (c)
	0x08, 0x08, 0x68, 0x98, 0x88, 0x98, 0x68, 0x00,	// Char 100 (d)
	0x00, 0x00, 0x70, 0x88, 0xF8, 0x80, 0x70, 0x00,	// Char 101 (e)
	0x30, 0x48, 0x40, 0xE0, 0x40, 0x40, 0x40, 0x00,	// Char 102 (f)
	0x00, 0x00, 0x34, 0x48, 0x48, 0x38, 0x08, 0x30,	// Char 103 (g)
	0x80, 0x80, 0xB0, 0xC8, 0x88, 0x88, 0x88, 0x00,	// Char 104 (h)
	0x20, 0x00, 0x60, 0x20, 0x20, 0x20, 0x70, 0x00,	// Char 105 (i)
	0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x90, 0x60,	// Char 106 (j)
	0x80, 0x80, 0x88, 0x90, 0xA0, 0xD0, 0x88, 0x00,	// Char 107 (k)
	0xC0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xE0, 0x00,	// Char 108 (l)
	0x00, 0x00, 0xEC, 0x92, 0x92, 0x92, 0x92, 0x00,	// Char 109 (m)
	0x00, 0x00, 0xB0, 0xC8, 0x88, 0x88, 0x88, 0x00,	// Char 110 (n)
	0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70, 0x00,	// Char 111 (o)
	0x00, 0x00, 0xB0, 0xC8, 0xC8, 0xB0, 0x80, 0x80,	// Char 112 (p)
	0x00, 0x00, 0x68, 0x98, 0x98, 0x68, 0x08, 0x08,	// Char 113 (q)
	0x00, 0x00, 0xB0, 0xC8, 0x80, 0x80, 0x80, 0x00,	// Char 114 (r)
	0x00, 0x00, 0x78, 0x80, 0x70, 0x08, 0xF0, 0x00,	// Char 115 (s)
	0x40, 0x40, 0xE0, 0x40, 0x40, 0x50, 0x20, 0x00,	// Char 116 (t)
	0x00, 0x00, 0x88, 0x88, 0x88, 0x98, 0x68, 0x00,	// Char 117 (u)
	0x00, 0x00, 0x88, 0x88, 0x88, 0x50, 0x20, 0x00,	// Char 118 (v)
	0x00, 0x00, 0x82, 0x82, 0x92, 0x92, 0x6C, 0x00,	// Char 119 (w)
	0x00, 0x00, 0x88, 0x50, 0x20, 0x50, 0x88, 0x00,	// Char 120 (x)
	0x00, 0x00, 0x88, 0x88, 0x98, 0x68, 0x08, 0x70,	// Char 121 (y)
	0x00, 0x00, 0xF8, 0x10, 0x20, 0x40, 0xF8, 0x00,	// Char 122 (z)
	0x10, 0x20, 0x20, 0x40, 0x20, 0x20, 0x10, 0x00,	// Char 123 ({)
	0x40, 0x40, 0x40, 0x00, 0x40, 0x40, 0x40, 0x00,	// Char 124 (|)
	0x40, 0x20, 0x20, 0x10, 0x20, 0x20, 0x40, 0x00,	// Char 125 (})
	0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 126 (~)
	0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00	// Char 127 (.)
};

//unsigned short temp1[320*240],temp2[320*240];

// UP  DOWN  LEFT RIGHT  A  B   X  Y    R  L     START  SELECT
// 3,    2,    1,    0,  4, 5, 12, 12, 15, 16,     14,      13

// | 00       | Joystick 1   | Right
// | 01       | Joystick 1   | Left
// | 02       | Joystick 1   | Down
// | 03       | Joystick 1   | Up
// | 04       | Joystick 1   | Button 1
// | 05       | Joystick 1   | Button 2
// | 06       | Joystick 2   | Right
// | 07       | Joystick 2   | Left
// | 08       | Joystick 2   | Down
// | 09       | Joystick 2   | Up
// | 10       | Joystick 2   | Button 1
// | 11       | Joystick 2   | Button 2
// | 12       | Console      | Reset
// | 13       | Console      | Select
// | 14       | Console      | Pause
// | 15       | Console      | Left Difficulty
// | 16       | Console      | Right Difficulty

//---------------------------------------------------------------------------------------
int system_is_load_state(void);

void menuReset(void);
void menuQuit(void);
void menuContinue(void);
void menuFileBrowse(void);
void menuSaveBmp(void);
void menuSaveState(void);
void menuLoadState(void);
void screen_showkeymenu(void);
void menuReturn(void);

//---------------------------------------------------------------------------------------
typedef struct {
	char itemName[16];
	int *itemPar;
	int itemParMaxValue;
	char *itemParName;
	void (*itemOnA)();
} MENUITEM;

typedef struct {
	int itemNum; // number of items
	int itemCur; // current item
	MENUITEM *m; // array of items
} MENU;

char mnuYesNo[2][16] = {"no", "yes"};
char mnuButtons[17][16] = {
  "Input Right","Input Left","Input Down","Input Up","But #1","But #2",  // P1 
  "Input Right","Input Left","Input Down","Input Up","But #1","But #2",  // P2
  "Reset","Select","Pause","L Diff","R Diff"
};

MENUITEM MainMenuItems[] = {
	{"Load rom", NULL, 0, NULL, &menuFileBrowse},
	{"Continue", NULL, 0, NULL, &menuContinue},
	{"Reset", NULL, 0, NULL, &menuReset},
	{"Load state", NULL, 0, NULL, &menuLoadState},
	{"Save state", NULL, 0, NULL, &menuSaveState},
	{"Button Settings", NULL, 0, NULL, &screen_showkeymenu},
	{"Take Screenshot", NULL, 0, NULL, &menuSaveBmp},
	{"Show FPS: ", &GameConf.m_DisplayFPS, 1,(char *) &mnuYesNo, NULL},
	{"Exit", NULL, 0, NULL, &menuQuit}
};
MENU mnuMainMenu = { 9, 0, (MENUITEM *) &MainMenuItems };

MENUITEM ConfigMenuItems[] = {
	{"Button A: ", &GameConf.Dingoo_Joy[4], 16, (char *)  &mnuButtons, NULL},
	{"Button B: ", &GameConf.Dingoo_Joy[5], 16, (char *)  &mnuButtons, NULL},
	{"Button X: ", &GameConf.Dingoo_Joy[6], 16, (char *)  &mnuButtons, NULL},
	{"Button Y: ", &GameConf.Dingoo_Joy[7], 16, (char *)  &mnuButtons, NULL},
	{"Button R: ", &GameConf.Dingoo_Joy[8], 16, (char *)  &mnuButtons, NULL},
	{"Button L: ", &GameConf.Dingoo_Joy[9], 16, (char *)  &mnuButtons, NULL},
	{"START   : ", &GameConf.Dingoo_Joy[10], 16, (char *) &mnuButtons, NULL},
	{"SELECT  : ", &GameConf.Dingoo_Joy[11], 16, (char *) &mnuButtons, NULL},
	{"Return to menu", NULL, 0, NULL, &menuReturn},
};
MENU mnuConfigMenu = { 9, 0, (MENUITEM *) &ConfigMenuItems };

//----------------------------------------------------------------------------------------------------
#if 0
void screen_drawpixel(SDL_Surface *s, unsigned int x, unsigned int y, unsigned int color) {
	unsigned int bpp, ofs;

	bpp = s->format->BytesPerPixel;
	ofs = s->pitch*y + x*bpp;

	memcpy(s->pixels + ofs, &color, bpp);
}

// Basic Bresenham line algorithm
static void SDL_DrawLine(SDL_Surface *s, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color) {
#define SGN(x) ((x)>0 ? 1 : ((x)==0 ? 0:(-1)))
#define ABS(x) ((x)>0 ? (x) : (-x))

	int lg_delta, sh_delta, cycle, lg_step, sh_step;

	lg_delta = x2 - x1;
	sh_delta = y2 - y1;
	lg_step = SGN(lg_delta);
	lg_delta = ABS(lg_delta);
	sh_step = SGN(sh_delta);
	sh_delta = ABS(sh_delta);
	if (sh_delta < lg_delta) {
		cycle = lg_delta >> 1;
		while (x1 != x2) {
			screen_drawpixel(s, x1, y1, color);
			cycle += sh_delta;
			if (cycle > lg_delta) {
				cycle -= lg_delta;
				y1 += sh_step;
			}
			x1 += lg_step;
		}
		screen_drawpixel(s, x1, y1, color);
	}
	cycle = sh_delta >> 1;
	while (y1 != y2) {
		screen_drawpixel(s, x1, y1, color);
		cycle += lg_delta;
		if (cycle > sh_delta) {
			cycle -= sh_delta;
			x1 += lg_step;
		}
		y1 += sh_step;
	}
	screen_drawpixel(s, x1, y1, color);
}
#endif

//----------------------------------------------------------------------------------------------------
// Prints char on a given surface
void screen_showchar32(SDL_Surface *s, int x, int y, unsigned char a, int fg_color, int bg_color) {
	unsigned int *dst;
	int w, h;

	for(h = 8; h; h--) {
		dst = (unsigned int *)s->pixels + (y+8-h)*s->w + x;
		for(w = 8; w; w--) {
			unsigned int color = *dst; // background
			if((fontdata8x8[a*8 + (8-h)] >> w) & 1) color = fg_color;
			*dst++ = color;
		}
	}
}

void screen_showchar(SDL_Surface *s, int x, int y, unsigned char a, int fg_color, int bg_color) {
	unsigned short *dst;
	int w, h;

	for(h = 8; h; h--) {
		dst = (unsigned short *)s->pixels + (y+8-h)*s->w + x;
		for(w = 8; w; w--) {
			unsigned short color = *dst; // background
			if((fontdata8x8[a*8 + (8-h)] >> w) & 1) color = fg_color;
			*dst++ = color;
		}
	}
}

// copy-pasted mostly from gpsp emulator by Exophaze. 	thanks for it
void print_string(const char *s, unsigned short fg_color, unsigned short bg_color, int x, int y) {
	int i, j = strlen(s);
	for(i = 0; i < j; i++, x += 6) screen_showchar(layer, x, y, s[i], fg_color, bg_color);
}

void print_string_video(int x, int y, const char *s) {
	int i, j = strlen(s);
#ifdef _VIDOD32_
	for(i = 0; i < j; i++, x += 8) screen_showchar32(video, x, y, s[i], ((232<<16)|(253<<8) | 77), 0);
#else
	for(i = 0; i < j; i++, x += 8) screen_showchar(video, x, y, s[i], PIX_TO_RGB(video->format, 232, 253, 77), 0);
#endif
}

void screen_showitem(int x, int y, MENUITEM *m, int fg_color) {
	static char i_str[24];

	// if no parameters, show simple menu item
	if(m->itemPar == NULL) print_string(m->itemName, fg_color, COLOR_BG, x, y);
	else {
		if(m->itemParName == NULL) {
			// if parameter is a digit
			sprintf(i_str, "%s%i", m->itemName, *m->itemPar);
		} else {
			// if parameter is a name in array
			sprintf(i_str, "%s%s", m->itemName, m->itemParName+(*m->itemPar)*16);
		}
		print_string(i_str, fg_color, COLOR_BG, x, y);
	}
}

// Shows menu items and pointing arrow
#define SPRX (16)
void screen_showmenu(MENU *menu) {
	int i;
	MENUITEM *mi = menu->m;
	
	// clear buffer
	SDL_BlitSurface(layerbackgrey, 0, layer, 0);

	// show menu lines
	for(i = 0; i < menu->itemNum; i++, mi++) {
		int fg_color;

		if(menu->itemCur == i) fg_color = COLOR_ACTIVE_ITEM; else fg_color = COLOR_INACTIVE_ITEM;
		screen_showitem(SPRX+10, 59+i*15, mi, fg_color);
		if(menu->itemCur == i) print_string("-", fg_color, COLOR_BG, SPRX+10-12, 59+i*15);
	}
}

// draw default emulator design
void screen_prepback(SDL_Surface *s, unsigned char *bmpBuf, unsigned int bmpSize) {
	// load logo, Convert the image to optimal display format and Free the temporary surface
	SDL_RWops *rw = SDL_RWFromMem(bmpBuf, bmpSize);
	SDL_Surface *temp = SDL_LoadBMP_RW(rw, 1);
	SDL_Surface *image;
	image = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);
	
	// Display image
 	SDL_BlitSurface(image, 0, s, 0);
	SDL_FreeSurface(image);
}

// draw main emulator design
void screen_prepbackground(void) {
	// draw default background
	screen_prepback(layerbackgrey, PROSYSTEM_BACKGROUND, PROSYSTEM_BACKGROUND_SIZE);
}

// wait for a key
void screen_waitkey(void) {
	bool akey=false;
		
	while (!akey) {
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_KEYDOWN)
				akey = true;
		}
	}
}

void screen_waitkeyarelease(void) {
	unsigned char *keys;
		
	// wait key release and go in menu
	while (1) {
		SDL_PollEvent(&event);
		keys = SDL_GetKeyState(NULL);
		if (keys[SDLK_LCTRL] != SDL_PRESSED) break;
	}
}

// flip the layer to screen
void screen_flip(void) {
	//SDL_BlitSurface(layer, 0, video, 0);
	SDL_SoftStretch(layer, 0, video, 0);
	SDL_Flip(video);
}

// Main function that runs all the stuff
void screen_showmainmenu(MENU *menu) {
	unsigned char *keys;
	MENUITEM *mi;
	char szVal[100];
	int isSta = 0;
	unsigned int keya=0, keyb=0, keyup=0, keydown=0, keyleft=0, keyright=0;

	gameMenu=true;

	// Test if load state available
	if (menu == &mnuMainMenu) {
		if (cartridge_IsLoaded()) {
			isSta = system_is_load_state();
		}
	}

	while(gameMenu) {
		SDL_PollEvent(&event);
		keys = SDL_GetKeyState(NULL);
		
		mi = menu->m + menu->itemCur; // pointer to highlit menu option

		// A - apply parameter or enter submenu
		if (keys[SDLK_LCTRL] == SDL_PRESSED) { 
			if (!keya) {
				keya = 1; 
				screen_waitkeyarelease();
				if (mi->itemOnA != NULL) (*mi->itemOnA)();
			}
		}
		else keya=0;

		// B - exit or back to previous menu
		if (keys[SDLK_LALT] == SDL_PRESSED) { 
			if (!keyb) {
				keyb = 1; if (menu != &mnuMainMenu) gameMenu = false;
			}
		}
		else keyb=0;

		// UP - arrow up
		if (keys[SDLK_UP] == SDL_PRESSED) { 
			if (!keyup) {
				keyup = 1; if(--menu->itemCur < 0) menu->itemCur = menu->itemNum - 1;
			}
			else {
				keyup++; if (keyup>12) keyup=0;
			}
		}
		else keyup=0;

		//DOWN - arrow down
		if (keys[SDLK_DOWN] == SDL_PRESSED) { 
			if (!keydown) {
				keydown = 1; if(++menu->itemCur == menu->itemNum) menu->itemCur = 0;
			}
			else {
				keydown++; if (keydown>12) keydown=0;
			}
		}
		else keydown=0;

		// LEFT - decrease parameter value
		if (keys[SDLK_LEFT] == SDL_PRESSED) { 
			if (!keyleft) {
				keyleft = 1; if(mi->itemPar != NULL && *mi->itemPar > 0) *mi->itemPar -= 1;
				// big hack for key conf
				if (menu == &mnuConfigMenu)  {
					if (*mi->itemPar == 11) *mi->itemPar = 5;
					if (*mi->itemPar < 4) *mi->itemPar = 4;
				}
			}
			else {
				keyleft++; if (keyleft>12) keyleft=0;
			}
		}
		else keyleft=0;

		// RIGHT - increase parameter value
		if (keys[SDLK_RIGHT] == SDL_PRESSED) { 
			if (!keyright) {
				keyright = 1; if(mi->itemPar != NULL && *mi->itemPar < mi->itemParMaxValue) *mi->itemPar += 1;
				// big hack for key conf
				if (menu == &mnuConfigMenu)  {
					if (*mi->itemPar == 6) *mi->itemPar = 12;
				}
			}
			else {
				keyright++; if (keyright>12) keyright=0;
			}
		}
		else keyright=0;

		if (gameMenu) {
			screen_showmenu(menu); // show menu items
			if (menu == &mnuMainMenu) {
				if (cartridge_IsLoaded()) {
#ifdef _OPENDINGUX_
					sprintf(szVal,"Game:%s",strrchr(gameName,'/')+1);szVal[38] = '\0'; 
#else
					sprintf(szVal,"Game:%s",strrchr(gameName,'\\')+1);szVal[38] = '\0'; 
#endif
					print_string(szVal, COLOR_LIGHT,COLOR_BG, 8,240-2-10-10);
					sprintf(szVal,"CRC:%08X",gameCRC); 
					print_string(szVal, COLOR_LIGHT, COLOR_BG,8,240-2-10);
					if (isSta) print_string("Load state available",COLOR_INFO, COLOR_BG,8+104,240-2-10);
				}
			}
		}
		SDL_Delay(16);
		screen_flip();
	}
}

// Menu function that runs keys configuration
void screen_showkeymenu(void) {
	screen_showmainmenu(&mnuConfigMenu);
}

// Menu function that runs main top menu
void screen_showtopmenu(void) {
	// Save screen in layer
	SDL_BlitSurface(video, 0, layerback, 0);
	screen_prepbackground();

	// Display and manage main menu
	screen_showmainmenu(&mnuMainMenu);

	// save actual config
	system_savecfg(current_conf_app);
}

//----------------------------------------------------------------------
int system_is_load_state(void) {
	char name[512];
	int fd;
	int n=0;
  
	strcpy(name, gameName);
	strcpy(strrchr(name, '.'), ".sta");

	fd = open(name, O_RDONLY | O_BINARY);
	if (fd >= 0) {
		n = 1;
		close(fd);
	}
	
	return (n ? 1 : 0);
}

// find a filename for bmp or state saving 
void findNextFilename(char *szFileFormat, char *szFilename) {
	uint32_t uBcl;
	int fp;
  
	for (uBcl = 0; uBcl<999; uBcl++) {
		sprintf(szFilename,szFileFormat,uBcl);
		fp = open(szFilename,O_RDONLY | O_BINARY);
		if (fp <0) break;
		close(fp);
	}
	if (uBcl == 1000)
		strcpy(szFilename,"NOSLOT");
	if (fp>=0) close(fp);
}

// Reset current game
void menuReset(void) {
	if (cartridge_IsLoaded()) {
		prosystem_Reset();
		gameMenu=false;
		m_Flag = GF_GAMERUNNING;
	}
}

// Quit prosystem emulator (oh noooo :P)
void menuQuit(void) {
	gameMenu=false;
	m_Flag = GF_GAMEQUIT;
}

// Return to game if loaded
void menuContinue(void) {
	if (cartridge_IsLoaded()) {
		gameMenu=false;
		m_Flag = GF_GAMERUNNING;
	}
}

// Rom file browser which is called from menu
#define MAX_FILES 512
typedef struct  {
	char name[255];
	unsigned int type;
} filedirtype;
filedirtype filedir_list[MAX_FILES];

int sort_function(const void *src_str_ptr, const void *dest_str_ptr) {
  filedirtype *p1 = (filedirtype *) src_str_ptr;
  filedirtype *p2 = (filedirtype *) dest_str_ptr;
  
  return strcmp (p1->name, p2->name);
}

signed int load_file(char **wildcards, char *result) {
	unsigned char *keys;
	unsigned int keya=0, keyb=0, keyup=0, kepufl=8, keydown=0, kepdfl=8, keyleft=0, keyright=0, keyr=0, keyl=0;

	char current_dir_name[MAX__PATH];
	DIR *current_dir;
	struct dirent *current_file;
	struct stat file_info;
	char current_dir_short[81];
	unsigned int current_dir_length;
	unsigned int num_filedir;

	char *file_name;
	unsigned int file_name_length;
	unsigned int ext_pos = -1;
	unsigned int dialog_result = 1;
	signed int return_value = 1;
	unsigned int repeat;
	unsigned int i;

	unsigned int current_filedir_scroll_value;
	unsigned int current_filedir_selection;
	unsigned int current_filedir_in_scroll;
	unsigned int current_filedir_number;
	
	// Init dir with saved one
	strcpy(current_dir_name,GameConf.current_dir_rom);
	chdir(GameConf.current_dir_rom);
	
	while (return_value == 1) {
		current_filedir_in_scroll = 0;
		current_filedir_scroll_value  = 0;
		current_filedir_number = 0;
		current_filedir_selection = 0;
		num_filedir = 0;
		
		getcwd(current_dir_name, MAX__PATH);
		current_dir = opendir(current_dir_name);
		
		do {
			if(current_dir) current_file = readdir(current_dir); else current_file = NULL;

			if(current_file) {
				file_name = current_file->d_name;
				file_name_length = strlen(file_name);

				if((stat(file_name, &file_info) >= 0) && ((file_name[0] != '.') || (file_name[1] == '.'))) {
					if(S_ISDIR(file_info.st_mode)) {
						filedir_list[num_filedir].type = 1; // 1 -> directory
						strcpy(filedir_list[num_filedir].name, file_name);
						num_filedir++;
						
					} else {
						// Must match one of the wildcards, also ignore the .
						if(file_name_length >= 4) {
							if(file_name[file_name_length - 4] == '.') ext_pos = file_name_length - 4;
							else if(file_name[file_name_length - 3] == '.') ext_pos = file_name_length - 3;
							else ext_pos = 0;

							for(i = 0; wildcards[i] != NULL; i++) {
								if(!strcasecmp((file_name + ext_pos), wildcards[i])) {
									filedir_list[num_filedir].type = 0; // 0 -> file
									strcpy(filedir_list[num_filedir].name, file_name);
									num_filedir++;

									break;
								}
							}
						}
					}
				}
			}
		} while(current_file);

		if (num_filedir)
			qsort((void *)filedir_list, num_filedir, sizeof(filedirtype), sort_function);

		closedir(current_dir);

		current_dir_length = strlen(current_dir_name);
		if(current_dir_length > 39) {
			memcpy(current_dir_short, "...", 3);
			memcpy(current_dir_short + 3, current_dir_name + current_dir_length - (39-3), (39-3));
			current_dir_short[39] = 0;
		} else {
			memcpy(current_dir_short, current_dir_name, current_dir_length + 1);
		}

		repeat = 1;

		char print_buffer[81];

		while(repeat) {
			//SDL_FillRect(layer, NULL, COLOR_BG);
			screen_prepback(layer, PROSYSTEM_LOAD, PROSYSTEM_LOAD_SIZE);
			print_string(current_dir_short, COLOR_ACTIVE_ITEM, COLOR_BG, 4, 10*3);
			print_string("Press B to return to the main menu", COLOR_HELP_TEXT, COLOR_BG, 160-(34*8/2), 240-5 -10*3);
			for(i = 0, current_filedir_number = i + current_filedir_scroll_value; i < FILE_LIST_ROWS; i++, current_filedir_number++) {
#define CHARLEN ((320/6)-2)
				if(current_filedir_number < num_filedir) {
					if (filedir_list[current_filedir_number].type == 0) { // file (0) or dir (1) ?
						strncpy(print_buffer,filedir_list[current_filedir_number].name, CHARLEN);
					}
					else {
						strncpy(print_buffer+1,filedir_list[current_filedir_number].name, CHARLEN-1);
						print_buffer[0] = '[';
						if (strlen(filedir_list[current_filedir_number].name)<(CHARLEN-1)) 
							print_buffer[strlen(filedir_list[current_filedir_number].name)+1] = ']';
						else
							print_buffer[CHARLEN-1] = ']';
					}
					print_buffer[CHARLEN] = 0;
					if((current_filedir_number == current_filedir_selection)) {
						print_string(print_buffer, COLOR_ACTIVE_ITEM, COLOR_BG, 4, 10*3+((i + 2) * 8));
					} else {
						print_string(print_buffer, COLOR_INACTIVE_ITEM, COLOR_BG, 4,10*3+ ((i + 2) * 8));
					}
				}
			}

			// Catch input
			SDL_PollEvent(&event);
			keys = SDL_GetKeyState(NULL);

			// A - choose file or enter directory
			if (keys[SDLK_LCTRL] == SDL_PRESSED) { 
				if (!keya) {
					keya = 1; 
					screen_waitkeyarelease();
					if (filedir_list[current_filedir_selection].type == 1)  { // so it's a directory
						repeat = 0;
						chdir(filedir_list[current_filedir_selection].name);
					}
					else {
						repeat = 0;
						return_value = 0;
#ifdef _OPENDINGUX_
						sprintf(result, "%s/%s", current_dir_name, filedir_list[current_filedir_selection].name);
#else
						sprintf(result, "%s\\%s", current_dir_name, filedir_list[current_filedir_selection].name);
#endif
					}
				}
			}
			else keya=0;

			// B - exit or back to previous menu
			if (keys[SDLK_LALT] == SDL_PRESSED) { 
				if (!keyb) {
					keyb = 1; 
					return_value = -1;
					repeat = 0;
				}
			}
			else keyb=0;

			// UP - arrow up
			if (keys[SDLK_UP] == SDL_PRESSED) { 
				if (!keyup) {
					keyup = 1; 
					if(current_filedir_selection) {
						current_filedir_selection--;
						if(current_filedir_in_scroll == 0) {
							current_filedir_scroll_value--;
						} else {
							current_filedir_in_scroll--;
						}
					}
				}
				else {
					keyup++; if (keyup>kepufl) keyup=0;
					if (kepufl>2) kepufl--; 
				}
			}
			else { keyup=0; kepufl = 8; }

			//DOWN - arrow down
			if (keys[SDLK_DOWN] == SDL_PRESSED) { 
				if (!keydown) {
					keydown = 1; 
					if(current_filedir_selection < (num_filedir - 1)) {
						current_filedir_selection++;
						if(current_filedir_in_scroll == (FILE_LIST_ROWS - 1)) {
							current_filedir_scroll_value++;
						} else {
							current_filedir_in_scroll++;
						}
					}
				}
				else {
					keydown++; if (keydown>kepdfl) keydown=0;
					if (kepdfl>2) kepdfl--; 
				}
			}
			else { keydown=0;	kepdfl = 8; }

			// R - arrow down from current screen
			if (keys[SDLK_BACKSPACE] == SDL_PRESSED) { 
				if (!keyr) {
					keyr = 1;
					if ( (current_filedir_selection+FILE_LIST_ROWS) < (num_filedir-1)) {
						current_filedir_selection+=FILE_LIST_ROWS;
						//current_filedir_in_scroll=0;
						current_filedir_scroll_value+=FILE_LIST_ROWS;
					}
				}
			}
			else keyr = 0;

			// L - arrow up from current screen
			if (keys[SDLK_TAB] == SDL_PRESSED) { 
				if (!keyl) {
					keyl = 1;
					if (current_filedir_selection> FILE_LIST_ROWS-1) {
						//current_filedir_in_scroll=0;
						current_filedir_selection-=FILE_LIST_ROWS-1;
						current_filedir_scroll_value-=FILE_LIST_ROWS-1;
					}
				}
			}
			else keyl = 0;

			SDL_Delay(16);
			screen_flip();
		}
		
		// Save current rom directory
		if (return_value != -1) {
			strcpy(GameConf.current_dir_rom,current_dir_name);
		}
	}
	
	return return_value;
}

char *file_ext[] = { (char *)  ".a78", (char *) ".bin", NULL };

void menuFileBrowse(void) {
	if (load_file(file_ext, gameName) != -1) { // exit if file is chosen
		gameMenu=false;
		m_Flag = GF_GAMEINIT;
	}
}

// Take a screenshot of current game
void menuSaveBmp(void) {
    char szFile[512], szFile1[512];
	
	if (cartridge_IsLoaded()) {
#ifdef _OPENDINGUX_
		sprintf(szFile,"./%s",strrchr(gameName,'/')+1);
#else
		sprintf(szFile,".\\%s",strrchr(gameName,'\\')+1);
#endif
		szFile[strlen(szFile)-8] = '%';
		szFile[strlen(szFile)-7] = '0';
		szFile[strlen(szFile)-6] = '3';
		szFile[strlen(szFile)-5] = 'd';
		szFile[strlen(szFile)-4] = '.';
		szFile[strlen(szFile)-3] = 'b';
		szFile[strlen(szFile)-2] = 'm';
		szFile[strlen(szFile)-1] = 'p';

		print_string("Saving...", COLOR_OK, COLOR_BG, 8,240-5 -10*3);
		screen_flip();
		findNextFilename(szFile,szFile1);
		SDL_SaveBMP(layerback, szFile1);
		print_string("Screen saved !", COLOR_OK, COLOR_BG, 8+10*8,240-5 -10*3);
		screen_flip();
		screen_waitkey();
	}
}

// Save current state of game emulated
void menuSaveState(void) {
    char szFile[512];
	
	if (cartridge_IsLoaded()) {
		strcpy(szFile, gameName);
		strcpy(strrchr(szFile, '.'), ".sta");
		print_string("Saving...", COLOR_OK, COLOR_BG, 8,240-5 -10*3);
		if (prosystem_Save(szFile,false) == 1)
			print_string("Save OK",COLOR_OK,COLOR_BG, 8+10*8,240-5 -10*3);
		else
			print_string("Save Not OK !",COLOR_KO,COLOR_BG, 8+10*8,240-5 -10*3);
		screen_flip();
		screen_waitkey();
	}
}

// Load current state of game emulated
void menuLoadState(void) {
    char szFile[512];
	
	if (cartridge_IsLoaded()) {
		strcpy(szFile, gameName);
		strcpy(strrchr(szFile, '.'), ".sta");
		print_string("Loading...", COLOR_OK, COLOR_BG, 8,240-5 -10*3);
		if (prosystem_Load(szFile) == 1)
			print_string("Load OK",COLOR_OK,COLOR_BG, 8+10*8,240-5 -10*3);
		else
			print_string("Load Not OK !",COLOR_KO,COLOR_BG, 8+10*8,240-5 -10*3);
		screen_flip();
		screen_waitkey();
		gameMenu=false;
		m_Flag = GF_GAMERUNNING;
	}
}

// Go back to menu
void menuReturn(void) {
	gameMenu=false;
}


void system_loadcfg(char *cfg_name) {
  int fd;

  fd = open(cfg_name, O_RDONLY | O_BINARY);
  if (fd >= 0) {
	read(fd, &GameConf, sizeof(GameConf));
    close(fd);
  }
  else {
    // UP  DOWN  LEFT RIGHT  A  B   X  Y    R  L     START  SELECT
    // 2,    3,    1,    0,  4, 5, 12, 12, 15, 16,     14,      13
    GameConf.Dingoo_Joy[ 0] = 3;  GameConf.Dingoo_Joy[ 1] = 2;
    GameConf.Dingoo_Joy[ 2] = 1;  GameConf.Dingoo_Joy[ 3] = 0;
    GameConf.Dingoo_Joy[ 4] = 4;  GameConf.Dingoo_Joy[ 5] = 5;
    GameConf.Dingoo_Joy[ 6] = 12;  GameConf.Dingoo_Joy[ 7] = 12;
    GameConf.Dingoo_Joy[ 8] = 15; GameConf.Dingoo_Joy[ 9] = 16;
    GameConf.Dingoo_Joy[10] = 14;  GameConf.Dingoo_Joy[11] = 13;
   
    GameConf.sndLevel=40;
    GameConf.m_ScreenRatio=1; // 0 = original show, 1 = full screen
    GameConf.m_DisplayFPS=1; // 1 = yes
	getcwd(GameConf.current_dir_rom, MAX__PATH);
  }
}

void system_savecfg(char *cfg_name) {
  int fd;
  
  fd = open(cfg_name, O_CREAT | O_RDWR | O_BINARY | O_TRUNC, S_IREAD | S_IWRITE);
  if (fd >= 0) {
    write(fd, &GameConf, sizeof(GameConf)); 
    close(fd);
 }
}
