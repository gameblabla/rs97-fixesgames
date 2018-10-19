/*
 *  Copyright (C) 2009 Ludovic Jacomme (ludovic.jacomme@gmail.com)
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <SDL.h>

#include "global.h"
#include "atari.h"
#include "input.h"
#include "psp_kbd.h"
#include "psp_menu.h"
#include "psp_sdl.h"
#include "psp_danzeff.h"

# define KBD_MIN_ANALOG_TIME  150000
# define KBD_MIN_START_TIME   800000
# define KBD_MAX_EVENT_TIME   500000
# define KBD_MIN_PENDING_TIME 300000
# define KBD_MIN_HOTKEY_TIME  1000000
# define KBD_MIN_DANZEFF_TIME 150000
# define KBD_MIN_COMMAND_TIME 100000
# define KBD_MIN_BATTCHECK_TIME 90000000
# define KBD_MIN_AUTOFIRE_TIME   1000000

 static gp2xCtrlData    loc_button_data;
 static unsigned int   loc_last_event_time = 0;
 static unsigned int   loc_last_hotkey_time = 0;
 static long           first_time_stamp = -1;
 static long           first_time_auto_stamp = -1;
 static char           loc_button_press[ KBD_MAX_BUTTONS ];
 static char           loc_button_release[ KBD_MAX_BUTTONS ];
 static unsigned int   loc_button_mask[ KBD_MAX_BUTTONS ] =
 {
   GP2X_CTRL_UP         , /*  KBD_UP         */
   GP2X_CTRL_RIGHT      , /*  KBD_RIGHT      */
   GP2X_CTRL_DOWN       , /*  KBD_DOWN       */
   GP2X_CTRL_LEFT       , /*  KBD_LEFT       */
   GP2X_CTRL_TRIANGLE   , /*  KBD_TRIANGLE   */
   GP2X_CTRL_CIRCLE     , /*  KBD_CIRCLE     */
   GP2X_CTRL_CROSS      , /*  KBD_CROSS      */
   GP2X_CTRL_SQUARE     , /*  KBD_SQUARE     */
   GP2X_CTRL_SELECT     , /*  KBD_SELECT     */
   GP2X_CTRL_START      , /*  KBD_START      */
   GP2X_CTRL_LTRIGGER   , /*  KBD_LTRIGGER   */
   GP2X_CTRL_RTRIGGER   , /*  KBD_RTRIGGER   */
 };

 char kbd_button_name[ KBD_ALL_BUTTONS ][20] =
 {
   "UP",
   "RIGHT",
   "DOWN",
   "LEFT",
# if defined(DINGUX_MODE) || defined(GCW0_MODE)
   "X",      // Triangle
   "A",      // Circle
   "B",      // Cross
   "Y",      // Square
# else
   "Y",      // Triangle
   "B",      // Circle
   "X",      // Cross
   "A",      // Square
# endif
   "SELECT",
   "START",
   "LTRIGGER",
   "RTRIGGER",
   "JOY_UP",
   "JOY_RIGHT",
   "JOY_DOWN",
   "JOY_LEFT"
 };

 static char kbd_button_name_L[ KBD_ALL_BUTTONS ][20] =
 {
   "L_UP",
   "L_RIGHT",
   "L_DOWN",
   "L_LEFT",
# if defined(DINGUX_MODE) || defined(GCW0_MODE)
   "L_X",      // Triangle
   "L_A",      // Circle
   "L_B",      // Cross
   "L_Y",      // Square
# else
   "L_Y",      // Triangle
   "L_B",      // Circle
   "L_X",      // Cross
   "L_A",      // Square
# endif
   "L_SELECT",
   "L_START",
   "L_LTRIGGER",
   "L_RTRIGGER",
   "L_JOY_UP",
   "L_JOY_RIGHT",
   "L_JOY_DOWN",
   "L_JOY_LEFT"
 };

  static char kbd_button_name_R[ KBD_ALL_BUTTONS ][20] =
 {
   "R_UP",
   "R_RIGHT",
   "R_DOWN",
   "R_LEFT",
# if defined(DINGUX_MODE) || defined(GCW0_MODE)
   "R_X",      // Triangle
   "R_A",      // Circle
   "R_B",      // Cross
   "R_Y",      // Square
# else
   "R_Y",      // Triangle
   "R_B",      // Circle
   "R_X",      // Cross
   "R_A",      // Square
# endif
   "R_SELECT",
   "R_START",
   "R_LTRIGGER",
   "R_RTRIGGER",
   "R_JOY_UP",
   "R_JOY_RIGHT",
   "R_JOY_DOWN",
   "R_JOY_LEFT"
 };

  struct atari_key_trans psp_atari_key_info[ATARIK_MAX_KEY]=
  {
    // ATARIK            MASK  SHIFT  NAME
    { ATARIK_UNDERSCORE, AKEY_UNDERSCORE, 1,     "_" },
    { ATARIK_1,          AKEY_1,          0,     "1" },
    { ATARIK_2,          AKEY_2,          0,     "2" },
    { ATARIK_3,          AKEY_3,          0,     "3" },
    { ATARIK_4,          AKEY_4,          0,     "4" },
    { ATARIK_5,          AKEY_5,          0,     "5" },
    { ATARIK_6,          AKEY_6,          0,     "6" },
    { ATARIK_7,          AKEY_7,          0,     "7" },
    { ATARIK_8,          AKEY_8,          0,     "8" },
    { ATARIK_9,          AKEY_9,          0,     "9" },
    { ATARIK_0,          AKEY_0,          0,     "0" },
    { ATARIK_SEMICOLON,  AKEY_SEMICOLON,  0,     ";" },
    { ATARIK_MINUS    ,  AKEY_MINUS    ,  0,     "-" },
    { ATARIK_DELETE,     AKEY_BACKSPACE,  0,     "DELETE" },
    { ATARIK_EXCLAMATN,  AKEY_EXCLAMATION,  1,     "!" },
    { ATARIK_DBLQUOTE,   AKEY_DBLQUOTE,   1,     "\"" },
    { ATARIK_HASH,       AKEY_HASH,       1,     "#" },
    { ATARIK_DOLLAR,     AKEY_DOLLAR,     1,     "$" },
    { ATARIK_PERCENT,    AKEY_PERCENT,    1,     "%" },
    { ATARIK_AMPERSAND,  AKEY_AMPERSAND,  1,     "&" },
    { ATARIK_QUOTE,      AKEY_QUOTE,      0,     "'" },
    { ATARIK_LEFTPAREN,  AKEY_PARENLEFT,  1,     "(" },
    { ATARIK_RIGHTPAREN, AKEY_PARENRIGHT, 1,     ")" },
    { ATARIK_PLUS,       AKEY_PLUS,       1,     "+" },
    { ATARIK_EQUAL,      AKEY_EQUAL,      0,     "=" },
    { ATARIK_TAB,        AKEY_TAB,        0,     "TAB  " },
    { ATARIK_a,          AKEY_a,          0,     "a" },
    { ATARIK_b,          AKEY_b,          0,     "b" },
    { ATARIK_c,          AKEY_c,          0,     "c" },
    { ATARIK_d,          AKEY_d,          0,     "d" },
    { ATARIK_e,          AKEY_e,          0,     "e" },
    { ATARIK_f,          AKEY_f,          0,     "f" },
    { ATARIK_g,          AKEY_g,          0,     "g" },
    { ATARIK_h,          AKEY_h,          0,     "h" },
    { ATARIK_i,          AKEY_i,          0,     "i" },
    { ATARIK_j,          AKEY_j,          0,     "j" },
    { ATARIK_k,          AKEY_k,          0,     "k" },
    { ATARIK_l,          AKEY_l,          0,     "l" },
    { ATARIK_m,          AKEY_m,          0,     "m" },
    { ATARIK_n,          AKEY_n,          0,     "n" },
    { ATARIK_o,          AKEY_o,          0,     "o" },
    { ATARIK_p,          AKEY_p,          0,     "p" },
    { ATARIK_q,          AKEY_q,          0,     "q" },
    { ATARIK_r,          AKEY_r,          0,     "r" },
    { ATARIK_s,          AKEY_s,          0,     "s" },
    { ATARIK_t,          AKEY_t,          0,     "t" },
    { ATARIK_u,          AKEY_u,          0,     "u" },
    { ATARIK_v,          AKEY_v,          0,     "v" },
    { ATARIK_w,          AKEY_w,          0,     "w" },
    { ATARIK_x,          AKEY_x,          0,     "x" },
    { ATARIK_y,          AKEY_y,          0,     "y" },
    { ATARIK_z,          AKEY_z,          0,     "z" },
    { ATARIK_A,          AKEY_A,          1,     "A" },
    { ATARIK_B,          AKEY_B,          1,     "B" },
    { ATARIK_C,          AKEY_C,          1,     "C" },
    { ATARIK_D,          AKEY_D,          1,     "D" },
    { ATARIK_E,          AKEY_E,          1,     "E" },
    { ATARIK_F,          AKEY_F,          1,     "F" },
    { ATARIK_G,          AKEY_G,          1,     "G" },
    { ATARIK_H,          AKEY_H,          1,     "H" },
    { ATARIK_I,          AKEY_I,          1,     "I" },
    { ATARIK_J,          AKEY_J,          1,     "J" },
    { ATARIK_K,          AKEY_K,          1,     "K" },
    { ATARIK_L,          AKEY_L,          1,     "L" },
    { ATARIK_M,          AKEY_M,          1,     "M" },
    { ATARIK_N,          AKEY_N,          1,     "N" },
    { ATARIK_O,          AKEY_O,          1,     "O" },
    { ATARIK_P,          AKEY_P,          1,     "P" },
    { ATARIK_Q,          AKEY_Q,          1,     "Q" },
    { ATARIK_R,          AKEY_R,          1,     "R" },
    { ATARIK_S,          AKEY_S,          1,     "S" },
    { ATARIK_T,          AKEY_T,          1,     "T" },
    { ATARIK_U,          AKEY_U,          1,     "U" },
    { ATARIK_V,          AKEY_V,          1,     "V" },
    { ATARIK_W,          AKEY_W,          1,     "W" },
    { ATARIK_X,          AKEY_X,          1,     "X" },
    { ATARIK_Y,          AKEY_Y,          1,     "Y" },
    { ATARIK_Z,          AKEY_Z,          1,     "Z" },
    { ATARIK_RETURN,     AKEY_RETURN,     0,     "RETURN" },
    { ATARIK_CTRL_L,     AKEY_CTRL_L,     0,     "CTRL_L" },
    { ATARIK_CTRL_R,     AKEY_CTRL_R,     0,     "CTRL_R" },
    { ATARIK_SHIFT,      AKEY_SHFT,       0,     "SHIFT" },
    { ATARIK_CAPSLOCK,   AKEY_CAPSLOCK,   0,     "CAPSLOCK" },
    { ATARIK_ESC,        AKEY_ESCAPE,     0,     "ESC" },
    { ATARIK_SPACE,      AKEY_SPACE,      0,     "SPACE" },
    { ATARIK_LEFT,       AKEY_LEFT,       0,     "LEFT" },
    { ATARIK_UP,         AKEY_UP,         0,     "UP" },
    { ATARIK_RIGHT,      AKEY_RIGHT,      0,     "RIGHT" },
    { ATARIK_DOWN,       AKEY_DOWN,       0,     "DOWN" },
    { ATARIK_F1,         AKEY_F1,         0,     "F1" },
    { ATARIK_F2,         AKEY_F2,         0,     "F2" },
    { ATARIK_F3,         AKEY_F3,         0,     "F3" },
    { ATARIK_F4,         AKEY_F4,         0,     "F4" },
    { ATARIK_AT,         AKEY_AT,         1,     "@" },
    { ATARIK_COLON,      AKEY_COLON,      1,     ":" },
    { ATARIK_COMMA,      AKEY_COMMA,      0,     "," },
    { ATARIK_PERIOD,     AKEY_FULLSTOP,   0,     "." },
    { ATARIK_SLASH,      AKEY_SLASH,      0,     "/" },
    { ATARIK_ASTERISK,   AKEY_ASTERISK,   1,     "*" },
    { ATARIK_LESS,       AKEY_LESS,       1,     "<" },
    { ATARIK_GREATER,    AKEY_GREATER,    1,     ">" },
    { ATARIK_QUESTION,   AKEY_QUESTION,   1,     "?" },
    { ATARIK_PIPE,       AKEY_BAR,        1,     "|" },
    { ATARIK_RBRACKET,   AKEY_BRACKETRIGHT,   0,     "]" },
    { ATARIK_LBRACKET,   AKEY_BRACKETLEFT,   0,     "[" },
    { ATARIK_BACKSLASH,  AKEY_BACKSLASH,  0,     "\\" },
    { ATARIK_POWER,      AKEY_CARET,      1,     "^" },
    { ATARIK_SUPPR,      AKEY_DELETE_CHAR,0,     "SUPPR" },

    { ATARIK_HELP,       AKEY_HELP,       0,     "HELP" },
    { ATARIK_ATARI,      AKEY_ATARI,      0,     "ATARI" },
    { ATARIK_CLEAR,      AKEY_CLEAR,      0,     "CLEAR" },
    { ATARIK_START,      AKEY_5200_START, 0,     "5200_START" },
    { ATARIK_PAUSE,      AKEY_5200_PAUSE, 0,     "5200_PAUSE" },
    { ATARIK_RESET,      AKEY_5200_RESET, 0,     "5200_RESET" },
    { ATARIK_UI,         AKEY_UI        , 0,     "UI" },
    { ATARIK_COLDSTART,  AKEY_COLDSTART , 0,     "COLDSTART" },
    { ATARIK_WARMSTART,  AKEY_WARMSTART , 0,     "WARMSTART" },
    { ATARIK_EXIT     ,  AKEY_EXIT      , 0,     "EXIT"      },

    { ATARIK_JOY_UP,     AKEY_UP,     0,     "JOY_UP" },
    { ATARIK_JOY_DOWN,   AKEY_DOWN,   0,     "JOY_DOWN" },
    { ATARIK_JOY_LEFT,   AKEY_LEFT,   0,     "JOY_LEFT" },
    { ATARIK_JOY_RIGHT,  AKEY_RIGHT,  0,     "JOY_RIGHT" },
    { ATARIK_JOY_FIRE1,  AKEY_SPACE,  0,     "JOY_FIRE1" },
    { ATARIK_JOY_FIRE2,  AKEY_SPACE,  0,     "JOY_FIRE2" },

    { ATARIK_CONSOLE_OPTION,  CONSOL_OPTION,  0, "COPTION" },
    { ATARIK_CONSOLE_SELECT,  CONSOL_SELECT,  0, "CSELECT" },
    { ATARIK_CONSOLE_START,   CONSOL_START,   0, "CSTART"  },

    { ATARIC_FPS,    0, 0, "C_FPS" },
    { ATARIC_JOY,    0, 0, "C_JOY" },
    { ATARIC_RENDER, 0, 0, "C_RENDER" },
    { ATARIC_LOAD,   0, 0, "C_LOAD" },
    { ATARIC_SAVE,   0, 0, "C_SAVE" },
    { ATARIC_RESET,  0, 0, "C_RESET" },
    { ATARIC_AUTOFIRE, 0,0, "C_AUTOFIRE" },
    { ATARIC_INCFIRE, 0,0, "C_INCFIRE" },
    { ATARIC_DECFIRE, 0,0, "C_DECFIRE" },
    { ATARIC_SCREEN, 0, 0, "C_SCREEN" }
  };

  static int loc_default_mapping[ KBD_ALL_BUTTONS ] = {
    ATARIK_JOY_UP          , /*  KBD_UP         */
    ATARIK_JOY_RIGHT       , /*  KBD_RIGHT      */
    ATARIK_JOY_DOWN        , /*  KBD_DOWN       */
    ATARIK_JOY_LEFT        , /*  KBD_LEFT       */
    ATARIK_CONSOLE_START   , /*  KBD_TRIANGLE   */
    ATARIK_CONSOLE_SELECT  , /*  KBD_CIRCLE     */
    ATARIK_JOY_FIRE1       , /*  KBD_CROSS      */
    ATARIK_JOY_FIRE2       , /*  KBD_SQUARE     */
    -1                     , /*  KBD_SELECT     */
    -1                     , /*  KBD_START      */
    KBD_LTRIGGER_MAPPING   , /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING   , /*  KBD_RTRIGGER   */
    ATARIK_UP              , /*  KBD_JOY_UP     */
    ATARIK_RIGHT           , /*  KBD_JOY_RIGHT  */
    ATARIK_DOWN            , /*  KBD_JOY_DOWN   */
    ATARIK_LEFT              /*  KBD_JOY_LEFT   */
  };

  static int loc_default_mapping_L[ KBD_ALL_BUTTONS ] = {
    ATARIK_CONSOLE_START   , /*  KBD_UP         */
    ATARIC_RENDER          , /*  KBD_RIGHT      */
    ATARIK_CONSOLE_SELECT  , /*  KBD_DOWN       */
    ATARIC_RENDER          , /*  KBD_LEFT       */
    ATARIC_LOAD            , /*  KBD_TRIANGLE   */
    ATARIC_JOY             , /*  KBD_CIRCLE     */
    ATARIC_SAVE            , /*  KBD_CROSS      */
    ATARIC_FPS             , /*  KBD_SQUARE     */
    -1                     , /*  KBD_SELECT     */
    -1                     , /*  KBD_START      */
    KBD_LTRIGGER_MAPPING   , /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING   , /*  KBD_RTRIGGER   */
    ATARIK_JOY_UP          , /*  KBD_JOY_UP     */
    ATARIK_JOY_RIGHT       , /*  KBD_JOY_RIGHT  */
    ATARIK_JOY_DOWN        , /*  KBD_JOY_DOWN   */
    ATARIK_JOY_LEFT          /*  KBD_JOY_LEFT   */
  };

  static int loc_default_mapping_R[ KBD_ALL_BUTTONS ] = {
    ATARIK_CONSOLE_OPTION  , /*  KBD_UP         */
    ATARIC_INCFIRE         , /*  KBD_RIGHT      */
    ATARIK_COLDSTART       , /*  KBD_DOWN       */
    ATARIC_DECFIRE         , /*  KBD_LEFT       */
    ATARIK_RETURN          , /*  KBD_TRIANGLE   */
    ATARIK_SPACE           , /*  KBD_CIRCLE     */
    ATARIC_AUTOFIRE        , /*  KBD_CROSS      */
    ATARIK_JOY_FIRE2       , /*  KBD_SQUARE     */
    -1                     , /*  KBD_SELECT     */
    -1                     , /*  KBD_START      */
    KBD_LTRIGGER_MAPPING   , /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING   , /*  KBD_RTRIGGER   */
    ATARIK_JOY_UP          , /*  KBD_JOY_UP     */
    ATARIK_JOY_RIGHT       , /*  KBD_JOY_RIGHT  */
    ATARIK_JOY_DOWN        , /*  KBD_JOY_DOWN   */
    ATARIK_JOY_LEFT          /*  KBD_JOY_LEFT   */
  };

  static int loc_ui_mapping[ KBD_ALL_BUTTONS ] = {
    ATARIK_UP              , /*  KBD_UP         */
    ATARIK_RIGHT           , /*  KBD_RIGHT      */
    ATARIK_DOWN            , /*  KBD_DOWN       */
    ATARIK_LEFT            , /*  KBD_LEFT       */
    ATARIK_RETURN          , /*  KBD_TRIANGLE   */
    ATARIK_SPACE           , /*  KBD_CIRCLE     */
    ATARIK_JOY_FIRE1       , /*  KBD_CROSS      */
    ATARIK_JOY_FIRE2       , /*  KBD_SQUARE     */
    -1                     , /*  KBD_SELECT     */
    -1                     , /*  KBD_START      */
    ATARIK_ESC             , /*  KBD_LTRIGGER   */
    ATARIK_CONSOLE_START   , /*  KBD_RTRIGGER   */
    ATARIK_JOY_UP          , /*  KBD_JOY_UP     */
    ATARIK_JOY_RIGHT       , /*  KBD_JOY_RIGHT  */
    ATARIK_JOY_DOWN        , /*  KBD_JOY_DOWN   */
    ATARIK_JOY_LEFT          /*  KBD_JOY_LEFT   */
  };

