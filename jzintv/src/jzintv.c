/*
 * ============================================================================
 *  Title:    MAIN
 *  Author:   J. Zbiciak
 *  $Id: jzintv.c,v 1.41 2001/11/02 02:00:02 im14u2c Exp $
 * ============================================================================
 *  Main Simulator Driver File
 * ============================================================================
 *  This doesn't do much yet.  :-)
 * ============================================================================
 */

#include "config.h"

#ifndef macintosh
#ifndef USE_AS_BACKEND
#include "sdl.h"
#endif
#endif

#include <signal.h>
#include "plat/plat.h"
#include "lzoe/lzoe.h"
#include "file/file.h"
#include "periph/periph.h"
#include "cp1600/cp1600.h"
#include "mem/mem.h"
#include "icart/icart.h"
#include "bincfg/bincfg.h"
#include "bincfg/legacy.h"
#include "pads/pads.h"
#include "pads/pads_cgc.h"
#include "pads/pads_intv2pc.h"
#include "gfx/gfx.h"
#include "snd/snd.h"
#include "ay8910/ay8910.h"
#include "demo/demo.h"
#include "stic/stic.h"
#include "speed/speed.h"
#include "debug/debug_.h"
#include "event/event.h"
#include "ivoice/ivoice.h"
#include "jlp/jlp.h"
#include "locutus/locutus_adapt.h"
#include "cfg/mapping.h"
#include "cfg/cfg.h"

#ifdef macintosh
# include "console.h"
#endif

#ifdef GCWZERO
#include <SDL_image.h>
#include "jzintv.h"
#include "name/name.h"
#include <sys/stat.h>
int displayplayer2=0;
int virtualnumberpad=0; 
/*
virtual numberpad settings:
0=default
1=display virtual numberpad
2=player toggle (then resets to 0)
3=display virtual keyboard
*/
int letterx=1;
int lettery=1;
int kbletterx=1;
int kblettery=1;
int shiftpressed=0;
char romfilenamecopy[999];
static int configurationsettings[4] = {1, 1, 1, 3};
int menu_sound=1;
int menu_customoverlays=1;
int menu_transparency=1;
int menu_overlayposition=3;
int drawmenu=0;
int selectedoption=0;
int displaycontrolinfo=0;
//char gamename[64];
#endif

cfg_t intv;

double elapsed(int);
void save_state(int);
void load_dump(void);

/*volatile int please_die = 0;*/
/*volatile int reset = 0;*/

/*static void usage(void);*/

/*
 * ============================================================================
 *  RELEASE      -- Program name / release
 * ============================================================================
 */
static char * release(void)
{
    static char buf[16];

    snprintf(buf, 16, "%d.%d", JZINTV_VERSION_MAJOR, JZINTV_VERSION_MINOR);
    return buf;
}

#if 1
/*
 * ============================================================================
 *  CART_NAME    -- Look for a game name in a cartridge image.
 * ============================================================================
 */
static const char * cart_name(void)
{
    static char name_buf[64];
    uint_16 title_addr, lo, hi, ch;
    int year;
    int i, ps;
    const char *base_name;
    char *s1, *s2;

#ifdef GCWZERO
    memcpy(romfilenamecopy,romfilename,sizeof(romfilename));
#endif

    if ((base_name = intv.cart_name) != NULL)
    {
        year = intv.cart_year;
        i = 0;
        goto got_name;
    }

    if ((base_name = strrchr(intv.fn_game, '/')) == NULL &&
        (base_name = strrchr(intv.fn_game, '\\')) == NULL)
        base_name = intv.fn_game;
    else
        base_name++;


    lo = periph_peek((periph_p)intv.intv, (periph_p)intv.intv, 0x500A, ~0);
    hi = periph_peek((periph_p)intv.intv, (periph_p)intv.intv, 0x500B, ~0);

    if ((lo | hi) & 0xFF00)
        return base_name;

    title_addr = ((hi << 8) | lo);

    year = 1900 + periph_peek((periph_p)intv.intv,
                              (periph_p)intv.intv, title_addr, ~0);

    if (year < 1977 || year > 2050)
        return base_name;

    for (i = 0; i < 64 - 8; i++)
    {
        ch = periph_peek((periph_p)intv.intv,
                         (periph_p)intv.intv, title_addr + i + 1, ~0);

        name_buf[i] = ch;

        if (ch == 0)
            break;

        if (ch < 32 || ch > 126)
            return base_name;
    }

    ps = 1;
    i  = 0;
    for (s1 = s2 = name_buf; *s1; s1++)
        if (!isspace(*s1) || !ps)
        {
            *s2++ = *s1;
            ps = isspace(*s1);
            if (!ps)
                i = s2 - name_buf;
        }

got_name:
    if (i == 0)
    {
        strncpy(name_buf, base_name, 64-8);
        name_buf[64-8] = 0;
        i = strlen(name_buf);
    }

    snprintf(name_buf + i, 8, " (%4d)", year);

    return name_buf;
}
#endif

/*
 * ============================================================================
 *  GRACEFUL_DEATH -- Die gracefully.
 * ============================================================================
 */
static void graceful_death(int x)
{
    if (intv.do_exit < 2)
    {
        if (intv.do_exit) fprintf(stderr, "\nOUCH!\n");
        fprintf(stderr,
                "\nRequesting exit:  Received signal %d.\n"
                "(You may need to press enter if you're at a prompt.)\n", x);
    } else
    {
        fprintf(stderr, "\nReceived 3 signals:  Aborting on signal %d.\n", x);
        exit(1);
    }
    intv.do_exit++;
}

/*
 * ============================================================================
 *  ELAPSED      -- Returns amount of time that's elapsed since the program
 *                  started, in CP1610 clock cycles (895kHz)
 * ============================================================================
 */
double elapsed(int restart)
{
    static double start;
    static int init = 0;
    double now;

    if (!init || restart)
    {
        start = get_time();
        init = 1;
    }

    now = get_time();

    return (now - start) * 894886.25;
}

/*
 * ============================================================================
 *  DO_GUI_MODE  -- Implement the simple GUI mode remote controls on stdin.
 * ============================================================================
 */

static void do_gui_mode(void)
{
    char cmd;

    while (read(STDIN_FILENO, &cmd, 1) == 1)
    {
        switch (cmd)
        {
            case '\r' : case '\n' : goto na; break; /* ignore CR, LF */
            case 'p'  : intv.do_pause = !intv.do_pause;     break;
            case 'q'  : intv.do_exit  = 1;                  break;
            case 'r'  : intv.do_reset = 2;                  break;
            default   : putchar('!');  goto bad;            break;
        }

        putchar('.');
bad:
        fflush(stdout);
na:     ;
    }
}


/*
 * ============================================================================
 *  In the beginning, there was a main....
 * ============================================================================
 */
