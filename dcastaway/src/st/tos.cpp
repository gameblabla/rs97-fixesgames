/*
  Hatari - Modified by Chui.

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  Load TOS image file into ST memory, fix/setup for emulator.

  The Atari ST TOS needs to be patched to help with emulation. Eg, it references
  the MMU chip to set memory size. This is patched to the sizes we need without
  the complicated emulation of hardware which is not needed (as yet). We also
  patch DMA devices and Hard Drives.
  NOTE: TOS versions 1.06 and 1.62 were not designed for use on a real STfm.
  These were for the STe machine ONLY. They access the DMA/Microwire addresses
  on boot-up which (correctly) cause a bus-error on Hatari as they would in a
  real STfm. If a user tries to select any of these images we bring up an error.
*/
char TOS_rcsid[] = "Hatari $Id: tos.c,v 1.20 2004/04/23 15:33:59 thothy Exp $";

#include <SDL_endian.h>

#include "dcastaway.h"
#include "config.h"
#include "st.h"
#include "mem.h"
#include "m68k_intrf.h"

unsigned short int TosVersion;          /* eg, 0x0100, 0x0102 */
short TosCountry;

/* This structure is used for patching the TOS ROMs */
typedef struct
{
  Uint16 Version;       /* TOS version number */
  Sint16 Country;       /* TOS country code: -1 if it does not matter, 0=US, 1=Germany, 2=France, etc. */
  Uint32 Address;       /* Where the patch should be applied */
  Uint32 OldData;       /* Expected first 4 old bytes */
  Uint32 Size;          /* Length of the patch */
  void *pNewData;       /* Pointer to the new bytes */
} TOS_PATCH;

static Uint8 pRtsOpcode[] = { 0x4E, 0x75 };  /* 0x4E75 = RTS */
static Uint8 pNopOpcodes[] = { 0x4E, 0x71, 0x4E, 0x71, 0x4E, 0x71, 0x4E, 0x71,
        0x4E, 0x71, 0x4E, 0x71, 0x4E, 0x71, 0x4E, 0x71, 0x4E, 0x71, 0x4E, 0x71,
        0x4E, 0x71, 0x4E, 0x71, 0x4E, 0x71, 0x4E, 0x71 };  /* 0x4E71 = NOP */
static Uint8 pMouseOpcode[] = { 0xD3, 0xC1 };  /* "ADDA.L D1,A1" (instead of "ADDA.W D1,A1") */
static Uint8 pRomCheckOpcode[] = { 0x60, 0x00, 0x00, 0x98 };  /* BRA $e00894 */
static Uint8 pBraOpcode[] = { 0x60 };  /* 0x60XX = BRA */

