/*
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

# ifndef _PSP_IRDA_JOY_H_
# define _PSP_IRDA_JOY_H_

# ifdef __cplusplus
extern "C" {
# endif

# define IRDAJOY_TYPE_NONE          0
# define IRDAJOY_TYPE_JOYSTICK      1
# define IRDAJOY_TYPE_PADDLE        2
# define IRDAJOY_TYPE_DUAL_PADDLE   3
# define IRDAJOY_MAX_TYPE           3


#include <psptypes.h>

  typedef struct irda_joy_t {
     u16 paddleA;
     u16 paddleB;
      u8 atariJ;

  } irda_joy_t;

# define JOY_ATARI_UP_MASK 0x10
# define JOY_ATARI_DOWN_MASK 0x08
# define JOY_ATARI_LEFT_MASK 0x04
# define JOY_ATARI_RIGHT_MASK 0x02
# define JOY_ATARI_BUTTON_MASK 0x01

  extern int psp_irda_get_joy_event( irda_joy_t* irda_joy_evt );
  extern void psp_irda_joy_convert( irda_joy_t* i_joy, SceCtrlData* psp_c );
  extern int psp_irda_joy_init();

  extern int psp_load_irda_mode();
  extern int psp_irda_is_kbd_mode();
  extern int psp_irda_is_joy_mode();

  extern int psp_irda_get_saved_mode();
  extern int psp_irda_set_saved_mode();

# ifdef __cplusplus
}
# endif

# endif
