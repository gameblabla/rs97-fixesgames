/*
   Copyright (C) 2008 Ludovic Jacomme (ludovic.jacomme@gmail.com)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Based on the source code of PelDet written by Danzel A.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <SDL.h>

#include "global.h"
#include "psp_sdl.h"
#include "psp_danzeff.h"
#include "psp_menu.h"
#include "psp_fmgr.h"
#include "psp_kbd.h"
#include "psp_editor.h"

# define EDITOR_MAX_LINES     50000
# define EDITOR_MAX_WIDTH      2048

# define psp_font_height   10
# define psp_font_width     6

/* NEOL : No end of line */
# define EDITOR_NEOL_MASK        0x01

# define psp_editor_is_neol(L)  ((L)->flags & EDITOR_NEOL_MASK)
# define psp_editor_set_neol(L) ((L)->flags |= EDITOR_NEOL_MASK)
# define psp_editor_clear_neol(L) ((L)->flags &= ~EDITOR_NEOL_MASK)

  typedef struct line_t {
    char* text;
    u16   width;
    u16   max_width;
    u8    flags;
  } line_t;

  typedef struct text_t {
    line_t*   lines[EDITOR_MAX_LINES];

    char*     wrap_buffer;
    int       wrap_buffer_size;

    int       max_width;
    int       max_lines;

    int       num_lines;
    int       top_line;
    int       left_col;
    int       curr_line;
    int       curr_col;

    u32       last_blink;
    char      cursor_blink;
    char      cursor_moving;
    char      cursor_end;
    int       last_col;

    int       sel_from_col;
    int       sel_from_line;
    int       sel_to_col;
    int       sel_to_line;
    int       sel_mode;

  } text_t;

  typedef struct line_list_t {

    struct line_list_t* next;
    line_t*             line;

  } line_list_t;

  typedef struct clip_t {
    line_list_t*  head_list;
    line_list_t*  last_list;
    int           number_lines;
  } clip_t;

  static text_t* g_text;
  static clip_t* g_clip;

  int editor_colors[EDITOR_MAX_COLOR];

  char* editor_colors_name[EDITOR_MAX_COLOR] = {
     "white" ,
     "black" ,
     "dark blue" ,
     "green" ,
     "red" ,
     "brown" ,
     "magenta" ,
     "orange" ,
     "yellow" ,
     "bright green" ,
     "cyan" ,
     "bright blue" ,
     "blue" ,
     "pink" ,
     "gray" ,
     "bright gray",
     "image"
  };

 extern void psp_editor_cut();
 extern int  psp_editor_rewrap_buffer_line(int from_line_id, int abs_curr_pos);
 extern void psp_editor_del_line_from(int line_id, int num_lines);

  PSPWRITE_t PSPWRITE;

static inline int
my_isspace(char c)
{
  return ((c == ' ') || (c == '\t') || (c == '\f') || (c == '\v'));
}

void
psp_editor_default()
{
  PSPWRITE.dos_mode   = 1;
  PSPWRITE.move_on_text = 1;
  PSPWRITE.expand_tab = 0;
  PSPWRITE.tab_stop = 4;
  PSPWRITE.psp_cpu_clock = 133;

  PSPWRITE.fg_color = COLOR_CYAN;
  PSPWRITE.bg_color = COLOR_IMAGE;

  PSPWRITE.psp_font_size = 0;
  PSPWRITE.screen_w  = PSP_SDL_SCREEN_WIDTH  / psp_font_width;
  PSPWRITE.screen_h  = PSP_SDL_SCREEN_HEIGHT / psp_font_height;
  PSPWRITE.wrap_w    = PSPWRITE.screen_w;
  PSPWRITE.wrap_mode = 1;
}

line_t*
psp_editor_alloc_line()
{
  line_t* a_line = (line_t*)malloc( sizeof(line_t) );
  memset(a_line, 0, sizeof( line_t ));
  return a_line;
}

line_t*
psp_editor_add_line(int line_id, char *buffer)
{
  if (line_id >= g_text->max_lines) return 0;

  line_t* a_line = g_text->lines[line_id];
  int length = strlen( buffer );

  if (! a_line) {
    a_line = psp_editor_alloc_line();
    g_text->lines[line_id] = a_line;
  }

  if (length < a_line->max_width) {
    strcpy(a_line->text, buffer);
    a_line->width = length;
    a_line->text[a_line->width] = 0;
  } else {
    if (a_line->text) free(a_line->text);
    a_line->width     = length;
    a_line->max_width = length;
    a_line->text = strdup( buffer );
    a_line->text[a_line->width] = 0;
  }

  if (line_id >= g_text->num_lines) {
    g_text->num_lines = line_id + 1;
  }
  PSPWRITE.is_modified = 1;
  return a_line;
}

line_t*
psp_editor_insert_line(int line_id, char* text)
{
  if (g_text->num_lines >= g_text->max_lines) return 0;

  PSPWRITE.is_modified = 1;

  int delta = g_text->num_lines - line_id;
  if (delta > 0) {
    int index = 0;
    for (index = g_text->num_lines - 1; index >= line_id; index--) {
      g_text->lines[index + 1] = g_text->lines[index];
    }
    g_text->lines[line_id] = 0;
  }
  line_t* a_line = psp_editor_add_line(line_id, text);
  g_text->num_lines++;

  psp_editor_update_column(1);
  psp_editor_update_line();

  return a_line;
}


int
psp_editor_rewrap_buffer_line(int from_line_id, int abs_curr_pos)
{
  char* begin_wrap  = g_text->wrap_buffer;
  int   length_left = strlen(begin_wrap);
  int   length_init = length_left;

  line_t* a_last_line = 0;
  int a_line_id = from_line_id;
  while (length_left > 0) {

    if (length_left > PSPWRITE.wrap_w) {
      /* find first space starting from wrap_w */
      int first_space = PSPWRITE.wrap_w;
      while (first_space > 0) {
        if (my_isspace(begin_wrap[first_space])) break;
        first_space--;
      }
      /* no space found */
      if (! first_space) {
        first_space = PSPWRITE.wrap_w;
      }

      char save_char = begin_wrap[first_space];
      begin_wrap[first_space] = 0;
      a_last_line = psp_editor_insert_line(a_line_id, begin_wrap);
      if (a_last_line) psp_editor_set_neol(a_last_line);
      begin_wrap[first_space] = save_char;

      if (abs_curr_pos >= 0) {
        /* if abs_curr_pos is between abs_begin and abs_end */
        int abs_begin = length_init - length_left;
        int abs_end   = abs_begin + first_space;
        if ((abs_curr_pos >= abs_begin) && (abs_curr_pos <= abs_end)) {
          /* found it ! */
          if (a_last_line) {
            g_text->curr_line = a_line_id;
            g_text->curr_col  = abs_curr_pos - abs_begin;
            if (g_text->curr_col > a_last_line->width) {
              g_text->curr_col = a_last_line->width;
            }
          } else {
            g_text->curr_line = a_line_id;
            g_text->curr_col  = 0;
          }
        }
      }

      length_left -= first_space;
      begin_wrap += first_space;

      while ((length_left > 0) && my_isspace(begin_wrap[0])) {
        begin_wrap++;
        length_left--;
      }

    } else {
      a_last_line = psp_editor_insert_line(a_line_id, begin_wrap);
      if (a_last_line) psp_editor_set_neol(a_last_line);

      if (abs_curr_pos >= 0) {
        /* if abs_curr_pos is between abs_begin and abs_end */
        int abs_begin = length_init - length_left;
        int abs_end   = length_init;
        if ((abs_curr_pos >= abs_begin) && (abs_curr_pos <= abs_end)) {
          /* found it ! */
          if (a_last_line) {
            g_text->curr_line = a_line_id;
            g_text->curr_col  = abs_curr_pos - abs_begin;
            if (g_text->curr_col > a_last_line->width) {
              g_text->curr_col = a_last_line->width;
            }
          } else {
            g_text->curr_line = a_line_id;
            g_text->curr_col  = 0;
          }
        }
      }
      length_left = 0;
    }
    a_line_id++;
  }
  if (a_last_line) psp_editor_clear_neol(a_last_line);

  return a_line_id;
}

