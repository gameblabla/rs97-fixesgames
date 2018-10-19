#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_events.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "pspsdk_linux.h"
#include "psptypes.h"
#include "pspctrl.h"

/* For internal use only */
# define PSP_CTRL_UPRIGHT    0x10000
# define PSP_CTRL_UPLEFT     0x20000
# define PSP_CTRL_DOWNRIGHT  0x40000
# define PSP_CTRL_DOWNLEFT   0x80000


static int    loc_LastEventMask    = 0;
static int    loc_CurrEventMask    = 0;
static int    loc_CurrEventButtons = 0;
static u32    loc_LastTimeStamp    = 0;
static u32    loc_CurrTimeStamp    = 0;

# define PSP_MIN_TIME_REPEAT  100000

static inline int
sceConvertMaskToButtons(int Mask)
{
  int Buttons = Mask & PSP_CTRL_MASK;
  if (Mask & PSP_CTRL_UPLEFT ) Buttons |= PSP_CTRL_UP|PSP_CTRL_LEFT;
  if (Mask & PSP_CTRL_UPRIGHT) Buttons |= PSP_CTRL_UP|PSP_CTRL_RIGHT;
  if (Mask & PSP_CTRL_DOWNLEFT ) Buttons |= PSP_CTRL_DOWN|PSP_CTRL_LEFT;
  if (Mask & PSP_CTRL_DOWNRIGHT) Buttons |= PSP_CTRL_DOWN|PSP_CTRL_RIGHT;
  return Buttons;
}


int  
sceCtrlPeekBufferPositive(SceCtrlData* c, int v)
{
static int loc_lx = 128;
static int loc_ly = 128;

  SDL_Event SDLEvent;    

  int Event         = 0;
  int ButtonPress   = 0;
  int ButtonRelease = 0;
  int Mask = 0;

  memset(c, 0x0, sizeof(SceCtrlData));
  loc_CurrTimeStamp = SDL_GetTicks() * 1000;

  if (SDL_PollEvent(&SDLEvent)) {
    Event=((SDL_KeyboardEvent*)(&SDLEvent))->keysym.scancode;
    if (SDLEvent.type==SDL_KEYDOWN) {
      ButtonPress = 1;
    } else
    if (SDLEvent.type==SDL_KEYUP) {
      ButtonRelease = 1;
    }
    switch (Event) {
      case PSP_UP        : Mask = PSP_CTRL_UP;
      break;
      case PSP_DOWN      : Mask = PSP_CTRL_DOWN;
      break;
      case PSP_LEFT      : Mask = PSP_CTRL_LEFT;
      break;
      case PSP_RIGHT     : Mask = PSP_CTRL_RIGHT;
      break;
      case PSP_UPLEFT    : Mask = PSP_CTRL_UPLEFT;
      break;
      case PSP_UPRIGHT   : Mask = PSP_CTRL_UPRIGHT;
      break;
      case PSP_DOWNLEFT  : Mask = PSP_CTRL_DOWNLEFT;
      break;
      case PSP_DOWNRIGHT : Mask = PSP_CTRL_DOWNRIGHT;
      break;
      case PSP_SQUARE    : Mask = PSP_CTRL_SQUARE;
      break;
      case PSP_CIRCLE    : Mask = PSP_CTRL_CIRCLE;
      break;
      case PSP_CROSS     : Mask = PSP_CTRL_CROSS;
      break;
      case PSP_TRIANGLE  : Mask = PSP_CTRL_TRIANGLE;
      break;
      case PSP_L         : Mask = PSP_CTRL_LTRIGGER;
      break;
      case PSP_R         : Mask = PSP_CTRL_RTRIGGER;
      break;
      case PSP_START     : Mask = PSP_CTRL_START;
      break;
      case PSP_SELECT    : Mask = PSP_CTRL_SELECT;
      break;
      case PSP_JOY_UP    : if (ButtonPress) loc_ly = 0;
                           else             loc_ly = 128;
      break;
      case PSP_JOY_DOWN  : if (ButtonPress) loc_ly = 255;
                           else             loc_ly = 128;
      break;
      case PSP_JOY_LEFT  : if (ButtonPress) loc_lx = 0;
                           else             loc_lx = 128;
      break;
      case PSP_JOY_RIGHT : if (ButtonPress) loc_lx = 255;
                           else             loc_lx = 128;
      break;
    }
    loc_LastEventMask = loc_CurrEventMask;
    if (ButtonPress) {
      loc_CurrEventMask |= Mask;
    } else 
    if (ButtonRelease) {
      loc_CurrEventMask &= ~Mask;
    }
    loc_CurrEventButtons = sceConvertMaskToButtons(loc_CurrEventMask);
    c->Buttons   = loc_CurrEventButtons;
    c->TimeStamp = loc_CurrTimeStamp;

    loc_LastTimeStamp = loc_CurrTimeStamp;

  } else {
    c->Buttons   = loc_CurrEventButtons;
    c->TimeStamp = loc_CurrTimeStamp;
  }

  c->Lx = loc_lx;
  c->Ly = loc_ly;

  return (c->Buttons != 0);
}

int
sceCtrlReadBufferPositive(SceCtrlData* c, int v)
{
  while (! sceCtrlPeekBufferPositive(c, v));
  return 1;
}


void
scePowerSetClockFrequency(int freq1, int freq2, int freq3)
{
}

void
pspDebugScreenInit()
{
}

void
sceKernelDelayThread(int uvalue)
{
  usleep(uvalue / 100);
}

void
sceKernelExitGame(int status)
{
  exit(status);
}

void
sceDisplayWaitVblankStart()
{
}
