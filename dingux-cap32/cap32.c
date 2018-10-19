/* Caprice32 - Amstrad CPC Emulator
   (c) Copyright 1997-2005 Ulrich Doewich

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
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <zlib.h>
#include <dirent.h>
#include <assert.h>
#include "SDL.h"
#include "global.h"
#include "psp_sdl.h"
#include "psp_run.h"
#include "psp_fmgr.h"
#include "psp_kbd.h"
#include "psp_danzeff.h"

#include "cpccat.h"
#include "crtc.h"
#include "video.h"
#include "z80.h"
#include "kbd.h"
#include "psp_fmgr.h"

#define ERR_INPUT_INIT           1
#define ERR_VIDEO_INIT           2
#define ERR_VIDEO_SET_MODE       3
#define ERR_VIDEO_SURFACE        4
#define ERR_VIDEO_PALETTE        5
#define ERR_VIDEO_COLOUR_DEPTH   6
#define ERR_AUDIO_INIT           7
#define ERR_AUDIO_RATE           8
#define ERR_OUT_OF_MEMORY        9
#define ERR_CPC_ROM_MISSING      10
#define ERR_NOT_A_CPC_ROM        11
#define ERR_ROM_NOT_FOUND        12
#define ERR_FILE_NOT_FOUND       13
#define ERR_FILE_BAD_ZIP         14
#define ERR_FILE_EMPTY_ZIP       15
#define ERR_FILE_UNZIP_FAILED    16
#define ERR_SNA_INVALID          17
#define ERR_SNA_SIZE             18
#define ERR_SNA_CPC_TYPE         19
#define ERR_SNA_WRITE            20
#define ERR_DSK_INVALID          21
#define ERR_DSK_SIDES            22
#define ERR_DSK_SECTORS          23
#define ERR_DSK_WRITE            24
#define MSG_DSK_ALTERED          25
#define ERR_TAP_INVALID          26
#define ERR_TAP_UNSUPPORTED      27
#define ERR_TAP_BAD_VOC          28
#define ERR_PRINTER              29
#define ERR_SDUMP                31

#define MSG_SNA_LOAD             32
#define MSG_SNA_SAVE             33
#define MSG_DSK_LOAD             34
#define MSG_DSK_SAVE             35
#define MSG_JOY_ENABLE           36
#define MSG_JOY_DISABLE          37
#define MSG_SPD_NORMAL           38
#define MSG_SPD_FULL             39
#define MSG_TAP_INSERT           40
#define MSG_SDUMP_SAVE           41
#define MSG_PAUSED               42
#define MSG_TAP_PLAY             43
#define MSG_TAP_STOP             44

#define DEFAULT_DISK_PATH         1

#define MAX_LINE_LEN 256

SDL_Surface* cpc_surface;

extern byte bTapeLevel;
extern t_z80regs z80;

int psp_screenshot_mode = 0;
int cpc_dsk_system = 0;

byte *pbGPBuffer = NULL;
byte  pbSndBufferIndex = 0;
byte *pbSndBuffer = NULL;
byte *pbSndBufferEnd = NULL;
byte *pbSndBufferArray[2] = { NULL, NULL };
byte *pbSndBufferEndArray[2] = { NULL, NULL };
volatile byte *pbSndStream = NULL;
byte *membank_read[4], *membank_write[4], *memmap_ROM[256];
byte *pbRAM = NULL;
byte *pbROMlo = NULL;
byte *pbROMhi = NULL;
byte *pbExpansionROM = NULL;
byte *pbTapeImage = NULL;
byte *pbTapeImageEnd = NULL;
byte mode0_table[512], mode1_table[1024];
byte keyboard_matrix[16];

static byte *membank_config[8][4];

FILE *pfoPrinter;

dword freq_table[MAX_FREQ_ENTRIES] = {
   11025,
   22050,
   44100,
   48000,
   96000
};

#include "font.c"

static double colours_rgb[32][3] = {
   { 0.5, 0.5, 0.5 }, { 0.5, 0.5, 0.5 },{ 0.0, 1.0, 0.5 }, { 1.0, 1.0, 0.5 },
   { 0.0, 0.0, 0.5 }, { 1.0, 0.0, 0.5 },{ 0.0, 0.5, 0.5 }, { 1.0, 0.5, 0.5 },
   { 1.0, 0.0, 0.5 }, { 1.0, 1.0, 0.5 },{ 1.0, 1.0, 0.0 }, { 1.0, 1.0, 1.0 },
   { 1.0, 0.0, 0.0 }, { 1.0, 0.0, 1.0 },{ 1.0, 0.5, 0.0 }, { 1.0, 0.5, 1.0 },
   { 0.0, 0.0, 0.5 }, { 0.0, 1.0, 0.5 },{ 0.0, 1.0, 0.0 }, { 0.0, 1.0, 1.0 },
   { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 },{ 0.0, 0.5, 0.0 }, { 0.0, 0.5, 1.0 },
   { 0.5, 0.0, 0.5 }, { 0.5, 1.0, 0.5 },{ 0.5, 1.0, 0.0 }, { 0.5, 1.0, 1.0 },
   { 0.5, 0.0, 0.0 }, { 0.5, 0.0, 1.0 },{ 0.5, 0.5, 0.0 }, { 0.5, 0.5, 1.0 }
};

static double colours_green[32] = {
   0.5647, 0.5647, 0.7529, 0.9412,
   0.1882, 0.3765, 0.4706, 0.6588,
   0.3765, 0.9412, 0.9098, 0.9725,
   0.3451, 0.4078, 0.6275, 0.6902,
   0.1882, 0.7529, 0.7216, 0.7843,
   0.1569, 0.2196, 0.4392, 0.5020,
   0.2824, 0.8471, 0.8157, 0.8784,
   0.2510, 0.3137, 0.5333, 0.5961
};

dword colour_table[32];
SDL_Color sdl_colors[32];

static byte CRTC_values[2][14] = {
   {0x3f, 0x28, 0x2e, 0x8e, 0x1f, 0x06, 0x19, 0x1b, 0x00, 0x07, 0x00, 0x00, 0x30, 0x00},
   {0x3f, 0x28, 0x2e, 0x8e, 0x26, 0x00, 0x19, 0x1e, 0x00, 0x07, 0x00, 0x00, 0x30, 0x00}
};


#define MAX_ROM_MODS 2

char chROMSelected[MAX_PATH + 1];
char chROMFile[3][14] = {
   "cpc464.rom",
   "cpc664.rom",
   "cpc6128.rom"
};

t_CPC CPC;
t_CRTC CRTC;
t_FDC FDC;
t_GateArray GateArray;
t_PPI PPI;
t_PSG PSG;
t_VDU VDU;

t_drive driveA;
t_drive driveB;

#define MAX_DISK_FORMAT 8
#define DEFAULT_DISK_FORMAT 0
#define FIRST_CUSTOM_DISK_FORMAT 2
t_disk_format disk_format[MAX_DISK_FORMAT] = {
   { "178K Data Format", 40, 1, 9, 2, 0x52, 0xe5, {{ 0xc1, 0xc6, 0xc2, 0xc7, 0xc3, 0xc8, 0xc4, 0xc9, 0xc5 }} },
   { "169K Vendor Format", 40, 1, 9, 2, 0x52, 0xe5, {{ 0x41, 0x46, 0x42, 0x47, 0x43, 0x48, 0x44, 0x49, 0x45 }} }
};



#define psg_write \
{ \
   byte control = PSG.control & 0xc0; /* isolate PSG control bits */ \
   if (control == 0xc0) { /* latch address? */ \
      PSG.reg_select = psg_data; /* select new PSG register */ \
   } else if (control == 0x80) { /* write? */ \
      if (PSG.reg_select < 16) { /* valid register? */ \
         SetAYRegister(PSG.reg_select, psg_data); \
      } \
   } \
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

void ga_init_banking (void)
{
   byte *romb0, *romb1, *romb2, *romb3, *romb4, *romb5, *romb6, *romb7;
   byte *pbRAMbank;

   romb0 = pbRAM;
   romb1 = pbRAM + 1*16384;
   romb2 = pbRAM + 2*16384;
   romb3 = pbRAM + 3*16384;

   pbRAMbank = pbRAM + ((GateArray.RAM_bank + 1) * 65536);
   romb4 = pbRAMbank;
   romb5 = pbRAMbank + 1*16384;
   romb6 = pbRAMbank + 2*16384;
   romb7 = pbRAMbank + 3*16384;

   membank_config[0][0] = romb0;
   membank_config[0][1] = romb1;
   membank_config[0][2] = romb2;
   membank_config[0][3] = romb3;

   membank_config[1][0] = romb0;
   membank_config[1][1] = romb1;
   membank_config[1][2] = romb2;
   membank_config[1][3] = romb7;

   membank_config[2][0] = romb4;
   membank_config[2][1] = romb5;
   membank_config[2][2] = romb6;
   membank_config[2][3] = romb7;

   membank_config[3][0] = romb0;
   membank_config[3][1] = romb3;
   membank_config[3][2] = romb2;
   membank_config[3][3] = romb7;

   membank_config[4][0] = romb0;
   membank_config[4][1] = romb4;
   membank_config[4][2] = romb2;
   membank_config[4][3] = romb3;

   membank_config[5][0] = romb0;
   membank_config[5][1] = romb5;
   membank_config[5][2] = romb2;
   membank_config[5][3] = romb3;

   membank_config[6][0] = romb0;
   membank_config[6][1] = romb6;
   membank_config[6][2] = romb2;
   membank_config[6][3] = romb3;

   membank_config[7][0] = romb0;
   membank_config[7][1] = romb7;
   membank_config[7][2] = romb2;
   membank_config[7][3] = romb3;
}



void ga_memory_manager (void)
{
  int n;

   dword mem_bank;
   if (CPC.ram_size == 64) { // 64KB of RAM?
      mem_bank = 0; // no expansion memory
      GateArray.RAM_config = 0; // the only valid configuration is 0
   } else {
      mem_bank = (GateArray.RAM_config >> 3) & 7; // extract expansion memory bank
      if (((mem_bank+2)*64) > CPC.ram_size) { // selection is beyond available memory?
         mem_bank = 0; // force default mapping
      }
   }
   if (mem_bank != GateArray.RAM_bank) { // requested bank is different from the active one?
      GateArray.RAM_bank = mem_bank;
      ga_init_banking();
   }
   for (n = 0; n < 4; n++) { // remap active memory banks
      membank_read[n] = membank_config[GateArray.RAM_config & 7][n];
      membank_write[n] = membank_config[GateArray.RAM_config & 7][n];
   }
   if (!(GateArray.ROM_config & 0x04)) { // lower ROM is enabled?
      membank_read[0] = pbROMlo; // 'page in' lower ROM
   }
   if (!(GateArray.ROM_config & 0x08)) { // upper/expansion ROM is enabled?
      membank_read[3] = pbExpansionROM; // 'page in' upper/expansion ROM
   }
}



byte z80_IN_handler (reg_pair port)
{
// CRTC -----------------------------------------------------------------------
   if (!(port.b.h & 0x40)) { // CRTC chip select?
      if ((port.b.h & 3) == 3) { // read CRTC register?
         if ((CRTC.reg_select > 11) && (CRTC.reg_select < 18)) { // valid range?
            return CRTC.registers[CRTC.reg_select];
         }
         return 0; // write only registers return 0
      }
   }
// PPI ------------------------------------------------------------------------
   else if (!(port.b.h & 0x08)) { // PPI chip select?
      byte ppi_port = port.b.h & 3;
      switch (ppi_port) {
         case 0: // read from port A?
            if (PPI.control & 0x10) { // port A set to input?
               if ((PSG.control & 0xc0) == 0x40) { // PSG control set to read?
                  if (PSG.reg_select < 16) { // within valid range?
                     if (PSG.reg_select == 14) { // PSG port A?
                        if (!(PSG.RegisterAY.Index[7] & 0x40)) { // port A in input mode?
                           return keyboard_matrix[CPC.keyboard_line & 0x0f]; // read keyboard matrix node status
                        } else {
                           return PSG.RegisterAY.Index[14] & (keyboard_matrix[CPC.keyboard_line & 0x0f]); // return last value w/ logic AND of input
                        }
                     } else if (PSG.reg_select == 15) { // PSG port B?
                        if ((PSG.RegisterAY.Index[7] & 0x80)) { // port B in output mode?
                           return PSG.RegisterAY.Index[15]; // return stored value
                        }
                     } else {
                        return PSG.RegisterAY.Index[PSG.reg_select]; // read PSG register
                     }
                  }
               }
            }
            return PPI.portA; // return last programmed value

         case 1: // read from port B?
            if (PPI.control & 2) { // port B set to input?
               return (0x40) | // ready line of connected printer
                      (CPC_jumpers & 0x7f) | // manufacturer + 50Hz
                      (CRTC.flags & VS_flag); // VSYNC status
            }
            return PPI.portB; // return last programmed value

         case 2: // read from port C?
          {
            byte direction = PPI.control & 9; // isolate port C directions
            byte ret_val = PPI.portC; // default to last programmed value
            if (direction) { // either half set to input?
               if (direction & 8) { // upper half set to input?
                  ret_val &= 0x0f; // blank out upper half
                  byte val = PPI.portC & 0xc0; // isolate PSG control bits
                  if (val == 0xc0) { // PSG specify register?
                     val = 0x80; // change to PSG write register
                  }
                  ret_val |= val | 0x20; // casette write data is always set
               }
               if (!(direction & 1)) { // lower half set to output?
                  ret_val |= 0x0f; // invalid - set all bits
               }
            }
            return ret_val;
          }
      }
   }
// ----------------------------------------------------------------------------
   else if (!(port.b.h & 0x04)) { // external peripheral?
      if ((port.b.h == 0xfb) && (!(port.b.l & 0x80))) { // FDC?
         if (!(port.b.l & 0x01)) { // FDC status register?
            return fdc_read_status();
         } else { // FDC data register
            return fdc_read_data();
         }
      }
   }
   return 0xff;
}



