/*
 * mzpokeysnd.c - POKEY sound chip emulation, v1.6
 *
 * Copyright (C) 2002 Michael Borisov
 * Copyright (C) 2002-2005 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"
#include <stdlib.h>
#include <math.h>

#ifdef ASAP /* external project, see http://asap.sf.net */
#include "asap_internal.h"
#else
#include "atari.h"
#endif
#include "mzpokeysnd.h"
#include "pokeysnd.h"
#include "remez.h"

#define SND_FILTER_SIZE  2048

#define NPOKEYS 2

/* Volume only emulations declarations */
#ifdef VOL_ONLY_SOUND

#define	SAMPBUF_MAX	2000
extern int	sampbuf_val[SAMPBUF_MAX];	/* volume values */
extern int	sampbuf_cnt[SAMPBUF_MAX];	/* relative start time */
extern int	sampbuf_ptr;                    /* pointer to sampbuf */
extern int	sampbuf_rptr;                   /* pointer to read from sampbuf */
extern int	sampbuf_last;                   /* last absolute time */
extern int	sampbuf_AUDV[4 * MAXPOKEYS];	/* prev. channel volume */
extern int	sampbuf_lastval;		/* last volume */
extern int	sampout;			/* last out volume */
extern uint16	samp_freq;
extern int	samp_consol_val;		/* actual value of console sound */
#endif  /* VOL_ONLY_SOUND */

/* M_PI was not defined in MSVC headers */
#ifndef M_PI
# define M_PI 3.141592653589793
#endif

static unsigned int num_cur_pokeys = 0;

/* Filter */
static unsigned int sample_rate; /* Hz */
static unsigned int pokey_frq; /* Hz - for easier resampling */
static int filter_size;
# if 0 //LUDO:
static double filter_data[SND_FILTER_SIZE];
# else
static double *filter_data;
# endif
static unsigned int audible_frq;

static const unsigned long pokey_frq_ideal =  1789790; /* Hz - True */
#if 0
static const int filter_size_44 = 1274;
static const int filter_size_44_8 = 884;
static const int filter_size_22 = 1239;
static const int filter_size_22_8 = 893;
static const int filter_size_11 = 1305;
static const int filter_size_11_8 = 937;
static const int filter_size_48 = 898;
static const int filter_size_48_8 = 626;
static const int filter_size_8  = 1322;
static const int filter_size_8_8 = 1214;
#endif

/* Flags and quality */
static int snd_flags = 0;
static int snd_quality = 0;

/* Poly tables */
static unsigned char poly4tbl[15];
static unsigned char poly5tbl[31];
static unsigned char poly17tbl[131071];
static unsigned char poly9tbl[511];


struct stPokeyState;

typedef unsigned char (*readout_t)(struct stPokeyState* ps);
typedef void (*event_t)(struct stPokeyState* ps, char p5v, char p4v, char p917v);



/* State variables for single Pokey Chip */
typedef struct stPokeyState
{
    /* Poly positions */
    unsigned int poly4pos;
    unsigned int poly5pos;
    unsigned int poly17pos;
    unsigned int poly9pos;

    /* Change queue */
    unsigned char ovola;
    int qet[1322]; /* maximal length of filter */
    unsigned char qev[1322];
    int qebeg;
    int qeend;

    /* Main divider (64khz/15khz) */
    unsigned char mdivk;    /* 28 for 64khz, 114 for 15khz */

    /* Main switches */
    unsigned char selpoly9;
    unsigned char c0_hf;
    unsigned char c1_f0;
    unsigned char c2_hf;
    unsigned char c3_f2;

    /* Main output state */
    unsigned char outvol_all;
    unsigned char forcero; /* Force readout */

    /* channel 0 state */

    readout_t readout_0;
    event_t event_0;

    unsigned long c0divpos;
    unsigned long c0divstart;   /* AUDF0 recalculated */
    unsigned long c0divstart_p; /* start value when c1_f0 */
    unsigned short c0diva;      /* AUDF0 register */
    unsigned char c0ctl;        /* AUDC0 register */

    unsigned char c0t1;         /* D - 5bit, Q goes to sw3 */
    unsigned char c0t2;         /* D - out sw2, Q goes to sw4 and t3 */
    unsigned char c0t3;         /* D - out t2, q goes to xor */

    unsigned char c0sw1;        /* in1 - 4bit, in2 - 17bit, out goes to sw2 */
    unsigned char c0sw2;        /* in1 - /Q t2, in2 - out sw1, out goes to t2 */
    unsigned char c0sw3;        /* in1 - +5, in2 - Q t1, out goes to C t2 */
    unsigned char c0sw4;        /* hi-pass sw */
    unsigned char c0vo;         /* volume only */

    unsigned char c0stop;       /* channel counter stopped */

    unsigned char vol0;

    unsigned char outvol_0;

    /* channel 1 state */

    readout_t readout_1;
    event_t event_1;

    unsigned long c1divpos;
    unsigned long c1divstart;
    unsigned short c1diva;
    unsigned char c1ctl;

    unsigned char c1t1;
    unsigned char c1t2;
    unsigned char c1t3;

    unsigned char c1sw1;
    unsigned char c1sw2;
    unsigned char c1sw3;
    unsigned char c1sw4;
    unsigned char c1vo;

    unsigned char c1stop;      /* channel counter stopped */

    unsigned char vol1;

    unsigned char outvol_1;

    /* channel 2 state */

    readout_t readout_2;
    event_t event_2;

    unsigned long c2divpos;
    unsigned long c2divstart;
    unsigned long c2divstart_p;     /* start value when c1_f0 */
    unsigned short c2diva;
    unsigned char c2ctl;

    unsigned char c2t1;
    unsigned char c2t2;

    unsigned char c2sw1;
    unsigned char c2sw2;
    unsigned char c2sw3;
    unsigned char c2vo;

    unsigned char c2stop;          /* channel counter stopped */

    unsigned char vol2;

    unsigned char outvol_2;

    /* channel 3 state */

    readout_t readout_3;
    event_t event_3;

    unsigned long c3divpos;
    unsigned long c3divstart;
    unsigned short c3diva;
    unsigned char c3ctl;

    unsigned char c3t1;
    unsigned char c3t2;

    unsigned char c3sw1;
    unsigned char c3sw2;
    unsigned char c3sw3;
    unsigned char c3vo;

    unsigned char c3stop;          /* channel counter stopped */

    unsigned char vol3;

    unsigned char outvol_3;

} PokeyState;

PokeyState pokey_states[NPOKEYS];

/* Forward declarations for ResetPokeyState */

static unsigned char readout0_normal(PokeyState* ps);
static void event0_pure(PokeyState* ps, char p5v, char p4v, char p917v);

static unsigned char readout1_normal(PokeyState* ps);
static void event1_pure(PokeyState* ps, char p5v, char p4v, char p917v);

static unsigned char readout2_normal(PokeyState* ps);
static void event2_pure(PokeyState* ps, char p5v, char p4v, char p917v);

static unsigned char readout3_normal(PokeyState* ps);
static void event3_pure(PokeyState* ps, char p5v, char p4v, char p917v);




void ResetPokeyState(PokeyState* ps)
{
    /* Poly positions */
    ps->poly4pos = 0;
    ps->poly5pos = 0;
    ps->poly9pos = 0;
    ps->poly17pos = 0;

    /* Change queue */
    ps->ovola = 0;
    ps->qebeg = 0;
    ps->qeend = 0;

    /* Global Pokey controls */
    ps->mdivk = 28;

    ps->selpoly9 = 0;
    ps->c0_hf = 0;
    ps->c1_f0 = 0;
    ps->c2_hf = 0;
    ps->c3_f2 = 0;

    ps->outvol_all = 0;
    ps->forcero = 0;

    /* Channel 0 state */
    ps->readout_0 = readout0_normal;
    ps->event_0 = event0_pure;

    ps->c0divpos = 1000;
    ps->c0divstart = 1000;
    ps->c0divstart_p = 1000;
    ps->c0diva = 255;
    ps->c0ctl = 0;

    ps->c0t1 = 0;
    ps->c0t2 = 0;
    ps->c0t3 = 0;

    ps->c0sw1 = 0;
    ps->c0sw2 = 0;
    ps->c0sw3 = 0;
    ps->c0sw4 = 0;
    ps->c0vo = 1;

    ps->c0stop = 1;

    ps->vol0 = 0;

    ps->outvol_0 = 0;


    /* Channel 1 state */
    ps->readout_1 = readout1_normal;
    ps->event_1 = event1_pure;

    ps->c1divpos = 1000;
    ps->c1divstart = 1000;
    ps->c1diva = 255;
    ps->c1ctl = 0;

    ps->c1t1 = 0;
    ps->c1t2 = 0;
    ps->c1t3 = 0;

    ps->c1sw1 = 0;
    ps->c1sw2 = 0;
    ps->c1sw3 = 0;
    ps->c1sw4 = 0;
    ps->c1vo = 1;

    ps->c1stop = 1;

    ps->vol1 = 0;

    ps->outvol_1 = 0;

    /* Channel 2 state */

    ps->readout_2 = readout2_normal;
    ps->event_2 = event2_pure;

    ps->c2divpos = 1000;
    ps->c2divstart = 1000;
    ps->c2divstart_p = 1000;
    ps->c2diva = 255;
    ps->c2ctl = 0;

    ps->c2t1 = 0;
    ps->c2t2 = 0;

    ps->c2sw1 = 0;
    ps->c2sw2 = 0;
    ps->c2sw3 = 0;

    ps->c2vo = 0;

    ps->c2stop = 0;

    ps->vol2 = 0;
    ps->outvol_2 = 0;

    /* Channel 3 state */
    ps->readout_3 = readout3_normal;
    ps->event_3 = event3_pure;

    ps->c3divpos = 1000;
    ps->c3divstart = 1000;
    ps->c3diva = 255;
    ps->c3ctl = 0;

    ps->c3t1 = 0;
    ps->c3t2 = 0;

    ps->c3sw1 = 0;
    ps->c3sw2 = 0;
    ps->c3sw3 = 0;

    ps->c3stop = 1;

    ps->vol3 = 0;

    ps->outvol_3 = 0;
}


