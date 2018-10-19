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

# ifndef _PSP_JOY_H_
# define _PSP_JOY_H_

# ifdef __cplusplus
extern "C" {
# endif

# define JOY_TYPE_NONE          0
# define JOY_TYPE_JOYSTICK      1
# define JOY_TYPE_PADDLE        2
# define JOY_TYPE_DUAL_PADDLE   3
# define JOY_MAX_TYPE           3

# define JOY_UP        0
# define JOY_DOWN      1
# define JOY_LEFT      2
# define JOY_RIGHT     3
# define JOY_FIRE      4
# define JOY_PADDLE1P  5
# define JOY_PADDLE1M  6
# define JOY_PADDLE1F  7
# define JOY_PADDLE2P  8
# define JOY_PADDLE2M  9
# define JOY_PADDLE2F 10

# define JOY_ALL_BUTTONS 11

  extern int psp_joy_mapping[ JOY_ALL_BUTTONS ];

  extern int psp_joy_load_settings(char *kbd_filename);
  extern int psp_joy_save_settings(char *kbd_filename);
  extern void psp_joy_default_settings();

# ifdef __cplusplus
}
# endif

# endif
