/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//todo - ensure that #ifdef WIN32 makes sense
//consider changing this to use sdl net stuff?

#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include "main.h"
#include "dface.h"
#include "dummy-netplay.h"

#include "../../fceu.h"
#include "../../utils/md5.h"

#include <string>
#include "../common/configSys.h"

extern Config *g_config;

int FCEUDnetplay=0;

int
FCEUD_NetworkConnect(void)
{
    return 1;
}


int
FCEUD_SendData(void *data,
               uint32 len)
{
    return 1;
}

int
FCEUD_RecvData(void *data,
               uint32 len)
{
    return 0;
}

void
FCEUD_NetworkClose(void)
{
    FCEUDnetplay = 0;
}


void
FCEUD_NetplayText(uint8 *text)
{
}
