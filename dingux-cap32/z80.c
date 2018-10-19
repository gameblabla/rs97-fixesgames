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

/* Zilog Z80A Microprocessor emulation
   (c) Copyright 1997-2003 Ulrich Doewich

   code portions and ideas taken from the Z80 emulations by:
    Juergen Buchmueller (MAME Z80 core v3.3)
    Marat Fayzullin
    and my own Z80 x86 assembly code (Caprice32 up to version 2.00b2)

   Oct 03, 2000 - 18:56    all un-prefixed opcodes done
   Oct 07, 2000 - 11:04    all CB opcodes done
   Oct 07, 2000 - 15:06    all DD opcodes done
   Oct 07, 2000 - 15:23    all DD CB opcodes done
   Oct 09, 2000 - 12:41    all ED, FD, and FD CB opcodes done
   Oct 14, 2000 - 17:48    added interrupt processing to z80_getopcode
   Oct 22, 2000 - 19:18    removed R register update from DDCB and FDCB opcode handlers
   Oct 22, 2000 - 19:43    added break-point and trace capabilities
   Oct 24, 2000 - 17:57    changed math based opcodes to always work with unsigned parameters
   Oct 29, 2000 - 20:46    fixed 16 bit memory read/write opcodes (read both halves from the low byte!)
   Oct 29, 2000 - 20:51    fixed LD L,byte; RRC r (used wrong registers - forgot to change them after copy!)
   Nov 06, 2000 - 21:08    fixed a couple of IX/IY instructions (forgot to change a few I?h/I?l related opcodes!)
   Nov 06, 2000 - 21:20    fixed some DDCB/FDCB instructions (one too many M cycles for BIT (I?+o) & co.)
   Nov 07, 2000 - 18:58    complete overhaul of DDCB/FDCB instructions (offset byte handling was wrong!)
   Jan 24, 2001 - 18:26    fixed LD (IX/IY + o), L and LD (IX/IY + o), H (uses L and H, not I?l and I?h!)
   Feb 19, 2001 - 18:37    removed machine cycle specific code; added cycle count tables and 'wait state' routine
   Mar 05, 2001 - 22:58    reworked all cycle count tables - verfied with the real CPC & an oscilloscope
   Mar 29, 2001 - 19:10    fixed the timing problem (z80_wait_states was called after interrupts even if they were disabled!)
   Apr 03, 2001 - 18:25    incorporated the changes from the MAME Z80 core v3.1 to v3.2 update
   Apr 09, 2001 - 19:30    fixed the problem with some CPC programs crashing (offset for IX/IY instructions was unsigned!)
   Jul 31, 2001 - 23:34    put the 'old' NOP cycle timing table back in
   Nov 12, 2001 - 18:15    incorporated the changes from the MAME Z80 core v3.2 to v3.3 update
   Nov 14, 2002 - 21:39    changed the length of processing an interrupt in IM2 from 28T to 76T
   Feb 10, 2003 - 18:24    corrected the cycle count of CPI/CPIR & CPD/CPDR with the help of Richard's PlusTest
   Feb 12, 2003 - 17:29    added the wait state adjustment on interrupts for a specific number of instructions
                           (see Richard's document on andercheran for the complete list)
   Apr 07, 2003 - 19:10    added z80_mf2stop to emulate the NMI caused by the stop button of the MF2
   Apr 07, 2003 - 22:48    added code to z80_execute to monitor when the MF2 finishes and has to be made 'invisible'
   May 10, 2003 - 19:12    fixed the unofficial DDCB/FDCB RES/SET instructions: the unaltered value was
                           stored in the associated register; some minor code clean up
   May 15, 2003 - 23:19    "Thomas the Tank Engine", "N.E.X.O.R." and "Jocky Wilson's Darts Compendium" work now:
                           DI did not clear the z80.EI_issued counter
*/

#include "cap32.h"
#include "z80.h"

extern t_CPC CPC;
extern t_FDC FDC;
extern t_GateArray GateArray;
extern t_PSG PSG;
extern t_VDU VDU;

enum opcodes {
   nop, ld_bc_word, ld_mbc_a, inc_bc, inc_b, dec_b, ld_b_byte, rlca,
   ex_af_af, add_hl_bc, ld_a_mbc, dec_bc, inc_c, dec_c, ld_c_byte, rrca,
   djnz, ld_de_word, ld_mde_a, inc_de, inc_d, dec_d, ld_d_byte, rla,
   jr, add_hl_de, ld_a_mde, dec_de, inc_e, dec_e, ld_e_byte, rra,
   jr_nz, ld_hl_word, ld_mword_hl, inc_hl, inc_h, dec_h, ld_h_byte, daa,
   jr_z, add_hl_hl, ld_hl_mword, dec_hl, inc_l, dec_l, ld_l_byte, cpl,
   jr_nc, ld_sp_word, ld_mword_a, inc_sp, inc_mhl, dec_mhl, ld_mhl_byte, scf,
   jr_c, add_hl_sp, ld_a_mword, dec_sp, inc_a, dec_a, ld_a_byte, ccf,
   ld_b_b, ld_b_c, ld_b_d, ld_b_e, ld_b_h, ld_b_l, ld_b_mhl, ld_b_a,
   ld_c_b, ld_c_c, ld_c_d, ld_c_e, ld_c_h, ld_c_l, ld_c_mhl, ld_c_a,
   ld_d_b, ld_d_c, ld_d_d, ld_d_e, ld_d_h, ld_d_l, ld_d_mhl, ld_d_a,
   ld_e_b, ld_e_c, ld_e_d, ld_e_e, ld_e_h, ld_e_l, ld_e_mhl, ld_e_a,
   ld_h_b, ld_h_c, ld_h_d, ld_h_e, ld_h_h, ld_h_l, ld_h_mhl, ld_h_a,
   ld_l_b, ld_l_c, ld_l_d, ld_l_e, ld_l_h, ld_l_l, ld_l_mhl, ld_l_a,
   ld_mhl_b, ld_mhl_c, ld_mhl_d, ld_mhl_e, ld_mhl_h, ld_mhl_l, halt, ld_mhl_a,
   ld_a_b, ld_a_c, ld_a_d, ld_a_e, ld_a_h, ld_a_l, ld_a_mhl, ld_a_a,
   add_b, add_c, add_d, add_e, add_h, add_l, add_mhl, add_a,
   adc_b, adc_c, adc_d, adc_e, adc_h, adc_l, adc_mhl, adc_a,
   sub_b, sub_c, sub_d, sub_e, sub_h, sub_l, sub_mhl, sub_a,
   sbc_b, sbc_c, sbc_d, sbc_e, sbc_h, sbc_l, sbc_mhl, sbc_a,
   and_b, and_c, and_d, and_e, and_h, and_l, and_mhl, and_a,
   xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, xor_mhl, xor_a,
   or_b, or_c, or_d, or_e, or_h, or_l, or_mhl, or_a,
   cp_b, cp_c, cp_d, cp_e, cp_h, cp_l, cp_mhl, cp_a,
   ret_nz, pop_bc, jp_nz, jp, call_nz, push_bc, add_byte, rst00,
   ret_z, ret, jp_z, pfx_cb, call_z, call, adc_byte, rst08,
   ret_nc, pop_de, jp_nc, outa, call_nc, push_de, sub_byte, rst10,
   ret_c, exx, jp_c, ina, call_c, pfx_dd, sbc_byte, rst18,
   ret_po, pop_hl, jp_po, ex_msp_hl, call_po, push_hl, and_byte, rst20,
   ret_pe, ld_pc_hl, jp_pe, ex_de_hl, call_pe, pfx_ed, xor_byte, rst28,
   ret_p, pop_af, jp_p, di, call_p, push_af, or_byte, rst30,
   ret_m, ld_sp_hl, jp_m, ei, call_m, pfx_fd, cp_byte, rst38
};

enum CBcodes {
   rlc_b, rlc_c, rlc_d, rlc_e, rlc_h, rlc_l, rlc_mhl, rlc_a,
   rrc_b, rrc_c, rrc_d, rrc_e, rrc_h, rrc_l, rrc_mhl, rrc_a,
   rl_b, rl_c, rl_d, rl_e, rl_h, rl_l, rl_mhl, rl_a,
   rr_b, rr_c, rr_d, rr_e, rr_h, rr_l, rr_mhl, rr_a,
   sla_b, sla_c, sla_d, sla_e, sla_h, sla_l, sla_mhl, sla_a,
   sra_b, sra_c, sra_d, sra_e, sra_h, sra_l, sra_mhl, sra_a,
   sll_b, sll_c, sll_d, sll_e, sll_h, sll_l, sll_mhl, sll_a,
   srl_b, srl_c, srl_d, srl_e, srl_h, srl_l, srl_mhl, srl_a,
   bit0_b, bit0_c, bit0_d, bit0_e, bit0_h, bit0_l, bit0_mhl, bit0_a,
   bit1_b, bit1_c, bit1_d, bit1_e, bit1_h, bit1_l, bit1_mhl, bit1_a,
   bit2_b, bit2_c, bit2_d, bit2_e, bit2_h, bit2_l, bit2_mhl, bit2_a,
   bit3_b, bit3_c, bit3_d, bit3_e, bit3_h, bit3_l, bit3_mhl, bit3_a,
   bit4_b, bit4_c, bit4_d, bit4_e, bit4_h, bit4_l, bit4_mhl, bit4_a,
   bit5_b, bit5_c, bit5_d, bit5_e, bit5_h, bit5_l, bit5_mhl, bit5_a,
   bit6_b, bit6_c, bit6_d, bit6_e, bit6_h, bit6_l, bit6_mhl, bit6_a,
   bit7_b, bit7_c, bit7_d, bit7_e, bit7_h, bit7_l, bit7_mhl, bit7_a,
   res0_b, res0_c, res0_d, res0_e, res0_h, res0_l, res0_mhl, res0_a,
   res1_b, res1_c, res1_d, res1_e, res1_h, res1_l, res1_mhl, res1_a,
   res2_b, res2_c, res2_d, res2_e, res2_h, res2_l, res2_mhl, res2_a,
   res3_b, res3_c, res3_d, res3_e, res3_h, res3_l, res3_mhl, res3_a,
   res4_b, res4_c, res4_d, res4_e, res4_h, res4_l, res4_mhl, res4_a,
   res5_b, res5_c, res5_d, res5_e, res5_h, res5_l, res5_mhl, res5_a,
   res6_b, res6_c, res6_d, res6_e, res6_h, res6_l, res6_mhl, res6_a,
   res7_b, res7_c, res7_d, res7_e, res7_h, res7_l, res7_mhl, res7_a,
   set0_b, set0_c, set0_d, set0_e, set0_h, set0_l, set0_mhl, set0_a,
   set1_b, set1_c, set1_d, set1_e, set1_h, set1_l, set1_mhl, set1_a,
   set2_b, set2_c, set2_d, set2_e, set2_h, set2_l, set2_mhl, set2_a,
   set3_b, set3_c, set3_d, set3_e, set3_h, set3_l, set3_mhl, set3_a,
   set4_b, set4_c, set4_d, set4_e, set4_h, set4_l, set4_mhl, set4_a,
   set5_b, set5_c, set5_d, set5_e, set5_h, set5_l, set5_mhl, set5_a,
   set6_b, set6_c, set6_d, set6_e, set6_h, set6_l, set6_mhl, set6_a,
   set7_b, set7_c, set7_d, set7_e, set7_h, set7_l, set7_mhl, set7_a
};

enum EDcodes {
   ed_00, ed_01, ed_02, ed_03, ed_04, ed_05, ed_06, ed_07,
   ed_08, ed_09, ed_0a, ed_0b, ed_0c, ed_0d, ed_0e, ed_0f,
   ed_10, ed_11, ed_12, ed_13, ed_14, ed_15, ed_16, ed_17,
   ed_18, ed_19, ed_1a, ed_1b, ed_1c, ed_1d, ed_1e, ed_1f,
   ed_20, ed_21, ed_22, ed_23, ed_24, ed_25, ed_26, ed_27,
   ed_28, ed_29, ed_2a, ed_2b, ed_2c, ed_2d, ed_2e, ed_2f,
   ed_30, ed_31, ed_32, ed_33, ed_34, ed_35, ed_36, ed_37,
   ed_38, ed_39, ed_3a, ed_3b, ed_3c, ed_3d, ed_3e, ed_3f,
   in_b_c, out_c_b, sbc_hl_bc, ld_EDmword_bc, neg, retn, im_0, ld_i_a,
   in_c_c, out_c_c, adc_hl_bc, ld_EDbc_mword, neg_1, reti, im_0_1, ld_r_a,
   in_d_c, out_c_d, sbc_hl_de, ld_EDmword_de, neg_2, retn_1, im_1, ld_a_i,
   in_e_c, out_c_e, adc_hl_de, ld_EDde_mword, neg_3, reti_1, im_2, ld_a_r,
   in_h_c, out_c_h, sbc_hl_hl, ld_EDmword_hl, neg_4, retn_2, im_0_2, rrd,
   in_l_c, out_c_l, adc_hl_hl, ld_EDhl_mword, neg_5, reti_2, im_0_3, rld,
   in_0_c, out_c_0, sbc_hl_sp, ld_EDmword_sp, neg_6, retn_3, im_1_1, ed_77,
   in_a_c, out_c_a, adc_hl_sp, ld_EDsp_mword, neg_7, reti_3, im_2_1, ed_7f,
   ed_80, ed_81, ed_82, ed_83, ed_84, ed_85, ed_86, ed_87,
   ed_88, ed_89, ed_8a, ed_8b, ed_8c, ed_8d, ed_8e, ed_8f,
   ed_90, ed_91, ed_92, ed_93, ed_94, ed_95, ed_96, ed_97,
   ed_98, ed_99, ed_9a, ed_9b, ed_9c, ed_9d, ed_9e, ed_9f,
   ldi, cpi, ini, outi, ed_a4, ed_a5, ed_a6, ed_a7,
   ldd, cpd, ind, outd, ed_ac, ed_ad, ed_ae, ed_af,
   ldir, cpir, inir, otir, ed_b4, ed_b5, ed_b6, ed_b7,
   lddr, cpdr, indr, otdr, ed_bc, ed_bd, ed_be, ed_bf,
   ed_c0, ed_c1, ed_c2, ed_c3, ed_c4, ed_c5, ed_c6, ed_c7,
   ed_c8, ed_c9, ed_ca, ed_cb, ed_cc, ed_cd, ed_ce, ed_cf,
   ed_d0, ed_d1, ed_d2, ed_d3, ed_d4, ed_d5, ed_d6, ed_d7,
   ed_d8, ed_d9, ed_da, ed_db, ed_dc, ed_dd, ed_de, ed_df,
   ed_e0, ed_e1, ed_e2, ed_e3, ed_e4, ed_e5, ed_e6, ed_e7,
   ed_e8, ed_e9, ed_ea, ed_eb, ed_ec, ed_ed, ed_ee, ed_ef,
   ed_f0, ed_f1, ed_f2, ed_f3, ed_f4, ed_f5, ed_f6, ed_f7,
   ed_f8, ed_f9, ed_fa, ed_fb, ed_fc, ed_fd, ed_fe, ed_ff
};



t_z80regs z80;
static byte iCycleCount, iWSAdjust;
static byte SZ[256]; // zero and sign flags
static byte SZ_BIT[256]; // zero, sign and parity/overflow (=zero) flags for BIT opcode
static byte SZP[256]; // zero, sign and parity flags
static byte SZHV_inc[256]; // zero, sign, half carry and overflow flags INC r8
static byte SZHV_dec[256]; // zero, sign, half carry and overflow flags DEC r8

#include "z80daa.h"

static byte irep_tmp1[4][4] = {
   {0, 0, 1, 0}, {0, 1, 0, 1}, {1, 0, 1, 1}, {0, 1, 1, 0}
};

/* tmp1 value for ind/indr/outd/otdr for [C.1-0][io.1-0] */
static byte drep_tmp1[4][4] = {
   {0, 1, 0, 0}, {1, 0, 0, 1}, {0, 0, 1, 0}, {0, 1, 0, 1}
};

/* tmp2 value for all in/out repeated opcodes for B.7-0 */
static byte breg_tmp2[256] = {
   0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
   0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
   1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1
};

#define Oa 8
#define Oa_ 4
#define Ia 12
#define Ia_ 0

static byte cc_op[256] = {
    4, 12,  8,  8,  4,  4,  8,  4,  4, 12,  8,  8,  4,  4,  8,  4,
   12, 12,  8,  8,  4,  4,  8,  4, 12, 12,  8,  8,  4,  4,  8,  4,
    8, 12, 20,  8,  4,  4,  8,  4,  8, 12, 20,  8,  4,  4,  8,  4,
    8, 12, 16,  8, 12, 12, 12,  4,  8, 12, 16,  8,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    8, 12, 12, 12, 12, 16,  8, 16,  8, 12, 12,  4, 12, 20,  8, 16,
    8, 12, 12, Oa, 12, 16,  8, 16,  8,  4, 12, Ia, 12,  4,  8, 16,
    8, 12, 12, 24, 12, 16,  8, 16,  8,  4, 12,  4, 12,  4,  8, 16,
    8, 12, 12,  4, 12, 16,  8, 16,  8,  8, 12,  4, 12,  4,  8, 16
};

static byte cc_cb[256] = {
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4,
    4,  4,  4,  4,  4,  4, 12,  4,  4,  4,  4,  4,  4,  4, 12,  4
};

#define Ox 8
#define Ox_ 4
#define Oy 12
#define Oy_ 4
#define Ix 12
#define Ix_ 0
#define Iy 16
#define Iy_ 0

static byte cc_ed[256] = {
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
   Ix, Ox, 12, 20,  4, 12,  4,  8, Ix, Ox, 12, 20,  4, 12,  4,  8,
   Ix, Ox, 12, 20,  4, 12,  4,  8, Ix, Ox, 12, 20,  4, 12,  4,  8,
   Ix, Ox, 12, 20,  4, 12,  4, 16, Ix, Ox, 12, 20,  4, 12,  4, 16,
   Ix, Ox, 12, 20,  4, 12,  4,  4, Ix, Ox, 12, 20,  4, 12,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
   16, 12, Iy, Oy,  4,  4,  4,  4, 16, 12, Iy, Oy,  4,  4,  4,  4,
   16, 12, Iy, Oy,  4,  4,  4,  4, 16, 12, Iy, Oy,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4
};

static byte cc_xy[256] = {
    4, 12,  8,  8,  4,  4,  8,  4,  4, 12,  8,  8,  4,  4,  8,  4,
   12, 12,  8,  8,  4,  4,  8,  4, 12, 12,  8,  8,  4,  4,  8,  4,
    8, 12, 20,  8,  4,  4,  8,  4,  8, 12, 20,  8,  4,  4,  8,  4,
    8, 12, 16,  8, 20, 20, 20,  4,  8, 12, 16,  8,  4,  4,  8,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
   16, 16, 16, 16, 16, 16,  4, 16,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    4,  4,  4,  4,  4,  4, 16,  4,  4,  4,  4,  4,  4,  4, 16,  4,
    8, 12, 12, 12, 12, 16,  8, 16,  8, 12, 12,  4, 12, 20,  8, 16,
    8, 12, 12, Oa, 12, 16,  8, 16,  8,  4, 12, Ia, 12,  4,  8, 16,
    8, 12, 12, 24, 12, 16,  8, 16,  8,  4, 12,  4, 12,  4,  8, 16,
    8, 12, 12,  4, 12, 16,  8, 16,  8,  8, 12,  4, 12,  4,  8, 16
};

