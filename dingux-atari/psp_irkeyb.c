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

# ifdef USE_PSP_IRKEYB

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include "psp_sdl.h"
#include "psp_irda.h"
#include "psp_irkeyb.h"

  static int loc_psp_irkeyb_mode = 0;

//#define CONFIG_FILE "ms0:/seplugins/pikey/pspirkeyb.ini"
#define KERNELMODE   0  /* 1 is untested but required for some keyboards to change baudrate */

/*
    PSP_IRKBD_OUTPUT_MODE_ASCII
    PSP_IRKBD_OUTPUT_MODE_RAW
    PSP_IRKBD_OUTPUT_MODE_SCANCODE
    PSP_IRKBD_OUTPUT_MODE_VT100    
  */

int
psp_irkeyb_init()
{
  if (! psp_irda_is_kbd_mode()) return 0;

  loc_psp_irkeyb_mode = 0;

#ifdef PSPFW30X
  /* Load irda PRX for CFW >= 3.80 */
  u32 mod_id = sceKernelLoadModule("flash0:/kd/irda.prx", 0, NULL);
  sceKernelStartModule(mod_id, 0, NULL, NULL, NULL);
#endif

  int ret = pspIrKeybInit( "./pspirkeyb.ini", KERNELMODE );

  if( ret == PSP_IRKBD_RESULT_OK ) {
    pspIrKeybOutputMode( PSP_IRKBD_OUTPUT_MODE_VT100 );
    loc_psp_irkeyb_mode = 1;
    return 0;

  } else {
    switch( ret )
    {
      case PSP_IRKBD_RESULT_CANT_OPEN_DEVICE:
          fprintf(stdout,  "error: can't open device\n" );
          break;
      case PSP_IRKBD_RESULT_CANT_OPEN_MAPFILE:
          fprintf(stdout,  "error: can't open mapfile\n" );
          break;
      case PSP_IRKBD_RESULT_MAPFILE_MAXDEPTHLEVEL:
          fprintf(stdout,  "error: mapfile max include level reached - recursion?\n" );
          break;
      case PSP_IRKBD_RESULT_CANT_OPEN_MAPFILE_INCLUDE:
          fprintf(stdout,  "error: can't open include in mapfile\n" );
          break;
      case PSP_IRKBD_RESULT_CANT_SET_BAUDTATE:
          fprintf(stdout,  "error: can't set baudrate - you need kernel support\n" );
          break; 
      case PSP_IRKBD_RESULT_CONFIG_FILE_NOT_FOUND:
          fprintf(stdout,  "error: can't read config file\n" );
          break; 
      case PSP_IRKBD_RESULT_UNKNOW_KEYBOARD:
          fprintf(stdout,  "error: unknown keyboard\n" );
          break;
      case PSP_IRKBD_RESULT_FAILED:
      default:
          fprintf(stdout,  "error: init failed\n" );
          break;
    }
  }
  return 1;
}

 static u8  loc_irkbd_buffer[ 512 ];
 static int loc_irkbd_length  = 0;
 static int loc_irkbd_current = 0;
 static int loc_irkbd_inc_step = 0;

static int
psp_irkeyb_check_key()
{
  int key = PSP_IRKEYB_EMPTY;
  loc_irkbd_inc_step = 0;
  if (loc_psp_irkeyb_mode) {
    if (loc_irkbd_current >= loc_irkbd_length) {
      loc_irkbd_current = 0;
      loc_irkbd_length  = 0;
      pspIrKeybReadinput(loc_irkbd_buffer, &loc_irkbd_length);
    }
    int delta = loc_irkbd_length - loc_irkbd_current;
    if (delta >= 5) {
      u8* scan_buffer = &loc_irkbd_buffer[loc_irkbd_current];
      if (!strncmp(scan_buffer, "\033[[3~", 5)) {
        key = PSP_IRKEYB_SUPPR;
        loc_irkbd_inc_step = 5;
      } else
      if (!strncmp(scan_buffer, "\033[[2~", 5)) {
        key = PSP_IRKEYB_INSERT;
        loc_irkbd_inc_step = 5;
      } else
      if (!strncmp(scan_buffer, "\033[OH~", 5)) {
        key = PSP_IRKEYB_HOME;
        loc_irkbd_inc_step = 5;
      } else
      if (!strncmp(scan_buffer, "\033[[5~", 5)) {
        key = PSP_IRKEYB_PAGEUP;
        loc_irkbd_inc_step = 5;
      } else
      if (!strncmp(scan_buffer, "\033[[6~", 5)) {
        key = PSP_IRKEYB_PAGEDOWN;
        loc_irkbd_inc_step = 5;
      }
    }
    if (delta >= 4) {
      u8* scan_buffer = &loc_irkbd_buffer[loc_irkbd_current];
      if (!strncmp(scan_buffer, "\033[OF", 4)) {
        key = PSP_IRKEYB_END;
        loc_irkbd_inc_step = 4;
      } else
      if (!strncmp(scan_buffer, "\033[[Z", 4)) {
        key = PSP_IRKEYB_START;
        loc_irkbd_inc_step = 4;
      }
    }
    if (delta >= 3) {
      u8* scan_buffer = &loc_irkbd_buffer[loc_irkbd_current];
      if (!strncmp(scan_buffer, "\033[B", 3)) {
        key = PSP_IRKEYB_DOWN;
        loc_irkbd_inc_step = 3;
      } else 
      if (!strncmp(scan_buffer, "\033[A", 3)) {
        key = PSP_IRKEYB_UP;
        loc_irkbd_inc_step = 3;
      } else 
      if (!strncmp(scan_buffer, "\033[C", 3)) {
        key = PSP_IRKEYB_RIGHT;
        loc_irkbd_inc_step = 3;
      } else
      if (!strncmp(scan_buffer, "\033[D", 3)) {
        key = PSP_IRKEYB_LEFT;
        loc_irkbd_inc_step = 3;
      }
    }
    if (delta >= 1) { 
      if (key == PSP_IRKEYB_EMPTY) {

        key = loc_irkbd_buffer[loc_irkbd_current];
        if ((key == 9) || (key == 23)) key = PSP_IRKEYB_START; //W
        else
        if ((key == 0x1b) || (key == 17)) key = PSP_IRKEYB_SELECT; //Q
        else
        if (key == 5) key = PSP_IRKEYB_TRIANGLE; //E
        else
        if (key == 19) key = PSP_IRKEYB_SQUARE; //S
        else
        if (key == 24) key = PSP_IRKEYB_CROSS; //X
        else
        if (key == 6) key = PSP_IRKEYB_CIRCLE; //F
        else
        if (key == 26) key = PSP_IRKEYB_LTRIGGER; //Z
        else
        if (key == 3) key = PSP_IRKEYB_RTRIGGER; //R

        loc_irkbd_inc_step = 1;
      }
    }
  }
# if 0 //LUDO: DEBUG
  if (key != PSP_IRKEYB_EMPTY) {
    fprintf(stdout, "press %x %c\n", key, key);
  }
# endif
  return key;
}

