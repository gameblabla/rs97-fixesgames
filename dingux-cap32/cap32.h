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

#ifndef CAP32_H
#define CAP32_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#include <SDL.h>
# ifndef CLK_TCK
#   define CLK_TCK  CLOCKS_PER_SEC
# endif

#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int

#include "gp2x_psp.h"
#include "psp_sdl.h"

#define MAX_PATH 256
#define MAX_FREQ_ENTRIES 5

#define CYCLE_COUNT_INIT  80000 // 4MHz divided by 50Hz = number of CPU cycles per frame

#define MIN_SPEED_SETTING 2
#define MAX_SPEED_SETTING 32
#define DEF_SPEED_SETTING 4

//#define CPC_SCR_WIDTH 1024 // max width
//#define CPC_SCR_HEIGHT 312 // max height

#define CPC_SCR_WIDTH  384
#define CPC_SCR_HEIGHT 312
#define CPC_VISIBLE_SCR_WIDTH  320 // visible width: 4+40+4 * 8
#define CPC_VISIBLE_SCR_HEIGHT 240

# define CPC_WIDTH  CPC_VISIBLE_SCR_WIDTH
# define CPC_HEIGHT CPC_VISIBLE_SCR_HEIGHT
# define SNAP_WIDTH   (CPC_WIDTH/3)
# define SNAP_HEIGHT  (CPC_HEIGHT/3)

#define CPC_RENDER_ULTRA        0
#define CPC_RENDER_FAST         1
#define CPC_RENDER_NORMAL       2
#define CPC_LAST_RENDER         2

#define CPC_EXPLORE_MANUAL    0
#define CPC_EXPLORE_LOAD      1
#define CPC_EXPLORE_AUTO      2
#define CPC_EXPLORE_FULL_AUTO 3

#define ICN_DISK_WIDTH 14
#define ICN_DISK_HEIGHT 16
#define ICN_TAPE_WIDTH 18
#define ICN_TAPE_HEIGHT 13

#define VOC_THRESHOLD 128

// CRTC flags
#define VS_flag      1
#define HS_flag      2
#define HDT_flag     4
#define VDT_flag     8
#define HT_flag      16
#define VT_flag      32
#define MR_flag      64
#define VTadj_flag   128
#define VSf_flag     256

// FDC constants
#define DSK_BPTMAX      8192
#define DSK_TRACKMAX    102   // max amount that fits in a DSK header
#define DSK_SIDEMAX     2
#define DSK_SECTORMAX   29    // max amount that fits in a track header

#define FDC_TO_CPU      0
#define CPU_TO_FDC      1

#define CMD_PHASE       0
#define EXEC_PHASE      1
#define RESULT_PHASE    2

#define SKIP_flag       1     // skip sectors with DDAM/DAM
#define SEEKDRVA_flag   2     // seek operation has finished for drive A
#define SEEKDRVB_flag   4     // seek operation has finished for drive B
#define RNDDE_flag      8     // simulate random DE sectors
#define OVERRUN_flag    16    // data transfer timed out
#define SCAN_flag       32    // one of the three scan commands is active
#define SCANFAILED_flag 64    // memory and sector data does not match
#define STATUSDRVA_flag 128   // status change of drive A
#define STATUSDRVB_flag 256   // status change of drive B

#define true            1
#define false           0
#ifndef __cplusplus
 typedef unsigned char bool;
#endif

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned long long ddword;