void z80_OUT_handler (reg_pair port, byte val)
{
// Gate Array -----------------------------------------------------------------
   if ((port.b.h & 0xc0) == 0x40) { // GA chip select?
      switch (val >> 6) {
         case 0: // select pen
            GateArray.pen = val & 0x10 ? 0x10 : val & 0x0f; // if bit 5 is set, pen indexes the border colour
            break;
         case 1: // set colour
            {
               byte colour = val & 0x1f; // isolate colour value
               GateArray.ink_values[GateArray.pen] = colour;
               GateArray.palette[GateArray.pen] = colour_table[colour];
            }
            break;
         case 2: // set mode
            GateArray.ROM_config = val;
            GateArray.requested_scr_mode = val & 0x03; // request a new CPC screen mode
            ga_memory_manager();
            if (val & 0x10) { // delay Z80 interrupt?
               z80.int_pending = 0; // clear pending interrupts
               GateArray.sl_count = 0; // reset GA scanline counter
            }
            break;
         case 3: // set memory configuration
            GateArray.RAM_config = val;
            ga_memory_manager();
            break;
      }
   }
// CRTC -----------------------------------------------------------------------
   if (!(port.b.h & 0x40)) { // CRTC chip select?
      byte crtc_port = port.b.h & 3;
      if (crtc_port == 0) { // CRTC register select?
         CRTC.reg_select = val;
      }
      else if (crtc_port == 1) { // CRTC write data?
         if (CRTC.reg_select < 16) { // only registers 0 - 15 can be written to
            CRTC.registers[CRTC.reg_select] = val;
            switch (CRTC.reg_select) {
               case 3: // sync width
                  CRTC.hsw = val & 0x0f; // isolate horizontal sync width
                  VDU.hsw = CRTC.hsw - 2; // GA delays HSYNC by 2 chars
                  if (VDU.hsw < 0) { // negative value?
                     VDU.hsw = 0; // no VDU HSYNC
                  }
                  else if (VDU.hsw > 4) { // HSYNC longer than 4 chars?
                     VDU.hsw = 4; // maxium of 4
                  }
                  CRTC.vsw = val >> 4; // isolate vertical sync width
                  if (!CRTC.vsw) {
                     CRTC.vsw = 16; // 0 = width of 16
                  }
                  break;
               case 5: // vertical total adjust
                  CRTC.vt_adjust = val & 0x1f;
                  break;
               case 8: // interlace and skew
                  CRTC.skew = (val >> 4) & 3; // isolate display timing skew
                  if (CRTC.skew == 3) { // no output?
                     CRTC.skew = 0xff;
                  }
                  break;
               case 9: // maximum raster count
                  CRTC.max_raster = val << 3; // modify value for easier handling
                  break;
               case 12: // start address high byte
               case 13: // start address low byte
                  {
                     dword val1 = CRTC.registers[12] & 0x3f;
                     dword val2 = val1 & 0x0f; // isolate screen size
                     val1 = (val1 << 1) & 0x60; // isolate CPC RAM bank
                     val2 |= val1; // combine
                     CRTC.requested_addr = (CRTC.registers[13] + (val2 << 8)) << 1;
                  }
                  break;
            }
         }
      }
   }
// ROM select -----------------------------------------------------------------
   if (!(port.b.h & 0x20)) { // ROM select?
      GateArray.upper_ROM = val;
      pbExpansionROM = memmap_ROM[val];
      if (pbExpansionROM == NULL) { // selected expansion ROM not present?
         pbExpansionROM = pbROMhi; // revert to BASIC ROM
      }
      if (!(GateArray.ROM_config & 0x08)) { // upper/expansion ROM is enabled?
         membank_read[3] = pbExpansionROM; // 'page in' upper/expansion ROM
      }
   }
// PPI ------------------------------------------------------------------------
   if (!(port.b.h & 0x08)) { // PPI chip select?
      switch (port.b.h & 3) {
         case 0: // write to port A?
            PPI.portA = val;
            if (!(PPI.control & 0x10)) { // port A set to output?
               byte psg_data = val;
               psg_write
            }
            break;
         case 1: // write to port B?
            PPI.portB = val;
            break;
         case 2: // write to port C?
            PPI.portC = val;
            if (!(PPI.control & 1)) { // output lower half?
               CPC.keyboard_line = val;
            }
            if (!(PPI.control & 8)) { // output upper half?
               PSG.control = val; // change PSG control
               byte psg_data = PPI.portA;
               psg_write
            }
            break;
         case 3: // modify PPI control
            if (val & 0x80) { // change PPI configuration
               PPI.control = val; // update control byte
               PPI.portA = 0; // clear data for all ports
               PPI.portB = 0;
               PPI.portC = 0;
            } else { // bit manipulation of port C data
               if (val & 1) { // set bit?
                  byte bit = (val >> 1) & 7; // isolate bit to set
                  PPI.portC |= bit_values[bit]; // set requested bit
                  if (!(PPI.control & 1)) { // output lower half?
                     CPC.keyboard_line = PPI.portC;
                  }
                  if (!(PPI.control & 8)) { // output upper half?
                     PSG.control = PPI.portC; // change PSG control
                     byte psg_data = PPI.portA;
                     psg_write
                  }
               } else {
                  byte bit = (val >> 1) & 7; // isolate bit to reset
                  PPI.portC &= ~(bit_values[bit]); // reset requested bit
                  if (!(PPI.control & 1)) { // output lower half?
                     CPC.keyboard_line = PPI.portC;
                  }
                  if (!(PPI.control & 8)) { // output upper half?
                     PSG.control = PPI.portC; // change PSG control
                     byte psg_data = PPI.portA;
                     psg_write
                  }
               }
            }
            break;
      }
   }
// ----------------------------------------------------------------------------
   if ((port.b.h == 0xfa) && (!(port.b.l & 0x80))) { // floppy motor control?
      FDC.motor = val & 0x01;
      FDC.flags |= STATUSDRVA_flag | STATUSDRVB_flag;
   }
   else if ((port.b.h == 0xfb) && (!(port.b.l & 0x80))) { // FDC data register?
      fdc_write_data(val);
   }
}

void
update_save_name(char *Name)
{
  char        TmpFileName[MAX_PATH];
  struct stat aStat;
  int         index;
  char       *SaveName;
  char       *Scan1;
  char       *Scan2;

  SaveName = strrchr(Name,'/');
  if (SaveName != (char *)0) SaveName++;
  else                       SaveName = Name;

  if (!strncasecmp(SaveName, "sav_", 4)) {
    Scan1 = SaveName + 4;
    Scan2 = strrchr(Scan1, '_');
    if (Scan2 && (Scan2[1] >= '0') && (Scan2[1] <= '5')) {
      strncpy(CPC.cpc_save_name, Scan1, MAX_PATH);
      CPC.cpc_save_name[Scan2 - Scan1] = '\0';
    } else {
      strncpy(CPC.cpc_save_name, SaveName, MAX_PATH);
    }
  } else {
    strncpy(CPC.cpc_save_name, SaveName, MAX_PATH);
  }

  if (CPC.cpc_save_name[0] == '\0') {
    strcpy(CPC.cpc_save_name,"default");
  }

  for (index = 0; index < CPC_MAX_SAVE_STATE; index++) {
    CPC.cpc_save_state[index].used  = 0;
    memset(&CPC.cpc_save_state[index].date, 0, sizeof(time_t));
    CPC.cpc_save_state[index].thumb = 0;

    snprintf(TmpFileName, MAX_PATH, "%s/sav_%s_%d.snz", CPC.cpc_save_path, CPC.cpc_save_name, index);
    if (! stat(TmpFileName, &aStat)) {
      CPC.cpc_save_state[index].used = 1;
      CPC.cpc_save_state[index].date = aStat.st_mtime;
      snprintf(TmpFileName, MAX_PATH, "%s/sav_%s_%d.png", CPC.cpc_save_path, CPC.cpc_save_name, index);
      if (! stat(TmpFileName, &aStat)) {
        if (psp_sdl_load_thumb_png(CPC.cpc_save_state[index].surface, TmpFileName)) {
          CPC.cpc_save_state[index].thumb = 1;
        }
      }
    }
  }

  CPC.comment_present = 0;
  snprintf(TmpFileName, MAX_PATH, "%s/txt/%s.txt", CPC.cpc_home_dir, CPC.cpc_save_name);
  if (! stat(TmpFileName, &aStat)) {
    CPC.comment_present = 1;
  }
}

void
reset_save_name()
{
  if (! driveA.tracks) update_save_name("");
}

typedef struct thumb_list {
  struct thumb_list *next;
  char              *name;
  char              *thumb;
} thumb_list;

static thumb_list* loc_head_thumb = 0;

static void
loc_del_thumb_list()
{
  while (loc_head_thumb != 0) {
    thumb_list *del_elem = loc_head_thumb;
    loc_head_thumb = loc_head_thumb->next;
    if (del_elem->name) free( del_elem->name );
    if (del_elem->thumb) free( del_elem->thumb );
    free(del_elem);
  }
}

static void
loc_add_thumb_list(char* filename)
{
  thumb_list *new_elem;
  char tmp_filename[MAX_PATH];

  strcpy(tmp_filename, filename);
  char* save_name = tmp_filename;

  /* .png extention */
  char* Scan = strrchr(save_name, '.');
  if ((! Scan) || (strcasecmp(Scan, ".png"))) return;
  *Scan = 0;

  if (strncasecmp(save_name, "sav_", 4)) return;
  save_name += 4;

  Scan = strrchr(save_name, '_');
  if (! Scan) return;
  *Scan = 0;

  /* only one png for a give save name */
  new_elem = loc_head_thumb;
  while (new_elem != 0) {
    if (! strcasecmp(new_elem->name, save_name)) return;
    new_elem = new_elem->next;
  }

  new_elem = (thumb_list *)malloc( sizeof( thumb_list ) );
  new_elem->next = loc_head_thumb;
  loc_head_thumb = new_elem;
  new_elem->name  = strdup( save_name );
  new_elem->thumb = strdup( filename );
}

void
load_thumb_list()
{
  char SaveDirName[MAX_PATH];
  DIR* fd = 0;

  loc_del_thumb_list();

  strncpy( SaveDirName, CPC.cpc_save_path, sizeof(SaveDirName));

  fd = opendir(SaveDirName);
  if (!fd) return;

  struct dirent *a_dirent;
  while ((a_dirent = readdir(fd)) != 0) {
    if(a_dirent->d_name[0] == '.') continue;
    if (a_dirent->d_type != DT_DIR)
    {
      loc_add_thumb_list( a_dirent->d_name );
    }
  }
  closedir(fd);
}

int
load_thumb_if_exists(char *Name)
{
  char        FileName[MAX_PATH];
  char        ThumbFileName[MAX_PATH];
  struct stat aStat;
  char       *SaveName;
  char       *Scan;

  strcpy(FileName, Name);
  SaveName = strrchr(FileName,'/');
  if (SaveName != (char *)0) SaveName++;
  else                       SaveName = FileName;

  Scan = strrchr(SaveName,'.');
  if (Scan) *Scan = '\0';

  if (!SaveName[0]) return 0;

  thumb_list *scan_list = loc_head_thumb;
  while (scan_list != 0) {
    if (! strcasecmp( SaveName, scan_list->name)) {
      snprintf(ThumbFileName, MAX_PATH, "%s/save/%s", CPC.cpc_home_dir, scan_list->thumb);
      if (! stat(ThumbFileName, &aStat))
      {
        if (psp_sdl_load_thumb_png(save_surface, ThumbFileName)) {
          return 1;
        }
      }
    }
    scan_list = scan_list->next;
  }
  return 0;
}

typedef struct comment_list {
  struct comment_list *next;
  char              *name;
  char              *filename;
} comment_list;

static comment_list* loc_head_comment = 0;

static void
loc_del_comment_list()
{
  while (loc_head_comment != 0) {
    comment_list *del_elem = loc_head_comment;
    loc_head_comment = loc_head_comment->next;
    if (del_elem->name) free( del_elem->name );
    if (del_elem->filename) free( del_elem->filename );
    free(del_elem);
  }
}

static void
loc_add_comment_list(char* filename)
{
  comment_list *new_elem;
  char  tmp_filename[MAX_PATH];

  strcpy(tmp_filename, filename);
  char* save_name = tmp_filename;

  /* .png extention */
  char* Scan = strrchr(save_name, '.');
  if ((! Scan) || (strcasecmp(Scan, ".txt"))) return;
  *Scan = 0;

  /* only one txt for a given save name */
  new_elem = loc_head_comment;
  while (new_elem != 0) {
    if (! strcasecmp(new_elem->name, save_name)) return;
    new_elem = new_elem->next;
  }

  new_elem = (comment_list *)malloc( sizeof( comment_list ) );
  new_elem->next = loc_head_comment;
  loc_head_comment = new_elem;
  new_elem->name  = strdup( save_name );
  new_elem->filename = strdup( filename );
}

void
load_comment_list()
{
  char SaveDirName[MAX_PATH];
  DIR* fd = 0;

  loc_del_comment_list();

  snprintf(SaveDirName, MAX_PATH, "%s/txt", CPC.cpc_home_dir);

  fd = opendir(SaveDirName);
  if (!fd) return;

  struct dirent *a_dirent;
  while ((a_dirent = readdir(fd)) != 0) {
    if(a_dirent->d_name[0] == '.') continue;
    if (a_dirent->d_type != DT_DIR)
    {
      loc_add_comment_list( a_dirent->d_name );
    }
  }
  closedir(fd);
}

char*
load_comment_if_exists(char *Name)
{
static char loc_comment_buffer[128];

  char        FileName[MAX_PATH];
  char        TmpFileName[MAX_PATH];
  FILE       *a_file;
  char       *SaveName;
  char       *Scan;

  loc_comment_buffer[0] = 0;

  strcpy(FileName, Name);
  SaveName = strrchr(FileName,'/');
  if (SaveName != (char *)0) SaveName++;
  else                       SaveName = FileName;

  Scan = strrchr(SaveName,'.');
  if (Scan) *Scan = '\0';

  if (!SaveName[0]) return 0;

  comment_list *scan_list = loc_head_comment;
  while (scan_list != 0) {
    if (! strcasecmp( SaveName, scan_list->name)) {
      snprintf(TmpFileName, MAX_PATH, "%s/txt/%s", CPC.cpc_home_dir, scan_list->filename);
      a_file = fopen(TmpFileName, "r");
      if (a_file) {
        char* a_scan = 0;
        loc_comment_buffer[0] = 0;
        if (fgets(loc_comment_buffer, 60, a_file) != 0) {
          a_scan = strchr(loc_comment_buffer, '\n');
          if (a_scan) *a_scan = '\0';
          /* For this #@$% of windows ! */
          a_scan = strchr(loc_comment_buffer,'\r');
          if (a_scan) *a_scan = '\0';
          fclose(a_file);
          return loc_comment_buffer;
        }
        fclose(a_file);
        return 0;
      }
    }
    scan_list = scan_list->next;
  }
  return 0;
}