void
psp_editor_rewrap_line(int rewrap_line_id)
{
  int from_line_id = rewrap_line_id;
  line_t* a_line_from = g_text->lines[from_line_id];
  if (! a_line_from) return;

  int a_line_id = rewrap_line_id;
  int to_line_id = rewrap_line_id;

  if (psp_editor_is_neol(a_line_from)) {

    /* scan for first neol line */
    while (a_line_from && psp_editor_is_neol(a_line_from)) {
      from_line_id = a_line_id;
      if (a_line_id > 0) a_line_id--;
      else break;
      a_line_from = g_text->lines[a_line_id];
    }

    /* scan for last neol line */
    a_line_id = to_line_id;
    line_t* a_line_to = g_text->lines[to_line_id];

    while (a_line_to && psp_editor_is_neol(a_line_to)) {
      to_line_id = a_line_id;
      if (a_line_id < g_text->num_lines) a_line_id++;
      else break;
      a_line_to = g_text->lines[a_line_id];
    }

    if (a_line_to && !psp_editor_is_neol(a_line_to)) {
      to_line_id = a_line_id;
    }

  } else {

    /* scan for first neol line */
    a_line_id = from_line_id;
    if (a_line_id > 0) a_line_id--;
    a_line_from = g_text->lines[a_line_id];

    while (a_line_from && psp_editor_is_neol(a_line_from)) {
      from_line_id = a_line_id;
      if (a_line_id > 0) a_line_id--;
      else break;
      a_line_from = g_text->lines[a_line_id];
    }
  }

  int abs_curr_pos = -1;
  int length = 0;
  for (a_line_id = from_line_id; a_line_id <= to_line_id; a_line_id++) {
    line_t* a_line = g_text->lines[a_line_id];
    if (a_line_id == g_text->curr_line) {
      abs_curr_pos = length;
      if (a_line) {
        if (g_text->curr_col > a_line->width) abs_curr_pos += a_line->width;
        else                                  abs_curr_pos += g_text->curr_col;
      }
    }
    if (a_line) length += a_line->width + 1;
  }
  if (! length) return;

  if (g_text->wrap_buffer_size < length) {
    g_text->wrap_buffer = realloc(g_text->wrap_buffer, length + 1);
    g_text->wrap_buffer_size = length;
  }

  char* scan_buffer = g_text->wrap_buffer;
  scan_buffer[0] = 0;
  for (a_line_id = from_line_id; a_line_id <= to_line_id; a_line_id++) {
    line_t* a_line = g_text->lines[a_line_id];
    if (a_line && a_line->width) {
      strcpy(scan_buffer, a_line->text);
      scan_buffer += a_line->width;
      if (a_line_id < to_line_id) {
        strcpy(scan_buffer, " ");
        scan_buffer++;
      }
    }
  }
  /* delete lines between line_from and line_to */
  int num_lines = 1 + to_line_id - from_line_id;
  psp_editor_del_line_from(from_line_id, num_lines);

  /* rewrap all deleted lines */
  psp_editor_rewrap_buffer_line(from_line_id, abs_curr_pos);
}

void
psp_editor_rewrap_curr_line()
{
  psp_editor_rewrap_line(g_text->curr_line);
  psp_editor_update_column(1);
  psp_editor_update_line();
  g_text->sel_mode = 0;
}

void
psp_editor_resize_line(line_t* a_line, int new_width)
{
  PSPWRITE.is_modified = 1;
  if (new_width > g_text->max_width) new_width = g_text->max_width;
  if (new_width > a_line->max_width) {
    a_line->text = realloc(a_line->text, new_width + 1);
    a_line->max_width = new_width;
  }
}

void
psp_editor_reset_text()
{
  int line_id;
  for (line_id = 0; line_id < g_text->num_lines; line_id++) {
    line_t* a_line = g_text->lines[line_id];
    if (a_line) {
      if (a_line->text) {
        free(a_line->text);
      }
      memset(a_line, 0, sizeof(line_t));
    }
  }
  g_text->num_lines = 0;
  g_text->top_line = 0;
  g_text->left_col = 0;
  g_text->curr_line = 0;
  g_text->curr_col = 0;
  g_text->cursor_moving = 0;
  g_text->cursor_blink = 0;
  g_text->cursor_end = 0;
  g_text->last_col = 0;

  g_text->sel_mode      = 0;
  g_text->sel_from_line = 0;
  g_text->sel_to_line   = 0;
  g_text->sel_from_col  = 0;
  g_text->sel_to_col    = 0;

  psp_editor_add_line(0, "");
}

void
psp_editor_update_column(int update_last)
{
  if (g_text->curr_col >= g_text->max_width) {
    g_text->curr_col = g_text->max_width - 1;
  }
  if ((g_text->curr_col - g_text->left_col) >= PSPWRITE.screen_w) {
    g_text->left_col = g_text->curr_col - PSPWRITE.screen_w + 1;
  }
  if (g_text->curr_col < 0) {
    g_text->curr_col = 0;
  }
  if (g_text->curr_col < g_text->left_col) {
    g_text->left_col = g_text->curr_col - PSPWRITE.screen_w / 3;
    if (g_text->left_col < 0) g_text->left_col = 0;
  }

  line_t* a_line = g_text->lines[g_text->curr_line];
  if (a_line && a_line->width) {
    if (a_line->width != g_text->curr_col) g_text->cursor_end = 0;
    if (! g_text->cursor_end) {
      if (update_last) {
        g_text->last_col = g_text->curr_col;
      }
    }
  }
}

void
psp_editor_sel_mode()
{
  g_text->sel_mode = ! g_text->sel_mode;
  if (g_text->sel_mode) {
    g_text->sel_from_line = g_text->curr_line;
    g_text->sel_to_line   = g_text->curr_line;
    g_text->sel_from_col  = g_text->curr_col;
    g_text->sel_to_col    = g_text->curr_col;
  }
  psp_kbd_wait_no_button();
}

