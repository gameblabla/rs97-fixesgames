/* ======================================================================== */
/*  INTVNAME  -- Attempt to determine the name of a .BIN or .ROM file.      */
/*                                                                          */
/*  Usage:                                                                  */
/*      intvname foo.bin                                                    */
/*                                                                          */
/*  Outputs the name of the .BIN or .ROM on stdout, or nothing if it can    */
/*  not determine the name.  If the game's year is known, it will be        */
/*  output on the second line of output.                                    */
/*                                                                          */
/*  First looks in CRC database to see if file's CRC is in the database.    */
/*  If present, it outputs the name from the database.  Otherwise, it       */
/*  looks for a cartridge header at $5000 and tries to interpret that       */
/*  using some heuristics.                                                  */
/*                                                                          */
/* ======================================================================== */
/*                 Copyright (c) 2002-2006, Joseph Zbiciak                  */
/* ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "lzoe/lzoe.h"
#include "file/file.h"
#include "misc/crc32.h"
#include "misc/file_crc32.h"
#include "icart/icartrom.h"
#include "icart/icartbin.h"
#include "icart/icartfile.h"
#include "name/name.h"

icartrom_t icart;
int debug = 0;

#define GET_BIT(bv,i,b) do {                                    \
                            int _ = (i);                        \
                            b = ((bv)[_ >> 5] >> (_ & 31)) & 1; \
                        } while(0)

#define MAX_NAME (64)

/* ======================================================================== */
/*  USAGE            -- Just give usage info and exit.                      */
/* ======================================================================== */
LOCAL void usage(void)
{
    fprintf(stderr, 
                                                                          "\n"
    "INTVNAME"                                                            "\n"
    "Copyright 2014, Joseph Zbiciak"                                      "\n"
                                                                          "\n"
    "Usage: \n"
    "    intvname foo.bin\n"
    "\n"
                                                                          "\n"
    "This program is free software; you can redistribute it and/or modify""\n"
    "it under the terms of the GNU General Public License as published by""\n"
    "the Free Software Foundation; either version 2 of the License, or"   "\n"
    "(at your option) any later version."                                 "\n"
                                                                          "\n"
    "This program is distributed in the hope that it will be useful,"     "\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of"      "\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU"   "\n"
    "General Public License for more details."                            "\n"
                                                                          "\n"
    "You should have received a copy of the GNU General Public License"   "\n"
    "along with this program; if not, write to the Free Software"         "\n"
    "Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA."           "\n"
                                                                          "\n"
    );

    exit(0);
}

/* ======================================================================== */
/*  GET_WORD         -- Read a word from the Intellicart ROM.               */
/*                      Returns -1 if the memory isn't mapped, or is        */
/*                      bankswitched or writeable memory.                   */
/* ======================================================================== */
static int get_word(int addr)
{
    int bit;
    int seg = addr >> 8;

    GET_BIT(icart.preload,  seg, bit); if (!bit) return -1;
    GET_BIT(icart.dobanksw, seg, bit); if ( bit) return -1;
    GET_BIT(icart.writable, seg, bit); if ( bit) return -1;

    return icart.image[addr];
}

/* ======================================================================== */
/*  GET_SDBD_WORD    -- Read an SDBD word from the Intellicart ROM.         */
/*                      Returns -1 if the memory isn't mapped, or is        */
/*                      bankswitched or writeable memory.                   */
/* ======================================================================== */
static int get_sdbd_word(int addr)
{
    int lo = get_word(addr    );
    int hi = get_word(addr + 1);

    if (lo < 0) return -1;
    if (hi < 0) return -1;

    return (lo & 0xFF) | ((hi << 8) & 0xFF00);
}

char name_buf[MAX_NAME];

/* ======================================================================== */
/*  MAIN             -- In The Beginning, there was MAIN, and C was without */
/*                      CONST and VOID, and Darkness was on the face of     */
/*                      the Programmer.                                     */
/* ======================================================================== */
int main(int argc, char *argv[])
{
    uint_32 crc;
    int i, year, name_addr, decle;
    const char *db_name;

    if (argc != 2)
        usage();

    /* -------------------------------------------------------------------- */
    /*  First get the file's CRC-32 value.                                  */
    /* -------------------------------------------------------------------- */
    crc = file_crc32(argv[1]);

    /* -------------------------------------------------------------------- */
    /*  Look it up in the database.                                         */
    /* -------------------------------------------------------------------- */
    db_name = find_cart_name(crc, &year, NULL, NULL);
    if (db_name)
    {
        printf("%s\n%d\n", db_name, year);
        exit(0);
    }

    /* -------------------------------------------------------------------- */
    /*  Didn't find it in the database.  Read in the requested file.        */
    /* -------------------------------------------------------------------- */
    icartrom_init(&icart);
    icart_readfile(argv[1], &icart, 0);

    if ((name_addr = get_sdbd_word(0x500A)) < 0)
    {
        if (debug) printf("no header\n");
        exit(0);    /* no header */
    }

    year = 1900 + get_word(name_addr);

    if (year < 1977 || year > 2050)
    {
        if (debug) printf("bad year %d\n", year);
        exit(0); /* year out of range */
    }
    
    for (i = 0; i < MAX_NAME - 1; i++)
    {
        decle = get_word(name_addr + i + 1);

        name_buf[i] = decle;

        if (!decle)
            break;

        if (decle < 0x20 || decle > 0x7E || !isprint(decle))
            exit(0); /* not a viable name string */
    }
    name_buf[i] = 0;

    if (i < 2) /* too short? */
    {
        if (debug) printf("too short\n");
        exit(0);    /* not a viable name string */
    }

    printf("%s\n%d\n", name_buf, year);

    return 0;
}

/* ======================================================================== */
/*  This program is free software; you can redistribute it and/or modify    */
/*  it under the terms of the GNU General Public License as published by    */
/*  the Free Software Foundation; either version 2 of the License, or       */
/*  (at your option) any later version.                                     */
/*                                                                          */
/*  This program is distributed in the hope that it will be useful,         */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       */
/*  General Public License for more details.                                */
/*                                                                          */
/*  You should have received a copy of the GNU General Public License       */
/*  along with this program; if not, write to the Free Software             */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               */
/* ======================================================================== */
/*                 Copyright (c) 2014-2014, Joseph Zbiciak                  */
/* ======================================================================== */
