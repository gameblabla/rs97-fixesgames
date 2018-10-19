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

# ifndef _GP2X_FMGR_H_
# define _GP2X_FMGR_H_

#ifdef __cplusplus
extern "C" {
#endif

# define GP2X_FMGR_MAX_PATH    512
# define GP2X_FMGR_MAX_NAME    256
# define GP2X_FMGR_MAX_ENTRY  2048

# define FMGR_FORMAT_ROM   1
# define FMGR_FORMAT_CART  2
# define FMGR_FORMAT_STATE 3
# define FMGR_FORMAT_KBD   4
# define FMGR_FORMAT_JOY   5
# define FMGR_FORMAT_CHT   6
# define FMGR_FORMAT_ZIP   7
# define FMGR_FORMAT_SET   8

  extern int psp_fmgr_menu(int format);
  extern int psp_fmgr_getExtId(const char *szFilePath);

#ifdef __cplusplus
}
#endif

# endif