# define KBD_MAX_ENTRIES   110

  int kbd_layout[KBD_MAX_ENTRIES][2] = {
    /* Key            Ascii */
    { ATARIK_0,          '0' },
    { ATARIK_1,          '1' },
    { ATARIK_2,          '2' },
    { ATARIK_3,          '3' },
    { ATARIK_4,          '4' },
    { ATARIK_5,          '5' },
    { ATARIK_6,          '6' },
    { ATARIK_7,          '7' },
    { ATARIK_8,          '8' },
    { ATARIK_9,          '9' },
    { ATARIK_A,          'A' },
    { ATARIK_B,          'B' },
    { ATARIK_C,          'C' },
    { ATARIK_D,          'D' },
    { ATARIK_E,          'E' },
    { ATARIK_F,          'F' },
    { ATARIK_G,          'G' },
    { ATARIK_H,          'H' },
    { ATARIK_I,          'I' },
    { ATARIK_J,          'J' },
    { ATARIK_K,          'K' },
    { ATARIK_L,          'L' },
    { ATARIK_M,          'M' },
    { ATARIK_N,          'N' },
    { ATARIK_O,          'O' },
    { ATARIK_P,          'P' },
    { ATARIK_Q,          'Q' },
    { ATARIK_R,          'R' },
    { ATARIK_S,          'S' },
    { ATARIK_T,          'T' },
    { ATARIK_U,          'U' },
    { ATARIK_V,          'V' },
    { ATARIK_W,          'W' },
    { ATARIK_X,          'X' },
    { ATARIK_Y,          'Y' },
    { ATARIK_Z,          'Z' },
    { ATARIK_a,          'a' },
    { ATARIK_b,          'b' },
    { ATARIK_c,          'c' },
    { ATARIK_d,          'd' },
    { ATARIK_e,          'e' },
    { ATARIK_f,          'f' },
    { ATARIK_g,          'g' },
    { ATARIK_h,          'h' },
    { ATARIK_i,          'i' },
    { ATARIK_j,          'j' },
    { ATARIK_k,          'k' },
    { ATARIK_l,          'l' },
    { ATARIK_m,          'm' },
    { ATARIK_n,          'n' },
    { ATARIK_o,          'o' },
    { ATARIK_p,          'p' },
    { ATARIK_q,          'q' },
    { ATARIK_r,          'r' },
    { ATARIK_s,          's' },
    { ATARIK_t,          't' },
    { ATARIK_u,          'u' },
    { ATARIK_v,          'v' },
    { ATARIK_w,          'w' },
    { ATARIK_x,          'x' },
    { ATARIK_y,          'y' },
    { ATARIK_z,          'z' },
    { ATARIK_DELETE,     DANZEFF_DEL },
    { ATARIK_SPACE,      ' '         },
    { ATARIK_F1,         DANZEFF_F1  },
    { ATARIK_F2,         DANZEFF_F2  },
    { ATARIK_F3,         DANZEFF_F3  },
    { ATARIK_F4,         DANZEFF_F4  },
    { ATARIK_CAPSLOCK,   DANZEFF_CAPSLOCK },
    { ATARIK_RETURN,     DANZEFF_RETURN   },
    { ATARIK_SHIFT,      DANZEFF_SHIFT    },
    { ATARIK_TAB,        DANZEFF_TAB      },
    { ATARIK_AMPERSAND,  '&' },
    { ATARIK_ASTERISK,   '*' },
    { ATARIK_AT,         '@' },
    { ATARIK_COLON,      ':' },
    { ATARIK_COMMA,      ',' },
    { ATARIK_CTRL_L,    DANZEFF_CONTROL  },
    { ATARIK_DOWN,       -1  },
    { ATARIK_LEFT,       -1  },
    { ATARIK_RIGHT,      -1  },
    { ATARIK_UP,         -1  },
    { ATARIK_DBLQUOTE,   '"' },
    { ATARIK_QUOTE,      '\'' },
    { ATARIK_DOLLAR,     '$' },
    { ATARIK_EQUAL,      '=' },
    { ATARIK_ESC,        DANZEFF_ESC },
    { ATARIK_UI,         DANZEFF_UI  },
    { ATARIK_EXCLAMATN,  '!' },
    { ATARIK_GREATER,    '>' },
    { ATARIK_HASH,       '#' },
    { ATARIK_LEFTPAREN,  '(' },
    { ATARIK_LESS,       '<' },
    { ATARIK_MINUS,      '-' },
    { ATARIK_PERCENT,    '%' },
    { ATARIK_PERIOD,     '.' },
    { ATARIK_PLUS,       '+' },
    { ATARIK_QUESTION,   '?' },
    { ATARIK_RIGHTPAREN, ')' },
    { ATARIK_SEMICOLON,  ';' },
    { ATARIK_SLASH,      '/' },
    { ATARIK_UNDERSCORE, '_'  },
    { ATARIK_PIPE,       '|' },
    { ATARIK_RBRACKET,   ']' },
    { ATARIK_LBRACKET,   '[' },
    { ATARIK_BACKSLASH,  '\\' },
    { ATARIK_POWER,      '^' },
    { ATARIK_CONSOLE_OPTION, DANZEFF_COPTION },
    { ATARIK_CONSOLE_SELECT, DANZEFF_CSELECT },
    { ATARIK_CONSOLE_START,  DANZEFF_CSTART  }
  };

 int psp_kbd_mapping[ KBD_ALL_BUTTONS ];
 int psp_kbd_mapping_L[ KBD_ALL_BUTTONS ];
 int psp_kbd_mapping_R[ KBD_ALL_BUTTONS ];
 int psp_kbd_presses[ KBD_ALL_BUTTONS ];
 int kbd_ltrigger_mapping_active;
 int kbd_rtrigger_mapping_active;

 static int danzeff_atari_key     = 0;
 static int danzeff_atari_pending = 0;
        int danzeff_mode        = 0;

        int psp_kbd_mode_ui     = 0;

       char command_keys[ 128 ];
 static int command_mode        = 0;
 static int command_index       = 0;
 static int command_size        = 0;
 static int command_atari_pending = 0;
 static int command_atari_key     = 0;

 static int ret_key_code;

