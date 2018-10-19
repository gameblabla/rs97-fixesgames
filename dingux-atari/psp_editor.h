/*
 *  Copyright (C) 2007 Ludovic Jacomme (ludovic.jacomme@gmail.com)
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

# ifndef _PSP_EDITOR_H_
# define _PSP_EDITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct PSPWRITE_t {

    char psp_homedir[MAX_PATH];
    char edit_filename[MAX_PATH];
    int  is_modified;
    int  ask_overwrite;
    int  psp_font_size;
    int   psp_cpu_clock;
    int  dos_mode;
    int  expand_tab;
    int  tab_stop;
    int  fg_color;
    int  bg_color;
    int  screen_w;
    int  screen_h;
    int  wrap_w;
    int  wrap_mode;
    int  move_on_text;

  } PSPWRITE_t;

  extern PSPWRITE_t PSPWRITE;


#ifdef __cplusplus
}
#endif

# define COLOR_WHITE              0
# define COLOR_BLACK              1
# define COLOR_DARK_BLUE          2
# define COLOR_GREEN              3
# define COLOR_RED                4
# define COLOR_BROWN              5
# define COLOR_MAGENTA            6
# define COLOR_ORANGE             7
# define COLOR_YELLOW             8
# define COLOR_BRIGHT_GREEN       9
# define COLOR_CYAN              10
# define COLOR_BRIGHT_BLUE       11
# define COLOR_BLUE              12
# define COLOR_PINK              13
# define COLOR_GRAY              14
# define COLOR_BRIGHT_GRAY       15
# define COLOR_IMAGE             16
# define EDITOR_MAX_COLOR        17

# define EDITOR_SCREEN_MIN_WIDTH  50
# define EDITOR_SCREEN_MIN_HEIGHT 12
# define EDITOR_MAX_WRAP_WIDTH   200

  extern int editor_colors[EDITOR_MAX_COLOR];

  extern char* editor_colors_name[EDITOR_MAX_COLOR];

  extern void psp_editor_main_loop();
  extern int psp_editor_save(char *filename);
  extern int psp_editor_load(char *filename);
  extern void psp_editor_new();

  extern void psp_editor_update_column();
  extern void psp_editor_update_line();
  extern void psp_editor_menu(char* filename);

# endif
