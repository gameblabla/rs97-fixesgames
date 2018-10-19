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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <png.h>

#include "global.h"
#include "atari.h"
#include "psp_sdl.h"
#include "psp_danzeff.h"

SDL_Surface *ScreenSurface;

  extern unsigned char psp_font_8x8[];
  extern unsigned char psp_font_6x10[];

  unsigned char *psp_font;
  int            psp_font_width  = 8;
  int            psp_font_height = 8;

  SDL_Surface *back_surface;
  SDL_Surface *back2_surface;

  SDL_Surface *background_surface = NULL;
  SDL_Surface *blit_surface;
  SDL_Surface *help_surface;
  SDL_Surface *splash_surface;
  SDL_Surface *thumb_surface;
  SDL_Surface *save_surface;

uint
psp_sdl_rgb(uchar R, uchar G, uchar B)
{
  return SDL_MapRGB(back_surface->format, R,G,B);
}

ushort *
psp_sdl_get_vram_addr(uint x, uint y)
{
  ushort *vram = (ushort *)back_surface->pixels;
  return vram + x + (y*PSP_LINE_SIZE);
}

ushort *
psp_sdl_get_blit_addr()
{
  return (ushort *)blit_surface->pixels;
}

void
loc_psp_debug(char *file, int line, char *message)
{
  static int current_line = 0;
  static int current_col  = 10;

  char buffer[128];
  current_line += 10;
  if (current_line > 220)
  {
    if (current_col == 200) {
      psp_sdl_clear_screen(psp_sdl_rgb(0, 0, 0xff));
      current_col = 10;
    } else {
      current_col = 200;
    }

    current_line = 10;
  }
  sprintf(buffer,"%s:%d %s", file, line, message);
  psp_sdl_print(current_col, current_line, buffer, psp_sdl_rgb(0xff,0xff,0xff) );
}

void
psp_sdl_print(int x,int y, char *str, int color)
{
  int index;
  int x0 = x;

  for (index = 0; str[index] != '\0'; index++) {
    psp_sdl_put_char(x, y, color, 0, str[index], 1, 0);
    x += psp_font_width;
    if (x >= (PSP_SDL_SCREEN_WIDTH - psp_font_width)) {
      x = x0; y+= psp_font_height;
    }
    if (y >= (PSP_SDL_SCREEN_HEIGHT - psp_font_width)) break;
  }
}

void
psp_sdl_clear_screen(int color)
{
  int x; int y;
  ushort *vram = psp_sdl_get_vram_addr(0,0);

  for (y = 0; y < PSP_SDL_SCREEN_HEIGHT; y++) {
    for (x = 0; x < PSP_SDL_SCREEN_WIDTH; x++) {
      vram[x + (y*PSP_LINE_SIZE)] = color;
    }
  }
}

void
psp_sdl_black_screen()
{
  SDL_FillRect(back_surface,NULL,SDL_MapRGB(back_surface->format,0x0,0x0,0x0));
  //SDL_Flip(back_surface);
  SDL_FillRect(back_surface,NULL,SDL_MapRGB(back_surface->format,0x0,0x0,0x0));
  //SDL_Flip(back_surface);
  
	SDL_FillRect(ScreenSurface,NULL,SDL_MapRGB(ScreenSurface->format,0x0,0x0,0x0));
  SDL_Flip(ScreenSurface);
  SDL_FillRect(ScreenSurface,NULL,SDL_MapRGB(ScreenSurface->format,0x0,0x0,0x0));
  SDL_Flip(ScreenSurface);
}

void
psp_sdl_clear_blit(int color)
{
  if (blit_surface) {
    ushort *vram = (ushort *)blit_surface->pixels;
    int my_size = blit_surface->h * blit_surface->w;
    while (my_size > 0) {
      vram[--my_size] = color;
    }
  }
}

void
psp_sdl_draw_rectangle(int x, int y, int w, int h, int border, int mode)
{
  ushort *vram = (ushort *)psp_sdl_get_vram_addr(x, y);
  int xo, yo;
  if (mode == PSP_SDL_XOR) {
    for (xo = 0; xo < w; xo++) {
      vram[xo] ^=  border;
      vram[xo + h * PSP_LINE_SIZE] ^=  border;
    }
    for (yo = 0; yo < h; yo++) {
      vram[yo * PSP_LINE_SIZE] ^=  border;
      vram[w + yo * PSP_LINE_SIZE] ^=  border;
    }
    vram[w + h * PSP_LINE_SIZE] ^=  border;
  } else {
    for (xo = 0; xo < w; xo++) {
      vram[xo] =  border;
      vram[xo + h * PSP_LINE_SIZE] =  border;
    }
    for (yo = 0; yo < h; yo++) {
      vram[yo * PSP_LINE_SIZE] =  border;
      vram[w + yo * PSP_LINE_SIZE] =  border;
    }
    vram[w + h * PSP_LINE_SIZE] =  border;
  }
}

