/*
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

# ifndef _PSP_SDL_H_
# define _PSP_SDL_H_

# ifdef __cplusplus
extern "C" {
# endif
# define psp_debug(m)   loc_psp_debug(__FILE__,__LINE__,m)

# define PSP_SDL_NOP   0
# define PSP_SDL_XOR   1

# define PSP_LINE_SIZE  320

# define PSP_SDL_SCREEN_WIDTH    320
# define PSP_SDL_SCREEN_HEIGHT   240

  typedef unsigned char   uchar;
  typedef unsigned int    uint;
  typedef unsigned short  ushort;

  extern SDL_Surface* back_surface;
  extern SDL_Surface* blit_surface;
  extern SDL_Surface* save_surface;
  extern SDL_Surface* thumb_surface;

  extern int psp_load_fonts(void);
  extern int psp_print_text(char * str, int colour, int v, int h);

  extern void loc_psp_debug(char *file, int line, char *message);

  /* PG -> SDL function */

  extern void psp_sdl_print(int x,int y, char *str, int color);
  extern void psp_sdl_clear_screen(int color);
  extern void psp_sdl_fill_rectangle(int x, int y, int w, int h, int color, int mode);
  extern void psp_sdl_draw_rectangle(int x, int y, int w, int h, int border, int mode);
  extern void psp_sdl_put_char(int x, int y, int color, int bgcolor, uchar c, int drawfg, int drawbg);
  extern void psp_sdl_fill_print(int x,int y,const char *str, int color, int bgcolor);
  extern void psp_sdl_flip(void);
  extern void psp_sdl_back2_print(int x,int y,const char *str, int color);
  extern void psp_sdl_back2_rectangle(int x, int y, int w, int h);

  extern void psp_sdl_gu_stretch(SDL_Rect* srcRect, SDL_Rect* dstRect);
  extern void psp_sdl_lock(void);
  extern void psp_sdl_flush(void);
  extern void psp_sdl_save_bmp(char *filename);
  extern void psp_sdl_blit_background();
  extern int psp_sdl_load_thumb_png(SDL_Surface* my_surface, char* filename);
  extern int psp_sdl_save_thumb_png(SDL_Surface* my_surface, char* filename);
  extern uint psp_sdl_rgb(uchar R, uchar G, uchar B);
  extern void psp_sdl_save_screenshot(void);
  extern int psp_sdl_init(void);
  extern void psp_sdl_black_screen();
  extern void psp_sdl_exit(int status);
  extern void psp_sdl_blit_thumb(int dst_x, int dst_y, SDL_Surface* thumb_surface);
  extern void psp_sdl_clear_blit(int color);
  extern unsigned char psp_convert_utf8_to_iso_8859_1(unsigned char c1, unsigned char c2);
# endif