void
psp_editor_update_sel()
{
  if (g_text->sel_mode) {
    if (g_text->curr_line < g_text->sel_from_line) {
      g_text->sel_from_line = g_text->curr_line;
      g_text->sel_from_col  = g_text->curr_col;
    } else
    if (g_text->curr_line > g_text->sel_to_line) {
      g_text->sel_to_line = g_text->curr_line;
      g_text->sel_to_col  = g_text->curr_col;
    } else {

      if (g_text->curr_line == g_text->sel_from_line) {
        if (g_text->curr_col < g_text->sel_from_col) g_text->sel_from_col = g_text->curr_col;
      }
      if (g_text->curr_line == g_text->sel_to_line) {
        if (g_text->curr_col > g_text->sel_to_col) g_text->sel_to_col = g_text->curr_col;
      }
      if ((g_text->curr_line == (g_text->sel_to_line-1)) &&
          (g_text->curr_line >  (g_text->sel_from_line))) {
        g_text->sel_to_line = g_text->curr_line;
        g_text->sel_to_col  = g_text->curr_col;
      } else
      if ((g_text->curr_line == (g_text->sel_from_line+1)) &&
          (g_text->curr_line <  (g_text->sel_to_line))) {
        g_text->sel_from_line = g_text->curr_line;
        g_text->sel_from_col  = g_text->curr_col;
      }
    }
  }
}

void
psp_editor_update_line()
{
  if (g_text->curr_line >= g_text->num_lines) {
    g_text->curr_line = g_text->num_lines - 1;
  }
  if ((g_text->curr_line - g_text->top_line) >= PSPWRITE.screen_h) {
    g_text->top_line = g_text->curr_line - PSPWRITE.screen_h + 1;
  }
  if (g_text->curr_line < 0) {
    g_text->curr_line = 0;
  }
  if (g_text->curr_line < g_text->top_line) {
    g_text->top_line = g_text->curr_line;
  }
}


void
psp_editor_goto_line_down()
{
  g_text->cursor_moving = 1;

  if (PSPWRITE.move_on_text) {

    if ((g_text->curr_line + 1) < g_text->num_lines) {
      g_text->curr_line++;
      line_t* a_line_next = g_text->lines[g_text->curr_line];
      if (! a_line_next) {
        g_text->curr_col = 0;
      } else
      if (g_text->cursor_end || (g_text->curr_col > a_line_next->width)) {
        g_text->curr_col = a_line_next->width;
        if ((g_text->last_col) && (g_text->curr_col > g_text->last_col)) {
          g_text->curr_col = g_text->last_col;
        }
      } else {
        if (g_text->last_col <= a_line_next->width) {
          g_text->curr_col = g_text->last_col;
        } else {
          g_text->curr_col = a_line_next->width;
        }
      }
    }

  } else {
    g_text->curr_line++;
  }
  psp_editor_update_column(0);
  psp_editor_update_line();
  psp_editor_update_sel();
}

void
psp_editor_goto_line_up()
{
  g_text->cursor_moving = 1;

  if (PSPWRITE.move_on_text) {

    if (g_text->curr_line > 0) {
      g_text->curr_line--;
      line_t* a_line_prev = g_text->lines[g_text->curr_line];
      if (! a_line_prev) {
        g_text->curr_col = 0;
      } else
      if (g_text->cursor_end || (g_text->curr_col > a_line_prev->width)) {
        g_text->curr_col = a_line_prev->width;
        if ((g_text->last_col) && (g_text->curr_col > g_text->last_col)) {
          g_text->curr_col = g_text->last_col;
        }
      } else {
        if (g_text->last_col <= a_line_prev->width) {
          g_text->curr_col = g_text->last_col;
        }
      }
    }
  } else {
    g_text->curr_line--;
  }
  psp_editor_update_column(0);
  psp_editor_update_line();
  psp_editor_update_sel();
}

void
psp_editor_goto_col_left()
{
  g_text->cursor_moving = 1;

  if (PSPWRITE.move_on_text) {

    line_t* a_line = g_text->lines[g_text->curr_line];
    if (! a_line) {
      g_text->curr_col = 0;
    } else {
      if ((g_text->curr_col == 0) && (g_text->curr_line > 0)) {
        g_text->curr_line--;
        line_t* a_line_prev = g_text->lines[g_text->curr_line];
        if (a_line_prev) {
          g_text->curr_col = a_line_prev->width;
          g_text->last_col = g_text->max_width - 1;
        } else {
          g_text->curr_col = 0;
        }
      } else {
        g_text->curr_col--;
      }
    }

  } else {
    g_text->curr_col--;
  }
  psp_editor_update_line();
  psp_editor_update_sel();
  psp_editor_update_column(1);
}

void
psp_editor_goto_col_right()
{
  g_text->cursor_moving = 1;

  if (PSPWRITE.move_on_text) {

    line_t* a_line = g_text->lines[g_text->curr_line];
    if (! a_line) {
      if ((g_text->curr_line + 1) < g_text->num_lines) {
        g_text->curr_line++;
        g_text->curr_col = 0;
      }
    } else {
      if ( (g_text->curr_col == a_line->width          ) &&
           ((g_text->curr_line + 1) < g_text->num_lines) ) {
        g_text->curr_line++;
        g_text->curr_col = 0;
      } else {
        g_text->curr_col++;
      }
    }

  } else {
    g_text->curr_col++;
  }
  psp_editor_update_line();
  psp_editor_update_sel();
  psp_editor_update_column(1);
}

void
psp_editor_goto_col_begin()
{
  g_text->cursor_moving = 1;
  g_text->curr_col = 0;
  psp_editor_update_column(1);
}

void
psp_editor_goto_col_end()
{
  g_text->cursor_moving = 1;
  line_t* a_line = g_text->lines[g_text->curr_line];
  if (a_line) {
    g_text->curr_col = a_line->width;
    g_text->last_col = g_text->max_width - 1;
    g_text->cursor_end = 1;
  } else {
    g_text->curr_col = 0;
  }
  psp_editor_update_column(1);
  psp_editor_update_sel();
}

void
psp_editor_goto_first_line()
{
  g_text->cursor_moving = 1;
  g_text->curr_col = 0;
  g_text->curr_line = 0;
  psp_editor_update_column(0);
  psp_editor_update_line();
  psp_editor_update_sel();
}

void
psp_editor_goto_last_line()
{
  g_text->cursor_moving = 1;
  g_text->curr_col = 0;
  g_text->curr_line = g_text->num_lines;
  psp_editor_update_column(0);
  psp_editor_update_line();
  psp_editor_update_sel();
}

void
psp_editor_goto_page_down()
{
  g_text->cursor_moving = 1;
  g_text->curr_line += PSPWRITE.screen_h;
  psp_editor_update_line();
  psp_editor_update_sel();
}

void
psp_editor_goto_page_up()
{
  g_text->cursor_moving = 1;
  g_text->curr_line -= PSPWRITE.screen_h;
  psp_editor_update_line();
  psp_editor_update_sel();
}

