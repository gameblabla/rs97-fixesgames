/* Caprice32 - Amstrad CPC Emulator
   (c) Copyright 1997-2004 Ulrich Doewich

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

/* Hitachi HD6845S CRT Controller (CRTC Type 0) emulation
   (c) Copyright 1997-2004 Ulrich Doewich

   Oct 16, 2000 - 23:12 started conversion from assembly to C
   Oct 17, 2000 - 19:25 finished converting main CRTC update loop
   Oct 17, 2000 - 22:04 added framework for mode draw handlers
   Oct 25, 2000 - 22:03 changed all CRTC counters to be only 8 bits wide; fixed address calculation
   Oct 30, 2000 - 19:03 fixed CPC screen address line advancement (test should have been for a _reset_ bit!)
   Mar 20, 2001 - 16:00 added draw_mode2
   Jun 20, 2001 - 23:24 added drawing routines for 32bpp video modes
   Jul 04, 2001 - 22:28 16bpp rendering; updated 8bpp code with VDU visible region limiting
   Sep 26, 2002 - 22:39 moved rendering code to separate files; added line doubling (in software) code
   Oct 07, 2002 - 21:58 removed the CPC.scr_max test in write_video_data; added support for variable line spacing

   May 23, 2004 - 17:38 added some preliminary VDU frame cropping to reduce the amount of data written to the video buffer
   May 24, 2004 - 00:44 moved the write_video_data code into the body of access_video_memory
*/

#include "cap32.h"
#include "crtc.h"
#include "z80.h"

extern t_CPC CPC;
extern t_CRTC CRTC;
extern t_GateArray GateArray;
extern t_VDU VDU;
extern t_z80regs z80;

extern byte *pbRAM;
extern byte mode0_table[512], mode1_table[1024];

static inline
void draw16bpp_border_half(void)
{
   if (CPC.cpc_display_border) {
     dword colour;
     register dword *mem_ptr;
     colour = GateArray.palette[16];
     mem_ptr = (dword *)(CPC.scr_base + CPC.scr_offs); // PC screen buffer address
     if ((mem_ptr < CPC.scr_min) || ((mem_ptr+4) > CPC.scr_max)) return;

     *mem_ptr++ = colour;
     *mem_ptr++ = colour;
     *mem_ptr++ = colour;
     *mem_ptr = colour;
   }
   CPC.scr_offs += 4; // update PC screen buffer address
}

static inline void 
draw16bpp_mode0_half(u16 addr)
{
# if 0
   byte idx;
   register dword *mem_ptr;
   dword val;

   mem_ptr = CPC.scr_base + CPC.scr_offs; // PC screen buffer address
   idx = *(pbRAM + addr); // grab first CPC screen memory byte
   val = GateArray.palette[mode0_table[(idx*2)]];
   *mem_ptr = val; // write one pixels
   val = GateArray.palette[mode0_table[(idx*2)+1]];
   *(mem_ptr+1) = val;

   idx = *(pbRAM + ((addr+1)&0xffff)); // grab second CPC screen memory byte
   val = GateArray.palette[mode0_table[(idx*2)]];
   *(mem_ptr+2) = val;
   val = GateArray.palette[mode0_table[(idx*2)+1]];
   *(mem_ptr+3) = val;
   CPC.scr_offs += 4; // update PC screen buffer address
# endif
   //ZX:
   dword *mem_ptr = CPC.scr_base + CPC.scr_offs; // PC screen buffer address
   if ((mem_ptr < CPC.scr_min) || ((mem_ptr+4) > CPC.scr_max)) return;

   byte  *mem_ram = pbRAM + addr;
   word   idx1 = (*mem_ram++) * 2; // grab first CPC screen memory byte
   word   idx2 = (*mem_ram  ) * 2;
   byte  *mem_mode0_1 = mode0_table + idx1;
   byte  *mem_mode0_2 = mode0_table + idx2;

   *mem_ptr++ = GateArray.palette[*mem_mode0_1++];
   *mem_ptr++ = GateArray.palette[*mem_mode0_1  ];
   *mem_ptr++ = GateArray.palette[*mem_mode0_2++];
   *mem_ptr   = GateArray.palette[*mem_mode0_2  ];

   CPC.scr_offs += 4; // update PC screen buffer address
}

