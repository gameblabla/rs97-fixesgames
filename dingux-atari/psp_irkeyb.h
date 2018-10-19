/*
 *  Copyright (C) 2007 Ludovic Jacomme (ludovic.jacomme@gmail.com)
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

# ifndef _PSP_IRKEYB_H_
# define _PSP_IRKEYB_H_

#ifdef __cplusplus
extern "C" {
#endif

# ifdef USE_PSP_IRKEYB

#include <pspirkeyb.h>
#include <pspirkeyb_rawkeys.h>

# define PSP_IRKEYB_UP       -100
# define PSP_IRKEYB_DOWN     -101
# define PSP_IRKEYB_LEFT     -102
# define PSP_IRKEYB_RIGHT    -103
# define PSP_IRKEYB_CROSS    -104
# define PSP_IRKEYB_TRIANGLE -105
# define PSP_IRKEYB_CIRCLE   -106
# define PSP_IRKEYB_SQUARE   -107
# define PSP_IRKEYB_SELECT   -108
# define PSP_IRKEYB_START    -109
# define PSP_IRKEYB_LTRIGGER -110
# define PSP_IRKEYB_RTRIGGER -111
# define PSP_IRKEYB_EMPTY    -1

# define PSP_IRKEYB_SUPPR     300
# define PSP_IRKEYB_INSERT    301
# define PSP_IRKEYB_HOME      302
# define PSP_IRKEYB_PAGEUP    303
# define PSP_IRKEYB_PAGEDOWN  304
# define PSP_IRKEYB_END       305

  extern int  psp_irkeyb_init();
  extern void psp_irkeyb_exit();
  extern void psp_irkeyb_wait_key();
  extern int  psp_irkeyb_read_key();
  extern int  psp_irkeyb_set_psp_key(SceCtrlData* c);

# endif

#ifdef __cplusplus
}
#endif

# endif
