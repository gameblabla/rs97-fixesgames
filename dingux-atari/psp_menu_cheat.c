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
#include "psp_menu_set.h"
#include "psp_menu_cheat.h"
#include "psp_menu_list.h"

extern SDL_Surface *back_surface;

# define MENU_CHEAT_OLD_VAL   0
# define MENU_CHEAT_NEW_VAL   1
# define MENU_CHEAT_POKE      2
# define MENU_CHEAT_ADD       3

# define MENU_CHEAT_RAM       4

# define MENU_CHEAT_ENABLE    5
# define MENU_CHEAT_ALL       6
# define MENU_CHEAT_VALUE     7
# define MENU_CHEAT_IMPORT    8
# define MENU_CHEAT_EDIT      9

# define MENU_CHEAT_DEL      10

# define MENU_CHEAT_LOAD     11
# define MENU_CHEAT_SAVE     12
# define MENU_CHEAT_RESET    13
                                 
# define MENU_CHEAT_BACK     14

# define MAX_MENU_CHEAT_ITEM (MENU_CHEAT_BACK + 1)

  static menu_item_t menu_list[] =
  {
    { "Scan Old   :"},
    { "Scan New   :"},
    { "Poke Value :"},
    { "Add Cheat  :"},
    { "Save RAM"    },

    { "Enable : "},
    { "All    : "},
    { "Value  : "},
    { "Import : "},
    { "Edit   : "},

    { "Delete : "},

    { "Load cheat" },
    { "Save cheat" },
    { "Reset cheat" },

    { "Back to Menu"         }
  };

# define MAX_SCAN_ADDR    10

  static int cur_menu_id    = MENU_CHEAT_LOAD;
  static int cur_cheat      = 0;

  static uchar  scan_old_value  = 3;
  static uchar  scan_new_value  = 3;
  static uchar  scan_poke_value = 3;

  static uchar *scan_prev_ram   = 0;
  static int    scan_addr[MAX_SCAN_ADDR];
  static int    scan_addr_found = 0;

static void 
psp_display_screen_cheat_menu(void)
{
  char buffer[128];
  int menu_id = 0;
  int color   = 0;
  int cheat_id = 0;
  int addr_id = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;
  int sav_y   = 0;
  int first   = 0;
  char* scan  = 0;

  psp_sdl_blit_help();

  x      = 10;
  y      =  5;
  y_step = 10;
  
  for (menu_id = 0; menu_id < MAX_MENU_CHEAT_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (menu_id == MENU_CHEAT_DEL) color = PSP_MENU_NOTE_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;

    psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);

    if (menu_id == MENU_CHEAT_OLD_VAL) {
      sprintf(buffer,"%02X", scan_old_value);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print( 90, y, buffer, color);
    } else
    if (menu_id == MENU_CHEAT_NEW_VAL) {
      sprintf(buffer,"%02X", scan_new_value);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print( 90, y, buffer, color);
    } else
    if (menu_id == MENU_CHEAT_POKE) {
      sprintf(buffer,"%02X", scan_poke_value);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print( 90, y, buffer, color);
    } else
    if (menu_id == MENU_CHEAT_ADD) {
      if (scan_addr_found) {
        scan = buffer;
        buffer[0] = 0;
        first = 1;
        sav_y = y;
        for (addr_id = 0; addr_id < scan_addr_found; addr_id++) {
          if (addr_id >= MAX_SCAN_ADDR) break;
          if (first) {
            sprintf(scan, "%04X", scan_addr[addr_id]);
            first = 0;
            scan += 4;
          } else {
            sprintf(scan, ",%04X", scan_addr[addr_id]);
            scan += 5;
            if (addr_id == 4) {
              string_fill_with_space(buffer, 38);
              psp_sdl_back2_print( 90, y, buffer, color);
              scan = buffer;
              buffer[0] = 0;
              first = 1;
              y += y_step;
            }
          }
        }
        if (scan_addr_found >= MAX_SCAN_ADDR) {
          sprintf(scan," ... (%d)", scan_addr_found );
        }
        string_fill_with_space(buffer, 38);
        psp_sdl_back2_print( 90, y, buffer, color);
        y = sav_y;

      } else {
        sprintf(buffer,"No match");
        string_fill_with_space(buffer, 38);
        psp_sdl_back2_print( 90, y, buffer, color);
      }
      //y += y_step;
    } else
    if (menu_id == MENU_CHEAT_RAM) {
      y += y_step;
    } else
    if (menu_id == MENU_CHEAT_EDIT) {
      y += y_step;
    } else
    if (menu_id == MENU_CHEAT_DEL) {
      y += (ATARI_MAX_CHEAT - 6) * y_step;
    } else
    if (menu_id == MENU_CHEAT_RESET) {
      y += y_step;
    }
    y += y_step;
  }

  y_step = 10;
  y      = 65;

  for (cheat_id = 0; cheat_id < ATARI_MAX_CHEAT; cheat_id++) {
    if (cheat_id == cur_cheat) color = PSP_MENU_SEL2_COLOR;
    else                            color = PSP_MENU_TEXT_COLOR;

    Atari_cheat_t* a_cheat = &ATARI.atari_cheat[cheat_id];

    if (a_cheat->type != ATARI_CHEAT_NONE) {
      char enable = (a_cheat->type == ATARI_CHEAT_ENABLE) ? 'X' : ' ';
      sprintf(buffer, "[%c] %04X-%02X %s", enable, a_cheat->addr, a_cheat->value, a_cheat->comment);
    } else {
      sprintf(buffer, "[ ]     -   Empty");
    }
    string_fill_with_space(buffer, 36);
    psp_sdl_back2_print(90, y, buffer, color);

    y += y_step;
  }

  // psp_menu_display_save_name();
}


