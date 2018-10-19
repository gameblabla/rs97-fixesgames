/*
* Castaway
*  (C) 1994 - 2002 Martin Doering, Joachim Hoenig
*
* $File$ - command line parser & system init
*
* This file is distributed under the GPL, version 2 or at your
* option any later version.  See doc/license.txt for details.
*
* revision history
*  23.05.2002  0.02.00 JH  FAST1.0.1 code import: KR -> ANSI, restructuring
*  09.06.2002  0.02.00 JH  Renamed io.c to st.c again (io.h conflicts with system headers)
*  19.06.2002  0.02.00 JH  -d option discontinued.
*/
static char     sccsid[] = "$Id: init.c,v 1.4 2002/06/24 23:21:35 jhoenig Exp $";
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "st.h"
#include "mem.h"
#include "m68k_intrf.h"

extern int readdsk;

int      Init()
{
    MemClean();
    FDCInit(0);
    FDCInit(1);
    IOInit();
    HWReset();
    Ikbd_Reset();
    return 0;
}