void
psp_sdl_fill_rectangle(int x, int y, int w, int h, int color, int mode)
{
  ushort *vram  = (ushort *)psp_sdl_get_vram_addr(x, y);
  int xo, yo;
  if (mode == PSP_SDL_XOR) {
    for (xo = 0; xo <= w; xo++) {
      for (yo = 0; yo <= h; yo++) {
        if ( ((xo == 0) && ((yo == 0) || (yo == h))) ||
             ((xo == w) && ((yo == 0) || (yo == h))) ) {
          /* Skip corner */
        } else {
          vram[xo + yo * PSP_LINE_SIZE] ^=  color;
        }
      }
    }
  } else {
    for (xo = 0; xo <= w; xo++) {
      for (yo = 0; yo <= h; yo++) {
        vram[xo + yo * PSP_LINE_SIZE] =  color;
      }
    }
  }
}

static int
psp_sdl_get_back2_color(int x, int y)
{
  uchar *back2 = (uchar *)back2_surface->pixels;
  int bytes_per_pixels = back2_surface->format->BytesPerPixel;
  int pitch            = back2_surface->pitch;
  Uint8 r = back2[0 + (y * pitch) + (x * bytes_per_pixels)];
  Uint8 g = back2[1 + (y * pitch) + (x * bytes_per_pixels)];
  Uint8 b = back2[2 + (y * pitch) + (x * bytes_per_pixels)];
	int color = psp_sdl_rgb(r, g, b);

  return color;
}

void
psp_sdl_back2_rectangle(int x, int y, int w, int h)
{
  if (! back2_surface) {
    psp_sdl_fill_rectangle(x, y, w, h, 0x0, 0);
    return;
  }

  ushort *vram  = (ushort *)psp_sdl_get_vram_addr(x, y);

  int xo, yo;
  for (xo = 0; xo <= w; xo++) {
    for (yo = 0; yo <= h; yo++) {
      vram[xo + yo * PSP_LINE_SIZE] = psp_sdl_get_back2_color(x + xo, y + yo);
    }
  }
}

void
psp_sdl_put_char(int x, int y, int color, int bgcolor, uchar c, int drawfg, int drawbg)
{
  int cx;
  int cy;
  int b;
  int index;

  ushort *vram = (ushort *)psp_sdl_get_vram_addr(x, y);
  index = ((ushort)c) * psp_font_height;

  for (cy=0; cy< psp_font_height; cy++) {
    b = 1 << (psp_font_width - 1);
    for (cx=0; cx< psp_font_width; cx++) {
      if (psp_font[index] & b) {
        if (drawfg) vram[cx + cy * PSP_LINE_SIZE] = color;
      } else {
        if (drawbg) vram[cx + cy * PSP_LINE_SIZE] = bgcolor;
      }
      b = b >> 1;
    }
    index++;
  }
}

void
psp_sdl_back2_put_char(int x, int y, int color, uchar c)
{
  int cx;
  int cy;
  int bmask;
  int index;

  if (! back2_surface) {
    psp_sdl_put_char(x, y, color, 0x0, c, 1, 1);
    return;
  }

  ushort *vram  = (ushort *)psp_sdl_get_vram_addr(x, y);

  index = ((ushort)c) * psp_font_height;

  for (cy=0; cy< psp_font_height; cy++) {
    bmask = 1 << (psp_font_width - 1);
    for (cx=0; cx< psp_font_width; cx++) {
      if (psp_font[index] & bmask) {
        vram[cx + cy * PSP_LINE_SIZE] = color;
      } else {
        vram[cx + cy * PSP_LINE_SIZE] = psp_sdl_get_back2_color(x + cx, y + cy);
      }
      bmask = bmask >> 1;
    }
    index++;
  }
}