extern  int key_shift;
extern  int key_consol;
extern  int key_pressed;
extern  int joystick_states[2][6];

int
atari_key_event(int atari_idx, int button_press)
{
  int index    = 0;
  int bit_mask = 0;

	key_consol = CONSOL_NONE;

  if ((atari_idx >=           0) &&
      (atari_idx < ATARIK_JOY_UP )) {

    key_pressed = button_press;

    if (button_press) ret_key_code = psp_atari_key_info[atari_idx].bit_mask;
    else              ret_key_code = AKEY_NONE;

  } else
  if ((atari_idx >= ATARIK_JOY_UP) &&
      (atari_idx <= ATARIK_JOY_FIRE2)) {

    joystick_states[0][atari_idx - ATARIK_JOY_UP]  = button_press;

  } else
  if ((atari_idx >= ATARIK_CONSOLE_OPTION) &&
      (atari_idx <= ATARIK_CONSOLE_START )) {

    if (button_press) {
      key_consol  &= ~psp_atari_key_info[atari_idx].bit_mask;
    } else {
      key_consol  |= psp_atari_key_info[atari_idx].bit_mask;
    }
  } else
  if ((atari_idx >= ATARIC_FPS) &&
      (atari_idx <= ATARIC_SCREEN)) {

    if (button_press) {
      gp2xCtrlData c;
      gp2xCtrlPeekBufferPositive(&c, 1);
      if ((c.TimeStamp - loc_last_hotkey_time) > KBD_MIN_HOTKEY_TIME)
      {
        loc_last_hotkey_time = c.TimeStamp;
        atari_treat_command_key(atari_idx);
      }
    }
  }
  return 0;
}