static int
snapshot_load_sna(char *FileName)
{
  FILE* a_file = 0;
  byte *pbTemp;
  int n;
  dword dwSnapSize, dwModel;
  char chPath[MAX_PATH + 1];
  byte val;
  reg_pair port;
  t_SNA_header sh;

   memset(&sh, 0, sizeof(sh));
   if ((a_file = fopen(FileName, "rb")) != NULL) {
      fread(&sh, sizeof(sh), 1, a_file); // read snapshot header
      if (memcmp(sh.id, "MV - SNA", 8) != 0) { // valid SNApshot image?
         fclose(a_file);
         return ERR_SNA_INVALID;
      }
      dwSnapSize = sh.ram_size[0] + (sh.ram_size[1] * 256); // memory dump size
      dwSnapSize &= ~0x3f; // limit to multiples of 64
      if (!dwSnapSize) {
         fclose(a_file);
         return ERR_SNA_SIZE;
      }
      if (dwSnapSize > CPC.ram_size) { // memory dump size differs from current RAM size?

         pbTemp = (byte *)malloc( sizeof(byte) * dwSnapSize*1024);
         if (pbTemp) {
            free( pbRAM );
            CPC.ram_size = dwSnapSize;
            pbRAM = pbTemp;
         } else {
            fclose(a_file);
            return ERR_OUT_OF_MEMORY;
         }
      }
      emulator_reset(false);
      n = fread(pbRAM, dwSnapSize*1024, 1, a_file); // read memory dump into CPC RAM
      fclose(a_file);
      if (!n) {
         emulator_reset(false);
         return ERR_SNA_INVALID;
      }

// Z80
      _A = sh.AF[1];
      _F = sh.AF[0];
      _B = sh.BC[1];
      _C = sh.BC[0];
      _D = sh.DE[1];
      _E = sh.DE[0];
      _H = sh.HL[1];
      _L = sh.HL[0];
      _R = sh.R & 0x7f;
      _Rb7 = sh.R & 0x80; // bit 7 of R
      _I = sh.I;
      if (sh.IFF0)
         _IFF1 = Pflag;
      if (sh.IFF1)
         _IFF2 = Pflag;
      _IXh = sh.IX[1];
      _IXl = sh.IX[0];
      _IYh = sh.IY[1];
      _IYl = sh.IY[0];
      z80.SP.b.h = sh.SP[1];
      z80.SP.b.l = sh.SP[0];
      z80.PC.b.h = sh.PC[1];
      z80.PC.b.l = sh.PC[0];
      _IM = sh.IM; // interrupt mode
      z80.AFx.b.h = sh.AFx[1];
      z80.AFx.b.l = sh.AFx[0];
      z80.BCx.b.h = sh.BCx[1];
      z80.BCx.b.l = sh.BCx[0];
      z80.DEx.b.h = sh.DEx[1];
      z80.DEx.b.l = sh.DEx[0];
      z80.HLx.b.h = sh.HLx[1];
      z80.HLx.b.l = sh.HLx[0];
// Gate Array
      port.b.h = 0x7f;
      for (n = 0; n < 17; n++) { // loop for all colours + border
         GateArray.pen = n;
         val = sh.ga_ink_values[n]; // GA palette entry
         z80_OUT_handler(port, val | (1<<6));
      }
      val = sh.ga_pen; // GA pen
      z80_OUT_handler(port, (val & 0x3f));
      val = sh.ga_ROM_config; // GA ROM configuration
      z80_OUT_handler(port, (val & 0x3f) | (2<<6));
      val = sh.ga_RAM_config; // GA RAM configuration
      z80_OUT_handler(port, (val & 0x3f) | (3<<6));
// CRTC
      port.b.h = 0xbd;
      for (n = 0; n < 18; n++) { // loop for all CRTC registers
         val = sh.crtc_registers[n];
         CRTC.reg_select = n;
         z80_OUT_handler(port, val);
      }
      port.b.h = 0xbc;
      val = sh.crtc_reg_select; // CRTC register select
      z80_OUT_handler(port, val);
// ROM select
      port.b.h = 0xdf;
      val = sh.upper_ROM; // upper ROM number
      z80_OUT_handler(port, val);
// PPI
      port.b.h = 0xf4; // port A
      z80_OUT_handler(port, sh.ppi_A);
      port.b.h = 0xf5; // port B
      z80_OUT_handler(port, sh.ppi_B);
      port.b.h = 0xf6; // port C
      z80_OUT_handler(port, sh.ppi_C);
      port.b.h = 0xf7; // control
      z80_OUT_handler(port, sh.ppi_control);
// PSG
      PSG.control = PPI.portC;
      PSG.reg_select = sh.psg_reg_select;
      for (n = 0; n < 16; n++) { // loop for all PSG registers
         SetAYRegister(n, sh.psg_registers[n]);
      }

      if (sh.version > 1) { // does the snapshot have version 2 data?
         dwModel = sh.cpc_model; // determine the model it was saved for
         if (dwModel != CPC.model) { // different from what we're currently running?
            if (dwModel > 2) { // not one of the known models?
               emulator_reset(false);
               return ERR_SNA_CPC_TYPE;
            }
            strncpy(chPath, CPC.cpc_bios_path, sizeof(chPath)-2);
            strcat(chPath, "/");
            strncat(chPath, chROMFile[dwModel], sizeof(chPath)-1 - strlen(chPath)); // path to the required ROM image
            if ((a_file = fopen(chPath, "rb")) != NULL) {
               n = fread(pbROMlo, 2*16384, 1, a_file);
               fclose(a_file);
               if (!n) {
                  emulator_reset(false);
                  return ERR_CPC_ROM_MISSING;
               }
               CPC.model = dwModel;
            } else { // ROM image load failed
               emulator_reset(false);
               return ERR_CPC_ROM_MISSING;
            }
         }
      }
      if (sh.version > 2) { // does the snapshot have version 3 data?
         FDC.motor = sh.fdc_motor;
         driveA.current_track = sh.drvA_current_track;
         driveB.current_track = sh.drvB_current_track;
         PSG.AmplitudeEnv = sh.psg_env_step << 1; // multiply by 2 to bring it into the 0 - 30 range
         PSG.FirstPeriod = false;
         if (sh.psg_env_direction == 0x01) { // up
            switch (PSG.RegisterAY.EnvType)
            {
               case 4:
               case 5:
               case 6:
               case 7:
               case 13:
               case 14:
               case 15:
                  PSG.FirstPeriod = true;
                  break;
            }
         } else if (sh.psg_env_direction == 0xff) { // down
            switch (PSG.RegisterAY.EnvType)
            {
               case 0:
               case 1:
               case 2:
               case 3:
               case 9:
               case 10:
               case 11:
                  PSG.FirstPeriod = true;
                  break;
            }
         }
         CRTC.addr = sh.crtc_addr[0] + (sh.crtc_addr[1] * 256);
         VDU.scanline = sh.crtc_scanline[0] + (sh.crtc_scanline[1] * 256);
         CRTC.char_count = sh.crtc_char_count[0];
         CRTC.line_count = sh.crtc_line_count & 127;
         CRTC.raster_count = sh.crtc_raster_count & 31;
         CRTC.hsw_count = sh.crtc_hsw_count & 15;
         CRTC.vsw_count = sh.crtc_vsw_count & 15;
         CRTC.flags = sh.crtc_flags[0] + (sh.crtc_flags[1] * 256);
         GateArray.int_delay = sh.ga_int_delay;
         GateArray.sl_count = sh.ga_sl_count;
         z80.int_pending = sh.z80_int_pending;
      }
   } else {
      return ERR_FILE_NOT_FOUND;
   }
   return 0;
}

static int
snapshot_load_sna_buffer(char *zip_buffer, int zip_size)
{
  FILE* a_file = 0;
  byte *pbTemp;
  int n;
  dword dwSnapSize, dwModel;
  char chPath[MAX_PATH + 1];
  byte val;
  reg_pair port;
  t_SNA_header sh;
  char* scan_buffer = zip_buffer;
  int   scan_size = zip_size;

  memset(&sh, 0, sizeof(sh));
  if (zip_size < sizeof(sh)) {
    return ERR_SNA_INVALID;
  }
  memcpy(&sh, scan_buffer, sizeof(sh));
  scan_buffer += sizeof(sh);
  scan_size -= sizeof(sh);
  {
    if (memcmp(sh.id, "MV - SNA", 8) != 0) { // valid SNApshot image?
     return ERR_SNA_INVALID;
    }
      dwSnapSize = sh.ram_size[0] + (sh.ram_size[1] * 256); // memory dump size
      dwSnapSize &= ~0x3f; // limit to multiples of 64
      if (!dwSnapSize) {
         return ERR_SNA_SIZE;
      }
      if (dwSnapSize > CPC.ram_size) { // memory dump size differs from current RAM size?

         pbTemp = (byte *)malloc( sizeof(byte) * dwSnapSize*1024);
         if (pbTemp) {
            free( pbRAM );
            CPC.ram_size = dwSnapSize;
            pbRAM = pbTemp;
         } else {
            fclose(a_file);
            return ERR_OUT_OF_MEMORY;
         }
      }
      emulator_reset(false);
      if (scan_size < dwSnapSize*1024) {
         return ERR_SNA_INVALID;
      }
      memcpy(pbRAM, scan_buffer, dwSnapSize*1024);
      scan_size -= dwSnapSize*1024;

// Z80
      _A = sh.AF[1];
      _F = sh.AF[0];
      _B = sh.BC[1];
      _C = sh.BC[0];
      _D = sh.DE[1];
      _E = sh.DE[0];
      _H = sh.HL[1];
      _L = sh.HL[0];
      _R = sh.R & 0x7f;
      _Rb7 = sh.R & 0x80; // bit 7 of R
      _I = sh.I;
      if (sh.IFF0)
         _IFF1 = Pflag;
      if (sh.IFF1)
         _IFF2 = Pflag;
      _IXh = sh.IX[1];
      _IXl = sh.IX[0];
      _IYh = sh.IY[1];
      _IYl = sh.IY[0];
      z80.SP.b.h = sh.SP[1];
      z80.SP.b.l = sh.SP[0];
      z80.PC.b.h = sh.PC[1];
      z80.PC.b.l = sh.PC[0];
      _IM = sh.IM; // interrupt mode
      z80.AFx.b.h = sh.AFx[1];
      z80.AFx.b.l = sh.AFx[0];
      z80.BCx.b.h = sh.BCx[1];
      z80.BCx.b.l = sh.BCx[0];
      z80.DEx.b.h = sh.DEx[1];
      z80.DEx.b.l = sh.DEx[0];
      z80.HLx.b.h = sh.HLx[1];
      z80.HLx.b.l = sh.HLx[0];
// Gate Array
      port.b.h = 0x7f;
      for (n = 0; n < 17; n++) { // loop for all colours + border
         GateArray.pen = n;
         val = sh.ga_ink_values[n]; // GA palette entry
         z80_OUT_handler(port, val | (1<<6));
      }
      val = sh.ga_pen; // GA pen
      z80_OUT_handler(port, (val & 0x3f));
      val = sh.ga_ROM_config; // GA ROM configuration
      z80_OUT_handler(port, (val & 0x3f) | (2<<6));
      val = sh.ga_RAM_config; // GA RAM configuration
      z80_OUT_handler(port, (val & 0x3f) | (3<<6));
// CRTC
      port.b.h = 0xbd;
      for (n = 0; n < 18; n++) { // loop for all CRTC registers
         val = sh.crtc_registers[n];
         CRTC.reg_select = n;
         z80_OUT_handler(port, val);
      }
      port.b.h = 0xbc;
      val = sh.crtc_reg_select; // CRTC register select
      z80_OUT_handler(port, val);
// ROM select
      port.b.h = 0xdf;
      val = sh.upper_ROM; // upper ROM number
      z80_OUT_handler(port, val);
// PPI
      port.b.h = 0xf4; // port A
      z80_OUT_handler(port, sh.ppi_A);
      port.b.h = 0xf5; // port B
      z80_OUT_handler(port, sh.ppi_B);
      port.b.h = 0xf6; // port C
      z80_OUT_handler(port, sh.ppi_C);
      port.b.h = 0xf7; // control
      z80_OUT_handler(port, sh.ppi_control);
// PSG
      PSG.control = PPI.portC;
      PSG.reg_select = sh.psg_reg_select;
      for (n = 0; n < 16; n++) { // loop for all PSG registers
         SetAYRegister(n, sh.psg_registers[n]);
      }

      if (sh.version > 1) { // does the snapshot have version 2 data?
         dwModel = sh.cpc_model; // determine the model it was saved for
         if (dwModel != CPC.model) { // different from what we're currently running?
            if (dwModel > 2) { // not one of the known models?
               emulator_reset(false);
               return ERR_SNA_CPC_TYPE;
            }
            strncpy(chPath, CPC.cpc_bios_path, sizeof(chPath)-2);
            strcat(chPath, "/");
            strncat(chPath, chROMFile[dwModel], sizeof(chPath)-1 - strlen(chPath)); // path to the required ROM image
            if ((a_file = fopen(chPath, "rb")) != NULL) {
               n = fread(pbROMlo, 2*16384, 1, a_file);
               fclose(a_file);
               if (!n) {
                  emulator_reset(false);
                  return ERR_CPC_ROM_MISSING;
               }
               CPC.model = dwModel;
            } else { // ROM image load failed
               emulator_reset(false);
               return ERR_CPC_ROM_MISSING;
            }
         }
      }
      if (sh.version > 2) { // does the snapshot have version 3 data?
         FDC.motor = sh.fdc_motor;
         driveA.current_track = sh.drvA_current_track;
         driveB.current_track = sh.drvB_current_track;
         PSG.AmplitudeEnv = sh.psg_env_step << 1; // multiply by 2 to bring it into the 0 - 30 range
         PSG.FirstPeriod = false;
         if (sh.psg_env_direction == 0x01) { // up
            switch (PSG.RegisterAY.EnvType)
            {
               case 4:
               case 5:
               case 6:
               case 7:
               case 13:
               case 14:
               case 15:
                  PSG.FirstPeriod = true;
                  break;
            }
         } else if (sh.psg_env_direction == 0xff) { // down
            switch (PSG.RegisterAY.EnvType)
            {
               case 0:
               case 1:
               case 2:
               case 3:
               case 9:
               case 10:
               case 11:
                  PSG.FirstPeriod = true;
                  break;
            }
         }
         CRTC.addr = sh.crtc_addr[0] + (sh.crtc_addr[1] * 256);
         VDU.scanline = sh.crtc_scanline[0] + (sh.crtc_scanline[1] * 256);
         CRTC.char_count = sh.crtc_char_count[0];
         CRTC.line_count = sh.crtc_line_count & 127;
         CRTC.raster_count = sh.crtc_raster_count & 31;
         CRTC.hsw_count = sh.crtc_hsw_count & 15;
         CRTC.vsw_count = sh.crtc_vsw_count & 15;
         CRTC.flags = sh.crtc_flags[0] + (sh.crtc_flags[1] * 256);
         GateArray.int_delay = sh.ga_int_delay;
         GateArray.sl_count = sh.ga_sl_count;
         z80.int_pending = sh.z80_int_pending;
      }
  }
  return 0;
}

