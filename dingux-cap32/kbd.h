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

#ifndef KBD_H  
#define KBD_H  

#define MOD_CPC_SHIFT   (0x01 << 8)
#define MOD_CPC_CTRL    (0x02 << 8)
#define MOD_EMU_KEY     (0x10 << 8)

typedef enum {
   CPC_0,
   CPC_1,
   CPC_2,
   CPC_3,
   CPC_4,
   CPC_5,
   CPC_6,
   CPC_7,
   CPC_8,
   CPC_9,
   CPC_A,
   CPC_B,
   CPC_C,
   CPC_D,
   CPC_E,
   CPC_F,
   CPC_G,
   CPC_H,
   CPC_I,
   CPC_J,
   CPC_K,
   CPC_L,
   CPC_M,
   CPC_N,
   CPC_O,
   CPC_P,
   CPC_Q,
   CPC_R,
   CPC_S,
   CPC_T,
   CPC_U,
   CPC_V,
   CPC_W,
   CPC_X,
   CPC_Y,
   CPC_Z,
   CPC_a,
   CPC_b,
   CPC_c,
   CPC_d,
   CPC_e,
   CPC_f,
   CPC_g,
   CPC_h,
   CPC_i,
   CPC_j,
   CPC_k,
   CPC_l,
   CPC_m,
   CPC_n,
   CPC_o,
   CPC_p,
   CPC_q,
   CPC_r,
   CPC_s,
   CPC_t,
   CPC_u,
   CPC_v,
   CPC_w,
   CPC_x,
   CPC_y,
   CPC_z,
   CPC_AMPERSAND,
   CPC_ASTERISK,
   CPC_AT,
   CPC_BACKQUOTE,
   CPC_BACKSLASH,
   CPC_CAPSLOCK,
   CPC_CLR,
   CPC_COLON,
   CPC_COMMA,
   CPC_CONTROL,
   CPC_COPY,
   CPC_CPY_DOWN,
   CPC_CPY_LEFT,
   CPC_CPY_RIGHT,
   CPC_CPY_UP,
   CPC_CUR_DOWN,
   CPC_CUR_LEFT,
   CPC_CUR_RIGHT,
   CPC_CUR_UP,
   CPC_CUR_ENDBL,
   CPC_CUR_HOMELN,
   CPC_CUR_ENDLN,
   CPC_CUR_HOMEBL,
   CPC_DBLQUOTE,
   CPC_DEL,
   CPC_DOLLAR,
   CPC_ENTER,
   CPC_EQUAL,
   CPC_ESC,
   CPC_EXCLAMATN,
   CPC_F0,
   CPC_F1,
   CPC_F2,
   CPC_F3,
   CPC_F4,
   CPC_F5,
   CPC_F6,
   CPC_F7,
   CPC_F8,
   CPC_F9,
   CPC_FPERIOD,
   CPC_GREATER,
   CPC_HASH,
   CPC_LBRACKET,
   CPC_LCBRACE,
   CPC_LEFTPAREN,
   CPC_LESS,
   CPC_LSHIFT,
   CPC_MINUS,
   CPC_PERCENT,
   CPC_PERIOD,
   CPC_PIPE,
   CPC_PLUS,
   CPC_POUND,
   CPC_POWER,
   CPC_QUESTION,
   CPC_QUOTE,
   CPC_RBRACKET,
   CPC_RCBRACE,
   CPC_RETURN,
   CPC_RIGHTPAREN,
   CPC_RSHIFT,
   CPC_SEMICOLON,
   CPC_SLASH,
   CPC_SPACE,
   CPC_TAB,
   CPC_UNDERSCORE,
   CPC_J0_UP,
   CPC_J0_DOWN,
   CPC_J0_LEFT,
   CPC_J0_RIGHT,
   CPC_J0_FIRE1,
   CPC_J0_FIRE2,
   CPC_J1_UP,
   CPC_J1_DOWN,
   CPC_J1_LEFT,
   CPC_J1_RIGHT,
   CPC_J1_FIRE1,
   CPC_J1_FIRE2,
   CPC_ES_NTILDE,
   CPC_ES_nTILDE,
   CPC_ES_PESETA,
   CPC_FR_eACUTE,
   CPC_FR_eGRAVE,
   CPC_FR_cCEDIL,
   CPC_FR_aGRAVE,
   CPC_FR_uGRAVE,
   CAP32_FPS,
   CAP32_JOY,
   CAP32_RENDER,
   CAP32_LOAD,
   CAP32_SAVE,
   CAP32_RESET,
   CAP32_AUTOFIRE,
   CAP32_INCFIRE,
   CAP32_DECFIRE,
   CAP32_SCREEN,
} CPC_KEYS;

# define  CPC_MAX_KEY (CAP32_SCREEN+1)

  extern char *cpc_keys_name[CPC_MAX_KEY];
  extern dword cpc_kbd[CPC_MAX_KEY];

#define MOD_PC_SHIFT    (KMOD_SHIFT << 16)
#define MOD_PC_CTRL     (KMOD_CTRL << 16)
#define MOD_PC_MODE     (KMOD_MODE << 16)

#define KBD_MAX_ENTRIES 143

#define  DANZEFF_LEFT          -5
#define  DANZEFF_RIGHT         -4
#define  DANZEFF_UP            -3
#define  DANZEFF_DOWN          -2

#define  DANZEFF_SELECT          1
#define  DANZEFF_START           2

#define  DANZEFF_CONTROL         3
#define  DANZEFF_COPY            4
#define  DANZEFF_ENDBL           5
#define  DANZEFF_HOMELN          6
#define  DANZEFF_ENDLN           7
#define  DANZEFF_HOMEBL          8
#define  DANZEFF_DEL             9
#define  DANZEFF_ENTER          10
#define  DANZEFF_ESC            11
#define  DANZEFF_F0             12
#define  DANZEFF_F1             13
#define  DANZEFF_F2             14
#define  DANZEFF_F3             15
#define  DANZEFF_F4             16
#define  DANZEFF_F5             17
#define  DANZEFF_F6             18
#define  DANZEFF_F7             19
#define  DANZEFF_F8             20
#define  DANZEFF_F9             21
#define  DANZEFF_LSHIFT         22
#define  DANZEFF_RETURN         23
#define  DANZEFF_RSHIFT         24
#define  DANZEFF_TAB            25
#define  DANZEFF_RUN            26
#define  DANZEFF_CAPSLOCK       27
#define  DANZEFF_CLR            28


  extern int kbd_layout[KBD_MAX_ENTRIES][2];
  extern byte keyboard_matrix[16];
  extern byte bit_values[8];

  extern dword keyboard_normal[SDLK_LAST];
  extern dword keyboard_shift[SDLK_LAST];
  extern dword keyboard_ctrl[SDLK_LAST];
  extern dword keyboard_mode[SDLK_LAST];

  extern int joy_layout[12][2];

  extern int cpc_kbd_init(void);
  extern int cpc_kbd_reset(void);
  extern int cpc_key_event(int cpc_key, int button_pressed);
  extern void cpc_kbd_swap_joy (void);

  extern int cpc_get_key_from_ascii(char ascii);

#endif