int
atari_kbd_reset()
{
  /* TO_BE_DONE ! */
  return 0;
}

int
atari_get_key_from_ascii(int key_ascii)
{
  int index;
  for (index = 0; index < KBD_MAX_ENTRIES; index++) {
   if (kbd_layout[index][1] == key_ascii) return kbd_layout[index][0];
  }
  return -1;
}

void
psp_kbd_run_command_ui()
{
  command_keys[0] = DANZEFF_UI;
  command_keys[1] = 0;
  command_size  = 1;
  command_index = 0;

  command_atari_key     = 0;
  command_atari_pending = 0;
  command_mode          = 1;
}

void
psp_kbd_default_settings()
{
  memcpy(psp_kbd_mapping, loc_default_mapping, sizeof(loc_default_mapping));
  memcpy(psp_kbd_mapping_L, loc_default_mapping_L, sizeof(loc_default_mapping_L));
  memcpy(psp_kbd_mapping_R, loc_default_mapping_R, sizeof(loc_default_mapping_R));
}

int
psp_kbd_reset_hotkeys(void)
{
  int index;
  int key_id;
  for (index = 0; index < KBD_ALL_BUTTONS; index++) {
    key_id = loc_default_mapping[index];
    if ((key_id >= ATARIC_FPS) && (key_id <= ATARIC_SCREEN)) {
      psp_kbd_mapping[index] = key_id;
    }
    key_id = loc_default_mapping_L[index];
    if ((key_id >= ATARIC_FPS) && (key_id <= ATARIC_SCREEN)) {
      psp_kbd_mapping_L[index] = key_id;
    }
    key_id = loc_default_mapping_R[index];
    if ((key_id >= ATARIC_FPS) && (key_id <= ATARIC_SCREEN)) {
      psp_kbd_mapping_R[index] = key_id;
    }
  }
  return 0;
}