static byte cc_xycb[256] = {
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

static byte cc_ex[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    4,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,
    4,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    4,  8,  4,  4,  0,  0,  0,  0,  4,  8,  4,  4,  0,  0,  0,  0,
    8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,
    8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,
    8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,
    8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0,  8,  0,  0,  0
};



extern byte *membank_read[4], *membank_write[4];

inline byte read_mem(word addr) {
   return (*(membank_read[addr >> 14] + (addr & 0x3fff))); // returns a byte from a 16KB memory bank
}

inline void write_mem(word addr, byte val) {
   *(membank_write[addr >> 14] + (addr & 0x3fff)) = val; // writes a byte to a 16KB memory bank
}


#define Z80_JMP_TBL
#define Z80_MACRO_WAIT_STATES

# ifdef Z80_MACRO_WAIT_STATES
#define z80_wait_states \
{ \
   if (iCycleCount) { \
      access_video_memory(iCycleCount >> 2); \
      if (CPC.snd_enabled) { \
         PSG.cycle_count.high += iCycleCount; \
         if (PSG.cycle_count.high >= CPC.snd_cycle_count_init.high) { \
            PSG.cycle_count.both -= CPC.snd_cycle_count_init.both; \
            PSG.Synthesizer(); \
         } \
      } \
      if (FDC.phase == EXEC_PHASE) { \
         FDC.timeout -= iCycleCount; \
         if (FDC.timeout <= 0) { \
            FDC.flags |= OVERRUN_flag; \
            if (FDC.cmd_direction == FDC_TO_CPU) { \
               fdc_read_data(); \
            } \
            else { \
               fdc_write_data(0xff); \
            } \
         } \
      } \
   } \
}
# else
void
loc_z80_wait_states(int iCycleCount)
{ 
   if (iCycleCount) { 
      access_video_memory(iCycleCount >> 2); 
      if (CPC.snd_enabled) { 
         PSG.cycle_count.high += iCycleCount; 
         if (PSG.cycle_count.high >= CPC.snd_cycle_count_init.high) { 
            PSG.cycle_count.both -= CPC.snd_cycle_count_init.both; 
            PSG.Synthesizer(); 
         } 
      } 
      if (FDC.phase == EXEC_PHASE) { 
         FDC.timeout -= iCycleCount; 
         if (FDC.timeout <= 0) { 
            FDC.flags |= OVERRUN_flag; 
            if (FDC.cmd_direction == FDC_TO_CPU) { 
               fdc_read_data(); 
            } 
            else { 
               fdc_write_data(0xff); 
            } 
         } 
      } 
   } 
}
#define z80_wait_states \
        loc_z80_wait_states(iCycleCount);
# endif

#define ADC(value) \
{ \
   unsigned val = value; \
   unsigned res = _A + val + (_F & Cflag); \
   _F = SZ[res & 0xff] | ((res >> 8) & Cflag) | ((_A ^ res ^ val) & Hflag) | \
      (((val ^ _A ^ 0x80) & (val ^ res) & 0x80) >> 5); \
   _A = res; \
}

#define ADD(value) \
{ \
   unsigned val = value; \
   unsigned res = _A + val; \
   _F = SZ[(byte)res] | ((res >> 8) & Cflag) | ((_A ^ res ^ val) & Hflag) | \
      (((val ^ _A ^ 0x80) & (val ^ res) & 0x80) >> 5); \
   _A = (byte)res; \
}

#define ADD16(dest, src) \
{ \
   dword res = z80.dest.d + z80.src.d; \
   _F = (_F & (Sflag | Zflag | Vflag)) | (((z80.dest.d ^ res ^ z80.src.d) >> 8) & Hflag) | \
      ((res >> 16) & Cflag) | ((res >> 8) & Xflags); \
   z80.dest.w.l = (word)res; \
}

#define AND(val) \
{ \
   _A &= val; \
   _F = SZP[_A] | Hflag; \
}

#define CALL \
{ \
   reg_pair dest; \
   dest.b.l = read_mem(_PC++); /* subroutine address low byte */ \
   dest.b.h = read_mem(_PC++); /* subroutine address high byte */ \
   write_mem(--_SP, z80.PC.b.h); /* store high byte of current PC */ \
   write_mem(--_SP, z80.PC.b.l); /* store low byte of current PC */ \
   _PC = dest.w.l; /* continue execution at subroutine */ \
}

#define CP(value) \
{ \
   unsigned val = value; \
   unsigned res = _A - val; \
   _F = (SZ[res & 0xff] & (Sflag | Zflag)) | (val & Xflags) | ((res >> 8) & Cflag) | Nflag | ((_A ^ res ^ val) & Hflag) | \
      ((((val ^ _A) & (_A ^ res)) >> 5) & Vflag); \
}

#define DAA \
{ \
   int idx = _A; \
   if(_F & Cflag) \
      idx |= 0x100; \
   if(_F & Hflag) \
      idx |= 0x200; \
   if(_F & Nflag) \
      idx |= 0x400; \
   _AF = DAATable[idx]; \
}

#define DEC(reg) \
{ \
   reg--; \
   _F = (_F & Cflag) | SZHV_dec[reg]; \
}

#define JR \
{ \
   signed char offset; \
   offset = (signed char)(read_mem(_PC)); /* grab signed jump offset */ \
   _PC += offset + 1; /* add offset & correct PC */ \
}

#define EXX \
{ \
   reg_pair temp; \
   temp = z80.BCx; \
   z80.BCx = z80.BC; \
   z80.BC = temp; \
   temp = z80.DEx; \
   z80.DEx = z80.DE; \
   z80.DE = temp; \
   temp = z80.HLx; \
   z80.HLx = z80.HL; \
   z80.HL = temp; \
}

#define EX(op1, op2) \
{ \
   reg_pair temp; \
   temp = op1; \
   op1 = op2; \
   op2 = temp; \
}

#define EX_SP(reg) \
{ \
   reg_pair temp; \
   temp.b.l = read_mem(_SP++); \
   temp.b.h = read_mem(_SP); \
   write_mem(_SP--, z80.reg.b.h); \
   write_mem(_SP, z80.reg.b.l); \
   z80.reg.w.l = temp.w.l; \
}

#define INC(reg) \
{ \
   reg++; \
   _F = (_F & Cflag) | SZHV_inc[reg]; \
}

#define JP \
{ \
   reg_pair addr; \
   addr.b.l = read_mem(_PC++); \
   addr.b.h = read_mem(_PC); \
   _PC = addr.w.l; \
}

#define LD16_MEM(reg) \
{ \
   reg_pair addr; \
   addr.b.l = read_mem(_PC++); \
   addr.b.h = read_mem(_PC++); \
   z80.reg.b.l = read_mem(addr.w.l); \
   z80.reg.b.h = read_mem(addr.w.l+1); \
}

#define LDMEM_16(reg) \
{ \
   reg_pair addr; \
   addr.b.l = read_mem(_PC++); \
   addr.b.h = read_mem(_PC++); \
   write_mem(addr.w.l, z80.reg.b.l); \
   write_mem(addr.w.l+1, z80.reg.b.h); \
}

#define OR(val) \
{ \
   _A |= val; \
   _F = SZP[_A]; \
}

#define POP(reg) \
{ \
   z80.reg.b.l = read_mem(_SP++); \
   z80.reg.b.h = read_mem(_SP++); \
}

#define PUSH(reg) \
{ \
   write_mem(--_SP, z80.reg.b.h); \
   write_mem(--_SP, z80.reg.b.l); \
}

#define RET \
{ \
   z80.PC.b.l = read_mem(_SP++); \
   z80.PC.b.h = read_mem(_SP++); \
}

#define RLA \
{ \
   byte res = (_A << 1) | (_F & Cflag); \
   byte carry = (_A & 0x80) ? Cflag : 0; \
   _F = (_F & (Sflag | Zflag | Pflag)) | carry | (res & Xflags); \
   _A = res; \
}

#define RLCA \
{ \
   _A = (_A << 1) | (_A >> 7); \
   _F = (_F & (Sflag | Zflag | Pflag)) | (_A & (Xflags | Cflag)); \
}

#define RRA \
{ \
   byte res = (_A >> 1) | (_F << 7); \
   byte carry = (_A & 0x01) ? Cflag : 0; \
   _F = (_F & (Sflag | Zflag | Pflag)) | carry | (res & Xflags); \
   _A = res; \
}

#define RRCA \
{ \
   _F = (_F & (Sflag | Zflag | Pflag)) | (_A & Cflag); \
   _A = (_A >> 1) | (_A << 7); \
   _F |= (_A & Xflags); \
}

#define RST(addr) \
{ \
   write_mem(--_SP, z80.PC.b.h); /* store high byte of current PC */ \
   write_mem(--_SP, z80.PC.b.l); /* store low byte of current PC */ \
   _PC = addr; /* continue execution at restart address */ \
}

#define SBC(value) \
{ \
   unsigned val = value; \
   unsigned res = _A - val - (_F & Cflag); \
   _F = SZ[res & 0xff] | ((res >> 8) & Cflag) | Nflag | ((_A ^ res ^ val) & Hflag) | \
      (((val ^ _A) & (_A ^ res) & 0x80) >> 5); \
   _A = res; \
}

#define SUB(value) \
{ \
   unsigned val = value; \
   unsigned res = _A - val; \
   _F = SZ[res & 0xff] | ((res >> 8) & Cflag) | Nflag | ((_A ^ res ^ val) & Hflag) | \
      (((val ^ _A) & (_A ^ res) & 0x80) >> 5); \
   _A = res; \
}

#define XOR(val) \
{ \
   _A ^= val; \
   _F = SZP[_A]; \
}

#define BIT(bit, reg) \
   _F = (_F & Cflag) | Hflag | SZ_BIT[reg & (1 << bit)]

#define BIT_XY BIT

inline byte RES(byte bit, byte val) {
   return val & ~(1 << bit);
}

inline byte RLC(byte val) {
   unsigned res = val;
   unsigned carry = (res & 0x80) ? Cflag : 0;
   res = ((res << 1) | (res >> 7)) & 0xff;
   _F = SZP[res] | carry;
   return res;
}

inline byte RL(byte val) {
   unsigned res = val;
   unsigned carry = (res & 0x80) ? Cflag : 0;
   res = ((res << 1) | (_F & Cflag)) & 0xff;
   _F = SZP[res] | carry;
   return res;
}

inline byte RRC(byte val) {
   unsigned res = val;
   unsigned carry = (res & 0x01) ? Cflag : 0;
   res = ((res >> 1) | (res << 7)) & 0xff;
   _F = SZP[res] | carry;
   return res;
}

inline byte RR(byte val) {
   unsigned res = val;
   unsigned carry = (res & 0x01) ? Cflag : 0;
   res = ((res >> 1) | (_F << 7)) & 0xff;
   _F = SZP[res] | carry;
   return res;
}

inline byte SET(byte bit, byte val) {
   return val | (1 << bit);
}

inline byte SLA(byte val) {
   unsigned res = val;
   unsigned carry = (res & 0x80) ? Cflag : 0;
   res = (res << 1) & 0xff;
   _F = SZP[res] | carry;
   return res;
}

inline byte SLL(byte val) {
   unsigned res = val;
   unsigned carry = (res & 0x80) ? Cflag : 0;
   res = ((res << 1) | 0x01) & 0xff;
   _F = SZP[res] | carry;
   return res;
}

inline byte SRA(byte val) {
   unsigned res = val;
   unsigned carry = (res & 0x01) ? Cflag : 0;
   res = ((res >> 1) | (res & 0x80)) & 0xff;
   _F = SZP[res] | carry;
   return res;
}

inline byte SRL(byte val) {
   unsigned res = val;
   unsigned carry = (res & 0x01) ? Cflag : 0;
   res = (res >> 1) & 0xff;
   _F = SZP[res] | carry;
   return res;
}

#define ADC16(reg) \
{ \
   dword res = _HLdword + z80.reg.d + (_F & Cflag); \
   _F = (((_HLdword ^ res ^ z80.reg.d) >> 8) & Hflag) | \
      ((res >> 16) & Cflag) | \
      ((res >> 8) & (Sflag | Xflags)) | \
      ((res & 0xffff) ? 0 : Zflag) | \
      (((z80.reg.d ^ _HLdword ^ 0x8000) & (z80.reg.d ^ res) & 0x8000) >> 13); \
   _HL = (word)res; \
}

#define CPD \
{ \
   byte val = read_mem(_HL); \
   byte res = _A - val; \
   _HL--; \
   _BC--; \
   _F = (_F & Cflag) | (SZ[res] & ~Xflags) | ((_A ^ val ^ res) & Hflag) | Nflag; \
   if(_F & Hflag) res -= 1; \
   if(res & 0x02) _F |= 0x20; \
   if(res & 0x08) _F |= 0x08; \
   if(_BC) _F |= Vflag; \
}

#define CPDR \
   CPD; \
   if(_BC && !(_F & Zflag)) \
   { \
      iCycleCount += cc_ex[bOpCode]; \
      _PC -= 2; \
      iWSAdjust++; \
   }

#define CPI \
{ \
   byte val = read_mem(_HL); \
   byte res = _A - val; \
   _HL++; \
   _BC--; \
   _F = (_F & Cflag) | (SZ[res] & ~Xflags) | ((_A ^ val ^ res) & Hflag) | Nflag; \
   if(_F & Hflag) res -= 1; \
   if(res & 0x02) _F |= 0x20; \
   if(res & 0x08) _F |= 0x08; \
   if(_BC) _F |= Vflag; \
}

#define CPIR \
   CPI; \
   if(_BC && !(_F & Zflag)) \
   { \
      iCycleCount += cc_ex[bOpCode]; \
      _PC -= 2; \
      iWSAdjust++; \
   }

#define IND \
{ \
   byte io = z80_IN_handler(z80.BC); \
   _B--; \
   write_mem(_HL, io); \
   _HL--; \
   _F = SZ[_B]; \
   if(io & Sflag) _F |= Nflag; \
   if((((_C - 1) & 0xff) + io) & 0x100) _F |= Hflag | Cflag; \
   if((drep_tmp1[_C & 3][io & 3] ^ breg_tmp2[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) \
      _F |= Pflag; \
}

#define INDR \
   IND; \
   if(_B) \
   { \
      iCycleCount += cc_ex[bOpCode]; \
      _PC -= 2; \
   }

#define INI \
{ \
   byte io = z80_IN_handler(z80.BC); \
   _B--; \
   write_mem(_HL, io); \
   _HL++; \
   _F = SZ[_B]; \
   if(io & Sflag) _F |= Nflag; \
   if((((_C + 1) & 0xff) + io) & 0x100) _F |= Hflag | Cflag; \
   if((irep_tmp1[_C & 3][io & 3] ^ breg_tmp2[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) \
      _F |= Pflag; \
}

#define INIR \
   INI; \
   if(_B) \
   { \
      iCycleCount += cc_ex[bOpCode]; \
      _PC -= 2; \
   }

#define LDD \
{ \
   byte io = read_mem(_HL); \
   write_mem(_DE, io); \
   _F &= Sflag | Zflag | Cflag; \
   if((_A + io) & 0x02) _F |= 0x20; \
   if((_A + io) & 0x08) _F |= 0x08; \
   _HL--; \
   _DE--; \
   _BC--; \
   if(_BC) _F |= Vflag; \
}

#define LDDR \
   LDD; \
   if(_BC) \
   { \
      iCycleCount += cc_ex[bOpCode]; \
      _PC -= 2; \
   }

#define LDI \
{ \
   byte io = read_mem(_HL); \
   write_mem(_DE, io); \
   _F &= Sflag | Zflag | Cflag; \
   if((_A + io) & 0x02) _F |= 0x20; \
   if((_A + io) & 0x08) _F |= 0x08; \
   _HL++; \
   _DE++; \
   _BC--; \
   if(_BC) _F |= Vflag; \
}

#define LDIR \
   LDI; \
   if(_BC) \
   { \
      iCycleCount += cc_ex[bOpCode]; \
      _PC -= 2; \
   }

#define NEG \
{ \
   byte value = _A; \
   _A = 0; \
   SUB(value); \
}

#define OUTD \
{ \
   byte io = read_mem(_HL); \
   _B--; \
   z80_OUT_handler(z80.BC, io); \
   _HL--; \
   _F = SZ[_B]; \
   if(io & Sflag) _F |= Nflag; \
   if((((_C - 1) & 0xff) + io) & 0x100) _F |= Hflag | Cflag; \
   if((drep_tmp1[_C & 3][io & 3] ^ breg_tmp2[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) \
      _F |= Pflag; \
}

#define OTDR \
   OUTD; \
   if(_B) \
   { \
      iCycleCount += cc_ex[bOpCode]; \
      _PC -= 2; \
   }

#define OUTI \
{ \
   byte io = read_mem(_HL); \
   _B--; \
   z80_OUT_handler(z80.BC, io); \
   _HL++; \
   _F = SZ[_B]; \
   if(io & Sflag) _F |= Nflag; \
   if((((_C + 1) & 0xff) + io) & 0x100) _F |= Hflag | Cflag; \
   if((irep_tmp1[_C & 3][io & 3] ^ breg_tmp2[_B] ^ (_C >> 2) ^ (io >> 2)) & 1) \
      _F |= Pflag; \
}

#define OTIR \
   OUTI; \
   if(_B) \
   { \
      iCycleCount += cc_ex[bOpCode]; \
      _PC -= 2; \
   }

#define RLD \
{ \
   byte n = read_mem(_HL); \
   write_mem(_HL, (n << 4) | (_A & 0x0f)); \
   _A = (_A & 0xf0) | (n >> 4); \
   _F = (_F & Cflag) | SZP[_A]; \
}

#define RRD \
{ \
   byte n = read_mem(_HL); \
   write_mem(_HL, (n >> 4) | (_A << 4)); \
   _A = (_A & 0xf0) | (n & 0x0f); \
   _F = (_F & Cflag) | SZP[_A]; \
}

#define SBC16(reg) \
{ \
   dword res = _HLdword - z80.reg.d - (_F & Cflag); \
   _F = (((_HLdword ^ res ^ z80.reg.d) >> 8) & Hflag) | Nflag | \
      ((res >> 16) & Cflag) | \
      ((res >> 8) & (Sflag | Xflags)) | \
      ((res & 0xffff) ? 0 : Zflag) | \
      (((z80.reg.d ^ _HLdword) & (_HLdword ^ res) &0x8000) >> 13); \
   _HL = (word)res; \
}



#define z80_int_handler \
{ \
   if (_IFF1) { /* process interrupts? */ \
      _R++; \
      _IFF1 = _IFF2 = 0; /* clear interrupt flip-flops */ \
      z80.int_pending = 0; \
      GateArray.sl_count &= 0x1f; /* clear bit 5 of GA scanline counter */ \
      if (_HALT) { /* HALT instruction active? */ \
         _HALT = 0; /* exit HALT 'loop' */ \
         _PC++; /* correct PC */ \
      } \
      if (_IM < 2) { /* interrupt mode 0 or 1? (IM0 = IM1 on the CPC) */ \
         iCycleCount = 20; \
         if (iWSAdjust) { \
            iCycleCount -= 4; \
         } \
         RST(0x0038); \
         z80_wait_states \
      } \
      else { /* interrupt mode 2 */ \
         reg_pair addr; \
         iCycleCount = 28; /* was 76 */ \
         if (iWSAdjust) { \
            iCycleCount -= 4; \
         } \
         write_mem(--_SP, z80.PC.b.h); /* store high byte of current PC */ \
         write_mem(--_SP, z80.PC.b.l); /* store low byte of current PC */ \
         addr.b.l = 0xff; /* assemble pointer */ \
         addr.b.h = _I; \
         z80.PC.b.l = read_mem(addr.w.l); /* retrieve low byte of vector */ \
         z80.PC.b.h = read_mem(addr.w.l+1); /* retrieve high byte of vector */ \
         z80_wait_states \
      } \
   } \
}



void z80_init_tables(void)
{
   int i, p;

   for (i = 0; i < 256; i++) {
      p = 0;
      if(i & 0x01) ++p;
      if(i & 0x02) ++p;
      if(i & 0x04) ++p;
      if(i & 0x08) ++p;
      if(i & 0x10) ++p;
      if(i & 0x20) ++p;
      if(i & 0x40) ++p;
      if(i & 0x80) ++p;
      SZ[i] = i ? i & Sflag : Zflag;
      SZ[i] |= (i & Xflags);
      SZ_BIT[i] = i ? i & Sflag : Zflag | Pflag;
      SZ_BIT[i] |= (i & Xflags);
      SZP[i] = SZ[i] | ((p & 1) ? 0 : Pflag);
      SZHV_inc[i] = SZ[i];
      if(i == 0x80) SZHV_inc[i] |= Vflag;
      if((i & 0x0f) == 0x00) SZHV_inc[i] |= Hflag;
      SZHV_dec[i] = SZ[i] | Nflag;
      if(i == 0x7f) SZHV_dec[i] |= Vflag;
      if((i & 0x0f) == 0x0f) SZHV_dec[i] |= Hflag;
   }
}

void 
z80_execute(void)
{
    __label__ 
   nop, ld_bc_word, ld_mbc_a, inc_bc, inc_b, dec_b, ld_b_byte, rlca,
   ex_af_af, add_hl_bc, ld_a_mbc, dec_bc, inc_c, dec_c, ld_c_byte, rrca,
   djnz, ld_de_word, ld_mde_a, inc_de, inc_d, dec_d, ld_d_byte, rla,
   jr, add_hl_de, ld_a_mde, dec_de, inc_e, dec_e, ld_e_byte, rra,
   jr_nz, ld_hl_word, ld_mword_hl, inc_hl, inc_h, dec_h, ld_h_byte, daa,
   jr_z, add_hl_hl, ld_hl_mword, dec_hl, inc_l, dec_l, ld_l_byte, cpl,
   jr_nc, ld_sp_word, ld_mword_a, inc_sp, inc_mhl, dec_mhl, ld_mhl_byte, scf,
   jr_c, add_hl_sp, ld_a_mword, dec_sp, inc_a, dec_a, ld_a_byte, ccf,
   ld_b_b, ld_b_c, ld_b_d, ld_b_e, ld_b_h, ld_b_l, ld_b_mhl, ld_b_a,
   ld_c_b, ld_c_c, ld_c_d, ld_c_e, ld_c_h, ld_c_l, ld_c_mhl, ld_c_a,
   ld_d_b, ld_d_c, ld_d_d, ld_d_e, ld_d_h, ld_d_l, ld_d_mhl, ld_d_a,
   ld_e_b, ld_e_c, ld_e_d, ld_e_e, ld_e_h, ld_e_l, ld_e_mhl, ld_e_a,
   ld_h_b, ld_h_c, ld_h_d, ld_h_e, ld_h_h, ld_h_l, ld_h_mhl, ld_h_a,
   ld_l_b, ld_l_c, ld_l_d, ld_l_e, ld_l_h, ld_l_l, ld_l_mhl, ld_l_a,
   ld_mhl_b, ld_mhl_c, ld_mhl_d, ld_mhl_e, ld_mhl_h, ld_mhl_l, halt, ld_mhl_a,
   ld_a_b, ld_a_c, ld_a_d, ld_a_e, ld_a_h, ld_a_l, ld_a_mhl, ld_a_a,
   add_b, add_c, add_d, add_e, add_h, add_l, add_mhl, add_a,
   adc_b, adc_c, adc_d, adc_e, adc_h, adc_l, adc_mhl, adc_a,
   sub_b, sub_c, sub_d, sub_e, sub_h, sub_l, sub_mhl, sub_a,
   sbc_b, sbc_c, sbc_d, sbc_e, sbc_h, sbc_l, sbc_mhl, sbc_a,
   and_b, and_c, and_d, and_e, and_h, and_l, and_mhl, and_a,
   xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, xor_mhl, xor_a,
   or_b, or_c, or_d, or_e, or_h, or_l, or_mhl, or_a,
   cp_b, cp_c, cp_d, cp_e, cp_h, cp_l, cp_mhl, cp_a,
   ret_nz, pop_bc, jp_nz, jp, call_nz, push_bc, add_byte, rst00,
   ret_z, ret, jp_z, pfx_cb, call_z, call, adc_byte, rst08,
   ret_nc, pop_de, jp_nc, outa, call_nc, push_de, sub_byte, rst10,
   ret_c, exx, jp_c, ina, call_c, pfx_dd, sbc_byte, rst18,
   ret_po, pop_hl, jp_po, ex_msp_hl, call_po, push_hl, and_byte, rst20,
   ret_pe, ld_pc_hl, jp_pe, ex_de_hl, call_pe, pfx_ed, xor_byte, rst28,
   ret_p, pop_af, jp_p, di, call_p, push_af, or_byte, rst30,
   ret_m, ld_sp_hl, jp_m, ei, call_m, pfx_fd, cp_byte, rst38;

    static const void* const a_jump_table[256] = 
	  {
   && nop,&& ld_bc_word,&& ld_mbc_a,&& inc_bc,&& inc_b,&& dec_b,&& ld_b_byte,&& rlca,&&
   ex_af_af,&& add_hl_bc,&& ld_a_mbc,&& dec_bc,&& inc_c,&& dec_c,&& ld_c_byte,&& rrca,&&
   djnz,&& ld_de_word,&& ld_mde_a,&& inc_de,&& inc_d,&& dec_d,&& ld_d_byte,&& rla,&&
   jr,&& add_hl_de,&& ld_a_mde,&& dec_de,&& inc_e,&& dec_e,&& ld_e_byte,&& rra,&&
   jr_nz,&& ld_hl_word,&& ld_mword_hl,&& inc_hl,&& inc_h,&& dec_h,&& ld_h_byte,&& daa,&&
   jr_z,&& add_hl_hl,&& ld_hl_mword,&& dec_hl,&& inc_l,&& dec_l,&& ld_l_byte,&& cpl,&&
   jr_nc,&& ld_sp_word,&& ld_mword_a,&& inc_sp,&& inc_mhl,&& dec_mhl,&& ld_mhl_byte,&& scf,&&
   jr_c,&& add_hl_sp,&& ld_a_mword,&& dec_sp,&& inc_a,&& dec_a,&& ld_a_byte,&& ccf,&&
   ld_b_b,&& ld_b_c,&& ld_b_d,&& ld_b_e,&& ld_b_h,&& ld_b_l,&& ld_b_mhl,&& ld_b_a,&&
   ld_c_b,&& ld_c_c,&& ld_c_d,&& ld_c_e,&& ld_c_h,&& ld_c_l,&& ld_c_mhl,&& ld_c_a,&&
   ld_d_b,&& ld_d_c,&& ld_d_d,&& ld_d_e,&& ld_d_h,&& ld_d_l,&& ld_d_mhl,&& ld_d_a,&&
   ld_e_b,&& ld_e_c,&& ld_e_d,&& ld_e_e,&& ld_e_h,&& ld_e_l,&& ld_e_mhl,&& ld_e_a,&&
   ld_h_b,&& ld_h_c,&& ld_h_d,&& ld_h_e,&& ld_h_h,&& ld_h_l,&& ld_h_mhl,&& ld_h_a,&&
   ld_l_b,&& ld_l_c,&& ld_l_d,&& ld_l_e,&& ld_l_h,&& ld_l_l,&& ld_l_mhl,&& ld_l_a,&&
   ld_mhl_b,&& ld_mhl_c,&& ld_mhl_d,&& ld_mhl_e,&& ld_mhl_h,&& ld_mhl_l,&& halt,&& ld_mhl_a,&&
   ld_a_b,&& ld_a_c,&& ld_a_d,&& ld_a_e,&& ld_a_h,&& ld_a_l,&& ld_a_mhl,&& ld_a_a,&&
   add_b,&& add_c,&& add_d,&& add_e,&& add_h,&& add_l,&& add_mhl,&& add_a,&&
   adc_b,&& adc_c,&& adc_d,&& adc_e,&& adc_h,&& adc_l,&& adc_mhl,&& adc_a,&&
   sub_b,&& sub_c,&& sub_d,&& sub_e,&& sub_h,&& sub_l,&& sub_mhl,&& sub_a,&&
   sbc_b,&& sbc_c,&& sbc_d,&& sbc_e,&& sbc_h,&& sbc_l,&& sbc_mhl,&& sbc_a,&&
   and_b,&& and_c,&& and_d,&& and_e,&& and_h,&& and_l,&& and_mhl,&& and_a,&&
   xor_b,&& xor_c,&& xor_d,&& xor_e,&& xor_h,&& xor_l,&& xor_mhl,&& xor_a,&&
   or_b,&& or_c,&& or_d,&& or_e,&& or_h,&& or_l,&& or_mhl,&& or_a,&&
   cp_b,&& cp_c,&& cp_d,&& cp_e,&& cp_h,&& cp_l,&& cp_mhl,&& cp_a,&&
   ret_nz,&& pop_bc,&& jp_nz,&& jp,&& call_nz,&& push_bc,&& add_byte,&& rst00,&&
   ret_z,&& ret,&& jp_z,&& pfx_cb,&& call_z,&& call,&& adc_byte,&& rst08,&&
   ret_nc,&& pop_de,&& jp_nc,&& outa,&& call_nc,&& push_de,&& sub_byte,&& rst10,&&
   ret_c,&& exx,&& jp_c,&& ina,&& call_c,&& pfx_dd,&& sbc_byte,&& rst18,&&
   ret_po,&& pop_hl,&& jp_po,&& ex_msp_hl,&& call_po,&& push_hl,&& and_byte,&& rst20,&&
   ret_pe,&& ld_pc_hl,&& jp_pe,&& ex_de_hl,&& call_pe,&& pfx_ed,&& xor_byte,&& rst28,&&
   ret_p,&& pop_af,&& jp_p,&& di,&& call_p,&& push_af,&& or_byte,&& rst30,&&
   ret_m,&& ld_sp_hl,&& jp_m,&& ei,&& call_m,&& pfx_fd,&& cp_byte,&& rst38
	  };

   byte bOpCode;

   goto lab_beg;
 //{
    
lab_end:
   z80_wait_states

   if (z80.EI_issued) { // EI 'delay' in effect?
      if (--z80.EI_issued == 0) {
         _IFF1 = _IFF2 = Pflag; // set interrupt flip-flops
         if (z80.int_pending) {
            z80_int_handler
         }
      }
   }
   else if (z80.int_pending) { // any interrupts pending?
      z80_int_handler
   }
   iWSAdjust = 0;

lab_beg:
   bOpCode = read_mem(_PC++);
   iCycleCount = cc_op[bOpCode];
   _R++;
	 goto *a_jump_table[bOpCode];
   //{
      pfx_cb:      z80_pfx_cb(); goto lab_end;
      pfx_dd:      z80_pfx_dd(); goto lab_end;
      pfx_ed:      z80_pfx_ed(); goto lab_end;
      pfx_fd:      z80_pfx_fd(); goto lab_end;
      adc_a:       ADC(_A); goto lab_end;
      adc_b:       ADC(_B); goto lab_end;
      adc_byte:    ADC(read_mem(_PC++)); goto lab_end;
      adc_c:       ADC(_C); goto lab_end;
      adc_d:       ADC(_D); goto lab_end;
      adc_e:       ADC(_E); goto lab_end;
      adc_h:       ADC(_H); goto lab_end;
      adc_l:       ADC(_L); goto lab_end;
      adc_mhl:     ADC(read_mem(_HL)); goto lab_end;
      add_a:       ADD(_A); goto lab_end;
      add_b:       ADD(_B); goto lab_end;
      add_byte:    ADD(read_mem(_PC++)); goto lab_end;
      add_c:       ADD(_C); goto lab_end;
      add_d:       ADD(_D); goto lab_end;
      add_e:       ADD(_E); goto lab_end;
      add_h:       ADD(_H); goto lab_end;
      add_hl_bc:   ADD16(HL, BC); goto lab_end;
      add_hl_de:   ADD16(HL, DE); goto lab_end;
      add_hl_hl:   ADD16(HL, HL); goto lab_end;
      add_hl_sp:   ADD16(HL, SP); goto lab_end;
      add_l:       ADD(_L); goto lab_end;
      add_mhl:     ADD(read_mem(_HL)); goto lab_end;
      and_a:       AND(_A); goto lab_end;
      and_b:       AND(_B); goto lab_end;
      and_byte:    AND(read_mem(_PC++)); goto lab_end;
      and_c:       AND(_C); goto lab_end;
      and_d:       AND(_D); goto lab_end;
      and_e:       AND(_E); goto lab_end;
      and_h:       AND(_H); goto lab_end;
      and_l:       AND(_L); goto lab_end;
      and_mhl:     AND(read_mem(_HL)); goto lab_end;
      call:        CALL; goto lab_end;
      call_c:      if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } goto lab_end;
      call_m:      if (_F & Sflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } goto lab_end;
      call_nc:     if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } goto lab_end;
      call_nz:     if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } goto lab_end;
      call_p:      if (!(_F & Sflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } goto lab_end;
      call_pe:     if (_F & Pflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } goto lab_end;
      call_po:     if (!(_F & Pflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } goto lab_end;
      call_z:      if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } goto lab_end;
      ccf:         _F = ((_F & (Sflag | Zflag | Pflag | Cflag)) | ((_F & CF) << 4) | (_A & Xflags)) ^ CF; goto lab_end;
      cpl:         _A ^= 0xff; _F = (_F & (Sflag | Zflag | Pflag | Cflag)) | Hflag | Nflag | (_A & Xflags); goto lab_end;
      cp_a:        CP(_A); goto lab_end;
      cp_b:        CP(_B); goto lab_end;
      cp_byte:     CP(read_mem(_PC++)); goto lab_end;
      cp_c:        CP(_C); goto lab_end;
      cp_d:        CP(_D); goto lab_end;
      cp_e:        CP(_E); goto lab_end;
      cp_h:        CP(_H); goto lab_end;
      cp_l:        CP(_L); goto lab_end;
      cp_mhl:      CP(read_mem(_HL)); goto lab_end;
      daa:         DAA; goto lab_end;
      dec_a:       DEC(_A); goto lab_end;
      dec_b:       DEC(_B); goto lab_end;
      dec_bc:      _BC--; iWSAdjust++; goto lab_end;
      dec_c:       DEC(_C); goto lab_end;
      dec_d:       DEC(_D); goto lab_end;
      dec_de:      _DE--; iWSAdjust++; goto lab_end;
      dec_e:       DEC(_E); goto lab_end;
      dec_h:       DEC(_H); goto lab_end;
      dec_hl:      _HL--; iWSAdjust++; goto lab_end;
      dec_l:       DEC(_L); goto lab_end;
      dec_mhl:     { byte b = read_mem(_HL); DEC(b); write_mem(_HL, b); } goto lab_end;
      dec_sp:      _SP--; iWSAdjust++; goto lab_end;
      di:          _IFF1 = _IFF2 = 0; z80.EI_issued = 0; goto lab_end;
      djnz:        if (--_B) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; } goto lab_end;
      ei:          z80.EI_issued = 2; goto lab_end;
      exx:         EXX; goto lab_end;
      ex_af_af:    EX(z80.AF, z80.AFx); goto lab_end;
      ex_de_hl:    EX(z80.DE, z80.HL); goto lab_end;
      ex_msp_hl:   EX_SP(HL); iWSAdjust++; goto lab_end;
      halt:        _HALT = 1; _PC--; goto lab_end;
      ina:         { z80_wait_states iCycleCount = Ia_;} { reg_pair p; p.b.l = read_mem(_PC++); p.b.h = _A; _A = z80_IN_handler(p); } goto lab_end;
      inc_a:       INC(_A); goto lab_end;
      inc_b:       INC(_B); goto lab_end;
      inc_bc:      _BC++; iWSAdjust++; goto lab_end;
      inc_c:       INC(_C); goto lab_end;
      inc_d:       INC(_D); goto lab_end;
      inc_de:      _DE++; iWSAdjust++; goto lab_end;
      inc_e:       INC(_E); goto lab_end;
      inc_h:       INC(_H); goto lab_end;
      inc_hl:      _HL++; iWSAdjust++; goto lab_end;
      inc_l:       INC(_L); goto lab_end;
      inc_mhl:     { byte b = read_mem(_HL); INC(b); write_mem(_HL, b); } goto lab_end;
      inc_sp:      _SP++; iWSAdjust++; goto lab_end;
      jp:          JP; goto lab_end;
      jp_c:        if (_F & Cflag) { JP } else { _PC += 2; }; goto lab_end;
      jp_m:        if (_F & Sflag) { JP } else { _PC += 2; }; goto lab_end;
      jp_nc:       if (!(_F & Cflag)) { JP } else { _PC += 2; }; goto lab_end;
      jp_nz:       if (!(_F & Zflag)) { JP } else { _PC += 2; }; goto lab_end;
      jp_p:        if (!(_F & Sflag)) { JP } else { _PC += 2; }; goto lab_end;
      jp_pe:       if (_F & Pflag) { JP } else { _PC += 2; }; goto lab_end;
      jp_po:       if (!(_F & Pflag)) { JP } else { _PC += 2; }; goto lab_end;
      jp_z:        if (_F & Zflag) { JP } else { _PC += 2; }; goto lab_end;
      jr:          JR; goto lab_end;
      jr_c:        if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; goto lab_end;
      jr_nc:       if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; goto lab_end;
      jr_nz:       if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; goto lab_end;
      jr_z:        if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; goto lab_end;
      ld_a_a:      goto lab_end;
      ld_a_b:      _A = _B; goto lab_end;
      ld_a_byte:   _A = read_mem(_PC++); goto lab_end;
      ld_a_c:      _A = _C; goto lab_end;
      ld_a_d:      _A = _D; goto lab_end;
      ld_a_e:      _A = _E; goto lab_end;
      ld_a_h:      _A = _H; goto lab_end;
      ld_a_l:      _A = _L; goto lab_end;
      ld_a_mbc:    _A = read_mem(_BC); goto lab_end;
      ld_a_mde:    _A = read_mem(_DE); goto lab_end;
      ld_a_mhl:    _A = read_mem(_HL); goto lab_end;
      ld_a_mword:  { reg_pair addr; addr.b.l = read_mem(_PC++); addr.b.h = read_mem(_PC++); _A = read_mem(addr.w.l); } goto lab_end;
      ld_bc_word:  z80.BC.b.l = read_mem(_PC++); z80.BC.b.h = read_mem(_PC++); goto lab_end;
      ld_b_a:      _B = _A; goto lab_end;
      ld_b_b:      goto lab_end;
      ld_b_byte:   _B = read_mem(_PC++); goto lab_end;
      ld_b_c:      _B = _C; goto lab_end;
      ld_b_d:      _B = _D; goto lab_end;
      ld_b_e:      _B = _E; goto lab_end;
      ld_b_h:      _B = _H; goto lab_end;
      ld_b_l:      _B = _L; goto lab_end;
      ld_b_mhl:    _B = read_mem(_HL); goto lab_end;
      ld_c_a:      _C = _A; goto lab_end;
      ld_c_b:      _C = _B; goto lab_end;
      ld_c_byte:   _C = read_mem(_PC++); goto lab_end;
      ld_c_c:      goto lab_end;
      ld_c_d:      _C = _D; goto lab_end;
      ld_c_e:      _C = _E; goto lab_end;
      ld_c_h:      _C = _H; goto lab_end;
      ld_c_l:      _C = _L; goto lab_end;
      ld_c_mhl:    _C = read_mem(_HL); goto lab_end;
      ld_de_word:  z80.DE.b.l = read_mem(_PC++); z80.DE.b.h = read_mem(_PC++); goto lab_end;
      ld_d_a:      _D = _A; goto lab_end;
      ld_d_b:      _D = _B; goto lab_end;
      ld_d_byte:   _D = read_mem(_PC++); goto lab_end;
      ld_d_c:      _D = _C; goto lab_end;
      ld_d_d:      goto lab_end;
      ld_d_e:      _D = _E; goto lab_end;
      ld_d_h:      _D = _H; goto lab_end;
      ld_d_l:      _D = _L; goto lab_end;
      ld_d_mhl:    _D = read_mem(_HL); goto lab_end;
      ld_e_a:      _E = _A; goto lab_end;
      ld_e_b:      _E = _B; goto lab_end;
      ld_e_byte:   _E = read_mem(_PC++); goto lab_end;
      ld_e_c:      _E = _C; goto lab_end;
      ld_e_d:      _E = _D; goto lab_end;
      ld_e_e:      goto lab_end;
      ld_e_h:      _E = _H; goto lab_end;
      ld_e_l:      _E = _L; goto lab_end;
      ld_e_mhl:    _E = read_mem(_HL); goto lab_end;
      ld_hl_mword: LD16_MEM(HL); goto lab_end;
      ld_hl_word:  z80.HL.b.l = read_mem(_PC++); z80.HL.b.h = read_mem(_PC++); goto lab_end;
      ld_h_a:      _H = _A; goto lab_end;
      ld_h_b:      _H = _B; goto lab_end;
      ld_h_byte:   _H = read_mem(_PC++); goto lab_end;
      ld_h_c:      _H = _C; goto lab_end;
      ld_h_d:      _H = _D; goto lab_end;
      ld_h_e:      _H = _E; goto lab_end;
      ld_h_h:      goto lab_end;
      ld_h_l:      _H = _L; goto lab_end;
      ld_h_mhl:    _H = read_mem(_HL); goto lab_end;
      ld_l_a:      _L = _A; goto lab_end;
      ld_l_b:      _L = _B; goto lab_end;
      ld_l_byte:   _L = read_mem(_PC++); goto lab_end;
      ld_l_c:      _L = _C; goto lab_end;
      ld_l_d:      _L = _D; goto lab_end;
      ld_l_e:      _L = _E; goto lab_end;
      ld_l_h:      _L = _H; goto lab_end;
      ld_l_l:      goto lab_end;
      ld_l_mhl:    _L = read_mem(_HL); goto lab_end;
      ld_mbc_a:    write_mem(_BC, _A); goto lab_end;
      ld_mde_a:    write_mem(_DE, _A); goto lab_end;
      ld_mhl_a:    write_mem(_HL, _A); goto lab_end;
      ld_mhl_b:    write_mem(_HL, _B); goto lab_end;
      ld_mhl_byte: { byte b = read_mem(_PC++); write_mem(_HL, b); } goto lab_end;
      ld_mhl_c:    write_mem(_HL, _C); goto lab_end;
      ld_mhl_d:    write_mem(_HL, _D); goto lab_end;
      ld_mhl_e:    write_mem(_HL, _E); goto lab_end;
      ld_mhl_h:    write_mem(_HL, _H); goto lab_end;
      ld_mhl_l:    write_mem(_HL, _L); goto lab_end;
      ld_mword_a:  { reg_pair addr; addr.b.l = read_mem(_PC++); addr.b.h = read_mem(_PC++); write_mem(addr.w.l, _A); } goto lab_end;
      ld_mword_hl: LDMEM_16(HL); goto lab_end;
      ld_pc_hl:    _PC = _HL; goto lab_end;
      ld_sp_hl:    _SP = _HL; iWSAdjust++; goto lab_end;
      ld_sp_word:  z80.SP.b.l = read_mem(_PC++); z80.SP.b.h = read_mem(_PC++); goto lab_end;
      nop:         goto lab_end;
      or_a:        OR(_A); goto lab_end;
      or_b:        OR(_B); goto lab_end;
      or_byte:     OR(read_mem(_PC++)); goto lab_end;
      or_c:        OR(_C); goto lab_end;
      or_d:        OR(_D); goto lab_end;
      or_e:        OR(_E); goto lab_end;
      or_h:        OR(_H); goto lab_end;
      or_l:        OR(_L); goto lab_end;
      or_mhl:      OR(read_mem(_HL)); goto lab_end;
      outa:        { z80_wait_states iCycleCount = Oa_;} { reg_pair p; p.b.l = read_mem(_PC++); p.b.h = _A; z80_OUT_handler(p, _A); } goto lab_end;
      pop_af:      POP(AF); goto lab_end;
      pop_bc:      POP(BC); goto lab_end;
      pop_de:      POP(DE); goto lab_end;
      pop_hl:      POP(HL); goto lab_end;
      push_af:     PUSH(AF); goto lab_end;
      push_bc:     PUSH(BC); goto lab_end;
      push_de:     PUSH(DE); goto lab_end;
      push_hl:     PUSH(HL); goto lab_end;
      ret:         RET; goto lab_end;
      ret_c:       if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; goto lab_end;
      ret_m:       if (_F & Sflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; goto lab_end;
      ret_nc:      if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; goto lab_end;
      ret_nz:      if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; goto lab_end;
      ret_p:       if (!(_F & Sflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; goto lab_end;
      ret_pe:      if (_F & Pflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; goto lab_end;
      ret_po:      if (!(_F & Pflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; goto lab_end;
      ret_z:       if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; goto lab_end;
      rla:         RLA; goto lab_end;
      rlca:        RLCA; goto lab_end;
      rra:         RRA; goto lab_end;
      rrca:        RRCA; goto lab_end;
      rst00:       RST(0x0000); goto lab_end;
      rst08:       RST(0x0008); goto lab_end;
      rst10:       RST(0x0010); goto lab_end;
      rst18:       RST(0x0018); goto lab_end;
      rst20:       RST(0x0020); goto lab_end;
      rst28:       RST(0x0028); goto lab_end;
      rst30:       RST(0x0030); goto lab_end;
      rst38:       RST(0x0038); goto lab_end;
      sbc_a:       SBC(_A); goto lab_end;
      sbc_b:       SBC(_B); goto lab_end;
      sbc_byte:    SBC(read_mem(_PC++)); goto lab_end;
      sbc_c:       SBC(_C); goto lab_end;
      sbc_d:       SBC(_D); goto lab_end;
      sbc_e:       SBC(_E); goto lab_end;
      sbc_h:       SBC(_H); goto lab_end;
      sbc_l:       SBC(_L); goto lab_end;
      sbc_mhl:     SBC(read_mem(_HL)); goto lab_end;
      scf:         _F = (_F & (Sflag | Zflag | Pflag)) | Cflag | (_A & Xflags); goto lab_end;
      sub_a:       SUB(_A); goto lab_end;
      sub_b:       SUB(_B); goto lab_end;
      sub_byte:    SUB(read_mem(_PC++)); goto lab_end;
      sub_c:       SUB(_C); goto lab_end;
      sub_d:       SUB(_D); goto lab_end;
      sub_e:       SUB(_E); goto lab_end;
      sub_h:       SUB(_H); goto lab_end;
      sub_l:       SUB(_L); goto lab_end;
      sub_mhl:     SUB(read_mem(_HL)); goto lab_end;
      xor_a:       XOR(_A); goto lab_end;
      xor_b:       XOR(_B); goto lab_end;
      xor_byte:    XOR(read_mem(_PC++)); goto lab_end;
      xor_c:       XOR(_C); goto lab_end;
      xor_d:       XOR(_D); goto lab_end;
      xor_e:       XOR(_E); goto lab_end;
      xor_h:       XOR(_H); goto lab_end;
      xor_l:       XOR(_L); goto lab_end;
      xor_mhl:     XOR(read_mem(_HL)); goto lab_end;
   //}
 //}
   //LUDO: return EC_BREAKPOINT;
   goto lab_end;
}



# ifdef Z80_JMP_TBL
void z80_pfx_cb(void)
{
   __label__
   rlc_b, rlc_c, rlc_d, rlc_e, rlc_h, rlc_l, rlc_mhl, rlc_a,
   rrc_b, rrc_c, rrc_d, rrc_e, rrc_h, rrc_l, rrc_mhl, rrc_a,
   rl_b, rl_c, rl_d, rl_e, rl_h, rl_l, rl_mhl, rl_a,
   rr_b, rr_c, rr_d, rr_e, rr_h, rr_l, rr_mhl, rr_a,
   sla_b, sla_c, sla_d, sla_e, sla_h, sla_l, sla_mhl, sla_a,
   sra_b, sra_c, sra_d, sra_e, sra_h, sra_l, sra_mhl, sra_a,
   sll_b, sll_c, sll_d, sll_e, sll_h, sll_l, sll_mhl, sll_a,
   srl_b, srl_c, srl_d, srl_e, srl_h, srl_l, srl_mhl, srl_a,
   bit0_b, bit0_c, bit0_d, bit0_e, bit0_h, bit0_l, bit0_mhl, bit0_a,
   bit1_b, bit1_c, bit1_d, bit1_e, bit1_h, bit1_l, bit1_mhl, bit1_a,
   bit2_b, bit2_c, bit2_d, bit2_e, bit2_h, bit2_l, bit2_mhl, bit2_a,
   bit3_b, bit3_c, bit3_d, bit3_e, bit3_h, bit3_l, bit3_mhl, bit3_a,
   bit4_b, bit4_c, bit4_d, bit4_e, bit4_h, bit4_l, bit4_mhl, bit4_a,
   bit5_b, bit5_c, bit5_d, bit5_e, bit5_h, bit5_l, bit5_mhl, bit5_a,
   bit6_b, bit6_c, bit6_d, bit6_e, bit6_h, bit6_l, bit6_mhl, bit6_a,
   bit7_b, bit7_c, bit7_d, bit7_e, bit7_h, bit7_l, bit7_mhl, bit7_a,
   res0_b, res0_c, res0_d, res0_e, res0_h, res0_l, res0_mhl, res0_a,
   res1_b, res1_c, res1_d, res1_e, res1_h, res1_l, res1_mhl, res1_a,
   res2_b, res2_c, res2_d, res2_e, res2_h, res2_l, res2_mhl, res2_a,
   res3_b, res3_c, res3_d, res3_e, res3_h, res3_l, res3_mhl, res3_a,
   res4_b, res4_c, res4_d, res4_e, res4_h, res4_l, res4_mhl, res4_a,
   res5_b, res5_c, res5_d, res5_e, res5_h, res5_l, res5_mhl, res5_a,
   res6_b, res6_c, res6_d, res6_e, res6_h, res6_l, res6_mhl, res6_a,
   res7_b, res7_c, res7_d, res7_e, res7_h, res7_l, res7_mhl, res7_a,
   set0_b, set0_c, set0_d, set0_e, set0_h, set0_l, set0_mhl, set0_a,
   set1_b, set1_c, set1_d, set1_e, set1_h, set1_l, set1_mhl, set1_a,
   set2_b, set2_c, set2_d, set2_e, set2_h, set2_l, set2_mhl, set2_a,
   set3_b, set3_c, set3_d, set3_e, set3_h, set3_l, set3_mhl, set3_a,
   set4_b, set4_c, set4_d, set4_e, set4_h, set4_l, set4_mhl, set4_a,
   set5_b, set5_c, set5_d, set5_e, set5_h, set5_l, set5_mhl, set5_a,
   set6_b, set6_c, set6_d, set6_e, set6_h, set6_l, set6_mhl, set6_a,
   set7_b, set7_c, set7_d, set7_e, set7_h, set7_l, set7_mhl, set7_a;

    static const void* const a_jump_table[256] =  { &&
   rlc_b,&& rlc_c,&& rlc_d,&& rlc_e,&& rlc_h,&& rlc_l,&& rlc_mhl,&& rlc_a,&&
   rrc_b,&& rrc_c,&& rrc_d,&& rrc_e,&& rrc_h,&& rrc_l,&& rrc_mhl,&& rrc_a,&&
   rl_b,&& rl_c,&& rl_d,&& rl_e,&& rl_h,&& rl_l,&& rl_mhl,&& rl_a,&&
   rr_b,&& rr_c,&& rr_d,&& rr_e,&& rr_h,&& rr_l,&& rr_mhl,&& rr_a,&&
   sla_b,&& sla_c,&& sla_d,&& sla_e,&& sla_h,&& sla_l,&& sla_mhl,&& sla_a,&&
   sra_b,&& sra_c,&& sra_d,&& sra_e,&& sra_h,&& sra_l,&& sra_mhl,&& sra_a,&&
   sll_b,&& sll_c,&& sll_d,&& sll_e,&& sll_h,&& sll_l,&& sll_mhl,&& sll_a,&&
   srl_b,&& srl_c,&& srl_d,&& srl_e,&& srl_h,&& srl_l,&& srl_mhl,&& srl_a,&&
   bit0_b,&& bit0_c,&& bit0_d,&& bit0_e,&& bit0_h,&& bit0_l,&& bit0_mhl,&& bit0_a,&&
   bit1_b,&& bit1_c,&& bit1_d,&& bit1_e,&& bit1_h,&& bit1_l,&& bit1_mhl,&& bit1_a,&&
   bit2_b,&& bit2_c,&& bit2_d,&& bit2_e,&& bit2_h,&& bit2_l,&& bit2_mhl,&& bit2_a,&&
   bit3_b,&& bit3_c,&& bit3_d,&& bit3_e,&& bit3_h,&& bit3_l,&& bit3_mhl,&& bit3_a,&&
   bit4_b,&& bit4_c,&& bit4_d,&& bit4_e,&& bit4_h,&& bit4_l,&& bit4_mhl,&& bit4_a,&&
   bit5_b,&& bit5_c,&& bit5_d,&& bit5_e,&& bit5_h,&& bit5_l,&& bit5_mhl,&& bit5_a,&&
   bit6_b,&& bit6_c,&& bit6_d,&& bit6_e,&& bit6_h,&& bit6_l,&& bit6_mhl,&& bit6_a,&&
   bit7_b,&& bit7_c,&& bit7_d,&& bit7_e,&& bit7_h,&& bit7_l,&& bit7_mhl,&& bit7_a,&&
   res0_b,&& res0_c,&& res0_d,&& res0_e,&& res0_h,&& res0_l,&& res0_mhl,&& res0_a,&&
   res1_b,&& res1_c,&& res1_d,&& res1_e,&& res1_h,&& res1_l,&& res1_mhl,&& res1_a,&&
   res2_b,&& res2_c,&& res2_d,&& res2_e,&& res2_h,&& res2_l,&& res2_mhl,&& res2_a,&&
   res3_b,&& res3_c,&& res3_d,&& res3_e,&& res3_h,&& res3_l,&& res3_mhl,&& res3_a,&&
   res4_b,&& res4_c,&& res4_d,&& res4_e,&& res4_h,&& res4_l,&& res4_mhl,&& res4_a,&&
   res5_b,&& res5_c,&& res5_d,&& res5_e,&& res5_h,&& res5_l,&& res5_mhl,&& res5_a,&&
   res6_b,&& res6_c,&& res6_d,&& res6_e,&& res6_h,&& res6_l,&& res6_mhl,&& res6_a,&&
   res7_b,&& res7_c,&& res7_d,&& res7_e,&& res7_h,&& res7_l,&& res7_mhl,&& res7_a,&&
   set0_b,&& set0_c,&& set0_d,&& set0_e,&& set0_h,&& set0_l,&& set0_mhl,&& set0_a,&&
   set1_b,&& set1_c,&& set1_d,&& set1_e,&& set1_h,&& set1_l,&& set1_mhl,&& set1_a,&&
   set2_b,&& set2_c,&& set2_d,&& set2_e,&& set2_h,&& set2_l,&& set2_mhl,&& set2_a,&&
   set3_b,&& set3_c,&& set3_d,&& set3_e,&& set3_h,&& set3_l,&& set3_mhl,&& set3_a,&&
   set4_b,&& set4_c,&& set4_d,&& set4_e,&& set4_h,&& set4_l,&& set4_mhl,&& set4_a,&&
   set5_b,&& set5_c,&& set5_d,&& set5_e,&& set5_h,&& set5_l,&& set5_mhl,&& set5_a,&&
   set6_b,&& set6_c,&& set6_d,&& set6_e,&& set6_h,&& set6_l,&& set6_mhl,&& set6_a,&&
   set7_b,&& set7_c,&& set7_d,&& set7_e,&& set7_h,&& set7_l,&& set7_mhl,&& set7_a
   };

   byte bOpCode;

   bOpCode = read_mem(_PC++);
   iCycleCount += cc_cb[bOpCode];
   _R++;
	 goto *a_jump_table[bOpCode];
   //switch(bOpCode)
   //{
      bit0_a:      BIT(0, _A); return;
      bit0_b:      BIT(0, _B); return;
      bit0_c:      BIT(0, _C); return;
      bit0_d:      BIT(0, _D); return;
      bit0_e:      BIT(0, _E); return;
      bit0_h:      BIT(0, _H); return;
      bit0_l:      BIT(0, _L); return;
      bit0_mhl:    BIT(0, read_mem(_HL)); return;
      bit1_a:      BIT(1, _A); return;
      bit1_b:      BIT(1, _B); return;
      bit1_c:      BIT(1, _C); return;
      bit1_d:      BIT(1, _D); return;
      bit1_e:      BIT(1, _E); return;
      bit1_h:      BIT(1, _H); return;
      bit1_l:      BIT(1, _L); return;
      bit1_mhl:    BIT(1, read_mem(_HL)); return;
      bit2_a:      BIT(2, _A); return;
      bit2_b:      BIT(2, _B); return;
      bit2_c:      BIT(2, _C); return;
      bit2_d:      BIT(2, _D); return;
      bit2_e:      BIT(2, _E); return;
      bit2_h:      BIT(2, _H); return;
      bit2_l:      BIT(2, _L); return;
      bit2_mhl:    BIT(2, read_mem(_HL)); return;
      bit3_a:      BIT(3, _A); return;
      bit3_b:      BIT(3, _B); return;
      bit3_c:      BIT(3, _C); return;
      bit3_d:      BIT(3, _D); return;
      bit3_e:      BIT(3, _E); return;
      bit3_h:      BIT(3, _H); return;
      bit3_l:      BIT(3, _L); return;
      bit3_mhl:    BIT(3, read_mem(_HL)); return;
      bit4_a:      BIT(4, _A); return;
      bit4_b:      BIT(4, _B); return;
      bit4_c:      BIT(4, _C); return;
      bit4_d:      BIT(4, _D); return;
      bit4_e:      BIT(4, _E); return;
      bit4_h:      BIT(4, _H); return;
      bit4_l:      BIT(4, _L); return;
      bit4_mhl:    BIT(4, read_mem(_HL)); return;
      bit5_a:      BIT(5, _A); return;
      bit5_b:      BIT(5, _B); return;
      bit5_c:      BIT(5, _C); return;
      bit5_d:      BIT(5, _D); return;
      bit5_e:      BIT(5, _E); return;
      bit5_h:      BIT(5, _H); return;
      bit5_l:      BIT(5, _L); return;
      bit5_mhl:    BIT(5, read_mem(_HL)); return;
      bit6_a:      BIT(6, _A); return;
      bit6_b:      BIT(6, _B); return;
      bit6_c:      BIT(6, _C); return;
      bit6_d:      BIT(6, _D); return;
      bit6_e:      BIT(6, _E); return;
      bit6_h:      BIT(6, _H); return;
      bit6_l:      BIT(6, _L); return;
      bit6_mhl:    BIT(6, read_mem(_HL)); return;
      bit7_a:      BIT(7, _A); return;
      bit7_b:      BIT(7, _B); return;
      bit7_c:      BIT(7, _C); return;
      bit7_d:      BIT(7, _D); return;
      bit7_e:      BIT(7, _E); return;
      bit7_h:      BIT(7, _H); return;
      bit7_l:      BIT(7, _L); return;
      bit7_mhl:    BIT(7, read_mem(_HL)); return;
      res0_a:      _A = RES(0, _A); return;
      res0_b:      _B = RES(0, _B); return;
      res0_c:      _C = RES(0, _C); return;
      res0_d:      _D = RES(0, _D); return;
      res0_e:      _E = RES(0, _E); return;
      res0_h:      _H = RES(0, _H); return;
      res0_l:      _L = RES(0, _L); return;
      res0_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(0, b)); } return;
      res1_a:      _A = RES(1, _A); return;
      res1_b:      _B = RES(1, _B); return;
      res1_c:      _C = RES(1, _C); return;
      res1_d:      _D = RES(1, _D); return;
      res1_e:      _E = RES(1, _E); return;
      res1_h:      _H = RES(1, _H); return;
      res1_l:      _L = RES(1, _L); return;
      res1_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(1, b)); } return;
      res2_a:      _A = RES(2, _A); return;
      res2_b:      _B = RES(2, _B); return;
      res2_c:      _C = RES(2, _C); return;
      res2_d:      _D = RES(2, _D); return;
      res2_e:      _E = RES(2, _E); return;
      res2_h:      _H = RES(2, _H); return;
      res2_l:      _L = RES(2, _L); return;
      res2_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(2, b)); } return;
      res3_a:      _A = RES(3, _A); return;
      res3_b:      _B = RES(3, _B); return;
      res3_c:      _C = RES(3, _C); return;
      res3_d:      _D = RES(3, _D); return;
      res3_e:      _E = RES(3, _E); return;
      res3_h:      _H = RES(3, _H); return;
      res3_l:      _L = RES(3, _L); return;
      res3_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(3, b)); } return;
      res4_a:      _A = RES(4, _A); return;
      res4_b:      _B = RES(4, _B); return;
      res4_c:      _C = RES(4, _C); return;
      res4_d:      _D = RES(4, _D); return;
      res4_e:      _E = RES(4, _E); return;
      res4_h:      _H = RES(4, _H); return;
      res4_l:      _L = RES(4, _L); return;
      res4_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(4, b)); } return;
      res5_a:      _A = RES(5, _A); return;
      res5_b:      _B = RES(5, _B); return;
      res5_c:      _C = RES(5, _C); return;
      res5_d:      _D = RES(5, _D); return;
      res5_e:      _E = RES(5, _E); return;
      res5_h:      _H = RES(5, _H); return;
      res5_l:      _L = RES(5, _L); return;
      res5_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(5, b)); } return;
      res6_a:      _A = RES(6, _A); return;
      res6_b:      _B = RES(6, _B); return;
      res6_c:      _C = RES(6, _C); return;
      res6_d:      _D = RES(6, _D); return;
      res6_e:      _E = RES(6, _E); return;
      res6_h:      _H = RES(6, _H); return;
      res6_l:      _L = RES(6, _L); return;
      res6_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(6, b)); } return;
      res7_a:      _A = RES(7, _A); return;
      res7_b:      _B = RES(7, _B); return;
      res7_c:      _C = RES(7, _C); return;
      res7_d:      _D = RES(7, _D); return;
      res7_e:      _E = RES(7, _E); return;
      res7_h:      _H = RES(7, _H); return;
      res7_l:      _L = RES(7, _L); return;
      res7_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(7, b)); } return;
      rlc_a:       _A = RLC(_A); return;
      rlc_b:       _B = RLC(_B); return;
      rlc_c:       _C = RLC(_C); return;
      rlc_d:       _D = RLC(_D); return;
      rlc_e:       _E = RLC(_E); return;
      rlc_h:       _H = RLC(_H); return;
      rlc_l:       _L = RLC(_L); return;
      rlc_mhl:     { byte b = read_mem(_HL); write_mem(_HL, RLC(b)); } return;
      rl_a:        _A = RL(_A); return;
      rl_b:        _B = RL(_B); return;
      rl_c:        _C = RL(_C); return;
      rl_d:        _D = RL(_D); return;
      rl_e:        _E = RL(_E); return;
      rl_h:        _H = RL(_H); return;
      rl_l:        _L = RL(_L); return;
      rl_mhl:      { byte b = read_mem(_HL); write_mem(_HL, RL(b)); } return;
      rrc_a:       _A = RRC(_A); return;
      rrc_b:       _B = RRC(_B); return;
      rrc_c:       _C = RRC(_C); return;
      rrc_d:       _D = RRC(_D); return;
      rrc_e:       _E = RRC(_E); return;
      rrc_h:       _H = RRC(_H); return;
      rrc_l:       _L = RRC(_L); return;
      rrc_mhl:     { byte b = read_mem(_HL); write_mem(_HL, RRC(b)); } return;
      rr_a:        _A = RR(_A); return;
      rr_b:        _B = RR(_B); return;
      rr_c:        _C = RR(_C); return;
      rr_d:        _D = RR(_D); return;
      rr_e:        _E = RR(_E); return;
      rr_h:        _H = RR(_H); return;
      rr_l:        _L = RR(_L); return;
      rr_mhl:      { byte b = read_mem(_HL); write_mem(_HL, RR(b)); } return;
      set0_a:      _A = SET(0, _A); return;
      set0_b:      _B = SET(0, _B); return;
      set0_c:      _C = SET(0, _C); return;
      set0_d:      _D = SET(0, _D); return;
      set0_e:      _E = SET(0, _E); return;
      set0_h:      _H = SET(0, _H); return;
      set0_l:      _L = SET(0, _L); return;
      set0_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(0, b)); } return;
      set1_a:      _A = SET(1, _A); return;
      set1_b:      _B = SET(1, _B); return;
      set1_c:      _C = SET(1, _C); return;
      set1_d:      _D = SET(1, _D); return;
      set1_e:      _E = SET(1, _E); return;
      set1_h:      _H = SET(1, _H); return;
      set1_l:      _L = SET(1, _L); return;
      set1_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(1, b)); } return;
      set2_a:      _A = SET(2, _A); return;
      set2_b:      _B = SET(2, _B); return;
      set2_c:      _C = SET(2, _C); return;
      set2_d:      _D = SET(2, _D); return;
      set2_e:      _E = SET(2, _E); return;
      set2_h:      _H = SET(2, _H); return;
      set2_l:      _L = SET(2, _L); return;
      set2_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(2, b)); } return;
      set3_a:      _A = SET(3, _A); return;
      set3_b:      _B = SET(3, _B); return;
      set3_c:      _C = SET(3, _C); return;
      set3_d:      _D = SET(3, _D); return;
      set3_e:      _E = SET(3, _E); return;
      set3_h:      _H = SET(3, _H); return;
      set3_l:      _L = SET(3, _L); return;
      set3_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(3, b)); } return;
      set4_a:      _A = SET(4, _A); return;
      set4_b:      _B = SET(4, _B); return;
      set4_c:      _C = SET(4, _C); return;
      set4_d:      _D = SET(4, _D); return;
      set4_e:      _E = SET(4, _E); return;
      set4_h:      _H = SET(4, _H); return;
      set4_l:      _L = SET(4, _L); return;
      set4_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(4, b)); } return;
      set5_a:      _A = SET(5, _A); return;
      set5_b:      _B = SET(5, _B); return;
      set5_c:      _C = SET(5, _C); return;
      set5_d:      _D = SET(5, _D); return;
      set5_e:      _E = SET(5, _E); return;
      set5_h:      _H = SET(5, _H); return;
      set5_l:      _L = SET(5, _L); return;
      set5_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(5, b)); } return;
      set6_a:      _A = SET(6, _A); return;
      set6_b:      _B = SET(6, _B); return;
      set6_c:      _C = SET(6, _C); return;
      set6_d:      _D = SET(6, _D); return;
      set6_e:      _E = SET(6, _E); return;
      set6_h:      _H = SET(6, _H); return;
      set6_l:      _L = SET(6, _L); return;
      set6_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(6, b)); } return;
      set7_a:      _A = SET(7, _A); return;
      set7_b:      _B = SET(7, _B); return;
      set7_c:      _C = SET(7, _C); return;
      set7_d:      _D = SET(7, _D); return;
      set7_e:      _E = SET(7, _E); return;
      set7_h:      _H = SET(7, _H); return;
      set7_l:      _L = SET(7, _L); return;
      set7_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(7, b)); } return;
      sla_a:       _A = SLA(_A); return;
      sla_b:       _B = SLA(_B); return;
      sla_c:       _C = SLA(_C); return;
      sla_d:       _D = SLA(_D); return;
      sla_e:       _E = SLA(_E); return;
      sla_h:       _H = SLA(_H); return;
      sla_l:       _L = SLA(_L); return;
      sla_mhl:     { byte b = read_mem(_HL); write_mem(_HL, SLA(b)); } return;
      sll_a:       _A = SLL(_A); return;
      sll_b:       _B = SLL(_B); return;
      sll_c:       _C = SLL(_C); return;
      sll_d:       _D = SLL(_D); return;
      sll_e:       _E = SLL(_E); return;
      sll_h:       _H = SLL(_H); return;
      sll_l:       _L = SLL(_L); return;
      sll_mhl:     { byte b = read_mem(_HL); write_mem(_HL, SLL(b)); } return;
      sra_a:       _A = SRA(_A); return;
      sra_b:       _B = SRA(_B); return;
      sra_c:       _C = SRA(_C); return;
      sra_d:       _D = SRA(_D); return;
      sra_e:       _E = SRA(_E); return;
      sra_h:       _H = SRA(_H); return;
      sra_l:       _L = SRA(_L); return;
      sra_mhl:     { byte b = read_mem(_HL); write_mem(_HL, SRA(b)); } return;
      srl_a:       _A = SRL(_A); return;
      srl_b:       _B = SRL(_B); return;
      srl_c:       _C = SRL(_C); return;
      srl_d:       _D = SRL(_D); return;
      srl_e:       _E = SRL(_E); return;
      srl_h:       _H = SRL(_H); return;
      srl_l:       _L = SRL(_L); return;
      srl_mhl:     { byte b = read_mem(_HL); write_mem(_HL, SRL(b)); } return;
   //}
}
# else
void z80_pfx_cb(void)
{
   byte bOpCode;

   bOpCode = read_mem(_PC++);
   iCycleCount += cc_cb[bOpCode];
   _R++;
   switch(bOpCode)
   {
      case bit0_a:      BIT(0, _A); break;
      case bit0_b:      BIT(0, _B); break;
      case bit0_c:      BIT(0, _C); break;
      case bit0_d:      BIT(0, _D); break;
      case bit0_e:      BIT(0, _E); break;
      case bit0_h:      BIT(0, _H); break;
      case bit0_l:      BIT(0, _L); break;
      case bit0_mhl:    BIT(0, read_mem(_HL)); break;
      case bit1_a:      BIT(1, _A); break;
      case bit1_b:      BIT(1, _B); break;
      case bit1_c:      BIT(1, _C); break;
      case bit1_d:      BIT(1, _D); break;
      case bit1_e:      BIT(1, _E); break;
      case bit1_h:      BIT(1, _H); break;
      case bit1_l:      BIT(1, _L); break;
      case bit1_mhl:    BIT(1, read_mem(_HL)); break;
      case bit2_a:      BIT(2, _A); break;
      case bit2_b:      BIT(2, _B); break;
      case bit2_c:      BIT(2, _C); break;
      case bit2_d:      BIT(2, _D); break;
      case bit2_e:      BIT(2, _E); break;
      case bit2_h:      BIT(2, _H); break;
      case bit2_l:      BIT(2, _L); break;
      case bit2_mhl:    BIT(2, read_mem(_HL)); break;
      case bit3_a:      BIT(3, _A); break;
      case bit3_b:      BIT(3, _B); break;
      case bit3_c:      BIT(3, _C); break;
      case bit3_d:      BIT(3, _D); break;
      case bit3_e:      BIT(3, _E); break;
      case bit3_h:      BIT(3, _H); break;
      case bit3_l:      BIT(3, _L); break;
      case bit3_mhl:    BIT(3, read_mem(_HL)); break;
      case bit4_a:      BIT(4, _A); break;
      case bit4_b:      BIT(4, _B); break;
      case bit4_c:      BIT(4, _C); break;
      case bit4_d:      BIT(4, _D); break;
      case bit4_e:      BIT(4, _E); break;
      case bit4_h:      BIT(4, _H); break;
      case bit4_l:      BIT(4, _L); break;
      case bit4_mhl:    BIT(4, read_mem(_HL)); break;
      case bit5_a:      BIT(5, _A); break;
      case bit5_b:      BIT(5, _B); break;
      case bit5_c:      BIT(5, _C); break;
      case bit5_d:      BIT(5, _D); break;
      case bit5_e:      BIT(5, _E); break;
      case bit5_h:      BIT(5, _H); break;
      case bit5_l:      BIT(5, _L); break;
      case bit5_mhl:    BIT(5, read_mem(_HL)); break;
      case bit6_a:      BIT(6, _A); break;
      case bit6_b:      BIT(6, _B); break;
      case bit6_c:      BIT(6, _C); break;
      case bit6_d:      BIT(6, _D); break;
      case bit6_e:      BIT(6, _E); break;
      case bit6_h:      BIT(6, _H); break;
      case bit6_l:      BIT(6, _L); break;
      case bit6_mhl:    BIT(6, read_mem(_HL)); break;
      case bit7_a:      BIT(7, _A); break;
      case bit7_b:      BIT(7, _B); break;
      case bit7_c:      BIT(7, _C); break;
      case bit7_d:      BIT(7, _D); break;
      case bit7_e:      BIT(7, _E); break;
      case bit7_h:      BIT(7, _H); break;
      case bit7_l:      BIT(7, _L); break;
      case bit7_mhl:    BIT(7, read_mem(_HL)); break;
      case res0_a:      _A = RES(0, _A); break;
      case res0_b:      _B = RES(0, _B); break;
      case res0_c:      _C = RES(0, _C); break;
      case res0_d:      _D = RES(0, _D); break;
      case res0_e:      _E = RES(0, _E); break;
      case res0_h:      _H = RES(0, _H); break;
      case res0_l:      _L = RES(0, _L); break;
      case res0_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(0, b)); } break;
      case res1_a:      _A = RES(1, _A); break;
      case res1_b:      _B = RES(1, _B); break;
      case res1_c:      _C = RES(1, _C); break;
      case res1_d:      _D = RES(1, _D); break;
      case res1_e:      _E = RES(1, _E); break;
      case res1_h:      _H = RES(1, _H); break;
      case res1_l:      _L = RES(1, _L); break;
      case res1_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(1, b)); } break;
      case res2_a:      _A = RES(2, _A); break;
      case res2_b:      _B = RES(2, _B); break;
      case res2_c:      _C = RES(2, _C); break;
      case res2_d:      _D = RES(2, _D); break;
      case res2_e:      _E = RES(2, _E); break;
      case res2_h:      _H = RES(2, _H); break;
      case res2_l:      _L = RES(2, _L); break;
      case res2_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(2, b)); } break;
      case res3_a:      _A = RES(3, _A); break;
      case res3_b:      _B = RES(3, _B); break;
      case res3_c:      _C = RES(3, _C); break;
      case res3_d:      _D = RES(3, _D); break;
      case res3_e:      _E = RES(3, _E); break;
      case res3_h:      _H = RES(3, _H); break;
      case res3_l:      _L = RES(3, _L); break;
      case res3_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(3, b)); } break;
      case res4_a:      _A = RES(4, _A); break;
      case res4_b:      _B = RES(4, _B); break;
      case res4_c:      _C = RES(4, _C); break;
      case res4_d:      _D = RES(4, _D); break;
      case res4_e:      _E = RES(4, _E); break;
      case res4_h:      _H = RES(4, _H); break;
      case res4_l:      _L = RES(4, _L); break;
      case res4_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(4, b)); } break;
      case res5_a:      _A = RES(5, _A); break;
      case res5_b:      _B = RES(5, _B); break;
      case res5_c:      _C = RES(5, _C); break;
      case res5_d:      _D = RES(5, _D); break;
      case res5_e:      _E = RES(5, _E); break;
      case res5_h:      _H = RES(5, _H); break;
      case res5_l:      _L = RES(5, _L); break;
      case res5_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(5, b)); } break;
      case res6_a:      _A = RES(6, _A); break;
      case res6_b:      _B = RES(6, _B); break;
      case res6_c:      _C = RES(6, _C); break;
      case res6_d:      _D = RES(6, _D); break;
      case res6_e:      _E = RES(6, _E); break;
      case res6_h:      _H = RES(6, _H); break;
      case res6_l:      _L = RES(6, _L); break;
      case res6_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(6, b)); } break;
      case res7_a:      _A = RES(7, _A); break;
      case res7_b:      _B = RES(7, _B); break;
      case res7_c:      _C = RES(7, _C); break;
      case res7_d:      _D = RES(7, _D); break;
      case res7_e:      _E = RES(7, _E); break;
      case res7_h:      _H = RES(7, _H); break;
      case res7_l:      _L = RES(7, _L); break;
      case res7_mhl:    { byte b = read_mem(_HL); write_mem(_HL, RES(7, b)); } break;
      case rlc_a:       _A = RLC(_A); break;
      case rlc_b:       _B = RLC(_B); break;
      case rlc_c:       _C = RLC(_C); break;
      case rlc_d:       _D = RLC(_D); break;
      case rlc_e:       _E = RLC(_E); break;
      case rlc_h:       _H = RLC(_H); break;
      case rlc_l:       _L = RLC(_L); break;
      case rlc_mhl:     { byte b = read_mem(_HL); write_mem(_HL, RLC(b)); } break;
      case rl_a:        _A = RL(_A); break;
      case rl_b:        _B = RL(_B); break;
      case rl_c:        _C = RL(_C); break;
      case rl_d:        _D = RL(_D); break;
      case rl_e:        _E = RL(_E); break;
      case rl_h:        _H = RL(_H); break;
      case rl_l:        _L = RL(_L); break;
      case rl_mhl:      { byte b = read_mem(_HL); write_mem(_HL, RL(b)); } break;
      case rrc_a:       _A = RRC(_A); break;
      case rrc_b:       _B = RRC(_B); break;
      case rrc_c:       _C = RRC(_C); break;
      case rrc_d:       _D = RRC(_D); break;
      case rrc_e:       _E = RRC(_E); break;
      case rrc_h:       _H = RRC(_H); break;
      case rrc_l:       _L = RRC(_L); break;
      case rrc_mhl:     { byte b = read_mem(_HL); write_mem(_HL, RRC(b)); } break;
      case rr_a:        _A = RR(_A); break;
      case rr_b:        _B = RR(_B); break;
      case rr_c:        _C = RR(_C); break;
      case rr_d:        _D = RR(_D); break;
      case rr_e:        _E = RR(_E); break;
      case rr_h:        _H = RR(_H); break;
      case rr_l:        _L = RR(_L); break;
      case rr_mhl:      { byte b = read_mem(_HL); write_mem(_HL, RR(b)); } break;
      case set0_a:      _A = SET(0, _A); break;
      case set0_b:      _B = SET(0, _B); break;
      case set0_c:      _C = SET(0, _C); break;
      case set0_d:      _D = SET(0, _D); break;
      case set0_e:      _E = SET(0, _E); break;
      case set0_h:      _H = SET(0, _H); break;
      case set0_l:      _L = SET(0, _L); break;
      case set0_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(0, b)); } break;
      case set1_a:      _A = SET(1, _A); break;
      case set1_b:      _B = SET(1, _B); break;
      case set1_c:      _C = SET(1, _C); break;
      case set1_d:      _D = SET(1, _D); break;
      case set1_e:      _E = SET(1, _E); break;
      case set1_h:      _H = SET(1, _H); break;
      case set1_l:      _L = SET(1, _L); break;
      case set1_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(1, b)); } break;
      case set2_a:      _A = SET(2, _A); break;
      case set2_b:      _B = SET(2, _B); break;
      case set2_c:      _C = SET(2, _C); break;
      case set2_d:      _D = SET(2, _D); break;
      case set2_e:      _E = SET(2, _E); break;
      case set2_h:      _H = SET(2, _H); break;
      case set2_l:      _L = SET(2, _L); break;
      case set2_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(2, b)); } break;
      case set3_a:      _A = SET(3, _A); break;
      case set3_b:      _B = SET(3, _B); break;
      case set3_c:      _C = SET(3, _C); break;
      case set3_d:      _D = SET(3, _D); break;
      case set3_e:      _E = SET(3, _E); break;
      case set3_h:      _H = SET(3, _H); break;
      case set3_l:      _L = SET(3, _L); break;
      case set3_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(3, b)); } break;
      case set4_a:      _A = SET(4, _A); break;
      case set4_b:      _B = SET(4, _B); break;
      case set4_c:      _C = SET(4, _C); break;
      case set4_d:      _D = SET(4, _D); break;
      case set4_e:      _E = SET(4, _E); break;
      case set4_h:      _H = SET(4, _H); break;
      case set4_l:      _L = SET(4, _L); break;
      case set4_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(4, b)); } break;
      case set5_a:      _A = SET(5, _A); break;
      case set5_b:      _B = SET(5, _B); break;
      case set5_c:      _C = SET(5, _C); break;
      case set5_d:      _D = SET(5, _D); break;
      case set5_e:      _E = SET(5, _E); break;
      case set5_h:      _H = SET(5, _H); break;
      case set5_l:      _L = SET(5, _L); break;
      case set5_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(5, b)); } break;
      case set6_a:      _A = SET(6, _A); break;
      case set6_b:      _B = SET(6, _B); break;
      case set6_c:      _C = SET(6, _C); break;
      case set6_d:      _D = SET(6, _D); break;
      case set6_e:      _E = SET(6, _E); break;
      case set6_h:      _H = SET(6, _H); break;
      case set6_l:      _L = SET(6, _L); break;
      case set6_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(6, b)); } break;
      case set7_a:      _A = SET(7, _A); break;
      case set7_b:      _B = SET(7, _B); break;
      case set7_c:      _C = SET(7, _C); break;
      case set7_d:      _D = SET(7, _D); break;
      case set7_e:      _E = SET(7, _E); break;
      case set7_h:      _H = SET(7, _H); break;
      case set7_l:      _L = SET(7, _L); break;
      case set7_mhl:    { byte b = read_mem(_HL); write_mem(_HL, SET(7, b)); } break;
      case sla_a:       _A = SLA(_A); break;
      case sla_b:       _B = SLA(_B); break;
      case sla_c:       _C = SLA(_C); break;
      case sla_d:       _D = SLA(_D); break;
      case sla_e:       _E = SLA(_E); break;
      case sla_h:       _H = SLA(_H); break;
      case sla_l:       _L = SLA(_L); break;
      case sla_mhl:     { byte b = read_mem(_HL); write_mem(_HL, SLA(b)); } break;
      case sll_a:       _A = SLL(_A); break;
      case sll_b:       _B = SLL(_B); break;
      case sll_c:       _C = SLL(_C); break;
      case sll_d:       _D = SLL(_D); break;
      case sll_e:       _E = SLL(_E); break;
      case sll_h:       _H = SLL(_H); break;
      case sll_l:       _L = SLL(_L); break;
      case sll_mhl:     { byte b = read_mem(_HL); write_mem(_HL, SLL(b)); } break;
      case sra_a:       _A = SRA(_A); break;
      case sra_b:       _B = SRA(_B); break;
      case sra_c:       _C = SRA(_C); break;
      case sra_d:       _D = SRA(_D); break;
      case sra_e:       _E = SRA(_E); break;
      case sra_h:       _H = SRA(_H); break;
      case sra_l:       _L = SRA(_L); break;
      case sra_mhl:     { byte b = read_mem(_HL); write_mem(_HL, SRA(b)); } break;
      case srl_a:       _A = SRL(_A); break;
      case srl_b:       _B = SRL(_B); break;
      case srl_c:       _C = SRL(_C); break;
      case srl_d:       _D = SRL(_D); break;
      case srl_e:       _E = SRL(_E); break;
      case srl_h:       _H = SRL(_H); break;
      case srl_l:       _L = SRL(_L); break;
      case srl_mhl:     { byte b = read_mem(_HL); write_mem(_HL, SRL(b)); } break;
   }
}
# endif