static int
snapshot_load_snz(char *FileName)
{
  gzFile a_file;
  byte *pbTemp;
  int n;
  dword dwSnapSize, dwModel;
  char chPath[MAX_PATH + 1];
  byte val;
  reg_pair port;
  t_SNA_header sh;

   memset(&sh, 0, sizeof(sh));
   if ((a_file = gzopen(FileName, "rb")) != NULL) {
      gzread(a_file, &sh, sizeof(sh)); // read snapshot header
      if (memcmp(sh.id, "MV - SNA", 8) != 0) { // valid SNApshot image?
         gzclose(a_file);
         return ERR_SNA_INVALID;
      }
      dwSnapSize = sh.ram_size[0] + (sh.ram_size[1] * 256); // memory dump size
      dwSnapSize &= ~0x3f; // limit to multiples of 64
      if (!dwSnapSize) {
         gzclose(a_file);
         return ERR_SNA_SIZE;
      }
      if (dwSnapSize > CPC.ram_size) { // memory dump size differs from current RAM size?

         pbTemp = (byte *)malloc( sizeof(byte) * dwSnapSize*1024);
         if (pbTemp) {
            free( pbRAM );
            CPC.ram_size = dwSnapSize;
            pbRAM = pbTemp;
         } else {
            gzclose(a_file);
            return ERR_OUT_OF_MEMORY;
         }
      }
      emulator_reset(false);
      n = gzread(a_file, pbRAM, dwSnapSize*1024); // read memory dump into CPC RAM
      gzclose(a_file);
      if (!n) {
         emulator_reset(false);
         return ERR_SNA_INVALID;
      }

// Z80
      _A = sh.AF[1];
      _F = sh.AF[0];
      _B = sh.BC[1];
      _C = sh.BC[0];
      _D = sh.DE[1];
      _E = sh.DE[0];
      _H = sh.HL[1];
      _L = sh.HL[0];
      _R = sh.R & 0x7f;
      _Rb7 = sh.R & 0x80; // bit 7 of R
      _I = sh.I;
      if (sh.IFF0)
         _IFF1 = Pflag;
      if (sh.IFF1)
         _IFF2 = Pflag;
      _IXh = sh.IX[1];
      _IXl = sh.IX[0];
      _IYh = sh.IY[1];
      _IYl = sh.IY[0];
      z80.SP.b.h = sh.SP[1];
      z80.SP.b.l = sh.SP[0];
      z80.PC.b.h = sh.PC[1];
      z80.PC.b.l = sh.PC[0];
      _IM = sh.IM; // interrupt mode
      z80.AFx.b.h = sh.AFx[1];
      z80.AFx.b.l = sh.AFx[0];
      z80.BCx.b.h = sh.BCx[1];
      z80.BCx.b.l = sh.BCx[0];
      z80.DEx.b.h = sh.DEx[1];
      z80.DEx.b.l = sh.DEx[0];
      z80.HLx.b.h = sh.HLx[1];
      z80.HLx.b.l = sh.HLx[0];
// Gate Array
      port.b.h = 0x7f;
      for (n = 0; n < 17; n++) { // loop for all colours + border
         GateArray.pen = n;
         val = sh.ga_ink_values[n]; // GA palette entry
         z80_OUT_handler(port, val | (1<<6));
      }
      val = sh.ga_pen; // GA pen
      z80_OUT_handler(port, (val & 0x3f));
      val = sh.ga_ROM_config; // GA ROM configuration
      z80_OUT_handler(port, (val & 0x3f) | (2<<6));
      val = sh.ga_RAM_config; // GA RAM configuration
      z80_OUT_handler(port, (val & 0x3f) | (3<<6));
// CRTC
      port.b.h = 0xbd;
      for (n = 0; n < 18; n++) { // loop for all CRTC registers
         val = sh.crtc_registers[n];
         CRTC.reg_select = n;
         z80_OUT_handler(port, val);
      }
      port.b.h = 0xbc;
      val = sh.crtc_reg_select; // CRTC register select
      z80_OUT_handler(port, val);
// ROM select
      port.b.h = 0xdf;
      val = sh.upper_ROM; // upper ROM number
      z80_OUT_handler(port, val);
// PPI
      port.b.h = 0xf4; // port A
      z80_OUT_handler(port, sh.ppi_A);
      port.b.h = 0xf5; // port B
      z80_OUT_handler(port, sh.ppi_B);
      port.b.h = 0xf6; // port C
      z80_OUT_handler(port, sh.ppi_C);
      port.b.h = 0xf7; // control
      z80_OUT_handler(port, sh.ppi_control);
// PSG
      PSG.control = PPI.portC;
      PSG.reg_select = sh.psg_reg_select;
      for (n = 0; n < 16; n++) { // loop for all PSG registers
         SetAYRegister(n, sh.psg_registers[n]);
      }

      if (sh.version > 1) { // does the snapshot have version 2 data?
         dwModel = sh.cpc_model; // determine the model it was saved for
         if (dwModel != CPC.model) { // different from what we're currently running?
            if (dwModel > 2) { // not one of the known models?
               emulator_reset(false);
               return ERR_SNA_CPC_TYPE;
            }
            strncpy(chPath, CPC.cpc_bios_path, sizeof(chPath)-2);
            strcat(chPath, "/");
            strncat(chPath, chROMFile[dwModel], sizeof(chPath)-1 - strlen(chPath)); // path to the required ROM image
            if ((a_file = fopen(chPath, "rb")) != NULL) {
               n = fread(pbROMlo, 2*16384, 1, a_file);
               fclose(a_file);
               if (!n) {
                  emulator_reset(false);
                  return ERR_CPC_ROM_MISSING;
               }
               CPC.model = dwModel;
            } else { // ROM image load failed
               emulator_reset(false);
               return ERR_CPC_ROM_MISSING;
            }
         }
      }
      if (sh.version > 2) { // does the snapshot have version 3 data?
         FDC.motor = sh.fdc_motor;
         driveA.current_track = sh.drvA_current_track;
         driveB.current_track = sh.drvB_current_track;
         PSG.AmplitudeEnv = sh.psg_env_step << 1; // multiply by 2 to bring it into the 0 - 30 range
         PSG.FirstPeriod = false;
         if (sh.psg_env_direction == 0x01) { // up
            switch (PSG.RegisterAY.EnvType)
            {
               case 4:
               case 5:
               case 6:
               case 7:
               case 13:
               case 14:
               case 15:
                  PSG.FirstPeriod = true;
                  break;
            }
         } else if (sh.psg_env_direction == 0xff) { // down
            switch (PSG.RegisterAY.EnvType)
            {
               case 0:
               case 1:
               case 2:
               case 3:
               case 9:
               case 10:
               case 11:
                  PSG.FirstPeriod = true;
                  break;
            }
         }
         CRTC.addr = sh.crtc_addr[0] + (sh.crtc_addr[1] * 256);
         VDU.scanline = sh.crtc_scanline[0] + (sh.crtc_scanline[1] * 256);
         CRTC.char_count = sh.crtc_char_count[0];
         CRTC.line_count = sh.crtc_line_count & 127;
         CRTC.raster_count = sh.crtc_raster_count & 31;
         CRTC.hsw_count = sh.crtc_hsw_count & 15;
         CRTC.vsw_count = sh.crtc_vsw_count & 15;
         CRTC.flags = sh.crtc_flags[0] + (sh.crtc_flags[1] * 256);
         GateArray.int_delay = sh.ga_int_delay;
         GateArray.sl_count = sh.ga_sl_count;
         z80.int_pending = sh.z80_int_pending;
      }
   } else {
      return ERR_FILE_NOT_FOUND;
   }
   return 0;
}

static int
snapshot_load(char *FileName)
{
  char *pszExt;
  if((pszExt = strrchr(FileName, '.'))) {
    if (!strcasecmp(pszExt, ".snz")) {
      return snapshot_load_snz(FileName);
    }
  }
  return snapshot_load_sna(FileName);
}

void
cap32_kbd_load(void)
{
  char        TmpFileName[MAX_PATH + 1];
  struct stat aStat;

  snprintf(TmpFileName, MAX_PATH, "%s/%s.kbd", CPC.cpc_kbd_path, CPC.cpc_save_name );
  if (! stat(TmpFileName, &aStat)) {
    psp_kbd_load_mapping(TmpFileName);
  }
}

int
cap32_kbd_save(void)
{
  char TmpFileName[MAX_PATH + 1];
  snprintf(TmpFileName, MAX_PATH, "%s/%s.kbd", CPC.cpc_kbd_path, CPC.cpc_save_name );
  return( psp_kbd_save_mapping(TmpFileName) );
}

void
cap32_joy_load(void)
{
  char        TmpFileName[MAX_PATH + 1];
  struct stat aStat;

  snprintf(TmpFileName, MAX_PATH, "%s/joy/%s.joy", CPC.cpc_home_dir, CPC.cpc_save_name );
  if (! stat(TmpFileName, &aStat)) {
    psp_joy_load_settings(TmpFileName);
  }
}

int
cap32_joy_save(void)
{
  char TmpFileName[MAX_PATH + 1];
  snprintf(TmpFileName, MAX_PATH, "%s/joy/%s.joy", CPC.cpc_home_dir, CPC.cpc_save_name );
  return( psp_joy_save_settings(TmpFileName) );
}


int
cap32_snapshot_load(char *FileName, int zip_format)
{
  char   SaveName[MAX_PATH+1];
  char*  ExtractName;
  char*  scan;
  char*  zip_buffer;
  int    error;
  size_t unzipped_size;

  error = 1;

  if (zip_format) {

    ExtractName = find_possible_filename_in_zip( FileName, "sna");
    if (ExtractName) {

      strncpy(SaveName, FileName, MAX_PATH);
      scan = strrchr(SaveName,'.');
      if (scan) *scan = '\0';
      update_save_name(SaveName);

      zip_buffer = extract_file_in_memory( FileName, ExtractName, &unzipped_size);
      if (zip_buffer) {
        error = snapshot_load_sna_buffer(zip_buffer, unzipped_size);
        free(zip_buffer);
      }
    }

  } else {
    strncpy(SaveName,FileName,MAX_PATH);
    scan = strrchr(SaveName,'.');
    if (scan) *scan = '\0';
    update_save_name(SaveName);
    error = snapshot_load(FileName);
  }

  if (! error ) {
    cap32_kbd_load();
    cap32_joy_load();
    cap32_load_cheat();
    cap32_load_settings();
  }

  return error;
}

static int
snapshot_save_sna(char *pchFileName)
{
   FILE* a_file;
   t_SNA_header sh;
   int n;

   memset(&sh, 0, sizeof(sh));
   strcpy(sh.id, "MV - SNA");
   sh.version = 3;
// Z80
   sh.AF[1] = _A;
   sh.AF[0] = _F;
   sh.BC[1] = _B;
   sh.BC[0] = _C;
   sh.DE[1] = _D;
   sh.DE[0] = _E;
   sh.HL[1] = _H;
   sh.HL[0] = _L;
   sh.R = (_R & 0x7f) | (_Rb7 & 0x80);
   sh.I = _I;
   if (_IFF1)
      sh.IFF0 = 1;
   if (_IFF2)
      sh.IFF1 = 1;
   sh.IX[1] = _IXh;
   sh.IX[0] = _IXl;
   sh.IY[1] = _IYh;
   sh.IY[0] = _IYl;
   sh.SP[1] = z80.SP.b.h;
   sh.SP[0] = z80.SP.b.l;
   sh.PC[1] = z80.PC.b.h;
   sh.PC[0] = z80.PC.b.l;
   sh.IM = _IM;
   sh.AFx[1] = z80.AFx.b.h;
   sh.AFx[0] = z80.AFx.b.l;
   sh.BCx[1] = z80.BCx.b.h;
   sh.BCx[0] = z80.BCx.b.l;
   sh.DEx[1] = z80.DEx.b.h;
   sh.DEx[0] = z80.DEx.b.l;
   sh.HLx[1] = z80.HLx.b.h;
   sh.HLx[0] = z80.HLx.b.l;
// Gate Array
   sh.ga_pen = GateArray.pen;
   for (n = 0; n < 17; n++) { // loop for all colours + border
      sh.ga_ink_values[n] = GateArray.ink_values[n];
   }
   sh.ga_ROM_config = GateArray.ROM_config;
   sh.ga_RAM_config = GateArray.RAM_config;
// CRTC
   sh.crtc_reg_select = CRTC.reg_select;
   for (n = 0; n < 18; n++) { // loop for all CRTC registers
      sh.crtc_registers[n] = CRTC.registers[n];
   }
// ROM select
   sh.upper_ROM = GateArray.upper_ROM;
// PPI
   sh.ppi_A = PPI.portA;
   sh.ppi_B = PPI.portB;
   sh.ppi_C = PPI.portC;
   sh.ppi_control = PPI.control;
// PSG
   sh.psg_reg_select = PSG.reg_select;
   for (n = 0; n < 16; n++) { // loop for all PSG registers
      sh.psg_registers[n] = PSG.RegisterAY.Index[n];
   }

   sh.ram_size[0] = CPC.ram_size & 0xff;
   sh.ram_size[1] = (CPC.ram_size >> 8) & 0xff;
// version 2 info
   sh.cpc_model = CPC.model;
// version 3 info
   sh.fdc_motor = FDC.motor;
   sh.drvA_current_track = driveA.current_track;
   sh.drvB_current_track = driveB.current_track;
   sh.psg_env_step = PSG.AmplitudeEnv >> 1; // divide by 2 to bring it into the 0 - 15 range
   if (PSG.FirstPeriod) {
      switch (PSG.RegisterAY.EnvType)
      {
         case 0:
         case 1:
         case 2:
         case 3:
         case 8:
         case 9:
         case 10:
         case 11:
            sh.psg_env_direction = 0xff; // down
            break;
         case 4:
         case 5:
         case 6:
         case 7:
         case 12:
         case 13:
         case 14:
         case 15:
            sh.psg_env_direction = 0x01; // up
            break;
      }
   } else {
      switch (PSG.RegisterAY.EnvType)
      {
         case 0:
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         case 6:
         case 7:
         case 9:
         case 11:
         case 13:
         case 15:
            sh.psg_env_direction = 0x00; // hold
            break;
         case 8:
         case 14:
            sh.psg_env_direction = 0xff; // down
            break;
         case 10:
         case 12:
            sh.psg_env_direction = 0x01; // up
            break;
      }
   }
   sh.crtc_addr[0] = CRTC.addr & 0xff;
   sh.crtc_addr[1] = (CRTC.addr >> 8) & 0xff;
   sh.crtc_scanline[0] = VDU.scanline & 0xff;
   sh.crtc_scanline[1] = (VDU.scanline >> 8) & 0xff;
   sh.crtc_char_count[0] = CRTC.char_count;
   sh.crtc_line_count = CRTC.line_count;
   sh.crtc_raster_count = CRTC.raster_count;
   sh.crtc_vt_adjust_count = CRTC.vt_adjust_count;
   sh.crtc_hsw_count = CRTC.hsw_count;
   sh.crtc_vsw_count = CRTC.vsw_count;
   sh.crtc_flags[0] = CRTC.flags & 0xff;
   sh.crtc_flags[1] = (CRTC.flags >> 8) & 0xff;
   sh.ga_int_delay = GateArray.int_delay;
   sh.ga_sl_count = GateArray.sl_count;
   sh.z80_int_pending = z80.int_pending;

   if ((a_file = fopen(pchFileName, "wb")) != NULL) {
      if (fwrite(&sh, sizeof(sh), 1, a_file) != 1) { // write snapshot header
         fclose(a_file);
         return ERR_SNA_WRITE;
      }
      if (fwrite(pbRAM, CPC.ram_size*1024, 1, a_file) != 1) { // write memory contents to snapshot file
         fclose(a_file);
         return ERR_SNA_WRITE;
      }
      fclose(a_file);
   } else {
      return ERR_SNA_WRITE;
   }
   return 0;
}

