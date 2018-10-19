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

# ifndef _PSP_MENU_H_
# define _PSP_MENU_H_

# ifdef __cplusplus
extern "C" {
# endif

# define PSP_MENU_BORDER_COLOR     psp_sdl_rgb(0x80,0x80,0xF0)
# define PSP_MENU_WARNING_COLOR    psp_sdl_rgb(0xFF,0x00,0x00)
# define PSP_MENU_NOTE_COLOR       psp_sdl_rgb(0xFF,0xFF,0x00)
# define PSP_MENU_BACKGROUND_COLOR psp_sdl_rgb(0x00,0x00,0x00)
# define PSP_MENU_BLACK_COLOR      psp_sdl_rgb(0x00,0x00,0x00)
# define PSP_MENU_AUTHOR_COLOR     psp_sdl_rgb(0x00,0x00,0xFF)
# define PSP_MENU_GREEN_COLOR      psp_sdl_rgb(0x00,0xFF,0x00)
# define PSP_MENU_RED_COLOR        psp_sdl_rgb(0xFF,0x00,0x00)

# define PSP_MENU_TEXT_COLOR       psp_sdl_rgb(0x80,0x80,0xFF)
# define PSP_MENU_TEXT2_COLOR      psp_sdl_rgb(0xff,0xff,0xff)
# define PSP_MENU_SEL_COLOR        psp_sdl_rgb(0x00,0xff,0xff)
# define PSP_MENU_SEL2_COLOR       psp_sdl_rgb(0xFF,0x00,0x80)

# define PSP_MENU_MIN_TIME         150000

  typedef struct menu_item_t {
    char *title;
  } menu_item_t;


   extern int psp_main_menu(void);
  extern void string_fill_with_space(char *buffer, int size);
  extern void psp_menu_display_save_name();
# ifdef __cplusplus
}
#endif

# endif
