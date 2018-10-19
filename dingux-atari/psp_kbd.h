/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
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

# ifndef _KBD_H_
# define _KBD_H_
# ifdef __cplusplus
extern "C" {
# endif

# define PSP_ALL_BUTTON_MASK 0xFFFF
# define GP2X_ALL_BUTTON_MASK 0xFFFF

 enum atari_keys_emum {

    ATARIK_UNDERSCORE,
    ATARIK_1,         
    ATARIK_2,         
    ATARIK_3,         
    ATARIK_4,         
    ATARIK_5,         
    ATARIK_6,         
    ATARIK_7,         
    ATARIK_8,         
    ATARIK_9,         
    ATARIK_0,         
    ATARIK_SEMICOLON, 
    ATARIK_MINUS    , 
    ATARIK_DELETE,    
    ATARIK_EXCLAMATN, 
    ATARIK_DBLQUOTE,  
    ATARIK_HASH,      
    ATARIK_DOLLAR,    
    ATARIK_PERCENT,   
    ATARIK_AMPERSAND, 
    ATARIK_QUOTE,     
    ATARIK_LEFTPAREN, 
    ATARIK_RIGHTPAREN,
    ATARIK_PLUS,      
    ATARIK_EQUAL,     
    ATARIK_TAB,       
    ATARIK_a,         
    ATARIK_b,         
    ATARIK_c,         
    ATARIK_d,         
    ATARIK_e,         
    ATARIK_f,         
    ATARIK_g,         
    ATARIK_h,         
    ATARIK_i,         
    ATARIK_j,         
    ATARIK_k,         
    ATARIK_l,         
    ATARIK_m,         
    ATARIK_n,         
    ATARIK_o,         
    ATARIK_p,         
    ATARIK_q,         
    ATARIK_r,         
    ATARIK_s,         
    ATARIK_t,         
    ATARIK_u,         
    ATARIK_v,         
    ATARIK_w,         
    ATARIK_x,         
    ATARIK_y,         
    ATARIK_z,         
    ATARIK_A,         
    ATARIK_B,         
    ATARIK_C,         
    ATARIK_D,         
    ATARIK_E,         
    ATARIK_F,         
    ATARIK_G,         
    ATARIK_H,         
    ATARIK_I,         
    ATARIK_J,         
    ATARIK_K,         
    ATARIK_L,         
    ATARIK_M,         
    ATARIK_N,         
    ATARIK_O,         
    ATARIK_P,         
    ATARIK_Q,         
    ATARIK_R,         
    ATARIK_S,         
    ATARIK_T,         
    ATARIK_U,         
    ATARIK_V,         
    ATARIK_W,         
    ATARIK_X,         
    ATARIK_Y,         
    ATARIK_Z,         
    ATARIK_RETURN,    
    ATARIK_CTRL_L,    
    ATARIK_CTRL_R,    
    ATARIK_SHIFT,     
    ATARIK_CAPSLOCK,  
    ATARIK_ESC,       
    ATARIK_SPACE,     
    ATARIK_LEFT,      
    ATARIK_UP,        
    ATARIK_RIGHT,     
    ATARIK_DOWN,      
    ATARIK_F1,        
    ATARIK_F2,        
    ATARIK_F3,        
    ATARIK_F4,        
    ATARIK_AT,        
    ATARIK_COLON,     
    ATARIK_COMMA,     
    ATARIK_PERIOD,    
    ATARIK_SLASH,     
    ATARIK_ASTERISK,  
    ATARIK_LESS,      
    ATARIK_GREATER,   
    ATARIK_QUESTION,  
    ATARIK_PIPE,      
    ATARIK_RBRACKET,  
    ATARIK_LBRACKET,  
    ATARIK_BACKSLASH, 
    ATARIK_POWER,     
    ATARIK_SUPPR,     

    ATARIK_HELP,
    ATARIK_ATARI,
    ATARIK_CLEAR,
    ATARIK_START,
    ATARIK_PAUSE,
    ATARIK_RESET,
    ATARIK_UI,
    ATARIK_COLDSTART,
    ATARIK_WARMSTART,
    ATARIK_EXIT,

    ATARIK_JOY_UP,    
    ATARIK_JOY_DOWN,  
    ATARIK_JOY_LEFT,  
    ATARIK_JOY_RIGHT, 
    ATARIK_JOY_FIRE1, 
    ATARIK_JOY_FIRE2, 

    ATARIK_CONSOLE_OPTION,
    ATARIK_CONSOLE_SELECT,
    ATARIK_CONSOLE_START,

    ATARIC_FPS,
    ATARIC_JOY,
    ATARIC_RENDER,
    ATARIC_LOAD,
    ATARIC_SAVE,
    ATARIC_RESET,
    ATARIC_AUTOFIRE,
    ATARIC_INCFIRE,
    ATARIC_DECFIRE,
    ATARIC_SCREEN,

    ATARIK_MAX_KEY      
  };

 struct atari_key_trans {
   int  key;
   int  bit_mask;
   int  shift;
   char name[10];
 };
  

# define KBD_UP           0
# define KBD_RIGHT        1
# define KBD_DOWN         2
# define KBD_LEFT         3
# define KBD_TRIANGLE     4
# define KBD_CIRCLE       5
# define KBD_CROSS        6
# define KBD_SQUARE       7
# define KBD_SELECT       8
# define KBD_START        9
# define KBD_LTRIGGER    10
# define KBD_RTRIGGER    11

# define KBD_MAX_BUTTONS 12

# define KBD_JOY_UP      12
# define KBD_JOY_RIGHT   13
# define KBD_JOY_DOWN    14
# define KBD_JOY_LEFT    15

# define KBD_ALL_BUTTONS 16

# define KBD_UNASSIGNED         -1

# define KBD_LTRIGGER_MAPPING   -2
# define KBD_RTRIGGER_MAPPING   -3
# define KBD_NORMAL_MAPPING     -1

  extern char kbd_button_name[ KBD_ALL_BUTTONS ][20];
  extern int psp_screenshot_mode;
  extern int psp_kbd_mapping[ KBD_ALL_BUTTONS ];
  extern int psp_kbd_mapping_L[ KBD_ALL_BUTTONS ];
  extern int psp_kbd_mapping_R[ KBD_ALL_BUTTONS ];
  extern int psp_kbd_presses[ KBD_ALL_BUTTONS ];
  extern int kbd_ltrigger_mapping_active;
  extern int kbd_rtrigger_mapping_active;
  extern unsigned int kbd_button_mask[ KBD_MAX_BUTTONS ];

  extern struct atari_key_trans psp_atari_key_info[ATARIK_MAX_KEY];

  extern int psp_kbd_mode_ui;

  extern void psp_kbd_default_settings();
  extern int  psp_update_keys(void);
  extern void kbd_wait_start(void);
  extern void psp_init_keyboard(void);
  extern void psp_kbd_wait_no_button(void);
  extern int  psp_kbd_is_danzeff_mode(void);
  extern int psp_kbd_load_mapping(char *kbd_filename);
  extern int psp_kbd_save_mapping(char *kbd_filename);
  extern void psp_kbd_display_active_mapping(void);
  extern void kbd_change_auto_fire(int auto_fire);

# ifdef __cplusplus
}
# endif
# endif