unsigned char
psp_convert_utf8_to_iso_8859_1(unsigned char c1, unsigned char c2)
{
  unsigned char res = 0;
  if (c1 == 0xc2) res = c2;
  else
  if (c1 == 0xc3) res = c2 | 0x40;
  return res;
}


void
psp_sdl_fill_print(int x,int y,const char *str, int color, int bgcolor)
{
  int index;
  int x0 = x;

  for (index = 0; str[index] != '\0'; index++) {
    uchar c = str[index];
    if ((c == 0xc2) || (c == 0xc3)) {
      uchar new_c = psp_convert_utf8_to_iso_8859_1(c, str[index+1]);
      if (new_c) { c = new_c; index++; }
    }
    psp_sdl_put_char(x, y, color, bgcolor, c, 1, 1);
    x += psp_font_width;
    if (x >= (PSP_SDL_SCREEN_WIDTH - psp_font_width)) {
      x = x0; y++;
    }
    if (y >= (PSP_SDL_SCREEN_HEIGHT - psp_font_width)) break;
  }
}

void
psp_sdl_back2_print(int x,int y,const char *str, int color)
{
  int index;
  int x0 = x;

  for (index = 0; str[index] != '\0'; index++) {
    uchar c = str[index];
    if ((c == 0xc2) || (c == 0xc3)) {
      uchar new_c = psp_convert_utf8_to_iso_8859_1(c, str[index+1]);
      if (new_c) { c = new_c; index++; }
    }
    psp_sdl_back2_put_char(x, y, color, c);
    x += psp_font_width;
    if (x >= (PSP_SDL_SCREEN_WIDTH - psp_font_width)) {
      x = x0; y++;
    }
    if (y >= (PSP_SDL_SCREEN_HEIGHT - psp_font_width)) break;
  }
}

void
psp_sdl_load_background()
{
  background_surface = IMG_Load("./menu.png");
  thumb_surface = IMG_Load("./thumb.png");
}

void
psp_sdl_blit_background()
{
  static int first = 1;

  if (first && (background_surface == NULL)) {
    psp_sdl_load_background();
    first = 0;
  }
  back2_surface = background_surface;

  if (back2_surface != NULL) {
	  SDL_BlitSurface(back2_surface, NULL, back_surface, NULL);
  } else {
    psp_sdl_clear_screen(psp_sdl_rgb(0x00, 0x00, 0x00));
  }
}

void
psp_sdl_blit_thumb(int dst_x, int dst_y, SDL_Surface* thumb_surface)
{
  SDL_Rect dstRect;
  dstRect.x = dst_x;
  dstRect.y = dst_y;
  dstRect.w = thumb_surface->w;
  dstRect.h = thumb_surface->h;
  SDL_BlitSurface(thumb_surface, NULL, back_surface, &dstRect);
}

void
psp_sdl_blit_splash()
{
  if (! splash_surface) {
    splash_surface = IMG_Load("./splash.png");
  }
	SDL_BlitSurface(splash_surface, NULL, back_surface, NULL);
}

void
psp_sdl_blit_help()
{
  if (! help_surface) {
    help_surface = IMG_Load("./help.png");
  }
  back2_surface = help_surface;
	SDL_BlitSurface(back2_surface, NULL, back_surface, NULL);
}

void
psp_sdl_display_splash()
{
  int index = 0;
  gp2xCtrlData c;

  //int x = (320 - (strlen(ATARI_VERSION) * 8)) / 2;
  //int y = 240 - 16;
  int x = 24;
  int y = 216;
  //int col = psp_sdl_rgb(0xa0, 0xa0, 0xa0);
  int col = psp_sdl_rgb(0x0, 0x0, 0x0);

  psp_sdl_blit_splash();
  psp_sdl_print(x, y, ATARI_VERSION, col);
  psp_sdl_flip();

  psp_sdl_blit_splash();
  psp_sdl_print(x, y, ATARI_VERSION, col);
  psp_sdl_flip();


  while (index < 50) {
    gp2xCtrlPeekBufferPositive(&c, 1);
    if (c.Buttons & (GP2X_CTRL_START|GP2X_CTRL_CROSS)) break;
    index++;
  }
}

void
psp_sdl_unlock(void)
{
  SDL_UnlockSurface(back_surface);
}

