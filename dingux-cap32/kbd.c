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
#include <zlib.h>
#include "SDL.h"

#include "cap32.h"
#include "z80.h"
#include "kbd.h"

   dword cpc_kbd[CPC_MAX_KEY] = {
   // original CPC keyboard
       0x40,                   // CPC_0
       0x80,                   // CPC_1
       0x81,                   // CPC_2
       0x71,                   // CPC_3
       0x70,                   // CPC_4
       0x61,                   // CPC_5
       0x60,                   // CPC_6
       0x51,                   // CPC_7
       0x50,                   // CPC_8
       0x41,                   // CPC_9
       0x85 | MOD_CPC_SHIFT,   // CPC_A
       0x66 | MOD_CPC_SHIFT,   // CPC_B
       0x76 | MOD_CPC_SHIFT,   // CPC_C
       0x75 | MOD_CPC_SHIFT,   // CPC_D
       0x72 | MOD_CPC_SHIFT,   // CPC_E
       0x65 | MOD_CPC_SHIFT,   // CPC_F
       0x64 | MOD_CPC_SHIFT,   // CPC_G
       0x54 | MOD_CPC_SHIFT,   // CPC_H
       0x43 | MOD_CPC_SHIFT,   // CPC_I
       0x55 | MOD_CPC_SHIFT,   // CPC_J
       0x45 | MOD_CPC_SHIFT,   // CPC_K
       0x44 | MOD_CPC_SHIFT,   // CPC_L
       0x46 | MOD_CPC_SHIFT,   // CPC_M
       0x56 | MOD_CPC_SHIFT,   // CPC_N
       0x42 | MOD_CPC_SHIFT,   // CPC_O
       0x33 | MOD_CPC_SHIFT,   // CPC_P
       0x83 | MOD_CPC_SHIFT,   // CPC_Q
       0x62 | MOD_CPC_SHIFT,   // CPC_R
       0x74 | MOD_CPC_SHIFT,   // CPC_S
       0x63 | MOD_CPC_SHIFT,   // CPC_T
       0x52 | MOD_CPC_SHIFT,   // CPC_U
       0x67 | MOD_CPC_SHIFT,   // CPC_V
       0x73 | MOD_CPC_SHIFT,   // CPC_W
       0x77 | MOD_CPC_SHIFT,   // CPC_X
       0x53 | MOD_CPC_SHIFT,   // CPC_Y
       0x87 | MOD_CPC_SHIFT,   // CPC_Z
       0x85,                   // CPC_a
       0x66,                   // CPC_b
       0x76,                   // CPC_c
       0x75,                   // CPC_d
       0x72,                   // CPC_e
       0x65,                   // CPC_f
       0x64,                   // CPC_g
       0x54,                   // CPC_h
       0x43,                   // CPC_i
       0x55,                   // CPC_j
       0x45,                   // CPC_k
       0x44,                   // CPC_l
       0x46,                   // CPC_m
       0x56,                   // CPC_n
       0x42,                   // CPC_o
       0x33,                   // CPC_p
       0x83,                   // CPC_q
       0x62,                   // CPC_r
       0x74,                   // CPC_s
       0x63,                   // CPC_t
       0x52,                   // CPC_u
       0x67,                   // CPC_v
       0x73,                   // CPC_w
       0x77,                   // CPC_x
       0x53,                   // CPC_y
       0x87,                   // CPC_z
       0x60 | MOD_CPC_SHIFT,   // CPC_AMPERSAND
       0x35 | MOD_CPC_SHIFT,   // CPC_ASTERISK
       0x32,                   // CPC_AT
       0x26 | MOD_CPC_SHIFT,   // CPC_BACKQUOTE
       0x26,                   // CPC_BACKSLASH
       0x86,                   // CPC_CAPSLOCK
       0x20,                   // CPC_CLR
       0x35,                   // CPC_COLON
       0x47,                   // CPC_COMMA
       0x27 | MOD_CPC_CTRL,    // CPC_CONTROL
       0x11,                   // CPC_COPY
       0x02 | MOD_CPC_SHIFT,   // CPC_CPY_DOWN
       0x10 | MOD_CPC_SHIFT,   // CPC_CPY_LEFT
       0x01 | MOD_CPC_SHIFT,   // CPC_CPY_RIGHT
       0x00 | MOD_CPC_SHIFT,   // CPC_CPY_UP
       0x02,                   // CPC_CUR_DOWN
       0x10,                   // CPC_CUR_LEFT
       0x01,                   // CPC_CUR_RIGHT
       0x00,                   // CPC_CUR_UP
       0x02 | MOD_CPC_CTRL,    // CPC_CUR_ENDBL
       0x10 | MOD_CPC_CTRL,    // CPC_CUR_HOMELN
       0x01 | MOD_CPC_CTRL,    // CPC_CUR_ENDLN
       0x00 | MOD_CPC_CTRL,    // CPC_CUR_HOMEBL
       0x81 | MOD_CPC_SHIFT,   // CPC_DBLQUOTE
       0x97,                   // CPC_DEL
       0x70 | MOD_CPC_SHIFT,   // CPC_DOLLAR
       0x06,                   // CPC_ENTER
       0x31 | MOD_CPC_SHIFT,   // CPC_EQUAL
       0x82,                   // CPC_ESC
       0x80 | MOD_CPC_SHIFT,   // CPC_EXCLAMATN
       0x17,                   // CPC_F0
       0x15,                   // CPC_F1
       0x16,                   // CPC_F2
       0x05,                   // CPC_F3
       0x24,                   // CPC_F4
       0x14,                   // CPC_F5
       0x04,                   // CPC_F6
       0x12,                   // CPC_F7
       0x13,                   // CPC_F8
       0x03,                   // CPC_F9
       0x07,                   // CPC_FPERIOD
       0x37 | MOD_CPC_SHIFT,   // CPC_GREATER
       0x71 | MOD_CPC_SHIFT,   // CPC_HASH
       0x21,                   // CPC_LBRACKET
       0x21 | MOD_CPC_SHIFT,   // CPC_LCBRACE
       0x50 | MOD_CPC_SHIFT,   // CPC_LEFTPAREN
       0x47 | MOD_CPC_SHIFT,   // CPC_LESS
       0x25 | MOD_CPC_SHIFT,   // CPC_LSHIFT
       0x31,                   // CPC_MINUS
       0x61 | MOD_CPC_SHIFT,   // CPC_PERCENT
       0x37,                   // CPC_PERIOD
       0x32 | MOD_CPC_SHIFT,   // CPC_PIPE
       0x34 | MOD_CPC_SHIFT,   // CPC_PLUS
       0x30 | MOD_CPC_SHIFT,   // CPC_POUND
       0x30,                   // CPC_POWER
       0x36 | MOD_CPC_SHIFT,   // CPC_QUESTION
       0x51 | MOD_CPC_SHIFT,   // CPC_QUOTE
       0x23,                   // CPC_RBRACKET
       0x23 | MOD_CPC_SHIFT,   // CPC_RCBRACE
       0x22,                   // CPC_RETURN
       0x41 | MOD_CPC_SHIFT,   // CPC_RIGHTPAREN
       0x25 | MOD_CPC_SHIFT,   // CPC_RSHIFT
       0x34,                   // CPC_SEMICOLON
       0x36,                   // CPC_SLASH
       0x57,                   // CPC_SPACE
       0x84,                   // CPC_TAB
       0x40 | MOD_CPC_SHIFT,   // CPC_UNDERSCORE
       0x90,                   // CPC_J0_UP
       0x91,                   // CPC_J0_DOWN
       0x92,                   // CPC_J0_LEFT
       0x93,                   // CPC_J0_RIGHT
       0x94,                   // CPC_J0_FIRE1
       0x95,                   // CPC_J0_FIRE2
       0x60,                   // CPC_J1_UP
       0x61,                   // CPC_J1_DOWN
       0x62,                   // CPC_J1_LEFT
       0x63,                   // CPC_J1_RIGHT
       0x64,                   // CPC_J1_FIRE1
       0x65,                   // CPC_J1_FIRE2
       0xff,                   // CPC_ES_NTILDE
       0xff,                   // CPC_ES_nTILDE
       0xff,                   // CPC_ES_PESETA
       0xff,                   // CPC_FR_eACUTE
       0xff,                   // CPC_FR_eGRAVE
       0xff,                   // CPC_FR_cCEDIL
       0xff,                   // CPC_FR_aGRAVE
       0xff,                   // CPC_FR_uGRAVE
       0xff,                   // CAP32_FPS,
       0xff,                   // CAP32_JOY,
       0xff,                   // CAP32_RENDER,
       0xff,                   // CAP32_LOAD,
       0xff,                   // CAP32_SAVE,
       0xff,                   // CAP32_RESET,
       0xff,                   // CAP32_AUTOFIRE
       0xff,                   // CAP32_INCFIRE 
       0xff,                   // CAP32_DECFIRE 
       0xff                    // CAP32_SCREEN
  };

   int kbd_layout[KBD_MAX_ENTRIES][2] = {
   // US PC to CPC keyboard layout translation
    { CPC_0,          '0' },
    { CPC_1,          '1' },
    { CPC_2,          '2' },
    { CPC_3,          '3' },
    { CPC_4,          '4' },
    { CPC_5,          '5' },
    { CPC_6,          '6' },
    { CPC_7,          '7' },
    { CPC_8,          '8' },
    { CPC_9,          '9' },
    { CPC_A,          'A' },
    { CPC_B,          'B' },
    { CPC_C,          'C' },
    { CPC_D,          'D' },
    { CPC_E,          'E' },
    { CPC_F,          'F' },
    { CPC_G,          'G' },
    { CPC_H,          'H' },
    { CPC_I,          'I' },
    { CPC_J,          'J' },
    { CPC_K,          'K' },
    { CPC_L,          'L' },
    { CPC_M,          'M' },
    { CPC_N,          'N' },
    { CPC_O,          'O' },
    { CPC_P,          'P' },
    { CPC_Q,          'Q' },
    { CPC_R,          'R' },
    { CPC_S,          'S' },
    { CPC_T,          'T' },
    { CPC_U,          'U' },
    { CPC_V,          'V' },
    { CPC_W,          'W' },
    { CPC_X,          'X' },
    { CPC_Y,          'Y' },
    { CPC_Z,          'Z' },
    { CPC_a,          'a' },
    { CPC_b,          'b' },
    { CPC_c,          'c' },
    { CPC_d,          'd' },
    { CPC_e,          'e' },
    { CPC_f,          'f' },
    { CPC_g,          'g' },
    { CPC_h,          'h' },
    { CPC_i,          'i' },
    { CPC_j,          'j' },
    { CPC_k,          'k' },
    { CPC_l,          'l' },
    { CPC_m,          'm' },
    { CPC_n,          'n' },
    { CPC_o,          'o' },
    { CPC_p,          'p' },
    { CPC_q,          'q' },
    { CPC_r,          'r' },
    { CPC_s,          's' },
    { CPC_t,          't' },
    { CPC_u,          'u' },
    { CPC_v,          'v' },
    { CPC_w,          'w' },
    { CPC_x,          'x' },
    { CPC_y,          'y' },
    { CPC_z,          'z' },
    { CPC_AMPERSAND,  '&' },
    { CPC_ASTERISK,   '*' },
    { CPC_AT,         '@' },
    { CPC_BACKQUOTE,  '`' },
    { CPC_BACKSLASH,  '\\' },
    { CPC_CAPSLOCK,   DANZEFF_CAPSLOCK },
    { CPC_CLR,        DANZEFF_CLR      },
    { CPC_COLON,      ':' },
    { CPC_COMMA,      ',' },
    { CPC_CONTROL,    DANZEFF_CONTROL  },
    { CPC_COPY,       DANZEFF_COPY     },
    { CPC_CPY_DOWN,   -1  },
    { CPC_CPY_LEFT,   -1  },
    { CPC_CPY_RIGHT,  -1  },
    { CPC_CPY_UP,     -1  },
    { CPC_CUR_DOWN,   DANZEFF_DOWN  },
    { CPC_CUR_LEFT,   DANZEFF_LEFT  },
    { CPC_CUR_RIGHT,  DANZEFF_RIGHT },
    { CPC_CUR_UP,     DANZEFF_UP    },
    { CPC_CUR_ENDBL,  DANZEFF_ENDBL  },
    { CPC_CUR_HOMELN, DANZEFF_HOMELN },
    { CPC_CUR_ENDLN,  DANZEFF_ENDLN  },
    { CPC_CUR_HOMEBL, DANZEFF_HOMEBL },
    { CPC_DBLQUOTE,   '"' },
    { CPC_DEL,        DANZEFF_DEL   },
    { CPC_DOLLAR,     '$' },
    { CPC_ENTER,      DANZEFF_ENTER },
    { CPC_EQUAL,      '=' },
    { CPC_ESC,        DANZEFF_ESC   },
    { CPC_EXCLAMATN,  '!' },
    { CPC_F0,         DANZEFF_F0 },
    { CPC_F1,         DANZEFF_F1 },
    { CPC_F2,         DANZEFF_F2 },
    { CPC_F3,         DANZEFF_F3 },
    { CPC_F4,         DANZEFF_F4 },
    { CPC_F5,         DANZEFF_F5 },
    { CPC_F6,         DANZEFF_F6 },
    { CPC_F7,         DANZEFF_F7 },
    { CPC_F8,         DANZEFF_F8 },
    { CPC_F9,         DANZEFF_F9 },
    { CPC_FPERIOD,    '.' },
    { CPC_GREATER,    '>' },
    { CPC_HASH,       '#' },
    { CPC_LBRACKET,   '[' },
    { CPC_LCBRACE,    '{' },
    { CPC_LEFTPAREN,  '(' },
    { CPC_LESS,       '<' },
    { CPC_LSHIFT,     DANZEFF_LSHIFT },
    { CPC_MINUS,      '-' },
    { CPC_PERCENT,    '%' },
    { CPC_PERIOD,     '.' },
    { CPC_PIPE,       '|' },
    { CPC_PLUS,       '+' },
    { CPC_POUND,      '$' },
    { CPC_POWER,      '^' },
    { CPC_QUESTION,   '?' },
    { CPC_QUOTE,      '\'' },
    { CPC_RBRACKET,   ']' },
    { CPC_RCBRACE,    '}' },
    { CPC_RETURN,     DANZEFF_RETURN  },
    { CPC_RIGHTPAREN, ')' },
    { CPC_RSHIFT,     DANZEFF_RSHIFT  },
    { CPC_SEMICOLON,  ';' },
    { CPC_SLASH,      '/' },
    { CPC_SPACE,      ' ' },
    { CPC_TAB,        DANZEFF_TAB     },
    { CPC_UNDERSCORE, '_'  },

    { CAP32_RESET,    -1 },
    { CAP32_FPS,      -1 },
    { CAP32_JOY,      -1 },
    { CAP32_RENDER,   -1 },
    { CAP32_LOAD,     -1 },
    { CAP32_SAVE,     -1 },
    { CAP32_RESET,    -1 },
    { CAP32_AUTOFIRE, -1 },
    { CAP32_INCFIRE   -1 },
    { CAP32_DECFIRE   -1 },
    { CAP32_SCREEN,   -1 },

    { MOD_CPC_CTRL|CPC_ENTER, DANZEFF_RUN }
  };

  int joy_layout[12][2] = {
    { CPC_J0_UP,      SDLK_UP },
    { CPC_J0_DOWN,    SDLK_DOWN },
    { CPC_J0_LEFT,    SDLK_LEFT },
    { CPC_J0_RIGHT,   SDLK_RIGHT },
    { CPC_J0_FIRE1,   SDLK_z },
    { CPC_J0_FIRE2,   SDLK_x },
    { CPC_J1_UP,      0 },
    { CPC_J1_DOWN,    0 },
    { CPC_J1_LEFT,    0 },
    { CPC_J1_RIGHT,   0 },
    { CPC_J1_FIRE1,   0 },
    { CPC_J1_FIRE2,   0 }
  };

  char *cpc_keys_name[CPC_MAX_KEY] = {
   "0",
   "1",
   "2",
   "3",
   "4",
   "5",
   "6",
   "7",
   "8",
   "9",
   "A",
   "B",
   "C",
   "D",
   "E",
   "F",
   "G",
   "H",
   "I",
   "J",
   "K",
   "L",
   "M",
   "N",
   "O",
   "P",
   "Q",
   "R",
   "S",
   "T",
   "U",
   "V",
   "W",
   "X",
   "Y",
   "Z",
   "a",
   "b",
   "c",
   "d",
   "e",
   "f",
   "g",
   "h",
   "i",
   "j",
   "k",
   "l",
   "m",
   "n",
   "o",
   "p",
   "q",
   "r",
   "s",
   "t",
   "u",
   "v",
   "w",
   "x",
   "y",
   "z",
   "&",
   "*",
   "@",
   "`",
   "/",
   "CAPSLOCK",
   "CLR",
   ":",
   ",",
   "CONTROL",
   "COPY",
   "CPY_DOWN",
   "CPY_LEFT",
   "CPY_RIGHT",
   "CPY_UP",
   "CUR_DOWN",
   "CUR_LEFT",
   "CUR_RIGHT",
   "CUR_UP",
   "CUR_ENDBL",
   "CUR_HOMELN",
   "CUR_ENDLN",
   "CUR_HOMEBL",
   "\"",
   "DEL",
   "$",
   "ENTER",
   "=",
   "ESC",
   "!",
   "F0",
   "F1",
   "F2",
   "F3",
   "F4",
   "F5",
   "F6",
   "F7",
   "F8",
   "F9",
   "FPERIOD",
   ">",
   "#",
   "[",
   "{",
   "(",
   "<",
   "LSHIFT",
   "-",
   "%",
   ".",
   "|",
   "+",
   "$",
   "^",
   "?",
   "\'",
   "]",
   "}",
   "RETURN",
   ")",
   "RSHIFT",
   ";",
   "/",
   "SPACE",
   "TAB",
   "_",
   "J0_UP",
   "J0_DOWN",
   "J0_LEFT",
   "J0_RIGHT",
   "J0_FIRE1",
   "J0_FIRE2",
   "J1_UP",
   "J1_DOWN",
   "J1_LEFT",
   "J1_RIGHT",
   "J1_FIRE1",
   "J1_FIRE2",
   "ES_NTILDE",
   "ES_nTILDE",
   "ES_PESETA",
   "FR_eACUTE",
   "FR_eGRAVE",
   "FR_cCEDIL",
   "FR_aGRAVE",
   "FR_uGRAVE",
   "C_FPS",
   "C_JOY",
   "C_RENDER",
   "C_LOAD",
   "C_SAVE",
   "C_RESET",
   "C_AUTOFIRE",
   "C_INCFIRE",
   "C_DECFIRE",
   "C_SCREEN"
  };

  extern byte keyboard_matrix[16];

  byte bit_values[8] = {
     0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
  };


