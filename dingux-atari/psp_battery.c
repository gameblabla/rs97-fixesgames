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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <zlib.h>
#include <psppower.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <SDL.h>
#include "psp_fmgr.h"
#include "psp_kbd.h"
#include "psp_battery.h"

static char loc_batt_str[128];

char *
psp_get_battery_string()
{
  strcpy(loc_batt_str, "none");

# ifndef LINUX_MODE
  char tmp[128];
  int ret;

  if (scePowerIsBatteryExist()) {
    ret = scePowerGetBatteryLifePercent();
    if (ret >= 0) {
      sprintf(tmp, "%d", ret);
      strcpy(loc_batt_str,tmp);
      strcat(loc_batt_str,"%");
      if(!scePowerIsPowerOnline()){
        if((ret=scePowerGetBatteryLifeTime()) >= 0){
          sprintf(tmp, " %dh", ret/60);
          strcat(loc_batt_str,tmp);
          sprintf(tmp, "%d", (ret%60) + 100);
          strcat(loc_batt_str,tmp+1);
        }
      }
    }
  }
# endif
  return loc_batt_str;
}

int
psp_is_low_battery()
{
  int ret = 0;
# ifndef LINUX_MODE
  if (scePowerIsBatteryExist()) {
    ret = scePowerGetBatteryLifePercent();
    if (ret < 4) return 1;
  }
# endif
  return 0;
}


