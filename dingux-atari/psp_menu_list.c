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
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include "SDL.h"

#include "global.h"
#include "psp_kbd.h"
#include "psp_sdl.h"
#include "psp_menu.h"
#include "psp_menu_cheat.h"
#include "psp_menu_list.h"

# define MAX_CHEAT_LINE    8192

# define CHEAT_LINE_BY_PAGE   22
# define CHEAT_CHAR_BY_LINE   52

  static char* psp_cheat[MAX_CHEAT_LINE];
  static int   psp_cheat_size    = -1;
  static int   psp_cheat_current = 0;
  static int   psp_cheat_top     = 0;

static void
psp_initialize_cheat_list(void)
{
  char  FileName[MAX_PATH+1];

  char  Buffer[512];
  char *Scan;
  FILE* FileDesc;

  /* Already done ? */
  if (psp_cheat_size > 0) return; 

  snprintf(FileName, MAX_PATH, "%s/cheat.txt", ATARI.atari_home_dir);
  FileDesc = fopen(FileName, "r");

  psp_cheat_current = 0;
  psp_cheat_top     = 0;

  if (FileDesc == (FILE *)0 ) {
    psp_cheat[0] = strdup( "no cheat file found !");
    psp_cheat_size = 1;
    return;
  }

  psp_cheat_size = 0;
  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';

    int index;
    int blank = 1;
    Scan = Buffer;
    for (index = 0; Scan[index]; index++) {
      if (Scan[index] != ' ') { blank = 0; break; }
    }
    if (blank) continue;

    psp_cheat[psp_cheat_size++] = strdup(Buffer);
    if (psp_cheat_size >= MAX_CHEAT_LINE) break;
  }
  fclose(FileDesc);
}

static void
psp_main_menu_cheat_applied()
{
  psp_display_screen_menu();
  psp_sdl_back2_print(110, 160, "Cheat applied !", PSP_MENU_NOTE_COLOR);
  psp_sdl_flip();
  sleep(1);
}


static void 
psp_display_screen_cheat_list(void)
{
  char buffer[512];
  int color   = 0;

  int cheat_line = 0;
  int index     = 0;

  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  
  psp_sdl_blit_help();

  x      = 0;
  y      = 5;
  y_step = 10;

  cheat_line = psp_cheat_top;
  index     = 0;
  
  while ((index < CHEAT_LINE_BY_PAGE) && (cheat_line < psp_cheat_size))  {
    strcpy(buffer, psp_cheat[cheat_line]);
    string_fill_with_space(buffer, CHEAT_CHAR_BY_LINE);
    if (cheat_line == psp_cheat_current) color = PSP_MENU_SEL2_COLOR;
    else                            color = PSP_MENU_TEXT_COLOR;
    psp_sdl_back2_print(x, y, buffer, color);
    y += y_step;
    index++;
    cheat_line++;
  }

  if (index != CHEAT_LINE_BY_PAGE) {
    buffer[0]=0;
    string_fill_with_space(buffer, CHEAT_CHAR_BY_LINE);
    while (index < CHEAT_LINE_BY_PAGE) {
      if (cheat_line == psp_cheat_current) color = PSP_MENU_SEL2_COLOR;
      else                            color = PSP_MENU_TEXT_COLOR;
      psp_sdl_back2_print(x, y, buffer, color);
      y += y_step;
      index++;
    }
  }
}

static int
psp_apply_current_cheat_list(int cheat_id)
{
  Atari_cheat_t* a_cheat;
  char*         scan_cheat;
  int           addr;
  int           value;

  if ((psp_cheat_current >= psp_cheat_size) || (! psp_cheat[psp_cheat_current])) return 0;

  a_cheat = &ATARI.atari_cheat[cheat_id];

  int num_field = sscanf(psp_cheat[psp_cheat_current], "%x-%x", &addr, &value);
  if (num_field != 2) return 0;
  addr = addr % ATARI_RAM_SIZE;

  a_cheat->comment[0] = 0;
  a_cheat->type  = ATARI_CHEAT_ENABLE;
  a_cheat->addr  = addr;
  a_cheat->value = value;

  scan_cheat = strchr(psp_cheat[psp_cheat_current], '-');
  if (scan_cheat) {
    scan_cheat++;
    scan_cheat = strchr(scan_cheat, ' ');
    if (scan_cheat) {
      while (*scan_cheat == ' ') scan_cheat++;
      strncpy(a_cheat->comment, scan_cheat, sizeof(a_cheat->comment));
    }
  }
  return 1;
}

int 
psp_menu_cheat_list(int cheat_id)
{
  gp2xCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  psp_kbd_wait_no_button();

  psp_initialize_cheat_list();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  while (! end_menu)
  {
    psp_display_screen_cheat_list();
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
      if (psp_cheat_current > 0) psp_cheat_current--;
    } else
    if(new_pad & GP2X_CTRL_DOWN) {
      if ((psp_cheat_current + 1) < psp_cheat_size) psp_cheat_current++;
    } else
    if(new_pad == GP2X_CTRL_LEFT) {
      if (psp_cheat_current > CHEAT_LINE_BY_PAGE) psp_cheat_current -= CHEAT_LINE_BY_PAGE;
      else                                        psp_cheat_current  = 0;
    } else
    if(new_pad == GP2X_CTRL_RIGHT) {
      if ((psp_cheat_current + CHEAT_LINE_BY_PAGE + 1) < psp_cheat_size) {
         psp_cheat_current += CHEAT_LINE_BY_PAGE;
      } else
      if (psp_cheat_size > 0) {
         psp_cheat_current = psp_cheat_size - 1;
      }

    } else
    if ((new_pad & GP2X_CTRL_CROSS ) || 
        (new_pad & GP2X_CTRL_CIRCLE)) {
      if (psp_apply_current_cheat_list(cheat_id)) {
        end_menu = 2;
      }
    }
    if (psp_cheat_current < psp_cheat_top) psp_cheat_top = psp_cheat_current;
    if (psp_cheat_current >= (psp_cheat_top + CHEAT_LINE_BY_PAGE)) {
      psp_cheat_top = psp_cheat_current - CHEAT_LINE_BY_PAGE + 1;
    }
  }
 
  psp_kbd_wait_no_button();

  return end_menu == 2;
}

