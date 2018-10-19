/*
 * atari_sdl.c - SDL library specific port code
 *
 * Copyright (c) 2001-2002 Jacek Poplawski
 * Copyright (C) 2001-2005 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
   Thanks to David Olofson for scaling tips!

   TODO:
   - implement all Atari800 parameters
   - use mouse and real joystick
   - turn off fullscreen when error happen
*/

#include "config.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//LUDO:
#include "global.h"
#include "psp_sdl.h"
#include "psp_kbd.h"

#if 0 //LUDO: def linux
#define LPTJOY  1
#endif

#if 0 //LUDO: def LPTJOY
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/lp.h>
#endif /* LPTJOY */

/* Atari800 includes */
#include "input.h"
#include "colours.h"
#include "monitor.h"
#include "platform.h"
#include "ui.h"
#include "screen.h"
#include "pokeysnd.h"
#include "gtia.h"
#include "antic.h"
#include "devices.h"
#include "cpu.h"
#include "memory.h"
#include "pia.h"
#include "log.h"
#include "util.h"
#include "atari_ntsc.h"

//LUDO:
  int psp_screenshot_mode = 0;

//LUDO:
int joystick_states[2][6];

/* you can set that variables in code, or change it when emulator is running
   I am not sure what to do with sound_enabled (can't turn it on inside
   emulator, probably we need two variables or command line argument) */
static int sound_flags = 0;
static int SDL_ATARI_BPP = 0;  /* 0 - autodetect */
//LUDO: static int FULLSCREEN = 1;
static int FULLSCREEN = 0;
static int BW = 0;
static int SWAP_JOYSTICKS = 0;
static int WIDTH_MODE = 1;
static int ROTATE90 = 0;
static int ntscemu = 0;
static int scanlines_percentage = 20;
static atari_ntsc_t *the_ntscemu;
/* making setup static conveniently clears all fields to 0 */
static atari_ntsc_setup_t atari_ntsc_setup;
#define SHORT_WIDTH_MODE 0
#define DEFAULT_WIDTH_MODE 1
#define FULL_WIDTH_MODE 2

/* you need to uncomment this to turn on fps counter */

/* #define FPS_COUNTER = 1 */

/* joystick emulation
   keys are loaded from config file
   Here the defaults if there is no keymap in the config file... */

/* a runtime switch for the kbd_joy_X_enabled vars is in the UI */
int kbd_joy_0_enabled = TRUE;  /* enabled by default, doesn't hurt */
int kbd_joy_1_enabled = FALSE;  /* disabled, would steal normal keys */

int KBD_TRIG_0 = SDLK_LCTRL;
int KBD_STICK_0_LEFT = SDLK_KP4;
int KBD_STICK_0_RIGHT = SDLK_KP6;
int KBD_STICK_0_DOWN = SDLK_KP2;
int KBD_STICK_0_UP = SDLK_KP8;
int KBD_STICK_0_LEFTUP = SDLK_KP7;
int KBD_STICK_0_RIGHTUP = SDLK_KP9;
int KBD_STICK_0_LEFTDOWN = SDLK_KP1;
int KBD_STICK_0_RIGHTDOWN = SDLK_KP3;

int KBD_TRIG_1 = SDLK_TAB;
int KBD_STICK_1_LEFT = SDLK_a;
int KBD_STICK_1_RIGHT = SDLK_d;
int KBD_STICK_1_DOWN = SDLK_x;
int KBD_STICK_1_UP = SDLK_w;
int KBD_STICK_1_LEFTUP = SDLK_q;
int KBD_STICK_1_RIGHTUP = SDLK_e;
int KBD_STICK_1_LEFTDOWN = SDLK_z;
int KBD_STICK_1_RIGHTDOWN = SDLK_c;

/* real joysticks */

int fd_joystick0 = -1;
int fd_joystick1 = -1;

SDL_Joystick *joystick0 = NULL;
SDL_Joystick *joystick1 = NULL;
int joystick0_nbuttons, joystick1_nbuttons;

#define minjoy 10000      /* real joystick tolerancy */

extern int alt_function;
    int key_pressed = 0;

