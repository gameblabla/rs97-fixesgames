/*
 *  Copyright (C) 2009 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>

#include "global.h"
#include "psp_sdl.h"
#include "psp_kbd.h"
#include "psp_menu.h"
#include "psp_fmgr.h"
#include "psp_menu_kbd.h"
#include "psp_menu_set.h"

extern SDL_Surface *back_surface;

# define MENU_SET_SOUND         0
# define MENU_SET_VIEW_FPS      1
# define MENU_SET_SPEED_LIMIT   2
# define MENU_SET_SKIP_FPS      3
# define MENU_SET_RENDER        4
# define MENU_SET_DANZEFF       5
# define MENU_SET_VSYNC         6

# define MENU_SET_LOAD          7
# define MENU_SET_SAVE          8
# define MENU_SET_RESET         9
# define MENU_SET_BACK         10

# define MAX_MENU_SET_ITEM (MENU_SET_BACK + 1)

  static menu_item_t menu_list[] =
  {
    { "Sound enable       :"},
    { "Display fps        :"},
    { "Speed limiter      :"},
    { "Skip frame         :"},
    { "Render mode        :"},
    { "Virtual keyboard   :"},
    { "Vsync              :"},
    { "Load settings"        },
    { "Save settings"        },
    { "Reset settings"       },
    { "Back to Menu"         }
  };

  static int cur_menu_id = MENU_SET_LOAD;

  static int atari_snd_enable     = 0;
  static int atari_render_mode    = 0;
  static int atari_view_fps       = 0;
  static int atari_vsync          = 0;
  static int danzeff_trans        = 0;
  static int atari_speed_limiter  = 60;
  static int atari_skip_fps       = 0;


static void
psp_display_screen_settings_menu(void)
{
  char buffer[64];
  int menu_id = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  psp_sdl_blit_help();

  x      = 10;
  y      =  5;
  y_step = 10;

  for (menu_id = 0; menu_id < MAX_MENU_SET_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;

    psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);

    if (menu_id == MENU_SET_SOUND) {
      if (atari_snd_enable) strcpy(buffer,"yes");
      else                 strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(140, y, buffer, color);
    } else
    if (menu_id == MENU_SET_DANZEFF) {

      if (danzeff_trans) strcpy(buffer, "transparent");
      else               strcpy(buffer, "opaque");
      string_fill_with_space(buffer, 13);
      psp_sdl_back2_print(140, y, buffer, color);
    } else
    if (menu_id == MENU_SET_VIEW_FPS) {
      if (atari_view_fps) strcpy(buffer,"yes");
      else                strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(140, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SKIP_FPS) {
      sprintf(buffer,"%d", atari_skip_fps);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(140, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SPEED_LIMIT) {
      if (atari_speed_limiter == 0) strcpy(buffer,"no");
      else sprintf(buffer, "%d fps", atari_speed_limiter);
      string_fill_with_space(buffer, 10);
      psp_sdl_back2_print(140, y, buffer, color);
    } else
    if (menu_id == MENU_SET_VSYNC) {
      if (atari_vsync) strcpy(buffer,"yes");
      else                strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(140, y, buffer, color);
      y += y_step;
    } else
    if (menu_id == MENU_SET_RENDER) {

      strcpy(buffer, "normal");
      psp_sdl_back2_print(140, y, buffer, color);
      string_fill_with_space(buffer, 13);
      psp_sdl_back2_print(140, y, buffer, color);
    } else
    if (menu_id == MENU_SET_RESET) {
      y += y_step;
    }

    y += y_step;
  }

  // psp_menu_display_save_name();
}

static void
psp_settings_menu_render(int step)
{
  if (step > 0) {
    if (atari_render_mode < ATARI_LAST_RENDER) atari_render_mode++;
    else                                       atari_render_mode = 0;
  } else {
    if (atari_render_mode > 0) atari_render_mode--;
    else                       atari_render_mode = ATARI_LAST_RENDER;
  }
}

static void
psp_settings_menu_skip_fps(int step)
{
  if (step > 0) {
    if (atari_skip_fps < 25) atari_skip_fps++;
  } else {
    if (atari_skip_fps > 0) atari_skip_fps--;
  }
}

static void
psp_settings_menu_limiter(int step)
{
  if (step > 0) {
    if (atari_speed_limiter < 60) atari_speed_limiter++;
    else                          atari_speed_limiter  = 0;
  } else {
    if (atari_speed_limiter >  0) atari_speed_limiter--;
    else                          atari_speed_limiter  = 60;
  }
}

static void
psp_settings_menu_init(void)
{
  atari_snd_enable     = ATARI.atari_snd_enable;
  atari_render_mode    = ATARI.atari_render_mode;
  danzeff_trans        = ATARI.danzeff_trans;
  atari_vsync          = ATARI.atari_vsync;
  atari_speed_limiter  = ATARI.atari_speed_limiter;
  atari_view_fps       = ATARI.atari_view_fps;
  atari_skip_fps       = ATARI.psp_skip_max_frame;
}

static void
psp_settings_menu_load(int format)
{
  int ret;

  ret = psp_fmgr_menu(format);
  if (ret ==  1) /* load OK */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(170, 110, "File loaded !",
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
    psp_settings_menu_init();
  }
  else
  if (ret == -1) /* Load Error */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(170, 110, "Can't load file !",
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_settings_menu_validate(void)
{
  /* Validate */
  ATARI.atari_snd_enable    = atari_snd_enable;
  ATARI.atari_render_mode   = atari_render_mode;
  ATARI.danzeff_trans       = danzeff_trans;
  ATARI.atari_vsync         = atari_vsync;
  ATARI.atari_speed_limiter = atari_speed_limiter;
  ATARI.atari_view_fps      = atari_view_fps;
  ATARI.psp_skip_max_frame  = atari_skip_fps;
  ATARI.psp_skip_cur_frame  = 0;
}

static void
psp_settings_menu_save()
{
  int error;

  psp_settings_menu_validate();
  error = atari_save_settings();

  if (! error) /* save OK */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(170, 110, "File saved !",
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  else
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(170, 110, "Can't save file !",
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
}

void
psp_settings_menu_reset(void)
{
  psp_display_screen_settings_menu();
  psp_sdl_back2_print(170,110, "Reset Settings !",
                     PSP_MENU_WARNING_COLOR);
  psp_sdl_flip();
  atari_default_settings();
  psp_settings_menu_init();
  sleep(1);
}

int
psp_settings_menu(void)
{
  gp2xCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  psp_kbd_wait_no_button();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  psp_settings_menu_init();

  while (! end_menu)
  {
    psp_display_screen_settings_menu();
    psp_sdl_flip();

    while (1)
    {
      gp2xCtrlReadBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;

      if (c.Buttons) break;
    }

    new_pad = c.Buttons;

    if ((old_pad != new_pad) || ((c.TimeStamp - last_time) > PSP_MENU_MIN_TIME)) {
      last_time = c.TimeStamp;
      old_pad = new_pad;

    } else continue;

    if ((c.Buttons & GP2X_CTRL_RTRIGGER) == GP2X_CTRL_RTRIGGER) {
      psp_settings_menu_reset();
      end_menu = 1;
    } else
    if ((new_pad == GP2X_CTRL_LEFT ) ||
        (new_pad == GP2X_CTRL_RIGHT) ||
        (new_pad == GP2X_CTRL_CROSS) ||
        (new_pad == GP2X_CTRL_CIRCLE))
    {
      int step = 0;

      if (new_pad & GP2X_CTRL_RIGHT) {
        step = 1;
      } else
      if (new_pad & GP2X_CTRL_LEFT) {
        step = -1;
      }

      switch (cur_menu_id )
      {
        case MENU_SET_SOUND      : atari_snd_enable = ! atari_snd_enable;
        break;
        case MENU_SET_SPEED_LIMIT : psp_settings_menu_limiter( step );
        break;
        case MENU_SET_SKIP_FPS   : psp_settings_menu_skip_fps( step );
        break;
        case MENU_SET_VIEW_FPS   : atari_view_fps = ! atari_view_fps;
        break;
        break;
        case MENU_SET_RENDER     : psp_settings_menu_render( step );
        break;
        case MENU_SET_DANZEFF    : danzeff_trans = ! danzeff_trans;
        break;
        case MENU_SET_VSYNC      : atari_vsync = ! atari_vsync;
        break;
        case MENU_SET_LOAD       : psp_settings_menu_load(FMGR_FORMAT_SET);
                                   old_pad = new_pad = 0;
        break;
        case MENU_SET_SAVE       : psp_settings_menu_save();
                                   old_pad = new_pad = 0;
        break;
        case MENU_SET_RESET      : psp_settings_menu_reset();
        break;

        case MENU_SET_BACK       : end_menu = 1;
        break;
      }

    } else
    if(new_pad & GP2X_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_SET_ITEM-1;

    } else
    if(new_pad & GP2X_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_SET_ITEM-1)) cur_menu_id++;
      else                                     cur_menu_id = 0;

    } else
    if(new_pad & GP2X_CTRL_SQUARE) {
      /* Cancel */
      end_menu = -1;
    } else
    if(new_pad & GP2X_CTRL_SELECT) {
      /* Back to ATARI */
      end_menu = 1;
    }
  }

  if (end_menu > 0) {
    psp_settings_menu_validate();
  }

  psp_kbd_wait_no_button();

  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();
  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();

  return 1;
}