void
psp_editor_goto_word_left()
{
  g_text->cursor_moving = 1;
  line_t* a_line = g_text->lines[g_text->curr_line];
  if (a_line && a_line->text) {
    int index = g_text->curr_col;
    if (index > 0) index--;
    while ((index > 0) && (a_line->text[index] == ' ')) index--;
    while ((index > 0) && (a_line->text[index] != ' ')) index--;
    if (a_line->text[index] == ' ') index++;
    g_text->curr_col = index;

  } else g_text->curr_col = 0;
  psp_editor_update_column(1);
  psp_editor_update_sel();
}

void
psp_editor_goto_word_right()
{
  g_text->cursor_moving = 1;
  line_t* a_line = g_text->lines[g_text->curr_line];
  if (a_line && a_line->text) {
    int index = g_text->curr_col;
    while ((index < a_line->width) && (a_line->text[index] != ' ')) index++;
    while ((index < a_line->width) && (a_line->text[index] == ' ')) index++;
    g_text->curr_col = index;

  } else g_text->curr_col = 0;
  psp_editor_update_column(1);
  psp_editor_update_sel();
}

void
psp_editor_insert_char(int line_id, int col_id, int c)
{
  PSPWRITE.is_modified = 1;

  uchar c1 = (c >> 8) & 0xff;
  uchar c2 = c & 0xff;
  if ((c1 == 0xc2) || (c1 == 0xc3)) {
    uchar new_c = psp_convert_utf8_to_iso_8859_1(c1, c2);
    if (new_c) { c = new_c; }
  }

  line_t* a_line = g_text->lines[line_id];
  if (! a_line) {
    a_line = psp_editor_add_line(line_id, "");
  }
  if (col_id >= a_line->width) {
    psp_editor_resize_line(a_line, 1 + ((col_id * 3) / 2) );
    while (a_line->width < col_id) {
      a_line->text[a_line->width++] = ' ';
    }
    a_line->text[col_id] = c;
    a_line->width++;
    a_line->text[a_line->width] = 0;
  } else {
    if ((a_line->width + 1) >= a_line->max_width) {
      psp_editor_resize_line(a_line, 1 + ((a_line->width * 3) / 2) );
    }
    a_line->width++;
    int index = a_line->width;
    while (index > col_id) {
      a_line->text[index] = a_line->text[index - 1];
      index--;
    }
    a_line->text[col_id] = c;
    a_line->text[a_line->width] = 0;
  }
  g_text->sel_mode = 0;
}

void
psp_editor_rewrap_if_needed()
{
  if (PSPWRITE.wrap_mode) {
    psp_editor_rewrap_line(g_text->curr_line);
    psp_editor_update_column(1);
    psp_editor_update_line();
    g_text->sel_mode = 0;
  }
}

void
psp_editor_insert_curr_char(int c)
{
  if ((c == '\t') && PSPWRITE.expand_tab) {
    int tab;
    for (tab = 0; tab < PSPWRITE.tab_stop; tab++) {
      psp_editor_insert_curr_char(' ');
    }
  } else {
    psp_editor_insert_char(g_text->curr_line, g_text->curr_col, c);
    g_text->curr_col++;
  }
  psp_editor_rewrap_if_needed();
  psp_editor_update_column(1);
  g_text->sel_mode = 0;
}

void
psp_editor_split_line(int line_id, int col_id)
{
  if (g_text->num_lines >= g_text->max_lines) return;

  PSPWRITE.is_modified = 1;

  line_t* a_line = g_text->lines[line_id];
  if (! a_line) {
    a_line = psp_editor_add_line(line_id, "");
  }
  int n_line_id = line_id + 1;
  int delta = g_text->num_lines - line_id - 1;
  if (delta > 0) {
    int index = 0;
    for (index = g_text->num_lines - 1; index > line_id; index--) {
      g_text->lines[index + 1] = g_text->lines[index];
    }
    g_text->lines[n_line_id] = 0;
  }
  line_t* n_line = psp_editor_add_line(n_line_id, "");

  if (col_id >= a_line->width) {
    /* nothing more to do */
  } else {
    int delta = a_line->width - col_id;
    psp_editor_resize_line(n_line, 1 + ((delta * 3) / 2));
    int index = col_id;
    while (index < a_line->width) {
      n_line->text[n_line->width++] = a_line->text[index];
      index++;
    }
    n_line->text[n_line->width] = 0;
    a_line->width = col_id;
  }
  g_text->num_lines++;
}

void
psp_editor_split_curr_line()
{
  psp_editor_split_line(g_text->curr_line, g_text->curr_col);
  g_text->curr_col = 0;
  g_text->curr_line++;
  psp_editor_rewrap_if_needed();
  psp_editor_update_column(1);
  psp_editor_update_line();
  psp_editor_update_sel();
  g_text->sel_mode = 0;
}

void
psp_editor_delete_char(int col_id)
{
  PSPWRITE.is_modified = 1;

  line_t* a_line = g_text->lines[g_text->curr_line];
  if (! a_line) {
    a_line = psp_editor_add_line(g_text->curr_line, "");
  }
  if (col_id > a_line->width) {
    /* Nothing to do */
  } else
  if (col_id > 0) {
    int delta = a_line->width - col_id;
    if (delta > 0) {
      char* scan_text = &a_line->text[col_id];
      memcpy( scan_text - 1, scan_text, delta);
    }
    a_line->width--;
    a_line->text[a_line->width] = 0;
  }
}

void
psp_editor_suppr_char(int col_id)
{
  PSPWRITE.is_modified = 1;

  line_t* a_line = g_text->lines[g_text->curr_line];
  if (! a_line) {
    a_line = psp_editor_add_line(g_text->curr_line, "");
  }
  if (col_id > a_line->width) {
    /* Nothing to do */
  } else {
    int delta = a_line->width - col_id;
    if (delta > 0) {
      char* scan_text = &a_line->text[col_id];
      memcpy( scan_text, scan_text + 1, delta);
    }
    a_line->width--;
    a_line->text[a_line->width] = 0;
  }
  g_text->sel_mode = 0;
}

void
psp_editor_delete_line(int line_id)
{
  PSPWRITE.is_modified = 1;

  line_t* a_line = g_text->lines[line_id];
  if (a_line) {
    if (a_line->text) free(a_line->text);
    a_line->text = 0;
    free(a_line);
    g_text->lines[line_id] = 0;
    a_line = 0;
  }
  int delta = g_text->num_lines - line_id - 1;
  if (delta > 0) {
    memcpy(&g_text->lines[line_id], &g_text->lines[line_id +1], sizeof(line_t *) * delta);
    g_text->lines[g_text->num_lines - 1] = 0;
  }
  g_text->num_lines--;
  if (g_text->num_lines <= 0) {
    psp_editor_add_line(0, "");
  }
  g_text->sel_mode = 0;
}