static int
snapshot_save_snz(char *pchFileName)
{
  gzFile a_file;
   t_SNA_header sh;
   int n;

   memset(&sh, 0, sizeof(sh));
   strcpy(sh.id, "MV - SNA");
   sh.version = 3;
// Z80
   sh.AF[1] = _A;
   sh.AF[0] = _F;
   sh.BC[1] = _B;
   sh.BC[0] = _C;
   sh.DE[1] = _D;
   sh.DE[0] = _E;
   sh.HL[1] = _H;
   sh.HL[0] = _L;
   sh.R = (_R & 0x7f) | (_Rb7 & 0x80);
   sh.I = _I;
   if (_IFF1)
      sh.IFF0 = 1;
   if (_IFF2)
      sh.IFF1 = 1;
   sh.IX[1] = _IXh;
   sh.IX[0] = _IXl;
   sh.IY[1] = _IYh;
   sh.IY[0] = _IYl;
   sh.SP[1] = z80.SP.b.h;
   sh.SP[0] = z80.SP.b.l;
   sh.PC[1] = z80.PC.b.h;
   sh.PC[0] = z80.PC.b.l;
   sh.IM = _IM;
   sh.AFx[1] = z80.AFx.b.h;
   sh.AFx[0] = z80.AFx.b.l;
   sh.BCx[1] = z80.BCx.b.h;
   sh.BCx[0] = z80.BCx.b.l;
   sh.DEx[1] = z80.DEx.b.h;
   sh.DEx[0] = z80.DEx.b.l;
   sh.HLx[1] = z80.HLx.b.h;
   sh.HLx[0] = z80.HLx.b.l;
// Gate Array
   sh.ga_pen = GateArray.pen;
   for (n = 0; n < 17; n++) { // loop for all colours + border
      sh.ga_ink_values[n] = GateArray.ink_values[n];
   }
   sh.ga_ROM_config = GateArray.ROM_config;
   sh.ga_RAM_config = GateArray.RAM_config;
// CRTC
   sh.crtc_reg_select = CRTC.reg_select;
   for (n = 0; n < 18; n++) { // loop for all CRTC registers
      sh.crtc_registers[n] = CRTC.registers[n];
   }
// ROM select
   sh.upper_ROM = GateArray.upper_ROM;
// PPI
   sh.ppi_A = PPI.portA;
   sh.ppi_B = PPI.portB;
   sh.ppi_C = PPI.portC;
   sh.ppi_control = PPI.control;
// PSG
   sh.psg_reg_select = PSG.reg_select;
   for (n = 0; n < 16; n++) { // loop for all PSG registers
      sh.psg_registers[n] = PSG.RegisterAY.Index[n];
   }

   sh.ram_size[0] = CPC.ram_size & 0xff;
   sh.ram_size[1] = (CPC.ram_size >> 8) & 0xff;
// version 2 info
   sh.cpc_model = CPC.model;
// version 3 info
   sh.fdc_motor = FDC.motor;
   sh.drvA_current_track = driveA.current_track;
   sh.drvB_current_track = driveB.current_track;
   sh.psg_env_step = PSG.AmplitudeEnv >> 1; // divide by 2 to bring it into the 0 - 15 range
   if (PSG.FirstPeriod) {
      switch (PSG.RegisterAY.EnvType)
      {
         case 0:
         case 1:
         case 2:
         case 3:
         case 8:
         case 9:
         case 10:
         case 11:
            sh.psg_env_direction = 0xff; // down
            break;
         case 4:
         case 5:
         case 6:
         case 7:
         case 12:
         case 13:
         case 14:
         case 15:
            sh.psg_env_direction = 0x01; // up
            break;
      }
   } else {
      switch (PSG.RegisterAY.EnvType)
      {
         case 0:
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         case 6:
         case 7:
         case 9:
         case 11:
         case 13:
         case 15:
            sh.psg_env_direction = 0x00; // hold
            break;
         case 8:
         case 14:
            sh.psg_env_direction = 0xff; // down
            break;
         case 10:
         case 12:
            sh.psg_env_direction = 0x01; // up
            break;
      }
   }
   sh.crtc_addr[0] = CRTC.addr & 0xff;
   sh.crtc_addr[1] = (CRTC.addr >> 8) & 0xff;
   sh.crtc_scanline[0] = VDU.scanline & 0xff;
   sh.crtc_scanline[1] = (VDU.scanline >> 8) & 0xff;
   sh.crtc_char_count[0] = CRTC.char_count;
   sh.crtc_line_count = CRTC.line_count;
   sh.crtc_raster_count = CRTC.raster_count;
   sh.crtc_vt_adjust_count = CRTC.vt_adjust_count;
   sh.crtc_hsw_count = CRTC.hsw_count;
   sh.crtc_vsw_count = CRTC.vsw_count;
   sh.crtc_flags[0] = CRTC.flags & 0xff;
   sh.crtc_flags[1] = (CRTC.flags >> 8) & 0xff;
   sh.ga_int_delay = GateArray.int_delay;
   sh.ga_sl_count = GateArray.sl_count;
   sh.z80_int_pending = z80.int_pending;

   if ((a_file = gzopen(pchFileName, "wb")) != NULL) {
      if (! gzwrite(a_file, &sh, sizeof(sh))) { // write snapshot header
         gzclose(a_file);
         return ERR_SNA_WRITE;
      }
      if (! gzwrite(a_file, pbRAM, CPC.ram_size*1024)) { // write memory contents to snapshot file
         gzclose(a_file);
         return ERR_SNA_WRITE;
      }
      gzclose(a_file);
   } else {
      return ERR_SNA_WRITE;
   }
   return 0;
}

static int
snapshot_save(char *FileName)
{
  char *pszExt;
  if((pszExt = strrchr(FileName, '.'))) {
    if (!strcasecmp(pszExt, ".snz")) {
      return snapshot_save_snz(FileName);
    }
  }
  return snapshot_save_sna(FileName);
}

int
cap32_snapshot_save_slot(int save_id)
{
  char        FileName[MAX_PATH+1];
  struct stat aStat;
  int         error;

  error = 1;

  if (save_id < CPC_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/sav_%s_%d.snz", CPC.cpc_save_path, CPC.cpc_save_name, save_id);
    error = snapshot_save(FileName);
    if (! error) {
      if (! stat(FileName, &aStat)) {
        CPC.cpc_save_state[save_id].used  = 1;
        CPC.cpc_save_state[save_id].thumb = 0;
        CPC.cpc_save_state[save_id].date  = aStat.st_mtime;
        snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.png", CPC.cpc_home_dir, CPC.cpc_save_name, save_id);
        if (psp_sdl_save_thumb_png(CPC.cpc_save_state[save_id].surface, FileName)) {
          CPC.cpc_save_state[save_id].thumb = 1;
        }
      }
    }
  }

  return error;
}

int
cap32_snapshot_del_slot(int save_id)
{
  char        FileName[MAX_PATH+1];
  struct stat aStat;
  int         error;

  error = 1;

  if (save_id < CPC_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/sav_%s_%d.snz", CPC.cpc_save_path, CPC.cpc_save_name, save_id);
    error = remove(FileName);
    if (! error) {
      CPC.cpc_save_state[save_id].used  = 0;
      CPC.cpc_save_state[save_id].thumb = 0;
      memset(&CPC.cpc_save_state[save_id].date, 0, sizeof(time_t));

      snprintf(FileName, MAX_PATH, "%s/sav_%s_%d.png", CPC.cpc_save_path, CPC.cpc_save_name, save_id);
      if (! stat(FileName, &aStat)) {
        remove(FileName);
      }
    }
  }

  return error;
}

int
cap32_snapshot_load_slot(int load_id)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  if (load_id < CPC_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/sav_%s_%d.snz", CPC.cpc_save_path, CPC.cpc_save_name, load_id);
    error = snapshot_load(FileName);
  }

  return error;
}



void
dsk_eject(t_drive *drive)
{
   dword track, side;

   for (track = 0; track < DSK_TRACKMAX; track++) { // loop for all tracks
      for (side = 0; side < DSK_SIDEMAX; side++) { // loop for all sides
         if (drive->track[track][side].data) { // track is formatted?
            free(drive->track[track][side].data); // release memory allocated for this track
         }
      }
   }
   dword dwTemp = drive->current_track; // save the drive head position
   memset(drive, 0, sizeof(t_drive)); // clear drive info structure
   drive->current_track = dwTemp;
}


void*
my_fopen(int gz_format, const char* filename, const char* mode)
{
  if (gz_format) return gzopen(filename, mode);
  return fopen(filename, mode);
}

int
my_fread(int gz_format, void* a_ptr, size_t a_size, size_t a_num, void* a_file)
{
  if (gz_format) {
    int num_elem = gzread(a_file, a_ptr, a_size * a_num);
    if (a_size) num_elem = num_elem / a_size;
    else num_elem = 0;
    return num_elem;
  }
  return fread(a_ptr, a_size, a_num, (FILE *)a_file);
}

int
my_fclose(int gz_format, void* a_file)
{
  if (gz_format) return gzclose(a_file);
  return fclose((FILE *)a_file);
}


int
dsk_load (char *pchFileName, t_drive *drive, char chID)
{
   void* a_file = 0;
   int gz_format = 0;
   int iRetCode;
   dword dwTrackSize, track, side, sector, dwSectorSize, dwSectors;
   byte *pbPtr, *pbDataPtr, *pbTempPtr, *pbTrackSizeTable;

   char *scan = strrchr(pchFileName, '.');
   if (scan && (!strcasecmp(scan, ".dsz"))) {
     gz_format = 1;
   }

   iRetCode = 0;
   dsk_eject(drive);

   if ((a_file = my_fopen(gz_format, pchFileName, "rb")) != NULL) {
      my_fread(gz_format, pbGPBuffer, 0x100, 1, a_file); // read DSK header
      pbPtr = pbGPBuffer;

      if (memcmp(pbPtr, "MV - CPC", 8) == 0) { // normal DSK image?
         drive->tracks = *(pbPtr + 0x30); // grab number of tracks
         if (drive->tracks > DSK_TRACKMAX) { // compare against upper limit
            drive->tracks = DSK_TRACKMAX; // limit to maximum
         }
         drive->sides = *(pbPtr + 0x31); // grab number of sides
         if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
            iRetCode = ERR_DSK_SIDES;
            goto exit;
         }
         dwTrackSize = (*(pbPtr + 0x32) + (*(pbPtr + 0x33) << 8)) - 0x100; // determine track size in bytes, minus track header
         drive->sides--; // zero base number of sides
         for (track = 0; track < drive->tracks; track++) { // loop for all tracks
            for (side = 0; side <= drive->sides; side++) { // loop for all sides
               my_fread(gz_format, pbGPBuffer+0x100, 0x100, 1, a_file); // read track header
               pbPtr = pbGPBuffer + 0x100;
               if (memcmp(pbPtr, "Track-Info", 10) != 0) { // abort if ID does not match
                  iRetCode = ERR_DSK_INVALID;
                  goto exit;
               }
               dwSectorSize = 0x80 << *(pbPtr + 0x14); // determine sector size in bytes
               dwSectors = *(pbPtr + 0x15); // grab number of sectors
               if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
                  iRetCode = ERR_DSK_SECTORS;
                  goto exit;
               }
               drive->track[track][side].sectors = dwSectors; // store sector count
               drive->track[track][side].size = dwTrackSize; // store track size
               drive->track[track][side].data = (byte *)malloc(dwTrackSize); // attempt to allocate the required memory
               if (drive->track[track][side].data == NULL) { // abort if not enough
                  iRetCode = ERR_OUT_OF_MEMORY;
                  goto exit;
               }
               pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
               pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
               for (sector = 0; sector < dwSectors; sector++) { // loop for all sectors
                  memcpy(drive->track[track][side].sector[sector].CHRN, (pbPtr + 0x18), 4); // copy CHRN
                  memcpy(drive->track[track][side].sector[sector].flags, (pbPtr + 0x1c), 2); // copy ST1 & ST2
                  drive->track[track][side].sector[sector].size = dwSectorSize;
                  drive->track[track][side].sector[sector].data = pbDataPtr; // store pointer to sector data
                  pbDataPtr += dwSectorSize;
                  pbPtr += 8;
               }
               if (!my_fread(gz_format, pbTempPtr, dwTrackSize, 1, a_file)) { // read entire track data in one go
                  iRetCode = ERR_DSK_INVALID;
                  goto exit;
               }
            }
         }
         drive->altered = 0; // disk is as yet unmodified
      } else {
         if (memcmp(pbPtr, "EXTENDED", 8) == 0) { // extended DSK image?
            drive->tracks = *(pbPtr + 0x30); // number of tracks
            if (drive->tracks > DSK_TRACKMAX) {  // limit to maximum possible
               drive->tracks = DSK_TRACKMAX;
            }
            drive->random_DEs = *(pbPtr + 0x31) & 0x80; // simulate random Data Errors?
            drive->sides = *(pbPtr + 0x31) & 3; // number of sides
            if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
               iRetCode = ERR_DSK_SIDES;
               goto exit;
            }
            pbTrackSizeTable = pbPtr + 0x34; // pointer to track size table in DSK header
            drive->sides--; // zero base number of sides
            for (track = 0; track < drive->tracks; track++) { // loop for all tracks
               for (side = 0; side <= drive->sides; side++) { // loop for all sides
                  dwTrackSize = (*pbTrackSizeTable++ << 8); // track size in bytes
                  if (dwTrackSize != 0) { // only process if track contains data
                     dwTrackSize -= 0x100; // compensate for track header
                     my_fread(gz_format,pbGPBuffer+0x100, 0x100, 1, a_file); // read track header
                     pbPtr = pbGPBuffer + 0x100;
                     if (memcmp(pbPtr, "Track-Info", 10) != 0) { // valid track header?
                        iRetCode = ERR_DSK_INVALID;
                        goto exit;
                     }
                     dwSectors = *(pbPtr + 0x15); // number of sectors for this track
                     if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
                        iRetCode = ERR_DSK_SECTORS;
                        goto exit;
                     }
                     drive->track[track][side].sectors = dwSectors; // store sector count
                     drive->track[track][side].size = dwTrackSize; // store track size
                     drive->track[track][side].data = (byte *)malloc(dwTrackSize); // attempt to allocate the required memory
                     if (drive->track[track][side].data == NULL) { // abort if not enough
                        iRetCode = ERR_OUT_OF_MEMORY;
                        goto exit;
                     }
                     pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
                     pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
                     for (sector = 0; sector < dwSectors; sector++) { // loop for all sectors
                        memcpy(drive->track[track][side].sector[sector].CHRN, (pbPtr + 0x18), 4); // copy CHRN
                        memcpy(drive->track[track][side].sector[sector].flags, (pbPtr + 0x1c), 2); // copy ST1 & ST2
                        dwSectorSize = *(pbPtr + 0x1e) + (*(pbPtr + 0x1f) << 8); // sector size in bytes
                        drive->track[track][side].sector[sector].size = dwSectorSize;
                        drive->track[track][side].sector[sector].data = pbDataPtr; // store pointer to sector data
                        pbDataPtr += dwSectorSize;
                        pbPtr += 8;
                     }
                     if (!my_fread(gz_format,pbTempPtr, dwTrackSize, 1, a_file)) { // read entire track data in one go
                        iRetCode = ERR_DSK_INVALID;
                        goto exit;
                     }
                  } else {
                     memset(&drive->track[track][side], 0, sizeof(t_track)); // track not formatted
                  }
               }
            }
            drive->altered = 0; // disk is as yet unmodified
         } else {
            iRetCode = ERR_DSK_INVALID; // file could not be identified as a valid DSK
         }
      }
exit:
      my_fclose(gz_format, a_file);
   } else {
      iRetCode = ERR_FILE_NOT_FOUND;
   }

   if (iRetCode != 0) { // on error, 'eject' disk from drive
      dsk_eject(drive);
   }
   return iRetCode;
}

int
cap32_disk_dir(char *FileName)
{
  cpc_dsk_system = 0;
  int error = cpc_dsk_dir(FileName);
  if (! error) {
    if (cpc_dsk_num_entry > 20) {
      int index;
      for (index = 0; index < cpc_dsk_num_entry; index++) {
        int cpos = 0;
        for (cpos = 0; cpc_dsk_dirent[index][cpos]; cpos++) {
          /* High number of files with no printable chars ? might be CPM */
          if (cpc_dsk_dirent[index][cpos] < 32) {
            cpc_dsk_system = 1;
            cpc_dsk_num_entry = 0;
            break;
          }
        }
      }
    }
  }
  return error;
}