double read_resam_all(PokeyState* ps)
{
    int i = ps->qebeg;
    unsigned char avol,bvol;
    double sum;

    if(ps->qebeg == ps->qeend)
    {
        return ps->ovola * filter_data[0]; /* if no events in the queue */
    }

    avol = ps->ovola;
    sum = 0;

    /* Separate two loop cases, for wrap-around and without */
    if(ps->qeend < ps->qebeg) /* With wrap */
    {
        while(i<filter_size)
        {
            bvol = ps->qev[i];
            sum += (avol-bvol)*filter_data[ps->qet[i]];
            avol = bvol;
            ++i;
        }
        i=0;
    }

    /* without wrap */
    while(i<ps->qeend)
    {
        bvol = ps->qev[i];
        sum += (avol-bvol)*filter_data[ps->qet[i]];
        avol = bvol;
        ++i;
    }

    sum += avol*filter_data[0];
    return sum;
}

static void add_change(PokeyState* ps, unsigned char a)
{
    ps->qev[ps->qeend] = a;
    ps->qet[ps->qeend] = 0;
    ++ps->qeend;
    if(ps->qeend >= filter_size)
        ps->qeend = 0;
}

static void bump_qe_subticks(PokeyState* ps, int subticks)
{
    /* Remove too old events from the queue while bumping */
    int i = ps->qebeg;
    if(ps->qeend < ps->qebeg) /* Loop with wrap */
    {
        while(i<filter_size)
        {
            ps->qet[i] += subticks;
            if(ps->qet[i] >= filter_size - 1)
            {
                ps->ovola = ps->qev[i];
                ++ps->qebeg;
                if(ps->qebeg >= filter_size)
                    ps->qebeg = 0;
            }
            ++i;
        }
        i=0;
    }
    /* loop without wrap */
    while(i<ps->qeend)
    {
        ps->qet[i] += subticks;
        if(ps->qet[i] >= filter_size - 1)
        {
            ps->ovola = ps->qev[i];
            ++ps->qebeg;
            if(ps->qebeg >= filter_size)
                ps->qebeg = 0;
        }
        ++i;
    }
}



static void build_poly4(void)
{
    unsigned char c;
    unsigned char i;
    unsigned char poly4=1;

    for(i=0; i<15; i++)
    {
        poly4tbl[i] = ~poly4;
        c = ((poly4>>2)&1) ^ ((poly4>>3)&1);
        poly4 = ((poly4<<1)&15) + c;
    }
}

static void build_poly5(void)
{
	unsigned char c;
	unsigned char i;
	unsigned char poly5 = 1;

	for(i = 0; i < 31; i++) {
		poly5tbl[i] = ~poly5; /* Inversion! Attention! */
		c = ((poly5 >> 2) ^ (poly5 >> 4)) & 1;
		poly5 = ((poly5 << 1) & 31) + c;
	}
}

static void build_poly17(void)
{
	unsigned int c;
	unsigned int i;
	unsigned int poly17 = 1;

	for(i = 0; i < 131071; i++) {
		poly17tbl[i] = (unsigned char) poly17;
		c = ((poly17 >> 11) ^ (poly17 >> 16)) & 1;
		poly17 = ((poly17 << 1) & 131071) + c;
	}
}

static void build_poly9(void)
{
	unsigned int c;
	unsigned int i;
	unsigned int poly9 = 1;

	for(i = 0; i < 511; i++) {
		poly9tbl[i] = (unsigned char) poly9;
		c = ((poly9 >> 3) ^ (poly9 >> 8)) & 1;
		poly9 = ((poly9 << 1) & 511) + c;
	}
}

static void advance_polies(PokeyState* ps, unsigned long tacts)
{
    ps->poly4pos = (tacts + ps->poly4pos) % 15;
    ps->poly5pos = (tacts + ps->poly5pos) % 31;
    ps->poly17pos = (tacts + ps->poly17pos) % 131071;
    ps->poly9pos = (tacts + ps->poly9pos) % 511;
}

/***********************************

   READ OUTPUT 0

  ************************************/

static unsigned char readout0_vo(PokeyState* ps)
{
    return ps->vol0;
}

static unsigned char readout0_hipass(PokeyState* ps)
{
    if(ps->c0t2 ^ ps->c0t3)
        return ps->vol0;
    else return 0;
}

static unsigned char readout0_normal(PokeyState* ps)
{
    if(ps->c0t2)
        return ps->vol0;
    else return 0;
}

/***********************************

   READ OUTPUT 1

  ************************************/

static unsigned char readout1_vo(PokeyState* ps)
{
    return ps->vol1;
}

static unsigned char readout1_hipass(PokeyState* ps)
{
    if(ps->c1t2 ^ ps->c1t3)
        return ps->vol1;
    else return 0;
}

static unsigned char readout1_normal(PokeyState* ps)
{
    if(ps->c1t2)
        return ps->vol1;
    else return 0;
}

/***********************************

   READ OUTPUT 2

  ************************************/

static unsigned char readout2_vo(PokeyState* ps)
{
    return ps->vol2;
}

static unsigned char readout2_normal(PokeyState* ps)
{
    if(ps->c2t2)
        return ps->vol2;
    else return 0;
}

/***********************************

   READ OUTPUT 3

  ************************************/

static unsigned char readout3_vo(PokeyState* ps)
{
    return ps->vol3;
}

static unsigned char readout3_normal(PokeyState* ps)
{
    if(ps->c3t2)
        return ps->vol3;
    else return 0;
}


/***********************************

   EVENT CHANNEL 0

  ************************************/

static void event0_pure(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c0t2 = !ps->c0t2;
    ps->c0t1 = p5v;
}

static void event0_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c0t1)
        ps->c0t2 = !ps->c0t2;
    ps->c0t1 = p5v;
}

static void event0_p4(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c0t2 = p4v;
    ps->c0t1 = p5v;
}

static void event0_p917(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c0t2 = p917v;
    ps->c0t1 = p5v;
}

static void event0_p4_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c0t1)
        ps->c0t2 = p4v;
    ps->c0t1 = p5v;
}

static void event0_p917_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c0t1)
        ps->c0t2 = p917v;
    ps->c0t1 = p5v;
}

/***********************************

   EVENT CHANNEL 1

  ************************************/

static void event1_pure(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c1t2 = !ps->c1t2;
    ps->c1t1 = p5v;
}

static void event1_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c1t1)
        ps->c1t2 = !ps->c1t2;
    ps->c1t1 = p5v;
}

static void event1_p4(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c1t2 = p4v;
    ps->c1t1 = p5v;
}

static void event1_p917(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c1t2 = p917v;
    ps->c1t1 = p5v;
}

static void event1_p4_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c1t1)
        ps->c1t2 = p4v;
    ps->c1t1 = p5v;
}

static void event1_p917_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c1t1)
        ps->c1t2 = p917v;
    ps->c1t1 = p5v;
}

/***********************************

   EVENT CHANNEL 2

  ************************************/

static void event2_pure(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c2t2 = !ps->c2t2;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

static void event2_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c2t1)
        ps->c2t2 = !ps->c2t2;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

static void event2_p4(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c2t2 = p4v;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

static void event2_p917(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c2t2 = p917v;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

static void event2_p4_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c2t1)
        ps->c2t2 = p4v;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

static void event2_p917_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c2t1)
        ps->c2t2 = p917v;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

/***********************************

   EVENT CHANNEL 3

  ************************************/

static void event3_pure(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c3t2 = !ps->c3t2;
    ps->c3t1 = p5v;
    /* high-pass closk for channel 1 */
    ps->c1t3 = ps->c1t2;
}

static void event3_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c3t1)
        ps->c3t2 = !ps->c3t2;
    ps->c3t1 = p5v;
    /* high-pass closk for channel 1 */
    ps->c1t3 = ps->c1t2;
}

static void event3_p4(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c3t2 = p4v;
    ps->c3t1 = p5v;
    /* high-pass closk for channel 1 */
    ps->c1t3 = ps->c1t2;
}

static void event3_p917(PokeyState* ps, char p5v, char p4v, char p917v)
{
    ps->c3t2 = p917v;
    ps->c3t1 = p5v;
    /* high-pass closk for channel 1 */
    ps->c1t3 = ps->c1t2;
}

static void event3_p4_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c3t1)
        ps->c3t2 = p4v;
    ps->c3t1 = p5v;
    /* high-pass closk for channel 1 */
    ps->c1t3 = ps->c1t2;
}

static void event3_p917_p5(PokeyState* ps, char p5v, char p4v, char p917v)
{
    if(ps->c3t1)
        ps->c3t2 = p917v;
    ps->c3t1 = p5v;
    /* high-pass closk for channel 1 */
    ps->c1t3 = ps->c1t2;
}