void
psp_sdl_flip(void)
{
  //SDL_Flip(back_surface);
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_LockSurface(ScreenSurface);
	int x, y;
	uint32_t *s = (uint32_t*)back_surface->pixels;
	uint32_t *d = (uint32_t*)ScreenSurface->pixels;
	for(y=0; y<240; y++){
		for(x=0; x<160; x++){
			*d++ = *s++;
		}
		d+= 160;
	}
	if(SDL_MUSTLOCK(ScreenSurface)) SDL_UnlockSurface(ScreenSurface);
	SDL_Flip(ScreenSurface);
}

#define  systemRedShift      (back_surface->format->Rshift)
#define  systemGreenShift    (back_surface->format->Gshift)
#define  systemBlueShift     (back_surface->format->Bshift)
#define  systemRedMask       (back_surface->format->Rmask)
#define  systemGreenMask     (back_surface->format->Gmask)
#define  systemBlueMask      (back_surface->format->Bmask)

int
psp_sdl_save_png(SDL_Surface* my_surface, char* filename)
{
  int w = my_surface->w;
  int h = my_surface->h;
  u8* pix = (u8*)my_surface->pixels;
  u8 writeBuffer[512 * 3];

  FILE *fp = fopen(filename,"wb");

  if(!fp) return 0;

  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL,
                                                NULL,
                                                NULL);
  if(!png_ptr) {
    fclose(fp);
    return 0;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if(!info_ptr) {
    png_destroy_write_struct(&png_ptr,NULL);
    fclose(fp);
    return 0;
  }

  png_init_io(png_ptr,fp);

  png_set_IHDR(png_ptr,
               info_ptr,
               w,
               h,
               8,
               PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr,info_ptr);

  u8 *b = writeBuffer;

  int sizeX = w;
  int sizeY = h;
  int y;
  int x;

  u16 *p = (u16 *)pix;
  for(y = 0; y < sizeY; y++) {
     for(x = 0; x < sizeX; x++) {
      u16 v = p[x];

      *b++ = ((v & systemRedMask  ) >> systemRedShift  ) << 3; // R
      *b++ = ((v & systemGreenMask) >> systemGreenShift) << 2; // G
      *b++ = ((v & systemBlueMask ) >> systemBlueShift ) << 3; // B
    }
    p += my_surface->pitch / 2;
    png_write_row(png_ptr,writeBuffer);

    b = writeBuffer;
  }

  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose(fp);
  return 1;
}

int
psp_sdl_load_png(SDL_Surface* my_surface, char* filename)
{
  int w = my_surface->w;
  int h = my_surface->h;
  int pitch = my_surface->pitch / 2;
  u16* pix = (u16*)my_surface->pixels;

  FILE *fp = fopen(filename,"rb");
  if (!fp) return 0;

  const size_t nSigSize = 8;
  u8 signature[nSigSize];
  if (fread(signature, sizeof(u8), nSigSize, fp) != nSigSize) {
    fclose(fp);
    return 0;
  }

  if (!png_check_sig(signature, nSigSize)) {
    fclose(fp);
    return 0;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                               NULL,
                                               NULL,
                                               NULL);
  if(!png_ptr) {
    fclose(fp);
    return 0;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if(!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    fclose(fp);
    return 0;
  }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, nSigSize);
  png_read_png(png_ptr, info_ptr,
    PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING |
    PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_BGR , NULL);

  png_uint_32 width = png_get_image_width( png_ptr,  info_ptr);//info_ptr->width;
  png_uint_32 height = png_get_image_height( png_ptr,  info_ptr);//info_ptr->height;
  int color_type = png_get_color_type( png_ptr,  info_ptr);//info_ptr->color_type;

  if ((width  > w) ||
      (height > h)) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return 0;
  }

  //png_byte **pRowTable = png_get_rowbytes( png_ptr,  info_ptr);//info_ptr->row_pointers;
  png_byte **pRowTable = png_get_rows( png_ptr,  info_ptr);//info_ptr->row_pointers;
  unsigned int x, y;
  u8 r, g, b;

  for (y=0; y<height; y++) {
    png_byte *pRow = pRowTable[y];
    for (x=0; x<width; x++) {
      switch(color_type) {
        case PNG_COLOR_TYPE_GRAY:
          r = g = b = *pRow++;
          break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
          r = g = b = pRow[0];
          pRow += 2;
          break;
        case PNG_COLOR_TYPE_RGB:
          b = pRow[0];
          g = pRow[1];
          r = pRow[2];
          pRow += 3;
          break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
          b = pRow[0];
          g = pRow[1];
          r = pRow[2];
          pRow += 4;
          break;
        default:
          r = g = b = 0;
          break;
      }

      u16 v = (((r >> 3) << systemRedShift  ) & systemRedMask) |
              (((g >> 2) << systemGreenShift) & systemGreenMask) |
              (((b >> 3) << systemBlueShift ) & systemBlueMask);
      *pix++= v;
    }
    pix += pitch - width;
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(fp);

  return 1;
}