void
psp_editor_join_prev_line(int a_line_id)
{
  /* Join with previous line */

  int p_line_id = a_line_id - 1;
  line_t* a_line = g_text->lines[a_line_id];
  line_t* p_line = g_text->lines[p_line_id];
  if (! a_line) {
    a_line = psp_editor_add_line(a_line_id, "");
  }
  if (! p_line) {
    p_line = psp_editor_add_line(p_line_id, "");
  }
  int a_width = a_line->width;
  int p_width = p_line->width;
  int r_width = p_width + a_width;
  psp_editor_resize_line(p_line, r_width);
  if (r_width > p_line->max_width) r_width = p_line->max_width;
  int delta = r_width - p_width;
  if (delta > 0) {
    memcpy(&p_line->text[p_width], a_line->text, delta);
  }
  p_line->width = r_width;
  p_line->text[p_line->width] = 0;
  g_text->curr_col  = p_width;
  g_text->curr_line = p_line_id;
  psp_editor_delete_line(a_line_id);
  g_text->sel_mode = 0;
}

void
psp_editor_delete_curr_char()
{
  if (g_text->sel_mode) {
    psp_editor_cut();
  } else {
    if (g_text->curr_col > 0) {
      psp_editor_delete_char(g_text->curr_col);
      g_text->curr_col--;
      psp_editor_rewrap_if_needed();
      psp_editor_update_column(1);
    } else
    if (g_text->curr_line > 0) {
      psp_editor_join_prev_line(g_text->curr_line);
      psp_editor_rewrap_if_needed();
      psp_editor_update_column(1);
      psp_editor_update_line();
    }
  }
  g_text->sel_mode = 0;
}

void
psp_editor_suppr_curr_char()
{
  if (g_text->sel_mode) {
    psp_editor_cut();
  } else {
    line_t* a_line = g_text->lines[g_text->curr_line];
    if (! a_line) {
      a_line = psp_editor_add_line(g_text->curr_line, "");
    }
    if (g_text->curr_col >= a_line->width) {
      psp_editor_join_prev_line(g_text->curr_line + 1);
    } else {
      psp_editor_suppr_char(g_text->curr_col);
    }
  }
  g_text->sel_mode = 0;
  psp_editor_rewrap_if_needed();
}

void
psp_editor_clear_curr_line()
{
  int line_id = g_text->curr_line;
  line_t* a_line = g_text->lines[line_id];
  if (! a_line) {
    a_line = psp_editor_add_line(line_id, "");
  }
  a_line->width = 0;
  a_line->text[0] = 0;
  g_text->curr_col = 0;
  psp_editor_update_column(1);
  g_text->sel_mode = 0;
}

void
psp_editor_new(const char* filename)
{
  psp_editor_reset_text();
  PSPWRITE.is_modified = 0;
  PSPWRITE.ask_overwrite = 1;

  strcpy(PSPWRITE.edit_filename, filename);
}

static char loc_Buffer[EDITOR_MAX_WIDTH];
static char loc_Line[EDITOR_MAX_WIDTH + 32];

int
psp_editor_load(char *edit_filename)
{
  FILE* FileDesc;

  psp_editor_reset_text();
  PSPWRITE.is_modified = 0;

  strncpy(PSPWRITE.edit_filename, edit_filename, MAX_PATH);

  FileDesc = fopen( edit_filename, "r");
  if (FileDesc != (FILE *)0) {


    int line_id = 0;
    while (fgets(loc_Buffer, g_text->max_width, FileDesc) != (char *)0) {
      char *Scan = strchr(loc_Buffer,'\n');
      if (Scan) *Scan = '\0';
      /* For this #@$% of windows ! */
      Scan = strchr(loc_Buffer,'\r');
      if (Scan) *Scan = '\0';

      /* Copy the buffer into line */
      int tab;
      int index;
      int target = 0;
      for (index = 0; loc_Buffer[index]; index++) {
        uchar c = (uchar)loc_Buffer[index];
        /* expand tabs */
        if (c == '\t') {
          if (PSPWRITE.expand_tab) {
            for (tab = 0; tab < PSPWRITE.tab_stop; tab++) {
              loc_Line[target++] = ' ';
            }
          } else loc_Line[target++] = c;

        } else {

          if (c < ' ') c = ' ';
          loc_Line[target++] = c;
        }
        if (target >= EDITOR_MAX_WIDTH) break;
      }
      loc_Line[target] = 0;

      if ((target >= EDITOR_MAX_WIDTH) || (PSPWRITE.wrap_mode && target)) {
        if (g_text->wrap_buffer_size < target) {
          g_text->wrap_buffer = realloc(g_text->wrap_buffer, target + 1);
          g_text->wrap_buffer_size = target;
        }
        strcpy(g_text->wrap_buffer, loc_Line);
        line_id = psp_editor_rewrap_buffer_line(line_id, -1);

      } else {
        psp_editor_add_line(line_id, loc_Line);
        line_id++;
      }
      if (line_id >= g_text->max_lines) break;
    }
    PSPWRITE.is_modified = 0;
    fclose(FileDesc);

    return 0;
  }
  return 1;
}

int
psp_editor_save( char* filename )
{
  FILE* FileDesc;

  PSPWRITE.is_modified = 0;

  if (g_text->num_lines < 1) {
    remove(filename);
    ATARI.comment_present = 0;
    return 0;

  } else
  if (g_text->num_lines == 1) {
    line_t* a_line = g_text->lines[0];
    if (!a_line || !a_line->width) {
      remove(filename);
      ATARI.comment_present = 0;
      return 0;
    }
  }

  FileDesc = fopen(filename, "w");

  if (FileDesc != (FILE *)0) {

    int line_id = 0;
    int last_line_empty = 1;
    for (line_id = 0; line_id < g_text->num_lines; line_id++) {
      line_t* a_line = g_text->lines[line_id];
      if (a_line && a_line->width) {
        memcpy(loc_Buffer, a_line->text, a_line->width);
        loc_Buffer[a_line->width] = 0;
        fputs(loc_Buffer, FileDesc);
        last_line_empty = 0;
      } else last_line_empty = 1;

      if (a_line && psp_editor_is_neol(a_line)) {
        fprintf(FileDesc, " ");
      } else {
        if (PSPWRITE.dos_mode) {
          fprintf(FileDesc, "\r");
        }
        fprintf(FileDesc, "\n");
      }
    }
    if (last_line_empty) {
      if (PSPWRITE.dos_mode) {
        fprintf(FileDesc, "\r");
      }
      fprintf(FileDesc, "\n");
    }
    fclose(FileDesc);

    ATARI.comment_present = 1;
    return 0;
  }
  return 1;
}

