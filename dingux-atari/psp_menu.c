/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
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
#include "psp_menu_joy.h"
#include "psp_menu_set.h"
#include "psp_menu_cheat.h"
#include "psp_menu_help.h"
#include "psp_editor.h"

extern SDL_Surface *back_surface;

# define MENU_SCREENSHOT   0
//# define MENU_VOLUME       1

# define MENU_LOAD_SLOT    1
# define MENU_SAVE_SLOT    2
# define MENU_DEL_SLOT     3

# define MENU_HELP         4

# define MENU_LOAD_ROM     5
# define MENU_LOAD_CART    6
# define MENU_ORIGINAL     7

# define MENU_EDITOR       8
# define MENU_CHEATS       9
# define MENU_KEYBOARD    10
# define MENU_JOYSTICK    11
# define MENU_SETTINGS    12

# define MENU_RESET       13
# define MENU_BACK        14
# define MENU_EXIT        15

# define MAX_MENU_ITEM (MENU_EXIT + 1)

  static menu_item_t menu_list[] =
  {
    { "Save Screenshot :" },
//    { "Volume          :" },
    { "Load Slot" },
    { "Save Slot" },
    { "Delete Slot" },

    { "Help" },

    { "Load Rom 800" },
    { "Load Cart 5200" },

    { "Atari menu" },

    { "Comments" },
    { "Cheats" },
    { "Keyboard" },
    { "Joystick" },
    { "Settings" },

    { "Reset Atari" },
    { "Back to Atari" },
    { "Exit" }
  };

  static int cur_menu_id = MENU_LOAD_ROM;
  static int cur_slot    = 0;

void
psp_menu_display_save_name()
{
  char buffer[128];
  int Length;

  snprintf(buffer, 30, "Game: %s", ATARI.atari_save_name);
  Length = strlen(buffer);
  psp_sdl_back2_print(300 - (6*Length),  5, buffer, PSP_MENU_TEXT2_COLOR);
}

void
string_fill_with_space(char *buffer, int size)
{
  int length = strlen(buffer);
  int index;

  for (index = length; index < size; index++) {
    buffer[index] = ' ';
  }
  buffer[size] = 0;
}

void
psp_display_screen_menu(void)
{
  char buffer[64];
  int menu_id = 0;
  int slot_id = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  psp_sdl_blit_background();

  x      = 10;
  y      = 15;
  y_step = 10;

  for (menu_id = 0; menu_id < MAX_MENU_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;
    else
    if (menu_id == MENU_EXIT) color = PSP_MENU_WARNING_COLOR;
    else
    if (menu_id == MENU_HELP) color = PSP_MENU_GREEN_COLOR;
    else
    if (menu_id == MENU_ORIGINAL) color = PSP_MENU_TEXT2_COLOR;

    if (menu_id == MENU_EDITOR) {
      if (ATARI.comment_present) {
        psp_sdl_back2_print(x, y, "View comment", color);
      } else {
        psp_sdl_back2_print(x, y, "New comment", color);
      }

    } else {
      psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);
    }

    if (menu_id == MENU_SCREENSHOT) {
      sprintf(buffer,"%d", ATARI.psp_screenshot_id);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(130, y, buffer, color);
      y += y_step;
      y += y_step;
/*    } else
    if (menu_id == MENU_VOLUME) {
      sprintf(buffer,"%d", gp2xGetSoundVolume());
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(130, y, buffer, color);
      y += y_step;*/
    } else
    if (menu_id == MENU_DEL_SLOT) {
      y += y_step;
    } else
    if (menu_id == MENU_ORIGINAL) {
      y += y_step;
    } else
    if (menu_id == MENU_SETTINGS) {
      y += y_step;
    } else
    if (menu_id == MENU_BACK) {
      // y += y_step;
    } else
    if (menu_id == MENU_CHEATS) {
    } else
    if (menu_id == MENU_HELP) {
      y += y_step;
    }

    y += y_step;
  }
  y_step = 10;
  y      = 45;

  for (slot_id = 0; slot_id < ATARI_MAX_SAVE_STATE; slot_id++) {
    if (slot_id == cur_slot) color = PSP_MENU_SEL2_COLOR;
    else                     color = PSP_MENU_TEXT_COLOR;

    if (ATARI.atari_save_state[slot_id].used) {
# if defined(LINUX_MODE) || defined(WIZ_MODE)
      struct tm *my_date = localtime(& ATARI.atari_save_state[slot_id].date);
      sprintf(buffer, "- %02d/%02d %02d:%02d:%02d",
         my_date->tm_mday, my_date->tm_mon + 1,
         my_date->tm_hour, my_date->tm_min, my_date->tm_sec );
# else
      sprintf(buffer, "- used");
# endif
    } else {
      sprintf(buffer, "- empty");
    }
    string_fill_with_space(buffer, 32);
    psp_sdl_back2_print(100, y, buffer, color);

    y += y_step;
  }

  if (ATARI.atari_save_state[cur_slot].thumb) {
    psp_sdl_blit_thumb(168,125, ATARI.atari_save_state[cur_slot].surface);
  } else {
    psp_sdl_blit_thumb(168,125, thumb_surface);
  }

  psp_menu_display_save_name();
}

