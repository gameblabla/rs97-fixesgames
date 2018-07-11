//---------------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version. See also the license.txt file for
//  additional informations.
//---------------------------------------------------------------------------

// state.cpp: state saving
//
//  01/20/2009 Cleaned up interface, added loading from memory
//             Moved signature-related stuff out of race_state (A.K.)
//  09/11/2008 Initial version (Akop Karapetyan)
//
//////////////////////////////////////////////////////////////////////

#include "cz80.h"
#include "neopopsound.h"

#include <string.h>
#include <stdio.h>
#include "state.h"
#include "tlcs900h.h"
#include "memory.h"

#ifdef PC
#undef PC
#endif

#define CURRENT_SAVE_STATE_VERSION 0x11

struct race_state_header
{
  u8 state_version;       /* State version */
  u8 rom_signature[0x40]; /* Rom signature, for verification */
};

struct race_state_0x11
{
	/* Memory */
	u8 ram[0xc000];
  u8 cpuram[0x08a0];

	/* TLCS-900h Registers */
	u32 pc, sr;
	u8 f_dash;
	u32 gpr[23];

  /* Z80 Registers */
  cz80_struc RACE_cz80_struc;
  u32 PC_offset;
  s32 Z80_ICount;

  /* Sound */
  int sndCycles;
  SoundChip toneChip;
  SoundChip noiseChip;

	/* Timers */
  int timer0, timer1, timer2, timer3;

	/* DMA */
  u8 ldcRegs[64];
};

struct race_state_0x10 /* Older state format */
{
  //Save state version
  u8 state_version; // = 0x10
 
  //Rom signature
  u8 rom_signature[0x40];
 
	//Memory
	u8 ram[0xc000];
  u8 cpuram[0x08a0];// 0xC000]; 0x38000 
 
	//TLCS-900h Registers
	u32 pc, sr;
	u8 f_dash;
	u32 gpr[23];
 
  //Z80 Registers
  cz80_struc RACE_cz80_struc;
  u32 PC_offset;
  s32 Z80_ICount;
 
  //Sound Chips
  int sndCycles;
  SoundChip toneChip;
  SoundChip noiseChip;
 
	//Timers
  int timer0, timer1, timer2, timer3;
 
	//DMA
  u8 ldcRegs[64];
};

typedef struct race_state_0x11 race_state_t;

static int state_store(race_state_t *rs);
static int state_restore(race_state_t *rs);
static int state_restore_0x10(FILE *stream);

int state_store_mem(void *state)
{
  return state_store((race_state_t*)state);
}

int state_restore_mem(void *state)
{
  return state_restore((race_state_t*)state);
}

int state_get_size()
{
  return sizeof(race_state_t);
}

static int state_store(race_state_t *rs)
{
  int i = 0;

  /* TLCS-900h Registers */
  rs->pc = gen_regsPC;
  rs->sr = gen_regsSR;
  rs->f_dash = F2;

  rs->gpr[i++] = gen_regsXWA0;
  rs->gpr[i++] = gen_regsXBC0;
  rs->gpr[i++] = gen_regsXDE0;
  rs->gpr[i++] = gen_regsXHL0;

  rs->gpr[i++] = gen_regsXWA1;
  rs->gpr[i++] = gen_regsXBC1;
  rs->gpr[i++] = gen_regsXDE1;
  rs->gpr[i++] = gen_regsXHL1;

  rs->gpr[i++] = gen_regsXWA2;
  rs->gpr[i++] = gen_regsXBC2;
  rs->gpr[i++] = gen_regsXDE2;
  rs->gpr[i++] = gen_regsXHL2;

  rs->gpr[i++] = gen_regsXWA3;
  rs->gpr[i++] = gen_regsXBC3;
  rs->gpr[i++] = gen_regsXDE3;
  rs->gpr[i++] = gen_regsXHL3;

  rs->gpr[i++] = gen_regsXIX;
  rs->gpr[i++] = gen_regsXIY;
  rs->gpr[i++] = gen_regsXIZ;
  rs->gpr[i++] = gen_regsXSP;

  rs->gpr[i++] = gen_regsSP;
  rs->gpr[i++] = gen_regsXSSP;
  rs->gpr[i++] = gen_regsXNSP;

  /* Z80 Registers */
  extern cz80_struc *RACE_cz80_struc;
  extern s32 Z80_ICount;
  int size_of_z80 = 
    (u32)(&(RACE_cz80_struc->CycleSup)) - (u32)(&(RACE_cz80_struc->BC));
  memcpy(&rs->RACE_cz80_struc, RACE_cz80_struc, size_of_z80);
  rs->Z80_ICount = Z80_ICount;
  rs->PC_offset = Cz80_Get_PC(RACE_cz80_struc);

  /* Sound */
  extern int sndCycles;
  rs->sndCycles = sndCycles;
  memcpy(&rs->toneChip, &toneChip, sizeof(SoundChip));
  memcpy(&rs->noiseChip, &noiseChip, sizeof(SoundChip));

  /* Timers */
  rs->timer0 = timer0;
  rs->timer1 = timer1;
  rs->timer2 = timer2;
  rs->timer3 = timer3;

  /* DMA */
  memcpy(&rs->ldcRegs, &ldcRegs, sizeof(ldcRegs));

  /* Memory */
  memcpy(rs->ram, mainram, sizeof(rs->ram));
  memcpy(rs->cpuram, &mainram[0x20000], sizeof(rs->cpuram));

  return 1;
}

