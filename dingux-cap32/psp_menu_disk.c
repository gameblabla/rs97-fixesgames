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
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include "cap32.h"
#include "cpccat.h"
#include "psp_kbd.h"
#include "psp_sdl.h"
#include "psp_menu.h"
#include "psp_menu_disk.h"

  static int psp_menu_dirty = 1;

  static int cur_name_id  = 0;
  static int cur_name_top = 0;

# define DISK_LINE_BY_PAGE   20
# define DISK_CHAR_BY_LINE   53

static void 
psp_display_screen_disk(void)
{
  char buffer[64];
  int index   = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  //if (psp_menu_dirty) 
  {
    psp_sdl_blit_background();
  }
  
  x      = 10;
  y      =  5;
  y_step = 10;
  
  int name_line = cur_name_top;
  index = 0;

  while ((index < DISK_LINE_BY_PAGE) && (name_line < cpc_dsk_num_entry))  {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_name_id == name_line) color = PSP_MENU_SEL_COLOR;
    strcpy(buffer, cpc_dsk_dirent[name_line]);
    string_fill_with_space(buffer, 32);
    psp_sdl_back2_print(x, y, buffer, color);
    y += y_step;

    index++;
    name_line++;
  }

  if (index != DISK_LINE_BY_PAGE) {
    buffer[0]=0;
    string_fill_with_space(buffer, DISK_CHAR_BY_LINE);
    while (index < DISK_LINE_BY_PAGE) {
      psp_sdl_back2_print(x, y, buffer, PSP_MENU_SEL_COLOR);
      y += y_step;
      index++;
    }
  }
}

static void
psp_disk_run_command()
{
  char Buffer[128];
  sprintf(Buffer, "RUN\"%s", cpc_dsk_dirent[cur_name_id]);
  
  psp_kbd_run_command(Buffer, 10);
}

int
psp_disk_check_cpm()
{
  int index = 0;
  for (index = 0; index < cpc_dsk_num_entry; index++) {
    int cpos = 0;
    for (cpos = 0; cpc_dsk_dirent[index][cpos]; cpos++) {
      /* No printable chars ? might be CPM */
      if (cpc_dsk_dirent[index][cpos] < 32) return 1;
    }
  }
  return 0;
}

int
psp_disk_auto()
{
  char Buffer[128];
  int  index;
  int  found = 0;
  int  first_bas = -1;
  int  first_spc = -1;
  int  first_bin = -1;

  cur_name_id = 0;

  char *RunName = psp_run_search(CPC.cpc_save_name);

  if (RunName != (char *)0 ) {

    if (!strcasecmp(RunName, "|CPM")) strcpy(Buffer, "|CPM");
    else  snprintf(Buffer, MAX_PATH, "RUN\"%s", RunName);

  } else {

    for (index = 0; index < cpc_dsk_num_entry; index++) {
      char* scan = strchr(cpc_dsk_dirent[index], '.');
      if (scan) {
        if (! strcasecmp(scan+1, "BAS")) {
          if (first_bas == -1) first_bas = index;
          found = 1;
        } else
        if (! strcasecmp(scan+1, "")) {
          if (first_spc == -1) first_spc = index;
          found = 1;
        } else 
        if (! strcasecmp(scan+1, "BIN")) {
          if (first_bin == -1) first_bin = index;
          found = 1;
        }
      }
    }
    if (! found) {

      if (cpc_dsk_system) {
        strcpy(Buffer, "|CPM");
      } else return psp_disk_menu();

    } else {
      if (first_bas != -1) cur_name_id = first_bas;
      else 
      if (first_spc != -1) cur_name_id = first_spc;
      else 
      if (first_bin != -1) cur_name_id = first_bin;

      sprintf(Buffer, "RUN\"%s", cpc_dsk_dirent[cur_name_id]);
    }
  }
  if (CPC.psp_explore_disk == CPC_EXPLORE_FULL_AUTO) {
    strcat(Buffer, "\n");
  }
  psp_kbd_run_command(Buffer, 12);

  return 1;
}

int 
psp_disk_menu()
{
  gp2xCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  psp_kbd_wait_no_button();

  if ( cpc_dsk_num_entry <= 0) {
    return 0;
  }

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  cur_name_id  = 0;
  cur_name_top = 0;

  psp_menu_dirty = 1;

  while (! end_menu)
  {
    psp_display_screen_disk();
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

    if ((c.Buttons & (GP2X_CTRL_LTRIGGER|GP2X_CTRL_RTRIGGER|GP2X_CTRL_START)) ==
        (GP2X_CTRL_LTRIGGER|GP2X_CTRL_RTRIGGER|GP2X_CTRL_START)) {
      /* Exit ! */
      psp_sdl_exit(0);
    } else
    if(new_pad & GP2X_CTRL_SELECT) {
      /* Back to Main menu */
      end_menu = 1;
    } else
    if(new_pad & GP2X_CTRL_UP) {

      if (cur_name_id > 0) {
        cur_name_id--;
        if (cur_name_id < cur_name_top) cur_name_top = cur_name_id;
      }

    } else
    if(new_pad & GP2X_CTRL_DOWN) {

      if (cur_name_id < (cpc_dsk_num_entry-1)) {
        cur_name_id++;
        if (cur_name_id > (cur_name_top + DISK_LINE_BY_PAGE - 1)) {
          cur_name_top = cur_name_id - DISK_LINE_BY_PAGE + 1;
        }
      }

    } else  
    if(new_pad & GP2X_CTRL_SQUARE) {
      /* Cancel */
      end_menu = -1;
    } else 
    if(new_pad & GP2X_CTRL_CROSS) {
      /* RUN" */
      psp_disk_run_command();
      end_menu = 2;
    } else 
    if((new_pad & GP2X_CTRL_SELECT) == GP2X_CTRL_SELECT) {
      /* Back to CPC */
      end_menu = 1;
    }
  }
 
  psp_kbd_wait_no_button();

  return (end_menu == 2);
}