// #define FRAGSIZE        1024
#define FRAGSIZE        1024

/* video */
SDL_Surface *MainScreen = NULL;
SDL_Color colors[256];      /* palette */
Uint16 Palette16[256];      /* 16-bit palette */

/* used in UI to show how the keyboard joystick is mapped */
char *joy_0_description(char *buffer, int maxsize)
{
  snprintf(buffer, maxsize, " (L=%s R=%s U=%s D=%s B=%s)",
      SDL_GetKeyName(KBD_STICK_0_LEFT),
      SDL_GetKeyName(KBD_STICK_0_RIGHT),
      SDL_GetKeyName(KBD_STICK_0_UP),
      SDL_GetKeyName(KBD_STICK_0_DOWN),
      SDL_GetKeyName(KBD_TRIG_0)
  );
  return buffer;
}

char *joy_1_description(char *buffer, int maxsize)
{
  snprintf(buffer, maxsize, " (L=%s R=%s U=%s D=%s B=%s)",
      SDL_GetKeyName(KBD_STICK_1_LEFT),
      SDL_GetKeyName(KBD_STICK_1_RIGHT),
      SDL_GetKeyName(KBD_STICK_1_UP),
      SDL_GetKeyName(KBD_STICK_1_DOWN),
      SDL_GetKeyName(KBD_TRIG_1)
  );
  return buffer;
}

void Sound_Pause(void)
{
  if (ATARI.atari_snd_enable) {
    /* stop audio output */
    SDL_PauseAudio(1);
  }
}

void Sound_Continue(void)
{
  if (ATARI.atari_snd_enable) {
    /* start audio output */
    SDL_PauseAudio(0);
  }
}

void Sound_Update(void)
{
  /* fake function */
}