# ifdef Z80_JMP_TBL
void z80_pfx_dd(void)
{
    __label__ 
   nop, ld_bc_word, ld_mbc_a, inc_bc, inc_b, dec_b, ld_b_byte, rlca,
   ex_af_af, add_hl_bc, ld_a_mbc, dec_bc, inc_c, dec_c, ld_c_byte, rrca,
   djnz, ld_de_word, ld_mde_a, inc_de, inc_d, dec_d, ld_d_byte, rla,
   jr, add_hl_de, ld_a_mde, dec_de, inc_e, dec_e, ld_e_byte, rra,
   jr_nz, ld_hl_word, ld_mword_hl, inc_hl, inc_h, dec_h, ld_h_byte, daa,
   jr_z, add_hl_hl, ld_hl_mword, dec_hl, inc_l, dec_l, ld_l_byte, cpl,
   jr_nc, ld_sp_word, ld_mword_a, inc_sp, inc_mhl, dec_mhl, ld_mhl_byte, scf,
   jr_c, add_hl_sp, ld_a_mword, dec_sp, inc_a, dec_a, ld_a_byte, ccf,
   ld_b_b, ld_b_c, ld_b_d, ld_b_e, ld_b_h, ld_b_l, ld_b_mhl, ld_b_a,
   ld_c_b, ld_c_c, ld_c_d, ld_c_e, ld_c_h, ld_c_l, ld_c_mhl, ld_c_a,
   ld_d_b, ld_d_c, ld_d_d, ld_d_e, ld_d_h, ld_d_l, ld_d_mhl, ld_d_a,
   ld_e_b, ld_e_c, ld_e_d, ld_e_e, ld_e_h, ld_e_l, ld_e_mhl, ld_e_a,
   ld_h_b, ld_h_c, ld_h_d, ld_h_e, ld_h_h, ld_h_l, ld_h_mhl, ld_h_a,
   ld_l_b, ld_l_c, ld_l_d, ld_l_e, ld_l_h, ld_l_l, ld_l_mhl, ld_l_a,
   ld_mhl_b, ld_mhl_c, ld_mhl_d, ld_mhl_e, ld_mhl_h, ld_mhl_l, halt, ld_mhl_a,
   ld_a_b, ld_a_c, ld_a_d, ld_a_e, ld_a_h, ld_a_l, ld_a_mhl, ld_a_a,
   add_b, add_c, add_d, add_e, add_h, add_l, add_mhl, add_a,
   adc_b, adc_c, adc_d, adc_e, adc_h, adc_l, adc_mhl, adc_a,
   sub_b, sub_c, sub_d, sub_e, sub_h, sub_l, sub_mhl, sub_a,
   sbc_b, sbc_c, sbc_d, sbc_e, sbc_h, sbc_l, sbc_mhl, sbc_a,
   and_b, and_c, and_d, and_e, and_h, and_l, and_mhl, and_a,
   xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, xor_mhl, xor_a,
   or_b, or_c, or_d, or_e, or_h, or_l, or_mhl, or_a,
   cp_b, cp_c, cp_d, cp_e, cp_h, cp_l, cp_mhl, cp_a,
   ret_nz, pop_bc, jp_nz, jp, call_nz, push_bc, add_byte, rst00,
   ret_z, ret, jp_z, pfx_cb, call_z, call, adc_byte, rst08,
   ret_nc, pop_de, jp_nc, outa, call_nc, push_de, sub_byte, rst10,
   ret_c, exx, jp_c, ina, call_c, pfx_dd, sbc_byte, rst18,
   ret_po, pop_hl, jp_po, ex_msp_hl, call_po, push_hl, and_byte, rst20,
   ret_pe, ld_pc_hl, jp_pe, ex_de_hl, call_pe, pfx_ed, xor_byte, rst28,
   ret_p, pop_af, jp_p, di, call_p, push_af, or_byte, rst30,
   ret_m, ld_sp_hl, jp_m, ei, call_m, pfx_fd, cp_byte, rst38;

    static const void* const a_jump_table[256] = 
	  {
   && nop,&& ld_bc_word,&& ld_mbc_a,&& inc_bc,&& inc_b,&& dec_b,&& ld_b_byte,&& rlca,&&
   ex_af_af,&& add_hl_bc,&& ld_a_mbc,&& dec_bc,&& inc_c,&& dec_c,&& ld_c_byte,&& rrca,&&
   djnz,&& ld_de_word,&& ld_mde_a,&& inc_de,&& inc_d,&& dec_d,&& ld_d_byte,&& rla,&&
   jr,&& add_hl_de,&& ld_a_mde,&& dec_de,&& inc_e,&& dec_e,&& ld_e_byte,&& rra,&&
   jr_nz,&& ld_hl_word,&& ld_mword_hl,&& inc_hl,&& inc_h,&& dec_h,&& ld_h_byte,&& daa,&&
   jr_z,&& add_hl_hl,&& ld_hl_mword,&& dec_hl,&& inc_l,&& dec_l,&& ld_l_byte,&& cpl,&&
   jr_nc,&& ld_sp_word,&& ld_mword_a,&& inc_sp,&& inc_mhl,&& dec_mhl,&& ld_mhl_byte,&& scf,&&
   jr_c,&& add_hl_sp,&& ld_a_mword,&& dec_sp,&& inc_a,&& dec_a,&& ld_a_byte,&& ccf,&&
   ld_b_b,&& ld_b_c,&& ld_b_d,&& ld_b_e,&& ld_b_h,&& ld_b_l,&& ld_b_mhl,&& ld_b_a,&&
   ld_c_b,&& ld_c_c,&& ld_c_d,&& ld_c_e,&& ld_c_h,&& ld_c_l,&& ld_c_mhl,&& ld_c_a,&&
   ld_d_b,&& ld_d_c,&& ld_d_d,&& ld_d_e,&& ld_d_h,&& ld_d_l,&& ld_d_mhl,&& ld_d_a,&&
   ld_e_b,&& ld_e_c,&& ld_e_d,&& ld_e_e,&& ld_e_h,&& ld_e_l,&& ld_e_mhl,&& ld_e_a,&&
   ld_h_b,&& ld_h_c,&& ld_h_d,&& ld_h_e,&& ld_h_h,&& ld_h_l,&& ld_h_mhl,&& ld_h_a,&&
   ld_l_b,&& ld_l_c,&& ld_l_d,&& ld_l_e,&& ld_l_h,&& ld_l_l,&& ld_l_mhl,&& ld_l_a,&&
   ld_mhl_b,&& ld_mhl_c,&& ld_mhl_d,&& ld_mhl_e,&& ld_mhl_h,&& ld_mhl_l,&& halt,&& ld_mhl_a,&&
   ld_a_b,&& ld_a_c,&& ld_a_d,&& ld_a_e,&& ld_a_h,&& ld_a_l,&& ld_a_mhl,&& ld_a_a,&&
   add_b,&& add_c,&& add_d,&& add_e,&& add_h,&& add_l,&& add_mhl,&& add_a,&&
   adc_b,&& adc_c,&& adc_d,&& adc_e,&& adc_h,&& adc_l,&& adc_mhl,&& adc_a,&&
   sub_b,&& sub_c,&& sub_d,&& sub_e,&& sub_h,&& sub_l,&& sub_mhl,&& sub_a,&&
   sbc_b,&& sbc_c,&& sbc_d,&& sbc_e,&& sbc_h,&& sbc_l,&& sbc_mhl,&& sbc_a,&&
   and_b,&& and_c,&& and_d,&& and_e,&& and_h,&& and_l,&& and_mhl,&& and_a,&&
   xor_b,&& xor_c,&& xor_d,&& xor_e,&& xor_h,&& xor_l,&& xor_mhl,&& xor_a,&&
   or_b,&& or_c,&& or_d,&& or_e,&& or_h,&& or_l,&& or_mhl,&& or_a,&&
   cp_b,&& cp_c,&& cp_d,&& cp_e,&& cp_h,&& cp_l,&& cp_mhl,&& cp_a,&&
   ret_nz,&& pop_bc,&& jp_nz,&& jp,&& call_nz,&& push_bc,&& add_byte,&& rst00,&&
   ret_z,&& ret,&& jp_z,&& pfx_cb,&& call_z,&& call,&& adc_byte,&& rst08,&&
   ret_nc,&& pop_de,&& jp_nc,&& outa,&& call_nc,&& push_de,&& sub_byte,&& rst10,&&
   ret_c,&& exx,&& jp_c,&& ina,&& call_c,&& pfx_dd,&& sbc_byte,&& rst18,&&
   ret_po,&& pop_hl,&& jp_po,&& ex_msp_hl,&& call_po,&& push_hl,&& and_byte,&& rst20,&&
   ret_pe,&& ld_pc_hl,&& jp_pe,&& ex_de_hl,&& call_pe,&& pfx_ed,&& xor_byte,&& rst28,&&
   ret_p,&& pop_af,&& jp_p,&& di,&& call_p,&& push_af,&& or_byte,&& rst30,&&
   ret_m,&& ld_sp_hl,&& jp_m,&& ei,&& call_m,&& pfx_fd,&& cp_byte,&& rst38
	  };

   byte bOpCode;

   bOpCode = read_mem(_PC++);
   iCycleCount += cc_xy[bOpCode];
   _R++;
	 goto *a_jump_table[bOpCode];
   //switch(bOpCode)
   //{
      pfx_cb:      z80_pfx_ddcb(); return;
      pfx_dd:      z80_pfx_dd(); return;
      pfx_ed:      z80_pfx_ed(); return;
      pfx_fd:      z80_pfx_fd(); return;
      adc_a:       ADC(_A); return;
      adc_b:       ADC(_B); return;
      adc_byte:    ADC(read_mem(_PC++)); return;
      adc_c:       ADC(_C); return;
      adc_d:       ADC(_D); return;
      adc_e:       ADC(_E); return;
      adc_h:       ADC(_IXh); return;
      adc_l:       ADC(_IXl); return;
      adc_mhl:     { signed char o = read_mem(_PC++); ADC(read_mem(_IX+o)); } return;
      add_a:       ADD(_A); return;
      add_b:       ADD(_B); return;
      add_byte:    ADD(read_mem(_PC++)); return;
      add_c:       ADD(_C); return;
      add_d:       ADD(_D); return;
      add_e:       ADD(_E); return;
      add_h:       ADD(_IXh); return;
      add_hl_bc:   ADD16(IX, BC); return;
      add_hl_de:   ADD16(IX, DE); return;
      add_hl_hl:   ADD16(IX, IX); return;
      add_hl_sp:   ADD16(IX, SP); return;
      add_l:       ADD(_IXl); return;
      add_mhl:     { signed char o = read_mem(_PC++); ADD(read_mem(_IX+o)); } return;
      and_a:       AND(_A); return;
      and_b:       AND(_B); return;
      and_byte:    AND(read_mem(_PC++)); return;
      and_c:       AND(_C); return;
      and_d:       AND(_D); return;
      and_e:       AND(_E); return;
      and_h:       AND(_IXh); return;
      and_l:       AND(_IXl); return;
      and_mhl:     { signed char o = read_mem(_PC++); AND(read_mem(_IX+o)); } return;
      call:        CALL; return;
      call_c:      if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_m:      if (_F & Sflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_nc:     if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_nz:     if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_p:      if (!(_F & Sflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_pe:     if (_F & Pflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_po:     if (!(_F & Pflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_z:      if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      ccf:         _F = ((_F & (Sflag | Zflag | Pflag | Cflag)) | ((_F & CF) << 4) | (_A & Xflags)) ^ CF; return;
      cpl:         _A ^= 0xff; _F = (_F & (Sflag | Zflag | Pflag | Cflag)) | Hflag | Nflag | (_A & Xflags); return;
      cp_a:        CP(_A); return;
      cp_b:        CP(_B); return;
      cp_byte:     CP(read_mem(_PC++)); return;
      cp_c:        CP(_C); return;
      cp_d:        CP(_D); return;
      cp_e:        CP(_E); return;
      cp_h:        CP(_IXh); return;
      cp_l:        CP(_IXl); return;
      cp_mhl:      { signed char o = read_mem(_PC++); CP(read_mem(_IX+o)); } return;
      daa:         DAA; return;
      dec_a:       DEC(_A); return;
      dec_b:       DEC(_B); return;
      dec_bc:      _BC--; iWSAdjust++; return;
      dec_c:       DEC(_C); return;
      dec_d:       DEC(_D); return;
      dec_de:      _DE--; iWSAdjust++; return;
      dec_e:       DEC(_E); return;
      dec_h:       DEC(_IXh); return;
      dec_hl:      _IX--; iWSAdjust++; return;
      dec_l:       DEC(_IXl); return;
      dec_mhl:     { signed char o = read_mem(_PC++); byte b = read_mem(_IX+o); DEC(b); write_mem(_IX+o, b); } return;
      dec_sp:      _SP--; iWSAdjust++; return;
      di:          _IFF1 = _IFF2 = 0; z80.EI_issued = 0; return;
      djnz:        if (--_B) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; } return;
      ei:          z80.EI_issued = 2; return;
      exx:         EXX; return;
      ex_af_af:    EX(z80.AF, z80.AFx); return;
      ex_de_hl:    EX(z80.DE, z80.HL); return;
      ex_msp_hl:   EX_SP(IX); iWSAdjust++; return;
      halt:        _HALT = 1; _PC--; return;
      ina:         { z80_wait_states iCycleCount = Ia_;} { reg_pair p; p.b.l = read_mem(_PC++); p.b.h = _A; _A = z80_IN_handler(p); } return;
      inc_a:       INC(_A); return;
      inc_b:       INC(_B); return;
      inc_bc:      _BC++; iWSAdjust++; return;
      inc_c:       INC(_C); return;
      inc_d:       INC(_D); return;
      inc_de:      _DE++; iWSAdjust++; return;
      inc_e:       INC(_E); return;
      inc_h:       INC(_IXh); return;
      inc_hl:      _IX++; iWSAdjust++; return;
      inc_l:       INC(_IXl); return;
      inc_mhl:     { signed char o = read_mem(_PC++); byte b = read_mem(_IX+o); INC(b); write_mem(_IX+o, b); } return;
      inc_sp:      _SP++; iWSAdjust++; return;
      jp:          JP; return;
      jp_c:        if (_F & Cflag) { JP } else { _PC += 2; }; return;
      jp_m:        if (_F & Sflag) { JP } else { _PC += 2; }; return;
      jp_nc:       if (!(_F & Cflag)) { JP } else { _PC += 2; }; return;
      jp_nz:       if (!(_F & Zflag)) { JP } else { _PC += 2; }; return;
      jp_p:        if (!(_F & Sflag)) { JP } else { _PC += 2; }; return;
      jp_pe:       if (_F & Pflag) { JP } else { _PC += 2; }; return;
      jp_po:       if (!(_F & Pflag)) { JP } else { _PC += 2; }; return;
      jp_z:        if (_F & Zflag) { JP } else { _PC += 2; }; return;
      jr:          JR; return;
      jr_c:        if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; return;
      jr_nc:       if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; return;
      jr_nz:       if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; return;
      jr_z:        if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; return;
      ld_a_a:      return;
      ld_a_b:      _A = _B; return;
      ld_a_byte:   _A = read_mem(_PC++); return;
      ld_a_c:      _A = _C; return;
      ld_a_d:      _A = _D; return;
      ld_a_e:      _A = _E; return;
      ld_a_h:      _A = _IXh; return;
      ld_a_l:      _A = _IXl; return;
      ld_a_mbc:    _A = read_mem(_BC); return;
      ld_a_mde:    _A = read_mem(_DE); return;
      ld_a_mhl:    { signed char o = read_mem(_PC++); _A = read_mem(_IX+o); } return;
      ld_a_mword:  { reg_pair addr; addr.b.l = read_mem(_PC++); addr.b.h = read_mem(_PC++); _A = read_mem(addr.w.l); } return;
      ld_bc_word:  z80.BC.b.l = read_mem(_PC++); z80.BC.b.h = read_mem(_PC++); return;
      ld_b_a:      _B = _A; return;
      ld_b_b:      return;
      ld_b_byte:   _B = read_mem(_PC++); return;
      ld_b_c:      _B = _C; return;
      ld_b_d:      _B = _D; return;
      ld_b_e:      _B = _E; return;
      ld_b_h:      _B = _IXh; return;
      ld_b_l:      _B = _IXl; return;
      ld_b_mhl:    { signed char o = read_mem(_PC++); _B = read_mem(_IX+o); } return;
      ld_c_a:      _C = _A; return;
      ld_c_b:      _C = _B; return;
      ld_c_byte:   _C = read_mem(_PC++); return;
      ld_c_c:      return;
      ld_c_d:      _C = _D; return;
      ld_c_e:      _C = _E; return;
      ld_c_h:      _C = _IXh; return;
      ld_c_l:      _C = _IXl; return;
      ld_c_mhl:    { signed char o = read_mem(_PC++); _C = read_mem(_IX+o); } return;
      ld_de_word:  z80.DE.b.l = read_mem(_PC++); z80.DE.b.h = read_mem(_PC++); return;
      ld_d_a:      _D = _A; return;
      ld_d_b:      _D = _B; return;
      ld_d_byte:   _D = read_mem(_PC++); return;
      ld_d_c:      _D = _C; return;
      ld_d_d:      return;
      ld_d_e:      _D = _E; return;
      ld_d_h:      _D = _IXh; return;
      ld_d_l:      _D = _IXl; return;
      ld_d_mhl:    { signed char o = read_mem(_PC++); _D = read_mem(_IX+o); } return;
      ld_e_a:      _E = _A; return;
      ld_e_b:      _E = _B; return;
      ld_e_byte:   _E = read_mem(_PC++); return;
      ld_e_c:      _E = _C; return;
      ld_e_d:      _E = _D; return;
      ld_e_e:      return;
      ld_e_h:      _E = _IXh; return;
      ld_e_l:      _E = _IXl; return;
      ld_e_mhl:    { signed char o = read_mem(_PC++); _E = read_mem(_IX+o); } return;
      ld_hl_mword: LD16_MEM(IX); return;
      ld_hl_word:  z80.IX.b.l = read_mem(_PC++); z80.IX.b.h = read_mem(_PC++); return;
      ld_h_a:      _IXh = _A; return;
      ld_h_b:      _IXh = _B; return;
      ld_h_byte:   _IXh = read_mem(_PC++); return;
      ld_h_c:      _IXh = _C; return;
      ld_h_d:      _IXh = _D; return;
      ld_h_e:      _IXh = _E; return;
      ld_h_h:      return;
      ld_h_l:      _IXh = _IXl; return;
      ld_h_mhl:    { signed char o = read_mem(_PC++); _H = read_mem(_IX+o); } return;
      ld_l_a:      _IXl = _A; return;
      ld_l_b:      _IXl = _B; return;
      ld_l_byte:   _IXl = read_mem(_PC++); return;
      ld_l_c:      _IXl = _C; return;
      ld_l_d:      _IXl = _D; return;
      ld_l_e:      _IXl = _E; return;
      ld_l_h:      _IXl = _IXh; return;
      ld_l_l:      return;
      ld_l_mhl:    { signed char o = read_mem(_PC++); _L = read_mem(_IX+o); } return;
      ld_mbc_a:    write_mem(_BC, _A); return;
      ld_mde_a:    write_mem(_DE, _A); return;
      ld_mhl_a:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _A); } return;
      ld_mhl_b:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _B); } return;
      ld_mhl_byte: { signed char o = read_mem(_PC++); byte b = read_mem(_PC++); write_mem(_IX+o, b); } return;
      ld_mhl_c:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _C); } return;
      ld_mhl_d:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _D); } return;
      ld_mhl_e:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _E); } return;
      ld_mhl_h:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _H); } return;
      ld_mhl_l:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _L); } return;
      ld_mword_a:  { reg_pair addr; addr.b.l = read_mem(_PC++); addr.b.h = read_mem(_PC++); write_mem(addr.w.l, _A); } return;
      ld_mword_hl: LDMEM_16(IX); return;
      ld_pc_hl:    _PC = _IX; return;
      ld_sp_hl:    _SP = _IX; iWSAdjust++; return;
      ld_sp_word:  z80.SP.b.l = read_mem(_PC++); z80.SP.b.h = read_mem(_PC++); return;
      nop:         return;
      or_a:        OR(_A); return;
      or_b:        OR(_B); return;
      or_byte:     OR(read_mem(_PC++)); return;
      or_c:        OR(_C); return;
      or_d:        OR(_D); return;
      or_e:        OR(_E); return;
      or_h:        OR(_IXh); return;
      or_l:        OR(_IXl); return;
      or_mhl:      { signed char o = read_mem(_PC++); OR(read_mem(_IX+o)); } return;
      outa:        { z80_wait_states iCycleCount = Oa_;} { reg_pair p; p.b.l = read_mem(_PC++); p.b.h = _A; z80_OUT_handler(p, _A); } return;
      pop_af:      POP(AF); return;
      pop_bc:      POP(BC); return;
      pop_de:      POP(DE); return;
      pop_hl:      POP(IX); return;
      push_af:     PUSH(AF); return;
      push_bc:     PUSH(BC); return;
      push_de:     PUSH(DE); return;
      push_hl:     PUSH(IX); return;
      ret:         RET; return;
      ret_c:       if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_m:       if (_F & Sflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_nc:      if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_nz:      if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_p:       if (!(_F & Sflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_pe:      if (_F & Pflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_po:      if (!(_F & Pflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_z:       if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      rla:         RLA; return;
      rlca:        RLCA; return;
      rra:         RRA; return;
      rrca:        RRCA; return;
      rst00:       RST(0x0000); return;
      rst08:       RST(0x0008); return;
      rst10:       RST(0x0010); return;
      rst18:       RST(0x0018); return;
      rst20:       RST(0x0020); return;
      rst28:       RST(0x0028); return;
      rst30:       RST(0x0030); return;
      rst38:       RST(0x0038); return;
      sbc_a:       SBC(_A); return;
      sbc_b:       SBC(_B); return;
      sbc_byte:    SBC(read_mem(_PC++)); return;
      sbc_c:       SBC(_C); return;
      sbc_d:       SBC(_D); return;
      sbc_e:       SBC(_E); return;
      sbc_h:       SBC(_IXh); return;
      sbc_l:       SBC(_IXl); return;
      sbc_mhl:     { signed char o = read_mem(_PC++); SBC(read_mem(_IX+o)); } return;
      scf:         _F = (_F & (Sflag | Zflag | Pflag)) | Cflag | (_A & Xflags); return;
      sub_a:       SUB(_A); return;
      sub_b:       SUB(_B); return;
      sub_byte:    SUB(read_mem(_PC++)); return;
      sub_c:       SUB(_C); return;
      sub_d:       SUB(_D); return;
      sub_e:       SUB(_E); return;
      sub_h:       SUB(_IXh); return;
      sub_l:       SUB(_IXl); return;
      sub_mhl:     { signed char o = read_mem(_PC++); SUB(read_mem(_IX+o)); } return;
      xor_a:       XOR(_A); return;
      xor_b:       XOR(_B); return;
      xor_byte:    XOR(read_mem(_PC++)); return;
      xor_c:       XOR(_C); return;
      xor_d:       XOR(_D); return;
      xor_e:       XOR(_E); return;
      xor_h:       XOR(_IXh); return;
      xor_l:       XOR(_IXl); return;
      xor_mhl:     { signed char o = read_mem(_PC++); XOR(read_mem(_IX+o)); } return;
   //}
}
# else
void z80_pfx_dd(void)
{
   byte bOpCode;

   bOpCode = read_mem(_PC++);
   iCycleCount += cc_xy[bOpCode];
   _R++;
   switch(bOpCode)
   {
      case adc_a:       ADC(_A); break;
      case adc_b:       ADC(_B); break;
      case adc_byte:    ADC(read_mem(_PC++)); break;
      case adc_c:       ADC(_C); break;
      case adc_d:       ADC(_D); break;
      case adc_e:       ADC(_E); break;
      case adc_h:       ADC(_IXh); break;
      case adc_l:       ADC(_IXl); break;
      case adc_mhl:     { signed char o = read_mem(_PC++); ADC(read_mem(_IX+o)); } break;
      case add_a:       ADD(_A); break;
      case add_b:       ADD(_B); break;
      case add_byte:    ADD(read_mem(_PC++)); break;
      case add_c:       ADD(_C); break;
      case add_d:       ADD(_D); break;
      case add_e:       ADD(_E); break;
      case add_h:       ADD(_IXh); break;
      case add_hl_bc:   ADD16(IX, BC); break;
      case add_hl_de:   ADD16(IX, DE); break;
      case add_hl_hl:   ADD16(IX, IX); break;
      case add_hl_sp:   ADD16(IX, SP); break;
      case add_l:       ADD(_IXl); break;
      case add_mhl:     { signed char o = read_mem(_PC++); ADD(read_mem(_IX+o)); } break;
      case and_a:       AND(_A); break;
      case and_b:       AND(_B); break;
      case and_byte:    AND(read_mem(_PC++)); break;
      case and_c:       AND(_C); break;
      case and_d:       AND(_D); break;
      case and_e:       AND(_E); break;
      case and_h:       AND(_IXh); break;
      case and_l:       AND(_IXl); break;
      case and_mhl:     { signed char o = read_mem(_PC++); AND(read_mem(_IX+o)); } break;
      case call:        CALL; break;
      case call_c:      if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_m:      if (_F & Sflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_nc:     if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_nz:     if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_p:      if (!(_F & Sflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_pe:     if (_F & Pflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_po:     if (!(_F & Pflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_z:      if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case ccf:         _F = ((_F & (Sflag | Zflag | Pflag | Cflag)) | ((_F & CF) << 4) | (_A & Xflags)) ^ CF; break;
      case cpl:         _A ^= 0xff; _F = (_F & (Sflag | Zflag | Pflag | Cflag)) | Hflag | Nflag | (_A & Xflags); break;
      case cp_a:        CP(_A); break;
      case cp_b:        CP(_B); break;
      case cp_byte:     CP(read_mem(_PC++)); break;
      case cp_c:        CP(_C); break;
      case cp_d:        CP(_D); break;
      case cp_e:        CP(_E); break;
      case cp_h:        CP(_IXh); break;
      case cp_l:        CP(_IXl); break;
      case cp_mhl:      { signed char o = read_mem(_PC++); CP(read_mem(_IX+o)); } break;
      case daa:         DAA; break;
      case dec_a:       DEC(_A); break;
      case dec_b:       DEC(_B); break;
      case dec_bc:      _BC--; iWSAdjust++; break;
      case dec_c:       DEC(_C); break;
      case dec_d:       DEC(_D); break;
      case dec_de:      _DE--; iWSAdjust++; break;
      case dec_e:       DEC(_E); break;
      case dec_h:       DEC(_IXh); break;
      case dec_hl:      _IX--; iWSAdjust++; break;
      case dec_l:       DEC(_IXl); break;
      case dec_mhl:     { signed char o = read_mem(_PC++); byte b = read_mem(_IX+o); DEC(b); write_mem(_IX+o, b); } break;
      case dec_sp:      _SP--; iWSAdjust++; break;
      case di:          _IFF1 = _IFF2 = 0; z80.EI_issued = 0; break;
      case djnz:        if (--_B) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; } break;
      case ei:          z80.EI_issued = 2; break;
      case exx:         EXX; break;
      case ex_af_af:    EX(z80.AF, z80.AFx); break;
      case ex_de_hl:    EX(z80.DE, z80.HL); break;
      case ex_msp_hl:   EX_SP(IX); iWSAdjust++; break;
      case halt:        _HALT = 1; _PC--; break;
      case ina:         { z80_wait_states iCycleCount = Ia_;} { reg_pair p; p.b.l = read_mem(_PC++); p.b.h = _A; _A = z80_IN_handler(p); } break;
      case inc_a:       INC(_A); break;
      case inc_b:       INC(_B); break;
      case inc_bc:      _BC++; iWSAdjust++; break;
      case inc_c:       INC(_C); break;
      case inc_d:       INC(_D); break;
      case inc_de:      _DE++; iWSAdjust++; break;
      case inc_e:       INC(_E); break;
      case inc_h:       INC(_IXh); break;
      case inc_hl:      _IX++; iWSAdjust++; break;
      case inc_l:       INC(_IXl); break;
      case inc_mhl:     { signed char o = read_mem(_PC++); byte b = read_mem(_IX+o); INC(b); write_mem(_IX+o, b); } break;
      case inc_sp:      _SP++; iWSAdjust++; break;
      case jp:          JP; break;
      case jp_c:        if (_F & Cflag) { JP } else { _PC += 2; }; break;
      case jp_m:        if (_F & Sflag) { JP } else { _PC += 2; }; break;
      case jp_nc:       if (!(_F & Cflag)) { JP } else { _PC += 2; }; break;
      case jp_nz:       if (!(_F & Zflag)) { JP } else { _PC += 2; }; break;
      case jp_p:        if (!(_F & Sflag)) { JP } else { _PC += 2; }; break;
      case jp_pe:       if (_F & Pflag) { JP } else { _PC += 2; }; break;
      case jp_po:       if (!(_F & Pflag)) { JP } else { _PC += 2; }; break;
      case jp_z:        if (_F & Zflag) { JP } else { _PC += 2; }; break;
      case jr:          JR; break;
      case jr_c:        if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case jr_nc:       if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case jr_nz:       if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case jr_z:        if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case ld_a_a:      break;
      case ld_a_b:      _A = _B; break;
      case ld_a_byte:   _A = read_mem(_PC++); break;
      case ld_a_c:      _A = _C; break;
      case ld_a_d:      _A = _D; break;
      case ld_a_e:      _A = _E; break;
      case ld_a_h:      _A = _IXh; break;
      case ld_a_l:      _A = _IXl; break;
      case ld_a_mbc:    _A = read_mem(_BC); break;
      case ld_a_mde:    _A = read_mem(_DE); break;
      case ld_a_mhl:    { signed char o = read_mem(_PC++); _A = read_mem(_IX+o); } break;
      case ld_a_mword:  { reg_pair addr; addr.b.l = read_mem(_PC++); addr.b.h = read_mem(_PC++); _A = read_mem(addr.w.l); } break;
      case ld_bc_word:  z80.BC.b.l = read_mem(_PC++); z80.BC.b.h = read_mem(_PC++); break;
      case ld_b_a:      _B = _A; break;
      case ld_b_b:      break;
      case ld_b_byte:   _B = read_mem(_PC++); break;
      case ld_b_c:      _B = _C; break;
      case ld_b_d:      _B = _D; break;
      case ld_b_e:      _B = _E; break;
      case ld_b_h:      _B = _IXh; break;
      case ld_b_l:      _B = _IXl; break;
      case ld_b_mhl:    { signed char o = read_mem(_PC++); _B = read_mem(_IX+o); } break;
      case ld_c_a:      _C = _A; break;
      case ld_c_b:      _C = _B; break;
      case ld_c_byte:   _C = read_mem(_PC++); break;
      case ld_c_c:      break;
      case ld_c_d:      _C = _D; break;
      case ld_c_e:      _C = _E; break;
      case ld_c_h:      _C = _IXh; break;
      case ld_c_l:      _C = _IXl; break;
      case ld_c_mhl:    { signed char o = read_mem(_PC++); _C = read_mem(_IX+o); } break;
      case ld_de_word:  z80.DE.b.l = read_mem(_PC++); z80.DE.b.h = read_mem(_PC++); break;
      case ld_d_a:      _D = _A; break;
      case ld_d_b:      _D = _B; break;
      case ld_d_byte:   _D = read_mem(_PC++); break;
      case ld_d_c:      _D = _C; break;
      case ld_d_d:      break;
      case ld_d_e:      _D = _E; break;
      case ld_d_h:      _D = _IXh; break;
      case ld_d_l:      _D = _IXl; break;
      case ld_d_mhl:    { signed char o = read_mem(_PC++); _D = read_mem(_IX+o); } break;
      case ld_e_a:      _E = _A; break;
      case ld_e_b:      _E = _B; break;
      case ld_e_byte:   _E = read_mem(_PC++); break;
      case ld_e_c:      _E = _C; break;
      case ld_e_d:      _E = _D; break;
      case ld_e_e:      break;
      case ld_e_h:      _E = _IXh; break;
      case ld_e_l:      _E = _IXl; break;
      case ld_e_mhl:    { signed char o = read_mem(_PC++); _E = read_mem(_IX+o); } break;
      case ld_hl_mword: LD16_MEM(IX); break;
      case ld_hl_word:  z80.IX.b.l = read_mem(_PC++); z80.IX.b.h = read_mem(_PC++); break;
      case ld_h_a:      _IXh = _A; break;
      case ld_h_b:      _IXh = _B; break;
      case ld_h_byte:   _IXh = read_mem(_PC++); break;
      case ld_h_c:      _IXh = _C; break;
      case ld_h_d:      _IXh = _D; break;
      case ld_h_e:      _IXh = _E; break;
      case ld_h_h:      break;
      case ld_h_l:      _IXh = _IXl; break;
      case ld_h_mhl:    { signed char o = read_mem(_PC++); _H = read_mem(_IX+o); } break;
      case ld_l_a:      _IXl = _A; break;
      case ld_l_b:      _IXl = _B; break;
      case ld_l_byte:   _IXl = read_mem(_PC++); break;
      case ld_l_c:      _IXl = _C; break;
      case ld_l_d:      _IXl = _D; break;
      case ld_l_e:      _IXl = _E; break;
      case ld_l_h:      _IXl = _IXh; break;
      case ld_l_l:      break;
      case ld_l_mhl:    { signed char o = read_mem(_PC++); _L = read_mem(_IX+o); } break;
      case ld_mbc_a:    write_mem(_BC, _A); break;
      case ld_mde_a:    write_mem(_DE, _A); break;
      case ld_mhl_a:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _A); } break;
      case ld_mhl_b:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _B); } break;
      case ld_mhl_byte: { signed char o = read_mem(_PC++); byte b = read_mem(_PC++); write_mem(_IX+o, b); } break;
      case ld_mhl_c:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _C); } break;
      case ld_mhl_d:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _D); } break;
      case ld_mhl_e:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _E); } break;
      case ld_mhl_h:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _H); } break;
      case ld_mhl_l:    { signed char o = read_mem(_PC++); write_mem(_IX+o, _L); } break;
      case ld_mword_a:  { reg_pair addr; addr.b.l = read_mem(_PC++); addr.b.h = read_mem(_PC++); write_mem(addr.w.l, _A); } break;
      case ld_mword_hl: LDMEM_16(IX); break;
      case ld_pc_hl:    _PC = _IX; break;
      case ld_sp_hl:    _SP = _IX; iWSAdjust++; break;
      case ld_sp_word:  z80.SP.b.l = read_mem(_PC++); z80.SP.b.h = read_mem(_PC++); break;
      case nop:         break;
      case or_a:        OR(_A); break;
      case or_b:        OR(_B); break;
      case or_byte:     OR(read_mem(_PC++)); break;
      case or_c:        OR(_C); break;
      case or_d:        OR(_D); break;
      case or_e:        OR(_E); break;
      case or_h:        OR(_IXh); break;
      case or_l:        OR(_IXl); break;
      case or_mhl:      { signed char o = read_mem(_PC++); OR(read_mem(_IX+o)); } break;
      case outa:        { z80_wait_states iCycleCount = Oa_;} { reg_pair p; p.b.l = read_mem(_PC++); p.b.h = _A; z80_OUT_handler(p, _A); } break;
      case pfx_cb:      z80_pfx_ddcb(); break;
      case pfx_dd:      z80_pfx_dd(); break;
      case pfx_ed:      z80_pfx_ed(); break;
      case pfx_fd:      z80_pfx_fd(); break;
      case pop_af:      POP(AF); break;
      case pop_bc:      POP(BC); break;
      case pop_de:      POP(DE); break;
      case pop_hl:      POP(IX); break;
      case push_af:     PUSH(AF); break;
      case push_bc:     PUSH(BC); break;
      case push_de:     PUSH(DE); break;
      case push_hl:     PUSH(IX); break;
      case ret:         RET; break;
      case ret_c:       if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_m:       if (_F & Sflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_nc:      if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_nz:      if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_p:       if (!(_F & Sflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_pe:      if (_F & Pflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_po:      if (!(_F & Pflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_z:       if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case rla:         RLA; break;
      case rlca:        RLCA; break;
      case rra:         RRA; break;
      case rrca:        RRCA; break;
      case rst00:       RST(0x0000); break;
      case rst08:       RST(0x0008); break;
      case rst10:       RST(0x0010); break;
      case rst18:       RST(0x0018); break;
      case rst20:       RST(0x0020); break;
      case rst28:       RST(0x0028); break;
      case rst30:       RST(0x0030); break;
      case rst38:       RST(0x0038); break;
      case sbc_a:       SBC(_A); break;
      case sbc_b:       SBC(_B); break;
      case sbc_byte:    SBC(read_mem(_PC++)); break;
      case sbc_c:       SBC(_C); break;
      case sbc_d:       SBC(_D); break;
      case sbc_e:       SBC(_E); break;
      case sbc_h:       SBC(_IXh); break;
      case sbc_l:       SBC(_IXl); break;
      case sbc_mhl:     { signed char o = read_mem(_PC++); SBC(read_mem(_IX+o)); } break;
      case scf:         _F = (_F & (Sflag | Zflag | Pflag)) | Cflag | (_A & Xflags); break;
      case sub_a:       SUB(_A); break;
      case sub_b:       SUB(_B); break;
      case sub_byte:    SUB(read_mem(_PC++)); break;
      case sub_c:       SUB(_C); break;
      case sub_d:       SUB(_D); break;
      case sub_e:       SUB(_E); break;
      case sub_h:       SUB(_IXh); break;
      case sub_l:       SUB(_IXl); break;
      case sub_mhl:     { signed char o = read_mem(_PC++); SUB(read_mem(_IX+o)); } break;
      case xor_a:       XOR(_A); break;
      case xor_b:       XOR(_B); break;
      case xor_byte:    XOR(read_mem(_PC++)); break;
      case xor_c:       XOR(_C); break;
      case xor_d:       XOR(_D); break;
      case xor_e:       XOR(_E); break;
      case xor_h:       XOR(_IXh); break;
      case xor_l:       XOR(_IXl); break;
      case xor_mhl:     { signed char o = read_mem(_PC++); XOR(read_mem(_IX+o)); } break;
   }
}
# endif


# ifdef Z80_JMP_TBL
void z80_pfx_ddcb(void)
{
   __label__
   rlc_b, rlc_c, rlc_d, rlc_e, rlc_h, rlc_l, rlc_mhl, rlc_a,
   rrc_b, rrc_c, rrc_d, rrc_e, rrc_h, rrc_l, rrc_mhl, rrc_a,
   rl_b, rl_c, rl_d, rl_e, rl_h, rl_l, rl_mhl, rl_a,
   rr_b, rr_c, rr_d, rr_e, rr_h, rr_l, rr_mhl, rr_a,
   sla_b, sla_c, sla_d, sla_e, sla_h, sla_l, sla_mhl, sla_a,
   sra_b, sra_c, sra_d, sra_e, sra_h, sra_l, sra_mhl, sra_a,
   sll_b, sll_c, sll_d, sll_e, sll_h, sll_l, sll_mhl, sll_a,
   srl_b, srl_c, srl_d, srl_e, srl_h, srl_l, srl_mhl, srl_a,
   bit0_b, bit0_c, bit0_d, bit0_e, bit0_h, bit0_l, bit0_mhl, bit0_a,
   bit1_b, bit1_c, bit1_d, bit1_e, bit1_h, bit1_l, bit1_mhl, bit1_a,
   bit2_b, bit2_c, bit2_d, bit2_e, bit2_h, bit2_l, bit2_mhl, bit2_a,
   bit3_b, bit3_c, bit3_d, bit3_e, bit3_h, bit3_l, bit3_mhl, bit3_a,
   bit4_b, bit4_c, bit4_d, bit4_e, bit4_h, bit4_l, bit4_mhl, bit4_a,
   bit5_b, bit5_c, bit5_d, bit5_e, bit5_h, bit5_l, bit5_mhl, bit5_a,
   bit6_b, bit6_c, bit6_d, bit6_e, bit6_h, bit6_l, bit6_mhl, bit6_a,
   bit7_b, bit7_c, bit7_d, bit7_e, bit7_h, bit7_l, bit7_mhl, bit7_a,
   res0_b, res0_c, res0_d, res0_e, res0_h, res0_l, res0_mhl, res0_a,
   res1_b, res1_c, res1_d, res1_e, res1_h, res1_l, res1_mhl, res1_a,
   res2_b, res2_c, res2_d, res2_e, res2_h, res2_l, res2_mhl, res2_a,
   res3_b, res3_c, res3_d, res3_e, res3_h, res3_l, res3_mhl, res3_a,
   res4_b, res4_c, res4_d, res4_e, res4_h, res4_l, res4_mhl, res4_a,
   res5_b, res5_c, res5_d, res5_e, res5_h, res5_l, res5_mhl, res5_a,
   res6_b, res6_c, res6_d, res6_e, res6_h, res6_l, res6_mhl, res6_a,
   res7_b, res7_c, res7_d, res7_e, res7_h, res7_l, res7_mhl, res7_a,
   set0_b, set0_c, set0_d, set0_e, set0_h, set0_l, set0_mhl, set0_a,
   set1_b, set1_c, set1_d, set1_e, set1_h, set1_l, set1_mhl, set1_a,
   set2_b, set2_c, set2_d, set2_e, set2_h, set2_l, set2_mhl, set2_a,
   set3_b, set3_c, set3_d, set3_e, set3_h, set3_l, set3_mhl, set3_a,
   set4_b, set4_c, set4_d, set4_e, set4_h, set4_l, set4_mhl, set4_a,
   set5_b, set5_c, set5_d, set5_e, set5_h, set5_l, set5_mhl, set5_a,
   set6_b, set6_c, set6_d, set6_e, set6_h, set6_l, set6_mhl, set6_a,
   set7_b, set7_c, set7_d, set7_e, set7_h, set7_l, set7_mhl, set7_a;

    static const void* const a_jump_table[256] =  { &&
   rlc_b,&& rlc_c,&& rlc_d,&& rlc_e,&& rlc_h,&& rlc_l,&& rlc_mhl,&& rlc_a,&&
   rrc_b,&& rrc_c,&& rrc_d,&& rrc_e,&& rrc_h,&& rrc_l,&& rrc_mhl,&& rrc_a,&&
   rl_b,&& rl_c,&& rl_d,&& rl_e,&& rl_h,&& rl_l,&& rl_mhl,&& rl_a,&&
   rr_b,&& rr_c,&& rr_d,&& rr_e,&& rr_h,&& rr_l,&& rr_mhl,&& rr_a,&&
   sla_b,&& sla_c,&& sla_d,&& sla_e,&& sla_h,&& sla_l,&& sla_mhl,&& sla_a,&&
   sra_b,&& sra_c,&& sra_d,&& sra_e,&& sra_h,&& sra_l,&& sra_mhl,&& sra_a,&&
   sll_b,&& sll_c,&& sll_d,&& sll_e,&& sll_h,&& sll_l,&& sll_mhl,&& sll_a,&&
   srl_b,&& srl_c,&& srl_d,&& srl_e,&& srl_h,&& srl_l,&& srl_mhl,&& srl_a,&&
   bit0_b,&& bit0_c,&& bit0_d,&& bit0_e,&& bit0_h,&& bit0_l,&& bit0_mhl,&& bit0_a,&&
   bit1_b,&& bit1_c,&& bit1_d,&& bit1_e,&& bit1_h,&& bit1_l,&& bit1_mhl,&& bit1_a,&&
   bit2_b,&& bit2_c,&& bit2_d,&& bit2_e,&& bit2_h,&& bit2_l,&& bit2_mhl,&& bit2_a,&&
   bit3_b,&& bit3_c,&& bit3_d,&& bit3_e,&& bit3_h,&& bit3_l,&& bit3_mhl,&& bit3_a,&&
   bit4_b,&& bit4_c,&& bit4_d,&& bit4_e,&& bit4_h,&& bit4_l,&& bit4_mhl,&& bit4_a,&&
   bit5_b,&& bit5_c,&& bit5_d,&& bit5_e,&& bit5_h,&& bit5_l,&& bit5_mhl,&& bit5_a,&&
   bit6_b,&& bit6_c,&& bit6_d,&& bit6_e,&& bit6_h,&& bit6_l,&& bit6_mhl,&& bit6_a,&&
   bit7_b,&& bit7_c,&& bit7_d,&& bit7_e,&& bit7_h,&& bit7_l,&& bit7_mhl,&& bit7_a,&&
   res0_b,&& res0_c,&& res0_d,&& res0_e,&& res0_h,&& res0_l,&& res0_mhl,&& res0_a,&&
   res1_b,&& res1_c,&& res1_d,&& res1_e,&& res1_h,&& res1_l,&& res1_mhl,&& res1_a,&&
   res2_b,&& res2_c,&& res2_d,&& res2_e,&& res2_h,&& res2_l,&& res2_mhl,&& res2_a,&&
   res3_b,&& res3_c,&& res3_d,&& res3_e,&& res3_h,&& res3_l,&& res3_mhl,&& res3_a,&&
   res4_b,&& res4_c,&& res4_d,&& res4_e,&& res4_h,&& res4_l,&& res4_mhl,&& res4_a,&&
   res5_b,&& res5_c,&& res5_d,&& res5_e,&& res5_h,&& res5_l,&& res5_mhl,&& res5_a,&&
   res6_b,&& res6_c,&& res6_d,&& res6_e,&& res6_h,&& res6_l,&& res6_mhl,&& res6_a,&&
   res7_b,&& res7_c,&& res7_d,&& res7_e,&& res7_h,&& res7_l,&& res7_mhl,&& res7_a,&&
   set0_b,&& set0_c,&& set0_d,&& set0_e,&& set0_h,&& set0_l,&& set0_mhl,&& set0_a,&&
   set1_b,&& set1_c,&& set1_d,&& set1_e,&& set1_h,&& set1_l,&& set1_mhl,&& set1_a,&&
   set2_b,&& set2_c,&& set2_d,&& set2_e,&& set2_h,&& set2_l,&& set2_mhl,&& set2_a,&&
   set3_b,&& set3_c,&& set3_d,&& set3_e,&& set3_h,&& set3_l,&& set3_mhl,&& set3_a,&&
   set4_b,&& set4_c,&& set4_d,&& set4_e,&& set4_h,&& set4_l,&& set4_mhl,&& set4_a,&&
   set5_b,&& set5_c,&& set5_d,&& set5_e,&& set5_h,&& set5_l,&& set5_mhl,&& set5_a,&&
   set6_b,&& set6_c,&& set6_d,&& set6_e,&& set6_h,&& set6_l,&& set6_mhl,&& set6_a,&&
   set7_b,&& set7_c,&& set7_d,&& set7_e,&& set7_h,&& set7_l,&& set7_mhl,&& set7_a
   };

   signed char o;
   byte bOpCode;

   o = read_mem(_PC++); // offset
   bOpCode = read_mem(_PC++);
   iCycleCount += cc_xycb[bOpCode];
	 goto *a_jump_table[bOpCode];
   //switch(bOpCode)
   //{
      bit0_a:
      bit0_b:
      bit0_c:
      bit0_d:
      bit0_e:
      bit0_h:
      bit0_l:
      bit0_mhl:    BIT_XY(0, read_mem(_IX+o)); return;
      bit1_a:
      bit1_b:
      bit1_c:
      bit1_d:
      bit1_e:
      bit1_h:
      bit1_l:
      bit1_mhl:    BIT_XY(1, read_mem(_IX+o)); return;
      bit2_a:
      bit2_b:
      bit2_c:
      bit2_d:
      bit2_e:
      bit2_h:
      bit2_l:
      bit2_mhl:    BIT_XY(2, read_mem(_IX+o)); return;
      bit3_a:   
      bit3_b:   
      bit3_c:   
      bit3_d:  
      bit3_e: 
      bit3_h:     
      bit3_l:    
      bit3_mhl:    BIT_XY(3, read_mem(_IX+o)); return;
      bit4_a:   
      bit4_b:   
      bit4_c:  
      bit4_d:  
      bit4_e:  
      bit4_h:  
      bit4_l:  
      bit4_mhl:    BIT_XY(4, read_mem(_IX+o)); return;
      bit5_a:  
      bit5_b:  
      bit5_c:  
      bit5_d:  
      bit5_e:  
      bit5_h:  
      bit5_l:  
      bit5_mhl:    BIT_XY(5, read_mem(_IX+o)); return;
      bit6_a: 
      bit6_b:     
      bit6_c:    
      bit6_d:   
      bit6_e:  
      bit6_h: 
      bit6_l:
      bit6_mhl:    BIT_XY(6, read_mem(_IX+o)); return;
      bit7_a:     
      bit7_b:    
      bit7_c:   
      bit7_d:  
      bit7_e: 
      bit7_h:
      bit7_l:  
      bit7_mhl:    BIT_XY(7, read_mem(_IX+o)); return;
      res0_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(0, _A)); return;
      res0_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(0, _B)); return;
      res0_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(0, _C)); return;
      res0_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(0, _D)); return;
      res0_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(0, _E)); return;
      res0_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(0, _H)); return;
      res0_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(0, _L)); return;
      res0_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(0, b)); } return;
      res1_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(1, _A)); return;
      res1_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(1, _B)); return;
      res1_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(1, _C)); return;
      res1_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(1, _D)); return;
      res1_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(1, _E)); return;
      res1_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(1, _H)); return;
      res1_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(1, _L)); return;
      res1_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(1, b)); } return;
      res2_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(2, _A)); return;
      res2_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(2, _B)); return;
      res2_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(2, _C)); return;
      res2_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(2, _D)); return;
      res2_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(2, _E)); return;
      res2_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(2, _H)); return;
      res2_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(2, _L)); return;
      res2_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(2, b)); } return;
      res3_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(3, _A)); return;
      res3_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(3, _B)); return;
      res3_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(3, _C)); return;
      res3_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(3, _D)); return;
      res3_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(3, _E)); return;
      res3_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(3, _H)); return;
      res3_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(3, _L)); return;
      res3_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(3, b)); } return;
      res4_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(4, _A)); return;
      res4_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(4, _B)); return;
      res4_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(4, _C)); return;
      res4_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(4, _D)); return;
      res4_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(4, _E)); return;
      res4_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(4, _H)); return;
      res4_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(4, _L)); return;
      res4_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(4, b)); } return;
      res5_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(5, _A)); return;
      res5_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(5, _B)); return;
      res5_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(5, _C)); return;
      res5_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(5, _D)); return;
      res5_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(5, _E)); return;
      res5_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(5, _H)); return;
      res5_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(5, _L)); return;
      res5_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(5, b)); } return;
      res6_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(6, _A)); return;
      res6_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(6, _B)); return;
      res6_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(6, _C)); return;
      res6_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(6, _D)); return;
      res6_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(6, _E)); return;
      res6_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(6, _H)); return;
      res6_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(6, _L)); return;
      res6_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(6, b)); } return;
      res7_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(7, _A)); return;
      res7_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(7, _B)); return;
      res7_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(7, _C)); return;
      res7_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(7, _D)); return;
      res7_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(7, _E)); return;
      res7_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(7, _H)); return;
      res7_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(7, _L)); return;
      res7_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(7, b)); } return;
      rlc_a:       _A = read_mem(_IX+o); _A = RLC(_A); write_mem(_IX+o, _A); return;
      rlc_b:       _B = read_mem(_IX+o); _B = RLC(_B); write_mem(_IX+o, _B); return;
      rlc_c:       _C = read_mem(_IX+o); _C = RLC(_C); write_mem(_IX+o, _C); return;
      rlc_d:       _D = read_mem(_IX+o); _D = RLC(_D); write_mem(_IX+o, _D); return;
      rlc_e:       _E = read_mem(_IX+o); _E = RLC(_E); write_mem(_IX+o, _E); return;
      rlc_h:       _H = read_mem(_IX+o); _H = RLC(_H); write_mem(_IX+o, _H); return;
      rlc_l:       _L = read_mem(_IX+o); _L = RLC(_L); write_mem(_IX+o, _L); return;
      rlc_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, RLC(b)); } return;
      rl_a:        _A = read_mem(_IX+o); _A = RL(_A); write_mem(_IX+o, _A); return;
      rl_b:        _B = read_mem(_IX+o); _B = RL(_B); write_mem(_IX+o, _B); return;
      rl_c:        _C = read_mem(_IX+o); _C = RL(_C); write_mem(_IX+o, _C); return;
      rl_d:        _D = read_mem(_IX+o); _D = RL(_D); write_mem(_IX+o, _D); return;
      rl_e:        _E = read_mem(_IX+o); _E = RL(_E); write_mem(_IX+o, _E); return;
      rl_h:        _H = read_mem(_IX+o); _H = RL(_H); write_mem(_IX+o, _H); return;
      rl_l:        _L = read_mem(_IX+o); _L = RL(_L); write_mem(_IX+o, _L); return;
      rl_mhl:      { byte b = read_mem(_IX+o); write_mem(_IX+o, RL(b)); } return;
      rrc_a:       _A = read_mem(_IX+o); _A = RRC(_A); write_mem(_IX+o, _A); return;
      rrc_b:       _B = read_mem(_IX+o); _B = RRC(_B); write_mem(_IX+o, _B); return;
      rrc_c:       _C = read_mem(_IX+o); _C = RRC(_C); write_mem(_IX+o, _C); return;
      rrc_d:       _D = read_mem(_IX+o); _D = RRC(_D); write_mem(_IX+o, _D); return;
      rrc_e:       _E = read_mem(_IX+o); _E = RRC(_E); write_mem(_IX+o, _E); return;
      rrc_h:       _H = read_mem(_IX+o); _H = RRC(_H); write_mem(_IX+o, _H); return;
      rrc_l:       _L = read_mem(_IX+o); _L = RRC(_L); write_mem(_IX+o, _L); return;
      rrc_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, RRC(b)); } return;
      rr_a:        _A = read_mem(_IX+o); _A = RR(_A); write_mem(_IX+o, _A); return;
      rr_b:        _B = read_mem(_IX+o); _B = RR(_B); write_mem(_IX+o, _B); return;
      rr_c:        _C = read_mem(_IX+o); _C = RR(_C); write_mem(_IX+o, _C); return;
      rr_d:        _D = read_mem(_IX+o); _D = RR(_D); write_mem(_IX+o, _D); return;
      rr_e:        _E = read_mem(_IX+o); _E = RR(_E); write_mem(_IX+o, _E); return;
      rr_h:        _H = read_mem(_IX+o); _H = RR(_H); write_mem(_IX+o, _H); return;
      rr_l:        _L = read_mem(_IX+o); _L = RR(_L); write_mem(_IX+o, _L); return;
      rr_mhl:      { byte b = read_mem(_IX+o); write_mem(_IX+o, RR(b)); } return;
      set0_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(0, _A)); return;
      set0_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(0, _B)); return;
      set0_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(0, _C)); return;
      set0_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(0, _D)); return;
      set0_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(0, _E)); return;
      set0_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(0, _H)); return;
      set0_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(0, _L)); return;
      set0_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(0, b)); } return;
      set1_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(1, _A)); return;
      set1_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(1, _B)); return;
      set1_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(1, _C)); return;
      set1_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(1, _D)); return;
      set1_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(1, _E)); return;
      set1_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(1, _H)); return;
      set1_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(1, _L)); return;
      set1_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(1, b)); } return;
      set2_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(2, _A)); return;
      set2_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(2, _B)); return;
      set2_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(2, _C)); return;
      set2_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(2, _D)); return;
      set2_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(2, _E)); return;
      set2_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(2, _H)); return;
      set2_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(2, _L)); return;
      set2_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(2, b)); } return;
      set3_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(3, _A)); return;
      set3_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(3, _B)); return;
      set3_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(3, _C)); return;
      set3_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(3, _D)); return;
      set3_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(3, _E)); return;
      set3_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(3, _H)); return;
      set3_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(3, _L)); return;
      set3_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(3, b)); } return;
      set4_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(4, _A)); return;
      set4_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(4, _B)); return;
      set4_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(4, _C)); return;
      set4_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(4, _D)); return;
      set4_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(4, _E)); return;
      set4_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(4, _H)); return;
      set4_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(4, _L)); return;
      set4_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(4, b)); } return;
      set5_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(5, _A)); return;
      set5_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(5, _B)); return;
      set5_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(5, _C)); return;
      set5_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(5, _D)); return;
      set5_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(5, _E)); return;
      set5_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(5, _H)); return;
      set5_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(5, _L)); return;
      set5_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(5, b)); } return;
      set6_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(6, _A)); return;
      set6_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(6, _B)); return;
      set6_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(6, _C)); return;
      set6_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(6, _D)); return;
      set6_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(6, _E)); return;
      set6_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(6, _H)); return;
      set6_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(6, _L)); return;
      set6_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(6, b)); } return;
      set7_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(7, _A)); return;
      set7_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(7, _B)); return;
      set7_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(7, _C)); return;
      set7_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(7, _D)); return;
      set7_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(7, _E)); return;
      set7_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(7, _H)); return;
      set7_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(7, _L)); return;
      set7_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(7, b)); } return;
      sla_a:       _A = read_mem(_IX+o); _A = SLA(_A); write_mem(_IX+o, _A); return;
      sla_b:       _B = read_mem(_IX+o); _B = SLA(_B); write_mem(_IX+o, _B); return;
      sla_c:       _C = read_mem(_IX+o); _C = SLA(_C); write_mem(_IX+o, _C); return;
      sla_d:       _D = read_mem(_IX+o); _D = SLA(_D); write_mem(_IX+o, _D); return;
      sla_e:       _E = read_mem(_IX+o); _E = SLA(_E); write_mem(_IX+o, _E); return;
      sla_h:       _H = read_mem(_IX+o); _H = SLA(_H); write_mem(_IX+o, _H); return;
      sla_l:       _L = read_mem(_IX+o); _L = SLA(_L); write_mem(_IX+o, _L); return;
      sla_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, SLA(b)); } return;
      sll_a:       _A = read_mem(_IX+o); _A = SLL(_A); write_mem(_IX+o, _A); return;
      sll_b:       _B = read_mem(_IX+o); _B = SLL(_B); write_mem(_IX+o, _B); return;
      sll_c:       _C = read_mem(_IX+o); _C = SLL(_C); write_mem(_IX+o, _C); return;
      sll_d:       _D = read_mem(_IX+o); _D = SLL(_D); write_mem(_IX+o, _D); return;
      sll_e:       _E = read_mem(_IX+o); _E = SLL(_E); write_mem(_IX+o, _E); return;
      sll_h:       _H = read_mem(_IX+o); _H = SLL(_H); write_mem(_IX+o, _H); return;
      sll_l:       _L = read_mem(_IX+o); _L = SLL(_L); write_mem(_IX+o, _L); return;
      sll_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, SLL(b)); } return;
      sra_a:       _A = read_mem(_IX+o); _A = SRA(_A); write_mem(_IX+o, _A); return;
      sra_b:       _B = read_mem(_IX+o); _B = SRA(_B); write_mem(_IX+o, _B); return;
      sra_c:       _C = read_mem(_IX+o); _C = SRA(_C); write_mem(_IX+o, _C); return;
      sra_d:       _D = read_mem(_IX+o); _D = SRA(_D); write_mem(_IX+o, _D); return;
      sra_e:       _E = read_mem(_IX+o); _E = SRA(_E); write_mem(_IX+o, _E); return;
      sra_h:       _H = read_mem(_IX+o); _H = SRA(_H); write_mem(_IX+o, _H); return;
      sra_l:       _L = read_mem(_IX+o); _L = SRA(_L); write_mem(_IX+o, _L); return;
      sra_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, SRA(b)); } return;
      srl_a:       _A = read_mem(_IX+o); _A = SRL(_A); write_mem(_IX+o, _A); return;
      srl_b:       _B = read_mem(_IX+o); _B = SRL(_B); write_mem(_IX+o, _B); return;
      srl_c:       _C = read_mem(_IX+o); _C = SRL(_C); write_mem(_IX+o, _C); return;
      srl_d:       _D = read_mem(_IX+o); _D = SRL(_D); write_mem(_IX+o, _D); return;
      srl_e:       _E = read_mem(_IX+o); _E = SRL(_E); write_mem(_IX+o, _E); return;
      srl_h:       _H = read_mem(_IX+o); _H = SRL(_H); write_mem(_IX+o, _H); return;
      srl_l:       _L = read_mem(_IX+o); _L = SRL(_L); write_mem(_IX+o, _L); return;
      srl_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, SRL(b)); } return;
   //}
}
# else
void z80_pfx_ddcb(void)
{
   signed char o;
   byte bOpCode;

   o = read_mem(_PC++); // offset
   bOpCode = read_mem(_PC++);
   iCycleCount += cc_xycb[bOpCode];
   switch(bOpCode)
   {
      case bit0_a:    
      case bit0_b:     
      case bit0_c:     
      case bit0_d:    
      case bit0_e:   
      case bit0_h:  
      case bit0_l: 
      case bit0_mhl:    BIT_XY(0, read_mem(_IX+o)); break;
      case bit1_a:     
      case bit1_b:    
      case bit1_c:   
      case bit1_d:  
      case bit1_e: 
      case bit1_h:
      case bit1_l:   
      case bit1_mhl:    BIT_XY(1, read_mem(_IX+o)); break;
      case bit2_a:  
      case bit2_b:     
      case bit2_c:    
      case bit2_d:   
      case bit2_e:  
      case bit2_h:  
      case bit2_l:  
      case bit2_mhl:    BIT_XY(2, read_mem(_IX+o)); break;
      case bit3_a:     
      case bit3_b:    
      case bit3_c:   
      case bit3_d:  
      case bit3_e: 
      case bit3_h:    
      case bit3_l:     
      case bit3_mhl:    BIT_XY(3, read_mem(_IX+o)); break;
      case bit4_a:    
      case bit4_b:   
      case bit4_c:  
      case bit4_d:     
      case bit4_e:    
      case bit4_h:   
      case bit4_l:  
      case bit4_mhl:    BIT_XY(4, read_mem(_IX+o)); break;
      case bit5_a:     
      case bit5_b:    
      case bit5_c:   
      case bit5_d:  
      case bit5_e: 
      case bit5_h:
      case bit5_l:   
      case bit5_mhl:    BIT_XY(5, read_mem(_IX+o)); break;
      case bit6_a:     
      case bit6_b:    
      case bit6_c:   
      case bit6_d:  
      case bit6_e: 
      case bit6_h:
      case bit6_l:     
      case bit6_mhl:    BIT_XY(6, read_mem(_IX+o)); break;
      case bit7_a:    
      case bit7_b:   
      case bit7_c:  
      case bit7_d: 
      case bit7_e:     
      case bit7_h:    
      case bit7_l:   
      case bit7_mhl:    BIT_XY(7, read_mem(_IX+o)); break;
      case res0_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(0, _A)); break;
      case res0_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(0, _B)); break;
      case res0_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(0, _C)); break;
      case res0_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(0, _D)); break;
      case res0_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(0, _E)); break;
      case res0_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(0, _H)); break;
      case res0_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(0, _L)); break;
      case res0_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(0, b)); } break;
      case res1_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(1, _A)); break;
      case res1_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(1, _B)); break;
      case res1_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(1, _C)); break;
      case res1_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(1, _D)); break;
      case res1_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(1, _E)); break;
      case res1_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(1, _H)); break;
      case res1_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(1, _L)); break;
      case res1_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(1, b)); } break;
      case res2_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(2, _A)); break;
      case res2_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(2, _B)); break;
      case res2_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(2, _C)); break;
      case res2_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(2, _D)); break;
      case res2_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(2, _E)); break;
      case res2_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(2, _H)); break;
      case res2_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(2, _L)); break;
      case res2_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(2, b)); } break;
      case res3_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(3, _A)); break;
      case res3_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(3, _B)); break;
      case res3_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(3, _C)); break;
      case res3_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(3, _D)); break;
      case res3_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(3, _E)); break;
      case res3_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(3, _H)); break;
      case res3_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(3, _L)); break;
      case res3_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(3, b)); } break;
      case res4_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(4, _A)); break;
      case res4_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(4, _B)); break;
      case res4_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(4, _C)); break;
      case res4_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(4, _D)); break;
      case res4_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(4, _E)); break;
      case res4_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(4, _H)); break;
      case res4_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(4, _L)); break;
      case res4_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(4, b)); } break;
      case res5_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(5, _A)); break;
      case res5_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(5, _B)); break;
      case res5_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(5, _C)); break;
      case res5_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(5, _D)); break;
      case res5_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(5, _E)); break;
      case res5_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(5, _H)); break;
      case res5_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(5, _L)); break;
      case res5_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(5, b)); } break;
      case res6_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(6, _A)); break;
      case res6_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(6, _B)); break;
      case res6_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(6, _C)); break;
      case res6_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(6, _D)); break;
      case res6_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(6, _E)); break;
      case res6_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(6, _H)); break;
      case res6_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(6, _L)); break;
      case res6_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(6, b)); } break;
      case res7_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = RES(7, _A)); break;
      case res7_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = RES(7, _B)); break;
      case res7_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = RES(7, _C)); break;
      case res7_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = RES(7, _D)); break;
      case res7_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = RES(7, _E)); break;
      case res7_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = RES(7, _H)); break;
      case res7_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = RES(7, _L)); break;
      case res7_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, RES(7, b)); } break;
      case rlc_a:       _A = read_mem(_IX+o); _A = RLC(_A); write_mem(_IX+o, _A); break;
      case rlc_b:       _B = read_mem(_IX+o); _B = RLC(_B); write_mem(_IX+o, _B); break;
      case rlc_c:       _C = read_mem(_IX+o); _C = RLC(_C); write_mem(_IX+o, _C); break;
      case rlc_d:       _D = read_mem(_IX+o); _D = RLC(_D); write_mem(_IX+o, _D); break;
      case rlc_e:       _E = read_mem(_IX+o); _E = RLC(_E); write_mem(_IX+o, _E); break;
      case rlc_h:       _H = read_mem(_IX+o); _H = RLC(_H); write_mem(_IX+o, _H); break;
      case rlc_l:       _L = read_mem(_IX+o); _L = RLC(_L); write_mem(_IX+o, _L); break;
      case rlc_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, RLC(b)); } break;
      case rl_a:        _A = read_mem(_IX+o); _A = RL(_A); write_mem(_IX+o, _A); break;
      case rl_b:        _B = read_mem(_IX+o); _B = RL(_B); write_mem(_IX+o, _B); break;
      case rl_c:        _C = read_mem(_IX+o); _C = RL(_C); write_mem(_IX+o, _C); break;
      case rl_d:        _D = read_mem(_IX+o); _D = RL(_D); write_mem(_IX+o, _D); break;
      case rl_e:        _E = read_mem(_IX+o); _E = RL(_E); write_mem(_IX+o, _E); break;
      case rl_h:        _H = read_mem(_IX+o); _H = RL(_H); write_mem(_IX+o, _H); break;
      case rl_l:        _L = read_mem(_IX+o); _L = RL(_L); write_mem(_IX+o, _L); break;
      case rl_mhl:      { byte b = read_mem(_IX+o); write_mem(_IX+o, RL(b)); } break;
      case rrc_a:       _A = read_mem(_IX+o); _A = RRC(_A); write_mem(_IX+o, _A); break;
      case rrc_b:       _B = read_mem(_IX+o); _B = RRC(_B); write_mem(_IX+o, _B); break;
      case rrc_c:       _C = read_mem(_IX+o); _C = RRC(_C); write_mem(_IX+o, _C); break;
      case rrc_d:       _D = read_mem(_IX+o); _D = RRC(_D); write_mem(_IX+o, _D); break;
      case rrc_e:       _E = read_mem(_IX+o); _E = RRC(_E); write_mem(_IX+o, _E); break;
      case rrc_h:       _H = read_mem(_IX+o); _H = RRC(_H); write_mem(_IX+o, _H); break;
      case rrc_l:       _L = read_mem(_IX+o); _L = RRC(_L); write_mem(_IX+o, _L); break;
      case rrc_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, RRC(b)); } break;
      case rr_a:        _A = read_mem(_IX+o); _A = RR(_A); write_mem(_IX+o, _A); break;
      case rr_b:        _B = read_mem(_IX+o); _B = RR(_B); write_mem(_IX+o, _B); break;
      case rr_c:        _C = read_mem(_IX+o); _C = RR(_C); write_mem(_IX+o, _C); break;
      case rr_d:        _D = read_mem(_IX+o); _D = RR(_D); write_mem(_IX+o, _D); break;
      case rr_e:        _E = read_mem(_IX+o); _E = RR(_E); write_mem(_IX+o, _E); break;
      case rr_h:        _H = read_mem(_IX+o); _H = RR(_H); write_mem(_IX+o, _H); break;
      case rr_l:        _L = read_mem(_IX+o); _L = RR(_L); write_mem(_IX+o, _L); break;
      case rr_mhl:      { byte b = read_mem(_IX+o); write_mem(_IX+o, RR(b)); } break;
      case set0_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(0, _A)); break;
      case set0_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(0, _B)); break;
      case set0_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(0, _C)); break;
      case set0_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(0, _D)); break;
      case set0_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(0, _E)); break;
      case set0_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(0, _H)); break;
      case set0_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(0, _L)); break;
      case set0_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(0, b)); } break;
      case set1_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(1, _A)); break;
      case set1_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(1, _B)); break;
      case set1_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(1, _C)); break;
      case set1_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(1, _D)); break;
      case set1_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(1, _E)); break;
      case set1_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(1, _H)); break;
      case set1_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(1, _L)); break;
      case set1_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(1, b)); } break;
      case set2_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(2, _A)); break;
      case set2_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(2, _B)); break;
      case set2_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(2, _C)); break;
      case set2_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(2, _D)); break;
      case set2_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(2, _E)); break;
      case set2_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(2, _H)); break;
      case set2_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(2, _L)); break;
      case set2_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(2, b)); } break;
      case set3_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(3, _A)); break;
      case set3_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(3, _B)); break;
      case set3_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(3, _C)); break;
      case set3_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(3, _D)); break;
      case set3_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(3, _E)); break;
      case set3_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(3, _H)); break;
      case set3_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(3, _L)); break;
      case set3_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(3, b)); } break;
      case set4_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(4, _A)); break;
      case set4_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(4, _B)); break;
      case set4_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(4, _C)); break;
      case set4_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(4, _D)); break;
      case set4_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(4, _E)); break;
      case set4_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(4, _H)); break;
      case set4_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(4, _L)); break;
      case set4_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(4, b)); } break;
      case set5_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(5, _A)); break;
      case set5_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(5, _B)); break;
      case set5_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(5, _C)); break;
      case set5_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(5, _D)); break;
      case set5_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(5, _E)); break;
      case set5_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(5, _H)); break;
      case set5_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(5, _L)); break;
      case set5_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(5, b)); } break;
      case set6_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(6, _A)); break;
      case set6_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(6, _B)); break;
      case set6_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(6, _C)); break;
      case set6_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(6, _D)); break;
      case set6_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(6, _E)); break;
      case set6_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(6, _H)); break;
      case set6_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(6, _L)); break;
      case set6_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(6, b)); } break;
      case set7_a:      _A = read_mem(_IX+o); write_mem(_IX+o, _A = SET(7, _A)); break;
      case set7_b:      _B = read_mem(_IX+o); write_mem(_IX+o, _B = SET(7, _B)); break;
      case set7_c:      _C = read_mem(_IX+o); write_mem(_IX+o, _C = SET(7, _C)); break;
      case set7_d:      _D = read_mem(_IX+o); write_mem(_IX+o, _D = SET(7, _D)); break;
      case set7_e:      _E = read_mem(_IX+o); write_mem(_IX+o, _E = SET(7, _E)); break;
      case set7_h:      _H = read_mem(_IX+o); write_mem(_IX+o, _H = SET(7, _H)); break;
      case set7_l:      _L = read_mem(_IX+o); write_mem(_IX+o, _L = SET(7, _L)); break;
      case set7_mhl:    { byte b = read_mem(_IX+o); write_mem(_IX+o, SET(7, b)); } break;
      case sla_a:       _A = read_mem(_IX+o); _A = SLA(_A); write_mem(_IX+o, _A); break;
      case sla_b:       _B = read_mem(_IX+o); _B = SLA(_B); write_mem(_IX+o, _B); break;
      case sla_c:       _C = read_mem(_IX+o); _C = SLA(_C); write_mem(_IX+o, _C); break;
      case sla_d:       _D = read_mem(_IX+o); _D = SLA(_D); write_mem(_IX+o, _D); break;
      case sla_e:       _E = read_mem(_IX+o); _E = SLA(_E); write_mem(_IX+o, _E); break;
      case sla_h:       _H = read_mem(_IX+o); _H = SLA(_H); write_mem(_IX+o, _H); break;
      case sla_l:       _L = read_mem(_IX+o); _L = SLA(_L); write_mem(_IX+o, _L); break;
      case sla_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, SLA(b)); } break;
      case sll_a:       _A = read_mem(_IX+o); _A = SLL(_A); write_mem(_IX+o, _A); break;
      case sll_b:       _B = read_mem(_IX+o); _B = SLL(_B); write_mem(_IX+o, _B); break;
      case sll_c:       _C = read_mem(_IX+o); _C = SLL(_C); write_mem(_IX+o, _C); break;
      case sll_d:       _D = read_mem(_IX+o); _D = SLL(_D); write_mem(_IX+o, _D); break;
      case sll_e:       _E = read_mem(_IX+o); _E = SLL(_E); write_mem(_IX+o, _E); break;
      case sll_h:       _H = read_mem(_IX+o); _H = SLL(_H); write_mem(_IX+o, _H); break;
      case sll_l:       _L = read_mem(_IX+o); _L = SLL(_L); write_mem(_IX+o, _L); break;
      case sll_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, SLL(b)); } break;
      case sra_a:       _A = read_mem(_IX+o); _A = SRA(_A); write_mem(_IX+o, _A); break;
      case sra_b:       _B = read_mem(_IX+o); _B = SRA(_B); write_mem(_IX+o, _B); break;
      case sra_c:       _C = read_mem(_IX+o); _C = SRA(_C); write_mem(_IX+o, _C); break;
      case sra_d:       _D = read_mem(_IX+o); _D = SRA(_D); write_mem(_IX+o, _D); break;
      case sra_e:       _E = read_mem(_IX+o); _E = SRA(_E); write_mem(_IX+o, _E); break;
      case sra_h:       _H = read_mem(_IX+o); _H = SRA(_H); write_mem(_IX+o, _H); break;
      case sra_l:       _L = read_mem(_IX+o); _L = SRA(_L); write_mem(_IX+o, _L); break;
      case sra_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, SRA(b)); } break;
      case srl_a:       _A = read_mem(_IX+o); _A = SRL(_A); write_mem(_IX+o, _A); break;
      case srl_b:       _B = read_mem(_IX+o); _B = SRL(_B); write_mem(_IX+o, _B); break;
      case srl_c:       _C = read_mem(_IX+o); _C = SRL(_C); write_mem(_IX+o, _C); break;
      case srl_d:       _D = read_mem(_IX+o); _D = SRL(_D); write_mem(_IX+o, _D); break;
      case srl_e:       _E = read_mem(_IX+o); _E = SRL(_E); write_mem(_IX+o, _E); break;
      case srl_h:       _H = read_mem(_IX+o); _H = SRL(_H); write_mem(_IX+o, _H); break;
      case srl_l:       _L = read_mem(_IX+o); _L = SRL(_L); write_mem(_IX+o, _L); break;
      case srl_mhl:     { byte b = read_mem(_IX+o); write_mem(_IX+o, SRL(b)); } break;
   }
}
# endif