void
psp_editor_init()
{
  if (! g_text) {
    g_text = (text_t *)malloc(sizeof( text_t ));
  }
  memset(g_text, 0, sizeof( text_t ));

  if (! g_clip) {
    g_clip = (clip_t *)malloc(sizeof( clip_t ));
  }
  memset(g_clip, 0, sizeof( clip_t ));

  g_text->max_width = EDITOR_MAX_WIDTH;
  g_text->max_lines = EDITOR_MAX_LINES;
  g_text->wrap_buffer = malloc( EDITOR_MAX_WIDTH + 1 );
  g_text->wrap_buffer_size = EDITOR_MAX_WIDTH;

  psp_editor_new("");

  /* initialize color */
  editor_colors[COLOR_WHITE         ] = psp_sdl_rgb(255, 255, 255);
  editor_colors[COLOR_BLACK         ] = psp_sdl_rgb(0, 0, 0);
  editor_colors[COLOR_DARK_BLUE     ] = psp_sdl_rgb(0, 0, 0x55);
  editor_colors[COLOR_GREEN         ] = psp_sdl_rgb(0, 0xAA, 0);
  editor_colors[COLOR_RED           ] = psp_sdl_rgb(0xAA, 0, 0);
  editor_colors[COLOR_BROWN         ] = psp_sdl_rgb(0xAA, 0x55, 0);
  editor_colors[COLOR_MAGENTA       ] = psp_sdl_rgb(0xAA, 0, 0xAA);
  editor_colors[COLOR_ORANGE        ] = psp_sdl_rgb(255, 0xAA, 0);
  editor_colors[COLOR_YELLOW        ] = psp_sdl_rgb(255, 255, 0x55);
  editor_colors[COLOR_BRIGHT_GREEN  ] = psp_sdl_rgb(0, 255, 0);
  editor_colors[COLOR_CYAN          ] = psp_sdl_rgb(0, 0xff, 0xff);
  editor_colors[COLOR_BRIGHT_BLUE   ] = psp_sdl_rgb(0x55, 0x55, 255);
  editor_colors[COLOR_BLUE          ] = psp_sdl_rgb(0, 0, 0xAA);
  editor_colors[COLOR_PINK          ] = psp_sdl_rgb(255, 0, 255);
  editor_colors[COLOR_GRAY          ] = psp_sdl_rgb(0x55, 0x55, 0x55);
  editor_colors[COLOR_BRIGHT_GRAY   ] = psp_sdl_rgb(0xAA, 0xAA, 0xAA);
  editor_colors[COLOR_IMAGE         ] = PSP_MENU_TEXT_COLOR;
}

# define PSP_EDITOR_BLINK_TIME 500

void
psp_display_cursor()
{
  int real_y_min = (PSP_SDL_SCREEN_HEIGHT - (PSPWRITE.screen_h * psp_font_height)) / 2;
  int real_x_min = (PSP_SDL_SCREEN_WIDTH  - (PSPWRITE.screen_w * psp_font_width)) / 2;

  /* Display cursor */
  int fg_color = editor_colors[PSPWRITE.fg_color];
  u32 curr_clock = SDL_GetTicks();
  if ((curr_clock - g_text->last_blink) > PSP_EDITOR_BLINK_TIME) {
    g_text->last_blink = curr_clock;
    g_text->cursor_blink = ! g_text->cursor_blink;
  }
  if (g_text->cursor_blink || g_text->cursor_moving) {
    int c_y = g_text->curr_line - g_text->top_line;
    int c_x = g_text->curr_col - g_text->left_col;
    if ((c_y >= 0) && (c_x >= 0)) {
      int c_real_y = real_y_min + c_y * psp_font_height;
      int c_real_x = real_x_min + c_x * psp_font_width;
      psp_sdl_fill_rectangle(c_real_x, c_real_y, 0, psp_font_height, fg_color, 0);
    }
  }
}

static int
psp_editor_is_sel_region(int line_id, int col_id)
{
  if (! g_text->sel_mode) return 0;

  if ((g_text->sel_to_line != g_text->sel_from_line) ||
      (g_text->sel_to_col  != g_text->sel_from_col )) {

    if (g_text->sel_to_line == g_text->sel_from_line) {
      if ((line_id == g_text->sel_from_line) &&
          (col_id  >= g_text->sel_from_col ) &&
          (col_id  <= g_text->sel_to_col   )) {
        return 1;
      }

    } else {
      if ((line_id == g_text->sel_from_line) &&
          (col_id  >= g_text->sel_from_col )) {
        return 1;
      }
      if ((line_id == g_text->sel_to_line) &&
          (col_id  <= g_text->sel_to_col )) {
        return 1;
      }
      if ((line_id < g_text->sel_to_line  ) &&
          (line_id > g_text->sel_from_line)) {
        return 1;
      }
    }
  }
  return 0;
}

void
psp_editor_clear_clipboard()
{
  line_list_t *scan_list;
  line_list_t *del_list;

  scan_list = g_clip->head_list;
  while (scan_list) {
    del_list  = scan_list;
    scan_list = scan_list->next;
    line_t* del_line = del_list->line;
    if (del_line) {
      if (del_line->text) {
        free(del_line->text);
      }
      free(del_line);
    }
    free(del_list);
  }
  g_clip->head_list = 0;
  g_clip->last_list = 0;
  g_clip->number_lines = 0;
}

void
psp_editor_append_clipboard(line_t* a_line, int col_from, int col_to)
{
  line_t*      a_clip_line;
  line_list_t* a_line_list;

  a_line_list = (line_list_t*)malloc( sizeof(line_list_t) );
  a_line_list->next = 0;

  g_clip->number_lines++;
  if (! g_clip->head_list) {
    g_clip->head_list = a_line_list;
    g_clip->last_list = a_line_list;
  } else {
    g_clip->last_list->next = a_line_list;
    g_clip->last_list = a_line_list;
  }

  a_clip_line = psp_editor_alloc_line();
  a_line_list->line = a_clip_line;

  if (a_line && a_line->width) {
    if (col_from < a_line->width) {
      if (col_to > a_line->width) col_to = a_line->width;
      int length = col_to - col_from + 1;
      a_clip_line->width     = length;
      a_clip_line->max_width = length;
      a_clip_line->text = malloc( length + 1 );
      memcpy(a_clip_line->text, a_line->text + col_from, length);
      a_clip_line->text[length] = 0;
    }
  }
}

int
psp_editor_is_valid_sel()
{
  if (! g_text->sel_mode) return 0;

  if ((g_text->sel_from_line != g_text->sel_to_line) ||
      (g_text->sel_from_col  != g_text->sel_to_col )) return 1;

  return 0;
}

void
psp_editor_copy()
{
  line_t* a_line;
  int     curr_line;

  if (! psp_editor_is_valid_sel()) return;

  /* Clear previous clip board */
  psp_editor_clear_clipboard();

  /* Copy lines to clipboard */
  if (g_text->sel_from_line == g_text->sel_to_line) {
    line_t* a_line = g_text->lines[g_text->sel_from_line];
    psp_editor_append_clipboard(a_line, g_text->sel_from_col, g_text->sel_to_col);
  } else {
    /* first line */
    a_line = g_text->lines[g_text->sel_from_line];
    if (a_line) {
      psp_editor_append_clipboard(a_line, g_text->sel_from_col, a_line->width);
    } else {
      psp_editor_append_clipboard(a_line, 0, 0);
    }
    /* middle lines */
    curr_line = g_text->sel_from_line + 1;
    while (curr_line < g_text->sel_to_line) {
      a_line = g_text->lines[curr_line];
      if (a_line) {
        psp_editor_append_clipboard(a_line, 0, a_line->width);
      } else {
        psp_editor_append_clipboard(a_line, 0, 0);
      }
      curr_line++;
    }
    /* last line */
    a_line = g_text->lines[curr_line];
    if (a_line) {
      psp_editor_append_clipboard(a_line, 0, g_text->sel_to_col);
    } else {
      psp_editor_append_clipboard(a_line, 0, 0);
    }
  }

# if 0
  psp_editor_display_clipboard();
# endif
  g_text->sel_mode = 0;
}