typedef struct {
   char id[8];
   char unused1[8];
   unsigned char version;
   unsigned char AF[2];
   unsigned char BC[2];
   unsigned char DE[2];
   unsigned char HL[2];
   unsigned char R;
   unsigned char I;
   unsigned char IFF0;
   unsigned char IFF1;
   unsigned char IX[2];
   unsigned char IY[2];
   unsigned char SP[2];
   unsigned char PC[2];
   unsigned char IM;
   unsigned char AFx[2];
   unsigned char BCx[2];
   unsigned char DEx[2];
   unsigned char HLx[2];
   unsigned char ga_pen;
   unsigned char ga_ink_values[17];
   unsigned char ga_ROM_config;
   unsigned char ga_RAM_config;
   unsigned char crtc_reg_select;
   unsigned char crtc_registers[18];
   unsigned char upper_ROM;
   unsigned char ppi_A;
   unsigned char ppi_B;
   unsigned char ppi_C;
   unsigned char ppi_control;
   unsigned char psg_reg_select;
   unsigned char psg_registers[16];
   unsigned char ram_size[2];
// version 2 info follows
   unsigned char cpc_model;
   unsigned char last_interrupt;
   unsigned char scr_modes[6];
// version 3 info follows
   unsigned char drvA_DOSfilename[13];
   unsigned char drvB_DOSfilename[13];
   unsigned char cart_DOSfilename[13];
   unsigned char fdc_motor;
   unsigned char drvA_current_track;
   unsigned char drvB_current_track;
   unsigned char drvC_current_track;
   unsigned char drvD_current_track;
   unsigned char printer_data;
   unsigned char psg_env_step;
   unsigned char psg_env_direction;
   unsigned char crtc_type;
   unsigned char crtc_addr[2];
   unsigned char crtc_scanline[2];
   unsigned char crtc_char_count[2];
   unsigned char crtc_line_count;
   unsigned char crtc_raster_count;
   unsigned char crtc_vt_adjust_count;
   unsigned char crtc_hsw_count;
   unsigned char crtc_vsw_count;
   unsigned char crtc_flags[2];
   unsigned char ga_int_delay;
   unsigned char ga_sl_count;
   unsigned char z80_int_pending;
   unsigned char unused2[75];
} t_SNA_header;

typedef struct {
   char id[34];
   char unused1[14];
   unsigned char tracks;
   unsigned char sides;
   unsigned char unused2[2];
   unsigned char track_size[DSK_TRACKMAX*DSK_SIDEMAX];
} t_DSK_header;

typedef struct {
   char id[12];
   char unused1[4];
   unsigned char track;
   unsigned char side;
   unsigned char unused2[2];
   unsigned char bps;
   unsigned char sectors;
   unsigned char gap3;
   unsigned char filler;
   unsigned char sector[DSK_SECTORMAX][8];
} t_track_header;

//LUDO:
# define CPC_MAX_SAVE_STATE 5
# define CPC_MAX_CHEAT      10

#define CPC_CHEAT_NONE    0
#define CPC_CHEAT_ENABLE  1
#define CPC_CHEAT_DISABLE 2

#define CPC_CHEAT_COMMENT_SIZE 25

  typedef struct CPC_cheat_t {
    unsigned char  type;
    unsigned short addr;
    unsigned char  value;
    char           comment[CPC_CHEAT_COMMENT_SIZE];
  } CPC_cheat_t;

  typedef struct CPC_save_t {

    SDL_Surface    *surface;
    char            used;
    char            thumb;
    time_t          date;

  } CPC_save_t;

# define CPC_jumpers 0x1e

