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

#include "SDL.h"

#include "global.h"
#include "psp_kbd.h"
#include "psp_sdl.h"
#include "psp_menu.h"
#include "psp_menu_help.h"

# define MAX_HELP_LINE    4096 

# define HELP_LINE_BY_PAGE   24
# define HELP_CHAR_BY_LINE   53

  static char* psp_help[MAX_HELP_LINE];
  static int   psp_help_size    = -1;
  static int   psp_help_current = 0;

static void
psp_initialize_help(void)
{
  char  FileName[MAX_PATH+1];

  char  Buffer[512];
  char *Scan;
  FILE* FileDesc;

  /* Already done ? */
  if (psp_help_size > 0) return; 

  strcpy(FileName, "./help.txt");
  FileDesc = fopen(FileName, "r");

  psp_help_current = 0;

  if (FileDesc == (FILE *)0 ) {
    psp_help[0] = strdup( "no help file found !");
    psp_help_size = 1;
    return;
  }

  psp_help_size = 0;
  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';

    psp_help[psp_help_size++] = strdup(Buffer);
    if (psp_help_size >= MAX_HELP_LINE) break;
  }
  fclose(FileDesc);
}

static void 
psp_display_screen_help(void)
{
  char buffer[512];

  int help_line = 0;
  int index     = 0;

  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  psp_sdl_blit_help();

  x      = 0;
  y      = 0;
  y_step = 10;

  help_line = psp_help_current;
  index     = 0;
  
  while ((index < HELP_LINE_BY_PAGE) && (help_line < psp_help_size))  {
    strcpy(buffer, psp_help[help_line]);
    string_fill_with_space(buffer, HELP_CHAR_BY_LINE);
    psp_sdl_back2_print(x, y, buffer, PSP_MENU_SEL_COLOR);
    y += y_step;
    index++;
    help_line++;
  }

  if (index != HELP_LINE_BY_PAGE) {
    buffer[0]=0;
    string_fill_with_space(buffer, HELP_CHAR_BY_LINE);
    while (index < HELP_LINE_BY_PAGE) {
      psp_sdl_back2_print(x, y, buffer, PSP_MENU_SEL_COLOR);
      y += y_step;
      index++;
    }
  }
}

int 
psp_help_menu()
{
  gp2xCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  psp_kbd_wait_no_button();

  psp_initialize_help();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  while (! end_menu)
  {
    psp_display_screen_help();
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

    if(new_pad & GP2X_CTRL_SELECT) {
      /* Back to Main menu */
      end_menu = 1;
    } else
    if(new_pad & GP2X_CTRL_UP) {
      if (psp_help_current > 0) psp_help_current--;
    } else
    if(new_pad & GP2X_CTRL_DOWN) {
      if ((psp_help_current + 1) < psp_help_size) psp_help_current++;
    } else
    if(new_pad == GP2X_CTRL_LEFT) {
      if (psp_help_current > HELP_LINE_BY_PAGE) psp_help_current -= HELP_LINE_BY_PAGE;
      else                                      psp_help_current  = 0;
    } else
    if(new_pad == GP2X_CTRL_RIGHT) {
      if ((psp_help_current + HELP_LINE_BY_PAGE + 1) < psp_help_size) psp_help_current += HELP_LINE_BY_PAGE;
    }
  }
 
  psp_kbd_wait_no_button();

  return 1;
}