void
psp_editor_del_col_from_to(line_t* a_line, int col_from, int col_to)
{
  if ((! a_line) || (col_from > a_line->width)) return;

  PSPWRITE.is_modified = 1;

  if (col_to > a_line->width) col_to = a_line->width;
  char* a_text = a_line->text;
  int  index = 0;
  for (index = col_to + 1; index <= a_line->width; index++) {
    a_text[col_from++] = a_text[index];
  }
  a_text[col_from] = 0;
  a_line->width = col_from;
}

void
psp_editor_del_line_from(int line_id, int num_lines)
{
  PSPWRITE.is_modified = 1;

  int index = 0;
  while (index < num_lines) {
    line_t* a_line = g_text->lines[line_id + index];
    if (a_line) {
      if (a_line->text) free(a_line->text);
      a_line->text = 0;
      free(a_line);
      g_text->lines[line_id + index] = 0;
      a_line = 0;
    }
    index++;
  }
  int line_id_last = line_id + num_lines;
  int delta = g_text->num_lines - line_id_last;
  if (delta > 0) {
    memcpy(&g_text->lines[line_id], &g_text->lines[line_id_last], sizeof(line_t *) * delta);
    memset(&g_text->lines[g_text->num_lines - num_lines], 0, sizeof(line_t *) * num_lines);
    g_text->num_lines -= num_lines;
  }
  if (g_text->num_lines <= 0) {
    psp_editor_add_line(0, "");
  }
  g_text->sel_mode = 0;
}

void
psp_editor_cut()
{
  if (! psp_editor_is_valid_sel()) return;

  psp_editor_copy();

  g_text->curr_col  = g_text->sel_from_col;
  g_text->curr_line = g_text->sel_from_line;

  line_t* a_line;

  /* only one line */
  if (g_text->sel_from_line == g_text->sel_to_line) {
    /* del characters between from -> to */
    a_line = g_text->lines[g_text->sel_from_line];
    psp_editor_del_col_from_to(a_line, g_text->sel_from_col, g_text->sel_to_col);

  } else {
    /* first line, del characters between from -> width */
    a_line = g_text->lines[g_text->sel_from_line];
    psp_editor_del_col_from_to(a_line, g_text->sel_from_col, a_line->width);
    /* last line */
    a_line = g_text->lines[g_text->sel_to_line];
    psp_editor_del_col_from_to(a_line, 0, g_text->sel_to_col);

    int delta = g_text->sel_to_line - g_text->sel_from_line - 1;
    if (delta > 0) {
      psp_editor_del_line_from(g_text->sel_from_line + 1, delta);
    }
    psp_editor_suppr_curr_char();
  }
  g_text->sel_mode = 0;
  psp_editor_rewrap_if_needed();
  psp_editor_update_column(1);
  psp_editor_update_line();
}

void
psp_editor_paste()
{
  if ((! g_clip->head_list) || (g_text->sel_mode)) return;

  /* merge first clip line with current line */
  line_list_t *scan_list = g_clip->head_list;
  if (scan_list) {
    line_t* a_line = scan_list->line;
    int index;
    for (index = 0; index < a_line->width; index++) {
      psp_editor_insert_char(g_text->curr_line, g_text->curr_col, a_line->text[index]);
      g_text->curr_col++;
    }
    scan_list = scan_list->next;
    if (scan_list) {
      psp_editor_split_curr_line();
    }
  }
  if (scan_list) {
    /* copy all other lines */
    while (scan_list != g_clip->last_list) {
      line_t* a_line = scan_list->line;
      char*   a_text = a_line->text;
      if (! a_text) a_text = "";
      psp_editor_insert_line(g_text->curr_line, a_text);
      g_text->curr_line++;
      scan_list = scan_list->next;
    }
  }
  /* last line */
  if (g_clip->head_list != g_clip->last_list) {
    line_t* a_line = g_clip->last_list->line;
    int index;
    for (index = 0; index < a_line->width; index++) {
      psp_editor_insert_char(g_text->curr_line, g_text->curr_col, a_line->text[index]);
      g_text->curr_col++;
    }
  }
  psp_editor_rewrap_if_needed();
  psp_editor_update_column(1);
  psp_editor_update_line();
}

void
psp_display_editor()
{
  int line_from = g_text->top_line;
  int line_to   = line_from + PSPWRITE.screen_h;
  if (line_to > g_text->num_lines) line_to = g_text->num_lines;

  int line_id;

  int real_y_min = (PSP_SDL_SCREEN_HEIGHT - (PSPWRITE.screen_h * psp_font_height)) / 2;
  int real_x_min = (PSP_SDL_SCREEN_WIDTH  - (PSPWRITE.screen_w * psp_font_width)) / 2;

  int y = 0;
  int real_y = real_y_min;
  int fg_color = editor_colors[PSPWRITE.fg_color];

  if (PSPWRITE.bg_color == COLOR_IMAGE) {
    psp_sdl_blit_help();
  } else {
    int bg_color = editor_colors[PSPWRITE.bg_color];
    psp_sdl_clear_screen(bg_color);
  }

  for (line_id = line_from; line_id < line_to; line_id++) {
    line_t* a_line = g_text->lines[line_id];
    if (a_line && a_line->text) {
      int col_from = g_text->left_col;
      int col_to   = a_line->width;
      int col_id;
      int x = 0;
      int real_x = real_x_min;
      for (col_id = col_from; col_id < col_to; col_id++) {
        unsigned char c = a_line->text[col_id];
        if (c == '\t') c = 1;
        else if (c < ' ') c = ' ';
        //if (psp_editor_is_sel_region(line_id, col_id) || (psp_editor_is_neol(a_line)))
        if (psp_editor_is_sel_region(line_id, col_id))
        {
          psp_sdl_put_char(real_x, real_y, fg_color, 0, c, 0, 1);
        } else {
          psp_sdl_put_char(real_x, real_y, fg_color, 0, c, 1, 0);
        }
        real_x += psp_font_width;
        x++;
        if (x >= PSPWRITE.screen_w) break;
      }
    }
    real_y += psp_font_height;
    y++;
    if (y > PSPWRITE.screen_h) break;
  }
}

# define PSP_EDITOR_MIN_MOVE_TIME   10000
# define PSP_EDITOR_REPEAT_TIME    300000
# define PSP_EDITOR_MIN_EDIT_TIME  180000
# define PSP_EDITOR_MIN_TIME       150000