static void advance_ticks(PokeyState* ps, unsigned long ticks)
{
    unsigned long ta,tbe, tbe0, tbe1, tbe2, tbe3;
    char p5v,p4v,p917v;

    unsigned char outvol_new;
    unsigned char need0=0;
    unsigned char need1=0;
    unsigned char need2=0;
    unsigned char need3=0;

    unsigned char need=0;

    if(ps->forcero)
    {
        ps->forcero = 0;
        outvol_new = ps->outvol_0 + ps->outvol_1 + ps->outvol_2 + ps->outvol_3;
        if(outvol_new != ps->outvol_all)
        {
            ps->outvol_all = outvol_new;
            add_change(ps, outvol_new);
        }
    }

    while(ticks>0)
    {
        tbe0 = ps->c0divpos;
        tbe1 = ps->c1divpos;
        tbe2 = ps->c2divpos;
        tbe3 = ps->c3divpos;

        tbe = ticks+1;

        if(!ps->c0stop && tbe0 < tbe)
            tbe = tbe0;
        if(!ps->c1stop && tbe1 < tbe)
            tbe = tbe1;
        if(!ps->c2stop && tbe2 < tbe)
            tbe = tbe2;
        if(!ps->c3stop && tbe3 < tbe)
            tbe = tbe3;

        if(tbe>ticks)
            ta = ticks;
        else
        {
            ta = tbe;
            need = 1;
        }

        ticks -= ta;

        if(!ps->c0stop) ps->c0divpos -= ta;
        if(!ps->c1stop) ps->c1divpos -= ta;
        if(!ps->c2stop) ps->c2divpos -= ta;
        if(!ps->c3stop) ps->c3divpos -= ta;

        advance_polies(ps,ta);
        bump_qe_subticks(ps,ta);

        if(need)
        {
            p5v = poly5tbl[ps->poly5pos] & 1;
            p4v = poly4tbl[ps->poly4pos] & 1;
            if(ps->selpoly9)
                p917v = poly9tbl[ps->poly9pos] & 1;
            else
                p917v = poly17tbl[ps->poly17pos] & 1;

            if(!ps->c0stop && ta == tbe0)
            {
                ps->event_0(ps,p5v,p4v,p917v);
                ps->c0divpos = ps->c0divstart;
                need0 = 1;
            }
            if(!ps->c1stop && ta == tbe1)
            {
                ps->event_1(ps,p5v,p4v,p917v);
                ps->c1divpos = ps->c1divstart;
                if(ps->c1_f0)
                    ps->c0divpos = ps->c0divstart_p;
                need1 = 1;
            }
            if(!ps->c2stop && ta == tbe2)
            {
                ps->event_2(ps,p5v,p4v,p917v);
                ps->c2divpos = ps->c2divstart;
                need2 = 1;
                if(ps->c0sw4)
                    need0 = 1;
            }
            if(!ps->c3stop && ta == tbe3)
            {
                ps->event_3(ps,p5v,p4v,p917v);
                ps->c3divpos = ps->c3divstart;
                if(ps->c3_f2)
                    ps->c2divpos = ps->c2divstart_p;
                need3 = 1;
                if(ps->c1sw4)
                    need1 = 1;
            }

            if(need0)
            {
                ps->outvol_0 = 2*ps->readout_0(ps);
            }
            if(need1)
            {
                ps->outvol_1 = 2*ps->readout_1(ps);
            }
            if(need2)
            {
                ps->outvol_2 = 2*ps->readout_2(ps);
            }
            if(need3)
            {
                ps->outvol_3 = 2*ps->readout_3(ps);
            }

            outvol_new = ps->outvol_0 + ps->outvol_1 + ps->outvol_2 + ps->outvol_3;
            if(outvol_new != ps->outvol_all)
            {
                ps->outvol_all = outvol_new;
                add_change(ps, outvol_new);
            }
        }
    }
}

static double generate_sample(PokeyState* ps)
{
    /*unsigned long ta = (subticks+pokey_frq)/sample_rate;
    subticks = (subticks+pokey_frq)%sample_rate;*/

    advance_ticks(ps, pokey_frq/sample_rate);
    return read_resam_all(ps);
}

/******************************************
 filter table generator by Krzysztof Nikiel
 ******************************************/

# if 0 //LUDO:
static int remez_filter_table(double resamp_rate, /* output_rate/input_rate */
                              double *cutoff, int quality)
{
  int i;
  static const int orders[] = {600, 800, 1000, 1200};
  static const struct {
    int stop;		/* stopband ripple */
    double weight;	/* stopband weight */
    double twidth[sizeof(orders)/sizeof(orders[0])];
  } paramtab[] =
  {
    {70, 90, {4.9e-3, 3.45e-3, 2.65e-3, 2.2e-3}},
    {55, 25, {3.4e-3, 2.7e-3, 2.05e-3, 1.7e-3}},
    {40, 6.0, {2.6e-3, 1.8e-3, 1.5e-3, 1.2e-3}},
    {-1, 0, {0, 0, 0, 0}}
  };
  static const double passtab[] = {0.5, 0.6, 0.7};
  int ripple = 0, order = 0;
  int size;
  double weights[2], desired[2], bands[4];
  static const int interlevel = 5;
  double step = 1.0 / interlevel;

  *cutoff = 0.95 * 0.5 * resamp_rate;

  if (quality >= (int) (sizeof(passtab) / sizeof(passtab[0])))
    quality = (int) (sizeof(passtab) / sizeof(passtab[0])) - 1;

  for (ripple = 0; paramtab[ripple].stop > 0; ripple++)
  {
    for (order = 0; order < (int) (sizeof(orders)/sizeof(orders[0])); order++)
    {
      if ((*cutoff - paramtab[ripple].twidth[order])
	  > passtab[quality] * 0.5 * resamp_rate)
	/* transition width OK */
	goto found;
    }
  }

  /* not found -- use shortest trasition */
  ripple--;
  order--;

found:

#if 0
  printf("order: %d, cutoff: %g\tstopband:%d\ttranswidth:%f\n",
         orders[order],
	 1789790 * *cutoff,
	 paramtab[ripple].stop,
	 1789790 * paramtab[ripple].twidth[order]);
  exit(1);
#endif

  size = orders[order] + 1;

  if (size > SND_FILTER_SIZE) /* static table too short */
    return 0;

  desired[0] = 1;
  desired[1] = 0;

  weights[0] = 1;
  weights[1] = paramtab[ripple].weight;

  bands[0] = 0;
  bands[2] = *cutoff;
  bands[1] = bands[2] - paramtab[ripple].twidth[order];
  bands[3] = 0.5;

  bands[1] *= (double)interlevel;
  bands[2] *= (double)interlevel;
  remez(filter_data, (size / interlevel) + 1, 2, bands, desired, weights, BANDPASS);
  for (i = size - interlevel; i >= 0; i -= interlevel)
  {
    int s;
    double h1 = filter_data[i/interlevel];
    double h2 = filter_data[i/interlevel+1];

    for (s = 0; s < interlevel; s++)
    {
      double d = (double)s * step;
      filter_data[i+s] = (h1*(1.0 - d) + h2 * d) * step;
    }
  }

  /* compute reversed cumulative sum table */
  for (i = size - 2; i >= 0; i--)
    filter_data[i] += filter_data[i + 1];

#if 0
  printf("filter_size=%d\n", size);
  for (i = 0; i < size; i++)
    printf("%.15f,\n", filter_data[i]);
  fflush(stdout);
  exit(1);
#endif

  return size;
}
# endif




/*****************************************************************************/
/* Module:  Pokey_sound_init()                                               */
/* Purpose: to handle the power-up initialization functions                  */
/*          these functions should only be executed on a cold-restart        */
/*                                                                           */
/* Authors: Michael Borisov, Krzystof Nikiel                                 */
/*                                                                           */
/* Inputs:  freq17 - the value for the '1.79MHz' Pokey audio clock           */
/*          playback_freq - the playback frequency in samples per second     */
/*          num_pokeys - specifies the number of pokey chips to be emulated  */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/

static void Pokey_process_8(void* sndbuffer, unsigned sndn);
static void Pokey_process_16(void* sndbuffer, unsigned sndn);
static void Update_pokey_sound_mz(uint16 addr, uint8 val, uint8 chip, uint8 gain);
#ifdef SERIO_SOUND
static void Update_serio_sound_mz(int out, UBYTE data);
#endif
#ifdef CONSOLE_SOUND
static void Update_consol_sound_mz( int set );
#endif
#ifdef VOL_ONLY_SOUND
static void Update_vol_only_sound_mz( void );
#endif