static inline 
void draw16bpp_mode1_half(u16 addr)
{
# if 0 // ORIG
   byte idx;
   register dword *mem_ptr;
   reg_pair val;

   mem_ptr = CPC.scr_base + CPC.scr_offs; // PC screen buffer address
   idx = *(pbRAM + addr); // grab first CPC screen memory byte
   val.w.l = GateArray.palette[mode1_table[(idx*4)]];
   val.w.h = GateArray.palette[mode1_table[(idx*4)+1]];
   *mem_ptr = val.d; // write one pixels
   val.w.l = GateArray.palette[mode1_table[(idx*4)+2]];
   val.w.h = GateArray.palette[mode1_table[(idx*4)+3]];
   *(mem_ptr+1) = val.d;

   idx = *(pbRAM + ((addr+1)&0xffff)); // grab second CPC screen memory byte
   val.w.l = GateArray.palette[mode1_table[(idx*4)]];
   val.w.h = GateArray.palette[mode1_table[(idx*4)+1]];
   *(mem_ptr+2) = val.d;
   val.w.l = GateArray.palette[mode1_table[(idx*4)+2]];
   val.w.h = GateArray.palette[mode1_table[(idx*4)+3]];
   *(mem_ptr+3) = val.d;
   CPC.scr_offs += 4; // update PC screen buffer address
# else
   //ZX:
   dword *mem_ptr = CPC.scr_base + CPC.scr_offs; // PC screen buffer address
   if ((mem_ptr < CPC.scr_min) || ((mem_ptr+4) > CPC.scr_max)) return;

   byte  *mem_ram = pbRAM + addr;
   word   idx1 = (*mem_ram++) * 4; // grab first CPC screen memory byte
   word   idx2 = (*mem_ram  ) * 4;
   byte  *mem_mode1_1 = mode1_table + idx1;
   byte  *mem_mode1_2 = mode1_table + idx2;

   reg_pair val;
   val.w.l = GateArray.palette[*mem_mode1_1++];
   val.w.h = GateArray.palette[*mem_mode1_1++];
   *mem_ptr++ = val.d;
   val.w.l = GateArray.palette[*mem_mode1_1++];
   val.w.h = GateArray.palette[*mem_mode1_1  ];
   *mem_ptr++ = val.d;

   val.w.l = GateArray.palette[*mem_mode1_2++];
   val.w.h = GateArray.palette[*mem_mode1_2++];
   *mem_ptr++ = val.d;
   val.w.l = GateArray.palette[*mem_mode1_2++];
   val.w.h = GateArray.palette[*mem_mode1_2  ];
   *mem_ptr = val.d;

   CPC.scr_offs += 4; // update PC screen buffer address
# endif
}

static inline void
draw16bpp_mode2_half(u16 addr)
{
   byte pat;
   register dword *mem_ptr;
   dword pen_on, pen_off;
   reg_pair val;

   mem_ptr = CPC.scr_base + CPC.scr_offs; // PC screen buffer address
   if ((mem_ptr < CPC.scr_min) || ((mem_ptr+4) > CPC.scr_max)) return;

   pen_on = GateArray.palette[1];
   pen_off = GateArray.palette[0];

   pat = *(pbRAM + addr); // grab first CPC screen memory byte
   val.w.l = (pat & 0x80) ? pen_on : pen_off;
   val.w.h = (pat & 0x20) ? pen_on : pen_off;
   *mem_ptr = val.d; // write four pixels
   val.w.l = (pat & 0x08) ? pen_on : pen_off;
   val.w.h = (pat & 0x02) ? pen_on : pen_off;
   *(mem_ptr+1) = val.d;
   pat = *(pbRAM + ((addr+1)&0xffff)); // grab second CPC screen memory byte
   val.w.l = (pat & 0x80) ? pen_on : pen_off;
   val.w.h = (pat & 0x20) ? pen_on : pen_off;
   *(mem_ptr+2) = val.d;
   val.w.l = (pat & 0x08) ? pen_on : pen_off;
   val.w.h = (pat & 0x02) ? pen_on : pen_off;
   *(mem_ptr+3) = val.d;
   CPC.scr_offs += 4; // update PC screen buffer address
}

# if 0
static void (*draw16bpp_mode[3])(u16 addr) = {
   draw16bpp_mode0_half, draw16bpp_mode1_half, draw16bpp_mode2_half
};
# else
static void (*draw16bpp_mode[3])(u16 addr) = {
   0, draw16bpp_mode1_half, draw16bpp_mode2_half
};
# endif