int
cpc_get_key_from_ascii(char ascii)
{
  int index;
  for (index = 0; index < KBD_MAX_ENTRIES; index++) {
   if (kbd_layout[index][1] == ascii) return kbd_layout[index][0];
  }
  return -1;
}

int 
cpc_kbd_init(void)
{
   return 0;
}

int
cpc_kbd_reset(void)
{
  dword cpc_key;
  int   cpc_idx;

  for (cpc_idx = 0; cpc_idx < KBD_MAX_ENTRIES; cpc_idx++) {
    cpc_key = cpc_kbd[cpc_idx];
    keyboard_matrix[(byte)cpc_key >> 4] |= bit_values[(byte)cpc_key & 7]; // key has been released
    keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
    keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is not held down
  }
  return 0;
}

int
cpc_key_event(int cpc_idx, int button_pressed)
{
  int ctrl_shift_skip = 0;
  dword cpc_key = 0xff;

  if (cpc_idx < CAP32_FPS) {
    cpc_key = cpc_kbd[cpc_idx];

    if ((cpc_idx >= CPC_J0_UP) && (cpc_idx <= CPC_J1_FIRE2)) {
      ctrl_shift_skip = 1;
    }

  } else {
    if (cpc_idx & MOD_CPC_CTRL) {
      cpc_idx &= ~MOD_CPC_CTRL;
      if (cpc_idx < KBD_MAX_ENTRIES) {
        cpc_key = cpc_kbd[cpc_idx] | MOD_CPC_CTRL;
      }
    } else
    if (cpc_idx & MOD_CPC_SHIFT) {
      cpc_idx &= ~MOD_CPC_SHIFT;
      if (cpc_idx < KBD_MAX_ENTRIES) {
        cpc_key = cpc_kbd[cpc_idx] | MOD_CPC_SHIFT;
      }
    } else {
      /* CAP32 keys */
      if (button_pressed) {
        cap32_treat_command_key(cpc_idx);
      }
      return 0;
    }   
  }

  if (button_pressed) {
    if ((byte)cpc_key != 0xff) {
      keyboard_matrix[(byte)cpc_key >> 4] &= ~bit_values[(byte)cpc_key & 7]; // key is being held down
      if (! ctrl_shift_skip) {
        if (cpc_key & MOD_CPC_SHIFT) { // CPC SHIFT key required?
          keyboard_matrix[0x25 >> 4] &= ~bit_values[0x25 & 7]; // key needs to be SHIFTed
        } else {
          keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
        }
        if (cpc_key & MOD_CPC_CTRL) { // CPC CONTROL key required?
          keyboard_matrix[0x27 >> 4] &= ~bit_values[0x27 & 7]; // CONTROL key is held down
        } else {
          keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is released
        }
      }
    }
  } else {
    if ((byte)cpc_key != 0xff) {
      keyboard_matrix[(byte)cpc_key >> 4] |= bit_values[(byte)cpc_key & 7]; // key has been released
      if (! ctrl_shift_skip) {
        keyboard_matrix[0x25 >> 4] |= bit_values[0x25 & 7]; // make sure key is unSHIFTed
        keyboard_matrix[0x27 >> 4] |= bit_values[0x27 & 7]; // make sure CONTROL key is not held down
      }
    }
  }
  return 0;
}