static double filter_44[] = {
0.980043340985629,
0.979855450422251,
0.979817872309576,
0.979780158092393,
0.979742307770703,
0.979704321344506,
0.979666198813802,
0.979627940178590,
0.979586529393675,
0.979541966459055,
0.979494251374731,
0.979443384140703,
0.979389364756971,
0.979332062641481,
0.979271477794233,
0.979207610215227,
0.979140459904463,
0.979070026861941,
0.978996550314163,
0.978920030261128,
0.978840466702837,
0.978757859639290,
0.978672209070486,
0.978584224307246,
0.978493905349571,
0.978401252197459,
0.978306264850912,
0.978208943309929,
0.978110536956477,
0.978011045790556,
0.977910469812165,
0.977808809021305,
0.977706063417977,
0.977604054991834,
0.977502783742877,
0.977402249671106,
0.977302452776521,
0.977203393059122,
0.977107441532257,
0.977014598195927,
0.976924863050131,
0.976838236094869,
0.976754717330142,
0.976677145544582,
0.976605520738191,
0.976539842910968,
0.976480112062913,
0.976426328194027,
0.976381641707171,
0.976346052602347,
0.976319560879553,
0.976302166538792,
0.976293869580061,
0.976297907655817,
0.976314280766060,
0.976342988910790,
0.976384032090006,
0.976437410303709,
0.976506161523749,
0.976590285750125,
0.976689782982838,
0.976804653221887,
0.976934896467272,
0.977083028606185,
0.977249049638625,
0.977432959564592,
0.977634758384086,
0.977854446097108,
0.978093671022007,
0.978352433158783,
0.978630732507435,
0.978928569067965,
0.979245942840372,
0.979583294309964,
0.979940623476742,
0.980317930340706,
0.980715214901855,
0.981132477160189,
0.981568644741127,
0.982023717644668,
0.982497695870813,
0.982990579419562,
0.983502368290915,
0.984030266363179,
0.984574273636355,
0.985134390110443,
0.985710615785442,
0.986302950661353,
0.986906777051227,
0.987522094955063,
0.988148904372862,
0.988787205304623,
0.989436997750347,
0.990091877635934,
0.990751844961384,
0.991416899726697,
0.992087041931873,
0.992762271576912,
0.993434592797291,
0.994104005593009,
0.994770509964066,
0.995434105910464,
0.996094793432200,
0.996743385257674,
0.997379881386883,
0.998004281819829,
0.998616586556511,
0.999216795596930,
0.999795067713595,
1.000351402906507,
1.000885801175666,
1.001398262521072,
1.001888786942725,
1.002347563621555,
1.002774592557561,
1.003169873750744,
1.003533407201105,
1.003865192908642,
1.004156287061045,
1.004406689658316,
1.004616400700453,
1.004785420187457,
1.004913748119327,
1.004994137889945,
1.005026589499310,
1.005011102947422,
1.004947678234282,
1.004836315359889,
1.004672354143615,
1.004455794585460,
1.004186636685424,
1.003864880443508,
1.003490525859710,
1.003062259079607,
1.002580080103198,
1.002043988930482,
1.001453985561461,
1.000810069996134,
1.000114905982497,
0.999368493520550,
0.998570832610293,
0.997721923251726,
0.996821765444849,
0.995877381606509,
0.994888771736706,
0.993855935835439,
0.992778873902708,
0.991657585938515,
0.990503553254616,
0.989316775851014,
0.988097253727706,
0.986844986884694,
0.985559975321978,
0.984257903982259,
0.982938772865537,
0.981602581971813,
0.980249331301085,
0.978879020853355,
0.977510921480945,
0.976145033183856,
0.974781355962087,
0.973419889815639,
0.972060634744511,
0.970725450751389,
0.969414337836272,
0.968127295999161,
0.966864325240056,
0.965625425558957,
0.964433719800221,
0.963289207963849,
0.962191890049841,
0.961141766058197,
0.960138835988916,
0.959205870238226,
0.958342868806125,
0.957549831692614,
0.956826758897694,
0.956173650421363,
0.955611118176173,
0.955139162162122,
0.954757782379212,
0.954466978827441,
0.954266751506811,
0.954173664001591,
0.954187716311782,
0.954308908437383,
0.954537240378395,
0.954872712134817,
0.955326004944710,
0.955897118808074,
0.956586053724908,
0.957392809695214,
0.958317386718990,
0.959362928246924,
0.960529434279016,
0.961816904815266,
0.963225339855674,
0.964754739400240,
0.966399384286788,
0.968159274515318,
0.970034410085830,
0.972024790998325,
0.974130417252802,
0.976335831179819,
0.978641032779376,
0.981046022051474,
0.983550798996111,
0.986155363613289,
0.988834200561610,
0.991587309841072,
0.994414691451677,
0.997316345393425,
1.000292271666314,
1.003307200492713,
1.006361131872621,
1.009454065806037,
1.012586002292963,
1.015756941333398,
1.018922836286053,
1.022083687150929,
1.025239493928026,
1.028390256617344,
1.031535975218882,
1.034625439919856,
1.037658650720263,
1.040635607620106,
1.043556310619383,
1.046420759718095,
1.049172801561159,
1.051812436148575,
1.054339663480344,
1.056754483556465,
1.059056896376939,
1.061188518256432,
1.063149349194945,
1.064939389192478,
1.066558638249031,
1.068007096364604,
1.069227208006521,
1.070218973174781,
1.070982391869385,
1.071517464090333,
1.071824189837625,
1.071849085716512,
1.071592151726993,
1.071053387869070,
1.070232794142741,
1.069130370548008,
1.067699924284936,
1.065941455353527,
1.063854963753780,
1.061440449485694,
1.058697912549271,
1.055591452885477,
1.052121070494312,
1.048286765375776,
1.044088537529868,
1.039526386956589,
1.034577283098268,
1.029241225954903,
1.023518215526495,
1.017408251813044,
1.010911334814550,
1.004019281266373,
0.996732091168514,
0.989049764520973,
0.980972301323749,
0.972499701576843,
0.963639856882105,
0.954392767239535,
0.944758432649135,
0.934736853110902,
0.924328028624838,
0.913556309049881,
0.902421694386031,
0.890924184633288,
0.879063779791652,
0.866840479861122,
0.854294568055692,
0.841426044375362,
0.828234908820132,
0.814721161390002,
0.800884802084972,
0.786780636228595,
0.772408663820872,
0.757768884861803,
0.742861299351388,
0.727685907289627,
0.712309789523511,
0.696732946053041,
0.680955376878215,
0.664977081999035,
0.648798061415500,
0.632494716601178,
0.616067047556069,
0.599515054280173,
0.582838736773490,
0.566038095036020,
0.549195351613541,
0.532310506506052,
0.515383559713554,
0.498414511236046,
0.481403361073529,
0.464434312596021,
0.447507365803523,
0.430622520696034,
0.413779777273554,
0.396979135536084,
0.380302818029401,
0.363750824753506,
0.347323155708397,
0.331019810894075,
0.314840790310540,
0.298862495431360,
0.283084926256534,
0.267508082786064,
0.252131965019948,
0.236956572958187,
0.222048987447772,
0.207409208488703,
0.193037236080980,
0.178933070224603,
0.165096710919573,
0.151582963489443,
0.138391827934213,
0.125523304253883,
0.112977392448453,
0.100754092517923,
0.088893687676287,
0.077396177923544,
0.066261563259694,
0.055489843684737,
0.045081019198673,
0.035059439660440,
0.025425105070040,
0.016178015427470,
0.007318170732732,
-0.001154429014174,
-0.009231892211398,
-0.016914218858939,
-0.024201408956798,
-0.031093462504975,
-0.037590379503469,
-0.043700343216920,
-0.049423353645328,
-0.054759410788693,
-0.059708514647015,
-0.064270665220293,
-0.068468893066201,
-0.072303198184737,
-0.075773580575903,
-0.078880040239697,
-0.081622577176120,
-0.084037091444205,
-0.086123583043952,
-0.087882051975361,
-0.089312498238433,
-0.090414921833166,
-0.091235515559495,
-0.091774279417418,
-0.092031213406937,
-0.092006317528050,
-0.091699591780758,
-0.091164519559810,
-0.090401100865206,
-0.089409335696946,
-0.088189224055029,
-0.086740765939456,
-0.085121516882903,
-0.083331476885370,
-0.081370645946857,
-0.079239024067364,
-0.076936611246890,
-0.074521791170769,
-0.071994563839000,
-0.069354929251584,
-0.066602887408520,
-0.063738438309808,
-0.060817735310531,
-0.057840778410688,
-0.054807567610281,
-0.051718102909307,
-0.048572384307769,
-0.045421621618451,
-0.042265814841354,
-0.039104963976478,
-0.035939069023823,
-0.032768129983388,
-0.029636193496462,
-0.026543259563046,
-0.023489328183138,
-0.020474399356739,
-0.017498473083849,
-0.014596819142102,
-0.011769437531497,
-0.009016328252034,
-0.006337491303714,
-0.003732926686536,
-0.001228149741898,
0.001176839530199,
0.003482041129756,
0.005687455056773,
0.007793081311250,
0.009783462223745,
0.011658597794257,
0.013418488022787,
0.015063132909336,
0.016592532453901,
0.018000967494309,
0.019288438030559,
0.020454944062651,
0.021500485590585,
0.022425062614362,
0.023231818584667,
0.023920753501501,
0.024491867364865,
0.024945160174758,
0.025280631931180,
0.025508963872192,
0.025630155997793,
0.025644208307984,
0.025551120802764,
0.025350893482134,
0.025060089930364,
0.024678710147453,
0.024206754133403,
0.023644221888212,
0.022991113411882,
0.022268040616961,
0.021475003503451,
0.020612002071350,
0.019679036320659,
0.018676106251379,
0.017625982259734,
0.016528664345726,
0.015384152509354,
0.014192446750619,
0.012953547069519,
0.011690576310414,
0.010403534473303,
0.009092421558187,
0.007757237565065,
0.006397982493937,
0.005036516347488,
0.003672839125719,
0.002306950828630,
0.000938851456220,
-0.000431458991510,
-0.001784709662237,
-0.003120900555962,
-0.004440031672683,
-0.005742103012403,
-0.007027114575119,
-0.008279381418131,
-0.009498903541438,
-0.010685680945041,
-0.011839713628939,
-0.012961001593133,
-0.014038063525863,
-0.015070899427130,
-0.016059509296934,
-0.017003893135274,
-0.017904050942151,
-0.018752960300717,
-0.019550621210974,
-0.020297033672921,
-0.020992197686559,
-0.021636113251886,
-0.022226116620907,
-0.022762207793622,
-0.023244386770031,
-0.023672653550135,
-0.024047008133932,
-0.024368764375848,
-0.024637922275884,
-0.024854481834039,
-0.025018443050313,
-0.025129805924706,
-0.025193230637847,
-0.025208717189735,
-0.025176265580370,
-0.025095875809752,
-0.024967547877881,
-0.024798528390877,
-0.024588817348740,
-0.024338414751470,
-0.024047320599066,
-0.023715534891529,
-0.023352001441169,
-0.022956720247986,
-0.022529691311979,
-0.022070914633149,
-0.021580390211497,
-0.021067928866091,
-0.020533530596932,
-0.019977195404019,
-0.019398923287354,
-0.018798714246935,
-0.018186409510253,
-0.017562009077307,
-0.016925512948098,
-0.016276921122625,
-0.015616233600888,
-0.014952637654491,
-0.014286133283433,
-0.013616720487715,
-0.012944399267336,
-0.012269169622297,
-0.011599027417121,
-0.010933972651808,
-0.010274005326358,
-0.009619125440771,
-0.008969332995047,
-0.008331032063286,
-0.007704222645487,
-0.007088904741651,
-0.006485078351778,
-0.005892743475866,
-0.005316517800867,
-0.004756401326779,
-0.004212394053603,
-0.003684495981339,
-0.003172707109986,
-0.002679823561238,
-0.002205845335092,
-0.001750772431551,
-0.001314604850613,
-0.000897342592279,
-0.000500058031130,
-0.000122751167167,
0.000234577999611,
0.000571929469204,
0.000889303241611,
0.001187139802141,
0.001465439150793,
0.001724201287569,
0.001963426212468,
0.002183113925490,
0.002384912744984,
0.002568822670951,
0.002734843703391,
0.002882975842304,
0.003013219087689,
0.003128089326738,
0.003227586559451,
0.003311710785827,
0.003380462005866,
0.003433840219570,
0.003474883398786,
0.003503591543516,
0.003519964653759,
0.003524002729515,
0.003515705770784,
0.003498311430022,
0.003471819707229,
0.003436230602405,
0.003391544115549,
0.003337760246663,
0.003278029398608,
0.003212351571385,
0.003140726764993,
0.003063154979434,
0.002979636214707,
0.002893009259445,
0.002803274113649,
0.002710430777319,
0.002614479250454,
0.002515419533055,
0.002415622638470,
0.002315088566699,
0.002213817317742,
0.002111808891599,
0.002009063288270,
0.001907402497411,
0.001806826519020,
0.001707335353099,
0.001608928999646,
0.001511607458663,
0.001416620112116,
0.001323966960005,
0.001233648002329,
0.001145663239090,
0.001060012670286,
0.000977405606739,
0.000897842048448,
0.000821321995413,
0.000747845447635,
0.000677412405113,
0.000610262094349,
0.000546394515343,
0.000485809668095,
0.000428507552605,
0.000374488168873,
0.000323620934845,
0.000275905850521,
0.000231342915901,
0.000189932130986,
0.000151673495774,
0.000113550965070,
0.000075564538873,
0.000037714217183
};

