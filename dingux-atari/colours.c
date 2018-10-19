/*
 * colours.c - Atari colour palette
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2005 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdlib.h>
#include <string.h>  /* for strcmp() */
#include <math.h>
#include "atari.h"
#include "colours.h"
#include "log.h"
#include "util.h"
#include "platform.h"
#ifdef __PLUS
#include "display_win.h"
#include "misc_win.h"
#endif

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define PALETTE_R(x,c) ((UBYTE) (c[x] >> 16))
#define PALETTE_G(x,c) ((UBYTE) (c[x] >> 8))
#define PALETTE_B(x,c) ((UBYTE) c[x])
#define PALETTE_Y(x,c) (0.30 * PALETTE_R(x,c) + 0.59 * PALETTE_G(x,c) + 0.11 * PALETTE_B(x,c))

int *colortable;

static int colortable_ntsc[256];
static int colortable_pal[256];

struct palette_settings *color_settings;

/* NTSC colorburst frequency is exactly 315/88, as some sources say (eg.
 * Wikipedia). The values 167 and 21 are lifted from the GTIA.PDF document
 * and are official. Using these three values, the angle of each color in the
 * YIQ colorspace is calculated. The result appears to reather precisely match
 * the NTSC palettes posted by various users on the net (with hue set to 0.5).
 */
static struct palette_settings color_settings_ntsc = {
  0.0, 1.25, 0.50, 0.50, 1.0, 0.0,
  315.0 / 88.0, 167, 21
};

/* For PAL, I haven't found the exact value of the colorburst frequency,
 * so the approximated value is given. Unfortunately, no official Atari
 * document on PAL GTIA has been found, so the color_delay and color_diff
 * values are chosen by hand (note that the color_delay value is the same
 * as in NTSC). The result, however, seems to perfectly match my 65XE,
 * and also, after contrast/brightness tweaking, looks exactly like the
 * Jakub.act palette.
 */ 
static struct palette_settings color_settings_pal = {
  0.0, 1.0, 0.20, 0.5, 1.0, 0.0,
  4.43361875, 167, 15
};

void Palette_SetRGB(int i, int r, int g, int b, int *colortable_ptr)
{
  if (r < 0)
    r = 0;
  else if (r > 255)
    r = 255;
  if (g < 0)
    g = 0;
  else if (g > 255)
    g = 255;
  if (b < 0)
    b = 0;
  else if (b > 255)
    b = 255;
  colortable_ptr[i] = (r << 16) + (g << 8) + b;
}

void Palette_Generate(int *colortable_ptr, struct palette_settings *colset)
{
  int lm;
  int cr;
  const double pi = acos(-1.0);

  double cycle_length = 1000.0 / colset->colorburst_freq; // ns
  double start_angle = ((double) colset->color_delay / cycle_length * 2 + colset->hue) * pi;
  double diff_angle = (double) colset->color_diff / cycle_length * 2 * pi;

  for (cr = 0; cr < 16; cr ++)
  {
    for (lm = 0; lm < 16; lm ++) {
      float lumafactor = (float)lm / 16;

      float phase = start_angle + diff_angle * (cr - 1);
      float y = pow(lumafactor, colset->gamma) * colset->contrast + colset->brightness;
      float crlv = (cr ? colset->saturation: 0.0f) * (1 - colset->saturation_ramp * lumafactor);
      float i = cos(phase) * crlv;
      float q = sin(phase) * crlv;

      double r = y + 0.9563 * i + 0.6210 * q;
      double g = y - 0.2721 * i - 0.6474 * q;
      double b = y - 1.1070 * i + 1.7046 * q;

      Palette_SetRGB(cr * 16 + lm, r * 255, g * 255, b * 255, colortable_ptr);
    }
  }
}

void Palette_SetVideoSystem(int mode) {
  if (mode == TV_NTSC) {
    colortable = colortable_ntsc;
    color_settings = &color_settings_ntsc;
  }
  else if (mode == TV_PAL) {
    colortable = colortable_pal;
    color_settings = &color_settings_pal;
  }
  else {
    Atari800_Exit(FALSE);
    Aprint("Interal error: Invalid tv_mode\n");
    exit(1);
  }
}

void Palette_Initialise(int *argc, char *argv[])
{
  Palette_Generate(colortable_ntsc, &color_settings_ntsc);
  Palette_Generate(colortable_pal, &color_settings_pal);

  Palette_SetVideoSystem(tv_mode); /* tv_mode is set before calling Palette_Initialise */
}