#ifdef USE_AS_BACKEND
int jzintv_entry_point(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    int iter = 0, arg;
    double cycles = 0, rate, irate, then, now, icyc;
    uint_32 s_cnt = 0;
    int paused = 0;
    char title[128];

#ifdef GCWZERO //create directory structure
//mkdir("/media/home/.jzintellivision",S_IRWXU);
//mkdir("/media/home/.jzintellivision/bios",S_IRWXU);
//mkdir("/media/home/.jzintellivision/overlays",S_IRWXU);
//mkdir("/media/home/.jzintellivision/configfiles",S_IRWXU);

mkdir("/mnt/int_sd/.jzintellivision",S_IRWXU);
mkdir("/mnt/int_sd/.jzintellivision/bios",S_IRWXU);
mkdir("/mnt/int_sd/.jzintellivision/overlays",S_IRWXU);
mkdir("/mnt/int_sd/.jzintellivision/configfiles",S_IRWXU);
#endif

    /* -------------------------------------------------------------------- */
    /*  On Windows, let's try to get back our stdio.                        */
    /* -------------------------------------------------------------------- */
#if 0
#ifdef WIN32
    {
        FILE *newfp;
        newfp   = fopen("CON:", "w");
        if (newfp) *stdout = *newfp;
        newfp   = fopen("CON:", "w");
        if (newfp) *stderr = *newfp;
        newfp   = fopen("CON:", "r");
        if (newfp) *stdin  = *newfp;
    }
#endif
#endif

#ifndef macintosh
    /* -------------------------------------------------------------------- */
    /*  Sneak real quick and see if the user included -h, --help, -?, or    */
    /*  no flags whatsoever.  In those cases, print a message and leave.    */
    /* -------------------------------------------------------------------- */
    if (argc < 2)
    {
        license();
    }

    for (iter = 1; iter < argc; iter++)
    {
        if (!strcmp(argv[iter], "--help") ||
            !strcmp(argv[iter], "-h")     ||
            !strcmp(argv[iter], "-?"))
        {
            usage();
        }
    }
#endif

    /* -------------------------------------------------------------------- */
    /*  Platform-specific initialization.                                   */
    /* -------------------------------------------------------------------- */
    if (plat_init())
    {
        fprintf(stderr, "Error initializing.\n");
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*  Temporary hack so ^C, kill work.                                    */
    /* -------------------------------------------------------------------- */
    signal( 2, graceful_death);
    signal(15, graceful_death);

    /* -------------------------------------------------------------------- */
    /*  Parse our arguments and go get an Intellivision!                    */
    /* -------------------------------------------------------------------- */
#ifdef macintosh
    argc = ccommand( &argv );
#endif
#ifdef USE_AS_BACKEND
	optind=0;
#endif
#ifdef GCWZERO
    static int gcw0_noexec;
    static int gcw0_noexec2;
    static int gcw0_nogrom;
    int cfg_outcome = cfg_init(&intv, argc, argv);
    if (cfg_outcome == 2)
    {
        gcw0_showerror(2);
    }
    if (cfg_outcome == 3)
    {
        gcw0_showerror(3);
    }
    if (cfg_outcome == 4)
    {
        gcw0_showerror(4);
    }
#else
    cfg_init(&intv, argc, argv);
#endif

    init_disp_width(0);
    jzp_flush();

    /* -------------------------------------------------------------------- */
    /*  On Windows, open \\.\UserPort\ if the program has requested         */
    /*  INTV2PC access.  This causes the UserPort driver to yield port      */
    /*  access, if UserPort is active.                                      */
    /* -------------------------------------------------------------------- */
#ifdef WIN32
    if (intv.i2pc0_port || intv.i2pc1_port)
    {
        FILE *UserPortFP;

        UserPortFP = fopen("\\\\.\\UserPort\\", "r");

        if (!UserPortFP)
        {
#if 0
            perror("fopen()");
            fprintf
            (
            stderr,
            "Warning: Unable to open \\\\.\\UserPort\\.\n"
            "         Under WinNT/2K/XP, jzIntv may be unable to use INTV2PC.\n"
            "         Win95/98/ME users can ignore this warning.\n"
            );
#endif
        }
    }
#endif


    /* -------------------------------------------------------------------- */
    /*  Set the window title.  If we recognize a standard Intellivision     */
    /*  ROM header at 0x5000, then also include the cartridge name.         */
    /* -------------------------------------------------------------------- */
    #if 1
    snprintf(title, 128, "jzintv %.32s : %.64s", release(), cart_name());
    title[127] = 0;
    gfx_set_title(&intv.gfx,title);
    #endif

    /* -------------------------------------------------------------------- */
    /*  Run the simulator.                                                  */
    /* -------------------------------------------------------------------- */
    if (intv.debugging)
        debug_tk((periph_p)&(intv.debug),1);

    jzp_printf("Starting jzIntv...\n");
    jzp_flush();

    if (intv.start_dly > 0)
        plat_delay(intv.start_dly);

restart:

    iter = 1;
    now = elapsed(1);
    while (now == elapsed(0))    /* Burn some time. */
        ;

    icyc   = 0;
    s_cnt  = 0;
    cycles = 0;
    if (intv.rate_ctl > 0.0)
        speed_resync(&(intv.speed));

    if (!intv.debugging)
        intv.debug.step_count = ~0U;

    paused = 0;

    while (intv.do_exit == 0)
    {

      uint_64 max_step;
        int do_reset = intv.do_reset;

        if (intv.gui_mode)
            do_gui_mode();

        if (intv.do_dump)
        {
			jzp_printf("\nDump requested.\n");
            intv.do_dump = 0;
			save_state(0);
		}

        if (intv.do_load)
        {
			jzp_printf("\nLoad requested.\n");
            intv.do_load = 0;
			load_dump();
		}

        if (do_reset)
        {
            if (intv.do_reset == 2)
                intv.do_reset = 0;
            max_step = 1000; /* arbitrary */
        } else
        {
            if (s_cnt)
            {
                s_cnt = 0;
                periph_reset(intv.intv);
            }
            /* This is incredibly hackish, and is an outgrowth of my
             * decoupled tick architecture.  */
            max_step = intv.stic.next_phase - intv.stic.stic_cr.now;
#if 1
            if (intv.cp1600.periph.now > intv.stic.stic_cr.now)
            {
                uint_64 diff;
                diff = intv.cp1600.periph.now - intv.stic.stic_cr.now;
                if (diff < max_step)
                    max_step -= diff;
            } else if (intv.stic.stic_cr.now > intv.cp1600.periph.now)
            {
                uint_64 diff;
                diff = intv.stic.stic_cr.now - intv.cp1600.periph.now;
                if (diff < max_step)
                    max_step -= diff;
            }
#endif
            if (max_step < 5) max_step = 5;
        }

#if 0
jzp_printf("cpu.now = %-8d  stic.now = %-8d diff = %-8d step = %-8d\n", (int)intv.cp1600.periph.now, (int)intv.stic.stic_cr.now, (int)intv.cp1600.periph.now-(int)intv.stic.stic_cr.now, (int)max_step);
#endif

        if (intv.do_pause)
        {
            paused = !paused; intv.do_pause = 0;
            if (!paused)
                speed_resync(&(intv.speed));
        }
#ifdef GCWZERO
	static int foundconfig=0;
	if (!foundconfig)
	{
		jzp_printf("Looking for gcw0 .config file...\n");
		foundconfig=1;
		FILE *configuration;
		configuration=fopen("/mnt/int_sd/.jzintellivision/.configuration","r");
		if (!configuration) {
			jzp_printf("None found, for now lets use default settings\n");
			menu_sound=configurationsettings[0];
			menu_customoverlays=configurationsettings[1];
			menu_transparency=configurationsettings[2];
			menu_overlayposition=configurationsettings[3];

		} else {
			jzp_printf("Found, loading configuration settings from file\n");
			int i;
			for (i = 0; i < 4; i++)
			{
				fscanf(configuration, "%1d,", &configurationsettings[i]);
			}
			menu_sound =		configurationsettings[0];
			menu_customoverlays =	configurationsettings[1];
			menu_transparency =	configurationsettings[2];
			menu_overlayposition =	configurationsettings[3];
			fclose(configuration);
		}
	}

	if (intv.event.do_menu)
	{
		drawmenu=1;
  	        paused = 1;

		//handle vertical movements
		if (intv.pad0.l[15]==4 || intv.pad0.r[15]==4) //left or right pad up
		{
			if 	(selectedoption == 0)	selectedoption=8;
			else if (selectedoption == 5)	selectedoption=3; //skip spacer
			else				selectedoption--;
			if 	(displaycontrolinfo)	selectedoption=5;
	                intv.pad0.l[15]=0;
	                intv.pad0.r[15]=0;
		}

		if (intv.pad0.l[15]==64 || intv.pad0.r[15]==64) //left or right pad down
		{
			if 	(selectedoption == 8)	selectedoption=0;
			else if (selectedoption == 3)	selectedoption=5; //skip spacer
			else				selectedoption++;
			if 	(displaycontrolinfo)	selectedoption=5;
	                intv.pad0.l[15]=0;
	                intv.pad0.r[15]=0;
		}
		//change settings
		//handle selected options with left or right pad
		if (	intv.pad0.l[11] || intv.pad0.r[11] || //kpad enter
			intv.pad0.l[12] || intv.pad0.r[12] || //top fire button
			intv.pad0.l[13] || intv.pad0.r[13] || //left fire button
			intv.pad0.l[14] || intv.pad0.r[14]  ) //right fire button
		{
			if (!selectedoption)
			{
				if (menu_sound) 		menu_sound=0;
				else 				menu_sound=1;
			}
			if (selectedoption == 1)
			{
				if (menu_customoverlays) 	menu_customoverlays=0;
				else 				menu_customoverlays=1;
			}
			if (selectedoption == 2)
			{
				if (menu_transparency) 		menu_transparency=0;
				else 				menu_transparency=1;
			}
			if (selectedoption == 3)
			{
				if 	(!menu_overlayposition) 	menu_overlayposition=1;//TR
				else if (menu_overlayposition==1)	menu_overlayposition=2;//BL
				else if (menu_overlayposition==2)	menu_overlayposition=3;//BR
				else if (menu_overlayposition==3)	menu_overlayposition=0;//TL
			}
			if (selectedoption == 5)//controls
			{
				displaycontrolinfo = (!displaycontrolinfo) ;
				
			}
			if (selectedoption == 6)//back
			{
				intv.event.do_menu = 0;
			}
			if (selectedoption == 7)//reset
			{
				intv.event.do_menu = 0;
		        	intv.do_reset = 2;
			}
			if (selectedoption == 8)//quit
			{
				intv.do_exit  = 1;
			}

			//now zero all fire buttons
			intv.pad0.l[11] = intv.pad0.r[11] = //kpad enter
			intv.pad0.l[12] = intv.pad0.r[12] = //top fire button
			intv.pad0.l[13] = intv.pad0.r[13] = //left fire button
			intv.pad0.l[14] = intv.pad0.r[14] = 0; //right fire button
			jzp_printf("\nConfig settings are: Sound %d CustomO %d Trans %d OverlayP %d\n",
				menu_sound,menu_customoverlays,menu_transparency,menu_overlayposition); 
//now save new configuration
			FILE *configuration;
			configuration=fopen("/mnt/int_sd/.jzintellivision/.configuration","w");
			fprintf(configuration, "%d%d%d%d", menu_sound, menu_customoverlays, menu_transparency, menu_overlayposition);
			fclose(configuration);
			}
	} else 
	{
		drawmenu=0;
  	        paused = 0;
//TODO breaks pause button!!!
	}

//TODO implement virtual keyboard
	if (intv.event.cur_kbd == 3)//map dpad movements to keyboard instead of numberpad
//pixels are 13x13, cells are 11x5, spacebar is 7 cells wide.
	{
                jzp_printf("Virtual keyboard controller selected");
		virtualnumberpad = 3; 

		if (intv.event.do_vkbd_up) 
		{
			if (kblettery == 0)
				kblettery=4;
			else	kblettery--;
	                intv.event.do_vkbd_up = 0;
		}
		if (intv.event.do_vkbd_down)
		{
			if (kblettery == 4)
				kblettery=0;
			else	kblettery++;
	                intv.event.do_vkbd_down = 0;
		}
		if (intv.event.do_vkbd_left)
		{
			if (kbletterx == 0)
				kbletterx=10;
			else if (kblettery==4 && kbletterx > 1 && kbletterx < 9) kbletterx=1;
			else kbletterx--;
	                intv.event.do_vkbd_left = 0;
		}
		if (intv.event.do_vkbd_right)
		{
			if (kbletterx == 10)
				kbletterx=0;
			else if (kblettery==4 && kbletterx > 1 && kbletterx < 9) kbletterx=9;
			else	kbletterx++;
	                intv.event.do_vkbd_right = 0;
		}
		if (intv.event.do_vkbd_select)//doesn't matter if player 1 or 2???
		{
			int keyboardnumber;
			keyboardnumber = kbletterx + 11*kblettery;
			if (!shiftpressed)
			{
				//ROW 1
				if (keyboardnumber==0)  intv.pad1.k[5]=16;                        //1
				if (keyboardnumber==1)  intv.pad1.k[4]=32;                        //2
				if (keyboardnumber==2)  intv.pad1.k[4]=16;                        //3
				if (keyboardnumber==3)  intv.pad1.k[3]=32;                        //4
				if (keyboardnumber==4)  intv.pad1.k[3]=16;                        //5
				if (keyboardnumber==5)  intv.pad1.k[2]=32;                        //6
				if (keyboardnumber==6)  intv.pad1.k[2]=16;                        //7
				if (keyboardnumber==7)  intv.pad1.k[1]=32;                        //8
				if (keyboardnumber==8)  intv.pad1.k[1]=16;                        //9
				if (keyboardnumber==9)  intv.pad1.k[0]=32;                        //0
				if (keyboardnumber==10) intv.pad1.k[0]=16;                        //ESC
				//ROW 2
				if (keyboardnumber==11) intv.pad1.k[5]=64;                        //CTL
				if (keyboardnumber==12) intv.pad1.k[5]=8;                         //Q
				if (keyboardnumber==13) intv.pad1.k[4]=8;                         //W
				if (keyboardnumber==14) intv.pad1.k[4]=64;                        //E
				if (keyboardnumber==15) intv.pad1.k[3]=8;                         //R
				if (keyboardnumber==16) intv.pad1.k[3]=64;                        //T
				if (keyboardnumber==17) intv.pad1.k[2]=8;                         //Y
				if (keyboardnumber==18) intv.pad1.k[2]=64;                        //U
				if (keyboardnumber==19) intv.pad1.k[1]=8;                         //I
				if (keyboardnumber==20) intv.pad1.k[1]=64;                        //O
				if (keyboardnumber==21) intv.pad1.k[0]=8;                         //P
				//ROW 3
				if (keyboardnumber==22) intv.pad1.k[5]=4;                         //up
				if (keyboardnumber==23) intv.pad1.k[5]=128;                       //A
				if (keyboardnumber==24) intv.pad1.k[4]=4;                         //S
				if (keyboardnumber==25) intv.pad1.k[4]=128;                       //D
				if (keyboardnumber==26) intv.pad1.k[3]=4;                         //F
				if (keyboardnumber==27) intv.pad1.k[3]=128;                       //G
				if (keyboardnumber==28) intv.pad1.k[2]=4;                         //H
				if (keyboardnumber==29) intv.pad1.k[2]=128;                       //J
				if (keyboardnumber==30) intv.pad1.k[1]=4;                         //K
				if (keyboardnumber==31) intv.pad1.k[1]=128;                       //L
				if (keyboardnumber==32) intv.pad1.k[0]=4;                         //;
				//ROW 4
				if (keyboardnumber==33) intv.pad1.k[0]=1;                         //left
				if (keyboardnumber==34) intv.pad1.k[5]=32;                        //right
				if (keyboardnumber==35) intv.pad1.k[4]=2;                         //Z
				if (keyboardnumber==36) intv.pad1.k[4]=1;                         //X
				if (keyboardnumber==37) intv.pad1.k[3]=2;                         //C
				if (keyboardnumber==38) intv.pad1.k[3]=1;                         //V
				if (keyboardnumber==39) intv.pad1.k[2]=2;                         //B
				if (keyboardnumber==40) intv.pad1.k[2]=1;                         //N
				if (keyboardnumber==41) intv.pad1.k[1]=2;                         //M
				if (keyboardnumber==42) intv.pad1.k[1]=1;                         //,
				if (keyboardnumber==43) intv.pad1.k[0]=2;                         //.
				//ROW 5
				if (keyboardnumber==44) intv.pad1.k[5]=2;                         //down
//				if (keyboardnumber==45) intv.pad1.k[6]=128;                       //SFT
				if (keyboardnumber==45) shiftpressed=(!shiftpressed);             //SFT
				if (keyboardnumber > 45 && keyboardnumber < 53) intv.pad1.k[5]=1; //space
//				if (keyboardnumber==53) intv.pad1.k[6]=128;                       //SFT
				if (keyboardnumber==53) shiftpressed=(!shiftpressed);             //SFT
				if (keyboardnumber==54) intv.pad1.k[0]=64;                        //ENTER
			} else {
				//ROW 1
				if (keyboardnumber==0)  intv.pad1.k[5]=(16 << 8);                 //=
				if (keyboardnumber==1)  intv.pad1.k[4]=(32 << 8);                 //"
				if (keyboardnumber==2)  intv.pad1.k[4]=(16 << 8);                 //#
				if (keyboardnumber==3)  intv.pad1.k[3]=(32 << 8);                 //$
				if (keyboardnumber==4)  intv.pad1.k[3]=(16 << 8);                 //+
				if (keyboardnumber==5)  intv.pad1.k[2]=(32 << 8);                 //-
				if (keyboardnumber==6)  intv.pad1.k[2]=(16 << 8);                 // /
				if (keyboardnumber==7)  intv.pad1.k[1]=(32 << 8);                 // *
				if (keyboardnumber==8)  intv.pad1.k[1]=(16 << 8);                 //(
				if (keyboardnumber==9)  intv.pad1.k[0]=(32 << 8);                 //)
				if (keyboardnumber==10) intv.pad1.k[0]=16;                        //ESC
				//ROW 2
				if (keyboardnumber==11) intv.pad1.k[5]=64;                        //CTL
				if (keyboardnumber==12) intv.pad1.k[5]=8;                         //Q
				if (keyboardnumber==13) intv.pad1.k[4]=8;                         //W
				if (keyboardnumber==14) intv.pad1.k[4]=64;                        //E
				if (keyboardnumber==15) intv.pad1.k[3]=8;                         //R
				if (keyboardnumber==16) intv.pad1.k[3]=64;                        //T
				if (keyboardnumber==17) intv.pad1.k[2]=8;                         //Y
				if (keyboardnumber==18) intv.pad1.k[2]=64;                        //U
				if (keyboardnumber==19) intv.pad1.k[1]=8;                         //I
				if (keyboardnumber==20) intv.pad1.k[1]=64;                        //O
				if (keyboardnumber==21) intv.pad1.k[0]=8;                         //P
				//ROW 3
				if (keyboardnumber==22) intv.pad1.k[5]=(4 << 8);                  //^
				if (keyboardnumber==23) intv.pad1.k[5]=128;                       //A
				if (keyboardnumber==24) intv.pad1.k[4]=4;                         //S
				if (keyboardnumber==25) intv.pad1.k[4]=128;                       //D
				if (keyboardnumber==26) intv.pad1.k[3]=4;                         //F
				if (keyboardnumber==27) intv.pad1.k[3]=128;                       //G
				if (keyboardnumber==28) intv.pad1.k[2]=4;                         //H
				if (keyboardnumber==29) intv.pad1.k[2]=128;                       //J
				if (keyboardnumber==30) intv.pad1.k[1]=4;                         //K
				if (keyboardnumber==31) intv.pad1.k[1]=128;                       //L
				if (keyboardnumber==32) intv.pad1.k[0]=(4 << 8);                  //:
				//ROW 4
//				if (keyboardnumber==33) intv.pad1.k[5]=(2 << 8);                  //%
//				if (keyboardnumber==34) intv.pad1.k[0]=(1 << 8);                  //'
				if (keyboardnumber==33) intv.pad1.k[0]=(1 << 8);                  //%
				if (keyboardnumber==34) intv.pad1.k[5]=(32 << 8);                 //'
				if (keyboardnumber==35) intv.pad1.k[4]=2;                         //Z
				if (keyboardnumber==36) intv.pad1.k[4]=1;                         //X
				if (keyboardnumber==37) intv.pad1.k[3]=2;                         //C
				if (keyboardnumber==38) intv.pad1.k[3]=1;                         //V
				if (keyboardnumber==39) intv.pad1.k[2]=2;                         //B
				if (keyboardnumber==40) intv.pad1.k[2]=1;                         //N
				if (keyboardnumber==41) intv.pad1.k[1]=2;                         //M
				if (keyboardnumber==42) intv.pad1.k[1]=(1 << 8);                  //<
				if (keyboardnumber==43) intv.pad1.k[0]=(2 << 8);                  //>
				//ROW 5
//				if (keyboardnumber==44) intv.pad1.k[5]=(32 << 8);                 //?
				if (keyboardnumber==44) intv.pad1.k[5]=(2 << 8);                  //?
//				if (keyboardnumber==45) intv.pad1.k[6]=128;                       //SFT
				if (keyboardnumber==45) shiftpressed=(!shiftpressed);             //SFT
				if (keyboardnumber > 45 && keyboardnumber < 53) intv.pad1.k[5]=1; //space
//				if (keyboardnumber==53) intv.pad1.k[6]=128;                       //SFT
				if (keyboardnumber==53) shiftpressed=(!shiftpressed);             //SFT
				if (keyboardnumber==54) intv.pad1.k[0]=64;                        //ENTER
			}

			static int keyboardcount=50;
			if (keyboardcount == 0) 
			{
				intv.pad1.k[0]=0;
				intv.pad1.k[1]=0;
				intv.pad1.k[2]=0;
				intv.pad1.k[3]=0;
				intv.pad1.k[4]=0;
				intv.pad1.k[5]=0;
				intv.pad1.k[6]=0;
				intv.event.do_vkbd_select=0;
				keyboardcount=50;
			} else keyboardcount--;
		}
	} else {
		if (intv.event.do_vkbd_up)
		{
			if 	(lettery == 0) 	lettery=3;
			else			lettery--;
	                intv.event.do_vkbd_up = 0;
		}
		if (intv.event.do_vkbd_down)
		{
			if 	(lettery == 3)	lettery=0;
			else			lettery++;
	                intv.event.do_vkbd_down = 0;
		}
		if (intv.event.do_vkbd_left)
		{
			if (letterx == 0)
				letterx=2;
			else	letterx--;
	                intv.event.do_vkbd_left = 0;
		}
		if (intv.event.do_vkbd_right)
		{
			if (letterx == 2)
				letterx=0;
			else	letterx++;
	                intv.event.do_vkbd_right = 0;
		}
		if (intv.event.do_vkbd_select) //select number
		{
			int numberpadnumber;
			if (lettery < 3) //numbers 1-9
				numberpadnumber=(3*lettery)+letterx+1;
			else //bottom row
			{
				if (letterx==0)
					numberpadnumber=10;//clear
				if (letterx==1)
					numberpadnumber=0;
				if (letterx==2)
					numberpadnumber=11;//enter
			}

// activate selected button!
			if (!displayplayer2)
			{
				if (numberpadnumber==0)
					intv.pad0.l[0]=0x48;
				if (numberpadnumber==1)
					intv.pad0.l[1]=0x81;
				if (numberpadnumber==2)
					intv.pad0.l[2]=0x41;
				if (numberpadnumber==3)
					intv.pad0.l[3]=0x21;
				if (numberpadnumber==4)
					intv.pad0.l[4]=0x82;
				if (numberpadnumber==5)
					intv.pad0.l[5]=0x42;
				if (numberpadnumber==6)
					intv.pad0.l[6]=0x22;
				if (numberpadnumber==7)
					intv.pad0.l[7]=0x84;
				if (numberpadnumber==8)
					intv.pad0.l[8]=0x44;
				if (numberpadnumber==9)
					intv.pad0.l[9]=0x24;
				if (numberpadnumber==10)
					intv.pad0.l[10]=0x88;
				if (numberpadnumber==11)
					intv.pad0.l[11]=0x28;
				static int numberpadcount=50;
				if (numberpadcount == 0) 
				{
					intv.pad0.l[0]=0;
					intv.pad0.l[1]=0;
					intv.pad0.l[2]=0;
					intv.pad0.l[3]=0;
					intv.pad0.l[4]=0;
					intv.pad0.l[5]=0;
					intv.pad0.l[6]=0;
					intv.pad0.l[7]=0;
					intv.pad0.l[8]=0;
					intv.pad0.l[9]=0;
					intv.pad0.l[10]=0;
					intv.pad0.l[11]=0;
					intv.event.do_vkbd_select=0;
					numberpadcount=50;
				} else numberpadcount--;
			} else {//player 2
				if (numberpadnumber==0)
					intv.pad0.r[0]=0x48;
				if (numberpadnumber==1)
					intv.pad0.r[1]=0x81;
				if (numberpadnumber==2)
					intv.pad0.r[2]=0x41;
				if (numberpadnumber==3)
					intv.pad0.r[3]=0x21;
				if (numberpadnumber==4)
					intv.pad0.r[4]=0x82;
				if (numberpadnumber==5)
					intv.pad0.r[5]=0x42;
				if (numberpadnumber==6)
					intv.pad0.r[6]=0x22;
				if (numberpadnumber==7)
					intv.pad0.r[7]=0x84;
				if (numberpadnumber==8)
					intv.pad0.r[8]=0x44;
				if (numberpadnumber==9)
					intv.pad0.r[9]=0x24;
				if (numberpadnumber==10)
					intv.pad0.r[10]=0x88;
				if (numberpadnumber==11)
					intv.pad0.r[11]=0x28;
				static int numberpadcount=50;
				if (numberpadcount == 0) 
				{
					intv.pad0.r[0]=0;
					intv.pad0.r[1]=0;
					intv.pad0.r[2]=0;
					intv.pad0.r[3]=0;
					intv.pad0.r[4]=0;
					intv.pad0.r[5]=0;
					intv.pad0.r[6]=0;
					intv.pad0.r[7]=0;
					intv.pad0.r[8]=0;
					intv.pad0.r[9]=0;
					intv.pad0.r[10]=0;
					intv.pad0.r[11]=0;
					intv.event.do_vkbd_select=0;
					numberpadcount=50;
				} else numberpadcount--;
			}

		}
	}
#endif

        if (intv.event.change_kbd)
        {
            if (intv.event.change_kbd == 5)
            {
                intv.event.cur_kbd = (intv.event.cur_kbd + 1) & 3;
            } else
            if (intv.event.change_kbd == 6)
            {
                intv.event.cur_kbd = (intv.event.cur_kbd - 1) & 3;
            } else
            if (intv.event.change_kbd == 7)
            {
                if (intv.event.prv_kbd)
                {
                    intv.event.cur_kbd = intv.event.prv_kbd - 1;
                    intv.event.prv_kbd = 0;
                }
            } else
            if (intv.event.change_kbd >= 8 && intv.event.change_kbd < 12)
            {
                intv.event.prv_kbd = intv.event.cur_kbd + 1;
                intv.event.cur_kbd = (intv.event.change_kbd - 8) & 3;
            } else
            {
                intv.event.cur_kbd = (intv.event.change_kbd - 1) & 3;
            }

#ifdef GCWZERO
            jzp_clear_and_eol(
                jzp_printf("Change keyboard to %d", intv.event.cur_kbd));
 	    virtualnumberpad = intv.event.cur_kbd;
	    if (virtualnumberpad == 2) {
		displayplayer2=(!displayplayer2);
		virtualnumberpad = 0;
	    }
#else 
            jzp_clear_and_eol(
                jzp_printf("Change keyboard to %d", intv.event.cur_kbd));
#endif
            jzp_flush();
            intv.event.change_kbd = 0;
            memset(intv.pad0.l, 0, sizeof(intv.pad0.l));
            memset(intv.pad0.r, 0, sizeof(intv.pad0.r));
            memset(intv.pad0.k, 0, sizeof(intv.pad0.k));
            memset(intv.pad1.l, 0, sizeof(intv.pad1.l));
            memset(intv.pad1.r, 0, sizeof(intv.pad1.r));
            memset(intv.pad1.k, 0, sizeof(intv.pad1.k));
        }


        if (paused)
        {
            intv.gfx.dirty = 1;
            intv.gfx.periph.tick  ((periph_p)&(intv.gfx),   20000);
            intv.event.periph.tick((periph_p)&(intv.event), 0);
            plat_delay(1000/60);
        } else if (do_reset)
        {
            intv.gfx.dirty = 1;
            intv.gfx.periph.tick  ((periph_p)&(intv.gfx),   20000);
            intv.event.periph.tick((periph_p)&(intv.event), 20000);
            plat_delay(1000/60);
            cycles += 20000;
        } else
            cycles += periph_tick((periph_p)(intv.intv), max_step);

        if (!intv.debugging && intv.debug.step_count == 0)
            intv.debug.step_count = ~0U;

        if (!intv.debugging && !do_reset && (iter++&1023) == 0)
        {

            then  = now;
            now   = elapsed(0);
            rate  = (cycles / now);
            if (now - then > 0.01)
            {
                irate = (cycles - icyc) / (now - then);
                icyc  = cycles;

#ifdef GCWZERO
#else
                jzp_printf("Rate: [%6.2f%% %6.2f%%]  Drop Gfx:[%6.2f%% %6d] "
                       "Snd:[%6.2f%% %2d %6.3f]\r",
                        rate * 100., irate * 100.,
                        100. * intv.gfx.tot_dropped_frames / intv.gfx.tot_frames,
                        (int)intv.gfx.tot_dropped_frames,
                        100. * intv.snd.mixbuf.tot_drop / intv.snd.tot_frame,
                        (int)intv.snd.mixbuf.tot_drop,
                        (double)intv.snd.tot_dirty / intv.snd.tot_frame);
#endif

#if 0
                jzp_printf("speed: min=%-8d max=%-8d thresh=%-8.1f frame=%-8d\n",
                        intv.speed.periph.min_tick,
                        intv.speed.periph.max_tick,
                        intv.speed.threshold * 1e6,
                        intv.gfx.tot_frames);
#endif
                jzp_flush();
            }

            if ((iter&65535) == 1)
            {
                then = elapsed(1);
                cycles = icyc = 0;
            }
        }

        if (do_reset)
        {
            intv.cp1600.r[7] = 0x1000;
            gfx_vid_enable(&(intv.gfx), 0);
            s_cnt++;
        } else
        {
            if (s_cnt > 140) break;
            s_cnt = 0;
        }
    }

    s_cnt = 0x2A3A4A5A;

    arg = 0;
    gfx_set_bord  (&(intv.gfx), 0);
    gfx_vid_enable(&(intv.gfx), 1);
    intv.gfx.scrshot |= GFX_RESET;

    while (intv.do_exit == 0)
    {
        int i, j;
        uint_8 p;

        if (intv.gui_mode)
            do_gui_mode();

        if (intv.do_reset) arg = 1;
        if (intv.do_reset != 1 && arg)
        {
            intv.do_reset = 0;

            p = intv.stic.raw[0x2C] & 15;
            for (i = 0; i < 160 * 200; i++)
                intv.stic.disp[i] = p;

            gfx_set_bord  (&(intv.gfx), p);
            intv.gfx.scrshot &= ~GFX_RESET;
            goto restart;
        }

        for (i = 0; i < 160 * 200; i++)
        {
            for (j = 0; j < 16; j++)
                s_cnt = (s_cnt << 1) | (1 & ((s_cnt >> 28) ^ (s_cnt >> 30)));


            p = (s_cnt & 0xF) + 16;
            if (p == 16) p = 0;

            intv.stic.disp[i] = p;
        }

        intv.gfx.dirty = 1;
        intv.gfx.periph.tick  ((periph_p)&(intv.gfx),   0);
        intv.event.periph.tick((periph_p)&(intv.event), 0);
        plat_delay(1000/60);
    }

    if (intv.do_exit)
        jzp_printf("\nExited on user request.\n");

    cfg_dtor(&intv);

    return 0;
}


#include "cp1600/op_decode.h"
#include "cp1600/op_exec.h"

/*
 * ============================================================================
 *  DUMP_STATE   -- Called by op_exec:fn_HLT.  Dumps state of universe.
 * ============================================================================
 */

void dump_state(void)
{
    FILE *f;
    int addr, data, i, j;

    f = fopen("dump.mem","wb");
    if (!f)
    {
        perror("fopen(\"dump.mem\", \"w\")");
        jzp_printf("couldn't open dump.mem, not dumping memory.\n");
    }
    else
    {
        intv.debug.show_rd = 0;
        intv.debug.show_wr = 0;
        for (addr = 0; addr <= 0xFFFF; addr++)
        {
            data = periph_peek((periph_p)intv.intv, (periph_p)intv.intv,
                               addr, 0);
            fputc((data >> 8) & 0xFF, f);
            fputc((data     ) & 0xFF, f);
        }

        fclose(f);
    }

    f = fopen("dump.cpu", "wb");
    if (!f)
    {
        perror("fopen(\"dump.cpu\", \"w\")");
        jzp_printf("couldn't open dump.cpu, not dumping cpu info.\n"); return;
    }

    fprintf(f, "CP-1600 State Dump\n");
    fprintf(f, "Tot Cycles:   %lld\n", intv.cp1600.tot_cycle);
    fprintf(f, "Tot Instrs:   %lld\n", intv.cp1600.tot_instr);
    fprintf(f, "Tot Cache:    %d\n", intv.cp1600.tot_cache);
    fprintf(f, "Tot NonCache: %d\n", intv.cp1600.tot_noncache);
    fprintf(f, "Registers:    %.4x %.4x %.4x %.4x %.4x %.4x %.4x %.4x\n",
            intv.cp1600.r[0], intv.cp1600.r[1],
            intv.cp1600.r[2], intv.cp1600.r[3],
            intv.cp1600.r[4], intv.cp1600.r[5],
            intv.cp1600.r[6], intv.cp1600.r[7]);
    fprintf(f, "Flags:        S:%d C:%d O:%d Z:%d I:%d D:%d intr:%d irq:%d\n",
            intv.cp1600.S, intv.cp1600.C, intv.cp1600.O, intv.cp1600.Z,
            intv.cp1600.I, intv.cp1600.D,
            intv.cp1600.intr, intv.cp1600.req_bus.intrq);

    fprintf(f, "Cacheability Map:\n");

    for (i = 0; i < 1 << (CP1600_MEMSIZE-CP1600_DECODE_PAGE - 5); i++)
    {
        addr = (i << (CP1600_DECODE_PAGE + 5));

        fprintf(f, "   %.4x-%.4x:", addr, addr+(32<<CP1600_DECODE_PAGE)-1);
        for (j = 0; j < 32; j++)
        {
            fprintf(f, " %d", 1 & (intv.cp1600.cacheable[i] >> j));
        }
        fprintf(f, "\n");
    }


    fprintf(f,"Decoded Instruction Map:\n");

    for (i = 0; i < 1 << (CP1600_MEMSIZE - 6); i++)
    {
        addr = i << 6;

        fprintf(f, "   %.4x-%.4x:", addr, addr + 63);
        for (j = 0; j < 64; j++)
        {
            fprintf(f, "%c",
                    intv.cp1600.execute[addr + j] == fn_decode_1st ? '-' :
                    intv.cp1600.execute[addr + j] == fn_decode     ? 'N' :
                    intv.cp1600.execute[addr + j] == fn_invalid    ? '!' :
                                                                     'C');
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

/*
 * ============================================================================
 *  SAVE_STATE
 * ============================================================================
 */

void save_state(int altname)
{
    FILE *f;
    int addr, i, k;
	unsigned short data;

	if (altname) {
		f = fopen("dump_check.sav","wb");
	} else {
		f = fopen("dump.sav","wb");
	}
    if (!f)
    {
        perror("fopen(\"dump.sav\", \"w\")");
        jzp_printf("couldn't open dump.sav.\n");

		return;
    }

    char phase[7];
    memcpy(phase, "phase_1", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    fwrite(&intv.cp1600.periph.addr_base, sizeof(intv.cp1600.periph.addr_base), 1, f);
    fwrite(&intv.cp1600.periph.addr_mask, sizeof(intv.cp1600.periph.addr_mask), 1, f);
    fwrite(&intv.cp1600.periph.now, sizeof(intv.cp1600.periph.now), 1, f);
    fwrite(&intv.cp1600.periph.min_tick, sizeof(intv.cp1600.periph.min_tick), 1, f);
    fwrite(&intv.cp1600.periph.max_tick, sizeof(intv.cp1600.periph.max_tick), 1, f);
    fwrite(&intv.cp1600.periph.next_tick, sizeof(intv.cp1600.periph.next_tick), 1, f);

    fwrite(&intv.cp1600.snoop.addr_base, sizeof(intv.cp1600.snoop.addr_base), 1, f);
    fwrite(&intv.cp1600.snoop.addr_mask, sizeof(intv.cp1600.snoop.addr_mask), 1, f);
    fwrite(&intv.cp1600.snoop.now, sizeof(intv.cp1600.snoop.now), 1, f);
    fwrite(&intv.cp1600.snoop.min_tick, sizeof(intv.cp1600.snoop.min_tick), 1, f);
    fwrite(&intv.cp1600.snoop.max_tick, sizeof(intv.cp1600.snoop.max_tick), 1, f);
    fwrite(&intv.cp1600.snoop.next_tick, sizeof(intv.cp1600.snoop.next_tick), 1, f);

    fwrite(&intv.cp1600.tot_cycle, sizeof(intv.cp1600.tot_cycle), 1, f);
    fwrite(&intv.cp1600.tot_instr, sizeof(intv.cp1600.tot_instr), 1, f);
    fwrite(&intv.cp1600.tot_cache, sizeof(intv.cp1600.tot_cache), 1, f);
    fwrite(&intv.cp1600.tot_noncache, sizeof(intv.cp1600.tot_noncache), 1, f);

    for (i = 0; i <= 7; i++)
    {
        fwrite(&intv.cp1600.r[i], sizeof(intv.cp1600.r[i]), 1, f);
    }

    fwrite(&intv.cp1600.S, sizeof(intv.cp1600.S), 1, f);
    fwrite(&intv.cp1600.C, sizeof(intv.cp1600.C), 1, f);
    fwrite(&intv.cp1600.O, sizeof(intv.cp1600.O), 1, f);
    fwrite(&intv.cp1600.Z, sizeof(intv.cp1600.Z), 1, f);
    fwrite(&intv.cp1600.I, sizeof(intv.cp1600.I), 1, f);
    fwrite(&intv.cp1600.D, sizeof(intv.cp1600.D), 1, f);

    fwrite(&intv.cp1600.intr, sizeof(intv.cp1600.intr), 1, f);

    fwrite(&intv.cp1600.req_bus.intrq, sizeof(intv.cp1600.req_bus.intrq), 1, f);

    memcpy(phase, "phase_2", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    fwrite(&intv.cp1600.req_bus.intrq_until, sizeof(intv.cp1600.req_bus.intrq_until), 1, f);
    fwrite(&intv.cp1600.req_bus.busrq_until, sizeof(intv.cp1600.req_bus.busrq_until), 1, f);
    fwrite(&intv.cp1600.req_bus.next_intrq, sizeof(intv.cp1600.req_bus.next_intrq), 1, f);
    fwrite(&intv.cp1600.req_bus.next_busrq, sizeof(intv.cp1600.req_bus.next_busrq), 1, f);
    fwrite(&intv.cp1600.req_bus.intak, sizeof(intv.cp1600.req_bus.intak), 1, f);
    fwrite(&intv.cp1600.req_bus.busak, sizeof(intv.cp1600.req_bus.busak), 1, f);

    memcpy(phase, "phase_3", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    fwrite(&intv.cp1600.oldpc, sizeof(intv.cp1600.oldpc), 1, f);
    fwrite(&intv.cp1600.ext, sizeof(intv.cp1600.ext), 1, f);
    fwrite(&intv.cp1600.int_vec, sizeof(intv.cp1600.int_vec), 1, f);

    memcpy(phase, "phase_4", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    for (i = 0; i < 1 << (CP1600_MEMSIZE-CP1600_DECODE_PAGE - 5); i++)
    {
        fwrite(&intv.cp1600.cacheable[i], sizeof(intv.cp1600.cacheable[i]), 1, f);
    }

    memcpy(phase, "phase_5", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    fwrite(&intv.cp1600.instr_tick_per, sizeof(intv.cp1600.instr_tick_per), 1, f);

	for (addr = 0; addr <= 0xFFFF; addr++)
	{
		data = periph_peek((periph_p)intv.intv, (periph_p)intv.intv,
						   addr, 0);
        fwrite(&data, sizeof(data), 1, f);
	}

    memcpy(phase, "phase_6", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    fwrite(&intv.stic.stic_cr.addr_base, sizeof(intv.stic.stic_cr.addr_base), 1, f);
    fwrite(&intv.stic.stic_cr.addr_mask, sizeof(intv.stic.stic_cr.addr_mask), 1, f);
    fwrite(&intv.stic.stic_cr.now, sizeof(intv.stic.stic_cr.now), 1, f);
    fwrite(&intv.stic.stic_cr.min_tick, sizeof(intv.stic.stic_cr.min_tick), 1, f);
    fwrite(&intv.stic.stic_cr.max_tick, sizeof(intv.stic.stic_cr.max_tick), 1, f);
    fwrite(&intv.stic.stic_cr.next_tick, sizeof(intv.stic.stic_cr.next_tick), 1, f);

    fwrite(&intv.stic.snoop_btab.addr_base, sizeof(intv.stic.snoop_btab.addr_base), 1, f);
    fwrite(&intv.stic.snoop_btab.addr_mask, sizeof(intv.stic.snoop_btab.addr_mask), 1, f);
    fwrite(&intv.stic.snoop_btab.now, sizeof(intv.stic.snoop_btab.now), 1, f);
    fwrite(&intv.stic.snoop_btab.min_tick, sizeof(intv.stic.snoop_btab.min_tick), 1, f);
    fwrite(&intv.stic.snoop_btab.max_tick, sizeof(intv.stic.snoop_btab.max_tick), 1, f);
    fwrite(&intv.stic.snoop_btab.next_tick, sizeof(intv.stic.snoop_btab.next_tick), 1, f);

    fwrite(&intv.stic.snoop_gram.addr_base, sizeof(intv.stic.snoop_gram.addr_base), 1, f);
    fwrite(&intv.stic.snoop_gram.addr_mask, sizeof(intv.stic.snoop_gram.addr_mask), 1, f);
    fwrite(&intv.stic.snoop_gram.now, sizeof(intv.stic.snoop_gram.now), 1, f);
    fwrite(&intv.stic.snoop_gram.min_tick, sizeof(intv.stic.snoop_gram.min_tick), 1, f);
    fwrite(&intv.stic.snoop_gram.max_tick, sizeof(intv.stic.snoop_gram.max_tick), 1, f);
    fwrite(&intv.stic.snoop_gram.next_tick, sizeof(intv.stic.snoop_gram.next_tick), 1, f);

    fwrite(&intv.stic.req_bus->intrq, sizeof(intv.stic.req_bus->intrq), 1, f);
    fwrite(&intv.stic.req_bus->intrq_until, sizeof(intv.stic.req_bus->intrq_until), 1, f);
    fwrite(&intv.stic.req_bus->busrq_until, sizeof(intv.stic.req_bus->busrq_until), 1, f);
    fwrite(&intv.stic.req_bus->next_intrq, sizeof(intv.stic.req_bus->next_intrq), 1, f);
    fwrite(&intv.stic.req_bus->next_busrq, sizeof(intv.stic.req_bus->next_busrq), 1, f);
    fwrite(&intv.stic.req_bus->intak, sizeof(intv.stic.req_bus->intak), 1, f);
    fwrite(&intv.stic.req_bus->busak, sizeof(intv.stic.req_bus->busak), 1, f);

    fwrite(&intv.stic.cp_row, sizeof(intv.stic.cp_row), 1, f);
    fwrite(&intv.stic.ve_post, sizeof(intv.stic.ve_post), 1, f);
    fwrite(&intv.stic.vid_enable, sizeof(intv.stic.vid_enable), 1, f);
    fwrite(&intv.stic.mode, sizeof(intv.stic.mode), 1, f);
    fwrite(&intv.stic.p_mode, sizeof(intv.stic.p_mode), 1, f);
    fwrite(&intv.stic.bt_dirty, sizeof(intv.stic.bt_dirty), 1, f);
    fwrite(&intv.stic.gr_dirty, sizeof(intv.stic.gr_dirty), 1, f);
    fwrite(&intv.stic.ob_dirty, sizeof(intv.stic.ob_dirty), 1, f);

    fwrite(&intv.stic.rand_regs, sizeof(intv.stic.rand_regs), 1, f);
    fwrite(&intv.stic.pal, sizeof(intv.stic.pal), 1, f);
    fwrite(&intv.stic.drop_frame, sizeof(intv.stic.drop_frame), 1, f);
    fwrite(&intv.stic.gmem_accessible, sizeof(intv.stic.gmem_accessible), 1, f);
    fwrite(&intv.stic.stic_accessible, sizeof(intv.stic.stic_accessible), 1, f);
    fwrite(&intv.stic.next_irq, sizeof(intv.stic.next_irq), 1, f);

    memcpy(phase, "phase_7", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    fwrite(&intv.stic.phase, sizeof(intv.stic.phase), 1, f);
    fwrite(&intv.stic.next_phase, sizeof(intv.stic.next_phase), 1, f);

    memcpy(phase, "phase_8", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    for (i=0; i<0x40; i++) {
        fwrite(&intv.stic.raw[i], sizeof(intv.stic.raw[i]), 1, f);
    }

    for (i=0; i<0x140 * 8; i++) {
        fwrite(&intv.stic.gmem[i], sizeof(intv.stic.gmem[i]), 1, f);
    }

    fwrite(&intv.stic.fifo_ptr, sizeof(intv.stic.fifo_ptr), 1, f);

    memcpy(phase, "phase8a", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    for (i=0; i<240; i++) {
        fwrite(&intv.stic.btab_sr[i], sizeof(intv.stic.btab_sr[i]), 1, f);
    }
    for (i=0; i<240; i++) {
        fwrite(&intv.stic.btab[i], sizeof(intv.stic.btab[i]), 1, f);
    }
    for (i=0; i<12; i++) {
        fwrite(&intv.stic.last_bg[i], sizeof(intv.stic.last_bg[i]), 1, f);
    }
    memcpy(phase, "phase8b", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    fwrite(&intv.stic.time, sizeof(intv.stic.time), 1, f);

    memcpy(phase, "phase8c", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    for (i=0; i<240*8; i++) {
        fwrite(&intv.stic.bt_bmp[i], sizeof(intv.stic.bt_bmp[i]), 1, f);
    }
    for (i=0; i<16*16  / 8; i++) {
        fwrite(&intv.stic.mob_img[i], sizeof(intv.stic.mob_img[i]), 1, f);
    }
    for (i=0; i<8; i++) {
        for (k=0; k<16; k++) {
            fwrite(&intv.stic.mob_bmp[i][k], sizeof(intv.stic.mob_bmp[i][k]), 1, f);
        }
    }
    for (i=0; i<192*224 / 8; i++) {
        fwrite(&intv.stic.mpl_img[i], sizeof(intv.stic.mpl_img[i]), 1, f);
    }
    for (i=0; i<192*224 /32; i++) {
        fwrite(&intv.stic.mpl_vsb[i], sizeof(intv.stic.mpl_vsb[i]), 1, f);
    }
    for (i=0; i<192*224 /32; i++) {
        fwrite(&intv.stic.mpl_pri[i], sizeof(intv.stic.mpl_pri[i]), 1, f);
    }
    for (i=0; i<192*224 / 8; i++) {
        fwrite(&intv.stic.xbt_img[i], sizeof(intv.stic.xbt_img[i]), 1, f);
    }
    for (i=0; i<192*224 /32; i++) {
        fwrite(&intv.stic.xbt_bmp[i], sizeof(intv.stic.xbt_bmp[i]), 1, f);
    }
    for (i=0; i<192*224 / 8; i++) {
        fwrite(&intv.stic.image[i], sizeof(intv.stic.image[i]), 1, f);
    }

    memcpy(phase, "phase8d", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    for (i=0; i<8; i++) {
        for (k=0; k<4; k++) {
            fwrite(&intv.stic.gfx->bbox[i][k], sizeof(intv.stic.gfx->bbox[i][k]), 1, f);
        }
    }

    fwrite(&intv.stic.gfx->dirty, sizeof(intv.stic.gfx->dirty), 1, f);
    fwrite(&intv.stic.gfx->drop_frame, sizeof(intv.stic.gfx->drop_frame), 1, f);
    fwrite(&intv.stic.gfx->dropped_frames, sizeof(intv.stic.gfx->dropped_frames), 1, f);
    fwrite(&intv.stic.gfx->tot_frames, sizeof(intv.stic.gfx->tot_frames), 1, f);
    fwrite(&intv.stic.gfx->tot_dropped_frames, sizeof(intv.stic.gfx->tot_dropped_frames), 1, f);

    fwrite(&intv.stic.gfx->b_color, sizeof(intv.stic.gfx->b_color), 1, f);
    fwrite(&intv.stic.gfx->b_dirty, sizeof(intv.stic.gfx->b_dirty), 1, f);
    fwrite(&intv.stic.gfx->x_blank, sizeof(intv.stic.gfx->x_blank), 1, f);
    fwrite(&intv.stic.gfx->y_blank, sizeof(intv.stic.gfx->y_blank), 1, f);
    fwrite(&intv.stic.gfx->x_delay, sizeof(intv.stic.gfx->x_delay), 1, f);
    fwrite(&intv.stic.gfx->y_delay, sizeof(intv.stic.gfx->y_delay), 1, f);
    fwrite(&intv.stic.gfx->debug_blank, sizeof(intv.stic.gfx->debug_blank), 1, f);

    memcpy(phase, "phase8e", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    fwrite(&intv.stic.gfx->periph.addr_base, sizeof(intv.stic.gfx->periph.addr_base), 1, f);
    fwrite(&intv.stic.gfx->periph.addr_mask, sizeof(intv.stic.gfx->periph.addr_mask), 1, f);
    fwrite(&intv.stic.gfx->periph.now, sizeof(intv.stic.gfx->periph.now), 1, f);
    fwrite(&intv.stic.gfx->periph.min_tick, sizeof(intv.stic.gfx->periph.min_tick), 1, f);
    fwrite(&intv.stic.gfx->periph.max_tick, sizeof(intv.stic.gfx->periph.max_tick), 1, f);
    fwrite(&intv.stic.gfx->periph.next_tick, sizeof(intv.stic.gfx->periph.next_tick), 1, f);

    fwrite(&intv.stic.gfx->periph.next_tick, sizeof(intv.stic.gfx->periph.next_tick), 1, f);

    memcpy(phase, "phase_9", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

#if 0
	for (k=0; k<32; k++) {
        fwrite(&intv.exec.banksw[k], sizeof(intv.exec.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.exec2.banksw[k], sizeof(intv.exec2.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.sys_ram.banksw[k], sizeof(intv.sys_ram.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.sys_ram2.banksw[k], sizeof(intv.sys_ram2.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.scr_ram.banksw[k], sizeof(intv.scr_ram.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.glt_ram.banksw[k], sizeof(intv.glt_ram.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.game0.banksw[k], sizeof(intv.game0.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.game1.banksw[k], sizeof(intv.game1.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.game2.banksw[k], sizeof(intv.game2.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.ecs0.banksw[k], sizeof(intv.ecs0.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.ecs1.banksw[k], sizeof(intv.ecs1.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.ecs2.banksw[k], sizeof(intv.ecs2.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fwrite(&intv.ecs_ram.banksw[k], sizeof(intv.ecs_ram.banksw[k]), 1, f);
	}
#endif

    memcpy(phase, "phase10", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

	for (k=0; k<4096 + 256; k++) {
        fwrite(&intv.exec_img[k], sizeof(intv.exec_img[k]), 1, f);
	}
	for (k=0; k<2048; k++) {
        fwrite(&intv.grom_img[k], sizeof(intv.grom_img[k]), 1, f);
	}
	for (k=0; k<4096 * 3; k++) {
        fwrite(&intv.ecs_img[k], sizeof(intv.ecs_img[k]), 1, f);
	}

    memcpy(phase, "phase11", sizeof(phase));
	fwrite(phase, sizeof(phase), 1, f);

    // Write periph's notion of 'now'
    fwrite(&intv.intv->periph.now, sizeof(intv.intv->periph.now), 1, f);

    fclose(f);
}


/*
 * ============================================================================
 *  LOAD_DUMP
 * ============================================================================
 */

void load_dump()
{
    FILE *f;
    int addr, i, test, k;
	unsigned short data, data2;
	char phase[8];
    memset(phase, 0, sizeof(phase));

    f = fopen("dump.sav","rb");
    if (!f)
    {
        perror("fopen(\"dump.sav\", \"r\")");
        jzp_printf("couldn't open dump.sav.\n");

		return;
    }

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase);//1

    fread(&intv.cp1600.periph.addr_base, sizeof(intv.cp1600.periph.addr_base), 1, f);
    fread(&intv.cp1600.periph.addr_mask, sizeof(intv.cp1600.periph.addr_mask), 1, f);
    fread(&intv.cp1600.periph.now, sizeof(intv.cp1600.periph.now), 1, f);
    fread(&intv.cp1600.periph.min_tick, sizeof(intv.cp1600.periph.min_tick), 1, f);
    fread(&intv.cp1600.periph.max_tick, sizeof(intv.cp1600.periph.max_tick), 1, f);
    fread(&intv.cp1600.periph.next_tick, sizeof(intv.cp1600.periph.next_tick), 1, f);

    fread(&intv.cp1600.snoop.addr_base, sizeof(intv.cp1600.snoop.addr_base), 1, f);
    fread(&intv.cp1600.snoop.addr_mask, sizeof(intv.cp1600.snoop.addr_mask), 1, f);
    fread(&intv.cp1600.snoop.now, sizeof(intv.cp1600.snoop.now), 1, f);
    fread(&intv.cp1600.snoop.min_tick, sizeof(intv.cp1600.snoop.min_tick), 1, f);
    fread(&intv.cp1600.snoop.max_tick, sizeof(intv.cp1600.snoop.max_tick), 1, f);
    fread(&intv.cp1600.snoop.next_tick, sizeof(intv.cp1600.snoop.next_tick), 1, f);

    fread(&intv.cp1600.tot_cycle, sizeof(intv.cp1600.tot_cycle), 1, f);
    fread(&intv.cp1600.tot_instr, sizeof(intv.cp1600.tot_instr), 1, f);
    fread(&intv.cp1600.tot_cache, sizeof(intv.cp1600.tot_cache), 1, f);
    fread(&intv.cp1600.tot_noncache, sizeof(intv.cp1600.tot_noncache), 1, f);

    for (i = 0; i <= 7; i++)
    {
        fread(&intv.cp1600.r[i], sizeof(intv.cp1600.r[i]), 1, f);
    }

    fread(&intv.cp1600.S, sizeof(intv.cp1600.S), 1, f);
    fread(&intv.cp1600.C, sizeof(intv.cp1600.C), 1, f);
    fread(&intv.cp1600.O, sizeof(intv.cp1600.O), 1, f);
    fread(&intv.cp1600.Z, sizeof(intv.cp1600.Z), 1, f);
    fread(&intv.cp1600.I, sizeof(intv.cp1600.I), 1, f);
    fread(&intv.cp1600.D, sizeof(intv.cp1600.D), 1, f);

    fread(&intv.cp1600.intr, sizeof(intv.cp1600.intr), 1, f);


	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //2

    fread(&intv.cp1600.req_bus.intrq, sizeof(intv.cp1600.req_bus.intrq), 1, f);
    fread(&intv.cp1600.req_bus.intrq_until, sizeof(intv.cp1600.req_bus.intrq_until), 1, f);
    fread(&intv.cp1600.req_bus.busrq_until, sizeof(intv.cp1600.req_bus.busrq_until), 1, f);
    fread(&intv.cp1600.req_bus.next_intrq, sizeof(intv.cp1600.req_bus.next_intrq), 1, f);
    fread(&intv.cp1600.req_bus.next_busrq, sizeof(intv.cp1600.req_bus.next_busrq), 1, f);
    fread(&intv.cp1600.req_bus.intak, sizeof(intv.cp1600.req_bus.intak), 1, f);
    fread(&intv.cp1600.req_bus.busak, sizeof(intv.cp1600.req_bus.busak), 1, f);

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //3

    fread(&intv.cp1600.oldpc, sizeof(intv.cp1600.oldpc), 1, f);
    fread(&intv.cp1600.ext, sizeof(intv.cp1600.ext), 1, f);
    fread(&intv.cp1600.int_vec, sizeof(intv.cp1600.int_vec), 1, f);

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //4

    for (i = 0; i < 1 << (CP1600_MEMSIZE-CP1600_DECODE_PAGE - 5); i++)
    {
        fread(&intv.cp1600.cacheable[i], sizeof(intv.cp1600.cacheable[i]), 1, f);
    }

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //5

    fread(&intv.cp1600.instr_tick_per, sizeof(intv.cp1600.instr_tick_per), 1, f);

	for (addr = 0; addr <= 0xFFFF; addr++)
	{
        fread(&data, sizeof(data), 1, f);
        if (addr > 0x40)
        {
            periph_write((periph_p)intv.intv, (periph_p)intv.intv,
                               addr, data);
        }
	}

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //6

    fread(&intv.stic.stic_cr.addr_base, sizeof(intv.stic.stic_cr.addr_base), 1, f);
    fread(&intv.stic.stic_cr.addr_mask, sizeof(intv.stic.stic_cr.addr_mask), 1, f);
    fread(&intv.stic.stic_cr.now, sizeof(intv.stic.stic_cr.now), 1, f);
    fread(&intv.stic.stic_cr.min_tick, sizeof(intv.stic.stic_cr.min_tick), 1, f);
    fread(&intv.stic.stic_cr.max_tick, sizeof(intv.stic.stic_cr.max_tick), 1, f);
    fread(&intv.stic.stic_cr.next_tick, sizeof(intv.stic.stic_cr.next_tick), 1, f);

    fread(&intv.stic.snoop_btab.addr_base, sizeof(intv.stic.snoop_btab.addr_base), 1, f);
    fread(&intv.stic.snoop_btab.addr_mask, sizeof(intv.stic.snoop_btab.addr_mask), 1, f);
    fread(&intv.stic.snoop_btab.now, sizeof(intv.stic.snoop_btab.now), 1, f);
    fread(&intv.stic.snoop_btab.min_tick, sizeof(intv.stic.snoop_btab.min_tick), 1, f);
    fread(&intv.stic.snoop_btab.max_tick, sizeof(intv.stic.snoop_btab.max_tick), 1, f);
    fread(&intv.stic.snoop_btab.next_tick, sizeof(intv.stic.snoop_btab.next_tick), 1, f);

    fread(&intv.stic.snoop_gram.addr_base, sizeof(intv.stic.snoop_gram.addr_base), 1, f);
    fread(&intv.stic.snoop_gram.addr_mask, sizeof(intv.stic.snoop_gram.addr_mask), 1, f);
    fread(&intv.stic.snoop_gram.now, sizeof(intv.stic.snoop_gram.now), 1, f);
    fread(&intv.stic.snoop_gram.min_tick, sizeof(intv.stic.snoop_gram.min_tick), 1, f);
    fread(&intv.stic.snoop_gram.max_tick, sizeof(intv.stic.snoop_gram.max_tick), 1, f);
    fread(&intv.stic.snoop_gram.next_tick, sizeof(intv.stic.snoop_gram.next_tick), 1, f);

    fread(&intv.stic.req_bus->intrq, sizeof(intv.stic.req_bus->intrq), 1, f);
    fread(&intv.stic.req_bus->intrq_until, sizeof(intv.stic.req_bus->intrq_until), 1, f);
    fread(&intv.stic.req_bus->busrq_until, sizeof(intv.stic.req_bus->busrq_until), 1, f);
    fread(&intv.stic.req_bus->next_intrq, sizeof(intv.stic.req_bus->next_intrq), 1, f);
    fread(&intv.stic.req_bus->next_busrq, sizeof(intv.stic.req_bus->next_busrq), 1, f);
    fread(&intv.stic.req_bus->intak, sizeof(intv.stic.req_bus->intak), 1, f);
    fread(&intv.stic.req_bus->busak, sizeof(intv.stic.req_bus->busak), 1, f);

    fread(&intv.stic.cp_row, sizeof(intv.stic.cp_row), 1, f);
    fread(&intv.stic.ve_post, sizeof(intv.stic.ve_post), 1, f);
    fread(&intv.stic.vid_enable, sizeof(intv.stic.vid_enable), 1, f);
    fread(&intv.stic.mode, sizeof(intv.stic.mode), 1, f);

    fread(&intv.stic.p_mode, sizeof(intv.stic.p_mode), 1, f);
    fread(&intv.stic.bt_dirty, sizeof(intv.stic.bt_dirty), 1, f);
    fread(&intv.stic.gr_dirty, sizeof(intv.stic.gr_dirty), 1, f);
    fread(&intv.stic.ob_dirty, sizeof(intv.stic.ob_dirty), 1, f);
    fread(&intv.stic.rand_regs, sizeof(intv.stic.rand_regs), 1, f);

    fread(&intv.stic.pal, sizeof(intv.stic.pal), 1, f);
    fread(&intv.stic.drop_frame, sizeof(intv.stic.drop_frame), 1, f);
    fread(&intv.stic.gmem_accessible, sizeof(intv.stic.gmem_accessible), 1, f);
    fread(&intv.stic.stic_accessible, sizeof(intv.stic.stic_accessible), 1, f);
    fread(&intv.stic.next_irq, sizeof(intv.stic.next_irq), 1, f);

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //7

    fread(&intv.stic.phase, sizeof(intv.stic.phase), 1, f);
    fread(&intv.stic.next_phase, sizeof(intv.stic.next_phase), 1, f);

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //8

    for (i=0; i<0x40; i++) {
        fread(&intv.stic.raw[i], sizeof(intv.stic.raw[i]), 1, f);
    }

    for (i=0; i<0x140 * 8; i++) {
        fread(&intv.stic.gmem[i], sizeof(intv.stic.gmem[i]), 1, f);
    }

    fread(&intv.stic.fifo_ptr, sizeof(intv.stic.fifo_ptr), 1, f);

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //8a

    for (i=0; i<240; i++) {
        fread(&intv.stic.btab_sr[i], sizeof(intv.stic.btab_sr[i]), 1, f);
    }
    for (i=0; i<240; i++) {
        fread(&intv.stic.btab[i], sizeof(intv.stic.btab[i]), 1, f);
    }
    for (i=0; i<12; i++) {
        fread(&intv.stic.last_bg[i], sizeof(intv.stic.last_bg[i]), 1, f);
    }

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //8b

    fread(&intv.stic.time, sizeof(intv.stic.time), 1, f);

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //8c

    for (i=0; i<240*8; i++) {
        fread(&intv.stic.bt_bmp[i], sizeof(intv.stic.bt_bmp[i]), 1, f);
    }
    for (i=0; i<16*16  / 8; i++) {
        fread(&intv.stic.mob_img[i], sizeof(intv.stic.mob_img[i]), 1, f);
    }
    for (i=0; i<8; i++) {
        for (k=0; k<16; k++) {
            fread(&intv.stic.mob_bmp[i][k], sizeof(intv.stic.mob_bmp[i][k]), 1, f);
        }
    }
    for (i=0; i<192*224 / 8; i++) {
        fread(&intv.stic.mpl_img[i], sizeof(intv.stic.mpl_img[i]), 1, f);
    }
    for (i=0; i<192*224 /32; i++) {
        fread(&intv.stic.mpl_vsb[i], sizeof(intv.stic.mpl_vsb[i]), 1, f);
    }
    for (i=0; i<192*224 /32; i++) {
        fread(&intv.stic.mpl_pri[i], sizeof(intv.stic.mpl_pri[i]), 1, f);
    }
    for (i=0; i<192*224 / 8; i++) {
        fread(&intv.stic.xbt_img[i], sizeof(intv.stic.xbt_img[i]), 1, f);
    }
    for (i=0; i<192*224 /32; i++) {
        fread(&intv.stic.xbt_bmp[i], sizeof(intv.stic.xbt_bmp[i]), 1, f);
    }
    for (i=0; i<192*224 / 8; i++) {
        fread(&intv.stic.image[i], sizeof(intv.stic.image[i]), 1, f);
    }

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //8d

    for (i=0; i<8; i++) {
        for (k=0; k<4; k++) {
            fread(&intv.stic.gfx->bbox[i][k], sizeof(intv.stic.gfx->bbox[i][k]), 1, f);
        }
    }

    fread(&intv.stic.gfx->dirty, sizeof(intv.stic.gfx->dirty), 1, f);
    fread(&intv.stic.gfx->drop_frame, sizeof(intv.stic.gfx->drop_frame), 1, f);
    fread(&intv.stic.gfx->dropped_frames, sizeof(intv.stic.gfx->dropped_frames), 1, f);
    fread(&intv.stic.gfx->tot_frames, sizeof(intv.stic.gfx->tot_frames), 1, f);
    fread(&intv.stic.gfx->tot_dropped_frames, sizeof(intv.stic.gfx->tot_dropped_frames), 1, f);

    fread(&intv.stic.gfx->b_color, sizeof(intv.stic.gfx->b_color), 1, f);
    fread(&intv.stic.gfx->b_dirty, sizeof(intv.stic.gfx->b_dirty), 1, f);
    fread(&intv.stic.gfx->x_blank, sizeof(intv.stic.gfx->x_blank), 1, f);
    fread(&intv.stic.gfx->y_blank, sizeof(intv.stic.gfx->y_blank), 1, f);
    fread(&intv.stic.gfx->x_delay, sizeof(intv.stic.gfx->x_delay), 1, f);
    fread(&intv.stic.gfx->y_delay, sizeof(intv.stic.gfx->y_delay), 1, f);
    fread(&intv.stic.gfx->debug_blank, sizeof(intv.stic.gfx->debug_blank), 1, f);

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //9

    fread(&intv.stic.gfx->periph.addr_base, sizeof(intv.stic.gfx->periph.addr_base), 1, f);
    fread(&intv.stic.gfx->periph.addr_mask, sizeof(intv.stic.gfx->periph.addr_mask), 1, f);
    fread(&intv.stic.gfx->periph.now, sizeof(intv.stic.gfx->periph.now), 1, f);
    fread(&intv.stic.gfx->periph.min_tick, sizeof(intv.stic.gfx->periph.min_tick), 1, f);
    fread(&intv.stic.gfx->periph.max_tick, sizeof(intv.stic.gfx->periph.max_tick), 1, f);
    fread(&intv.stic.gfx->periph.next_tick, sizeof(intv.stic.gfx->periph.next_tick), 1, f);

    fread(&intv.stic.gfx->periph.next_tick, sizeof(intv.stic.gfx->periph.next_tick), 1, f);

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //9

#if 0
	for (k=0; k<32; k++) {
        fread(&intv.exec.banksw[k], sizeof(intv.exec.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.exec2.banksw[k], sizeof(intv.exec2.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.sys_ram.banksw[k], sizeof(intv.sys_ram.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.sys_ram2.banksw[k], sizeof(intv.sys_ram2.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.scr_ram.banksw[k], sizeof(intv.scr_ram.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.glt_ram.banksw[k], sizeof(intv.glt_ram.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.game0.banksw[k], sizeof(intv.game0.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.game1.banksw[k], sizeof(intv.game1.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.game2.banksw[k], sizeof(intv.game2.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.ecs0.banksw[k], sizeof(intv.ecs0.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.ecs1.banksw[k], sizeof(intv.ecs1.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.ecs2.banksw[k], sizeof(intv.ecs2.banksw[k]), 1, f);
	}
	for (k=0; k<32; k++) {
        fread(&intv.ecs_ram.banksw[k], sizeof(intv.ecs_ram.banksw[k]), 1, f);
	}
#endif

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //10

	for (k=0; k<4096 + 256; k++) {
        fread(&intv.exec_img[k], sizeof(intv.exec_img[k]), 1, f);
	}
	for (k=0; k<2048; k++) {
        fread(&intv.grom_img[k], sizeof(intv.grom_img[k]), 1, f);
	}
	for (k=0; k<4096 * 3; k++) {
        fread(&intv.ecs_img[k], sizeof(intv.ecs_img[k]), 1, f);
	}

	fread(phase, sizeof(phase)-1, 1, f);
	jzp_printf("phase read: %s\n", phase); //10


    // Get periph's notion of 'now'
    fread(&intv.intv->periph.now, sizeof(intv.intv->periph.now), 1, f);

    fclose(f);

	save_state(1);
    
    // Force display refreshes everywhere
    stic_resync(&(intv.stic));
    gfx_resync(&(intv.gfx));

    // Force time to resync
    speed_resync(&(intv.speed));

	jzp_printf("load ended\n");
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
/*                 Copyright (c) 1998-2006, Joseph Zbiciak                  */
/* ======================================================================== */