static inline void
access_video_ht()
{
         CRTC.flags &= ~HT_flag;
         CRTC.hsw_active = CRTC.hsw;
         VDU.hsw_active = VDU.hsw;
         CRTC.char_count = 0; // reset cc
// next raster ----------------------------------------------------------------
         CRTC.raster_count += 8; // advance rc by one
         if (CRTC.flags & VS_flag) { // in VSYNC?
            CRTC.vsw_count++; // update width counter
            if (CRTC.vsw_count >= CRTC.vsw) { // reached end of VSYNC?
               CRTC.flags = (CRTC.flags & ~VS_flag) | VSf_flag; // reset VSYNC, set 'just finished'
            }
         }
         if (CRTC.flags & MR_flag) { // reached maximum raster address on last rc?
            CRTC.flags &= ~MR_flag;
            CRTC.raster_count = 0; // reset rc
            if (!(CRTC.flags & HDT_flag)) { // HDISPTMG still on (i.e. R01 > R00)?
               CRTC.addr += CRTC.last_hdisp * 2; // advance CPC screen address to next line
            }
            CRTC.line_count++; // update line count
            CRTC.line_count &= 127; // limit to 7 bits
         }
         if (CRTC.vt_adjust_count) { // vertical total adjust active?
            if (--CRTC.vt_adjust_count == 0) { // done with vta?
               CRTC.flags = (CRTC.flags & ~VSf_flag) | VDT_flag; // enable VDISPTMG
               CRTC.raster_count = 0; // reset rc
               CRTC.line_count = 0; // reset lc
               CRTC.addr = CRTC.requested_addr; // update start of CPC screen address
            }
         }
         if (CRTC.flags & VT_flag) { // reached vertical total on last lc?
            CRTC.flags &= ~VT_flag;
            if (CRTC.vt_adjust) { // do a vertical total adjust?
               CRTC.vt_adjust_count = CRTC.vt_adjust; // init vta counter
            } else {
               CRTC.flags = (CRTC.flags & ~VSf_flag) | VDT_flag; // enable VDISPTMG
               CRTC.raster_count = 0; // reset rc
               CRTC.line_count = 0; // reset lc
               CRTC.addr = CRTC.requested_addr; // update start of CPC screen address
            }
         }
         if (CRTC.raster_count == CRTC.max_raster) { // rc = maximum raster address?
            CRTC.flags |= MR_flag; // process max raster address on next rc
            if (!CRTC.vt_adjust_count) { // no vertical total adjust?
               if (CRTC.line_count == CRTC.registers[4]) { // lc = vertical total?
                  CRTC.flags |= VT_flag; // takes effect on next lc
               }
            }
         }
         if (CRTC.line_count == CRTC.registers[6]) { // lc = vertical displayed?
            CRTC.flags &= ~VDT_flag; // disable VDISPTMG
         }
         if (CRTC.line_count == CRTC.registers[7]) { // lc = vertical sync position?
            if (!(CRTC.flags & (VSf_flag | VS_flag))) { // not in VSYNC?
               CRTC.flags |= VS_flag;
               CRTC.vsw_count = 0; // clear vsw counter
               VDU.vdelay = 2; // GA delays vsync by 2 scanlines
               VDU.vsw_count = 4; // GA vsync is always 4 scanlines long
               GateArray.int_delay = 2; // arm GA two scanlines interrupt delay
            }
         }
// ----------------------------------------------------------------------------
         CRTC.flags |= HDT_flag; // enable horizontal display
}