# ifdef Z80_JMP_TBL
void z80_pfx_ed(void)
{
   __label__
   ed_00, ed_01, ed_02, ed_03, ed_04, ed_05, ed_06, ed_07,
   ed_08, ed_09, ed_0a, ed_0b, ed_0c, ed_0d, ed_0e, ed_0f,
   ed_10, ed_11, ed_12, ed_13, ed_14, ed_15, ed_16, ed_17,
   ed_18, ed_19, ed_1a, ed_1b, ed_1c, ed_1d, ed_1e, ed_1f,
   ed_20, ed_21, ed_22, ed_23, ed_24, ed_25, ed_26, ed_27,
   ed_28, ed_29, ed_2a, ed_2b, ed_2c, ed_2d, ed_2e, ed_2f,
   ed_30, ed_31, ed_32, ed_33, ed_34, ed_35, ed_36, ed_37,
   ed_38, ed_39, ed_3a, ed_3b, ed_3c, ed_3d, ed_3e, ed_3f,
   in_b_c, out_c_b, sbc_hl_bc, ld_EDmword_bc, neg, retn, im_0, ld_i_a,
   in_c_c, out_c_c, adc_hl_bc, ld_EDbc_mword, neg_1, reti, im_0_1, ld_r_a,
   in_d_c, out_c_d, sbc_hl_de, ld_EDmword_de, neg_2, retn_1, im_1, ld_a_i,
   in_e_c, out_c_e, adc_hl_de, ld_EDde_mword, neg_3, reti_1, im_2, ld_a_r,
   in_h_c, out_c_h, sbc_hl_hl, ld_EDmword_hl, neg_4, retn_2, im_0_2, rrd,
   in_l_c, out_c_l, adc_hl_hl, ld_EDhl_mword, neg_5, reti_2, im_0_3, rld,
   in_0_c, out_c_0, sbc_hl_sp, ld_EDmword_sp, neg_6, retn_3, im_1_1, ed_77,
   in_a_c, out_c_a, adc_hl_sp, ld_EDsp_mword, neg_7, reti_3, im_2_1, ed_7f,
   ed_80, ed_81, ed_82, ed_83, ed_84, ed_85, ed_86, ed_87,
   ed_88, ed_89, ed_8a, ed_8b, ed_8c, ed_8d, ed_8e, ed_8f,
   ed_90, ed_91, ed_92, ed_93, ed_94, ed_95, ed_96, ed_97,
   ed_98, ed_99, ed_9a, ed_9b, ed_9c, ed_9d, ed_9e, ed_9f,
   ldi, cpi, ini, outi, ed_a4, ed_a5, ed_a6, ed_a7,
   ldd, cpd, ind, outd, ed_ac, ed_ad, ed_ae, ed_af,
   ldir, cpir, inir, otir, ed_b4, ed_b5, ed_b6, ed_b7,
   lddr, cpdr, indr, otdr, ed_bc, ed_bd, ed_be, ed_bf,
   ed_c0, ed_c1, ed_c2, ed_c3, ed_c4, ed_c5, ed_c6, ed_c7,
   ed_c8, ed_c9, ed_ca, ed_cb, ed_cc, ed_cd, ed_ce, ed_cf,
   ed_d0, ed_d1, ed_d2, ed_d3, ed_d4, ed_d5, ed_d6, ed_d7,
   ed_d8, ed_d9, ed_da, ed_db, ed_dc, ed_dd, ed_de, ed_df,
   ed_e0, ed_e1, ed_e2, ed_e3, ed_e4, ed_e5, ed_e6, ed_e7,
   ed_e8, ed_e9, ed_ea, ed_eb, ed_ec, ed_ed, ed_ee, ed_ef,
   ed_f0, ed_f1, ed_f2, ed_f3, ed_f4, ed_f5, ed_f6, ed_f7,
   ed_f8, ed_f9, ed_fa, ed_fb, ed_fc, ed_fd, ed_fe, ed_ff;

    static const void* const a_jump_table[256] = 
	  {
   && ed_00,&& ed_01,&& ed_02,&& ed_03,&& ed_04,&& ed_05,&& ed_06,&& ed_07,&&
   ed_08,&& ed_09,&& ed_0a,&& ed_0b,&& ed_0c,&& ed_0d,&& ed_0e,&& ed_0f,&&
   ed_10,&& ed_11,&& ed_12,&& ed_13,&& ed_14,&& ed_15,&& ed_16,&& ed_17,&&
   ed_18,&& ed_19,&& ed_1a,&& ed_1b,&& ed_1c,&& ed_1d,&& ed_1e,&& ed_1f,&&
   ed_20,&& ed_21,&& ed_22,&& ed_23,&& ed_24,&& ed_25,&& ed_26,&& ed_27,&&
   ed_28,&& ed_29,&& ed_2a,&& ed_2b,&& ed_2c,&& ed_2d,&& ed_2e,&& ed_2f,&&
   ed_30,&& ed_31,&& ed_32,&& ed_33,&& ed_34,&& ed_35,&& ed_36,&& ed_37,&&
   ed_38,&& ed_39,&& ed_3a,&& ed_3b,&& ed_3c,&& ed_3d,&& ed_3e,&& ed_3f,&&
   in_b_c,&& out_c_b,&& sbc_hl_bc,&& ld_EDmword_bc,&& neg,&& retn,&& im_0,&& ld_i_a,&&
   in_c_c,&& out_c_c,&& adc_hl_bc,&& ld_EDbc_mword,&& neg_1,&& reti,&& im_0_1,&& ld_r_a,&&
   in_d_c,&& out_c_d,&& sbc_hl_de,&& ld_EDmword_de,&& neg_2,&& retn_1,&& im_1,&& ld_a_i,&&
   in_e_c,&& out_c_e,&& adc_hl_de,&& ld_EDde_mword,&& neg_3,&& reti_1,&& im_2,&& ld_a_r,&&
   in_h_c,&& out_c_h,&& sbc_hl_hl,&& ld_EDmword_hl,&& neg_4,&& retn_2,&& im_0_2,&& rrd,&&
   in_l_c,&& out_c_l,&& adc_hl_hl,&& ld_EDhl_mword,&& neg_5,&& reti_2,&& im_0_3,&& rld,&&
   in_0_c,&& out_c_0,&& sbc_hl_sp,&& ld_EDmword_sp,&& neg_6,&& retn_3,&& im_1_1,&& ed_77,&&
   in_a_c,&& out_c_a,&& adc_hl_sp,&& ld_EDsp_mword,&& neg_7,&& reti_3,&& im_2_1,&& ed_7f,&&
   ed_80,&& ed_81,&& ed_82,&& ed_83,&& ed_84,&& ed_85,&& ed_86,&& ed_87,&&
   ed_88,&& ed_89,&& ed_8a,&& ed_8b,&& ed_8c,&& ed_8d,&& ed_8e,&& ed_8f,&&
   ed_90,&& ed_91,&& ed_92,&& ed_93,&& ed_94,&& ed_95,&& ed_96,&& ed_97,&&
   ed_98,&& ed_99,&& ed_9a,&& ed_9b,&& ed_9c,&& ed_9d,&& ed_9e,&& ed_9f,&&
   ldi,&& cpi,&& ini,&& outi,&& ed_a4,&& ed_a5,&& ed_a6,&& ed_a7,&&
   ldd,&& cpd,&& ind,&& outd,&& ed_ac,&& ed_ad,&& ed_ae,&& ed_af,&&
   ldir,&& cpir,&& inir,&& otir,&& ed_b4,&& ed_b5,&& ed_b6,&& ed_b7,&&
   lddr,&& cpdr,&& indr,&& otdr,&& ed_bc,&& ed_bd,&& ed_be,&& ed_bf,&&
   ed_c0,&& ed_c1,&& ed_c2,&& ed_c3,&& ed_c4,&& ed_c5,&& ed_c6,&& ed_c7,&&
   ed_c8,&& ed_c9,&& ed_ca,&& ed_cb,&& ed_cc,&& ed_cd,&& ed_ce,&& ed_cf,&&
   ed_d0,&& ed_d1,&& ed_d2,&& ed_d3,&& ed_d4,&& ed_d5,&& ed_d6,&& ed_d7,&&
   ed_d8,&& ed_d9,&& ed_da,&& ed_db,&& ed_dc,&& ed_dd,&& ed_de,&& ed_df,&&
   ed_e0,&& ed_e1,&& ed_e2,&& ed_e3,&& ed_e4,&& ed_e5,&& ed_e6,&& ed_e7,&&
   ed_e8,&& ed_e9,&& ed_ea,&& ed_eb,&& ed_ec,&& ed_ed,&& ed_ee,&& ed_ef,&&
   ed_f0,&& ed_f1,&& ed_f2,&& ed_f3,&& ed_f4,&& ed_f5,&& ed_f6,&& ed_f7,&&
   ed_f8,&& ed_f9,&& ed_fa,&& ed_fb,&& ed_fc,&& ed_fd,&& ed_fe,&& ed_ff
   };
   byte bOpCode;

   bOpCode = read_mem(_PC++);
   iCycleCount += cc_ed[bOpCode];
   _R++;
	 goto *a_jump_table[bOpCode];
   //switch(bOpCode)
   //{
      adc_hl_bc:   ADC16(BC); return;
      adc_hl_de:   ADC16(DE); return;
      adc_hl_hl:   ADC16(HL); return;
      adc_hl_sp:   ADC16(SP); return;
      cpd:         CPD; return;
      cpdr:        CPDR; return;
      cpi:         CPI; return;
      cpir:        CPIR; return;
      ed_00: 
      ed_01: 
      ed_02: 
      ed_03: 
      ed_04: 
      ed_05: 
      ed_06: 
      ed_07: 
      ed_08: 
      ed_09: 
      ed_0a: 
      ed_0b: 
      ed_0c: 
      ed_0d: 
      ed_0e: 
      ed_0f: 
      ed_10: 
      ed_11: 
      ed_12: 
      ed_13: 
      ed_14: 
      ed_15: 
      ed_16: 
      ed_17: 
      ed_18: 
      ed_19: 
      ed_1a: 
      ed_1b: 
      ed_1c: 
      ed_1d: 
      ed_1e: 
      ed_1f: 
      ed_20: 
      ed_21: 
      ed_22: 
      ed_23: 
      ed_24: 
      ed_25: 
      ed_26: 
      ed_27: 
      ed_28: 
      ed_29: 
      ed_2a: 
      ed_2b: 
      ed_2c: 
      ed_2d: 
      ed_2e: 
      ed_2f: 
      ed_30: 
      ed_31: 
      ed_32: 
      ed_33: 
      ed_34: 
      ed_35: 
      ed_36: 
      ed_37: 
      ed_38: 
      ed_39: 
      ed_3a: 
      ed_3b: 
      ed_3c: 
      ed_3d: 
      ed_3e: 
      ed_3f: 
      ed_77: 
      ed_7f: 
      ed_80: 
      ed_81: 
      ed_82: 
      ed_83: 
      ed_84: 
      ed_85: 
      ed_86: 
      ed_87: 
      ed_88: 
      ed_89: 
      ed_8a: 
      ed_8b: 
      ed_8c: 
      ed_8d: 
      ed_8e: 
      ed_8f: 
      ed_90: 
      ed_91: 
      ed_92: 
      ed_93: 
      ed_94: 
      ed_95: 
      ed_96: 
      ed_97: 
      ed_98: 
      ed_99: 
      ed_9a: 
      ed_9b: 
      ed_9c: 
      ed_9d: 
      ed_9e: 
      ed_9f: 
      ed_a4: 
      ed_a5: 
      ed_a6: 
      ed_a7: 
      ed_ac: 
      ed_ad: 
      ed_ae: 
      ed_af: 
      ed_b4: 
      ed_b5: 
      ed_b6: 
      ed_b7: 
      ed_bc: 
      ed_bd: 
      ed_be: 
      ed_bf: 
      ed_c0: 
      ed_c1: 
      ed_c2: 
      ed_c3: 
      ed_c4: 
      ed_c5: 
      ed_c6: 
      ed_c7: 
      ed_c8: 
      ed_c9: 
      ed_ca: 
      ed_cb: 
      ed_cc: 
      ed_cd: 
      ed_ce: 
      ed_cf: 
      ed_d0: 
      ed_d1: 
      ed_d2: 
      ed_d3: 
      ed_d4: 
      ed_d5: 
      ed_d6: 
      ed_d7: 
      ed_d8: 
      ed_d9: 
      ed_da: 
      ed_db: 
      ed_dc: 
      ed_dd: 
      ed_de: 
      ed_df: 
      ed_e0: 
      ed_e1: 
      ed_e2: 
      ed_e3: 
      ed_e4: 
      ed_e5: 
      ed_e6: 
      ed_e7: 
      ed_e8: 
      ed_e9: 
      ed_ea: 
      ed_eb: 
      ed_ec: 
      ed_ed: 
      ed_ee: 
      ed_ef: 
      ed_f0: 
      ed_f1: 
      ed_f2: 
      ed_f3: 
      ed_f4: 
      ed_f5: 
      ed_f6: 
      ed_f7: 
      ed_f8: 
      ed_f9: 
      ed_fa: 
      ed_fb: 
      ed_fc: 
      ed_fd: 
      ed_fe: 
      ed_ff:       return;
      im_0:     
      im_0_1:   
      im_0_2:  
      im_0_3:      _IM = 0; return;
      im_1:   
      im_1_1:      _IM = 1; return;
      im_2:   
      im_2_1:      _IM = 2; return;
      ind:         { z80_wait_states iCycleCount = Iy_;} IND; return;
      indr:        { z80_wait_states iCycleCount = Iy_;} INDR; return;
      ini:         { z80_wait_states iCycleCount = Iy_;} INI; return;
      inir:        { z80_wait_states iCycleCount = Iy_;} INIR; return;
      in_0_c:      { z80_wait_states iCycleCount = Ix_;} { byte res = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[res]; } return;
      in_a_c:      { z80_wait_states iCycleCount = Ix_;} _A = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_A]; return;
      in_b_c:      { z80_wait_states iCycleCount = Ix_;} _B = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_B]; return;
      in_c_c:      { z80_wait_states iCycleCount = Ix_;} _C = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_C]; return;
      in_d_c:      { z80_wait_states iCycleCount = Ix_;} _D = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_D]; return;
      in_e_c:      { z80_wait_states iCycleCount = Ix_;} _E = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_E]; return;
      in_h_c:      { z80_wait_states iCycleCount = Ix_;} _H = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_H]; return;
      in_l_c:      { z80_wait_states iCycleCount = Ix_;} _L = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_L]; return;
      ldd:         LDD; iWSAdjust++; return;
      lddr:        LDDR; iWSAdjust++; return;
      ldi:         LDI; iWSAdjust++; return;
      ldir:        LDIR; iWSAdjust++; return;
      ld_a_i:      _A = _I; _F = (_F & Cflag) | SZ[_A] | _IFF2; iWSAdjust++; return;
      ld_a_r:      _A = (_R & 0x7f) | _Rb7; _F = (_F & Cflag) | SZ[_A] | _IFF2; iWSAdjust++; return;
      ld_EDbc_mword:  LD16_MEM(BC); return;
      ld_EDde_mword:  LD16_MEM(DE); return;
      ld_EDhl_mword:  LD16_MEM(HL); return;
      ld_EDmword_bc:  LDMEM_16(BC); return;
      ld_EDmword_de:  LDMEM_16(DE); return;
      ld_EDmword_hl:  LDMEM_16(HL); return;
      ld_EDmword_sp:  LDMEM_16(SP); return;
      ld_EDsp_mword:  LD16_MEM(SP); return;
      ld_i_a:      _I = _A; iWSAdjust++; return;
      ld_r_a:      _R = _A; _Rb7 = _A & 0x80; iWSAdjust++; return;
      neg:      
      neg_1:   
      neg_2:  
      neg_3: 
      neg_4:
      neg_5:   
      neg_6:  
      neg_7:       NEG; return;
      otdr:        { z80_wait_states iCycleCount = Oy_;} OTDR; return;
      otir:        { z80_wait_states iCycleCount = Oy_;} OTIR; return;
      outd:        { z80_wait_states iCycleCount = Oy_;} OUTD; return;
      outi:        { z80_wait_states iCycleCount = Oy_;} OUTI; return;
      out_c_0:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, 0); return;
      out_c_a:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _A); return;
      out_c_b:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _B); return;
      out_c_c:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _C); return;
      out_c_d:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _D); return;
      out_c_e:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _E); return;
      out_c_h:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _H); return;
      out_c_l:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _L); return;
      reti:      
      reti_1:   
      reti_2:  
      reti_3: 
      retn:  
      retn_1:  
      retn_2: 
      retn_3:      _IFF1 = _IFF2; RET; return;
      rld:         RLD; return;
      rrd:         RRD; return;
      sbc_hl_bc:   SBC16(BC); return;
      sbc_hl_de:   SBC16(DE); return;
      sbc_hl_hl:   SBC16(HL); return;
      sbc_hl_sp:   SBC16(SP); return;
   //}
}
# else
void z80_pfx_ed(void)
{
   byte bOpCode;

   bOpCode = read_mem(_PC++);
   iCycleCount += cc_ed[bOpCode];
   _R++;
   switch(bOpCode)
   {
      case adc_hl_bc:   ADC16(BC); break;
      case adc_hl_de:   ADC16(DE); break;
      case adc_hl_hl:   ADC16(HL); break;
      case adc_hl_sp:   ADC16(SP); break;
      case cpd:         CPD; break;
      case cpdr:        CPDR; break;
      case cpi:         CPI; break;
      case cpir:        CPIR; break;
      case im_0:     
      case im_0_1:   
      case im_0_2:  
      case im_0_3:      _IM = 0; break;
      case im_1:   
      case im_1_1:      _IM = 1; break;
      case im_2:  
      case im_2_1:      _IM = 2; break;
      case ind:         { z80_wait_states iCycleCount = Iy_;} IND; break;
      case indr:        { z80_wait_states iCycleCount = Iy_;} INDR; break;
      case ini:         { z80_wait_states iCycleCount = Iy_;} INI; break;
      case inir:        { z80_wait_states iCycleCount = Iy_;} INIR; break;
      case in_0_c:      { z80_wait_states iCycleCount = Ix_;} { byte res = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[res]; } break;
      case in_a_c:      { z80_wait_states iCycleCount = Ix_;} _A = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_A]; break;
      case in_b_c:      { z80_wait_states iCycleCount = Ix_;} _B = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_B]; break;
      case in_c_c:      { z80_wait_states iCycleCount = Ix_;} _C = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_C]; break;
      case in_d_c:      { z80_wait_states iCycleCount = Ix_;} _D = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_D]; break;
      case in_e_c:      { z80_wait_states iCycleCount = Ix_;} _E = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_E]; break;
      case in_h_c:      { z80_wait_states iCycleCount = Ix_;} _H = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_H]; break;
      case in_l_c:      { z80_wait_states iCycleCount = Ix_;} _L = z80_IN_handler(z80.BC); _F = (_F & Cflag) | SZP[_L]; break;
      case ldd:         LDD; iWSAdjust++; break;
      case lddr:        LDDR; iWSAdjust++; break;
      case ldi:         LDI; iWSAdjust++; break;
      case ldir:        LDIR; iWSAdjust++; break;
      case ld_a_i:      _A = _I; _F = (_F & Cflag) | SZ[_A] | _IFF2; iWSAdjust++; break;
      case ld_a_r:      _A = (_R & 0x7f) | _Rb7; _F = (_F & Cflag) | SZ[_A] | _IFF2; iWSAdjust++; break;
      case ld_EDbc_mword:  LD16_MEM(BC); break;
      case ld_EDde_mword:  LD16_MEM(DE); break;
      case ld_EDhl_mword:  LD16_MEM(HL); break;
      case ld_EDmword_bc:  LDMEM_16(BC); break;
      case ld_EDmword_de:  LDMEM_16(DE); break;
      case ld_EDmword_hl:  LDMEM_16(HL); break;
      case ld_EDmword_sp:  LDMEM_16(SP); break;
      case ld_EDsp_mword:  LD16_MEM(SP); break;
      case ld_i_a:      _I = _A; iWSAdjust++; break;
      case ld_r_a:      _R = _A; _Rb7 = _A & 0x80; iWSAdjust++; break;
      case neg:        
      case neg_1:     
      case neg_2:    
      case neg_3:   
      case neg_4:  
      case neg_5: 
      case neg_6:
      case neg_7:       NEG; break;
      case otdr:        { z80_wait_states iCycleCount = Oy_;} OTDR; break;
      case otir:        { z80_wait_states iCycleCount = Oy_;} OTIR; break;
      case outd:        { z80_wait_states iCycleCount = Oy_;} OUTD; break;
      case outi:        { z80_wait_states iCycleCount = Oy_;} OUTI; break;
      case out_c_0:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, 0); break;
      case out_c_a:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _A); break;
      case out_c_b:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _B); break;
      case out_c_c:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _C); break;
      case out_c_d:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _D); break;
      case out_c_e:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _E); break;
      case out_c_h:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _H); break;
      case out_c_l:     { z80_wait_states iCycleCount = Ox_;} z80_OUT_handler(z80.BC, _L); break;
      case reti:      
      case reti_1:   
      case reti_2:  
      case reti_3: 
      case retn:  
      case retn_1:  
      case retn_2: 
      case retn_3:      _IFF1 = _IFF2; RET; break;
      case rld:         RLD; break;
      case rrd:         RRD; break;
      case sbc_hl_bc:   SBC16(BC); break;
      case sbc_hl_de:   SBC16(DE); break;
      case sbc_hl_hl:   SBC16(HL); break;
      case sbc_hl_sp:   SBC16(SP); break;
   }
}
# endif