int
psp_kbd_load_mapping_file(FILE *KbdFile)
{
  char     Buffer[512];
  char    *Scan;
  int      tmp_mapping[KBD_ALL_BUTTONS];
  int      tmp_mapping_L[KBD_ALL_BUTTONS];
  int      tmp_mapping_R[KBD_ALL_BUTTONS];
  int      atari_key_id = 0;
  int      kbd_id = 0;

  memcpy(tmp_mapping, loc_default_mapping, sizeof(loc_default_mapping));
  memcpy(tmp_mapping_L, loc_default_mapping_L, sizeof(loc_default_mapping_L));
  memcpy(tmp_mapping_R, loc_default_mapping_R, sizeof(loc_default_mapping_R));

  while (fgets(Buffer,512,KbdFile) != (char *)0) {

      Scan = strchr(Buffer,'\n');
      if (Scan) *Scan = '\0';
      /* For this #@$% of windows ! */
      Scan = strchr(Buffer,'\r');
      if (Scan) *Scan = '\0';
      if (Buffer[0] == '#') continue;

      Scan = strchr(Buffer,'=');
      if (! Scan) continue;

      *Scan = '\0';
      atari_key_id = atoi(Scan + 1);

      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,kbd_button_name[kbd_id])) {
          tmp_mapping[kbd_id] = atari_key_id;
          //break;
        }
      }
      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,kbd_button_name_L[kbd_id])) {
          tmp_mapping_L[kbd_id] = atari_key_id;
          //break;
        }
      }
      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,kbd_button_name_R[kbd_id])) {
          tmp_mapping_R[kbd_id] = atari_key_id;
          //break;
        }
      }
  }

  memcpy(psp_kbd_mapping, tmp_mapping, sizeof(psp_kbd_mapping));
  memcpy(psp_kbd_mapping_L, tmp_mapping_L, sizeof(psp_kbd_mapping_L));
  memcpy(psp_kbd_mapping_R, tmp_mapping_R, sizeof(psp_kbd_mapping_R));

  return 0;
}