int Pokey_sound_init_mz(uint32 freq17, uint16 playback_freq, uint8 num_pokeys,
                        int flags, int quality
#ifdef __PLUS
                        , int clear_regs
#endif
                       )
{
    double cutoff;

    sample_rate = playback_freq;
    snd_flags = flags;
    snd_quality = quality;

    Update_pokey_sound = Update_pokey_sound_mz;
#ifdef SERIO_SOUND
    Update_serio_sound = Update_serio_sound_mz;
#endif
#ifdef CONSOLE_SOUND
    Update_consol_sound = Update_consol_sound_mz;
#endif
#ifdef VOL_ONLY_SOUND
    Update_vol_only_sound = Update_vol_only_sound_mz;
#endif

#ifdef VOL_ONLY_SOUND
	samp_freq=playback_freq;
#endif  /* VOL_ONLY_SOUND */

	Pokey_process_ptr = (flags & SND_BIT16) ? Pokey_process_16 : Pokey_process_8;
# if 0 //LUDO:
    switch(playback_freq)
    {
# if 0
    case 44100:
        if(flags & SND_BIT16)
        {
            filter_data = filter_44;
            filter_size = filter_size_44;
        }
        else
        {
            filter_data = filter_44_8;
            filter_size = filter_size_44_8;
        }
        pokey_frq = 1808100; /* 1.02% off ideal */
        audible_frq = 20000; /* ultrasound */
        break;
    case 22050:
        if(flags & SND_BIT16)
        {
            filter_data = filter_22;
            filter_size = filter_size_22;
        }
        else
        {
            filter_data = filter_22_8;
            filter_size = filter_size_22_8;
        }
        pokey_frq = 1786050; /* 0.2% off ideal */
        audible_frq = 10000; /* 30db filter attenuation */
        break;
    case 11025:
        if(flags & SND_BIT16)
        {
            filter_data = filter_11;
            filter_size = filter_size_11;
        }
        else
        {
            filter_data = filter_11_8;
            filter_size = filter_size_11_8;
        }
        pokey_frq = 1786050; /* 0.2% off ideal */
        audible_frq = 4500; /* 30db filter attenuation */
        break;
    case 48000:
        if(flags & SND_BIT16)
        {
            filter_data = filter_48;
            filter_size = filter_size_48;
        }
        else
        {
            filter_data = filter_48_8;
            filter_size = filter_size_48_8;
        }
        pokey_frq = 1776000; /* 0.7% off ideal */
        audible_frq = 20000; /* ultrasound */
        break;
    case 8000:
        if(flags & SND_BIT16)
        {
            filter_data = filter_8;
            filter_size = filter_size_8;
        }
        else
        {
            filter_data = filter_8_8;
            filter_size = filter_size_8_8;
        }
        pokey_frq = 1792000; /* 0.1% off ideal */
        audible_frq = 4000; /* Nyquist, also 30db attn, should be 50 */
        break;
# endif
    default:
        pokey_frq = (int)(((double)pokey_frq_ideal/sample_rate) + 0.5) * sample_rate;
	      filter_size = remez_filter_table((double)sample_rate/pokey_frq, &cutoff, quality);
	      audible_frq = (unsigned) (cutoff * pokey_frq);
    }
# else
    filter_data = filter_44;
    filter_size = sizeof(filter_44)/sizeof(filter_44[0]);
    pokey_frq = 1808100; /* 1.02% off ideal */
    audible_frq = 20000; /* ultrasound */
# endif

    build_poly4();
    build_poly5();
    build_poly9();
    build_poly17();

#ifdef __PLUS
	if (clear_regs)
#endif
	{
		ResetPokeyState(pokey_states);
		ResetPokeyState(pokey_states + 1);
	}
	num_cur_pokeys = num_pokeys;

	return 0; /* OK */
}

/*****************************************************************************/
/* Function: Update_pokey_sound()                                            */
/*                                                                           */
/* Inputs:  addr - the address of the parameter to be changed                */
/*          val - the new value to be placed in the specified address        */
/*          gain - specified as an 8-bit fixed point number - use 1 for no   */
/*                 amplification (output is multiplied by gain)              */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/

static void Update_readout_0(PokeyState* ps)
{
    if(ps->c0vo)
        ps->readout_0 = readout0_vo;
    else if(ps->c0sw4)
        ps->readout_0 = readout0_hipass;
    else
        ps->readout_0 = readout0_normal;
}

static void Update_readout_1(PokeyState* ps)
{
    if(ps->c1vo)
        ps->readout_1 = readout1_vo;
    else if(ps->c1sw4)
        ps->readout_1 = readout1_hipass;
    else
        ps->readout_1 = readout1_normal;
}

static void Update_readout_2(PokeyState* ps)
{
    if(ps->c2vo)
        ps->readout_2 = readout2_vo;
    else
        ps->readout_2 = readout2_normal;
}

static void Update_readout_3(PokeyState* ps)
{
    if(ps->c3vo)
        ps->readout_3 = readout3_vo;
    else
        ps->readout_3 = readout3_normal;
}

static void Update_event0(PokeyState* ps)
{
    if(ps->c0sw3)
    {
        if(ps->c0sw2)
            ps->event_0 = event0_pure;
        else
        {
            if(ps->c0sw1)
                ps->event_0 = event0_p4;
            else
                ps->event_0 = event0_p917;
        }
    }
    else
    {
        if(ps->c0sw2)
            ps->event_0 = event0_p5;
        else
        {
            if(ps->c0sw1)
                ps->event_0 = event0_p4_p5;
            else
                ps->event_0 = event0_p917_p5;
        }
    }
}

static void Update_event1(PokeyState* ps)
{
    if(ps->c1sw3)
    {
        if(ps->c1sw2)
            ps->event_1 = event1_pure;
        else
        {
            if(ps->c1sw1)
                ps->event_1 = event1_p4;
            else
                ps->event_1 = event1_p917;
        }
    }
    else
    {
        if(ps->c1sw2)
            ps->event_1 = event1_p5;
        else
        {
            if(ps->c1sw1)
                ps->event_1 = event1_p4_p5;
            else
                ps->event_1 = event1_p917_p5;
        }
    }
}

static void Update_event2(PokeyState* ps)
{
    if(ps->c2sw3)
    {
        if(ps->c2sw2)
            ps->event_2 = event2_pure;
        else
        {
            if(ps->c2sw1)
                ps->event_2 = event2_p4;
            else
                ps->event_2 = event2_p917;
        }
    }
    else
    {
        if(ps->c2sw2)
            ps->event_2 = event2_p5;
        else
        {
            if(ps->c2sw1)
                ps->event_2 = event2_p4_p5;
            else
                ps->event_2 = event2_p917_p5;
        }
    }
}

static void Update_event3(PokeyState* ps)
{
    if(ps->c3sw3)
    {
        if(ps->c3sw2)
            ps->event_3 = event3_pure;
        else
        {
            if(ps->c3sw1)
                ps->event_3 = event3_p4;
            else
                ps->event_3 = event3_p917;
        }
    }
    else
    {
        if(ps->c3sw2)
            ps->event_3 = event3_p5;
        else
        {
            if(ps->c3sw1)
                ps->event_3 = event3_p4_p5;
            else
                ps->event_3 = event3_p917_p5;
        }
    }
}

static void Update_c0divstart(PokeyState* ps)
{
    if(ps->c1_f0)
    {
        if(ps->c0_hf)
        {
            ps->c0divstart = 256;
            ps->c0divstart_p = ps->c0diva + 7;
        }
        else
        {
            ps->c0divstart = 256 * ps->mdivk;
            ps->c0divstart_p = (ps->c0diva+1)*ps->mdivk;
        }
    }
    else
    {
        if(ps->c0_hf)
            ps->c0divstart = ps->c0diva + 4;
        else
            ps->c0divstart = (ps->c0diva+1) * ps->mdivk;
    }
}