void SetPalette(void)
{
  SDL_SetPalette(MainScreen, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
}

void CalcPalette(void)
{
  int i, rgb;
  float y;
  Uint32 c;
  if (BW == 0)
    for (i = 0; i < 256; i++) {
      rgb = colortable[i];
      colors[i].r = (rgb & 0x00ff0000) >> 16;
      colors[i].g = (rgb & 0x0000ff00) >> 8;
      colors[i].b = (rgb & 0x000000ff) >> 0;
    }
  else
    for (i = 0; i < 256; i++) {
      rgb = colortable[i];
      y = 0.299 * ((rgb & 0x00ff0000) >> 16) +
        0.587 * ((rgb & 0x0000ff00) >> 8) +
        0.114 * ((rgb & 0x000000ff) >> 0);
      colors[i].r = y;
      colors[i].g = y;
      colors[i].b = y;
    }
  for (i = 0; i < 256; i++) {
    c = SDL_MapRGB(MainScreen->format, colors[i].r, colors[i].g, colors[i].b);
    Palette16[i] = (Uint16) c;
  }

}

void ModeInfo(void)
{
  char bwflag, fullflag, width, joyflag;
  if (BW)
    bwflag = '*';
  else
    bwflag = ' ';
  if (FULLSCREEN)
    fullflag = '*';
  else
    fullflag = ' ';
  switch (WIDTH_MODE) {
  case FULL_WIDTH_MODE:
    width = 'f';
    break;
  case DEFAULT_WIDTH_MODE:
    width = 'd';
    break;
  case SHORT_WIDTH_MODE:
    width = 's';
    break;
  default:
    width = '?';
    break;
  }
  if (SWAP_JOYSTICKS)
    joyflag = '*';
  else
    joyflag = ' ';
  Aprint("Video Mode: %dx%dx%d", MainScreen->w, MainScreen->h,
       MainScreen->format->BitsPerPixel);
  Aprint("[%c] FULLSCREEN  [%c] BW  [%c] WIDTH MODE  [%c] JOYSTICKS SWAPPED",
     fullflag, bwflag, width, joyflag);
}

# if 0 //LUDO:
void SetVideoMode(int w, int h, int bpp)
{
# if 0 //LUDO:
  if (FULLSCREEN)
    MainScreen = SDL_SetVideoMode(w, h, bpp, SDL_FULLSCREEN);
  else
    MainScreen = SDL_SetVideoMode(w, h, bpp, SDL_RESIZABLE);
  if (MainScreen == NULL) {
    Aprint("Setting Video Mode: %dx%dx%d FAILED", w, h, bpp);
    Aflushlog();
    exit(-1);
  }
# else
  MainScreen=SDL_SetVideoMode(w,h,bpp, SDL_ANYFORMAT|SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_HWPALETTE);
# endif

}
# endif

void SetNewVideoMode(int w, int h, int bpp)
{
  float ww, hh;

# if 0 //LUDO:
  if (ROTATE90||ntscemu) {
    SetVideoMode(w, h, bpp);
  }
  else
# endif
    {

    if ((h < ATARI_HEIGHT) || (w < ATARI_WIDTH)) {
      h = ATARI_HEIGHT;
      w = ATARI_WIDTH;
    }

    /* aspect ratio, floats needed */
    ww = w;
    hh = h;
    switch (WIDTH_MODE) {
    case SHORT_WIDTH_MODE:
      if (ww * 0.75 < hh)
        hh = ww * 0.75;
      else
        ww = hh / 0.75;
      break;
    case DEFAULT_WIDTH_MODE:
      if (ww / 1.4 < hh)
        hh = ww / 1.4;
      else
        ww = hh * 1.4;
      break;
    case FULL_WIDTH_MODE:
      if (ww / 1.6 < hh)
        hh = ww / 1.6;
      else
        ww = hh * 1.6;
      break;
    }
    w = ww;
    h = hh;
    w = w / 8;
    w = w * 8;
    h = h / 8;
    h = h * 8;

# if 0 //LUDO:
    SetVideoMode(w, h, bpp);
# endif

    SDL_ATARI_BPP = MainScreen->format->BitsPerPixel;
# if 0 //LUDO:
    if (bpp == 0) {
      Aprint("detected %dbpp", SDL_ATARI_BPP);
      if ((SDL_ATARI_BPP != 8) && (SDL_ATARI_BPP != 16)
        && (SDL_ATARI_BPP != 32)) {
        Aprint("it's unsupported, so setting 8bit mode (slow conversion)");
        SetVideoMode(w, h, 8);
      }
    }
# endif
  }

  SetPalette();

# if 0  //LUDO:
  SDL_ShowCursor(SDL_DISABLE);  /* hide mouse cursor */
# endif

  ModeInfo();

}

void SwitchFullscreen(void)
{
  FULLSCREEN = 1 - FULLSCREEN;
  SetNewVideoMode(MainScreen->w, MainScreen->h,
          MainScreen->format->BitsPerPixel);
  Atari_DisplayScreen();
}

void SwitchWidth(void)
{
  WIDTH_MODE++;
  if (WIDTH_MODE > FULL_WIDTH_MODE)
    WIDTH_MODE = SHORT_WIDTH_MODE;
  SetNewVideoMode(MainScreen->w, MainScreen->h,
          MainScreen->format->BitsPerPixel);
  Atari_DisplayScreen();
}

void SwitchBW(void)
{
  BW = 1 - BW;
  CalcPalette();
  SetPalette();
  ModeInfo();
}

void SwapJoysticks(void)
{
  SWAP_JOYSTICKS = 1 - SWAP_JOYSTICKS;
  ModeInfo();
}

static  Uint8 dsp_buffer[FRAGSIZE * 4]; /* x2, because 16bit buffers */
void SDL_Sound_Update(void *userdata, Uint8 *stream, int len)
{
  Pokey_process(dsp_buffer, len);

  if (ATARI.atari_snd_enable) {
# if defined(GP2X_MODE) || defined(WIZ_MODE)
    long volume = (SDL_MIX_MAXVOLUME * gp2xGetSoundVolume()) / 100;
    SDL_MixAudio(stream, (unsigned char *)dsp_buffer, len * 2, volume);
# else
    Uint32 *tgt = stream;
    Uint32 *src = dsp_buffer;
    len = len >> 2;
    while (len-- > 0) *tgt++ = *src++;
# endif
  } else {
    Uint32 *tgt = stream;
    len = len >> 2;
    while (len-- > 0) *tgt++ = 0;
  }
}

void
SDL_Sound_Initialise(void)
{
  SDL_AudioSpec desired, obtained;

  sound_flags |= SND_BIT16;

  desired.freq = 44100;
  desired.format = AUDIO_S16;
  desired.channels = 1;
  desired.samples = FRAGSIZE;
  desired.callback = SDL_Sound_Update;
  desired.userdata = NULL;

  if (SDL_OpenAudio(&desired, &obtained) < 0) {
    fprintf(stderr, "Couldn't open sound: %s\n", SDL_GetError());
    return;
  }
  SDL_Delay(1000);        // Give sound some time to init
  Pokey_sound_init(FREQ_17_EXACT, 44100, 1, sound_flags);

  if (ATARI.atari_snd_enable) SDL_PauseAudio(0);
  else                        SDL_PauseAudio(1);

  gp2xInitSoundVolume();
}


int Atari_Keyboard(void)
{
  int ret_key_code;
  ret_key_code = psp_update_keys();
# if 0 //LUDO:
   if (ret_key_code != AKEY_NONE) {
     fprintf(stdout, "ret_key_code=%d press=%d\n", ret_key_code, key_pressed);
   }
# endif
  return ret_key_code;
}

void
Init_SDL_Joysticks(int first, int second)
{
# if 0 //LUDO:
  if (first) {
    joystick0 = SDL_JoystickOpen(0);
    if (joystick0 == NULL)
      Aprint("joystick 0 not found");
    else {
      Aprint("joystick 0 found!");
      joystick0_nbuttons = SDL_JoystickNumButtons(joystick0);
      SWAP_JOYSTICKS = 1;    /* real joy is STICK(0) and numblock is STICK(1) */
    }
  }

  if (second) {
    joystick1 = SDL_JoystickOpen(1);
    if (joystick1 == NULL)
      Aprint("joystick 1 not found");
    else {
      Aprint("joystick 1 found!");
      joystick1_nbuttons = SDL_JoystickNumButtons(joystick1);
    }
  }
# endif
}

void Init_Joysticks(int *argc, char *argv[])
{
#if 0 //LUDO: def LPTJOY
  char *lpt_joy0 = NULL;
  char *lpt_joy1 = NULL;
  int i;
  int j;

  for (i = j = 1; i < *argc; i++) {
    if (!strcmp(argv[i], "-joy0")) {
      if (i == *argc - 1) {
        Aprint("joystick device path missing!");
        break;
      }
      lpt_joy0 = argv[++i];
    }
    else if (!strcmp(argv[i], "-joy1")) {
      if (i == *argc - 1) {
        Aprint("joystick device path missing!");
        break;
      }
      lpt_joy1 = argv[++i];
    }
    else {
      argv[j++] = argv[i];
    }
  }
  *argc = j;

  if (lpt_joy0 != NULL) {        /* LPT1 joystick */
    fd_joystick0 = open(lpt_joy0, O_RDONLY);
    if (fd_joystick0 == -1)
      perror(lpt_joy0);
  }
  if (lpt_joy1 != NULL) {        /* LPT2 joystick */
    fd_joystick1 = open(lpt_joy1, O_RDONLY);
    if (fd_joystick1 == -1)
      perror(lpt_joy1);
  }
#endif /* LPTJOY */
  Init_SDL_Joysticks(fd_joystick0 == -1, fd_joystick1 == -1);
}


void Atari_Initialise(int *argc, char *argv[])
{
  int no_joystick;
  int width, height, bpp;

  no_joystick = 0;

  width = 320;
  height = 240;
  bpp = 16;

  MainScreen = back_surface;

  SetNewVideoMode(width, height, bpp);
  CalcPalette();
  SetPalette();

  Aprint("video initialized");

  if (no_joystick == 0)
    Init_Joysticks(argc, argv);

  SDL_EnableUNICODE(1);
}

int Atari_Exit(int run_monitor)
{
  int restart;
  int original_fullscreen = FULLSCREEN;

  if (run_monitor) {
    /* disable graphics, set alpha mode */
    if (FULLSCREEN) {
      SwitchFullscreen();
    }
    Sound_Pause();
    restart = monitor();
    Sound_Continue();
  }
  else {
    restart = FALSE;
  }

  if (restart) {
    /* set up graphics and all the stuff */
    if (original_fullscreen != FULLSCREEN) {
      SwitchFullscreen();
    }
    return 1;
  }

  SDL_Quit();

  Aflushlog();

  return restart;
}

# define PSP_LINE_WIDTH 320

static void
Display_normal(void)
{
  uchar  *atari_top_scr = (UBYTE *)atari_screen;
  ushort *psp_top_scr   = (ushort *)MainScreen->pixels;

  uchar  *atari_scr = atari_top_scr + 32;
  u32    *psp_scr = psp_top_scr;

# if 0
  int y_s = ATARI_HEIGHT;
  while (y_s-- > 0) {
    int x_s = 80; // 320 / 4
    while (x_s-- > 0) {
      u32 v = Palette16[ atari_scr[0] ];
      v |= Palette16[ atari_scr[1] ] << 16;
      *psp_scr++ = v;
      v = Palette16[ atari_scr[2] ];
      v |= Palette16[ atari_scr[3] ] << 16;
      *psp_scr++ = v;
      atari_scr += 4;
    }
    atari_scr += ATARI_WIDTH - 320;
    // psp_scr   += (PSP_LINE_WIDTH - 320) >> 1;
  }
# else
  int y_s = ATARI_HEIGHT;
  while (y_s-- > 0) {
    int x_s = 40; // 320 / 4
    while (x_s-- > 0) {
      u32 v = Palette16[ *atari_scr++ ];
      v |= Palette16[ *atari_scr++ ] << 16;
      *psp_scr++ = v;
      v = Palette16[ *atari_scr++ ];
      v |= Palette16[ *atari_scr++ ] << 16;
      *psp_scr++ = v;
      v = Palette16[ *atari_scr++ ];
      v |= Palette16[ *atari_scr++ ] << 16;
      *psp_scr++ = v;
      v = Palette16[ *atari_scr++ ];
      v |= Palette16[ *atari_scr++ ] << 16;
      *psp_scr++ = v;
    }
    atari_scr += ATARI_WIDTH - 320;
  }
# endif
}

void
psp_atari_blit_image()
{
  uchar  *atari_top_scr = (UBYTE *)atari_screen;
  ushort *psp_top_scr   = (ushort *)blit_surface->pixels;

  uchar  *atari_scr = atari_top_scr + 32;
  ushort *psp_scr = psp_top_scr;

  int y_s = ATARI_HEIGHT;
  while (y_s-- > 0) {
    int x_s = 80; // 320 / 4
    while (x_s-- > 0) {
      *psp_scr++ = Palette16[ *atari_scr++ ];
      *psp_scr++ = Palette16[ *atari_scr++ ];
      *psp_scr++ = Palette16[ *atari_scr++ ];
      *psp_scr++ = Palette16[ *atari_scr++ ];
    }
    atari_scr += ATARI_WIDTH - 320;
    psp_scr   += ATARI_WIDTH - 320;
  }
}

/* LUDO: */
void
atari_synchronize()
{
  static u32 nextclock = 1;
  static u32 next_sec_clock = 0;
  static u32 cur_num_frame = 0;

  u32 curclock = SDL_GetTicks();

  if (ATARI.atari_speed_limiter) {
    while (curclock < nextclock) {
     curclock = SDL_GetTicks();
    }
    u32 f_period = 1000 / ATARI.atari_speed_limiter;
    nextclock += f_period;
    if (nextclock < curclock) nextclock = curclock + f_period;
  }

  if (ATARI.atari_view_fps) {
    cur_num_frame++;
    if (curclock > next_sec_clock) {
      next_sec_clock = curclock + 1000;
      ATARI.atari_current_fps = cur_num_frame * (1 + ATARI.psp_skip_max_frame);
      cur_num_frame = 0;
    }
  }
}

void
ui_synchronize()
{
  static u32 nextclock = 1;
  u32 curclock = SDL_GetTicks();

  while (curclock < nextclock) {
   curclock = SDL_GetTicks();
  }
  u32 f_period = 1000 / 60;
  nextclock += f_period;
  if (nextclock < curclock) nextclock = curclock + f_period;
}

void
Atari_DisplayScreen(void)
{
  if (ATARI.psp_skip_cur_frame <= 0) {

    ATARI.psp_skip_cur_frame = ATARI.psp_skip_max_frame;

    Display_normal();
    atari_synchronize();

    if (psp_kbd_is_danzeff_mode()) {
      danzeff_moveTo(-10, -65);
      danzeff_render( ATARI.danzeff_trans );
    }

    if (ATARI.atari_view_fps) {
      char buffer[32];
      sprintf(buffer, "%03d %3d", ATARI.atari_current_clock, (int)ATARI.atari_current_fps );
      psp_sdl_fill_print(0, 0, buffer, 0xffffff, 0 );
    }

    psp_sdl_flip();

    if (psp_screenshot_mode) {
      psp_screenshot_mode--;
      if (psp_screenshot_mode <= 0) {
        psp_sdl_save_screenshot();
        psp_screenshot_mode = 0;
      }
    }
  } else if (ATARI.psp_skip_max_frame) {
    ATARI.psp_skip_cur_frame--;
  }
}

# if 0 //LUDO:
int get_SDL_joystick_state(SDL_Joystick *joystick)
{
  int x;
  int y;

  x = SDL_JoystickGetAxis(joystick, 0);
  y = SDL_JoystickGetAxis(joystick, 1);

  if (x > minjoy) {
    if (y < -minjoy)
      return STICK_UR;
    else if (y > minjoy)
      return STICK_LR;
    else
      return STICK_RIGHT;
  }
  else if (x < -minjoy) {
    if (y < -minjoy)
      return STICK_UL;
    else if (y > minjoy)
      return STICK_LL;
    else
      return STICK_LEFT;
  }
  else {
    if (y < -minjoy)
      return STICK_FORWARD;
    else if (y > minjoy)
      return STICK_BACK;
    else
      return STICK_CENTRE;
  }
}
# endif

int get_LPT_joystick_state(int fd)
{
# if 0 //LUDO #ifdef LPTJOY
  int status;

  ioctl(fd, LPGETSTATUS, &status);
  status ^= 0x78;

  if (status & 0x40) {      /* right */
    if (status & 0x10) {    /* up */
      return STICK_UR;
    }
    else if (status & 0x20) {  /* down */
      return STICK_LR;
    }
    else {
      return STICK_RIGHT;
    }
  }
  else if (status & 0x80) {    /* left */
    if (status & 0x10) {    /* up */
      return STICK_UL;
    }
    else if (status & 0x20) {  /* down */
      return STICK_LL;
    }
    else {
      return STICK_LEFT;
    }
  }
  else {
    if (status & 0x10) {    /* up */
      return STICK_FORWARD;
    }
    else if (status & 0x20) {  /* down */
      return STICK_BACK;
    }
    else {
      return STICK_CENTRE;
    }
  }
#else
  return 0;
#endif /* LPTJOY */
}

# define JOY_UP       0
# define JOY_DOWN     1
# define JOY_LEFT     2
# define JOY_RIGHT    3
# define JOY_FIRE1    4
# define JOY_FIRE2    5

void SDL_Atari_PORT(Uint8 *s0, Uint8 *s1)
{
  int stick0, stick1;
  stick0 = stick1 = STICK_CENTRE;

  /* 0 Up  1 Down 2 Left 3 Right 4 Fire1 5 Fire2 */
  if (kbd_joy_0_enabled) {
    if (joystick_states[0][JOY_LEFT])
      stick0 = STICK_LEFT;
    if (joystick_states[0][JOY_RIGHT])
      stick0 = STICK_RIGHT;
    if (joystick_states[0][JOY_UP])
      stick0 = STICK_FORWARD;
    if (joystick_states[0][JOY_DOWN])
      stick0 = STICK_BACK;
    if ((joystick_states[0][JOY_LEFT]) && (joystick_states[0][JOY_UP]))
      stick0 = STICK_UL;
    if ((joystick_states[0][JOY_LEFT]) && (joystick_states[0][JOY_DOWN]))
      stick0 = STICK_LL;
    if ((joystick_states[0][JOY_RIGHT]) && (joystick_states[0][JOY_UP]))
      stick0 = STICK_UR;
    if ((joystick_states[0][JOY_RIGHT]) && (joystick_states[0][JOY_DOWN]))
      stick0 = STICK_LR;
  }

  if (SWAP_JOYSTICKS) {
    *s1 = stick0;
    *s0 = stick1;
  }
  else {
    *s0 = stick0;
    *s1 = stick1;
  }
# if 0 //LUDO:
  if (kbd_joy_0_enabled) {
    if (kbhits[KBD_STICK_0_LEFT])
      stick0 = STICK_LEFT;
    if (kbhits[KBD_STICK_0_RIGHT])
      stick0 = STICK_RIGHT;
    if (kbhits[KBD_STICK_0_UP])
      stick0 = STICK_FORWARD;
    if (kbhits[KBD_STICK_0_DOWN])
      stick0 = STICK_BACK;
    if ((kbhits[KBD_STICK_0_LEFTUP])
      || ((kbhits[KBD_STICK_0_LEFT]) && (kbhits[KBD_STICK_0_UP])))
      stick0 = STICK_UL;
    if ((kbhits[KBD_STICK_0_LEFTDOWN])
      || ((kbhits[KBD_STICK_0_LEFT]) && (kbhits[KBD_STICK_0_DOWN])))
      stick0 = STICK_LL;
    if ((kbhits[KBD_STICK_0_RIGHTUP])
      || ((kbhits[KBD_STICK_0_RIGHT]) && (kbhits[KBD_STICK_0_UP])))
      stick0 = STICK_UR;
    if ((kbhits[KBD_STICK_0_RIGHTDOWN])
      || ((kbhits[KBD_STICK_0_RIGHT]) && (kbhits[KBD_STICK_0_DOWN])))
      stick0 = STICK_LR;
  }
  if (kbd_joy_1_enabled) {
    if (kbhits[KBD_STICK_1_LEFT])
      stick1 = STICK_LEFT;
    if (kbhits[KBD_STICK_1_RIGHT])
      stick1 = STICK_RIGHT;
    if (kbhits[KBD_STICK_1_UP])
      stick1 = STICK_FORWARD;
    if (kbhits[KBD_STICK_1_DOWN])
      stick1 = STICK_BACK;
    if ((kbhits[KBD_STICK_1_LEFTUP])
      || ((kbhits[KBD_STICK_1_LEFT]) && (kbhits[KBD_STICK_1_UP])))
      stick1 = STICK_UL;
    if ((kbhits[KBD_STICK_1_LEFTDOWN])
      || ((kbhits[KBD_STICK_1_LEFT]) && (kbhits[KBD_STICK_1_DOWN])))
      stick1 = STICK_LL;
    if ((kbhits[KBD_STICK_1_RIGHTUP])
      || ((kbhits[KBD_STICK_1_RIGHT]) && (kbhits[KBD_STICK_1_UP])))
      stick1 = STICK_UR;
    if ((kbhits[KBD_STICK_1_RIGHTDOWN])
      || ((kbhits[KBD_STICK_1_RIGHT]) && (kbhits[KBD_STICK_1_DOWN])))
      stick1 = STICK_LR;
  }
  if (SWAP_JOYSTICKS) {
    *s1 = stick0;
    *s0 = stick1;
  }
  else {
    *s0 = stick0;
    *s1 = stick1;
  }

  if ((joystick0 != NULL) || (joystick1 != NULL))  /* can only joystick1!=NULL ? */
  {
    SDL_JoystickUpdate();
  }

  if (fd_joystick0 != -1)
    *s0 = get_LPT_joystick_state(fd_joystick0);
  else if (joystick0 != NULL)
    *s0 = get_SDL_joystick_state(joystick0);

  if (fd_joystick1 != -1)
    *s1 = get_LPT_joystick_state(fd_joystick1);
  else if (joystick1 != NULL)
    *s1 = get_SDL_joystick_state(joystick1);
# endif
}

void SDL_Atari_TRIG(Uint8 *t0, Uint8 *t1)
{
  int trig0, trig1, i;
  trig0 = trig1 = 1;

  if (kbd_joy_0_enabled) {
    trig0 = joystick_states[0][JOY_FIRE1] ? 0 : 1;
  }

  if (kbd_joy_1_enabled) {
    trig1 = joystick_states[0][JOY_FIRE2] ? 0 : 1;
  }

  if (SWAP_JOYSTICKS) {
    *t1 = trig0;
    *t0 = trig1;
  }
  else {
    *t0 = trig0;
    *t1 = trig1;
  }

# if 0 //LUDO:
  if (kbd_joy_0_enabled) {
    trig0 = kbhits[KBD_TRIG_0] ? 0 : 1;
  }

  if (kbd_joy_1_enabled) {
    trig1 = kbhits[KBD_TRIG_1] ? 0 : 1;
  }

  if (SWAP_JOYSTICKS) {
    *t1 = trig0;
    *t0 = trig1;
  }
  else {
    *t0 = trig0;
    *t1 = trig1;
  }

  if (fd_joystick0 != -1) {
#ifdef LPTJOY
    int status;
    ioctl(fd_joystick0, LPGETSTATUS, &status);
    if (status & 8)
      *t0 = 1;
    else
      *t0 = 0;
#endif /* LPTJOY */
  }
  else if (joystick0 != NULL) {
    trig0 = 1;
    for (i = 0; i < joystick0_nbuttons; i++) {
      if (SDL_JoystickGetButton(joystick0, i)) {
        trig0 = 0;
        break;
      }
    }
    *t0 = trig0;
  }

  if (fd_joystick1 != -1) {
#ifdef LPTJOY
    int status;
    ioctl(fd_joystick1, LPGETSTATUS, &status);
    if (status & 8)
      *t1 = 1;
    else
      *t1 = 0;
#endif /* LPTJOY */
  }
  else if (joystick1 != NULL) {
    trig1 = 1;
    for (i = 0; i < joystick1_nbuttons; i++) {
      if (SDL_JoystickGetButton(joystick1, i)) {
        trig1 = 0;
        break;
      }
    }
    *t1 = trig1;
  }
# endif
}

void CountFPS(void)
{
  static int ticks1 = 0, ticks2, shortframes;
  if (ticks1 == 0)
    ticks1 = SDL_GetTicks();
  ticks2 = SDL_GetTicks();
  shortframes++;
  if (ticks2 - ticks1 > 1000) {
    ticks1 = ticks2;
    Aprint("%d fps", shortframes);
    shortframes = 0;
  }
}

int Atari_PORT(int num)
{
#ifndef DONT_DISPLAY
  if (num == 0) {
    UBYTE a, b;
    SDL_Atari_PORT(&a, &b);
    return (b << 4) | (a & 0x0f);
  }
#endif
  return 0xff;
}

int Atari_TRIG(int num)
{
#ifndef DONT_DISPLAY
  UBYTE a, b;
  SDL_Atari_TRIG(&a, &b);
  switch (num) {
  case 0:
    return a;
  case 1:
    return b;
  default:
    break;
  }
#endif
  return 1;
}

int
SDL_main(int argc, char **argv)
{
  /* initialise Atari800 core */
  if (!Atari800_Initialise(&argc, argv))
    return 3;
  /* main loop */
  for (;;) {
    key_code = Atari_Keyboard();
    Atari800_Frame();
    if (display_screen)
      Atari_DisplayScreen();
  }
}

void
audio_pause(void)
{
  if (ATARI.atari_snd_enable) {
    SDL_PauseAudio(1);
  }
}

void
audio_resume(void)
{
  if (ATARI.atari_snd_enable) {
    SDL_PauseAudio(0);
  }
}

void
audio_shutdown(void)
{
  SDL_CloseAudio();
}