int
psp_kbd_load_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      error = 0;

  KbdFile = fopen(kbd_filename, "r");
  error   = 1;

  if (KbdFile != (FILE*)0) {
    psp_kbd_load_mapping_file(KbdFile);
    error = 0;
    fclose(KbdFile);
  }

  kbd_ltrigger_mapping_active = 0;
  kbd_rtrigger_mapping_active = 0;

  return error;
}

static int loc_restore_mapping[KBD_ALL_BUTTONS];

void
psp_kbd_enter_ui_mapping()
{
  psp_kbd_mode_ui = 1;
  memcpy(loc_restore_mapping, psp_kbd_mapping, sizeof(loc_restore_mapping));
  memcpy(psp_kbd_mapping    , loc_ui_mapping , sizeof(loc_restore_mapping));
}

void
psp_kbd_leave_ui_mapping()
{
  memcpy(psp_kbd_mapping, loc_restore_mapping, sizeof(loc_restore_mapping));
  psp_kbd_mode_ui = 0;
}

int
psp_kbd_save_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      kbd_id = 0;
  int      error = 0;

  KbdFile = fopen(kbd_filename, "w");
  error   = 1;

  if (KbdFile != (FILE*)0) {

    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", kbd_button_name[kbd_id], psp_kbd_mapping[kbd_id]);
    }
    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", kbd_button_name_L[kbd_id], psp_kbd_mapping_L[kbd_id]);
    }
    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", kbd_button_name_R[kbd_id], psp_kbd_mapping_R[kbd_id]);
    }
    error = 0;
    fclose(KbdFile);
  }

  return error;
}