static void Update_c1divstart(PokeyState* ps)
{
    if(ps->c1_f0)
    {
        if(ps->c0_hf)
            ps->c1divstart = ps->c0diva + 256*ps->c1diva + 7;
        else
            ps->c1divstart = (ps->c0diva + 256*ps->c1diva + 1) * ps->mdivk;
    }
    else
        ps->c1divstart = (ps->c1diva + 1) * ps->mdivk;
}

static void Update_c2divstart(PokeyState* ps)
{
    if(ps->c3_f2)
    {
        if(ps->c2_hf)
        {
            ps->c2divstart = 256;
            ps->c2divstart_p = ps->c2diva + 7;
        }
        else
        {
            ps->c2divstart = 256 * ps->mdivk;
            ps->c2divstart_p = (ps->c2diva+1)*ps->mdivk;
        }
    }
    else
    {
        if(ps->c2_hf)
            ps->c2divstart = ps->c2diva + 4;
        else
            ps->c2divstart = (ps->c2diva+1) * ps->mdivk;
    }
}

static void Update_c3divstart(PokeyState* ps)
{
    if(ps->c3_f2)
    {
        if(ps->c2_hf)
            ps->c3divstart = ps->c2diva + 256*ps->c3diva + 7;
        else
            ps->c3divstart = (ps->c2diva + 256*ps->c3diva + 1) * ps->mdivk;
    }
    else
        ps->c3divstart = (ps->c3diva + 1) * ps->mdivk;
}

static void Update_audctl(PokeyState* ps, unsigned char val)
{
    unsigned char nc0_hf,nc2_hf,nc1_f0,nc3_f2,nc0sw4,nc1sw4,new_divk;
    unsigned char recalc0=0;
    unsigned char recalc1=0;
    unsigned char recalc2=0;
    unsigned char recalc3=0;

    unsigned int cnt0 = 0;
    unsigned int cnt1 = 0;
    unsigned int cnt2 = 0;
    unsigned int cnt3 = 0;

    nc0_hf = (val & 0x40) != 0;
    nc2_hf = (val & 0x20) != 0;
    nc1_f0 = (val & 0x10) != 0;
    nc3_f2 = (val & 0x08) != 0;
    nc0sw4 = (val & 0x04) != 0;
    nc1sw4 = (val & 0x02) != 0;
    if(val & 0x01)
        new_divk = 114;
    else
        new_divk = 28;

    if(new_divk != ps->mdivk)
    {
        recalc0 = recalc1 = recalc2 = recalc3 = 1;
    }
    if(nc1_f0 != ps->c1_f0)
    {
        recalc0 = recalc1 = 1;
    }
    if(nc3_f2 != ps->c3_f2)
    {
        recalc2 = recalc3 = 1;
    }
    if(nc0_hf != ps->c0_hf)
    {
        recalc0 = 1;
        if(nc1_f0)
            recalc1 = 1;
    }
    if(nc2_hf != ps->c2_hf)
    {
        recalc2 = 1;
        if(nc3_f2)
            recalc3 = 1;
    }

    if(recalc0)
    {
        if(ps->c0_hf)
            cnt0 = ps->c0divpos;
        else
            cnt0 = ps->c0divpos/ps->mdivk;
    }
    if(recalc1)
    {
        if(ps->c1_f0)
        {
            if(ps->c0_hf)
                cnt1 = ps->c1divpos/256;
            else
                cnt1 = ps->c1divpos/256/ps->mdivk;
        }
        else
        {
            cnt1 = ps->c1divpos/ps->mdivk;
        }
    }
    if(recalc2)
    {
        if(ps->c2_hf)
            cnt2 = ps->c2divpos;
        else
            cnt2 = ps->c2divpos/ps->mdivk;
    }
    if(recalc3)
    {
        if(ps->c3_f2)
        {
            if(ps->c2_hf)
                cnt3 = ps->c3divpos/256;
            else
                cnt3 = ps->c3divpos/256/ps->mdivk;
        }
    }

    if(recalc0)
    {
        if(nc0_hf)
            ps->c0divpos = cnt0;
        else
            ps->c0divpos = cnt0*new_divk;
    }
    if(recalc1)
    {
        if(nc1_f0)
        {
            if(nc0_hf)
                ps->c1divpos = cnt1*256+cnt0;
            else
                ps->c1divpos = (cnt1*256+cnt0)*new_divk;
        }
        else
        {
            ps->c1divpos = cnt1*new_divk;
        }
    }

    if(recalc2)
    {
        if(nc2_hf)
            ps->c2divpos = cnt2;
        else
            ps->c2divpos = cnt2*new_divk;
    }
    if(recalc3)
    {
        if(nc3_f2)
        {
            if(nc2_hf)
                ps->c3divpos = cnt3*256+cnt2;
            else
                ps->c3divpos = (cnt3*256+cnt2)*new_divk;
        }
    }

    ps->c0_hf = nc0_hf;
    ps->c2_hf = nc2_hf;
    ps->c1_f0 = nc1_f0;
    ps->c3_f2 = nc3_f2;
    ps->c0sw4 = nc0sw4;
    ps->c1sw4 = nc1sw4;
    ps->mdivk = new_divk;
}

static void Update_c0stop(PokeyState* ps)
{
    unsigned long lim = pokey_frq/2/audible_frq;

    unsigned char hfa = 0;
    ps->c0stop = 0;

    if(ps->c0vo || ps->vol0 == 0)
        ps->c0stop = 1;
    else if(!ps->c0sw4 && ps->c0sw3 && ps->c0sw2) /* If channel 0 is a pure tone... */
    {
        if(ps->c1_f0)
        {
            if(ps->c1divstart <= lim)
            {
                ps->c0stop = 1;
                hfa = 1;
            }
        }
        else
        {
            if(ps->c0divstart <= lim)
            {
                ps->c0stop = 1;
                hfa = 1;
            }
        }
    }
    else if(!ps->c0sw4 && ps->c0sw3 && !ps->c0sw2 && ps->c0sw1) /* if channel 0 is poly4... */
    {
        /* period for poly4 signal is 15 cycles */
        if(ps->c1_f0)
        {
            if(ps->c1divstart <= lim*2/15) /* all poly4 signal is above Nyquist */
            {
                ps->c0stop = 1;
                hfa = 1;
            }
        }
        else
        {
            if(ps->c0divstart <= lim*2/15)
            {
                ps->c0stop = 1;
                hfa = 1;
            }
        }
    }

    ps->outvol_0 = 2*ps->readout_0(ps);
    if(hfa)
        ps->outvol_0 = ps->vol0;
}

static void Update_c1stop(PokeyState* ps)
{
    unsigned long lim = pokey_frq/2/audible_frq;

    unsigned char hfa = 0;
    ps->c1stop = 0;

    if(!ps->c1_f0 && (ps->c1vo || ps->vol1 == 0))
        ps->c1stop = 1;
    else if(!ps->c1sw4 && ps->c1sw3 && ps->c1sw2 && ps->c1divstart <= lim) /* If channel 1 is a pure tone */
    {
        ps->c1stop = 1;
        hfa = 1;
    }
    else if(!ps->c1sw4 && ps->c1sw3 && !ps->c1sw2 && ps->c1sw1 && ps->c1divstart <= lim*2/15)  /* all poly4 signal is above Nyquist */
    {
        ps->c1stop = 1;
        hfa = 1;
    }

    ps->outvol_1 = 2*ps->readout_1(ps);
    if(hfa)
        ps->outvol_1 = ps->vol1;
}

static void Update_c2stop(PokeyState* ps)
{
    unsigned long lim = pokey_frq/2/audible_frq;

    unsigned char hfa = 0;
    ps->c2stop = 0;

    if(!ps->c0sw4 && (ps->c2vo || ps->vol2 == 0))
        ps->c2stop = 1;
    /* If channel 2 is a pure tone and no filter for c0... */
    else if(ps->c2sw3 && ps->c2sw2 && !ps->c0sw4)
    {
        if(ps->c3_f2)
        {
            if(ps->c3divstart <= lim)
            {
                ps->c2stop = 1;
                hfa = 1;
            }
        }
        else
        {
            if(ps->c2divstart <= lim)
            {
                ps->c2stop = 1;
                hfa = 1;
            }
        }
    }
    else if(ps->c2sw3 && !ps->c2sw2 && ps->c2sw1 && !ps->c0sw4) /* if channel 2 is poly4 and no filter for c0... */
    {
        /* period for poly4 signal is 15 cycles */
        if(ps->c3_f2)
        {
            if(ps->c3divstart <= lim*2/15) /* all poly4 signal is above Nyquist */
            {
                ps->c2stop = 1;
                hfa = 1;
            }
        }
        else
        {
            if(ps->c2divstart <= lim*2/15)
            {
                ps->c2stop = 1;
                hfa = 1;
            }
        }
    }

    ps->outvol_2 = 2*ps->readout_2(ps);
    if(hfa)
        ps->outvol_2 = ps->vol2;
}

static void Update_c3stop(PokeyState* ps)
{
    unsigned long lim = pokey_frq/2/audible_frq;
    unsigned char hfa = 0;
    ps->c3stop = 0;

    if(!ps->c1sw4 && !ps->c3_f2 && (ps->c3vo || ps->vol3 == 0))
        ps->c3stop = 1;
    /* If channel 3 is a pure tone */
    else if(ps->c3sw3 && ps->c3sw2 && !ps->c1sw4 && ps->c3divstart <= lim)
    {
        ps->c3stop = 1;
        hfa = 1;
    }
    else if(ps->c3sw3 && !ps->c3sw2 && ps->c3sw1 && !ps->c1sw4 && ps->c3divstart <= lim*2/15)  /* all poly4 signal is above Nyquist */
    {
        ps->c3stop = 1;
        hfa = 1;
    }

    ps->outvol_3 = 2*ps->readout_3(ps);
    if(hfa)
        ps->outvol_3 = ps->vol3;
}