int
cap32_disk_load(char *FileName, char drive, int zip_format)
{
  char   SaveName[MAX_PATH+1];
  char   TmpFileName[MAX_PATH + 1];
  FILE*  TmpFile;
  char*  ExtractName;
  char*  scan;
  char*  zip_buffer;
  int    error;
  size_t unzipped_size;

  error = 1;

  if (zip_format) {

    ExtractName = find_possible_filename_in_zip( FileName, "dsk");
    if (ExtractName) {

      strncpy(SaveName, FileName, MAX_PATH);
      scan = strrchr(SaveName,'.');
      if (scan) *scan = '\0';
      update_save_name(SaveName);

      zip_buffer = extract_file_in_memory( FileName, ExtractName, &unzipped_size);

      if (zip_buffer) {
        strcpy(TmpFileName, CPC.cpc_home_dir);
        strcat(TmpFileName, "/unzip.tmp");
        if (TmpFile = fopen( TmpFileName, "wb")) {
          fwrite(zip_buffer, unzipped_size, 1, TmpFile);
          fclose(TmpFile);
          if (drive == 'A') {
            if (CPC.cpc_reset_load_disk) emulator_reset(false);
            error = dsk_load(TmpFileName, &driveA, 'A');
            if (! error) {
              cap32_disk_dir(TmpFileName);
            }
          }
          else {
            error = dsk_load(TmpFileName, &driveB, 'B');
          }
          remove(TmpFileName);
        }
        free(zip_buffer);
      }
    }

  } else {
    strncpy(SaveName,FileName,MAX_PATH);
    scan = strrchr(SaveName,'.');
    if (scan) *scan = '\0';
    update_save_name(SaveName);

    if (drive == 'A') {
      if (CPC.cpc_reset_load_disk) emulator_reset(false);
      error = dsk_load(FileName, &driveA, 'A');
      if (! error) {
        cap32_disk_dir(FileName);
      }
    } else {
      error = dsk_load(FileName, &driveB, 'B');
    }
  }

  if (drive == 'A') {
    if (! error ) {
      cap32_kbd_load();
      cap32_joy_load();
      cap32_load_cheat();
      cap32_load_settings();

      char *file_name = (char*)calloc(1, MAX_PATH+1);
      char *file_path = (char*)calloc(1, MAX_PATH+1);
      file_name = (strrchr(FileName, '/'))+1;
      memcpy(file_path, FileName, strlen(FileName) - strlen(file_name));
      file_path[strlen(file_path) + 1] = '\0';
      loc_cap32_save_paths(DEFAULT_DISK_PATH, file_path);
    }
  }

  return error;
}

int dsk_save (char *pchFileName, t_drive *drive, char chID)
{
   FILE* a_file;
   t_DSK_header dh;
   t_track_header th;
   dword track, side, pos, sector;

   if ((a_file = fopen(pchFileName, "wb")) != NULL) {
      memset(&dh, 0, sizeof(dh));
      strcpy(dh.id, "EXTENDED CPC DSK File\r\nDisk-Info\r\n");
      strcpy(dh.unused1, "Caprice32\r\n");
      dh.tracks = drive->tracks;
      dh.sides = (drive->sides+1) | (drive->random_DEs); // correct side count and indicate random DEs, if necessary
      pos = 0;
      for (track = 0; track < drive->tracks; track++) { // loop for all tracks
         for (side = 0; side <= drive->sides; side++) { // loop for all sides
            if (drive->track[track][side].size) { // track is formatted?
               dh.track_size[pos] = (drive->track[track][side].size + 0x100) >> 8; // track size + header in bytes
            }
            pos++;
         }
      }
      if (!fwrite(&dh, sizeof(dh), 1, a_file)) { // write header to file
         fclose(a_file);
         return ERR_DSK_WRITE;
      }

      memset(&th, 0, sizeof(th));
      strcpy(th.id, "Track-Info\r\n");
      for (track = 0; track < drive->tracks; track++) { // loop for all tracks
         for (side = 0; side <= drive->sides; side++) { // loop for all sides
            if (drive->track[track][side].size) { // track is formatted?
               th.track = track;
               th.side = side;
               th.bps = 2;
               th.sectors = drive->track[track][side].sectors;
               th.gap3 = 0x4e;
               th.filler = 0xe5;
               for (sector = 0; sector < th.sectors; sector++) {
                  memcpy(&th.sector[sector][0], drive->track[track][side].sector[sector].CHRN, 4); // copy CHRN
                  memcpy(&th.sector[sector][4], drive->track[track][side].sector[sector].flags, 2); // copy ST1 & ST2
                  th.sector[sector][6] = drive->track[track][side].sector[sector].size & 0xff;
                  th.sector[sector][7] = (drive->track[track][side].sector[sector].size >> 8) & 0xff; // sector size in bytes
               }
               if (!fwrite(&th, sizeof(th), 1, a_file)) { // write track header
                  fclose(a_file);
                  return ERR_DSK_WRITE;
               }
               if (!fwrite(drive->track[track][side].data, drive->track[track][side].size, 1, a_file)) { // write track data
                  fclose(a_file);
                  return ERR_DSK_WRITE;
               }
            }
         }
      }
      fclose(a_file);
   } else {
      return ERR_DSK_WRITE; // write attempt failed
   }

/* char *pchTmpBuffer = new char[MAX_LINE_LEN];
   LoadString(hAppInstance, MSG_DSK_SAVE, chMsgBuffer, sizeof(chMsgBuffer));
   snprintf(pchTmpBuffer, MAX_PATH-1, chMsgBuffer, chID, chID == 'A' ? CPC.drvA_file : CPC.drvB_file);
   add_message(pchTmpBuffer);
   delete [] pchTmpBuffer; */
   return 0;
}


void
vdu_init()
{
  VDU.hstart = 11;
  VDU.hwidth = CPC_VISIBLE_SCR_WIDTH / 8;
  VDU.vstart = 47;
  VDU.vheight = CPC_VISIBLE_SCR_HEIGHT;
}

int
emulator_patch_ROM(void)
{
   FILE* a_file;
   char chPath[MAX_PATH + 1];

   strncpy(chPath, CPC.cpc_bios_path, sizeof(chPath)-2);
   strcat(chPath, "/");
   strncat(chPath, chROMFile[CPC.model], sizeof(chPath)-1 - strlen(chPath));
   if ((a_file = fopen(chPath, "r")) != NULL) { // load CPC OS + Basic
      fread(pbROMlo, 2*16384, 1, a_file);
      fclose(a_file);
   } else {
      return ERR_CPC_ROM_MISSING;
   }

   return 0;
}



void
emulator_reset(bool bolMF2Reset)
{
   int n, val1, val2;

// Z80
   memset(&z80, 0, sizeof(z80)); // clear all Z80 registers and support variables
   _IX =
  _IY = 0xffff; // IX and IY are FFFF after a reset!
   _F = Zflag; // set zero flag

// CPC
   memset(keyboard_matrix, 0xff, sizeof(keyboard_matrix)); // clear CPC keyboard matrix

// CRTC
   memset(&CRTC, 0, sizeof(CRTC)); // clear CRTC data structure
   for (n = 0; n < 14; n++) { // program CRTC with 'valid' data
      CRTC.registers[n] = CRTC_values[(CPC_jumpers & 0x10)>>4][n];
   }
   CRTC.flags = HDT_flag | VDT_flag;
   CRTC.hsw =
   CRTC.hsw_active = CRTC.registers[3] & 0x0f;
   CRTC.vsw = CRTC.registers[3] >> 4;
   CRTC.vt_adjust = CRTC.registers[5] & 0x1f;
   CRTC.skew = (CRTC.registers[8] >> 4) & 3;
   if (CRTC.skew == 3) { // no output?
      CRTC.skew = 0xff;
   }
   CRTC.max_raster = CRTC.registers[9] << 3;
   val1 = CRTC.registers[12] & 0x3f;
   val2 = val1 & 0x0f; // isolate screen size
   val1 = (val1 << 1) & 0x60; // isolate CPC RAM bank
   val2 |= val1; // combine
   CRTC.addr =
   CRTC.requested_addr = (CRTC.registers[13] + (val2 << 8)) << 1;
   CRTC.last_hdisp = 0x28;

// VDU
   memset(&VDU, 0, sizeof(VDU)); // clear VDU data structure
   VDU.hsw =
   VDU.hsw_active = 4;
   VDU.scanline_min = 240; //272;
   vdu_init();

// Gate Array
   memset(&GateArray, 0, sizeof(GateArray)); // clear GA data structure
   GateArray.scr_mode = GateArray.requested_scr_mode = 1;
   ga_init_banking();

// PPI
   memset(&PPI, 0, sizeof(PPI)); // clear PPI data structure

// PSG
   PSG.control = 0;
   ResetAYChipEmulation();

// FDC
   memset(&FDC, 0, sizeof(FDC)); // clear FDC data structure
   FDC.phase = CMD_PHASE;
   FDC.flags = STATUSDRVA_flag | STATUSDRVB_flag;

// memory
   if (bolMF2Reset) {
      memset(pbRAM, 0, 64*1024); // clear only the first 64K of CPC memory
   } else {
      memset(pbRAM, 0, CPC.ram_size*1024); // clear all memory used for CPC RAM
   }
   for (n = 0; n < 4; n++) { // initialize active read/write bank configuration
      membank_read[n] = membank_config[0][n];
      membank_write[n] = membank_config[0][n];
   }
   membank_read[0] = pbROMlo; // 'page in' lower ROM
   membank_read[3] = pbROMhi; // 'page in' upper ROM
}

void
cap32_change_ram_size(int new_ram_size)
{
  if (new_ram_size != CPC.ram_size) {
    CPC.ram_size = new_ram_size;
    if (pbRAM) {
      free(pbRAM);
      pbRAM = NULL;
    }
    CPC.ram_size = new_ram_size;
    pbRAM = (byte*)malloc( sizeof(byte) * CPC.ram_size*1024);
    emulator_reset(false);
  }
}

int
emulator_init(void)
{
  FILE* a_file;
  int iErr, iRomNum;
  int n = 0;
  char chPath[MAX_PATH + 1];
  char *pchRomData;

  pbGPBuffer = (byte*)malloc( sizeof(byte) * 128*1024); // attempt to allocate the general purpose buffer
  pbRAM = (byte*)malloc( sizeof(byte) * CPC.ram_size*1024); // allocate memory for desired amount of RAM
  pbROMlo = (byte*)malloc( sizeof(byte) * 32*1024); // allocate memory for 32K of ROM
  if ((!pbGPBuffer) || (!pbRAM) || (!pbROMlo)) {
     return ERR_OUT_OF_MEMORY;
  }
  pbROMhi =
  pbExpansionROM = pbROMlo + 16384;
  memset(memmap_ROM, 0, sizeof(memmap_ROM[0]) * 256); // clear the expansion ROM map
  ga_init_banking(); // init the CPC memory banking map
  if ((iErr = emulator_patch_ROM())) {
     return iErr;
  }

  for (iRomNum = 0; iRomNum < 16; iRomNum++) { // loop for ROMs 0-15
     if (CPC.rom_file[iRomNum][0]) { // is a ROM image specified for this slot?
        pchRomData = (char*)malloc( sizeof(char) * 16384); // allocate 16K
        memset(pchRomData, 0, 16384); // clear memory
        strncpy(chPath, CPC.cpc_bios_path, sizeof(chPath)-2);
        strcat(chPath, "/");
        strncat(chPath, CPC.rom_file[iRomNum], sizeof(chPath)-1 - strlen(chPath));
        if ((a_file = fopen(chPath, "rb")) != NULL) { // attempt to open the ROM image
           fread(pchRomData, 128, 1, a_file); // read 128 bytes of ROM data
           word checksum = 0;
           for (n = 0; n < 0x43; n++) {
              checksum += pchRomData[n];
           }
           if (checksum == ((pchRomData[0x43] << 8) + pchRomData[0x44])) { // if the checksum matches, we got us an AMSDOS header
              fread(pchRomData, 128, 1, a_file); // skip it
           }
           if (!(pchRomData[0] & 0xfc)) { // is it a valid CPC ROM image (0 = forground, 1 = background, 2 = extension)?
              fread(pchRomData+128, 16384-128, 1, a_file); // read the rest of the ROM file
              memmap_ROM[iRomNum] = (byte *)pchRomData; // update the ROM map
           } else { // not a valid ROM file
              fprintf(stderr, "ERROR: %s is not a CPC ROM file - clearing ROM slot %d.\n", CPC.rom_file[iRomNum], iRomNum);
              free( pchRomData ); // free memory on error
              CPC.rom_file[iRomNum][0] = 0;
           }
           fclose(a_file);
        } else { // file not found
           fprintf(stderr, "ERROR: The %s file is missing - clearing ROM slot %d.\n", CPC.rom_file[iRomNum], iRomNum);
           free( pchRomData ); // free memory on error
           CPC.rom_file[iRomNum][0] = 0;
        }
     }
  }

  emulator_reset(false);

  return 0;
}



void
emulator_shutdown(void)
{
  int iRomNum;

  for (iRomNum = 2; iRomNum < 16; iRomNum++) // loop for ROMs 2-15
  {
     if (memmap_ROM[iRomNum] != NULL) // was a ROM assigned to this slot?
        free( memmap_ROM[iRomNum] ); // if so, release the associated memory
  }
  free( pbROMlo );
  free( pbRAM );
  free( pbGPBuffer );
}



void
audio_update(void *userdata, byte *stream, int len)
{
  if (pbSndStream) {
# if defined(DINGUX_MODE)
    uint *src = (uint *)pbSndStream;
    uint *tgt = stream;
    len = len >> 2;
    while (len-- > 0) *tgt++ = *src++;
    // SDL_MixAudio(stream, (unsigned char *)pbSndStream, len, 100);
# else
    long volume = (SDL_MIX_MAXVOLUME * gp2xGetSoundVolume()) / 50;
    SDL_MixAudio(stream, (unsigned char *)pbSndStream, len, volume);
# endif
  }
}

void
cap32_swap_sound_buffer(void)
{
  pbSndStream = pbSndBufferArray[pbSndBufferIndex];
  pbSndBufferIndex  = ! pbSndBufferIndex;
  pbSndBuffer       = pbSndBufferArray[pbSndBufferIndex];
  pbSndBufferEnd    = pbSndBufferEndArray[pbSndBufferIndex];
  CPC.snd_bufferptr = pbSndBuffer;
}

int
audio_align_samples(int given)
{
  int actual = 1;
  while (actual < given) {
    actual <<= 1;
  }
  return actual; // return the closest match as 2^n
}

int
audio_init(void)
{
   int n;
   SDL_AudioSpec desired;
   SDL_AudioSpec obtained;

   memset(&obtained,0,sizeof(SDL_AudioSpec));
   memset(&desired,0,sizeof(SDL_AudioSpec));

# if defined(DINGUX_MODE) || defined(GCW0_MODE)
   desired.format   = AUDIO_S16;
   desired.freq     = 44100;
# else
   desired.freq     = freq_table[CPC.snd_playback_rate];
   desired.format   = CPC.snd_bits ? AUDIO_S16LSB : AUDIO_S8;
# endif
   desired.channels = CPC.snd_stereo+1;
   desired.samples  = 1024;//desired.freq / 50;
   desired.callback = audio_update;
   desired.userdata = NULL;

   if (SDL_OpenAudio(&desired, &obtained) < 0) {
      fprintf(stderr, "Could not open audio: %s\n", SDL_GetError());
      return 1;
   }

   CPC.snd_buffersize = obtained.size; // size is samples * channels * bytes per sample (1 or 2)

   pbSndBufferArray[0] = (byte *)malloc(CPC.snd_buffersize); // allocate the sound data buffer
   pbSndBufferEndArray[0] = pbSndBufferArray[0] + CPC.snd_buffersize;

   pbSndBufferArray[1] = (byte *)malloc(CPC.snd_buffersize); // allocate the sound data buffer
   pbSndBufferEndArray[1] = pbSndBufferArray[1] + CPC.snd_buffersize;
   pbSndStream = pbSndBufferArray[1];

   pbSndBuffer = pbSndBufferArray[0];
   pbSndBufferEnd = pbSndBufferEndArray[0];

   memset(pbSndBuffer, 0, CPC.snd_buffersize);
   memset(pbSndStream, 0, CPC.snd_buffersize);

   CPC.snd_bufferptr = pbSndBuffer; // init write cursor

   SDL_Delay(1000);        // Give sound some time to init
   SDL_PauseAudio(0);

   gp2xInitSoundVolume();

   InitAY();

   for (n = 0; n < 16; n++) {
      SetAYRegister(n, PSG.RegisterAY.Index[n]); // init sound emulation with valid values
   }

   SDL_Delay(1000);        // Give sound some time to init
   SDL_PauseAudio(1);

   return 0;
}