int
psp_kbd_enter_command()
{
  gp2xCtrlData  c;

  unsigned int command_key = 0;
  int          atari_key     = 0;

  gp2xCtrlPeekBufferPositive(&c, 1);

  if (command_atari_pending)
  {
    atari_key_event(command_atari_key, 1);

    if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
      loc_last_event_time = c.TimeStamp;
      command_atari_pending = 0;
      atari_key_event(command_atari_key, 0);
    }

    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
    loc_last_event_time = c.TimeStamp;

    if (command_index >= command_size) {

      command_mode  = 0;
      command_index = 0;
      command_size  = 0;

      command_atari_pending = 0;
      command_atari_key     = 0;

      return 0;
    }

    command_key = command_keys[command_index++];
    atari_key = atari_get_key_from_ascii(command_key);

    if (atari_key != -1) {
      command_atari_key     = atari_key;
      command_atari_pending = 1;
      atari_key_event(command_atari_key, 1);
    }

    return 1;
  }

  return 0;
}

int
psp_kbd_is_danzeff_mode()
{
  return danzeff_mode;
}

int
psp_kbd_enter_danzeff()
{
  unsigned int danzeff_key = 0;
  int          atari_key   = 0;
  int          key_code    = AKEY_NONE;
  gp2xCtrlData  c;

  if (psp_kbd_mode_ui) return 0;

  if (! danzeff_mode) {
    psp_init_keyboard();
    danzeff_mode = 1;
  }

  gp2xCtrlPeekBufferPositive(&c, 1);

  if (danzeff_atari_pending)
  {
    atari_key_event(danzeff_atari_key, 1);

    if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_PENDING_TIME) {
      loc_last_event_time = c.TimeStamp;
      danzeff_atari_pending = 0;
      atari_key_event(danzeff_atari_key, 0);
    }
    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_DANZEFF_TIME) {
    loc_last_event_time = c.TimeStamp;

    gp2xCtrlPeekBufferPositive(&c, 1);
    danzeff_key = danzeff_readInput( &c);
  }

  if (danzeff_key == DANZEFF_UI) {
    psp_kbd_run_command_ui();

    danzeff_mode          = 0;
    danzeff_atari_pending = 0;
    danzeff_atari_key     = 0;

    psp_kbd_wait_no_button();
  }
  else if (danzeff_key > DANZEFF_START) {
    atari_key = atari_get_key_from_ascii(danzeff_key);

    if (atari_key != -1) {
      danzeff_atari_key     = atari_key;
      danzeff_atari_pending = 1;
      atari_key_event(danzeff_atari_key, 1);
    }

    return 0;

  } else if (danzeff_key == DANZEFF_START) {
    danzeff_mode          = 0;
    danzeff_atari_pending = 0;
    danzeff_atari_key     = 0;

    psp_kbd_wait_no_button();

  } else if (danzeff_key == DANZEFF_SELECT) {
    danzeff_mode          = 0;
    danzeff_atari_pending = 0;
    danzeff_atari_key     = 0;
    psp_main_menu();
    psp_init_keyboard();

    psp_kbd_wait_no_button();
  }

  return danzeff_key;
}