/* The patches for the TOS: */
static TOS_PATCH TosPatches[] =
{
//{ 0x100, -1, 0xFC0D60, 0x4E56FFF0, 2, pRtsOpcode },
//{ 0x100, -1, 0xFC1384, 0x4EB900FC, 6, pNopOpcodes },
  { 0x100, -1, 0xFC03D6, 0x610000D0, 4, pNopOpcodes }, 

//{ 0x102, -1, 0xFC0F44, 0x4E56FFF0, 2, pRtsOpcode },
//{ 0x102, -1, 0xFC1568, 0x4EB900FC, 6, pNopOpcodes },
//{ 0x102, -1, 0xFC0302, 0x42B90000, 6, pNopOpcodes },
  { 0x102, -1, 0xFC0472, 0x610000E4, 4, pNopOpcodes },
  { 0x102, 0, 0xFD0030, 0xD2C147F9, 2, pMouseOpcode },
  { 0x102, 1, 0xFD008A, 0xD2C147F9, 2, pMouseOpcode },
  { 0x102, 2, 0xFD00A8, 0xD2C147F9, 2, pMouseOpcode },
  { 0x102, 3, 0xFD0030, 0xD2C147F9, 2, pMouseOpcode },
  { 0x102, 6, 0xFCFEF0, 0xD2C147F9, 2, pMouseOpcode },
  { 0x102, 8, 0xFCFEFE, 0xD2C147F9, 2, pMouseOpcode },

//{ 0x104, -1, 0xFC16BA, 0x4E56FFF0, 2, pRtsOpcode },
//{ 0x104, -1, 0xFC1CCE, 0x4EB900FC, 6, pNopOpcodes },
//{ 0x104, -1, 0xFC02E6, 0x42AD04C2, 4, pNopOpcodes },
  { 0x104, -1, 0xFC0466, 0x610000E4, 4, pNopOpcodes },

//{ 0x205, -1, 0xE002FC, 0x42B804C2, 4, pNopOpcodes },
  { 0x205, -1, 0xE006AE, 0x610000E4, 4, pNopOpcodes },
  { 0x205, 0, 0xE0468C, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x205, 1, 0xE046E6, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x205, 2, 0xE04704, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x205, 4, 0xE04712, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x205, 5, 0xE046F4, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x205, 6, 0xE04704, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x205, 0, 0xE04CA0, 0x4EB900E0, 6, pNopOpcodes },
  { 0x205, 1, 0xE04CFA, 0x4EB900E0, 6, pNopOpcodes },
  { 0x205, 2, 0xE04D18, 0x4EB900E0, 6, pNopOpcodes },
  { 0x205, 4, 0xE04D26, 0x4EB900E0, 6, pNopOpcodes },
  { 0x205, 5, 0xE04D08, 0x4EB900E0, 6, pNopOpcodes },
  { 0x205, 6, 0xE04D18, 0x4EB900E0, 6, pNopOpcodes },
  /* An unpatched TOS 2.05 only works on STEs, so apply some anti-STE patches... */
  { 0x205, -1, 0xE00096, 0x42788900, 4, pNopOpcodes },
  { 0x205, -1, 0xE0009E, 0x31D88924, 4, pNopOpcodes },
  { 0x205, -1, 0xE000A6, 0x09D10AA9, 28, pNopOpcodes },
  { 0x205, -1, 0xE003A0, 0x30389200, 4, pNopOpcodes },
  { 0x205, -1, 0xE004EA, 0x61000CBC, 4, pNopOpcodes },
  { 0x205, -1, 0xE00508, 0x61000C9E, 4, pNopOpcodes },
  { 0x205, -1, 0xE007A0, 0x631E2F3C, 1, pBraOpcode },
  { 0x205, -1, 0xE00928, 0x10388901, 4, pNopOpcodes },
  { 0x205, -1, 0xE00944, 0xB0388901, 4, pNopOpcodes },
  { 0x205, -1, 0xE00950, 0x67024601, 1, pBraOpcode },
  { 0x205, -1, 0xE00968, 0x61000722, 4, pNopOpcodes },
  { 0x205, -1, 0xE00CF2, 0x1038820D, 4, pNopOpcodes },
  { 0x205, -1, 0xE00E00, 0x1038820D, 4, pNopOpcodes },
  { 0x205, 0, 0xE03038, 0x31C0860E, 4, pNopOpcodes },
  { 0x205, 0, 0xE034A8, 0x31C0860E, 4, pNopOpcodes },
  { 0x205, 0, 0xE034F6, 0x31E90002, 6, pNopOpcodes },

  /* E007FA  MOVE.L  #$1FFFE,D7  Run checksums on 2xROMs (skip) */
  /* Checksum is total of TOS ROM image, but get incorrect results */
  /* as we've changed bytes in the ROM! So, just skip anyway! */
  { 0x206, -1, 0xE007FA, 0x2E3C0001, 4, pRomCheckOpcode },
//{ 0x206, -1, 0xE00362, 0x42B804C2, 4, pNopOpcodes },
//{ 0x206, -1, 0xE00898, 0x610000E0, 4, pNopOpcodes },
  { 0x206, 0, 0xE0518E, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x206, 1, 0xE051E8, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x206, 2, 0xE05206, 0x4E56FFF0, 2, pRtsOpcode },
//{ 0x206, 3, 0xE0518E, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x206, 6, 0xE05206, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x206, 8, 0xE05214, 0x4E56FFF0, 2, pRtsOpcode },
  { 0x206, 0, 0xE05944, 0x4EB900E0, 6, pNopOpcodes },
  { 0x206, 1, 0xE0599E, 0x4EB900E0, 6, pNopOpcodes },
  { 0x206, 2, 0xE059BC, 0x4EB900E0, 6, pNopOpcodes },
//{ 0x206, 3, 0xE05944, 0x4EB900E0, 6, pNopOpcodes },
  { 0x206, 6, 0xE059BC, 0x4EB900E0, 6, pNopOpcodes },
  { 0x206, 8, 0xE059CA, 0x4EB900E0, 6, pNopOpcodes },

  { 0, 0, 0, 0, 0, NULL }
};



/*-----------------------------------------------------------------------*/
/*
  Patch TOS to skip some TOS setup code which we don't support/need.

  So, how do we find these addresses when we have no commented source code?
  - Hdv_init: Scan start of TOS for table of move.l <addr>,$46A(a5), around 0x224 bytes in
    and look at the first entry - that's the hdv_init address.
  - Hdv_boot: Scan start of TOS for table of move.l <addr>,$47A(a5), and look for 5th entry,
    that's the hdv_boot address. The function starts with link,movem,jsr.
  - Boot from DMA bus: again scan at start of rom for tst.w $482, boot call will be just above it.
  - Clear connected drives: search for 'clr.w' and '$4c2' to find, may use (a5) in which case op-code
    is only 4 bytes and also note this is only do on TOS > 1.00

  If we use hard disk emulation, we also need to force set condrv ($4c2),
  because the ACSI driver (if any) will reset it. This is done after the DMA
  bus boot (when the driver loads), replacing the RTS with our own routine which
  sets condrv and then RTSes.
*/
void TOS_FixRom(uint8 *TosAddress)
{
  TOS_PATCH *pPatch;

  TosCountry = (*((unsigned short *)(TosAddress+28)))>>1;   /* TOS country code */
  TosVersion = (*(Uint16 *)((Uint32)TosAddress+2));

  /* Check for EmuTOS first since we can not patch it */
  if (((*((unsigned short *)(TosAddress+0x2c)))==0x4554) &&
	((*((unsigned short *)(TosAddress+0x2d)))==0x5345))
		return;

  pPatch = TosPatches;

  /* Apply TOS patches: */
  while(pPatch->Version)
  {
    /* Only apply patches that suit to the actual TOS  version: */
    if(pPatch->Version == TosVersion
       && (pPatch->Country == TosCountry || pPatch->Country == -1))
    {
      /* Make sure that we really patch the right place by comparing data: */
      if(ReadSL(rombase+pPatch->Address) == pPatch->OldData)
      { 
	  register uint8 *dst=(uint8*)rombase+pPatch->Address;
	  register uint8 *src=(uint8*)pPatch->pNewData;
	  register unsigned i;
	  for(i=0;i<pPatch->Size;i+=2)
	  {
#ifndef USE_BIG_ENDIAN
		  dst[i]=src[i+1];
		  dst[i+1]=src[i];
#else
		  dst[i]=src[i];
		  dst[i+1]=src[i+1];
#endif
	  }
      }
    }
    pPatch += 1;
  }
}