static void
psp_main_menu_original(void)
{
  psp_kbd_run_command_ui();
}

static void
psp_main_menu_reset(void)
{
  /* Reset ! */
  psp_display_screen_menu();
  psp_sdl_back2_print( 140, 100, "Reset Atari !", PSP_MENU_WARNING_COLOR);
  psp_sdl_flip();
  emulator_reset();
  sleep(1);
}

static int
psp_main_menu_load(int format)
{
  int ret;

  ret = psp_fmgr_menu(format);

  if (ret ==  1) /* load OK */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print( 140, 100, "File loaded !",
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
    return 1;
  }
  else
  if (ret == -1) /* Load Error */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print( 140, 100, "Can't load file !",
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  return 0;
}

/* used by command keys */
int
psp_main_menu_load_current()
{
  if (ATARI.atari_save_state[cur_slot].used) {
    return atari_snapshot_load_slot(cur_slot);
  }
  return -1;
}

int
psp_main_menu_save_current()
{
  return atari_snapshot_save_slot(cur_slot);
}

static int
psp_main_menu_load_slot()
{
  int error;

  if (! ATARI.atari_save_state[cur_slot].used) {

    psp_display_screen_menu();
    psp_sdl_back2_print( 140, 100, "Slot is empty !",
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);

    return 0;
  }

  error = atari_snapshot_load_slot(cur_slot);

  if (! error) /* load OK */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print( 140, 100, "File loaded !",
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);

    return 1;
  }
  else
  {
    psp_display_screen_menu();
    psp_sdl_back2_print( 140, 100, "Can't load file !",
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  return 0;
}

static void
psp_main_menu_save_slot()
{
  int error;
  error = atari_snapshot_save_slot(cur_slot);

  if (! error) /* save OK */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print( 140, 100, "File saved !",
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  else
  {
    psp_display_screen_menu();
    psp_sdl_back2_print( 140, 100, "Can't save file !",
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_main_menu_del_slot()
{
  int error;

  if (! ATARI.atari_save_state[cur_slot].used) {

    psp_display_screen_menu();
    psp_sdl_back2_print( 140, 100, "Slot is empty !",
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);

    return;
  }

  error = atari_snapshot_del_slot(cur_slot);

  if (! error) /* save OK */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print( 140, 100, "File deleted !",
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  else
  {
    psp_display_screen_menu();
    psp_sdl_back2_print( 140, 100, "Can't delete file !",
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_main_menu_cur_slot(int step)
{
  if (step == 1) {
    cur_slot++; if (cur_slot >= ATARI_MAX_SAVE_STATE) cur_slot = 0;
  } else if (step == -1) {
    cur_slot--; if (cur_slot < 0) cur_slot = ATARI_MAX_SAVE_STATE - 1;
  }
}

static void
psp_main_menu_screenshot(void)
{
  psp_screenshot_mode = 10;
}

static void
psp_main_menu_volume(int step)
{
  if (step == 1) {
    gp2xIncreaseVolume();
  } else if (step == -1) {
    gp2xDecreaseVolume();
  }
}

int
psp_main_menu_exit(void)
{
  gp2xCtrlData c;

  psp_display_screen_menu();
  psp_sdl_back2_print( 140, 100, "press B to confirm !", PSP_MENU_WARNING_COLOR);
  psp_sdl_flip();

  psp_kbd_wait_no_button();

  do
  {
    gp2xCtrlReadBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (c.Buttons & GP2X_CTRL_CROSS) {
      psp_sdl_clear_screen(0);
      psp_sdl_flip();
      psp_sdl_clear_screen(0);
      psp_sdl_flip();
      psp_sdl_exit(0);
    }

  } while (c.Buttons == 0);

  psp_kbd_wait_no_button();

  return 0;
}

void
psp_main_menu_editor()
{
  char TmpFileName[MAX_PATH];
  sprintf(TmpFileName, "%s/txt/%s.txt", ATARI.atari_home_dir, ATARI.atari_save_name);
  psp_editor_menu( TmpFileName );
}

int
psp_main_menu(void)
{
  gp2xCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  audio_pause();

  psp_kbd_wait_no_button();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  while (! end_menu)
  {
    psp_display_screen_menu();
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

    if ((c.Buttons & GP2X_CTRL_LTRIGGER) == GP2X_CTRL_LTRIGGER) {
      psp_settings_menu();
      old_pad = new_pad = 0;
    } else
    if ((c.Buttons & GP2X_CTRL_RTRIGGER) == GP2X_CTRL_RTRIGGER) {
      psp_main_menu_reset();
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
        case MENU_LOAD_SLOT : if (step) psp_main_menu_cur_slot(step);
                              else
                              if (psp_main_menu_load_slot()) {
                                end_menu = 1;
                              }
        break;
        case MENU_SAVE_SLOT : if (step) psp_main_menu_cur_slot(step);
                              else      psp_main_menu_save_slot();
        break;
        case MENU_DEL_SLOT  : if (step) psp_main_menu_cur_slot(step);
                              else      psp_main_menu_del_slot();
        break;

        case MENU_LOAD_ROM  : if (psp_main_menu_load(FMGR_FORMAT_ROM)) {
                                end_menu = 1;
                              }
                              old_pad = new_pad = 0;
        break;
        case MENU_LOAD_CART : if (psp_main_menu_load(FMGR_FORMAT_CART)) {
                                end_menu = 1;
                              }
                              old_pad = new_pad = 0;
        break;
        case MENU_ORIGINAL  : psp_main_menu_original();
                              end_menu = 1;
        break;
        case MENU_EDITOR    : psp_main_menu_editor();
                              old_pad = new_pad = 0;
        break;

        case MENU_CHEATS    : psp_cheat_menu();
                              old_pad = new_pad = 0;
        break;

        case MENU_KEYBOARD   : psp_keyboard_menu();
                               old_pad = new_pad = 0;
        break;
        case MENU_JOYSTICK   : psp_joystick_menu();
                               old_pad = new_pad = 0;
        break;
        case MENU_SETTINGS   : psp_settings_menu();
                               old_pad = new_pad = 0;
        break;

        case MENU_SCREENSHOT : psp_main_menu_screenshot();
                               end_menu = 1;
        break;
/*        case MENU_VOLUME     : psp_main_menu_volume(step);
                               old_pad = new_pad = 0;*/
        break;

        case MENU_RESET     : psp_main_menu_reset();
                              end_menu = 1;
        break;

        case MENU_BACK      : end_menu = 1;
        break;

        case MENU_EXIT      : psp_main_menu_exit();
        break;

        case MENU_HELP      : psp_help_menu();
                              old_pad = new_pad = 0;
        break;
      }

    } else
    if(new_pad & GP2X_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_ITEM-1;

    } else
    if(new_pad & GP2X_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_ITEM-1)) cur_menu_id++;
      else                                 cur_menu_id = 0;

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

  psp_kbd_wait_no_button();

  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();
  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();

  psp_sdl_clear_blit(0);

  audio_resume();

  return 1;
}