void
audio_shutdown(void)
{
   SDL_CloseAudio();
}

void
audio_pause(void)
{
  SDL_PauseAudio(1);
}

static int
loc_cpc_save_cheat(char *chFileName)
{
  FILE* FileDesc;
  int   cheat_num;
  int   error = 0;

  FileDesc = fopen(chFileName, "w");
  if (FileDesc != (FILE *)0 ) {

    for (cheat_num = 0; cheat_num < CPC_MAX_CHEAT; cheat_num++) {
      CPC_cheat_t* a_cheat = &CPC.cpc_cheat[cheat_num];
      if (a_cheat->type != CPC_CHEAT_NONE) {
        fprintf(FileDesc, "%d,%x,%x,%s\n",
                a_cheat->type, a_cheat->addr, a_cheat->value, a_cheat->comment);
      }
    }
    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

int
cap32_save_cheat(void)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/cht/%s.cht", CPC.cpc_home_dir, CPC.cpc_save_name);
  error = loc_cpc_save_cheat(FileName);

  return error;
}

static int
loc_cpc_load_cheat(char *chFileName)
{
  char  Buffer[512];
  char *Scan;
  char *Field;
  unsigned int  cheat_addr;
  unsigned int  cheat_value;
  unsigned int  cheat_type;
  char         *cheat_comment;
  int           cheat_num;
  FILE* FileDesc;

  memset(CPC.cpc_cheat, 0, sizeof(CPC.cpc_cheat));
  cheat_num = 0;

  FileDesc = fopen(chFileName, "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    /* %d, %x, %x, %s */
    Field = Buffer;
    Scan = strchr(Field, ',');
    if (! Scan) continue;
    *Scan = 0;
    if (sscanf(Field, "%d", &cheat_type) != 1) continue;
    Field = Scan + 1;
    Scan = strchr(Field, ',');
    if (! Scan) continue;
    *Scan = 0;
    if (sscanf(Field, "%x", &cheat_addr) != 1) continue;
    Field = Scan + 1;
    Scan = strchr(Field, ',');
    if (! Scan) continue;
    *Scan = 0;
    if (sscanf(Field, "%x", &cheat_value) != 1) continue;
    Field = Scan + 1;
    cheat_comment = Field;

    if (cheat_type <= CPC_CHEAT_NONE) continue;

    CPC_cheat_t* a_cheat = &CPC.cpc_cheat[cheat_num];

    a_cheat->type  = cheat_type;
    a_cheat->addr  = cheat_addr;
    a_cheat->value = cheat_value;
    strncpy(a_cheat->comment, cheat_comment, sizeof(a_cheat->comment));
    a_cheat->comment[sizeof(a_cheat->comment)-1] = 0;

    if (++cheat_num >= CPC_MAX_CHEAT) break;
  }
  fclose(FileDesc);

  return 0;
}

int
cap32_load_cheat()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/cht/%s.cht", CPC.cpc_home_dir, CPC.cpc_save_name);
  error = loc_cpc_load_cheat(FileName);

  return error;
}

int
cap32_load_file_cheat(char *FileName)
{
  return loc_cpc_load_cheat(FileName);
}

void
cap32_apply_cheats()
{
  int cheat_num;
  for (cheat_num = 0; cheat_num < CPC_MAX_CHEAT; cheat_num++) {
    CPC_cheat_t* a_cheat = &CPC.cpc_cheat[cheat_num];
    if (a_cheat->type == CPC_CHEAT_ENABLE) {
      pbRAM[a_cheat->addr % (CPC.ram_size * 1024)] = a_cheat->value;
    }
  }
}


void
audio_resume(void)
{
   if (CPC.snd_enabled) {
      SDL_PauseAudio(0);
   }
}

void video_init_tables (void)
{
   int idx, n, p1, p2, p3, p4;

   idx = 0;
   for (n = 0; n < 256; n++) { // calculate mode0 byte-to-pixel translation table
      p1 = ((n & 0x80) >> 7) + ((n & 0x08) >> 2) + ((n & 0x20) >> 3) + ((n & 0x02) << 2);
      p2 = ((n & 0x40) >> 6) + ((n & 0x04) >> 1) + ((n & 0x10) >> 2) + ((n & 0x01) << 3);
      mode0_table[idx++] = p1;
      mode0_table[idx++] = p2;
   }

   idx = 0;
   for (n = 0; n < 256; n++) { // calculate mode1 byte-to-pixel translation table
      p1 = ((n & 0x80) >> 7) + ((n & 0x08) >> 2);
      p2 = ((n & 0x40) >> 6) + ((n & 0x04) >> 1);
      p3 = ((n & 0x20) >> 5) +  (n & 0x02);
      p4 = ((n & 0x10) >> 4) + ((n & 0x01) << 1);
      mode1_table[idx++] = p1;
      mode1_table[idx++] = p2;
      mode1_table[idx++] = p3;
      mode1_table[idx++] = p4;
   }
}


int
video_set_palette(void)
{
  int n;

  if (! CPC.cpc_green) {
    for (n = 0; n < 32; n++) {
      dword red = (dword)(colours_rgb[n][0] * (CPC.scr_intensity / 10.0) * 255);
      if (red > 255) { // limit to the maximum
         red = 255;
      }
      dword green = (dword)(colours_rgb[n][1] * (CPC.scr_intensity / 10.0) * 255);
      if (green > 255) {
         green = 255;
      }
      dword blue = (dword)(colours_rgb[n][2] * (CPC.scr_intensity / 10.0) * 255);
      if (blue > 255) {
         blue = 255;
      }
      sdl_colors[n].r = red;
      sdl_colors[n].g = green;
      sdl_colors[n].b = blue;
      ddword a_color = SDL_MapRGB(cpc_surface->format, red, green, blue);
      colour_table[n] = a_color | (a_color << 16);
    }
  } else {
    for (n = 0; n < 32; n++) {
      dword green = (dword)(colours_green[n] * (CPC.scr_intensity / 10.0) * 255);
      if (green > 255) {
        green = 255;
      }
      sdl_colors[n].r = 0;
      sdl_colors[n].g = green;
      sdl_colors[n].b = 0;
      ddword a_color = SDL_MapRGB(cpc_surface->format, 0, green, 0);
      colour_table[n] = a_color | (a_color << 16);
    }
  }

  psp_sdl_set_palette(sdl_colors);

  for (n = 0; n < 17; n++) { // loop for all colours + border
    int i=GateArray.ink_values[n];
    GateArray.palette[n] = colour_table[i];
  }
  return 0;
}


void
video_set_style(void)
{
  GateArray.scr_mode = GateArray.requested_scr_mode = GateArray.ROM_config & 3;
}

void
cap32_set_direct_surface()
{
  cpc_surface = back_surface;
  CPC.scr_bps = cpc_surface->pitch / 4;
  CPC.scr_base = (dword *)cpc_surface->pixels;
  CPC.scr_min  = (dword *)cpc_surface->pixels;
  CPC.scr_max  = CPC.scr_min + CPC.scr_bps * CPC_VISIBLE_SCR_HEIGHT;

  CPC.scr_pixels_offs = 0;
  CPC.scr_line_offs = CPC.scr_bps;

  VDU.vstart = 47;
  VDU.vheight = CPC_VISIBLE_SCR_HEIGHT;
}

void
cap32_set_blit_surface()
{
  cpc_surface = blit_surface;
  CPC.scr_bps = cpc_surface->pitch / 4;
  CPC.scr_base = (dword *)cpc_surface->pixels;
  CPC.scr_min  = (dword *)cpc_surface->pixels;
  CPC.scr_max  = CPC.scr_min + CPC.scr_bps * CPC_VISIBLE_SCR_HEIGHT;

  CPC.scr_pixels_offs = 0;
  CPC.scr_line_offs = CPC.scr_bps;

  VDU.vstart = 47;
  VDU.vheight = CPC_VISIBLE_SCR_HEIGHT;
}

void
cap32_save_back_to_blit()
{
  int x; int y;
  /* Bakcup direct back_surface to blit_surface for thumb images ! */
  if (cpc_surface != blit_surface) {
    //u16* pt_src = (u16*)((u8*)cpc_surface->pixels + CPC.scr_pixels_offs);
    u16* pt_src = (u16*)cpc_surface->pixels;
    u16* pt_dst = (u16*)blit_surface->pixels;
    for (y = 0; y < CPC_SCR_HEIGHT; y++) {
      for (x = 0; x < CPC_SCR_WIDTH; x++) {
        *pt_dst++ = pt_src[x];
      }
      pt_src += PSP_LINE_SIZE;
    }
  }
}

int
video_init(void)
{
  cpc_surface = blit_surface;
  CPC.scr_bpp = cpc_surface->format->BitsPerPixel; // bit depth of the surface

  int iErrCode = video_set_palette(); // init CPC colours
  if (iErrCode) {
     return iErrCode;
  }

  CPC.scr_bps = cpc_surface->pitch / 4; // rendered screen line length (changing bytes to dwords)
  CPC.scr_pixels_offs = 0;
  CPC.scr_base = (dword *)cpc_surface->pixels; // memory address of back buffer
  CPC.scr_min  = (dword *)cpc_surface->pixels;
  CPC.scr_max  = CPC.scr_min + CPC.scr_bps * CPC_VISIBLE_SCR_HEIGHT;
  CPC.scr_line_offs = CPC.scr_bps;

  video_set_style(); // select rendering style
  vdu_init(); // initialize the monitor emulation

  return 0;
}

void
cap32_synchronize(void)
{
  static u32 nextclock = 1;

  if (CPC.cpc_speed_limiter) {
    u32 curclock = SDL_GetTicks();
    while (curclock < nextclock) {
     curclock = SDL_GetTicks();
    }
    u32 f_period = 1000 / CPC.cpc_speed_limiter;
    if (CPC.cpc_render_mode == CPC_RENDER_ULTRA) f_period *= 2;
    nextclock += f_period;
    if (nextclock < curclock) nextclock = curclock + f_period;
  }
}

void
cap32_update_fps()
{
  static u32 next_sec_clock = 0;
  static u32 cur_num_frame = 0;
  cur_num_frame++;
  u32 curclock = SDL_GetTicks();
  if (curclock > next_sec_clock) {
    next_sec_clock = curclock + 1000;
    CPC.cpc_current_fps = cur_num_frame;
    cur_num_frame = 0;
  }
}

void
vdu_half_top()
{
  VDU.vstart = 47;
  VDU.vheight = (CPC_VISIBLE_SCR_HEIGHT/2);
  CPC.scr_pixels_offs = 0;
  CPC.scr_base = (dword *)((u8*)cpc_surface->pixels + CPC.scr_pixels_offs);
}

void
vdu_half_bottom()
{
  CPC.scr_pixels_offs = CPC.scr_bps * 4 * CPC.scr_fs_height / 2;
  CPC.scr_base = (dword *)((u8*)cpc_surface->pixels + CPC.scr_pixels_offs);

  VDU.vstart = (CPC_VISIBLE_SCR_HEIGHT/2)+46;
  VDU.vheight = (CPC_VISIBLE_SCR_HEIGHT/2);
}

void
cap32_change_render_mode(int new_render_mode)
{
  if (new_render_mode >= CPC_RENDER_NORMAL) {
    cap32_set_blit_surface();
  } else {
    cap32_set_direct_surface();
  }
  CPC.cpc_render_mode = new_render_mode;
}

void
cap32_treat_command_key(int cpc_idx)
{
  int new_render;

  audio_pause();

  switch (cpc_idx)
  {
    case CAP32_FPS: CPC.cpc_view_fps = ! CPC.cpc_view_fps;
    break;
    case CAP32_JOY: CPC.psp_reverse_analog = ! CPC.psp_reverse_analog;
    break;
    case CAP32_RENDER:
      psp_sdl_black_screen();
      new_render = CPC.cpc_render_mode + 1;
      if (new_render > CPC_LAST_RENDER) new_render = 0;
      cap32_change_render_mode(new_render);
    break;
    case CAP32_LOAD: psp_main_menu_load_current();
    break;
    case CAP32_SAVE: psp_main_menu_save_current();
    break;
    case CAP32_RESET:
       psp_sdl_black_screen();
       emulator_reset(false);
       reset_save_name();
    break;
    case CAP32_AUTOFIRE:
       kbd_change_auto_fire(! CPC.cpc_auto_fire);
    break;
    case CAP32_DECFIRE:
      if (CPC.cpc_auto_fire_period > 0) CPC.cpc_auto_fire_period--;
    break;
    case CAP32_INCFIRE:
      if (CPC.cpc_auto_fire_period < 19) CPC.cpc_auto_fire_period++;
    break;
    case CAP32_SCREEN: psp_screenshot_mode = 10;
    break;
  }
  audio_resume();
}

void
cap32_change_green_mode(int new_green)
{
  if (new_green != CPC.cpc_green) {
    CPC.cpc_green = new_green;
    video_set_palette();
  }
}

static inline void
video_display(void)
{
static int vdusplit = 0;

  if (CPC.cpc_render_mode == CPC_RENDER_ULTRA) {
    if (! vdusplit) {
      vdu_half_top();
    } else {
      vdu_half_bottom();
    }
    vdusplit = ! vdusplit;
    if (vdusplit) return;
  } else
  if (CPC.cpc_render_mode != CPC_RENDER_FAST) {
    render16bpp_blit();
  }

  if (psp_kbd_is_danzeff_mode()) {
    danzeff_moveTo(-48, -48);
    danzeff_render();
  }

  if (CPC.cpc_view_fps) {
    char buffer[32];
    sprintf(buffer, "%03d %3d", CPC.cpc_current_clock, (int)CPC.cpc_current_fps );
    psp_sdl_fill_print(0, 0, buffer, 0xffffff, 0 );
  }

  psp_sdl_flip();

  cap32_synchronize();

  if (psp_screenshot_mode) {
    psp_screenshot_mode--;
    if (psp_screenshot_mode <= 0) {
      psp_sdl_save_screenshot();
      psp_screenshot_mode = 0;
    }
  }
}


int
getConfigValueInt(char* pchFileName, char* pchSection, char* pchKey, int iDefaultValue)
{
   return iDefaultValue;
}



void
getConfigValueString(char* pchFileName, char* pchSection, char* pchKey,
                     char* pchValue, int iSize, char* pchDefaultValue)
{
   strncpy(pchValue, pchDefaultValue, iSize); // no value found, return the default
}

