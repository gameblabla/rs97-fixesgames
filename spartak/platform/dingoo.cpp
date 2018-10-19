/*
Spartak Chess based on stockfish engine.
Copyright (C) 2010 djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef _DINGOO
#ifndef USE_SDL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dingoo/time.h>
#include <dingoo/keyboard.h>
#include <dingoo/slcd.h>
#include <dingoo/cache.h>
#include "../game.h"
#include "../io.h"
#include "../ui/dialog.h"

//#define USE_DINGOO_THREAD // use separate thread because we need more stack than default in .app (64K)

#ifdef USE_DINGOO_THREAD
#include <dingoo/ucos2.h>
#endif//USE_DINGOO_THREAD

void* g_pGameDecodeBuf = NULL;
static bool thread_finished = false;

static eGame* game = NULL;

enum eKeyBit
{
	K_POWER			= 1 << 7,
	K_BUTTON_A		= 1 << 31,
	K_BUTTON_B		= 1 << 21,
	K_BUTTON_X		= 1 << 16,
	K_BUTTON_Y      = 1 << 6,
	K_BUTTON_START	= 1 << 11,
	K_BUTTON_SELECT	= 1 << 10,
	K_TRIGGER_LEFT	= 1 << 8,
	K_TRIGGER_RIGHT	= 1 << 29,
	K_DPAD_UP		= 1 << 20,
	K_DPAD_DOWN		= 1 << 27,
	K_DPAD_LEFT		= 1 << 28,
	K_DPAD_RIGHT	= 1 << 18
};
bool CheckKey(const KEY_STATUS& ks, eKeyBit k, bool* state)
{
	if(ks.status&k)
	{
		if(!*state)
		{
			*state = true;
			return true;
		}
		*state = true;
	}
	else
		*state = false;
	return false;
}
bool UpdateKeys()
{
	if(_sys_judge_event(NULL) < 0)
		return false;

	KEY_STATUS ks;
	_kbd_get_status(&ks);
	if((ks.status&K_BUTTON_START) && (ks.status&K_BUTTON_SELECT))
		return false;

	static bool keys[] = { false, false, false, false, false, false, false, false, false };
	if(CheckKey(ks, K_DPAD_LEFT, &keys[0]))
		game->Command('l');
	if(CheckKey(ks, K_DPAD_RIGHT, &keys[1]))
		game->Command('r');
	if(CheckKey(ks, K_DPAD_UP, &keys[2]))
		game->Command('u');
	if(CheckKey(ks, K_DPAD_DOWN, &keys[3]))
		game->Command('d');
	if(CheckKey(ks, K_BUTTON_A, &keys[4]))
		game->Command('a');
	if(CheckKey(ks, K_BUTTON_B, &keys[5]))
		game->Command('b');
	if(CheckKey(ks, K_TRIGGER_LEFT, &keys[6]))
		game->Command('n');
	if(CheckKey(ks, K_TRIGGER_RIGHT, &keys[7]))
		game->Command('g');
	if(CheckKey(ks, K_BUTTON_SELECT, &keys[8]))
		game->Command('f');
	return true;
}
inline word BGR565(byte r, byte g, byte b) { return (((r&~7) << 8)|((g&~3) << 3)|(b >> 3)); }
void UpdateScreen()
{
	if(!game->Desktop().Update())
		return;
	word* screen = (word*)_lcd_get_frame();
	eRGBA* data = game->Desktop().Buffer();
	for(int y = 0; y < 240; ++y)
	{
		for(int x = 0; x < 320; ++x)
		{
			eRGBA c(*data++);
			*screen++ = BGR565(c.r, c.g, c.b);
		}
	}
	__dcache_writeback_all();
	_lcd_set_frame();
}

void thread_proc(void* arg)
{
	game = new eGame;
	while(UpdateKeys() && game->Update())
	{
		UpdateScreen();
		mdelay(15);
	}
	delete game;
	thread_finished = true;
#ifdef USE_DINGOO_THREAD
	OSTaskDel(0);
#endif//USE_DINGOO_THREAD
}

int main(int argc, char** argv)
{
	xIo::ResourcePath(argv[0]);
//	xLog::Open();

#ifdef USE_DINGOO_THREAD
	enum { STACK_SIZE = 256*1024 };
	OS_STK* stack = new OS_STK[STACK_SIZE];
	OSTaskCreate(thread_proc, NULL, &stack[STACK_SIZE - 1], 0);
	while(!thread_finished)
	{
		OSTimeDly(20);
	}
	delete[] stack;
#else
	thread_proc(NULL);
#endif//USE_DINGOO_THREAD
	return 0;
}

#endif//USE_SDL
#endif//_DINGOO