typedef struct {
   unsigned int model;
   unsigned int jumpers;
   unsigned int ram_size;
   unsigned int speed;
   unsigned int limit_speed;
   unsigned int auto_pause;
   unsigned int keyboard_line;
   unsigned int joysticks;
   int cycle_count;

   unsigned int scr_fs_width;
   unsigned int scr_fs_height;
   unsigned int scr_fs_bpp;
   unsigned int scr_style;
   unsigned int scr_vsync;
   unsigned int scr_led;
   unsigned int scr_fps;
   unsigned int scr_tube;
   unsigned int scr_intensity;
   unsigned int scr_window;
   unsigned int scr_bpp;
   unsigned int scr_bps;
   unsigned int scr_line_offs;
   unsigned int *scr_base;
   unsigned int *scr_min;
   unsigned int *scr_max;
   unsigned int scr_pixels_offs;
   unsigned int scr_offs;
   unsigned int scr_line;

   unsigned int snd_enabled;
   unsigned int snd_playback_rate;
   unsigned int snd_bits;
   unsigned int snd_stereo;
   unsigned int snd_volume;
   unsigned int snd_buffersize;
   unsigned char *snd_bufferptr;
   union {
      struct {
         unsigned int low;
         unsigned int high;
      };
      int64_t both;
   } snd_cycle_count_init;

   unsigned int max_tracksize;

   char rom_path[MAX_PATH + 1];
   char rom_file[16][MAX_PATH + 1];

   //LUDO:
  char           cpc_home_dir[MAX_PATH];
   char          cpc_save_path[MAX_PATH + 1];
   char          cpc_kbd_path[MAX_PATH + 1];
   char          cpc_set_path[MAX_PATH + 1];
   char          cpc_txt_path[MAX_PATH + 1];
   char          cpc_joy_path[MAX_PATH + 1];
   char          cpc_cht_path[MAX_PATH + 1];
   char          cpc_disk_path[MAX_PATH + 1];
   char          cpc_snap_path[MAX_PATH + 1];
   char          cpc_bios_path[MAX_PATH + 1];
   char          cpc_save_name[MAX_PATH+1];
   CPC_save_t    cpc_save_state[CPC_MAX_SAVE_STATE];
   CPC_cheat_t   cpc_cheat[CPC_MAX_CHEAT];

   int           comment_present;

   char          psp_screenshot_path[MAX_PATH + 1];
   int           psp_screenshot_id;

   unsigned int  psp_cpu_clock;
   unsigned int  psp_skip_cur_frame;
   unsigned int  psp_skip_max_frame;

   int           cpc_green;
   int           cpc_view_fps;
   int           cpc_current_clock;
   int           cpc_current_fps;
   int           psp_active_joystick;
   int           cpc_display_border;
   int           psp_explore_disk;
   int           cpc_reset_load_disk;

   unsigned char cpc_render_mode;
   unsigned char psp_reverse_analog;
   int           cpc_speed_limiter;
   int           cpc_auto_fire;
   int           cpc_auto_fire_pressed;
   int           cpc_auto_fire_period;

} t_CPC;

typedef struct {
   unsigned int flags;
   unsigned int requested_addr;
   unsigned int addr;
   unsigned char char_count;
   unsigned char line_count;
   unsigned char raster_count;
   unsigned char hsw;
   unsigned char hsw_active;
   unsigned char hsw_count;
   unsigned char vsw;
   unsigned char vsw_count;
   unsigned char vt_adjust;
   unsigned char vt_adjust_count;
   unsigned char skew;
   unsigned char max_raster;
   unsigned char last_hdisp;
   unsigned char reg_select;
   unsigned char registers[18];
} t_CRTC;

typedef struct {
   int timeout;
   int motor;
   int led;
   int flags;
   int phase;
   int byte_count;
   int buffer_count;
   int cmd_length;
   int res_length;
   int cmd_direction;
   void (*cmd_handler)(void);
   unsigned char *buffer_ptr;
   unsigned char *buffer_endptr;
   unsigned char command[12];
   unsigned char result[8];
} t_FDC;

typedef struct {
   unsigned char ROM_config;
   unsigned char RAM_bank;
   unsigned char RAM_config;
   unsigned char upper_ROM;
   int           requested_scr_mode;
   int           scr_mode;
   unsigned char pen;
   unsigned char ink_values[17];
   dword         palette[17];
   unsigned char sl_count;
   unsigned char int_delay;
} t_GateArray;

typedef struct {
   unsigned char control;
   unsigned char portA;
   unsigned char portB;
   unsigned char portC;
} t_PPI;

typedef struct {
   union {
      struct {
         unsigned int low;
         unsigned int high;
      };
      int64_t both;
   } cycle_count;
   unsigned char control;
   unsigned char reg_select;
   union {
      unsigned char Index[16];
      struct {
         unsigned char TonALo, TonAHi;
         unsigned char TonBLo, TonBHi;
         unsigned char TonCLo, TonCHi;
         unsigned char Noise;
         unsigned char Mixer;
         unsigned char AmplitudeA, AmplitudeB, AmplitudeC;
         unsigned char EnvelopeLo, EnvelopeHi;
         unsigned char EnvType;
         unsigned char PortA;
         unsigned char PortB;
      };
   } RegisterAY;
   int AmplitudeEnv;
   bool FirstPeriod;
   void (*Synthesizer)(void);
} t_PSG;

