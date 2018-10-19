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

/* A Buzz production :) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pspctrl.h>
#include <psptypes.h>
# ifdef USE_IRDA_JOY
#include <pspsircs.h>
#include <pspiofilemgr.h>
#include <pspmodulemgr.h>
# endif
#include <pspthreadman.h>
#include <png.h>

#include "global.h"
#include "psp_joy.h"
#include "psp_irda.h"

#define PSP_IRDA_MODE_NONE  0
#define PSP_IRDA_MODE_KBD   1
#define PSP_IRDA_MODE_JOY   2

  static int psp_irda_mode   = PSP_IRDA_MODE_NONE; 
  static int psp_irda_saved_mode = PSP_IRDA_MODE_NONE;

static int
psp_irda_save_config()
{
  FILE* FileDesc = fopen("psp_irda.cfg", "w");
  if (FileDesc == (FILE*)0) return -1;
  fprintf(FileDesc, "# PSP IRDA Mode\n");
  fprintf(FileDesc, "# 0 - None\n");
  fprintf(FileDesc, "# 1 - Irda keyboard\n");
  fprintf(FileDesc, "# 2 - Irda Joystick / Paddle\n");
  fprintf(FileDesc, "psp_irda_mode=%d\n", psp_irda_saved_mode);
  fclose( FileDesc );
  return 0;
}


int
psp_irda_load_config()
{
  char  Buffer[512];
  char *Scan;
  unsigned int Value;
  FILE* FileDesc;

  psp_irda_mode = PSP_IRDA_MODE_NONE;
  psp_irda_saved_mode = psp_irda_mode;

  FileDesc = fopen("psp_irda.cfg", "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    Scan = strchr(Buffer,'=');
    if (! Scan) continue;

    *Scan = '\0';
    Value = atoi(Scan+1);

    if (!strcasecmp(Buffer,"psp_irda_mode")) psp_irda_mode = Value;
  }
  fclose(FileDesc);

  psp_irda_saved_mode = psp_irda_mode;

  return 0;
}

int
psp_irda_is_kbd_mode()
{
  return psp_irda_mode == PSP_IRDA_MODE_KBD;
}

int
psp_irda_is_joy_mode()
{
  return psp_irda_mode == PSP_IRDA_MODE_JOY;
}

int
psp_irda_get_saved_mode()
{
  return psp_irda_saved_mode;
}

int
psp_irda_set_saved_mode(int mode)
{
  if (mode != psp_irda_saved_mode) {
    psp_irda_saved_mode = mode;
    psp_irda_save_config();
    return 1;
  }
  return 0;
}

# ifdef USE_IRDA_JOY
// States
#define MODE_WAIT_FRAME 0
#define MODE_STORE      1

static int loc_fd = -1;

static int frame_decode (u8 c);
static int frame_analyze ();

// Global variables that handle paddles values and button states
static u16 loc_paddleA;
static u16 loc_paddleB;
static u8  loc_atariJ; // contains all atari joystick bits. Use ATARI_*_MASK to get corresponding info

#define MODE_WAIT_FRAME 0
#define MODE_STORE      1

#define FRAME_SIZE   5

// return true if frame is completed and decoded
static u8 mode=MODE_WAIT_FRAME;
static u8 cnt=0;
static u8 frame_buf[6];

static int 
frame_analyze() 
{
#ifdef USE_IRDA_JOY
  int len = 0;
  u8 c;
  int good_frame = 0;
  do {
    len = sceIoRead(loc_fd, &c, 1); 
    if (len != 1) return good_frame; // drop
    switch (mode) {
      case MODE_WAIT_FRAME: // Waiting start of frame
        if (c == '#') {
          mode=MODE_STORE;
          cnt=0;
        }
      break;
      case MODE_STORE:
        frame_buf[cnt]=c;
        cnt++;
        if (cnt >= FRAME_SIZE - 1) { // End of frame. frame size without '#' (start tag)
          u8 f0 = frame_buf[0];
          u8 f2 = frame_buf[2];
          u16 msbJA = (u16) (f0 & 0x03) << 8;
          u16 msbJB = (u16) (f2 & 0x03) << 8;
          loc_paddleA = msbJA | frame_buf[1];
          loc_paddleA = (0x3ff - loc_paddleA);
          loc_paddleB = msbJB | frame_buf[3];
          loc_paddleB = (0x3ff - loc_paddleB);
# if 0
          if (f0 & 0x04) loc_buttonA = 1;
          else loc_buttonA = 0;
          if (f2 & 0x04) loc_buttonB = 1;
          else loc_buttonB = 0;
# endif
          loc_atariJ = (f0 & 0x7C) >> 2; // bits => 000<UP><DW><LT><RT><BT>
          good_frame = 1;
          cnt = 0;
          mode = MODE_WAIT_FRAME;
        }
        break;
      default:
        cnt = 0;
        mode = MODE_WAIT_FRAME;
      break;
    }
  } while (1);
  return good_frame;
# endif
}

int 
psp_irda_get_joy_event( irda_joy_t* irda_joy_evt )
{
  if (! psp_irda_is_joy_mode()) return 0;

# ifdef USE_IRDA_JOY
  // check for IrDA input
  int status = frame_analyze ();
  if (status != 1) return 0;

  irda_joy_evt->paddleA = loc_paddleA;
  irda_joy_evt->paddleB = loc_paddleB;
  irda_joy_evt->atariJ  = loc_atariJ;
  return 1;
# else
  return 0;
# endif
}

void
psp_irda_joy_convert( irda_joy_t* i_joy, SceCtrlData* psp_c )
{
  if (! psp_irda_is_joy_mode()) return;

  if (ATARI.psp_irdajoy_type == IRDAJOY_TYPE_JOYSTICK) {
    psp_joy_convert_joystick( i_joy, psp_c );
  } else 
  if (ATARI.psp_irdajoy_type == IRDAJOY_TYPE_PADDLE) {
    psp_joy_convert_paddle( i_joy, psp_c );
  } else 
  if (ATARI.psp_irdajoy_type == IRDAJOY_TYPE_DUAL_PADDLE) {
    psp_joy_convert_dual_paddle( i_joy, psp_c );
  }
}

int 
psp_irda_joy_init(void)
{
  if (! psp_irda_is_joy_mode()) return 0;

# ifdef USE_IRDA_JOY
# ifdef PSPFW30X
  /* Load irda PRX for CFW >= 3.80 */
  u32 mod_id = sceKernelLoadModule("flash0:/kd/irda.prx", 0, NULL);
  sceKernelStartModule(mod_id, 0, NULL, NULL, NULL);
# endif
  loc_fd = sceIoOpen("irda0:", PSP_O_RDWR, 0);
# endif //IRDA_JOY
  return 0;
}

char *
psp_irda_get_debug_string()
{
  static char buffer[64];
# if 1
  sprintf(buffer, "%03x %03x %02x", loc_paddleA, loc_paddleB, loc_atariJ );
# else
  sprintf(buffer, "%02x %02x %02x %02x %02x", 
    frame_buf[0], frame_buf[1], frame_buf[2], frame_buf[3], frame_buf[4] );
# endif
  return buffer;
}
# endif

