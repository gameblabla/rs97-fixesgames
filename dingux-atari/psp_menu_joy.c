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
#include "psp_kbd.h"

extern SDL_Surface *back_surface;

# define MENU_JOY_ANALOG        0
# define MENU_JOY_AUTOFIRE_T    1
# define MENU_JOY_AUTOFIRE_M    2

# define MENU_JOY_LOAD          3
# define MENU_JOY_SAVE          4
# define MENU_JOY_RESET         5
# define MENU_JOY_BACK          6

# define MAX_MENU_JOY_ITEM (MENU_JOY_BACK + 1)

  static menu_item_t menu_list[] =
  {
    { "Swap Analog/Cursor :"},
    { "Auto fire period   :"},
    { "Auto fire mode     :"},

    { "Load joystick"       },
    { "Save joystick"       },
    { "Reset joystick"      },

    { "Back to Menu"        }
  };

  static int cur_menu_id = MENU_JOY_LOAD;

  static int psp_reverse_analog    = 0;
  static int atari_auto_fire_period = 0;
  static int atari_auto_fire_mode   = 0;
 

static void
psp_joystick_menu_reset(void);

static void
psp_joystick_menu_autofire(int step)
{
  if (step > 0) {
    if (atari_auto_fire_period < 19) atari_auto_fire_period++;
  } else {
    if (atari_auto_fire_period >  0) atari_auto_fire_period--;
  }
}

static void 
psp_display_screen_joystick_menu(void)
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
  
  for (menu_id = 0; menu_id < MAX_MENU_JOY_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;

    psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);

    if (menu_id == MENU_JOY_ANALOG) {
      if (psp_reverse_analog) strcpy(buffer,"yes");
      else                    strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(140, y, buffer, color);
    } else
    if (menu_id == MENU_JOY_AUTOFIRE_T) {
      sprintf(buffer,"%d", atari_auto_fire_period+1);
      string_fill_with_space(buffer, 7);
      psp_sdl_back2_print(140, y, buffer, color);
    } else
    if (menu_id == MENU_JOY_AUTOFIRE_M) {
      if (atari_auto_fire_mode) strcpy(buffer,"yes");
      else                    strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(140, y, buffer, color);
      y += y_step;
    } else
    if (menu_id == MENU_JOY_RESET) {
      //y += y_step;
    }

    y += y_step;
  }

  // psp_menu_display_save_name();
}

static void
psp_joystick_menu_init(void)
{
  psp_reverse_analog    = ATARI.psp_reverse_analog;
  atari_auto_fire_period = ATARI.atari_auto_fire_period;
  atari_auto_fire_mode   = ATARI.atari_auto_fire;
}

static void
psp_joystick_menu_validate(void)
{
  /* Validate */
  ATARI.psp_reverse_analog  = psp_reverse_analog;
  ATARI.atari_auto_fire_period = atari_auto_fire_period;
  if (atari_auto_fire_mode != ATARI.atari_auto_fire) {
    kbd_change_auto_fire(atari_auto_fire_mode);
  }
}

static void
psp_joystick_menu_load(int format)
{
  int ret;

  ret = psp_fmgr_menu(format);
  if (ret ==  1) /* load OK */
  {
    psp_display_screen_joystick_menu();
    psp_sdl_back2_print(180,  80, "File loaded !", 
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
    psp_joystick_menu_init();
  }
  else 
  if (ret == -1) /* Load Error */
  {
    psp_display_screen_joystick_menu();
    psp_sdl_back2_print(180,  80, "Can't load file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_joystick_menu_save()
{
  int error;

  psp_joystick_menu_validate();
  error = atari_joy_save();

  if (! error) /* save OK */
  {
    psp_display_screen_joystick_menu();
    psp_sdl_back2_print(180, 80, "File saved !", 
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_joystick_menu();
    psp_sdl_back2_print(180, 80, "Can't save file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_joystick_menu_reset(void)
{
  psp_display_screen_joystick_menu();
  psp_sdl_back2_print( 180, 80, "Reset joystick !", 
                     PSP_MENU_WARNING_COLOR);
  psp_sdl_flip();
  psp_joy_default_settings();
  psp_joystick_menu_init();
  sleep(1);
}

int 
psp_joystick_menu(void)
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

  psp_joystick_menu_init();

  while (! end_menu)
  {
    psp_display_screen_joystick_menu();
    psp_sdl_flip();

    while (1)
    {
      gp2xCtrlPeekBufferPositive(&c, 1);
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
        case MENU_JOY_ANALOG     : psp_reverse_analog = ! psp_reverse_analog;
        break;              
        case MENU_JOY_AUTOFIRE_T  : psp_joystick_menu_autofire( step );
        break;              
        case MENU_JOY_AUTOFIRE_M  : atari_auto_fire_mode = ! atari_auto_fire_mode;
        break;              
        case MENU_JOY_LOAD       : psp_joystick_menu_load(FMGR_FORMAT_JOY);
                                   old_pad = new_pad = 0;
        break;              
        case MENU_JOY_SAVE       : psp_joystick_menu_save();
                                   old_pad = new_pad = 0;
        break;                     
        case MENU_JOY_RESET      : psp_joystick_menu_reset();
        break;                     
        case MENU_JOY_BACK       : end_menu = 1;
        break;                     
      }

    } else
    if(new_pad & GP2X_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_JOY_ITEM-1;

    } else
    if(new_pad & GP2X_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_JOY_ITEM-1)) cur_menu_id++;
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
    psp_joystick_menu_validate();
  }

  psp_kbd_wait_no_button();

  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();
  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();

  return 1;
}