static void
psp_cheat_menu_load(int format)
{
  int ret;

  ret = psp_fmgr_menu(format);
  if (ret ==  1) /* load OK */
  {
    psp_display_screen_cheat_menu();
    psp_sdl_back2_print(150, 180, "File loaded !", 
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  else 
  if (ret == -1) /* Load Error */
  {
    psp_display_screen_cheat_menu();
    psp_sdl_back2_print(150, 180, "Can't load file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_cheat_menu_save()
{
  int error;

  error = atari_save_cheat();

  if (! error) /* save OK */
  {
    psp_display_screen_cheat_menu();
    psp_sdl_back2_print(150, 180, "File saved !", 
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_cheat_menu();
    psp_sdl_back2_print(150, 180, "Can't save file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_cheat_menu_del_cheat()
{
  Atari_cheat_t* a_cheat = &ATARI.atari_cheat[cur_cheat];

  if (a_cheat->type == ATARI_CHEAT_NONE) {

    psp_display_screen_cheat_menu();
    psp_sdl_back2_print(150, 180, "Cheat is empty !", 
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);

    return;
  }
  memset(a_cheat, 0, sizeof(*a_cheat));
}

static void
psp_cheat_menu_add_cheat()
{
  int cheat_id;
  int addr;

  if (! scan_addr_found) {
    psp_display_screen_cheat_menu();
    psp_sdl_back2_print(150, 180, "No address to add !", 
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);

    return;
  }

  int num_free  = 0;
  for (cheat_id = 0; cheat_id < ATARI_MAX_CHEAT; cheat_id++) {
    Atari_cheat_t* a_cheat = &ATARI.atari_cheat[cheat_id];
    if (a_cheat->type == ATARI_CHEAT_NONE) num_free++;
  }
  if (! num_free) num_free = 1;

  int num_add = 0;
  u8 *a_ram = get_memory_ram();
  for (addr = 0; addr < ATARI_RAM_SIZE; addr++) {
    if ((scan_prev_ram[addr] == scan_old_value) &&
        (a_ram[addr]    == scan_new_value  )) {

      for (cheat_id = 0; cheat_id < ATARI_MAX_CHEAT; cheat_id++) {
        Atari_cheat_t* a_cheat = &ATARI.atari_cheat[cheat_id];
        if (a_cheat->type == ATARI_CHEAT_NONE) break;
      }
      if (cheat_id >= ATARI_MAX_CHEAT) {
        cheat_id = cur_cheat;
      }
      Atari_cheat_t* a_cheat = &ATARI.atari_cheat[cheat_id];
      memset(a_cheat, 0, sizeof(*a_cheat));
      a_cheat->type  = ATARI_CHEAT_ENABLE;
      a_cheat->addr  = addr;
      a_cheat->value = scan_poke_value;
      sprintf(a_cheat->comment, "Cheat %04X", addr);
      if (++num_add >= num_free) break;
    }
  }
}

static void
psp_cheat_menu_reset_cheat()
{
  psp_display_screen_cheat_menu();
  psp_sdl_back2_print(150, 180, "Reset cheat !", PSP_MENU_NOTE_COLOR);
  psp_sdl_flip();
  sleep(1);

  memset(ATARI.atari_cheat, 0, sizeof(ATARI.atari_cheat));
}

static void
psp_cheat_menu_enable_cheat()
{
  Atari_cheat_t* a_cheat = &ATARI.atari_cheat[cur_cheat];

  if (a_cheat->type == ATARI_CHEAT_NONE) {

    psp_display_screen_cheat_menu();
    psp_sdl_back2_print(150, 180, "Cheat is empty !", 
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);

    return;
  }

  if (a_cheat->type == ATARI_CHEAT_ENABLE) a_cheat->type = ATARI_CHEAT_DISABLE;
  else                                    a_cheat->type = ATARI_CHEAT_ENABLE;
}

static void
psp_cheat_menu_enable_all_cheat()
{
  int cheat_id;
  int cur_type = ATARI_CHEAT_NONE;

  for (cheat_id = 0; cheat_id < ATARI_MAX_CHEAT; cheat_id++) {
    Atari_cheat_t* a_cheat = &ATARI.atari_cheat[cheat_id];
    if (a_cheat->type != ATARI_CHEAT_NONE) {
      if (cur_type == ATARI_CHEAT_NONE) {
        if (a_cheat->type == ATARI_CHEAT_ENABLE) cur_type = ATARI_CHEAT_DISABLE;
        else                                   cur_type = ATARI_CHEAT_ENABLE;
      }
      a_cheat->type = cur_type;
    }
  }
  if (cur_type == ATARI_CHEAT_NONE) {

    psp_display_screen_cheat_menu();
    psp_sdl_back2_print(150, 180, "Cheats are empty !", 
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);

    return;
  }
}

static void
psp_cheat_menu_value_cheat(int step)
{
  Atari_cheat_t* a_cheat = &ATARI.atari_cheat[cur_cheat];

  if (a_cheat->type == ATARI_CHEAT_NONE) {

    psp_display_screen_cheat_menu();
    psp_sdl_back2_print(150, 180, "Cheat is empty !", 
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_flip();
    sleep(1);

    return;
  }

  if (step ==  1) a_cheat->value++;
  else
  if (step == -1) a_cheat->value--;
}

static void
psp_cheat_menu_cur_cheat(int step)
{
  if (step == 1) {
    cur_cheat++; if (cur_cheat >= ATARI_MAX_CHEAT) cur_cheat = 0;
  } else if (step == -1) {
    cur_cheat--; if (cur_cheat < 0) cur_cheat = ATARI_MAX_CHEAT - 1;
  }
}

static void
psp_cheat_update_match()
{
  int addr;
  u8 *a_ram = get_memory_ram();

  scan_addr_found = 0;
  for (addr = 0; addr < ATARI_RAM_SIZE; addr++) {
    if ((scan_prev_ram[addr] == scan_old_value) &&
        (a_ram[addr]           == scan_new_value  )) {
      if (scan_addr_found < MAX_SCAN_ADDR) {
        scan_addr[scan_addr_found] = addr;
      }
      scan_addr_found++;
    }
  }
}

static void
psp_cheat_menu_new_val(int step)
{
  if (step == 1) scan_new_value++; 
  else
  if (step == -1) scan_new_value--;

  psp_cheat_update_match();
}

static void
psp_cheat_menu_old_val(int step)
{
  if (step == 1) scan_old_value++; 
  else
  if (step == -1) scan_old_value--;

  psp_cheat_update_match();
}

static void
psp_cheat_menu_poke_val(int step)
{
  if (step == 1) scan_poke_value++; 
  else
  if (step == -1) scan_poke_value--;

  psp_cheat_update_match();
}


static void
psp_cheat_menu_cheat_list()
{
  int cheat_id;

  for (cheat_id = 0; cheat_id < ATARI_MAX_CHEAT; cheat_id++) {
    Atari_cheat_t* a_cheat = &ATARI.atari_cheat[cheat_id];
    if (a_cheat->type == ATARI_CHEAT_NONE) break;
  }
  if (cheat_id >= ATARI_MAX_CHEAT) {
    cheat_id = cur_cheat;
  }
  psp_menu_cheat_list(cheat_id);
}

static void
psp_cheat_menu_edit_list()
{
  char TmpFileName[MAX_PATH];
  sprintf(TmpFileName, "%s/cheat.txt", ATARI.atari_home_dir);
  psp_editor_menu(TmpFileName);
}

static void
psp_cheat_menu_init()
{
  u8 *a_ram = get_memory_ram();
  if (scan_prev_ram == 0) {
    scan_prev_ram = (uchar *)malloc( ATARI_RAM_SIZE );
    memcpy(scan_prev_ram, a_ram, ATARI_RAM_SIZE);
  }
  psp_cheat_update_match();
}

static void
psp_cheat_menu_save_ram()
{
  u8 *a_ram = get_memory_ram();
  memcpy(scan_prev_ram, a_ram, ATARI_RAM_SIZE);
  psp_cheat_update_match();

  psp_display_screen_cheat_menu();
  psp_sdl_back2_print(150, 180, "Ram saved !", 
                      PSP_MENU_NOTE_COLOR);
  psp_sdl_flip();
  sleep(1);
}

int 
psp_cheat_menu(void)
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

  psp_cheat_menu_init();

  while (! end_menu)
  {
    psp_display_screen_cheat_menu();
    psp_sdl_flip();

    while (1)
    {
      gp2xCtrlPeekBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;

# ifdef USE_PSP_IRKEYB
      psp_irkeyb_set_psp_key(&c);
# endif
      if (c.Buttons) break;
    }

    new_pad = c.Buttons;

    if ((old_pad != new_pad) || ((c.TimeStamp - last_time) > PSP_MENU_MIN_TIME)) {
      last_time = c.TimeStamp;
      old_pad = new_pad;

    } else continue;

    if ((c.Buttons & (GP2X_CTRL_LTRIGGER|GP2X_CTRL_RTRIGGER|GP2X_CTRL_START)) ==
        (GP2X_CTRL_LTRIGGER|GP2X_CTRL_RTRIGGER|GP2X_CTRL_START)) {
      /* Exit ! */
      psp_sdl_exit(0);
    } else
    if ((c.Buttons & GP2X_CTRL_LTRIGGER) == GP2X_CTRL_LTRIGGER) {
      psp_keyboard_menu();
      end_menu = 1;
    } else
    if ((c.Buttons & GP2X_CTRL_RTRIGGER) == GP2X_CTRL_RTRIGGER) {
      psp_cheat_menu_reset_cheat();
    } else
    if ((new_pad == GP2X_CTRL_LEFT ) || 
        (new_pad == GP2X_CTRL_RIGHT) ||
        (new_pad == GP2X_CTRL_CROSS) || 
        (new_pad == GP2X_CTRL_CIRCLE))
    {
      int step;
      int step2;

      if (new_pad & GP2X_CTRL_LEFT)  step = -1;
      else 
      if (new_pad & GP2X_CTRL_RIGHT) step =  1;
      else                          step =  0;

      if (new_pad & GP2X_CTRL_CROSS)  step2 = -1;
      else 
      if (new_pad & GP2X_CTRL_CIRCLE) step2 =  1;
      else                           step2 =  0;

      switch (cur_menu_id ) 
      {
        case MENU_CHEAT_OLD_VAL : psp_cheat_menu_old_val(step);
        break;
        case MENU_CHEAT_NEW_VAL : psp_cheat_menu_new_val(step);
        break;
        case MENU_CHEAT_POKE    : psp_cheat_menu_poke_val(step);
        break;
        case MENU_CHEAT_RAM     : psp_cheat_menu_save_ram();
        break;
        case MENU_CHEAT_ADD     : psp_cheat_menu_add_cheat();
        break;
        case MENU_CHEAT_ENABLE : if (step) psp_cheat_menu_cur_cheat(step);
                                 else      psp_cheat_menu_enable_cheat();
        break;
        case MENU_CHEAT_ALL    : psp_cheat_menu_enable_all_cheat();
        break;
        case MENU_CHEAT_VALUE  : if (step) psp_cheat_menu_cur_cheat(step);
                                 else psp_cheat_menu_value_cheat(step2);
        break;
        case MENU_CHEAT_IMPORT : if (step) psp_cheat_menu_cur_cheat(step);
                                 else      psp_cheat_menu_cheat_list();
        break;
        case MENU_CHEAT_EDIT   : psp_cheat_menu_edit_list();
        break;

        case MENU_CHEAT_DEL : if (step) psp_cheat_menu_cur_cheat(step);
                              else      psp_cheat_menu_del_cheat();
        break;
        case MENU_CHEAT_LOAD : psp_cheat_menu_load(FMGR_FORMAT_CHT);
                               old_pad = new_pad = 0;
        break;              
        case MENU_CHEAT_SAVE : psp_cheat_menu_save(FMGR_FORMAT_CHT);
                               old_pad = new_pad = 0;
        break;              
        case MENU_CHEAT_RESET : psp_cheat_menu_reset_cheat();
        break;

        case MENU_CHEAT_BACK : end_menu = 1;
        break;
      }


    } else
    if(new_pad & GP2X_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_CHEAT_ITEM-1;

    } else
    if(new_pad & GP2X_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_CHEAT_ITEM-1)) cur_menu_id++;
      else                                       cur_menu_id = 0;

    } else  
    if(new_pad & GP2X_CTRL_SQUARE) {
      /* Cancel */
      end_menu = -1;
    } else 
    if(new_pad & GP2X_CTRL_SELECT) {
      /* Back to MENU */
      end_menu = 1;
    }
  }
 
  psp_kbd_wait_no_button();

  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();
  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();

  return 1;
}