# ifdef Z80_JMP_TBL
void z80_pfx_fd(void)
{
    __label__ 
   nop, ld_bc_word, ld_mbc_a, inc_bc, inc_b, dec_b, ld_b_byte, rlca,
   ex_af_af, add_hl_bc, ld_a_mbc, dec_bc, inc_c, dec_c, ld_c_byte, rrca,
   djnz, ld_de_word, ld_mde_a, inc_de, inc_d, dec_d, ld_d_byte, rla,
   jr, add_hl_de, ld_a_mde, dec_de, inc_e, dec_e, ld_e_byte, rra,
   jr_nz, ld_hl_word, ld_mword_hl, inc_hl, inc_h, dec_h, ld_h_byte, daa,
   jr_z, add_hl_hl, ld_hl_mword, dec_hl, inc_l, dec_l, ld_l_byte, cpl,
   jr_nc, ld_sp_word, ld_mword_a, inc_sp, inc_mhl, dec_mhl, ld_mhl_byte, scf,
   jr_c, add_hl_sp, ld_a_mword, dec_sp, inc_a, dec_a, ld_a_byte, ccf,
   ld_b_b, ld_b_c, ld_b_d, ld_b_e, ld_b_h, ld_b_l, ld_b_mhl, ld_b_a,
   ld_c_b, ld_c_c, ld_c_d, ld_c_e, ld_c_h, ld_c_l, ld_c_mhl, ld_c_a,
   ld_d_b, ld_d_c, ld_d_d, ld_d_e, ld_d_h, ld_d_l, ld_d_mhl, ld_d_a,
   ld_e_b, ld_e_c, ld_e_d, ld_e_e, ld_e_h, ld_e_l, ld_e_mhl, ld_e_a,
   ld_h_b, ld_h_c, ld_h_d, ld_h_e, ld_h_h, ld_h_l, ld_h_mhl, ld_h_a,
   ld_l_b, ld_l_c, ld_l_d, ld_l_e, ld_l_h, ld_l_l, ld_l_mhl, ld_l_a,
   ld_mhl_b, ld_mhl_c, ld_mhl_d, ld_mhl_e, ld_mhl_h, ld_mhl_l, halt, ld_mhl_a,
   ld_a_b, ld_a_c, ld_a_d, ld_a_e, ld_a_h, ld_a_l, ld_a_mhl, ld_a_a,
   add_b, add_c, add_d, add_e, add_h, add_l, add_mhl, add_a,
   adc_b, adc_c, adc_d, adc_e, adc_h, adc_l, adc_mhl, adc_a,
   sub_b, sub_c, sub_d, sub_e, sub_h, sub_l, sub_mhl, sub_a,
   sbc_b, sbc_c, sbc_d, sbc_e, sbc_h, sbc_l, sbc_mhl, sbc_a,
   and_b, and_c, and_d, and_e, and_h, and_l, and_mhl, and_a,
   xor_b, xor_c, xor_d, xor_e, xor_h, xor_l, xor_mhl, xor_a,
   or_b, or_c, or_d, or_e, or_h, or_l, or_mhl, or_a,
   cp_b, cp_c, cp_d, cp_e, cp_h, cp_l, cp_mhl, cp_a,
   ret_nz, pop_bc, jp_nz, jp, call_nz, push_bc, add_byte, rst00,
   ret_z, ret, jp_z, pfx_cb, call_z, call, adc_byte, rst08,
   ret_nc, pop_de, jp_nc, outa, call_nc, push_de, sub_byte, rst10,
   ret_c, exx, jp_c, ina, call_c, pfx_dd, sbc_byte, rst18,
   ret_po, pop_hl, jp_po, ex_msp_hl, call_po, push_hl, and_byte, rst20,
   ret_pe, ld_pc_hl, jp_pe, ex_de_hl, call_pe, pfx_ed, xor_byte, rst28,
   ret_p, pop_af, jp_p, di, call_p, push_af, or_byte, rst30,
   ret_m, ld_sp_hl, jp_m, ei, call_m, pfx_fd, cp_byte, rst38;

    static const void* const a_jump_table[256] = 
	  {
   && nop,&& ld_bc_word,&& ld_mbc_a,&& inc_bc,&& inc_b,&& dec_b,&& ld_b_byte,&& rlca,&&
   ex_af_af,&& add_hl_bc,&& ld_a_mbc,&& dec_bc,&& inc_c,&& dec_c,&& ld_c_byte,&& rrca,&&
   djnz,&& ld_de_word,&& ld_mde_a,&& inc_de,&& inc_d,&& dec_d,&& ld_d_byte,&& rla,&&
   jr,&& add_hl_de,&& ld_a_mde,&& dec_de,&& inc_e,&& dec_e,&& ld_e_byte,&& rra,&&
   jr_nz,&& ld_hl_word,&& ld_mword_hl,&& inc_hl,&& inc_h,&& dec_h,&& ld_h_byte,&& daa,&&
   jr_z,&& add_hl_hl,&& ld_hl_mword,&& dec_hl,&& inc_l,&& dec_l,&& ld_l_byte,&& cpl,&&
   jr_nc,&& ld_sp_word,&& ld_mword_a,&& inc_sp,&& inc_mhl,&& dec_mhl,&& ld_mhl_byte,&& scf,&&
   jr_c,&& add_hl_sp,&& ld_a_mword,&& dec_sp,&& inc_a,&& dec_a,&& ld_a_byte,&& ccf,&&
   ld_b_b,&& ld_b_c,&& ld_b_d,&& ld_b_e,&& ld_b_h,&& ld_b_l,&& ld_b_mhl,&& ld_b_a,&&
   ld_c_b,&& ld_c_c,&& ld_c_d,&& ld_c_e,&& ld_c_h,&& ld_c_l,&& ld_c_mhl,&& ld_c_a,&&
   ld_d_b,&& ld_d_c,&& ld_d_d,&& ld_d_e,&& ld_d_h,&& ld_d_l,&& ld_d_mhl,&& ld_d_a,&&
   ld_e_b,&& ld_e_c,&& ld_e_d,&& ld_e_e,&& ld_e_h,&& ld_e_l,&& ld_e_mhl,&& ld_e_a,&&
   ld_h_b,&& ld_h_c,&& ld_h_d,&& ld_h_e,&& ld_h_h,&& ld_h_l,&& ld_h_mhl,&& ld_h_a,&&
   ld_l_b,&& ld_l_c,&& ld_l_d,&& ld_l_e,&& ld_l_h,&& ld_l_l,&& ld_l_mhl,&& ld_l_a,&&
   ld_mhl_b,&& ld_mhl_c,&& ld_mhl_d,&& ld_mhl_e,&& ld_mhl_h,&& ld_mhl_l,&& halt,&& ld_mhl_a,&&
   ld_a_b,&& ld_a_c,&& ld_a_d,&& ld_a_e,&& ld_a_h,&& ld_a_l,&& ld_a_mhl,&& ld_a_a,&&
   add_b,&& add_c,&& add_d,&& add_e,&& add_h,&& add_l,&& add_mhl,&& add_a,&&
   adc_b,&& adc_c,&& adc_d,&& adc_e,&& adc_h,&& adc_l,&& adc_mhl,&& adc_a,&&
   sub_b,&& sub_c,&& sub_d,&& sub_e,&& sub_h,&& sub_l,&& sub_mhl,&& sub_a,&&
   sbc_b,&& sbc_c,&& sbc_d,&& sbc_e,&& sbc_h,&& sbc_l,&& sbc_mhl,&& sbc_a,&&
   and_b,&& and_c,&& and_d,&& and_e,&& and_h,&& and_l,&& and_mhl,&& and_a,&&
   xor_b,&& xor_c,&& xor_d,&& xor_e,&& xor_h,&& xor_l,&& xor_mhl,&& xor_a,&&
   or_b,&& or_c,&& or_d,&& or_e,&& or_h,&& or_l,&& or_mhl,&& or_a,&&
   cp_b,&& cp_c,&& cp_d,&& cp_e,&& cp_h,&& cp_l,&& cp_mhl,&& cp_a,&&
   ret_nz,&& pop_bc,&& jp_nz,&& jp,&& call_nz,&& push_bc,&& add_byte,&& rst00,&&
   ret_z,&& ret,&& jp_z,&& pfx_cb,&& call_z,&& call,&& adc_byte,&& rst08,&&
   ret_nc,&& pop_de,&& jp_nc,&& outa,&& call_nc,&& push_de,&& sub_byte,&& rst10,&&
   ret_c,&& exx,&& jp_c,&& ina,&& call_c,&& pfx_dd,&& sbc_byte,&& rst18,&&
   ret_po,&& pop_hl,&& jp_po,&& ex_msp_hl,&& call_po,&& push_hl,&& and_byte,&& rst20,&&
   ret_pe,&& ld_pc_hl,&& jp_pe,&& ex_de_hl,&& call_pe,&& pfx_ed,&& xor_byte,&& rst28,&&
   ret_p,&& pop_af,&& jp_p,&& di,&& call_p,&& push_af,&& or_byte,&& rst30,&&
   ret_m,&& ld_sp_hl,&& jp_m,&& ei,&& call_m,&& pfx_fd,&& cp_byte,&& rst38
	  };

   byte bOpCode;

   bOpCode = read_mem(_PC++);
   iCycleCount += cc_xy[bOpCode];
   _R++;
	 goto *a_jump_table[bOpCode];
   //switch(bOpCode)
   //{
      adc_a:       ADC(_A); return;
      adc_b:       ADC(_B); return;
      adc_byte:    ADC(read_mem(_PC++)); return;
      adc_c:       ADC(_C); return;
      adc_d:       ADC(_D); return;
      adc_e:       ADC(_E); return;
      adc_h:       ADC(_IYh); return;
      adc_l:       ADC(_IYl); return;
      adc_mhl:     { signed char o = read_mem(_PC++); ADC(read_mem(_IY+o)); } return;
      add_a:       ADD(_A); return;
      add_b:       ADD(_B); return;
      add_byte:    ADD(read_mem(_PC++)); return;
      add_c:       ADD(_C); return;
      add_d:       ADD(_D); return;
      add_e:       ADD(_E); return;
      add_h:       ADD(_IYh); return;
      add_hl_bc:   ADD16(IY, BC); return;
      add_hl_de:   ADD16(IY, DE); return;
      add_hl_hl:   ADD16(IY, IY); return;
      add_hl_sp:   ADD16(IY, SP); return;
      add_l:       ADD(_IYl); return;
      add_mhl:     { signed char o = read_mem(_PC++); ADD(read_mem(_IY+o)); } return;
      and_a:       AND(_A); return;
      and_b:       AND(_B); return;
      and_byte:    AND(read_mem(_PC++)); return;
      and_c:       AND(_C); return;
      and_d:       AND(_D); return;
      and_e:       AND(_E); return;
      and_h:       AND(_IYh); return;
      and_l:       AND(_IYl); return;
      and_mhl:     { signed char o = read_mem(_PC++); AND(read_mem(_IY+o)); } return;
      call:        CALL; return;
      call_c:      if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_m:      if (_F & Sflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_nc:     if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_nz:     if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_p:      if (!(_F & Sflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_pe:     if (_F & Pflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_po:     if (!(_F & Pflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      call_z:      if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } return;
      ccf:         _F = ((_F & (Sflag | Zflag | Pflag | Cflag)) | ((_F & CF) << 4) | (_A & Xflags)) ^ CF; return;
      cpl:         _A ^= 0xff; _F = (_F & (Sflag | Zflag | Pflag | Cflag)) | Hflag | Nflag | (_A & Xflags); return;
      cp_a:        CP(_A); return;
      cp_b:        CP(_B); return;
      cp_byte:     CP(read_mem(_PC++)); return;
      cp_c:        CP(_C); return;
      cp_d:        CP(_D); return;
      cp_e:        CP(_E); return;
      cp_h:        CP(_IYh); return;
      cp_l:        CP(_IYl); return;
      cp_mhl:      { signed char o = read_mem(_PC++); CP(read_mem(_IY+o)); } return;
      daa:         DAA; return;
      dec_a:       DEC(_A); return;
      dec_b:       DEC(_B); return;
      dec_bc:      _BC--; iWSAdjust++; return;
      dec_c:       DEC(_C); return;
      dec_d:       DEC(_D); return;
      dec_de:      _DE--; iWSAdjust++; return;
      dec_e:       DEC(_E); return;
      dec_h:       DEC(_IYh); return;
      dec_hl:      _IY--; iWSAdjust++; return;
      dec_l:       DEC(_IYl); return;
      dec_mhl:     { signed char o = read_mem(_PC++); byte b = read_mem(_IY+o); DEC(b); write_mem(_IY+o, b); } return;
      dec_sp:      _SP--; iWSAdjust++; return;
      di:          _IFF1 = _IFF2 = 0; z80.EI_issued = 0; return;
      djnz:        if (--_B) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; } return;
      ei:          z80.EI_issued = 2; return;
      exx:         EXX; return;
      ex_af_af:    EX(z80.AF, z80.AFx); return;
      ex_de_hl:    EX(z80.DE, z80.HL); return;
      ex_msp_hl:   EX_SP(IY); iWSAdjust++; return;
      halt:        _HALT = 1; _PC--; return;
      ina:         { z80_wait_states iCycleCount = Ia_;} { reg_pair p; p.b.l = read_mem(_PC++); p.b.h = _A; _A = z80_IN_handler(p); } return;
      inc_a:       INC(_A); return;
      inc_b:       INC(_B); return;
      inc_bc:      _BC++; iWSAdjust++; return;
      inc_c:       INC(_C); return;
      inc_d:       INC(_D); return;
      inc_de:      _DE++; iWSAdjust++; return;
      inc_e:       INC(_E); return;
      inc_h:       INC(_IYh); return;
      inc_hl:      _IY++; iWSAdjust++; return;
      inc_l:       INC(_IYl); return;
      inc_mhl:     { signed char o = read_mem(_PC++); byte b = read_mem(_IY+o); INC(b); write_mem(_IY+o, b); } return;
      inc_sp:      _SP++; iWSAdjust++; return;
      jp:          JP; return;
      jp_c:        if (_F & Cflag) { JP } else { _PC += 2; }; return;
      jp_m:        if (_F & Sflag) { JP } else { _PC += 2; }; return;
      jp_nc:       if (!(_F & Cflag)) { JP } else { _PC += 2; }; return;
      jp_nz:       if (!(_F & Zflag)) { JP } else { _PC += 2; }; return;
      jp_p:        if (!(_F & Sflag)) { JP } else { _PC += 2; }; return;
      jp_pe:       if (_F & Pflag) { JP } else { _PC += 2; }; return;
      jp_po:       if (!(_F & Pflag)) { JP } else { _PC += 2; }; return;
      jp_z:        if (_F & Zflag) { JP } else { _PC += 2; }; return;
      jr:          JR; return;
      jr_c:        if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; return;
      jr_nc:       if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; return;
      jr_nz:       if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; return;
      jr_z:        if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; return;
      ld_a_a:      return;
      ld_a_b:      _A = _B; return;
      ld_a_byte:   _A = read_mem(_PC++); return;
      ld_a_c:      _A = _C; return;
      ld_a_d:      _A = _D; return;
      ld_a_e:      _A = _E; return;
      ld_a_h:      _A = _IYh; return;
      ld_a_l:      _A = _IYl; return;
      ld_a_mbc:    _A = read_mem(_BC); return;
      ld_a_mde:    _A = read_mem(_DE); return;
      ld_a_mhl:    { signed char o = read_mem(_PC++); _A = read_mem(_IY+o); } return;
      ld_a_mword:  { reg_pair addr; addr.b.l = read_mem(_PC++); addr.b.h = read_mem(_PC++); _A = read_mem(addr.w.l); } return;
      ld_bc_word:  z80.BC.b.l = read_mem(_PC++); z80.BC.b.h = read_mem(_PC++); return;
      ld_b_a:      _B = _A; return;
      ld_b_b:      return;
      ld_b_byte:   _B = read_mem(_PC++); return;
      ld_b_c:      _B = _C; return;
      ld_b_d:      _B = _D; return;
      ld_b_e:      _B = _E; return;
      ld_b_h:      _B = _IYh; return;
      ld_b_l:      _B = _IYl; return;
      ld_b_mhl:    { signed char o = read_mem(_PC++); _B = read_mem(_IY+o); } return;
      ld_c_a:      _C = _A; return;
      ld_c_b:      _C = _B; return;
      ld_c_byte:   _C = read_mem(_PC++); return;
      ld_c_c:      return;
      ld_c_d:      _C = _D; return;
      ld_c_e:      _C = _E; return;
      ld_c_h:      _C = _IYh; return;
      ld_c_l:      _C = _IYl; return;
      ld_c_mhl:    { signed char o = read_mem(_PC++); _C = read_mem(_IY+o); } return;
      ld_de_word:  z80.DE.b.l = read_mem(_PC++); z80.DE.b.h = read_mem(_PC++); return;
      ld_d_a:      _D = _A; return;
      ld_d_b:      _D = _B; return;
      ld_d_byte:   _D = read_mem(_PC++); return;
      ld_d_c:      _D = _C; return;
      ld_d_d:      return;
      ld_d_e:      _D = _E; return;
      ld_d_h:      _D = _IYh; return;
      ld_d_l:      _D = _IYl; return;
      ld_d_mhl:    { signed char o = read_mem(_PC++); _D = read_mem(_IY+o); } return;
      ld_e_a:      _E = _A; return;
      ld_e_b:      _E = _B; return;
      ld_e_byte:   _E = read_mem(_PC++); return;
      ld_e_c:      _E = _C; return;
      ld_e_d:      _E = _D; return;
      ld_e_e:      return;
      ld_e_h:      _E = _IYh; return;
      ld_e_l:      _E = _IYl; return;
      ld_e_mhl:    { signed char o = read_mem(_PC++); _E = read_mem(_IY+o); } return;
      ld_hl_mword: LD16_MEM(IY); return;
      ld_hl_word:  z80.IY.b.l = read_mem(_PC++); z80.IY.b.h = read_mem(_PC++); return;
      ld_h_a:      _IYh = _A; return;
      ld_h_b:      _IYh = _B; return;
      ld_h_byte:   _IYh = read_mem(_PC++); return;
      ld_h_c:      _IYh = _C; return;
      ld_h_d:      _IYh = _D; return;
      ld_h_e:      _IYh = _E; return;
      ld_h_h:      return;
      ld_h_l:      _IYh = _IYl; return;
      ld_h_mhl:    { signed char o = read_mem(_PC++); _H = read_mem(_IY+o); } return;
      ld_l_a:      _IYl = _A; return;
      ld_l_b:      _IYl = _B; return;
      ld_l_byte:   _IYl = read_mem(_PC++); return;
      ld_l_c:      _IYl = _C; return;
      ld_l_d:      _IYl = _D; return;
      ld_l_e:      _IYl = _E; return;
      ld_l_h:      _IYl = _IYh; return;
      ld_l_l:      return;
      ld_l_mhl:    { signed char o = read_mem(_PC++); _L = read_mem(_IY+o); } return;
      ld_mbc_a:    write_mem(_BC, _A); return;
      ld_mde_a:    write_mem(_DE, _A); return;
      ld_mhl_a:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _A); } return;
      ld_mhl_b:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _B); } return;
      ld_mhl_byte: { signed char o = read_mem(_PC++); byte b = read_mem(_PC++); write_mem(_IY+o, b); } return;
      ld_mhl_c:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _C); } return;
      ld_mhl_d:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _D); } return;
      ld_mhl_e:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _E); } return;
      ld_mhl_h:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _H); } return;
      ld_mhl_l:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _L); } return;
      ld_mword_a:  { reg_pair addr; addr.b.l = read_mem(_PC++); addr.b.h = read_mem(_PC++); write_mem(addr.w.l, _A); } return;
      ld_mword_hl: LDMEM_16(IY); return;
      ld_pc_hl:    _PC = _IY; return;
      ld_sp_hl:    _SP = _IY; iWSAdjust++; return;
      ld_sp_word:  z80.SP.b.l = read_mem(_PC++); z80.SP.b.h = read_mem(_PC++); return;
      nop:         return;
      or_a:        OR(_A); return;
      or_b:        OR(_B); return;
      or_byte:     OR(read_mem(_PC++)); return;
      or_c:        OR(_C); return;
      or_d:        OR(_D); return;
      or_e:        OR(_E); return;
      or_h:        OR(_IYh); return;
      or_l:        OR(_IYl); return;
      or_mhl:      { signed char o = read_mem(_PC++); OR(read_mem(_IY+o)); } return;
      outa:        { z80_wait_states iCycleCount = Oa_;} { reg_pair p; p.b.l = read_mem(_PC++); p.b.h = _A; z80_OUT_handler(p, _A); } return;
      pfx_cb:      z80_pfx_fdcb(); return;
      pfx_dd:      z80_pfx_dd(); return;
      pfx_ed:      z80_pfx_ed(); return;
      pfx_fd:      z80_pfx_fd(); return;
      pop_af:      POP(AF); return;
      pop_bc:      POP(BC); return;
      pop_de:      POP(DE); return;
      pop_hl:      POP(IY); return;
      push_af:     PUSH(AF); return;
      push_bc:     PUSH(BC); return;
      push_de:     PUSH(DE); return;
      push_hl:     PUSH(IY); return;
      ret:         RET; return;
      ret_c:       if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_m:       if (_F & Sflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_nc:      if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_nz:      if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_p:       if (!(_F & Sflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_pe:      if (_F & Pflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_po:      if (!(_F & Pflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      ret_z:       if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; return;
      rla:         RLA; return;
      rlca:        RLCA; return;
      rra:         RRA; return;
      rrca:        RRCA; return;
      rst00:       RST(0x0000); return;
      rst08:       RST(0x0008); return;
      rst10:       RST(0x0010); return;
      rst18:       RST(0x0018); return;
      rst20:       RST(0x0020); return;
      rst28:       RST(0x0028); return;
      rst30:       RST(0x0030); return;
      rst38:       RST(0x0038); return;
      sbc_a:       SBC(_A); return;
      sbc_b:       SBC(_B); return;
      sbc_byte:    SBC(read_mem(_PC++)); return;
      sbc_c:       SBC(_C); return;
      sbc_d:       SBC(_D); return;
      sbc_e:       SBC(_E); return;
      sbc_h:       SBC(_IYh); return;
      sbc_l:       SBC(_IYl); return;
      sbc_mhl:     { signed char o = read_mem(_PC++); SBC(read_mem(_IY+o)); } return;
      scf:         _F = (_F & (Sflag | Zflag | Pflag)) | Cflag | (_A & Xflags); return;
      sub_a:       SUB(_A); return;
      sub_b:       SUB(_B); return;
      sub_byte:    SUB(read_mem(_PC++)); return;
      sub_c:       SUB(_C); return;
      sub_d:       SUB(_D); return;
      sub_e:       SUB(_E); return;
      sub_h:       SUB(_IYh); return;
      sub_l:       SUB(_IYl); return;
      sub_mhl:     { signed char o = read_mem(_PC++); SUB(read_mem(_IY+o)); } return;
      xor_a:       XOR(_A); return;
      xor_b:       XOR(_B); return;
      xor_byte:    XOR(read_mem(_PC++)); return;
      xor_c:       XOR(_C); return;
      xor_d:       XOR(_D); return;
      xor_e:       XOR(_E); return;
      xor_h:       XOR(_IYh); return;
      xor_l:       XOR(_IYl); return;
      xor_mhl:     { signed char o = read_mem(_PC++); XOR(read_mem(_IY+o)); } return;
   //}
}
# else
void z80_pfx_fd(void)
{
   byte bOpCode;

   bOpCode = read_mem(_PC++);
   iCycleCount += cc_xy[bOpCode];
   _R++;
   switch(bOpCode)
   {
      case adc_a:       ADC(_A); break;
      case adc_b:       ADC(_B); break;
      case adc_byte:    ADC(read_mem(_PC++)); break;
      case adc_c:       ADC(_C); break;
      case adc_d:       ADC(_D); break;
      case adc_e:       ADC(_E); break;
      case adc_h:       ADC(_IYh); break;
      case adc_l:       ADC(_IYl); break;
      case adc_mhl:     { signed char o = read_mem(_PC++); ADC(read_mem(_IY+o)); } break;
      case add_a:       ADD(_A); break;
      case add_b:       ADD(_B); break;
      case add_byte:    ADD(read_mem(_PC++)); break;
      case add_c:       ADD(_C); break;
      case add_d:       ADD(_D); break;
      case add_e:       ADD(_E); break;
      case add_h:       ADD(_IYh); break;
      case add_hl_bc:   ADD16(IY, BC); break;
      case add_hl_de:   ADD16(IY, DE); break;
      case add_hl_hl:   ADD16(IY, IY); break;
      case add_hl_sp:   ADD16(IY, SP); break;
      case add_l:       ADD(_IYl); break;
      case add_mhl:     { signed char o = read_mem(_PC++); ADD(read_mem(_IY+o)); } break;
      case and_a:       AND(_A); break;
      case and_b:       AND(_B); break;
      case and_byte:    AND(read_mem(_PC++)); break;
      case and_c:       AND(_C); break;
      case and_d:       AND(_D); break;
      case and_e:       AND(_E); break;
      case and_h:       AND(_IYh); break;
      case and_l:       AND(_IYl); break;
      case and_mhl:     { signed char o = read_mem(_PC++); AND(read_mem(_IY+o)); } break;
      case call:        CALL; break;
      case call_c:      if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_m:      if (_F & Sflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_nc:     if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_nz:     if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_p:      if (!(_F & Sflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_pe:     if (_F & Pflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_po:     if (!(_F & Pflag)) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case call_z:      if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; CALL } else { _PC += 2; } break;
      case ccf:         _F = ((_F & (Sflag | Zflag | Pflag | Cflag)) | ((_F & CF) << 4) | (_A & Xflags)) ^ CF; break;
      case cpl:         _A ^= 0xff; _F = (_F & (Sflag | Zflag | Pflag | Cflag)) | Hflag | Nflag | (_A & Xflags); break;
      case cp_a:        CP(_A); break;
      case cp_b:        CP(_B); break;
      case cp_byte:     CP(read_mem(_PC++)); break;
      case cp_c:        CP(_C); break;
      case cp_d:        CP(_D); break;
      case cp_e:        CP(_E); break;
      case cp_h:        CP(_IYh); break;
      case cp_l:        CP(_IYl); break;
      case cp_mhl:      { signed char o = read_mem(_PC++); CP(read_mem(_IY+o)); } break;
      case daa:         DAA; break;
      case dec_a:       DEC(_A); break;
      case dec_b:       DEC(_B); break;
      case dec_bc:      _BC--; iWSAdjust++; break;
      case dec_c:       DEC(_C); break;
      case dec_d:       DEC(_D); break;
      case dec_de:      _DE--; iWSAdjust++; break;
      case dec_e:       DEC(_E); break;
      case dec_h:       DEC(_IYh); break;
      case dec_hl:      _IY--; iWSAdjust++; break;
      case dec_l:       DEC(_IYl); break;
      case dec_mhl:     { signed char o = read_mem(_PC++); byte b = read_mem(_IY+o); DEC(b); write_mem(_IY+o, b); } break;
      case dec_sp:      _SP--; iWSAdjust++; break;
      case di:          _IFF1 = _IFF2 = 0; z80.EI_issued = 0; break;
      case djnz:        if (--_B) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; } break;
      case ei:          z80.EI_issued = 2; break;
      case exx:         EXX; break;
      case ex_af_af:    EX(z80.AF, z80.AFx); break;
      case ex_de_hl:    EX(z80.DE, z80.HL); break;
      case ex_msp_hl:   EX_SP(IY); iWSAdjust++; break;
      case halt:        _HALT = 1; _PC--; break;
      case ina:         { z80_wait_states iCycleCount = Ia_;} { reg_pair p; p.b.l = read_mem(_PC++); p.b.h = _A; _A = z80_IN_handler(p); } break;
      case inc_a:       INC(_A); break;
      case inc_b:       INC(_B); break;
      case inc_bc:      _BC++; iWSAdjust++; break;
      case inc_c:       INC(_C); break;
      case inc_d:       INC(_D); break;
      case inc_de:      _DE++; iWSAdjust++; break;
      case inc_e:       INC(_E); break;
      case inc_h:       INC(_IYh); break;
      case inc_hl:      _IY++; iWSAdjust++; break;
      case inc_l:       INC(_IYl); break;
      case inc_mhl:     { signed char o = read_mem(_PC++); byte b = read_mem(_IY+o); INC(b); write_mem(_IY+o, b); } break;
      case inc_sp:      _SP++; iWSAdjust++; break;
      case jp:          JP; break;
      case jp_c:        if (_F & Cflag) { JP } else { _PC += 2; }; break;
      case jp_m:        if (_F & Sflag) { JP } else { _PC += 2; }; break;
      case jp_nc:       if (!(_F & Cflag)) { JP } else { _PC += 2; }; break;
      case jp_nz:       if (!(_F & Zflag)) { JP } else { _PC += 2; }; break;
      case jp_p:        if (!(_F & Sflag)) { JP } else { _PC += 2; }; break;
      case jp_pe:       if (_F & Pflag) { JP } else { _PC += 2; }; break;
      case jp_po:       if (!(_F & Pflag)) { JP } else { _PC += 2; }; break;
      case jp_z:        if (_F & Zflag) { JP } else { _PC += 2; }; break;
      case jr:          JR; break;
      case jr_c:        if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case jr_nc:       if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case jr_nz:       if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case jr_z:        if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; JR } else { _PC++; }; break;
      case ld_a_a:      break;
      case ld_a_b:      _A = _B; break;
      case ld_a_byte:   _A = read_mem(_PC++); break;
      case ld_a_c:      _A = _C; break;
      case ld_a_d:      _A = _D; break;
      case ld_a_e:      _A = _E; break;
      case ld_a_h:      _A = _IYh; break;
      case ld_a_l:      _A = _IYl; break;
      case ld_a_mbc:    _A = read_mem(_BC); break;
      case ld_a_mde:    _A = read_mem(_DE); break;
      case ld_a_mhl:    { signed char o = read_mem(_PC++); _A = read_mem(_IY+o); } break;
      case ld_a_mword:  { reg_pair addr; addr.b.l = read_mem(_PC++); addr.b.h = read_mem(_PC++); _A = read_mem(addr.w.l); } break;
      case ld_bc_word:  z80.BC.b.l = read_mem(_PC++); z80.BC.b.h = read_mem(_PC++); break;
      case ld_b_a:      _B = _A; break;
      case ld_b_b:      break;
      case ld_b_byte:   _B = read_mem(_PC++); break;
      case ld_b_c:      _B = _C; break;
      case ld_b_d:      _B = _D; break;
      case ld_b_e:      _B = _E; break;
      case ld_b_h:      _B = _IYh; break;
      case ld_b_l:      _B = _IYl; break;
      case ld_b_mhl:    { signed char o = read_mem(_PC++); _B = read_mem(_IY+o); } break;
      case ld_c_a:      _C = _A; break;
      case ld_c_b:      _C = _B; break;
      case ld_c_byte:   _C = read_mem(_PC++); break;
      case ld_c_c:      break;
      case ld_c_d:      _C = _D; break;
      case ld_c_e:      _C = _E; break;
      case ld_c_h:      _C = _IYh; break;
      case ld_c_l:      _C = _IYl; break;
      case ld_c_mhl:    { signed char o = read_mem(_PC++); _C = read_mem(_IY+o); } break;
      case ld_de_word:  z80.DE.b.l = read_mem(_PC++); z80.DE.b.h = read_mem(_PC++); break;
      case ld_d_a:      _D = _A; break;
      case ld_d_b:      _D = _B; break;
      case ld_d_byte:   _D = read_mem(_PC++); break;
      case ld_d_c:      _D = _C; break;
      case ld_d_d:      break;
      case ld_d_e:      _D = _E; break;
      case ld_d_h:      _D = _IYh; break;
      case ld_d_l:      _D = _IYl; break;
      case ld_d_mhl:    { signed char o = read_mem(_PC++); _D = read_mem(_IY+o); } break;
      case ld_e_a:      _E = _A; break;
      case ld_e_b:      _E = _B; break;
      case ld_e_byte:   _E = read_mem(_PC++); break;
      case ld_e_c:      _E = _C; break;
      case ld_e_d:      _E = _D; break;
      case ld_e_e:      break;
      case ld_e_h:      _E = _IYh; break;
      case ld_e_l:      _E = _IYl; break;
      case ld_e_mhl:    { signed char o = read_mem(_PC++); _E = read_mem(_IY+o); } break;
      case ld_hl_mword: LD16_MEM(IY); break;
      case ld_hl_word:  z80.IY.b.l = read_mem(_PC++); z80.IY.b.h = read_mem(_PC++); break;
      case ld_h_a:      _IYh = _A; break;
      case ld_h_b:      _IYh = _B; break;
      case ld_h_byte:   _IYh = read_mem(_PC++); break;
      case ld_h_c:      _IYh = _C; break;
      case ld_h_d:      _IYh = _D; break;
      case ld_h_e:      _IYh = _E; break;
      case ld_h_h:      break;
      case ld_h_l:      _IYh = _IYl; break;
      case ld_h_mhl:    { signed char o = read_mem(_PC++); _H = read_mem(_IY+o); } break;
      case ld_l_a:      _IYl = _A; break;
      case ld_l_b:      _IYl = _B; break;
      case ld_l_byte:   _IYl = read_mem(_PC++); break;
      case ld_l_c:      _IYl = _C; break;
      case ld_l_d:      _IYl = _D; break;
      case ld_l_e:      _IYl = _E; break;
      case ld_l_h:      _IYl = _IYh; break;
      case ld_l_l:      break;
      case ld_l_mhl:    { signed char o = read_mem(_PC++); _L = read_mem(_IY+o); } break;
      case ld_mbc_a:    write_mem(_BC, _A); break;
      case ld_mde_a:    write_mem(_DE, _A); break;
      case ld_mhl_a:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _A); } break;
      case ld_mhl_b:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _B); } break;
      case ld_mhl_byte: { signed char o = read_mem(_PC++); byte b = read_mem(_PC++); write_mem(_IY+o, b); } break;
      case ld_mhl_c:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _C); } break;
      case ld_mhl_d:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _D); } break;
      case ld_mhl_e:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _E); } break;
      case ld_mhl_h:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _H); } break;
      case ld_mhl_l:    { signed char o = read_mem(_PC++); write_mem(_IY+o, _L); } break;
      case ld_mword_a:  { reg_pair addr; addr.b.l = read_mem(_PC++); addr.b.h = read_mem(_PC++); write_mem(addr.w.l, _A); } break;
      case ld_mword_hl: LDMEM_16(IY); break;
      case ld_pc_hl:    _PC = _IY; break;
      case ld_sp_hl:    _SP = _IY; iWSAdjust++; break;
      case ld_sp_word:  z80.SP.b.l = read_mem(_PC++); z80.SP.b.h = read_mem(_PC++); break;
      case nop:         break;
      case or_a:        OR(_A); break;
      case or_b:        OR(_B); break;
      case or_byte:     OR(read_mem(_PC++)); break;
      case or_c:        OR(_C); break;
      case or_d:        OR(_D); break;
      case or_e:        OR(_E); break;
      case or_h:        OR(_IYh); break;
      case or_l:        OR(_IYl); break;
      case or_mhl:      { signed char o = read_mem(_PC++); OR(read_mem(_IY+o)); } break;
      case outa:        { z80_wait_states iCycleCount = Oa_;} { reg_pair p; p.b.l = read_mem(_PC++); p.b.h = _A; z80_OUT_handler(p, _A); } break;
      case pfx_cb:      z80_pfx_fdcb(); break;
      case pfx_dd:      z80_pfx_dd(); break;
      case pfx_ed:      z80_pfx_ed(); break;
      case pfx_fd:      z80_pfx_fd(); break;
      case pop_af:      POP(AF); break;
      case pop_bc:      POP(BC); break;
      case pop_de:      POP(DE); break;
      case pop_hl:      POP(IY); break;
      case push_af:     PUSH(AF); break;
      case push_bc:     PUSH(BC); break;
      case push_de:     PUSH(DE); break;
      case push_hl:     PUSH(IY); break;
      case ret:         RET; break;
      case ret_c:       if (_F & Cflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_m:       if (_F & Sflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_nc:      if (!(_F & Cflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_nz:      if (!(_F & Zflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_p:       if (!(_F & Sflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_pe:      if (_F & Pflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_po:      if (!(_F & Pflag)) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case ret_z:       if (_F & Zflag) { iCycleCount += cc_ex[bOpCode]; RET } else { iWSAdjust++; } ; break;
      case rla:         RLA; break;
      case rlca:        RLCA; break;
      case rra:         RRA; break;
      case rrca:        RRCA; break;
      case rst00:       RST(0x0000); break;
      case rst08:       RST(0x0008); break;
      case rst10:       RST(0x0010); break;
      case rst18:       RST(0x0018); break;
      case rst20:       RST(0x0020); break;
      case rst28:       RST(0x0028); break;
      case rst30:       RST(0x0030); break;
      case rst38:       RST(0x0038); break;
      case sbc_a:       SBC(_A); break;
      case sbc_b:       SBC(_B); break;
      case sbc_byte:    SBC(read_mem(_PC++)); break;
      case sbc_c:       SBC(_C); break;
      case sbc_d:       SBC(_D); break;
      case sbc_e:       SBC(_E); break;
      case sbc_h:       SBC(_IYh); break;
      case sbc_l:       SBC(_IYl); break;
      case sbc_mhl:     { signed char o = read_mem(_PC++); SBC(read_mem(_IY+o)); } break;
      case scf:         _F = (_F & (Sflag | Zflag | Pflag)) | Cflag | (_A & Xflags); break;
      case sub_a:       SUB(_A); break;
      case sub_b:       SUB(_B); break;
      case sub_byte:    SUB(read_mem(_PC++)); break;
      case sub_c:       SUB(_C); break;
      case sub_d:       SUB(_D); break;
      case sub_e:       SUB(_E); break;
      case sub_h:       SUB(_IYh); break;
      case sub_l:       SUB(_IYl); break;
      case sub_mhl:     { signed char o = read_mem(_PC++); SUB(read_mem(_IY+o)); } break;
      case xor_a:       XOR(_A); break;
      case xor_b:       XOR(_B); break;
      case xor_byte:    XOR(read_mem(_PC++)); break;
      case xor_c:       XOR(_C); break;
      case xor_d:       XOR(_D); break;
      case xor_e:       XOR(_E); break;
      case xor_h:       XOR(_IYh); break;
      case xor_l:       XOR(_IYl); break;
      case xor_mhl:     { signed char o = read_mem(_PC++); XOR(read_mem(_IY+o)); } break;
   }
}
# endif

# ifdef Z80_JMP_TBL
void z80_pfx_fdcb(void)
{
   __label__
   rlc_b, rlc_c, rlc_d, rlc_e, rlc_h, rlc_l, rlc_mhl, rlc_a,
   rrc_b, rrc_c, rrc_d, rrc_e, rrc_h, rrc_l, rrc_mhl, rrc_a,
   rl_b, rl_c, rl_d, rl_e, rl_h, rl_l, rl_mhl, rl_a,
   rr_b, rr_c, rr_d, rr_e, rr_h, rr_l, rr_mhl, rr_a,
   sla_b, sla_c, sla_d, sla_e, sla_h, sla_l, sla_mhl, sla_a,
   sra_b, sra_c, sra_d, sra_e, sra_h, sra_l, sra_mhl, sra_a,
   sll_b, sll_c, sll_d, sll_e, sll_h, sll_l, sll_mhl, sll_a,
   srl_b, srl_c, srl_d, srl_e, srl_h, srl_l, srl_mhl, srl_a,
   bit0_b, bit0_c, bit0_d, bit0_e, bit0_h, bit0_l, bit0_mhl, bit0_a,
   bit1_b, bit1_c, bit1_d, bit1_e, bit1_h, bit1_l, bit1_mhl, bit1_a,
   bit2_b, bit2_c, bit2_d, bit2_e, bit2_h, bit2_l, bit2_mhl, bit2_a,
   bit3_b, bit3_c, bit3_d, bit3_e, bit3_h, bit3_l, bit3_mhl, bit3_a,
   bit4_b, bit4_c, bit4_d, bit4_e, bit4_h, bit4_l, bit4_mhl, bit4_a,
   bit5_b, bit5_c, bit5_d, bit5_e, bit5_h, bit5_l, bit5_mhl, bit5_a,
   bit6_b, bit6_c, bit6_d, bit6_e, bit6_h, bit6_l, bit6_mhl, bit6_a,
   bit7_b, bit7_c, bit7_d, bit7_e, bit7_h, bit7_l, bit7_mhl, bit7_a,
   res0_b, res0_c, res0_d, res0_e, res0_h, res0_l, res0_mhl, res0_a,
   res1_b, res1_c, res1_d, res1_e, res1_h, res1_l, res1_mhl, res1_a,
   res2_b, res2_c, res2_d, res2_e, res2_h, res2_l, res2_mhl, res2_a,
   res3_b, res3_c, res3_d, res3_e, res3_h, res3_l, res3_mhl, res3_a,
   res4_b, res4_c, res4_d, res4_e, res4_h, res4_l, res4_mhl, res4_a,
   res5_b, res5_c, res5_d, res5_e, res5_h, res5_l, res5_mhl, res5_a,
   res6_b, res6_c, res6_d, res6_e, res6_h, res6_l, res6_mhl, res6_a,
   res7_b, res7_c, res7_d, res7_e, res7_h, res7_l, res7_mhl, res7_a,
   set0_b, set0_c, set0_d, set0_e, set0_h, set0_l, set0_mhl, set0_a,
   set1_b, set1_c, set1_d, set1_e, set1_h, set1_l, set1_mhl, set1_a,
   set2_b, set2_c, set2_d, set2_e, set2_h, set2_l, set2_mhl, set2_a,
   set3_b, set3_c, set3_d, set3_e, set3_h, set3_l, set3_mhl, set3_a,
   set4_b, set4_c, set4_d, set4_e, set4_h, set4_l, set4_mhl, set4_a,
   set5_b, set5_c, set5_d, set5_e, set5_h, set5_l, set5_mhl, set5_a,
   set6_b, set6_c, set6_d, set6_e, set6_h, set6_l, set6_mhl, set6_a,
   set7_b, set7_c, set7_d, set7_e, set7_h, set7_l, set7_mhl, set7_a;

    static const void* const a_jump_table[256] =  { &&
   rlc_b,&& rlc_c,&& rlc_d,&& rlc_e,&& rlc_h,&& rlc_l,&& rlc_mhl,&& rlc_a,&&
   rrc_b,&& rrc_c,&& rrc_d,&& rrc_e,&& rrc_h,&& rrc_l,&& rrc_mhl,&& rrc_a,&&
   rl_b,&& rl_c,&& rl_d,&& rl_e,&& rl_h,&& rl_l,&& rl_mhl,&& rl_a,&&
   rr_b,&& rr_c,&& rr_d,&& rr_e,&& rr_h,&& rr_l,&& rr_mhl,&& rr_a,&&
   sla_b,&& sla_c,&& sla_d,&& sla_e,&& sla_h,&& sla_l,&& sla_mhl,&& sla_a,&&
   sra_b,&& sra_c,&& sra_d,&& sra_e,&& sra_h,&& sra_l,&& sra_mhl,&& sra_a,&&
   sll_b,&& sll_c,&& sll_d,&& sll_e,&& sll_h,&& sll_l,&& sll_mhl,&& sll_a,&&
   srl_b,&& srl_c,&& srl_d,&& srl_e,&& srl_h,&& srl_l,&& srl_mhl,&& srl_a,&&
   bit0_b,&& bit0_c,&& bit0_d,&& bit0_e,&& bit0_h,&& bit0_l,&& bit0_mhl,&& bit0_a,&&
   bit1_b,&& bit1_c,&& bit1_d,&& bit1_e,&& bit1_h,&& bit1_l,&& bit1_mhl,&& bit1_a,&&
   bit2_b,&& bit2_c,&& bit2_d,&& bit2_e,&& bit2_h,&& bit2_l,&& bit2_mhl,&& bit2_a,&&
   bit3_b,&& bit3_c,&& bit3_d,&& bit3_e,&& bit3_h,&& bit3_l,&& bit3_mhl,&& bit3_a,&&
   bit4_b,&& bit4_c,&& bit4_d,&& bit4_e,&& bit4_h,&& bit4_l,&& bit4_mhl,&& bit4_a,&&
   bit5_b,&& bit5_c,&& bit5_d,&& bit5_e,&& bit5_h,&& bit5_l,&& bit5_mhl,&& bit5_a,&&
   bit6_b,&& bit6_c,&& bit6_d,&& bit6_e,&& bit6_h,&& bit6_l,&& bit6_mhl,&& bit6_a,&&
   bit7_b,&& bit7_c,&& bit7_d,&& bit7_e,&& bit7_h,&& bit7_l,&& bit7_mhl,&& bit7_a,&&
   res0_b,&& res0_c,&& res0_d,&& res0_e,&& res0_h,&& res0_l,&& res0_mhl,&& res0_a,&&
   res1_b,&& res1_c,&& res1_d,&& res1_e,&& res1_h,&& res1_l,&& res1_mhl,&& res1_a,&&
   res2_b,&& res2_c,&& res2_d,&& res2_e,&& res2_h,&& res2_l,&& res2_mhl,&& res2_a,&&
   res3_b,&& res3_c,&& res3_d,&& res3_e,&& res3_h,&& res3_l,&& res3_mhl,&& res3_a,&&
   res4_b,&& res4_c,&& res4_d,&& res4_e,&& res4_h,&& res4_l,&& res4_mhl,&& res4_a,&&
   res5_b,&& res5_c,&& res5_d,&& res5_e,&& res5_h,&& res5_l,&& res5_mhl,&& res5_a,&&
   res6_b,&& res6_c,&& res6_d,&& res6_e,&& res6_h,&& res6_l,&& res6_mhl,&& res6_a,&&
   res7_b,&& res7_c,&& res7_d,&& res7_e,&& res7_h,&& res7_l,&& res7_mhl,&& res7_a,&&
   set0_b,&& set0_c,&& set0_d,&& set0_e,&& set0_h,&& set0_l,&& set0_mhl,&& set0_a,&&
   set1_b,&& set1_c,&& set1_d,&& set1_e,&& set1_h,&& set1_l,&& set1_mhl,&& set1_a,&&
   set2_b,&& set2_c,&& set2_d,&& set2_e,&& set2_h,&& set2_l,&& set2_mhl,&& set2_a,&&
   set3_b,&& set3_c,&& set3_d,&& set3_e,&& set3_h,&& set3_l,&& set3_mhl,&& set3_a,&&
   set4_b,&& set4_c,&& set4_d,&& set4_e,&& set4_h,&& set4_l,&& set4_mhl,&& set4_a,&&
   set5_b,&& set5_c,&& set5_d,&& set5_e,&& set5_h,&& set5_l,&& set5_mhl,&& set5_a,&&
   set6_b,&& set6_c,&& set6_d,&& set6_e,&& set6_h,&& set6_l,&& set6_mhl,&& set6_a,&&
   set7_b,&& set7_c,&& set7_d,&& set7_e,&& set7_h,&& set7_l,&& set7_mhl,&& set7_a
   };

   signed char o;
   byte bOpCode;

   o = read_mem(_PC++); // offset
   bOpCode = read_mem(_PC++);
   iCycleCount += cc_xycb[bOpCode];
	 goto *a_jump_table[bOpCode];
   //switch(bOpCode)
   //{
      bit0_a:     
      bit0_b:    
      bit0_c:   
      bit0_d:  
      bit0_e: 
      bit0_h:
      bit0_l:   
      bit0_mhl:    BIT_XY(0, read_mem(_IY+o)); return;
      bit1_a:     
      bit1_b:    
      bit1_c:   
      bit1_d:  
      bit1_e: 
      bit1_h:   
      bit1_l:  
      bit1_mhl:    BIT_XY(1, read_mem(_IY+o)); return;
      bit2_a:    
      bit2_b:   
      bit2_c:  
      bit2_d: 
      bit2_e:
      bit2_h:     
      bit2_l:    
      bit2_mhl:    BIT_XY(2, read_mem(_IY+o)); return;
      bit3_a:     
      bit3_b:    
      bit3_c:   
      bit3_d:  
      bit3_e: 
      bit3_h:
      bit3_l:   
      bit3_mhl:    BIT_XY(3, read_mem(_IY+o)); return;
      bit4_a:     
      bit4_b:    
      bit4_c:   
      bit4_d:  
      bit4_e: 
      bit4_h:   
      bit4_l:  
      bit4_mhl:    BIT_XY(4, read_mem(_IY+o)); return;
      bit5_a:     
      bit5_b:    
      bit5_c:   
      bit5_d:  
      bit5_e: 
      bit5_h:     
      bit5_l:    
      bit5_mhl:    BIT_XY(5, read_mem(_IY+o)); return;
      bit6_a:   
      bit6_b:  
      bit6_c:    
      bit6_d:   
      bit6_e:     
      bit6_h:    
      bit6_l:   
      bit6_mhl:    BIT_XY(6, read_mem(_IY+o)); return;
      bit7_a:     
      bit7_b:    
      bit7_c:  
      bit7_d:  
      bit7_e: 
      bit7_h:     
      bit7_l:    
      bit7_mhl:    BIT_XY(7, read_mem(_IY+o)); return;
      res0_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(0, _A)); return;
      res0_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(0, _B)); return;
      res0_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(0, _C)); return;
      res0_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(0, _D)); return;
      res0_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(0, _E)); return;
      res0_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(0, _H)); return;
      res0_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(0, _L)); return;
      res0_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(0, b)); } return;
      res1_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(1, _A)); return;
      res1_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(1, _B)); return;
      res1_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(1, _C)); return;
      res1_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(1, _D)); return;
      res1_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(1, _E)); return;
      res1_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(1, _H)); return;
      res1_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(1, _L)); return;
      res1_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(1, b)); } return;
      res2_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(2, _A)); return;
      res2_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(2, _B)); return;
      res2_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(2, _C)); return;
      res2_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(2, _D)); return;
      res2_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(2, _E)); return;
      res2_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(2, _H)); return;
      res2_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(2, _L)); return;
      res2_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(2, b)); } return;
      res3_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(3, _A)); return;
      res3_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(3, _B)); return;
      res3_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(3, _C)); return;
      res3_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(3, _D)); return;
      res3_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(3, _E)); return;
      res3_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(3, _H)); return;
      res3_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(3, _L)); return;
      res3_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(3, b)); } return;
      res4_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(4, _A)); return;
      res4_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(4, _B)); return;
      res4_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(4, _C)); return;
      res4_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(4, _D)); return;
      res4_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(4, _E)); return;
      res4_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(4, _H)); return;
      res4_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(4, _L)); return;
      res4_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(4, b)); } return;
      res5_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(5, _A)); return;
      res5_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(5, _B)); return;
      res5_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(5, _C)); return;
      res5_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(5, _D)); return;
      res5_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(5, _E)); return;
      res5_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(5, _H)); return;
      res5_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(5, _L)); return;
      res5_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(5, b)); } return;
      res6_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(6, _A)); return;
      res6_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(6, _B)); return;
      res6_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(6, _C)); return;
      res6_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(6, _D)); return;
      res6_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(6, _E)); return;
      res6_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(6, _H)); return;
      res6_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(6, _L)); return;
      res6_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(6, b)); } return;
      res7_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(7, _A)); return;
      res7_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(7, _B)); return;
      res7_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(7, _C)); return;
      res7_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(7, _D)); return;
      res7_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(7, _E)); return;
      res7_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(7, _H)); return;
      res7_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(7, _L)); return;
      res7_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(7, b)); } return;
      rlc_a:       _A = read_mem(_IY+o); _A = RLC(_A); write_mem(_IY+o, _A); return;
      rlc_b:       _B = read_mem(_IY+o); _B = RLC(_B); write_mem(_IY+o, _B); return;
      rlc_c:       _C = read_mem(_IY+o); _C = RLC(_C); write_mem(_IY+o, _C); return;
      rlc_d:       _D = read_mem(_IY+o); _D = RLC(_D); write_mem(_IY+o, _D); return;
      rlc_e:       _E = read_mem(_IY+o); _E = RLC(_E); write_mem(_IY+o, _E); return;
      rlc_h:       _H = read_mem(_IY+o); _H = RLC(_H); write_mem(_IY+o, _H); return;
      rlc_l:       _L = read_mem(_IY+o); _L = RLC(_L); write_mem(_IY+o, _L); return;
      rlc_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, RLC(b)); } return;
      rl_a:        _A = read_mem(_IY+o); _A = RL(_A); write_mem(_IY+o, _A); return;
      rl_b:        _B = read_mem(_IY+o); _B = RL(_B); write_mem(_IY+o, _B); return;
      rl_c:        _C = read_mem(_IY+o); _C = RL(_C); write_mem(_IY+o, _C); return;
      rl_d:        _D = read_mem(_IY+o); _D = RL(_D); write_mem(_IY+o, _D); return;
      rl_e:        _E = read_mem(_IY+o); _E = RL(_E); write_mem(_IY+o, _E); return;
      rl_h:        _H = read_mem(_IY+o); _H = RL(_H); write_mem(_IY+o, _H); return;
      rl_l:        _L = read_mem(_IY+o); _L = RL(_L); write_mem(_IY+o, _L); return;
      rl_mhl:      { byte b = read_mem(_IY+o); write_mem(_IY+o, RL(b)); } return;
      rrc_a:       _A = read_mem(_IY+o); _A = RRC(_A); write_mem(_IY+o, _A); return;
      rrc_b:       _B = read_mem(_IY+o); _B = RRC(_B); write_mem(_IY+o, _B); return;
      rrc_c:       _C = read_mem(_IY+o); _C = RRC(_C); write_mem(_IY+o, _C); return;
      rrc_d:       _D = read_mem(_IY+o); _D = RRC(_D); write_mem(_IY+o, _D); return;
      rrc_e:       _E = read_mem(_IY+o); _E = RRC(_E); write_mem(_IY+o, _E); return;
      rrc_h:       _H = read_mem(_IY+o); _H = RRC(_H); write_mem(_IY+o, _H); return;
      rrc_l:       _L = read_mem(_IY+o); _L = RRC(_L); write_mem(_IY+o, _L); return;
      rrc_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, RRC(b)); } return;
      rr_a:        _A = read_mem(_IY+o); _A = RR(_A); write_mem(_IY+o, _A); return;
      rr_b:        _B = read_mem(_IY+o); _B = RR(_B); write_mem(_IY+o, _B); return;
      rr_c:        _C = read_mem(_IY+o); _C = RR(_C); write_mem(_IY+o, _C); return;
      rr_d:        _D = read_mem(_IY+o); _D = RR(_D); write_mem(_IY+o, _D); return;
      rr_e:        _E = read_mem(_IY+o); _E = RR(_E); write_mem(_IY+o, _E); return;
      rr_h:        _H = read_mem(_IY+o); _H = RR(_H); write_mem(_IY+o, _H); return;
      rr_l:        _L = read_mem(_IY+o); _L = RR(_L); write_mem(_IY+o, _L); return;
      rr_mhl:      { byte b = read_mem(_IY+o); write_mem(_IY+o, RR(b)); } return;
      set0_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(0, _A)); return;
      set0_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(0, _B)); return;
      set0_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(0, _C)); return;
      set0_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(0, _D)); return;
      set0_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(0, _E)); return;
      set0_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(0, _H)); return;
      set0_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(0, _L)); return;
      set0_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(0, b)); } return;
      set1_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(1, _A)); return;
      set1_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(1, _B)); return;
      set1_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(1, _C)); return;
      set1_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(1, _D)); return;
      set1_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(1, _E)); return;
      set1_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(1, _H)); return;
      set1_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(1, _L)); return;
      set1_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(1, b)); } return;
      set2_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(2, _A)); return;
      set2_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(2, _B)); return;
      set2_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(2, _C)); return;
      set2_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(2, _D)); return;
      set2_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(2, _E)); return;
      set2_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(2, _H)); return;
      set2_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(2, _L)); return;
      set2_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(2, b)); } return;
      set3_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(3, _A)); return;
      set3_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(3, _B)); return;
      set3_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(3, _C)); return;
      set3_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(3, _D)); return;
      set3_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(3, _E)); return;
      set3_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(3, _H)); return;
      set3_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(3, _L)); return;
      set3_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(3, b)); } return;
      set4_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(4, _A)); return;
      set4_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(4, _B)); return;
      set4_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(4, _C)); return;
      set4_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(4, _D)); return;
      set4_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(4, _E)); return;
      set4_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(4, _H)); return;
      set4_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(4, _L)); return;
      set4_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(4, b)); } return;
      set5_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(5, _A)); return;
      set5_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(5, _B)); return;
      set5_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(5, _C)); return;
      set5_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(5, _D)); return;
      set5_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(5, _E)); return;
      set5_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(5, _H)); return;
      set5_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(5, _L)); return;
      set5_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(5, b)); } return;
      set6_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(6, _A)); return;
      set6_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(6, _B)); return;
      set6_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(6, _C)); return;
      set6_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(6, _D)); return;
      set6_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(6, _E)); return;
      set6_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(6, _H)); return;
      set6_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(6, _L)); return;
      set6_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(6, b)); } return;
      set7_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(7, _A)); return;
      set7_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(7, _B)); return;
      set7_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(7, _C)); return;
      set7_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(7, _D)); return;
      set7_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(7, _E)); return;
      set7_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(7, _H)); return;
      set7_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(7, _L)); return;
      set7_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(7, b)); } return;
      sla_a:       _A = read_mem(_IY+o); _A = SLA(_A); write_mem(_IY+o, _A); return;
      sla_b:       _B = read_mem(_IY+o); _B = SLA(_B); write_mem(_IY+o, _B); return;
      sla_c:       _C = read_mem(_IY+o); _C = SLA(_C); write_mem(_IY+o, _C); return;
      sla_d:       _D = read_mem(_IY+o); _D = SLA(_D); write_mem(_IY+o, _D); return;
      sla_e:       _E = read_mem(_IY+o); _E = SLA(_E); write_mem(_IY+o, _E); return;
      sla_h:       _H = read_mem(_IY+o); _H = SLA(_H); write_mem(_IY+o, _H); return;
      sla_l:       _L = read_mem(_IY+o); _L = SLA(_L); write_mem(_IY+o, _L); return;
      sla_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, SLA(b)); } return;
      sll_a:       _A = read_mem(_IY+o); _A = SLL(_A); write_mem(_IY+o, _A); return;
      sll_b:       _B = read_mem(_IY+o); _B = SLL(_B); write_mem(_IY+o, _B); return;
      sll_c:       _C = read_mem(_IY+o); _C = SLL(_C); write_mem(_IY+o, _C); return;
      sll_d:       _D = read_mem(_IY+o); _D = SLL(_D); write_mem(_IY+o, _D); return;
      sll_e:       _E = read_mem(_IY+o); _E = SLL(_E); write_mem(_IY+o, _E); return;
      sll_h:       _H = read_mem(_IY+o); _H = SLL(_H); write_mem(_IY+o, _H); return;
      sll_l:       _L = read_mem(_IY+o); _L = SLL(_L); write_mem(_IY+o, _L); return;
      sll_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, SLL(b)); } return;
      sra_a:       _A = read_mem(_IY+o); _A = SRA(_A); write_mem(_IY+o, _A); return;
      sra_b:       _B = read_mem(_IY+o); _B = SRA(_B); write_mem(_IY+o, _B); return;
      sra_c:       _C = read_mem(_IY+o); _C = SRA(_C); write_mem(_IY+o, _C); return;
      sra_d:       _D = read_mem(_IY+o); _D = SRA(_D); write_mem(_IY+o, _D); return;
      sra_e:       _E = read_mem(_IY+o); _E = SRA(_E); write_mem(_IY+o, _E); return;
      sra_h:       _H = read_mem(_IY+o); _H = SRA(_H); write_mem(_IY+o, _H); return;
      sra_l:       _L = read_mem(_IY+o); _L = SRA(_L); write_mem(_IY+o, _L); return;
      sra_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, SRA(b)); } return;
      srl_a:       _A = read_mem(_IY+o); _A = SRL(_A); write_mem(_IY+o, _A); return;
      srl_b:       _B = read_mem(_IY+o); _B = SRL(_B); write_mem(_IY+o, _B); return;
      srl_c:       _C = read_mem(_IY+o); _C = SRL(_C); write_mem(_IY+o, _C); return;
      srl_d:       _D = read_mem(_IY+o); _D = SRL(_D); write_mem(_IY+o, _D); return;
      srl_e:       _E = read_mem(_IY+o); _E = SRL(_E); write_mem(_IY+o, _E); return;
      srl_h:       _H = read_mem(_IY+o); _H = SRL(_H); write_mem(_IY+o, _H); return;
      srl_l:       _L = read_mem(_IY+o); _L = SRL(_L); write_mem(_IY+o, _L); return;
      srl_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, SRL(b)); } return;
   //}
}
# else
void z80_pfx_fdcb(void)
{
   signed char o;
   byte bOpCode;

   o = read_mem(_PC++); // offset
   bOpCode = read_mem(_PC++);
   iCycleCount += cc_xycb[bOpCode];
   switch(bOpCode)
   {
      case bit0_a:     
      case bit0_b:     
      case bit0_c:    
      case bit0_d:   
      case bit0_e:  
      case bit0_h: 
      case bit0_l:
      case bit0_mhl:    BIT_XY(0, read_mem(_IY+o)); break;
      case bit1_a:     
      case bit1_b:    
      case bit1_c:   
      case bit1_d:  
      case bit1_e: 
      case bit1_h:
      case bit1_l:    
      case bit1_mhl:    BIT_XY(1, read_mem(_IY+o)); break;
      case bit2_a:     
      case bit2_b:    
      case bit2_c:   
      case bit2_d:  
      case bit2_e: 
      case bit2_h:     
      case bit2_l:    
      case bit2_mhl:    BIT_XY(2, read_mem(_IY+o)); break;
      case bit3_a:   
      case bit3_b:     
      case bit3_c:    
      case bit3_d:   
      case bit3_e:  
      case bit3_h: 
      case bit3_l:     
      case bit3_mhl:    BIT_XY(3, read_mem(_IY+o)); break;
      case bit4_a:    
      case bit4_b:   
      case bit4_c:  
      case bit4_d: 
      case bit4_e:
      case bit4_h:    
      case bit4_l:   
      case bit4_mhl:    BIT_XY(4, read_mem(_IY+o)); break;
      case bit5_a:     
      case bit5_b:    
      case bit5_c:   
      case bit5_d:  
      case bit5_e:   
      case bit5_h:  
      case bit5_l:  
      case bit5_mhl:    BIT_XY(5, read_mem(_IY+o)); break;
      case bit6_a:     
      case bit6_b:    
      case bit6_c:   
      case bit6_d:  
      case bit6_e: 
      case bit6_h:     
      case bit6_l:    
      case bit6_mhl:    BIT_XY(6, read_mem(_IY+o)); break;
      case bit7_a:     
      case bit7_b:    
      case bit7_c:   
      case bit7_d:  
      case bit7_e: 
      case bit7_h:     
      case bit7_l:    
      case bit7_mhl:    BIT_XY(7, read_mem(_IY+o)); break;
      case res0_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(0, _A)); break;
      case res0_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(0, _B)); break;
      case res0_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(0, _C)); break;
      case res0_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(0, _D)); break;
      case res0_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(0, _E)); break;
      case res0_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(0, _H)); break;
      case res0_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(0, _L)); break;
      case res0_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(0, b)); } break;
      case res1_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(1, _A)); break;
      case res1_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(1, _B)); break;
      case res1_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(1, _C)); break;
      case res1_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(1, _D)); break;
      case res1_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(1, _E)); break;
      case res1_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(1, _H)); break;
      case res1_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(1, _L)); break;
      case res1_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(1, b)); } break;
      case res2_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(2, _A)); break;
      case res2_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(2, _B)); break;
      case res2_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(2, _C)); break;
      case res2_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(2, _D)); break;
      case res2_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(2, _E)); break;
      case res2_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(2, _H)); break;
      case res2_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(2, _L)); break;
      case res2_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(2, b)); } break;
      case res3_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(3, _A)); break;
      case res3_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(3, _B)); break;
      case res3_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(3, _C)); break;
      case res3_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(3, _D)); break;
      case res3_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(3, _E)); break;
      case res3_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(3, _H)); break;
      case res3_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(3, _L)); break;
      case res3_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(3, b)); } break;
      case res4_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(4, _A)); break;
      case res4_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(4, _B)); break;
      case res4_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(4, _C)); break;
      case res4_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(4, _D)); break;
      case res4_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(4, _E)); break;
      case res4_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(4, _H)); break;
      case res4_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(4, _L)); break;
      case res4_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(4, b)); } break;
      case res5_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(5, _A)); break;
      case res5_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(5, _B)); break;
      case res5_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(5, _C)); break;
      case res5_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(5, _D)); break;
      case res5_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(5, _E)); break;
      case res5_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(5, _H)); break;
      case res5_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(5, _L)); break;
      case res5_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(5, b)); } break;
      case res6_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(6, _A)); break;
      case res6_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(6, _B)); break;
      case res6_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(6, _C)); break;
      case res6_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(6, _D)); break;
      case res6_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(6, _E)); break;
      case res6_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(6, _H)); break;
      case res6_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(6, _L)); break;
      case res6_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(6, b)); } break;
      case res7_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = RES(7, _A)); break;
      case res7_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = RES(7, _B)); break;
      case res7_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = RES(7, _C)); break;
      case res7_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = RES(7, _D)); break;
      case res7_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = RES(7, _E)); break;
      case res7_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = RES(7, _H)); break;
      case res7_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = RES(7, _L)); break;
      case res7_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, RES(7, b)); } break;
      case rlc_a:       _A = read_mem(_IY+o); _A = RLC(_A); write_mem(_IY+o, _A); break;
      case rlc_b:       _B = read_mem(_IY+o); _B = RLC(_B); write_mem(_IY+o, _B); break;
      case rlc_c:       _C = read_mem(_IY+o); _C = RLC(_C); write_mem(_IY+o, _C); break;
      case rlc_d:       _D = read_mem(_IY+o); _D = RLC(_D); write_mem(_IY+o, _D); break;
      case rlc_e:       _E = read_mem(_IY+o); _E = RLC(_E); write_mem(_IY+o, _E); break;
      case rlc_h:       _H = read_mem(_IY+o); _H = RLC(_H); write_mem(_IY+o, _H); break;
      case rlc_l:       _L = read_mem(_IY+o); _L = RLC(_L); write_mem(_IY+o, _L); break;
      case rlc_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, RLC(b)); } break;
      case rl_a:        _A = read_mem(_IY+o); _A = RL(_A); write_mem(_IY+o, _A); break;
      case rl_b:        _B = read_mem(_IY+o); _B = RL(_B); write_mem(_IY+o, _B); break;
      case rl_c:        _C = read_mem(_IY+o); _C = RL(_C); write_mem(_IY+o, _C); break;
      case rl_d:        _D = read_mem(_IY+o); _D = RL(_D); write_mem(_IY+o, _D); break;
      case rl_e:        _E = read_mem(_IY+o); _E = RL(_E); write_mem(_IY+o, _E); break;
      case rl_h:        _H = read_mem(_IY+o); _H = RL(_H); write_mem(_IY+o, _H); break;
      case rl_l:        _L = read_mem(_IY+o); _L = RL(_L); write_mem(_IY+o, _L); break;
      case rl_mhl:      { byte b = read_mem(_IY+o); write_mem(_IY+o, RL(b)); } break;
      case rrc_a:       _A = read_mem(_IY+o); _A = RRC(_A); write_mem(_IY+o, _A); break;
      case rrc_b:       _B = read_mem(_IY+o); _B = RRC(_B); write_mem(_IY+o, _B); break;
      case rrc_c:       _C = read_mem(_IY+o); _C = RRC(_C); write_mem(_IY+o, _C); break;
      case rrc_d:       _D = read_mem(_IY+o); _D = RRC(_D); write_mem(_IY+o, _D); break;
      case rrc_e:       _E = read_mem(_IY+o); _E = RRC(_E); write_mem(_IY+o, _E); break;
      case rrc_h:       _H = read_mem(_IY+o); _H = RRC(_H); write_mem(_IY+o, _H); break;
      case rrc_l:       _L = read_mem(_IY+o); _L = RRC(_L); write_mem(_IY+o, _L); break;
      case rrc_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, RRC(b)); } break;
      case rr_a:        _A = read_mem(_IY+o); _A = RR(_A); write_mem(_IY+o, _A); break;
      case rr_b:        _B = read_mem(_IY+o); _B = RR(_B); write_mem(_IY+o, _B); break;
      case rr_c:        _C = read_mem(_IY+o); _C = RR(_C); write_mem(_IY+o, _C); break;
      case rr_d:        _D = read_mem(_IY+o); _D = RR(_D); write_mem(_IY+o, _D); break;
      case rr_e:        _E = read_mem(_IY+o); _E = RR(_E); write_mem(_IY+o, _E); break;
      case rr_h:        _H = read_mem(_IY+o); _H = RR(_H); write_mem(_IY+o, _H); break;
      case rr_l:        _L = read_mem(_IY+o); _L = RR(_L); write_mem(_IY+o, _L); break;
      case rr_mhl:      { byte b = read_mem(_IY+o); write_mem(_IY+o, RR(b)); } break;
      case set0_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(0, _A)); break;
      case set0_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(0, _B)); break;
      case set0_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(0, _C)); break;
      case set0_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(0, _D)); break;
      case set0_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(0, _E)); break;
      case set0_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(0, _H)); break;
      case set0_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(0, _L)); break;
      case set0_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(0, b)); } break;
      case set1_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(1, _A)); break;
      case set1_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(1, _B)); break;
      case set1_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(1, _C)); break;
      case set1_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(1, _D)); break;
      case set1_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(1, _E)); break;
      case set1_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(1, _H)); break;
      case set1_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(1, _L)); break;
      case set1_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(1, b)); } break;
      case set2_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(2, _A)); break;
      case set2_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(2, _B)); break;
      case set2_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(2, _C)); break;
      case set2_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(2, _D)); break;
      case set2_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(2, _E)); break;
      case set2_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(2, _H)); break;
      case set2_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(2, _L)); break;
      case set2_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(2, b)); } break;
      case set3_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(3, _A)); break;
      case set3_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(3, _B)); break;
      case set3_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(3, _C)); break;
      case set3_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(3, _D)); break;
      case set3_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(3, _E)); break;
      case set3_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(3, _H)); break;
      case set3_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(3, _L)); break;
      case set3_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(3, b)); } break;
      case set4_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(4, _A)); break;
      case set4_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(4, _B)); break;
      case set4_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(4, _C)); break;
      case set4_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(4, _D)); break;
      case set4_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(4, _E)); break;
      case set4_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(4, _H)); break;
      case set4_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(4, _L)); break;
      case set4_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(4, b)); } break;
      case set5_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(5, _A)); break;
      case set5_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(5, _B)); break;
      case set5_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(5, _C)); break;
      case set5_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(5, _D)); break;
      case set5_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(5, _E)); break;
      case set5_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(5, _H)); break;
      case set5_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(5, _L)); break;
      case set5_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(5, b)); } break;
      case set6_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(6, _A)); break;
      case set6_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(6, _B)); break;
      case set6_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(6, _C)); break;
      case set6_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(6, _D)); break;
      case set6_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(6, _E)); break;
      case set6_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(6, _H)); break;
      case set6_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(6, _L)); break;
      case set6_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(6, b)); } break;
      case set7_a:      _A = read_mem(_IY+o); write_mem(_IY+o, _A = SET(7, _A)); break;
      case set7_b:      _B = read_mem(_IY+o); write_mem(_IY+o, _B = SET(7, _B)); break;
      case set7_c:      _C = read_mem(_IY+o); write_mem(_IY+o, _C = SET(7, _C)); break;
      case set7_d:      _D = read_mem(_IY+o); write_mem(_IY+o, _D = SET(7, _D)); break;
      case set7_e:      _E = read_mem(_IY+o); write_mem(_IY+o, _E = SET(7, _E)); break;
      case set7_h:      _H = read_mem(_IY+o); write_mem(_IY+o, _H = SET(7, _H)); break;
      case set7_l:      _L = read_mem(_IY+o); write_mem(_IY+o, _L = SET(7, _L)); break;
      case set7_mhl:    { byte b = read_mem(_IY+o); write_mem(_IY+o, SET(7, b)); } break;
      case sla_a:       _A = read_mem(_IY+o); _A = SLA(_A); write_mem(_IY+o, _A); break;
      case sla_b:       _B = read_mem(_IY+o); _B = SLA(_B); write_mem(_IY+o, _B); break;
      case sla_c:       _C = read_mem(_IY+o); _C = SLA(_C); write_mem(_IY+o, _C); break;
      case sla_d:       _D = read_mem(_IY+o); _D = SLA(_D); write_mem(_IY+o, _D); break;
      case sla_e:       _E = read_mem(_IY+o); _E = SLA(_E); write_mem(_IY+o, _E); break;
      case sla_h:       _H = read_mem(_IY+o); _H = SLA(_H); write_mem(_IY+o, _H); break;
      case sla_l:       _L = read_mem(_IY+o); _L = SLA(_L); write_mem(_IY+o, _L); break;
      case sla_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, SLA(b)); } break;
      case sll_a:       _A = read_mem(_IY+o); _A = SLL(_A); write_mem(_IY+o, _A); break;
      case sll_b:       _B = read_mem(_IY+o); _B = SLL(_B); write_mem(_IY+o, _B); break;
      case sll_c:       _C = read_mem(_IY+o); _C = SLL(_C); write_mem(_IY+o, _C); break;
      case sll_d:       _D = read_mem(_IY+o); _D = SLL(_D); write_mem(_IY+o, _D); break;
      case sll_e:       _E = read_mem(_IY+o); _E = SLL(_E); write_mem(_IY+o, _E); break;
      case sll_h:       _H = read_mem(_IY+o); _H = SLL(_H); write_mem(_IY+o, _H); break;
      case sll_l:       _L = read_mem(_IY+o); _L = SLL(_L); write_mem(_IY+o, _L); break;
      case sll_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, SLL(b)); } break;
      case sra_a:       _A = read_mem(_IY+o); _A = SRA(_A); write_mem(_IY+o, _A); break;
      case sra_b:       _B = read_mem(_IY+o); _B = SRA(_B); write_mem(_IY+o, _B); break;
      case sra_c:       _C = read_mem(_IY+o); _C = SRA(_C); write_mem(_IY+o, _C); break;
      case sra_d:       _D = read_mem(_IY+o); _D = SRA(_D); write_mem(_IY+o, _D); break;
      case sra_e:       _E = read_mem(_IY+o); _E = SRA(_E); write_mem(_IY+o, _E); break;
      case sra_h:       _H = read_mem(_IY+o); _H = SRA(_H); write_mem(_IY+o, _H); break;
      case sra_l:       _L = read_mem(_IY+o); _L = SRA(_L); write_mem(_IY+o, _L); break;
      case sra_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, SRA(b)); } break;
      case srl_a:       _A = read_mem(_IY+o); _A = SRL(_A); write_mem(_IY+o, _A); break;
      case srl_b:       _B = read_mem(_IY+o); _B = SRL(_B); write_mem(_IY+o, _B); break;
      case srl_c:       _C = read_mem(_IY+o); _C = SRL(_C); write_mem(_IY+o, _C); break;
      case srl_d:       _D = read_mem(_IY+o); _D = SRL(_D); write_mem(_IY+o, _D); break;
      case srl_e:       _E = read_mem(_IY+o); _E = SRL(_E); write_mem(_IY+o, _E); break;
      case srl_h:       _H = read_mem(_IY+o); _H = SRL(_H); write_mem(_IY+o, _H); break;
      case srl_l:       _L = read_mem(_IY+o); _L = SRL(_L); write_mem(_IY+o, _L); break;
      case srl_mhl:     { byte b = read_mem(_IY+o); write_mem(_IY+o, SRL(b)); } break;
   }
}
# endif