int
is_fdc_led_on(void)
{
  return FDC.led;
}

int
loc_cap32_save_settings(char *FileName)
{
  FILE* FileDesc;
  int   error = 0;

  FileDesc = fopen(FileName, "w");
  if (FileDesc != (FILE *)0 ) {

    fprintf(FileDesc, "model=%d\n"              , CPC.model);
    fprintf(FileDesc, "ram_size=%d\n"           , CPC.ram_size);
    fprintf(FileDesc, "speed=%d\n"              , CPC.speed);
    fprintf(FileDesc, "cpc_speed_limiter=%d\n"  , CPC.cpc_speed_limiter);
    fprintf(FileDesc, "cpc_view_fps=%d\n"       , CPC.cpc_view_fps);
    fprintf(FileDesc, "cpc_green=%d\n"          , CPC.cpc_green);
    fprintf(FileDesc, "snd_playback_rate=%d\n"  , CPC.snd_playback_rate);
    fprintf(FileDesc, "snd_volume=%d\n"         , CPC.snd_volume);
    fprintf(FileDesc, "psp_cpu_clock=%d\n"      , CPC.psp_cpu_clock);
    fprintf(FileDesc, "psp_skip_max_frame=%d\n" , CPC.psp_skip_max_frame);
    fprintf(FileDesc, "cpc_render_mode=%d\n"    , CPC.cpc_render_mode);
    fprintf(FileDesc, "cpc_display_border=%d\n" , CPC.cpc_display_border);
    fprintf(FileDesc, "psp_explore_disk=%d\n"   , CPC.psp_explore_disk);
    fprintf(FileDesc, "cpc_reset_load_disk=%d\n", CPC.cpc_reset_load_disk);
    fprintf(FileDesc, "psp_kbd_skin=%d\n"       , psp_kbd_skin);

    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

int
cap32_save_settings(void)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", CPC.cpc_home_dir, CPC.cpc_save_name);
  error = loc_cap32_save_settings(FileName);

  return error;
}


int
loc_cap32_save_paths(int path_type, char path[])
{

  char  FileName[MAX_PATH+1];
  int   error;
  FILE* FileDesc;

  error = 0;

  snprintf(FileName, MAX_PATH, "%s/set/default_paths.set", CPC.cpc_home_dir);

  FileDesc = fopen(FileName, "w");

  if (FileDesc != (FILE *)0 ) {
    fprintf(FileDesc, "%d=%s\n", path_type, path);
    fclose(FileDesc);
  } else {
    error = 1;
  }

  return error;
}

int
loc_cap32_load_paths()
{
   char *line;
   FILE* fp;
   char  file_name[MAX_PATH+1];
   int len = 100;
   int read;
   char **options;

   snprintf(file_name, MAX_PATH, "%s/set/default_paths.set", CPC.cpc_home_dir);
   fp = fopen(file_name, "r");
   if (fp == (FILE *)0 ) return 0;
   line = malloc(MAX_PATH*2);

   while ((read = getline(&line, &len, fp)) != -1) {
      options = str_split(line, '=');
      if(options){
         if(atoi(options[0]) == DEFAULT_DISK_PATH){
            strcpy(CPC.cpc_disk_path, options[1]);
         }
      }
   }

   if(line){
      free(line);
   }

   return 0;

}

int
loc_cap32_load_settings(char *FileName)
{
  char  Buffer[512];
  char *Scan;
  unsigned int Value;
  FILE* FileDesc;

  int cpc_green = CPC.cpc_green;

  FileDesc = fopen(FileName, "r");
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

    if (!strcasecmp(Buffer,"model")) CPC.model = Value;
    else
    if (!strcasecmp(Buffer,"ram_size")) CPC.ram_size = Value;
    else
    if (!strcasecmp(Buffer,"speed")) CPC.speed = Value;
    else
    if (!strcasecmp(Buffer,"cpc_speed_limiter")) CPC.cpc_speed_limiter = Value;
    else
    if (!strcasecmp(Buffer,"cpc_view_fps")) CPC.cpc_view_fps = Value;
    else
    if (!strcasecmp(Buffer,"cpc_green")) cpc_green = Value;
    else
    if (!strcasecmp(Buffer,"snd_playback_rate")) CPC.snd_playback_rate = Value;
    else
    if (!strcasecmp(Buffer,"snd_volume")) CPC.snd_volume = Value;
    else
    if (!strcasecmp(Buffer,"psp_cpu_clock")) CPC.psp_cpu_clock = Value;
    else
    if (!strcasecmp(Buffer,"psp_skip_max_frame")) CPC.psp_skip_max_frame = Value;
    else
    if (!strcasecmp(Buffer,"cpc_render_mode")) CPC.cpc_render_mode = Value;
    else
    if (!strcasecmp(Buffer,"cpc_display_border")) CPC.cpc_display_border = Value;
    else
    if (!strcasecmp(Buffer,"psp_explore_disk")) CPC.psp_explore_disk = Value;
    else
    if (!strcasecmp(Buffer,"cpc_reset_load_disk")) CPC.cpc_reset_load_disk = Value;
    else
    if (!strcasecmp(Buffer,"psp_kbd_skin")) psp_kbd_skin = Value;
  }

  fclose(FileDesc);

  if (CPC.model > 2) {
     CPC.model = 2;
  }
  if (CPC.ram_size > 576) {
     CPC.ram_size = 576;
  } else if ((CPC.model == 2) && (CPC.ram_size < 128)) {
     CPC.ram_size = 128; // minimum RAM size for CPC 6128 is 128KB
  }
  if ((CPC.speed < MIN_SPEED_SETTING) || (CPC.speed > MAX_SPEED_SETTING)) {
    CPC.speed = DEF_SPEED_SETTING;
  }
  if ((CPC.scr_intensity < 5) || (CPC.scr_intensity > 15)) {
     CPC.scr_intensity = 10;
  }
  if (CPC.snd_playback_rate > (MAX_FREQ_ENTRIES-1)) {
     CPC.snd_playback_rate = 3;
  }

  if (CPC.cpc_green != cpc_green) {
    cap32_change_green_mode(cpc_green);
  }
  cap32_change_render_mode(CPC.cpc_render_mode);

  return 0;
}

void
cap32_default_settings()
{
  CPC.model = 2; // CPC 6128
  CPC.ram_size = 128;
  CPC.speed = DEF_SPEED_SETTING; // original CPC speed
  CPC.cpc_speed_limiter = 50;
  CPC.cpc_view_fps = 0;
  cap32_change_green_mode(0);
  CPC.snd_playback_rate = 2;
  CPC.snd_bits = 1;
  CPC.snd_stereo = 1;
  CPC.snd_volume = 80;

  CPC.psp_cpu_clock       = 0;
  CPC.psp_skip_max_frame  = 0;
  CPC.cpc_render_mode     = CPC_RENDER_FAST;
  CPC.cpc_display_border = 1;
  CPC.psp_explore_disk = CPC_EXPLORE_FULL_AUTO;
  CPC.cpc_reset_load_disk = 1;

  cap32_change_render_mode(CPC.cpc_render_mode);
}

int
cap32_load_settings(void)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "./set/%s.set", CPC.cpc_save_name);
  error = loc_cap32_load_settings(FileName);

  return error;
}

int
cap32_load_file_settings(char *FileName)
{
  return loc_cap32_load_settings(FileName);
}

int
cap32_initialize()
{
  char chPath[MAX_PATH+1];

  memset(&CPC, 0, sizeof(CPC));
  strcpy(CPC.cpc_home_dir, ".");

  CPC.model                 = 2; // CPC 6128
  CPC.jumpers               = CPC_jumpers;
  CPC.ram_size              = 128;
  CPC.speed                 = DEF_SPEED_SETTING; // original CPC speed
  CPC.cpc_speed_limiter     = 50;
  CPC.cpc_auto_fire         = 0;
  CPC.cpc_auto_fire_period  = 6;
  CPC.cpc_auto_fire_pressed = 0;
  CPC.auto_pause            = 1;
  CPC.joysticks             = 0;

  CPC.scr_fs_width          = 320;
  CPC.scr_fs_height         = 240;
  CPC.scr_fs_bpp            = 16;
  CPC.scr_style             = 0;
  CPC.cpc_view_fps          = 0;
  CPC.cpc_green             = 0;

  CPC.scr_intensity         = 10;
  CPC.scr_window            = 0;
  CPC.snd_enabled           = 1;
  CPC.snd_playback_rate     = 2;
  CPC.snd_bits              = 1;
  CPC.snd_stereo            = 1;
  CPC.snd_volume            = 80;
  CPC.max_tracksize         = 6144-154;

  strcpy(chPath, "./rom");
  strcpy(CPC.rom_path, chPath);
  strcpy(CPC.rom_file[7], "amsdos.rom"); // insert AMSDOS in slot 7 if the config file does not exist yet

  //LUDO:
  CPC.psp_skip_cur_frame = 0;
  CPC.psp_skip_max_frame = 0;

  strcpy(chPath, "./save");
  strcpy(CPC.cpc_save_path, chPath);

  strcpy(chPath, "./kbd");
  strcpy(CPC.cpc_kbd_path, chPath);

  strcpy(chPath, "./scr");
  strcpy(CPC.psp_screenshot_path, chPath);

#if defined(GCW0_MODE)
   static char *tmp_directory, *home_name;

   home_name = malloc(sizeof(char)*6 + 1);
   strcpy(home_name,".cap32");


   sprintf(CPC.cpc_home_dir, "%s/%s/", getenv("HOME"), home_name);
   mkdir(CPC.cpc_home_dir, 0777);

   tmp_directory = malloc(MAX_PATH + 1);
   sprintf(tmp_directory, "%s/%s/save/", getenv("HOME"), home_name);
   strcpy(CPC.cpc_save_path, tmp_directory);
   mkdir(CPC.cpc_save_path, 0777);
   free (tmp_directory);

   tmp_directory = malloc(MAX_PATH + 1);
   sprintf(tmp_directory, "%s/%s/set/", getenv("HOME"), home_name);
   strcpy(CPC.cpc_set_path, tmp_directory);
   mkdir(CPC.cpc_set_path, 0777);
   free (tmp_directory);

   tmp_directory = malloc(MAX_PATH + 1);
   sprintf(tmp_directory, "%s/%s/txt/", getenv("HOME"), home_name);
   strcpy(CPC.cpc_txt_path, tmp_directory);
   mkdir(CPC.cpc_txt_path, 0777);
   free (tmp_directory);

   tmp_directory = malloc(MAX_PATH + 1);
   sprintf(tmp_directory, "%s/%s/kbd/", getenv("HOME"), home_name);
   strcpy(CPC.cpc_kbd_path, tmp_directory);
   mkdir(CPC.cpc_kbd_path, 0777);
   free (tmp_directory);

   tmp_directory = malloc(MAX_PATH + 1);
   sprintf(tmp_directory, "%s/%s/joy/", getenv("HOME"), home_name);
   strcpy(CPC.cpc_joy_path, tmp_directory);
   mkdir(CPC.cpc_joy_path, 0777);
   free (tmp_directory);

   tmp_directory = malloc(MAX_PATH + 1);
   sprintf(tmp_directory, "%s/%s/cht/", getenv("HOME"), home_name);
   strcpy(CPC.cpc_cht_path, tmp_directory);
   mkdir(CPC.cpc_cht_path, 0777);
   free (tmp_directory);

   tmp_directory = malloc(MAX_PATH + 1);
   sprintf(tmp_directory, "%s/%s/scr/", getenv("HOME"), home_name);
   strcpy(CPC.psp_screenshot_path, tmp_directory);
   mkdir(CPC.psp_screenshot_path, 0777);
   free (tmp_directory);

   tmp_directory = malloc(MAX_PATH + 1);
   sprintf(tmp_directory, "%s/%s/rom/", getenv("HOME"), home_name);
   mkdir(CPC.rom_path, 0777);
   free (tmp_directory);

   tmp_directory = malloc(MAX_PATH + 1);
   //sprintf(tmp_directory, "%s/%s/bios/", getenv("HOME"), home_name);
   sprintf(tmp_directory, "./bios/");
   strcpy(CPC.cpc_bios_path, tmp_directory);
   mkdir(CPC.cpc_bios_path, 0777);
   free (tmp_directory);

   tmp_directory = malloc(MAX_PATH + 1);
   sprintf(tmp_directory, "%s/%s/disk/", getenv("HOME"), home_name);
   strcpy(CPC.cpc_disk_path, tmp_directory);
   mkdir(CPC.cpc_disk_path, 0777);
   free (tmp_directory);

   tmp_directory = malloc(MAX_PATH + 1);
   sprintf(tmp_directory, "%s/%s/snap/", getenv("HOME"), home_name);
   strcpy(CPC.cpc_snap_path, tmp_directory);
   mkdir(CPC.cpc_snap_path, 0777);
   free (tmp_directory);

#endif;

   loc_cap32_load_paths();

  CPC.psp_screenshot_id = 0;
  CPC.cpc_render_mode = CPC_RENDER_ULTRA;
  CPC.cpc_display_border = 1;
  CPC.psp_explore_disk   = CPC_EXPLORE_FULL_AUTO;
  CPC.cpc_reset_load_disk = 1;
  CPC.psp_reverse_analog = 0;

  return 0;
}

int
cap32_load_default()
{
  update_save_name("");

  psp_run_load_file();

  cap32_default_settings();
  psp_joy_default_settings();
  psp_kbd_default_settings();

  cap32_load_settings();
  cap32_kbd_load();
  cap32_joy_load();
  cap32_load_cheat();

  return 0;
}

void
cap32_new_frame()
{
  psp_update_keys();
  cap32_apply_cheats();

  if (CPC.cpc_view_fps) {
    cap32_update_fps();
  }
  if (CPC.psp_skip_cur_frame == 0) {
    video_display(); // update PC display
    CPC.psp_skip_cur_frame = CPC.psp_skip_max_frame;
  } else if (CPC.psp_skip_max_frame) {
    CPC.psp_skip_cur_frame--;
  }
  CPC.scr_base = (dword *)((u8*)cpc_surface->pixels + CPC.scr_pixels_offs);
  CPC.scr_min  = (dword *)(cpc_surface->pixels);
  CPC.scr_max  = CPC.scr_min + CPC.scr_bps * CPC_VISIBLE_SCR_HEIGHT;
}

void
cap32_main_loop()
{
  z80_execute(); // run the emulation until an exit condition is met
}

int
SDL_main(int argc, char **argv)
{
  cap32_initialize(); // retrieve the emulator configuration

  psp_sdl_init();

  cap32_load_default();

  z80_init_tables(); // init Z80 emulation
  video_init_tables(); // generate the byte-to-pixels translation tables

  if (cpc_kbd_init()) {
     fprintf(stderr, "input_init() failed. Aborting.\n");
     psp_sdl_exit(1);
  }

  if (video_init()) {
     fprintf(stderr, "video_init() failed. Aborting.\n");
     psp_sdl_exit(1);
  }

  if (audio_init()) {
     fprintf(stderr, "audio_init() failed. Disabling sound.\n");
     CPC.snd_enabled = 0; // disable sound emulation
  }

  cap32_change_render_mode(CPC.cpc_render_mode);

  if (emulator_init()) {
     psp_sdl_exit(1);
  }

  memset(&driveA, 0, sizeof(t_drive)); // clear disk drive A data structure

  audio_resume();

  psp_sdl_black_screen();

  cap32_main_loop();

  return 0;
}