static int state_restore(race_state_t *rs)
{
  int i = 0;

  /* TLCS-900h Registers */
  gen_regsPC = rs->pc;
  gen_regsSR = rs->sr;
  F2 = rs->f_dash;

  gen_regsXWA0 = rs->gpr[i++];
  gen_regsXBC0 = rs->gpr[i++];
  gen_regsXDE0 = rs->gpr[i++];
  gen_regsXHL0 = rs->gpr[i++];

  gen_regsXWA1 = rs->gpr[i++];
  gen_regsXBC1 = rs->gpr[i++];
  gen_regsXDE1 = rs->gpr[i++];
  gen_regsXHL1 = rs->gpr[i++];

  gen_regsXWA2 = rs->gpr[i++];
  gen_regsXBC2 = rs->gpr[i++];
  gen_regsXDE2 = rs->gpr[i++];
  gen_regsXHL2 = rs->gpr[i++];

  gen_regsXWA3 = rs->gpr[i++];
  gen_regsXBC3 = rs->gpr[i++];
  gen_regsXDE3 = rs->gpr[i++];
  gen_regsXHL3 = rs->gpr[i++];

  gen_regsXIX = rs->gpr[i++];
  gen_regsXIY = rs->gpr[i++];
  gen_regsXIZ = rs->gpr[i++];
  gen_regsXSP = rs->gpr[i++];

  gen_regsSP = rs->gpr[i++];
  gen_regsXSSP = rs->gpr[i++];
  gen_regsXNSP = rs->gpr[i++];

  /* Z80 Registers */
  extern cz80_struc *RACE_cz80_struc;
  extern s32 Z80_ICount;
  int size_of_z80 = 
    (u32)(&(RACE_cz80_struc->CycleSup)) - (u32)(&(RACE_cz80_struc->BC));

  memcpy(RACE_cz80_struc, &rs->RACE_cz80_struc, size_of_z80);
  Z80_ICount = rs->Z80_ICount;
  Cz80_Set_PC(RACE_cz80_struc, rs->PC_offset);

  /* Sound */
  extern int sndCycles;
  sndCycles = rs->sndCycles;
  memcpy(&toneChip, &rs->toneChip, sizeof(SoundChip));
  memcpy(&noiseChip, &rs->noiseChip, sizeof(SoundChip));

  /* Timers */
  timer0 = rs->timer0;
  timer1 = rs->timer1;
  timer2 = rs->timer2;
  timer3 = rs->timer3;

  /* DMA */
  memcpy(&ldcRegs, &rs->ldcRegs, sizeof(ldcRegs));

  /* Memory */
  memcpy(mainram, rs->ram, sizeof(rs->ram));
  memcpy(&mainram[0x20000], rs->cpuram, sizeof(rs->cpuram));

  /* Reinitialize TLCS (mainly redirect pointers) */
  tlcs_reinit();

  return 1;
}

int state_restore(FILE *stream)
{
  /* Note current position (in case of compatibility rewinds */
  long read_pos = ftell(stream);

  /* Read header */
  struct race_state_header rsh;
  if (fread(&rsh, sizeof(rsh), 1, stream) < 1)
    return 0;

  /* Verify state version */
  if (rsh.state_version == 0x10)
  {
    fseek(stream, read_pos, SEEK_SET); /* Rewind and load old format */
    return state_restore_0x10(stream);
  }
  else if (rsh.state_version != CURRENT_SAVE_STATE_VERSION)
    return 0; /* Unsupported version */

  /* Verify ROM signature */
  if (memcmp(mainrom, rsh.rom_signature, sizeof(rsh.rom_signature)) != 0)
    return 0;

  /* Read state data */
  race_state_t rs;
  if (fread(&rs, sizeof(rs), 1, stream) < 1)
    return 0;

  /* Restore state */
  return state_restore(&rs);
}