static inline void
access_video_hs()
{

// check hsw ------------------------------------------------------------------
         if (VDU.hdelay == 2) { // ready to trigger VDU HSYNC?
            if (--VDU.hsw_count == 0) {
               if (CPC.scr_line++ < CPC_SCR_HEIGHT) {
                  if (VDU.vcount) { // in the visible portion of the screen?
                     CPC.scr_base += CPC.scr_line_offs; // advance to next line
                  }
               }
               CPC.scr_offs = 0;
               VDU.char_count = 0;
               VDU.hdelay++; // prevent reentry
            }
         } else {
            VDU.hdelay++; // update delay counter
         }
         if (--CRTC.hsw_count == 0) { // end of HSYNC?
// hsw end --------------------------------------------------------------------
            CRTC.flags &= ~HS_flag; // reset HSYNC
            GateArray.scr_mode = GateArray.requested_scr_mode; // execute mode change
            VDU.scanline++;
            if (VDU.vdelay) { // monitor delaying VSYNC?
               VDU.vdelay--;
            }
            if (VDU.vdelay == 0) { // enter monitor VSYNC?
               if (VDU.vsw_count) { // still in monitor VSYNC?
                  if (--VDU.vsw_count == 0) { // done with monitor VSYNC?
                     if (VDU.scanline > VDU.scanline_min) { // above minimum scanline count?
                        CPC.scr_offs = 0;
                        CPC.scr_line = 0;
                        cap32_new_frame();
                     }
                  }
               }
            }
// GA interrupt trigger -------------------------------------------------------
            GateArray.sl_count++; // update GA scanline counter
            if (GateArray.int_delay) { // delay on VSYNC?
               if (--GateArray.int_delay == 0) { // delay expired?
                  if (GateArray.sl_count >= 32) { // counter above save margin?
                     z80.int_pending = 1; // queue interrupt
                  }
                  GateArray.sl_count = 0;
               }
            }
            if (GateArray.sl_count == 52) { // trigger interrupt?
               z80.int_pending = 1; // queue interrupt
               GateArray.sl_count = 0; // clear counter
            }
         }
}

void 
access_video_memory(int repeat_count)
{
   do {
      CRTC.char_count++; // update character count (cc)
      VDU.char_count++;
      if (CRTC.flags & HT_flag) { // reached horizontal total on last cc?
        access_video_ht();
      } else
      if (CRTC.flags & HS_flag) { // in horizontal sync?
        access_video_hs();
      }
      if (CRTC.char_count == CRTC.registers[2]) { // cc = horizontal sync position?
        if (CRTC.hsw_active) { // allow HSYNCs?
            CRTC.flags |= HS_flag; // set HSYNC
            CRTC.hsw_count = CRTC.hsw_active; // load hsw counter
            VDU.hdelay = 0; // clear VDU 2 chars HSYNC delay
            VDU.hsw_count = VDU.hsw_active; // load VDU hsw counter
        }
      }
      if (CRTC.char_count == CRTC.registers[1]) { // cc = horizontal displayed?
        CRTC.flags &= ~HDT_flag; // disable horizontal display
        CRTC.last_hdisp = CRTC.registers[1]; // save width for line advancement
      }
      if (CRTC.char_count == CRTC.registers[0]) { // cc = horizontal total?
        CRTC.flags |= HT_flag; // takes effect on next cc
      }
      if (VDU.hcount) {
         if ((CRTC.flags & (HDT_flag|VDT_flag)) == (HDT_flag|VDT_flag)) { // DISPTMG enabled?
            if (CRTC.skew) {
               CRTC.skew--;
               draw16bpp_border_half();
            } else {
               word a = CRTC.addr + (CRTC.char_count << 1);
               if (a & 0x2000) a += 0x4000;
               a &= 0xc7ff;
               a |= (CRTC.raster_count & 0x38) << 8;

               if (! GateArray.scr_mode) {
                  draw16bpp_mode0_half(a); //ZX: inline mode0 : used in most of games
               } else draw16bpp_mode[GateArray.scr_mode](a);
            }
         } else {
            draw16bpp_border_half();
         }
         VDU.hcount--;

      } else
      if (VDU.char_count == VDU.hstart) {
        if (VDU.vcount) {
           VDU.hcount = VDU.hwidth;
           VDU.vcount--;
        } else {
           if (CPC.scr_line == VDU.vstart) {
              VDU.vcount = VDU.vheight;
           }
        }
      }

   } while (--repeat_count > 0);
}


static void 
render16bpp_blit_normal()
{
  u32* src_pixel = (u32*)blit_surface->pixels;
  u32* dst_pixel = (u32*)back_surface->pixels;

  int h = PSP_SDL_SCREEN_HEIGHT;
  while (h > 0) {
    int w = PSP_SDL_SCREEN_WIDTH >> 1;
    while (w > 0) {
      *dst_pixel++ = *src_pixel++;
      w--;
    }
    src_pixel += ((CPC_SCR_WIDTH - PSP_SDL_SCREEN_WIDTH) >> 1);
    h--;
  }
}

void
render16bpp_blit()
{
  if (CPC.cpc_render_mode == CPC_RENDER_NORMAL) render16bpp_blit_normal();
}