int
psp_sdl_save_screenshot_png(char *filename)
{
  return psp_sdl_save_png(back_surface, filename);
}

int
psp_sdl_save_thumb_png(SDL_Surface* my_surface, char* filename)
{
  psp_atari_blit_image();

  /* First dump blit_surface to my_surface */
  int x;
  int y;
  ushort* src_pixel = (ushort*)blit_surface->pixels;
  ushort* dst_pixel = (ushort*)my_surface->pixels;
  ushort* scan_src_pixel = 0;

  for (y = 0; y < SNAP_HEIGHT; y++) {
    scan_src_pixel = src_pixel + (ATARI_WIDTH * y * 3);
    for (x = 0; x < SNAP_WIDTH; x++) {
      *dst_pixel++ = scan_src_pixel[x * 3];
    }
  }
  /* Then save thumb in file */
  return psp_sdl_save_png(my_surface, filename);
}

int
psp_sdl_load_thumb_png(SDL_Surface* my_surface, char* filename)
{
  return psp_sdl_load_png( my_surface, filename);
}

void
psp_sdl_save_screenshot(void)
{
  char TmpFileName[MAX_PATH];

  sprintf(TmpFileName,"%s/scr/screenshot_%d.png", ATARI.atari_home_dir, ATARI.psp_screenshot_id++);
  if (ATARI.psp_screenshot_id >= 10) ATARI.psp_screenshot_id = 0;
  psp_sdl_save_screenshot_png(TmpFileName);
}

int
psp_sdl_init(void)
{
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK) < 0) {
     return 0;
  }
  SDL_JoystickEventState(SDL_ENABLE);
  SDL_JoystickOpen(0);

  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    return 0;
  }

  psp_sdl_select_font_6x10();

  //back_surface=SDL_SetVideoMode(PSP_SDL_SCREEN_WIDTH,PSP_SDL_SCREEN_HEIGHT, 16, SDL_SWSURFACE);
	ScreenSurface = SDL_SetVideoMode(320, 480, 16, SDL_SWSURFACE);
  back_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, PSP_SDL_SCREEN_WIDTH, PSP_SDL_SCREEN_HEIGHT, 16, 0, 0, 0, 0);

  if ( !back_surface) {
    return 0;
  }

  blit_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
    ATARI_WIDTH, ATARI_HEIGHT,
    back_surface->format->BitsPerPixel,
    back_surface->format->Rmask,
    back_surface->format->Gmask,
    back_surface->format->Bmask, 0);
  SDL_ShowCursor(SDL_DISABLE);

  psp_sdl_display_splash();

  /* Danzeff Keyboard */
  danzeff_load();
  danzeff_set_screen(back_surface);

  /* Create surface for save state */
  int Index = 0;
  for (Index = 0; Index < ATARI_MAX_SAVE_STATE; Index++) {
    ATARI.atari_save_state[Index].surface =
       SDL_CreateRGBSurface(SDL_SWSURFACE,
                            SNAP_WIDTH, SNAP_HEIGHT,
                            back_surface->format->BitsPerPixel,
                            back_surface->format->Rmask,
                            back_surface->format->Gmask,
                            back_surface->format->Bmask, 0);
  }
  save_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                            SNAP_WIDTH, SNAP_HEIGHT,
                            back_surface->format->BitsPerPixel,
                            back_surface->format->Rmask,
                            back_surface->format->Gmask,
                            back_surface->format->Bmask, 0);

  SDL_Sound_Initialise();
  return 1;
}

void
psp_sdl_exit(int status)
{
  SDL_CloseAudio();
  SDL_Quit();
  sleep(1);

  exit(status);
}

void
psp_sdl_select_font_8x8()
{
  psp_font = psp_font_8x8;
  psp_font_height = 8;
  psp_font_width  = 8;
}

void
psp_sdl_select_font_6x10()
{
  psp_font = psp_font_6x10;
  psp_font_height = 10;
  psp_font_width  = 6;
}