static void Update_pokey_sound_mz(uint16 addr, uint8 val, uint8 chip, uint8 gain)
{
    PokeyState* ps = pokey_states+chip;

    switch(addr & 0x0f)
    {
    case _AUDF1:
        ps->c0diva = val;
        Update_c0divstart(ps);
        if(ps->c1_f0)
        {
            Update_c1divstart(ps);
            Update_c1stop(ps);
        }
        Update_c0stop(ps);
        ps->forcero = 1;
        break;
    case _AUDC1:
        ps->c0sw1 = (val & 0x40) != 0;
        ps->c0sw2 = (val & 0x20) != 0;
        ps->c0sw3 = (val & 0x80) != 0;
        ps->vol0 = val & 0xF;
        ps->c0vo = (val & 0x10) != 0;
        Update_readout_0(ps);
        Update_event0(ps);
        Update_c0stop(ps);
        ps->forcero = 1;
        break;
    case _AUDF2:
        ps->c1diva = val;
        Update_c1divstart(ps);
        if(ps->c1_f0)
        {
            Update_c0divstart(ps);
            Update_c0stop(ps);
        }
        Update_c1stop(ps);
        ps->forcero = 1;
        break;
    case _AUDC2:
        ps->c1sw1 = (val & 0x40) != 0;
        ps->c1sw2 = (val & 0x20) != 0;
        ps->c1sw3 = (val & 0x80) != 0;
        ps->vol1 = val & 0xF;
        ps->c1vo = (val & 0x10) != 0;
        Update_readout_1(ps);
        Update_event1(ps);
        Update_c1stop(ps);
        ps->forcero = 1;
        break;
    case _AUDF3:
        ps->c2diva = val;
        Update_c2divstart(ps);
        if(ps->c3_f2)
        {
            Update_c3divstart(ps);
            Update_c3stop(ps);
        }
        Update_c2stop(ps);
        ps->forcero = 1;
        break;
    case _AUDC3:
        ps->c2sw1 = (val & 0x40) != 0;
        ps->c2sw2 = (val & 0x20) != 0;
        ps->c2sw3 = (val & 0x80) != 0;
        ps->vol2 = val & 0xF;
        ps->c2vo = (val & 0x10) != 0;
        Update_readout_2(ps);
        Update_event2(ps);
        Update_c2stop(ps);
        ps->forcero = 1;
        break;
    case _AUDF4:
        ps->c3diva = val;
        Update_c3divstart(ps);
        if(ps->c3_f2)
        {
            Update_c2divstart(ps);
            Update_c2stop(ps);
        }
        Update_c3stop(ps);
        ps->forcero = 1;
        break;
    case _AUDC4:
        ps->c3sw1 = (val & 0x40) != 0;
        ps->c3sw2 = (val & 0x20) != 0;
        ps->c3sw3 = (val & 0x80) != 0;
        ps->vol3 = val & 0xF;
        ps->c3vo = (val & 0x10) != 0;
        Update_readout_3(ps);
        Update_event3(ps);
        Update_c3stop(ps);
        ps->forcero = 1;
        break;
    case _AUDCTL:
        ps->selpoly9 = (val & 0x80) != 0;
        Update_audctl(ps,val);
        Update_readout_0(ps);
        Update_readout_1(ps);
        Update_readout_2(ps);
        Update_readout_3(ps);
        Update_c0divstart(ps);
        Update_c1divstart(ps);
        Update_c2divstart(ps);
        Update_c3divstart(ps);
        Update_c0stop(ps);
        Update_c1stop(ps);
        Update_c2stop(ps);
        Update_c3stop(ps);
        ps->forcero = 1;
        break;
    case _STIMER:
        if(ps->c1_f0)
            ps->c0divpos = ps->c0divstart_p;
        else
            ps->c0divpos = ps->c0divstart;
        ps->c1divpos = ps->c1divstart;
        if(ps->c3_f2)
            ps->c2divpos = ps->c2divstart_p;
        else
            ps->c2divpos = ps->c2divstart;

        ps->c3divpos = ps->c3divstart;
        ps->c0t2 = 1;
        ps->c1t2 = 1;
        ps->c2t2 = 0;
        ps->c3t2 = 0;
        break;
    }
}

void Pokey_debugreset(uint8 chip)
{
    PokeyState* ps = pokey_states+chip;

    if(ps->c1_f0)
        ps->c0divpos = ps->c0divstart_p;
    else
        ps->c0divpos = ps->c0divstart;
    ps->c1divpos = ps->c1divstart;
    if(ps->c3_f2)
        ps->c2divpos = ps->c2divstart_p;
    else
        ps->c2divpos = ps->c2divstart;
    ps->c3divpos = ps->c3divstart;

    ps->c0t2 = 1;
    ps->c1t2 = 1;
    ps->c2t2 = 1;
    ps->c3t2 = 1;
}

/**************************************************************

           Master gain and DC offset calculation
                 by Michael Borisov

 In order to use the available 8-bit or 16-bit dynamic range
 to full extent, reducing the influence of quantization
 noise while simultaneously avoiding overflows, gain
 and DC offset should be set to appropriate value.

 All Pokey-generated sounds have maximal amplitude of 15.
 When all four channels sound simultaneously and in the
 same phase, amplidudes would add up to 60.

 If Pokey is generating a 'pure tone', it always has a DC
 offset of half its amplitude. For other signals (produced
 by poly generators) DC offset varies, but it is always near
 to half amplitude and never exceeds this value.

 In case that pure tone base frequency is outside of audible
 range (ultrasound frequency for high sample rates and above-
 Nyquist frequency for low sample rates), to speed up the engine,
 the generator is stopped while having only DC offset on the
 output (half of corresponding AUDV value). In order that this
 DC offset can be always represented as integer, AUDV values
 are multiplied by 2 when the generator works.

 Therefore maximum integer value before resampling filters
 would be 60*2 = 120 while having maximum DC offset of 60.
 Resampling does not change the DC offset, therefore we may
 subtract it from the signal either before or after resampling.
 In mzpokeysnd, DC offset is subtracted after resampling, however
 for better understanding in further measurements I assume
 subtracting DC before. So, input range for the resampler
 becomes [-60 .. 60].

 Resampling filter removes some harmonics from the signal as if
 the rectangular wave was Fourier transformed forth-and-back,
 while zeroing all harmonics above cutoff frequency. In case
 of high-frequency pure tone (above samplerate/8), only first
 harmonic of the Fourier transofm will remain. As it
 is known, Fourier-transform of the rectangular function of
 amplitude 1 has first oscillation component of amplitude 4/M_PI.
 Therefore, maximum sample values for filtered rectangular
 signal may exceed the amplitude  of rectangular signal
 by up to 4/M_PI times.

 Since our range before resampler is -60 .. 60, taking into
 account mentioned effect with band limiting, range of values
 on the resampler output appears to be in the following bounds:
 [-60*4/M_PI .. 60*4/M_PI]

 In order to map this into signed 8-bit range [-128 .. 127], we
 should multiply the resampler output by 127/60/4*M_PI.

 As it is common for sound hardware to have 8-bit sound unsigned,
 additional DC offset of 128 must be added.

 For 16-bit case the output range is [-32768 .. 32767], and
 we should multiply the resampler output by 32767/60/4*M_PI

 To make some room for numerical errors, filter ripples and
 quantization noise, so that they do not cause overflows in
 quantization, dynamic range is reduced in mzpokeysnd by
 multiplying the output amplitude with 0.95, reserving 5%
 of the total range for such effects, which is about 0.51db.

 Mentioned gain and DC values were tested with 17kHz audio
 playing synchronously on 4 channels, which showed to be
 utilizing 95% of the sample values range.

 Since any other gain value will be not optimal, I removed
 user gain setting and hard-coded the gain into mzpokeysnd

 ---

 A note from Piotr Fusik:
 I've added support for the key click sound generated by GTIA. Its
 volume seems to be pretty much like 8 on single POKEY's channel.
 So, the volumes now can sum up to 136 (4 channels * 15 * 2
 + 8 * 2 for GTIA), not 120.

 A note from Mark Grebe:
 I've added back in the console and sio sounds from the old
 pokey version.  So, now the volumes can sum up to 152
 (4 channesl * 15 * 2 + 8 * 4 for old sound), not 120 or 136.

 ******************************************************************/