void
psp_editor_menu(char* filename)
{
static int first_time = 1;

  int  danzeff_mode;
  int  danzeff_key;
  long old_pad;
  long new_pad;
  int  last_time;
  int  repeat_mode;

  SceCtrlData c;

  old_pad      = 0;
  last_time    = 0;
  danzeff_mode = 0;
  repeat_mode  = 0;

  new_pad = 0;

  psp_kbd_wait_no_button();

  if (first_time) {
    psp_editor_init();
    psp_editor_default();
    first_time = 0;
  }

  if (strcmp(PSPWRITE.edit_filename, filename)) {
    if (psp_editor_load(filename)) {
      psp_editor_new(filename);
    }
  }

  while (1) {

    psp_display_editor();
    psp_display_cursor();

    if (danzeff_mode) {
      danzeff_moveTo(0, 0);
      danzeff_render( ATARI.danzeff_trans );
    }
    psp_sdl_flip();

    while (1) {
      gp2xCtrlPeekBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;
      new_pad = c.Buttons;

      if (old_pad != new_pad) {
        repeat_mode = 0;
        last_time = c.TimeStamp;
        old_pad = new_pad;
        break;

      } else
      if (new_pad != 0) {
        if ((c.TimeStamp - last_time) > PSP_EDITOR_REPEAT_TIME) {
          repeat_mode = 1;
        }
      } else {
        repeat_mode = 0;
      }

      if (repeat_mode) {
        if ((c.TimeStamp - last_time) > PSP_EDITOR_MIN_MOVE_TIME) {
          last_time = c.TimeStamp;
          old_pad = new_pad;
          break;
        }
      } else {

        if ((c.TimeStamp - last_time) > PSP_EDITOR_MIN_TIME) {
          last_time = c.TimeStamp;
          old_pad = new_pad;
          break;
        }
      }
    }

    if (danzeff_mode) {

      danzeff_key = danzeff_readInput( &c);

      if (danzeff_key > DANZEFF_START) {

        if (danzeff_key >= ' ') {
          psp_editor_insert_curr_char(danzeff_key);
        } else
        if (danzeff_key == DANZEFF_DEL) {
          psp_editor_delete_curr_char();
        } else
        if (danzeff_key == DANZEFF_TAB) {
          psp_editor_insert_curr_char('\t');
        } else
        if (danzeff_key == DANZEFF_CLR) {
          psp_editor_clear_curr_line();
        } else
# if 0
        if (danzeff_key == DANZEFF_HOME) {
          psp_editor_goto_first_line();
        } else
        if (danzeff_key == DANZEFF_PAGE_UP) {
          psp_editor_goto_page_up();
        } else
        if (danzeff_key == DANZEFF_PAGE_DOWN) {
          psp_editor_goto_page_down();
        } else
        if (danzeff_key == DANZEFF_END) {
          psp_editor_goto_col_end();
        } else
        if (danzeff_key == DANZEFF_ENTER) {
          psp_editor_split_curr_line();
        } else
        if (danzeff_key == DANZEFF_UP) {
          psp_editor_goto_col_begin();
        } else
        if (danzeff_key == DANZEFF_DOWN) {
          psp_editor_split_curr_line();
        } else
        if (danzeff_key == DANZEFF_LEFT) {
          psp_editor_goto_col_left();
        } else
        if (danzeff_key == DANZEFF_RIGHT) {
          psp_editor_goto_col_right();
        }
# else
        if (danzeff_key == DANZEFF_ENTER) {
          psp_editor_split_curr_line();
        }
# endif

      } else
      if ((danzeff_key == DANZEFF_START ) ||
          (danzeff_key == DANZEFF_SELECT))
      {
        danzeff_mode = 0;
        old_pad = new_pad = 0;

        psp_kbd_wait_no_button();

      }
# if 0
      else
      if (danzeff_key == DANZEFF_UP) {
          psp_editor_goto_line_up();
      } else
      if (danzeff_key == DANZEFF_DOWN) {
        psp_editor_goto_line_down();
      } else
      if (danzeff_key == DANZEFF_LEFT) {
        psp_editor_goto_col_left();
      } else
      if (danzeff_key == DANZEFF_RIGHT) {
        psp_editor_goto_col_right();
      }
# endif

      continue;
    }

    if (!c.Buttons) {
      g_text->cursor_moving = 0;
    }

    if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
        (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
      /* Exit ! */
      psp_sdl_exit(0);
    } else
    if(new_pad & PSP_CTRL_START) {
      danzeff_mode = 1;
    } else
    if (new_pad & PSP_CTRL_RTRIGGER) {

      if(new_pad & PSP_CTRL_UP) {
        psp_editor_goto_page_up();
      } else
      if(new_pad & PSP_CTRL_DOWN) {
        psp_editor_goto_page_down();
      } else
      if(new_pad & PSP_CTRL_RIGHT) {
        psp_editor_goto_word_right();
      } else
      if(new_pad & PSP_CTRL_LEFT) {
        psp_editor_goto_word_left();
      } else
      if(new_pad & PSP_CTRL_SELECT) {
        psp_editor_sel_mode();
      } else
      if(new_pad & PSP_CTRL_TRIANGLE) {
        psp_editor_copy();
      } else
      if(new_pad & PSP_CTRL_CROSS) {
        psp_editor_cut();
      } else
      if(new_pad & PSP_CTRL_CIRCLE) {
        psp_editor_paste();
      } else
      if(new_pad & PSP_CTRL_SQUARE) {
        psp_editor_rewrap_curr_line();
      }

    } else
    if (new_pad & PSP_CTRL_LTRIGGER) {

      if(new_pad & PSP_CTRL_UP) {
        psp_editor_goto_first_line();
      } else
      if(new_pad & PSP_CTRL_DOWN) {
        psp_editor_goto_last_line();
      } else
      if(new_pad & PSP_CTRL_RIGHT) {
        psp_editor_goto_col_end();
      } else
      if(new_pad & PSP_CTRL_LEFT) {
        psp_editor_goto_col_begin();
      } else
# if 0
      if(new_pad & PSP_CTRL_SELECT) {
        psp_help_menu();
      } else
# endif
      if(new_pad & PSP_CTRL_TRIANGLE) {
        psp_editor_goto_first_line();
      } else
      if(new_pad & PSP_CTRL_CROSS) {
        psp_editor_goto_last_line();
      } else
      if(new_pad & PSP_CTRL_CIRCLE) {
        psp_editor_goto_col_end();
      } else
      if(new_pad & PSP_CTRL_SQUARE) {
        psp_editor_goto_col_begin();
      }

    } else
    if(new_pad & PSP_CTRL_UP) {
      psp_editor_goto_line_up();
    } else
    if(new_pad & PSP_CTRL_DOWN) {
      psp_editor_goto_line_down();
    } else
    if(new_pad & PSP_CTRL_RIGHT) {
      psp_editor_goto_col_right();
    } else
    if(new_pad & PSP_CTRL_LEFT) {
      psp_editor_goto_col_left();
    } else
    if(new_pad & PSP_CTRL_TRIANGLE) {
      psp_editor_delete_curr_char();
    } else
    if(new_pad & PSP_CTRL_SQUARE) {
      psp_editor_suppr_curr_char();
    } else
    if(new_pad & PSP_CTRL_CIRCLE) {
      psp_editor_insert_curr_char(' ');
    } else
    if(new_pad & PSP_CTRL_CROSS) {
      psp_editor_split_curr_line();
    } else
    if(new_pad & PSP_CTRL_SELECT) {
      break;
    }
  }

  if (PSPWRITE.is_modified) {
    psp_editor_save( filename );
  }

  psp_kbd_wait_no_button();
}