static void
psp_irkeyb_inc_buffer()
{
  if (loc_irkbd_inc_step) {
    loc_irkbd_current += loc_irkbd_inc_step;
    loc_irkbd_inc_step = 0;
  }
}

static int
loc_psp_irkeyb_read_key()
{
  int psp_irkeyb = psp_irkeyb_check_key();
  if (psp_irkeyb < 0) psp_irkeyb = PSP_IRKEYB_EMPTY;
  else {
    psp_irkeyb_inc_buffer();
  }
  return psp_irkeyb;
}

int
psp_irkeyb_read_key()
{
  if (! psp_irda_is_kbd_mode()) return PSP_IRKEYB_EMPTY;

  int psp_irkeyb = loc_psp_irkeyb_read_key();
  if (psp_irkeyb != PSP_IRKEYB_EMPTY) {
    if ((psp_irkeyb == 0xc2) || (psp_irkeyb == 0xc3)) {
      u8 c1 = psp_irkeyb;
      int new_key = loc_psp_irkeyb_read_key();
      if (new_key != PSP_IRKEYB_EMPTY) {
        u8 c2 = new_key;
        psp_irkeyb = psp_convert_utf8_to_iso_8859_1(c1, c2);
      }
    }
  }
  return psp_irkeyb;
}

int
psp_irkeyb_set_psp_key(SceCtrlData* c)
{
  if (! psp_irda_is_kbd_mode()) return PSP_IRKEYB_EMPTY;

  int prev_inc_step = loc_irkbd_inc_step;
  int irkeyb_key = psp_irkeyb_check_key();
  if (irkeyb_key != PSP_IRKEYB_EMPTY) {

    if (irkeyb_key == PSP_IRKEYB_UP) c->Buttons |= PSP_CTRL_UP;
    else 
    if (irkeyb_key == PSP_IRKEYB_DOWN) c->Buttons |= PSP_CTRL_DOWN;
    else 
    if (irkeyb_key == PSP_IRKEYB_RIGHT) c->Buttons |= PSP_CTRL_RIGHT;
    else 
    if (irkeyb_key == PSP_IRKEYB_LEFT) c->Buttons |= PSP_CTRL_LEFT;
    else
    if (irkeyb_key == PSP_IRKEYB_START) c->Buttons |= PSP_CTRL_START;
    else
    if (irkeyb_key == PSP_IRKEYB_SELECT) c->Buttons |= PSP_CTRL_SELECT;
    else
    if (irkeyb_key == PSP_IRKEYB_TRIANGLE) c->Buttons |= PSP_CTRL_TRIANGLE;
    else
    if (irkeyb_key == PSP_IRKEYB_SQUARE) c->Buttons |= PSP_CTRL_SQUARE;
    else
    if (irkeyb_key == PSP_IRKEYB_CROSS) c->Buttons |= PSP_CTRL_CROSS;
    else
    if (irkeyb_key == PSP_IRKEYB_CIRCLE) c->Buttons |= PSP_CTRL_CIRCLE;
    else
    if (irkeyb_key == PSP_IRKEYB_LTRIGGER) c->Buttons |= PSP_CTRL_LTRIGGER;
    else
    if (irkeyb_key == PSP_IRKEYB_RTRIGGER) c->Buttons |= PSP_CTRL_RTRIGGER;
    else {
       if (prev_inc_step) {
         /* drop it ! */
         loc_irkbd_inc_step = prev_inc_step;
         psp_irkeyb_inc_buffer();
       }
       return irkeyb_key;
    }
    psp_irkeyb_inc_buffer();
  }
  return irkeyb_key;
}

void
psp_irkeyb_exit()
{
  if (psp_irda_is_kbd_mode()) {
    if (loc_psp_irkeyb_mode) {
      loc_psp_irkeyb_mode = 0;
      pspIrKeybFinish();
    }
  }
}

# endif