/******************************************************************

          Quantization effects and dithering
              by Michael Borisov

 Quantization error in the signal has an expectation value of half
 the LSB, when the rounding is performed properly. Sometimes they
 express quantization error as a random function with even
 distribution over the range [-0.5 to 0.5]. Spectrum of this function
 is flat, because it's a white noise.

 Power of a discrete signal (including noise) is calculated as
 mean square of its samples. For the mentioned above noise
 this is approximately 0.33. Therefore, in decibels for 8-bit case,
 our noise will have power of 10*log10(0.33/256/256) = -53dB

 Because noise is white, this power of -53dB will be evenly
 distributed over the whole signal band upto Nyquist frequency.
 The larger the band is (higher sampling frequency), less
 is the quantisation noise floor. For 8000Hz noise floor is
 10*log10(0.33/256/256/4000) = -89dB/Hz, and for 44100Hz noise
 floor is 10*log10(0.33/256/256/22050) = -96.4dB/Hz.
 This shows that higher sampling rates are better in sense of
 quantization noise. Moreover, as large part of quantization noise
 in case of 44100Hz will fall into ultrasound and hi-frequency
 area 10-20kHz where human ear is less sensitive, this will
 show up as great improvement in quantization noise performance
 compared to 8000Hz.

 I was plotting spectral analysis for sounds produced by mzpokeysnd
 to check these measures. And it showed up that in 8-bit case
 there is no expected flat noise floor of -89db/Hz for 8000Hz,
 but some distortion spectral peaks had higher amplitude than
 the aliasing peaks in 16-bit case. This was a proof to another
 statement which says that quantization noise tends to become
 correlated with the signal. Correlation is especially strong
 for simple signals generated by Pokey. Correlation means that
 the noise power of -53db is no longer evenly distributed
 across the whole frequency range, but concentrates in larger
 peaks at locations which depend on the Pokey signal.

 To decorrelate quantization distortion and make it again
 white noise, which would improve the sound spectrum, since
 the maximum distortion peaks will have less amplitude,
 dithering is used. Another white noise is added to the signal
 before quantization. Since this added noise is not correlated
 with the signal, it shows itself as a flat noise floor.
 Quantization noise now tries to correlate with the dithering
 noise, but this does not lead to appearance of sharp
 spectral peaks any more :)

 Another thing is that for listening, white noise is better than
 distortion. This is because human hearing has some 'noise
 reduction' system which makes it easier to percept sounds
 on the white noise background.

 From the other point of view, if a signal has high and low
 spectral peaks, it is desirable that there is no distortion
 component with peaks of amplitude comparable to those of
 the true signal. Otherwise, perception of background low-
 amplitude signals will be disrupted. That's why they say
 that dithering extends dynamic range.

 Dithering does not eliminate correlation of quantization noise
 completely. Degree of reduction of this effect depends on
 the dithering noise power. The higher is dithering noise,
 the more quantization noise is decorrelated. But this also
 leads to increase of noise percepted by the listener. So, an
 optimum value should be selected. My experiments show that
 unbiased rand() noise of amplitude 0.25 LSB is doing well.

 Test spectral pictures for 8-bit sound, 8kHz sampling rate,
 dithered, show a noise floor of approx. -87dB/Hz.

 ******************************************************************/

#define MAX_SAMPLE 152
extern int atari_speaker;

static void Pokey_process_8(void* sndbuffer, unsigned int sndn)
{
    unsigned short i;
    int nsam = sndn;
    uint8 *buffer = (uint8 *) sndbuffer;

    if(num_cur_pokeys<1)
        return; /* module was not initialized */

    /* if there are two pokeys, then the signal is stereo
       we assume even sndn */
    while(nsam >= (int) num_cur_pokeys)
    {
#ifdef VOL_ONLY_SOUND
        if( sampbuf_rptr!=sampbuf_ptr )
            { int l;
            if( sampbuf_cnt[sampbuf_rptr]>0 )
                sampbuf_cnt[sampbuf_rptr]-=1280;
            while(  (l=sampbuf_cnt[sampbuf_rptr])<=0 )
                {	sampout=sampbuf_val[sampbuf_rptr];
                        sampbuf_rptr++;
                        if( sampbuf_rptr>=SAMPBUF_MAX )
                                sampbuf_rptr=0;
                        if( sampbuf_rptr!=sampbuf_ptr )
                            {
                            sampbuf_cnt[sampbuf_rptr]+=l;
                            }
                        else	break;
                }
            }
#endif

#ifdef VOL_ONLY_SOUND
        buffer[0] = (uint8)floor((generate_sample(pokey_states) + sampout - MAX_SAMPLE / 2.0)
         * (255.0 / MAX_SAMPLE / 4 * M_PI * 0.95) + 128 + 0.5 + 0.5 * rand() / RAND_MAX - 0.25);
#else
        buffer[0] = (uint8)floor((generate_sample(pokey_states) - MAX_SAMPLE / 2.0)
         * (255.0 / MAX_SAMPLE / 4 * M_PI * 0.95) + 128 + 0.5 + 0.5 * rand() / RAND_MAX - 0.25);
#endif
        for(i=1; i<num_cur_pokeys; i++)
        {
            buffer[i] = (uint8)floor((generate_sample(pokey_states + i) - MAX_SAMPLE / 2.0)
             * (255.0 / MAX_SAMPLE / 4 * M_PI * 0.95) + 128 + 0.5 + 0.5 * rand() / RAND_MAX - 0.25);
        }
        buffer += num_cur_pokeys;
        nsam -= num_cur_pokeys;
    }
}

static void Pokey_process_16(void* sndbuffer, unsigned int sndn)
{
    unsigned short i;
    int nsam = sndn;
    int16 *buffer = (int16 *) sndbuffer;

    if(num_cur_pokeys<1)
        return; /* module was not initialized */

    /* if there are two pokeys, then the signal is stereo
       we assume even sndn */
# if 1 //LUDO: FOR_TEST !
    while(nsam >= (int) num_cur_pokeys)
    {
#ifdef VOL_ONLY_SOUND
        if( sampbuf_rptr!=sampbuf_ptr )
            { int l;
            if( sampbuf_cnt[sampbuf_rptr]>0 )
                sampbuf_cnt[sampbuf_rptr]-=1280;
            while(  (l=sampbuf_cnt[sampbuf_rptr])<=0 )
                {	sampout=sampbuf_val[sampbuf_rptr];
                        sampbuf_rptr++;
                        if( sampbuf_rptr>=SAMPBUF_MAX )
                                sampbuf_rptr=0;
                        if( sampbuf_rptr!=sampbuf_ptr )
                            {
                            sampbuf_cnt[sampbuf_rptr]+=l;
                            }
                        else	break;
                }
            }
#endif
#ifdef VOL_ONLY_SOUND
        buffer[0] = (int16)floor((generate_sample(pokey_states) + sampout - MAX_SAMPLE / 2.0)
         * (65535.0 / MAX_SAMPLE / 4 * M_PI * 0.95) + 0.5 + 0.5 * rand() / RAND_MAX - 0.25);
#else
        buffer[0] = (int16)floor((generate_sample(pokey_states) - MAX_SAMPLE / 2.0)
         * (65535.0 / MAX_SAMPLE / 4 * M_PI * 0.95) + 0.5 + 0.5 * rand() / RAND_MAX - 0.25);
#endif
        for(i=1; i<num_cur_pokeys; i++)
        {
            buffer[i] = (int16)floor((generate_sample(pokey_states + i) - MAX_SAMPLE / 2.0)
             * (65535.0 / MAX_SAMPLE / 4 * M_PI * 0.95) + 0.5 + 0.5 * rand() / RAND_MAX - 0.25);
        }
        buffer += num_cur_pokeys;
        nsam -= num_cur_pokeys;
    }
# endif
}

#ifdef SERIO_SOUND
static void Update_serio_sound_mz( int out, UBYTE data )
{
#ifdef VOL_ONLY_SOUND
   int bits,pv,future;
        if (!serio_sound_enabled) return;

	pv=0;
	future=0;
	bits= (data<<1) | 0x200;
	while( bits )
	{
		sampbuf_lastval-=pv;
		pv=(bits&0x01)*pokey_states[0].vol3;
		sampbuf_lastval+=pv;

	sampbuf_val[sampbuf_ptr]=sampbuf_lastval;
	sampbuf_cnt[sampbuf_ptr]=
		(cpu_clock+future-sampbuf_last)*128*samp_freq/178979;
	sampbuf_last=cpu_clock+future;
	sampbuf_ptr++;
	if( sampbuf_ptr>=SAMPBUF_MAX )
		sampbuf_ptr=0;
	if( sampbuf_ptr==sampbuf_rptr )
	{	sampbuf_rptr++;
		if( sampbuf_rptr>=SAMPBUF_MAX )
			sampbuf_rptr=0;
	}
			/* 1789790/19200 = 93 */
		future+=93;	/* ~ 19200 bit/s - FIXME!!! set speed form AUDF [2] ??? */
		bits>>=1;
	}
	sampbuf_lastval-=pv;
#endif  /* VOL_ONLY_SOUND */
}
#endif /* SERIO_SOUND */

#ifdef CONSOLE_SOUND
static void Update_consol_sound_mz( int set )
{
#ifdef VOL_ONLY_SOUND
  static int prev_atari_speaker=0;
  static unsigned int prev_cpu_clock=0;
  int d;
        if (!console_sound_enabled) return;

	if( !set && samp_consol_val==0 )	return;
	sampbuf_lastval-=samp_consol_val;
	if( prev_atari_speaker!=atari_speaker )
	{	samp_consol_val=atari_speaker*8*4;	/* gain */
		prev_cpu_clock=cpu_clock;
	}
	else if( !set )
	{	d=cpu_clock - prev_cpu_clock;
		if( d<114 )
		{	sampbuf_lastval+=samp_consol_val;   return;	}
		while( d>=114 /* CPUL */ )
		{	samp_consol_val=samp_consol_val*99/100;
			d-=114;
		}
		prev_cpu_clock=cpu_clock-d;
	}
	sampbuf_lastval+=samp_consol_val;
	prev_atari_speaker=atari_speaker;

	sampbuf_val[sampbuf_ptr]=sampbuf_lastval;
	sampbuf_cnt[sampbuf_ptr]=
		(cpu_clock-sampbuf_last)*128*samp_freq/178979;
	sampbuf_last=cpu_clock;
	sampbuf_ptr++;
	if( sampbuf_ptr>=SAMPBUF_MAX )
		sampbuf_ptr=0;
	if( sampbuf_ptr==sampbuf_rptr )
	{	sampbuf_rptr++;
		if( sampbuf_rptr>=SAMPBUF_MAX )
			sampbuf_rptr=0;
	}
#endif  /* VOL_ONLY_SOUND */
}
#endif

#ifdef VOL_ONLY_SOUND
static void Update_vol_only_sound_mz( void )
{
#ifdef CONSOLE_SOUND
	Update_consol_sound(0);	/* mmm */
#endif /* CONSOLE_SOUND */
}
#endif