int state_store(FILE *stream)
{
  /* Set version & ROM signature information */
  struct race_state_header rsh;
  rsh.state_version = CURRENT_SAVE_STATE_VERSION;
  memcpy(rsh.rom_signature, mainrom, sizeof(rsh.rom_signature));

  /* Initialize state data */
  race_state_t rs;
  if (!state_store(&rs))
    return 0;

  /* Write to file */
  if (fwrite(&rsh, sizeof(rsh), 1, stream) < 1)
    return 0;
  if (fwrite(&rs, sizeof(rs), 1, stream) < 1)
    return 0;

  return 1;
}

int state_store(char* filename)
{
  FILE *stream;
  if (!(stream = fopen(filename, "w")))
    return 0;

  int status = state_store(stream);
  fclose(stream);

  return status;
}

int state_restore(char* filename)
{
  FILE *stream;
  if (!(stream = fopen(filename, "r")))
    return 0;

  int status = state_restore(stream);
  fclose(stream);

  return status;
}

static int state_restore_0x10(FILE *stream)
{
  struct race_state_0x10 rs;
  if (fread(&rs, sizeof(rs), 1, stream) < 1)
    return 0;
 
  // Verify state version
  if (rs.state_version != 0x10)
    return 0;
 
  // Verify ROM signature
  if (memcmp(mainrom, rs.rom_signature, sizeof(rs.rom_signature)) != 0)
    return 0;
 
  //TLCS-900h Registers
  gen_regsPC = rs.pc;
  gen_regsSR = rs.sr;
  F2 = rs.f_dash;
 
  int i = 0;
  gen_regsXWA0 = rs.gpr[i++];
  gen_regsXBC0 = rs.gpr[i++];
  gen_regsXDE0 = rs.gpr[i++];
  gen_regsXHL0 = rs.gpr[i++];
 
  gen_regsXWA1 = rs.gpr[i++];
  gen_regsXBC1 = rs.gpr[i++];
  gen_regsXDE1 = rs.gpr[i++];
  gen_regsXHL1 = rs.gpr[i++];
 
  gen_regsXWA2 = rs.gpr[i++];
  gen_regsXBC2 = rs.gpr[i++];
  gen_regsXDE2 = rs.gpr[i++];
  gen_regsXHL2 = rs.gpr[i++];
 
  gen_regsXWA3 = rs.gpr[i++];
  gen_regsXBC3 = rs.gpr[i++];
  gen_regsXDE3 = rs.gpr[i++];
  gen_regsXHL3 = rs.gpr[i++];
 
  gen_regsXIX = rs.gpr[i++];
  gen_regsXIY = rs.gpr[i++];
  gen_regsXIZ = rs.gpr[i++];
  gen_regsXSP = rs.gpr[i++];
 
  gen_regsSP = rs.gpr[i++];
  gen_regsXSSP = rs.gpr[i++];
  gen_regsXNSP = rs.gpr[i++];
 
  //Z80 Registers
  extern cz80_struc *RACE_cz80_struc;
  extern s32 Z80_ICount;
  int size_of_z80 = 
    (u32)(&(RACE_cz80_struc->CycleSup)) - (u32)(&(RACE_cz80_struc->BC));
 
  memcpy(RACE_cz80_struc, &rs.RACE_cz80_struc, size_of_z80);
  Z80_ICount = rs.Z80_ICount;
  Cz80_Set_PC(RACE_cz80_struc, rs.PC_offset);
 
  //Sound Chips
  extern int sndCycles;
  sndCycles = rs.sndCycles;
  memcpy(&toneChip, &rs.toneChip, sizeof(SoundChip));
  memcpy(&noiseChip, &rs.noiseChip, sizeof(SoundChip));
 
  //Timers
  timer0 = rs.timer0;
  timer1 = rs.timer1;
  timer2 = rs.timer2;
  timer3 = rs.timer3;
 
  //DMA
  memcpy(&ldcRegs, &rs.ldcRegs, sizeof(ldcRegs));
 
  //Memory
  memcpy(mainram, rs.ram, sizeof(rs.ram));
  memcpy(&mainram[0x20000], rs.cpuram, sizeof(rs.cpuram));
 
  tlcs_reinit();
 
  return 1;
}