int
atari_decode_key(int psp_b, int button_pressed)
{
  int wake = 0;
  int reverse_analog = ATARI.psp_reverse_analog;

  if (reverse_analog) {
    if ((psp_b >= KBD_JOY_UP  ) &&
        (psp_b <= KBD_JOY_LEFT)) {
      psp_b = psp_b - KBD_JOY_UP + KBD_UP;
    } else
    if ((psp_b >= KBD_UP  ) &&
        (psp_b <= KBD_LEFT)) {
      psp_b = psp_b - KBD_UP + KBD_JOY_UP;
    }
  }

  if (psp_b == KBD_START) {
     if (button_pressed) psp_kbd_enter_danzeff();
  } else
  if (psp_b == KBD_SELECT) {
    if (button_pressed) {
      if (!psp_kbd_mode_ui) psp_main_menu();
      psp_init_keyboard();
    }
  } else {

    if (psp_kbd_mapping[psp_b] >= 0) {
      wake = 1;
      if (button_pressed) {
        // Determine which buton to press first (ie which mapping is currently active)
        if (kbd_ltrigger_mapping_active) {
          // Use ltrigger mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping_L[psp_b];
          atari_key_event(psp_kbd_presses[psp_b], button_pressed);
        } else
        if (kbd_rtrigger_mapping_active) {
          // Use rtrigger mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping_R[psp_b];
          atari_key_event(psp_kbd_presses[psp_b], button_pressed);
        } else {
          // Use standard mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping[psp_b];
          atari_key_event(psp_kbd_presses[psp_b], button_pressed);
        }
      } else {
          // Determine which button to release (ie what was pressed before)
          atari_key_event(psp_kbd_presses[psp_b], button_pressed);
      }

    } else {
      if (psp_kbd_mapping[psp_b] == KBD_LTRIGGER_MAPPING) {
        kbd_ltrigger_mapping_active = button_pressed;
        kbd_rtrigger_mapping_active = 0;
      } else
      if (psp_kbd_mapping[psp_b] == KBD_RTRIGGER_MAPPING) {
        kbd_rtrigger_mapping_active = button_pressed;
        kbd_ltrigger_mapping_active = 0;
      }
    }
  }
  return 0;
}

void
kbd_change_auto_fire(int auto_fire)
{
  ATARI.atari_auto_fire = auto_fire;
  if (ATARI.atari_auto_fire_pressed) {
    atari_key_event(ATARIK_JOY_FIRE1, 0);
    ATARI.atari_auto_fire_pressed = 0;
  }
}

int
kbd_scan_keyboard(void)
{
  gp2xCtrlData c;
  long        delta_stamp;
  int         event;
  int         b;

  event = 0;
  gp2xCtrlPeekBufferPositive( &c, 1 );

  if (ATARI.atari_auto_fire) {
    delta_stamp = c.TimeStamp - first_time_auto_stamp;
    if ((delta_stamp < 0) ||
        (delta_stamp > (KBD_MIN_AUTOFIRE_TIME / (1 + ATARI.atari_auto_fire_period)))) {
      first_time_auto_stamp = c.TimeStamp;
      atari_key_event(psp_kbd_mapping[KBD_CROSS], ATARI.atari_auto_fire_pressed);
      ATARI.atari_auto_fire_pressed = ! ATARI.atari_auto_fire_pressed;
    }
  }

  for (b = 0; b < KBD_MAX_BUTTONS; b++)
  {
    if (c.Buttons & loc_button_mask[b]) {
# if 0  //GAME MODE !
      if (!(loc_button_data.Buttons & loc_button_mask[b]))
# endif
      {
        loc_button_press[b] = 1;
        event = 1;
      }
    } else {
      if (loc_button_data.Buttons & loc_button_mask[b]) {
        loc_button_release[b] = 1;
        loc_button_press[b] = 0;
        event = 1;
      }
    }
  }
  memcpy(&loc_button_data,&c,sizeof(gp2xCtrlData));

  return event;
}

void
kbd_wait_start(void)
{
  while (1)
  {
    gp2xCtrlData c;
    gp2xCtrlReadBufferPositive(&c, 1);
    if (c.Buttons & GP2X_CTRL_START) break;
  }
  psp_kbd_wait_no_button();
}

void
psp_init_keyboard(void)
{
  atari_kbd_reset();
  kbd_ltrigger_mapping_active = 0;
  kbd_rtrigger_mapping_active = 0;
}

void
psp_kbd_wait_no_button(void)
{
  gp2xCtrlData c;

  do {
   gp2xCtrlPeekBufferPositive(&c, 1);
  } while (c.Buttons != 0);
}

void
psp_kbd_wait_button(void)
{
  gp2xCtrlData c;

  do {
   gp2xCtrlReadBufferPositive(&c, 1);
  } while (c.Buttons == 0);
}

int
psp_update_keys(void)
{
  int         b;
  gp2xCtrlData  c;

  static char first_time = 1;
  static int release_pending = 0;

  if (first_time) {

    gp2xCtrlPeekBufferPositive(&c, 1);
    if (first_time_stamp == -1) first_time_stamp = c.TimeStamp;

    first_time      = 0;
    release_pending = 0;

    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      loc_button_release[b] = 0;
      loc_button_press[b] = 0;
    }
    gp2xCtrlPeekBufferPositive(&loc_button_data, 1);

    psp_main_menu();
    psp_init_keyboard();

    return ret_key_code;
  }

  atari_apply_cheats();

  if (command_mode) {
    psp_kbd_enter_command();
    return ret_key_code;
  }

  if (psp_kbd_mode_ui) {
    danzeff_mode = 0;
  }

  if (danzeff_mode) {
    psp_kbd_enter_danzeff();
  }

  if (release_pending)
  {
    release_pending = 0;
    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      if (loc_button_release[b]) {
        loc_button_release[b] = 0;
        loc_button_press[b] = 0;
        atari_decode_key(b, 0);
      }
    }
  }

  kbd_scan_keyboard();

  /* check press event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_press[b]) {
      loc_button_press[b] = 0;
      release_pending     = 0;
      atari_decode_key(b, 1);
    }
  }
  /* check release event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_release[b]) {
      release_pending = 1;
      break;
    }
  }

  return ret_key_code;
}