typedef struct {
   unsigned int hsw;
   unsigned int hsw_active;
   unsigned int hsw_count;
   unsigned int vsw_count;
   unsigned int hdelay;
   unsigned int vdelay;
   unsigned int scanline;
   unsigned int scanline_min;
   unsigned int frame_completed;
   unsigned int char_count;
   unsigned int hcount;
   unsigned int hstart;
   unsigned int hwidth;
   unsigned int vcount;
   unsigned int vstart;
   unsigned int vheight;
} t_VDU;

typedef struct {
   unsigned char CHRN[4]; // the CHRN for this sector
   unsigned char flags[4]; // ST1 and ST2 - reflects any possible error conditions
   unsigned int size; // sector size in bytes
   unsigned char *data; // pointer to sector data
} t_sector;

typedef struct {
   unsigned int sectors; // sector count for this track
   unsigned int size; // track size in bytes
   unsigned char *data; // pointer to track data
   t_sector sector[DSK_SECTORMAX]; // array of sector information structures
} t_track;

typedef struct {
   unsigned int tracks; // total number of tracks
   unsigned int current_track; // location of drive head
   unsigned int sides; // total number of sides
   unsigned int current_side; // side being accessed
   unsigned int current_sector; // sector being accessed
   unsigned int altered; // has the image been modified?
   unsigned int write_protected; // is the image write protected?
   unsigned int random_DEs; // sectors with Data Errors return random data?
   unsigned int flipped; // reverse the side to access?
   t_track track[DSK_TRACKMAX][DSK_SIDEMAX]; // array of track information structures
} t_drive;

typedef struct {
   char *pchZipFile;
   char *pchExtension;
   char *pchFileNames;
   char *pchSelection;
   int iFiles;
   unsigned int dwOffset;
} t_zip_info;

typedef struct {
   unsigned char label[40]; // label to display in options dialog
   unsigned int tracks; // number of tracks
   unsigned int sides; // number of sides
   unsigned int sectors; // sectors per track
   unsigned int sector_size; // sector size as N value
   unsigned int gap3_length; // GAP#3 size
   unsigned char filler_byte; // default byte to use
   unsigned char sector_ids[2][16]; // sector IDs
} t_disk_format;


  extern t_CPC CPC;
  extern dword freq_table[MAX_FREQ_ENTRIES];
  extern int cpc_dsk_system;

// cap32.cpp
void emulator_reset(bool bolMF2Reset);

// render mode
void cap32_change_render_mode(int new_render_mode);

// fdc.c
void fdc_write_data(unsigned char val);
unsigned char fdc_read_status(void);
unsigned char fdc_read_data(void);

// psg.c
void SetAYRegister(int Num, unsigned char Value);
void Calculate_Level_Tables(void);
void ResetAYChipEmulation(void);
void InitAYCounterVars(void);
void InitAY(void);

// Load
extern int cap32_disk_load(char *FileName, char drive, int zip_format);
extern int cap32_snapshot_load(char *FileName, int zip_format);
extern int cap32_snapshot_del_slot(int save_id);
extern void audio_pause(void);
extern void cap32_save_back_to_blit();
extern void audio_resume(void);

extern int cap32_load_settings(void);
extern int cap32_kbd_save(void);
extern void cap32_kbd_load(void);
extern int cap32_load_file_settings(char *FileName);
extern void cap32_swap_sound_buffer(void);
extern int cap32_snapshot_load_slot(int load_id);
extern int cap32_snapshot_save_slot(int save_id);
extern void reset_save_name();
extern void cap32_change_ram_size(int new_ram_size);
extern int cap32_save_settings(void);
extern void cap32_default_settings();
extern void cap32_treat_command_key(int cpc_idx);
extern void cap32_change_green_mode(int new_green);
extern int cap32_load_cheat();
extern int cap32_save_cheat(void);

extern void load_thumb_list();
extern void load_comment_list();
extern int load_thumb_if_exists(char *Name);
extern char* load_comment_if_exists(char *Name);
extern int cap32_load_file_cheat(char *FileName);
#endif
