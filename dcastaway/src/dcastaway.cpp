#include "dcastaway.h"
#include "config.h"
#include "st.h"
#include "mem.h"
#include "sound/sound.h"
#include "vkbd.h"
#include "menu.h"
#include "m68k_intrf.h"
		

extern unsigned char fdc_motor;
extern unsigned int fdc_commands_executed;
extern int mainMenu_frameskip;
extern int mainMenu_sound;
extern int mainMenu_savedisk;
int readdsk=1;
int emulating=0;
int frameskip=0;
int maxframeskip=-1;
int mouse_mul=1;
int draw_border=0, maybe_border=0;
unsigned cyclenext=512;
unsigned vid_adr_cycleyet=0;

#ifndef USE_FAME_CORE
unsigned IO_CYCLE=0;
#endif

int dcastaway_disc_writed[2] = { 0 , 0 };
int dcastaway_disc_for_write[2] = { 0 , 0 };
Uint32 dcastaway_disc_crc[2] = { 0 , 0 };
Uint32 dcastaway_disc_actual_crc[2] = { 0 , 0 };
void *dcastaway_disc_orig[2] = { NULL, NULL };
int waitstate=0;

int nScreenRefreshRate=50;

#define AUTOFRAME_50 (1000 / 50)
#define AUTOFRAME_60 (1000 / 58)
#define AUTOFRAME_MIN (3)
Uint32 autoframeskip_next=0;

#ifdef DEBUG_FRAMESKIP

static unsigned total_frames=0;
static unsigned total_frameskip=0;
static double framerate=0.0;
static Uint32 framerate_start_time=0;
static Uint32 framerate_numframes=0;
static Uint32 wait_frames=0;

void print_frameskip(void)
{
        double p0=(((double)total_frameskip)*((double)100.0))/((double)total_frames);
        double p1=(((double)wait_frames)*((double)100.0))/((double)total_frames);
        printf("%i frames, %i skipped (%.2f%%), %i waited (%.2f%%), framerate=%.2f.\n",total_frames,total_frameskip,p0,wait_frames,p1,framerate);
}
static void calcule_framerate(void)
{
	if (!framerate_start_time)
	{
		framerate_start_time=SDL_GetTicks();
		framerate_numframes=total_frames;
	}
	else
	{
		Uint32 now=SDL_GetTicks();
		if (now-framerate_start_time>=1000)
		{
			if (framerate!=0.0)
				framerate=(framerate+((double)(total_frames-framerate_numframes)))/2.0;
			else
				framerate=(double)(total_frames-framerate_numframes);
			framerate_start_time=now;
			framerate_numframes=total_frames;
		}
	}
}
#endif

void dcastaway_disc_real_write(int num);

static void check_disc_write(void)
{
	int i;
	for(i=0;i<2;i++)
	{
		if (dcastaway_disc_writed[i])
		{
			dcastaway_disc_writed[i]=0;
			dcastaway_disc_for_write[i]=1;
		}
		else
		if (dcastaway_disc_for_write[i])
		{
			dcastaway_disc_for_write[i]++;
			if (dcastaway_disc_for_write[i]>33)
			{
				dcastaway_disc_real_write(i);
				dcastaway_disc_for_write[i]=0;
			}
		}
	}
}


#ifndef NO_RENDER
static void redraw_dummy(int a, int b)
{
}

typedef void (*redraw_func_t)(int, int);

static redraw_func_t redraw_func=redraw_dummy;
static redraw_func_t real_redraw_func=redraw_dummy;

static void change_redraw_func(redraw_func_t new_redraw_func)
{
	if (new_redraw_func!=real_redraw_func) {
		if (new_redraw_func==Redraw_med)
		{
			real_redraw_func=new_redraw_func;
			video_change_to_med();
			mouse_mul=2;
		}
		else
		if (new_redraw_func==Redraw)
		{
			real_redraw_func=new_redraw_func;
			video_change_to_low();
			mouse_mul=1;
		}
	}
	redraw_func=new_redraw_func;
}

void change_redraw_to_menu(void)
{
	if (real_redraw_func==Redraw_med)
	{
		video_change_to_menu();
		change_redraw_func(redraw_dummy);
		real_redraw_func=redraw_dummy;
	}
}
#endif

void reset_frameskip(void)
{
#ifndef NO_RENDER
    redraw_func=redraw_dummy;
#endif
    autoframeskip_next=SDL_GetTicks()+AUTOFRAME_50;
    maxframeskip=mainMenu_frameskip;
#ifdef DEBUG_FRAMESKIP
    total_frames=0;
    total_frameskip=0;
    wait_frames=0;
    framerate=0.0;
    framerate_start_time=0;
    framerate_numframes=0;
#endif
}


static int timeframe=AUTOFRAME_50;

#ifdef DREAMCAST
static __inline__ int autoframeskip(void)
{
	static int anteriorskip=0;
	Uint32 now=SDL_GetTicks();
	int ret=0;
	if (now>autoframeskip_next-AUTOFRAME_MIN)
	{
		if (now<autoframeskip_next+timeframe)
			ret=1;
		else
		if (now<autoframeskip_next+(timeframe*2))
			ret=2;
		else
		if (now<autoframeskip_next+(timeframe*3))
			ret=3;
		else
			ret=4;
		anteriorskip+=ret;
		if (anteriorskip>5)
		{
			autoframeskip_next=now;
			anteriorskip=2;
			ret=0;
		}
	}
	else
	{
		if (!anteriorskip)
		{
			Uint32 diff=autoframeskip_next-now;
			if (diff>AUTOFRAME_MIN)
				SDL_Delay(diff-AUTOFRAME_MIN);
		}
		else
			anteriorskip=0;
	}
	autoframeskip_next+=timeframe;
	return ret;
}
#else

static __inline__ int autoframeskip(void)
{
	static Uint32 lastframe=0;
	static int anteriorskip=0;
	Uint32 now=SDL_GetTicks();
	int ret=0;
	if (now>autoframeskip_next)
	{
		if (anteriorskip<0)
			anteriorskip=1;
		else
			anteriorskip++;
		if (anteriorskip>3)
		{
			autoframeskip_next=now;
			anteriorskip=0;
		}
		else
			ret=1;
	}
	else
	{
		if (anteriorskip<=0)
		{
			Uint32 diff=autoframeskip_next-now;
			if (!anteriorskip)
				lastframe=now-lastframe;
			if (diff>lastframe)
			{
#ifdef DEBUG_FRAMESKIP
				wait_frames++;
#endif
				SDL_Delay(diff-lastframe);
			}
			anteriorskip=-1;

		}
		else
		{
			lastframe=now;
			anteriorskip=0;
		}
	}
	autoframeskip_next+=timeframe;
	return ret;
}
#endif

static unsigned char vid_cycles_pal[1024];
static unsigned char vid_cycles_ntsc[1024];
unsigned char *vid_cycle=(unsigned char *)&vid_cycles_pal;

void init_vid_cycles(void)
{
	int i;

#ifdef USE_SHORT_SLICE
	double pal=136.0/512.0;
	double ntsc=136.0/427.0;
#else
#ifdef DREAMCAST
	double pal=160.0/512.0;
	double ntsc=160.0/427.0;
#else
	double pal=168.0/512.0;
	double ntsc=168.0/427.0;
#endif
#endif
	for(i=0;i<12;i++)
		vid_cycles_pal[i]=vid_cycles_ntsc[i]=0;
	for(i=12;i<512-12;i++)
		vid_cycles_pal[i]=(unsigned char)(((double)i-12)*pal);
	for(i=12;i<426-12;i++)
		vid_cycles_ntsc[i]=(unsigned char)(((double)i-12)*ntsc);
	for(i=512-12;i<1024;i++)
		vid_cycles_pal[i]=(unsigned char)(((double)512-12)*pal);
	for(i=426-12;i<1024;i++)
		vid_cycles_ntsc[i]=(unsigned char)(((double)426-12)*pal);

	vid_cycle=(unsigned char *)&vid_cycles_pal;
}

static void emulate(void)
{
    int hsync=512, hbl=0, hsync_add=512;
    int max_scanline=264,sub_scanline=64;
    unsigned long cycleco=0;
    unsigned long oldpend,newpend;
    int vsyncpend=0, hsyncpend=0;
    int delay_fdc_motor=0;

#ifdef AUTOLOAD
    extern int changed_fdc0;
    extern char *dcastaway_image_file;
    changed_fdc0=1;
    strcpy(dcastaway_image_file,AUTOLOAD);
#ifdef DREAMCAST
    fs_chdir(ROM_PATH_PREFIX);
#endif
    FDCInit(0);
#endif
#ifdef AUTOSAVESTATE
    loadstate(0);
#endif
    events_init();
    reset_frameskip();
    waitstate=0;
    init_vid_cycles();
    cyclenext=hsync;
    vid_adr_cycleyet=0;
    emulating=1;
    while(emulating)
    {
	cycleco=cpu_loop(cyclenext);
	cycleco+=waitstate;
	waitstate=0;
#ifndef NO_SOUND
	SoundCycles+=cycleco;
#endif
	//MFP timer A delay mode
	if (mfp_ascale>1) {
		mfp_acount-=mfp_ascale*cycleco;
		if (mfp_acount<=0) {
		  do {mfp_acount+=mfp_tadr;} while (mfp_acount<=0);
		  oldpend=mfp_ipra; newpend=(oldpend|0x20)&mfp_iera;
		  if (newpend!=oldpend) {mfp_ipra=newpend;
#ifndef USE_FAME_CORE
			  recalc_int=1;
#endif
		  }
		}
#ifdef USE_SHORT_SLICE
		cyclenext=4+(mfp_acount/mfp_ascale);
#endif
	}
#ifdef USE_SHORT_SLICE
	else
		cyclenext=512;
#endif
	//MFP timer B delay mode
	if (mfp_bscale>1) {
		mfp_bcount-=mfp_bscale*cycleco;
		if (mfp_bcount<=0) {
		  do {mfp_bcount+=mfp_tbdr;} while (mfp_bcount<=0);
			oldpend=mfp_ipra; newpend=(oldpend|0x1)&mfp_iera;
			if (newpend!=oldpend) {mfp_ipra=newpend;
#ifndef USE_FAME_CORE
				recalc_int=1;
#endif
			}
		}
#ifdef USE_SHORT_SLICE
		{
			int n=4+(mfp_bcount/mfp_bscale);
			if (n<cyclenext)
				cyclenext=n;
		}
#endif
	}
	//MFP timer C delay mode
	if (mfp_cscale>1) {
		mfp_ccount-=mfp_cscale*cycleco;
		if (mfp_ccount<=0) {
		  do {mfp_ccount+=mfp_tcdr;} while (mfp_ccount<=0);
			oldpend=mfp_iprb; newpend=(oldpend|0x20)&mfp_ierb;
			if (newpend!=oldpend) {mfp_iprb=newpend;
#ifndef USE_FAME_CORE
				recalc_int=1;
#endif
			}
		}
#ifdef USE_SHORT_SLICE
		{
			int n=4+(mfp_ccount/mfp_cscale);
			if (n<cyclenext)
				cyclenext=n;
		}
#endif
	}
	//MFP timer D delay mode
	if (mfp_dscale>1) {
		mfp_dcount-=mfp_dscale*cycleco;
		if (mfp_dcount<=0) {
		  do {mfp_dcount+=mfp_tddr;} while (mfp_dcount<=0);
			oldpend=mfp_iprb; newpend=(oldpend|0x10)&mfp_ierb;
			if (newpend!=oldpend) {mfp_iprb=newpend;
#ifndef USE_FAME_CORE
				recalc_int=1;
#endif
			}
		}
#ifdef USE_SHORT_SLICE
		{
			int n=4+(mfp_dcount/mfp_dscale);
			if (n<cyclenext)
				cyclenext=n;
		}
#endif
	}
	vid_adr+=(vid_cycle[cycleco]-vid_adr_cycleyet)&(~3);
	vid_adr_cycleyet=0;
	hsync-=cycleco;
	if (hsync<=0) {
		hbl++; hsync+=hsync_add;
		//Generate hbl interrupt
#ifndef USE_FAME_CORE
		if (hsyncpend==0) {hsyncpend=1; recalc_int=1;}
#else
		Interrupt(AUTOINT2, 2);
#endif
		//Do IO every 64 hbls
		if (!(hbl&63)) {
			//Generate FDC interrupt in mfp?
			if (!(mfp_gpip & 0x20)) {
				mfp_iprb |= 0x80;
				mfp_iprb &= mfp_ierb;
#ifndef USE_FAME_CORE
				recalc_int=1;
#endif
			} 
			//Generate ACIA interrupt in mfp?
			IkbdWriteBuffer();
			if (!(mfp_gpip & 0x10)) {
				mfp_iprb |= 0x40;
				mfp_iprb &= mfp_ierb;
#ifndef USE_FAME_CORE
				recalc_int=1;
#endif
			}
		}
		if (hbl<64) 
			vid_adr=(vid_baseh<<16)+(vid_basem<<8);
		else
		if (hbl<max_scanline) {
			vid_adr=(vid_baseh<<16)+(vid_basem<<8)+(hbl-63)*160;
#ifndef NO_RENDER
			(*redraw_func)(hbl-sub_scanline,vid_adr-160);
#endif
			//Timer-A event count mode
			if (mfp_tacr==0x8) //mfp_ascale==1)
			{
				mfp_acount-=1<<20; //(((hsync_add*2)-hsync)*2048); //1048576;
				if (mfp_acount<=0) {
					mfp_acount+=mfp_tadr;
					oldpend=mfp_ipra; newpend=(oldpend|0x20)&mfp_iera;
					if (newpend!=oldpend) {mfp_ipra=newpend;
#ifndef USE_FAME_CORE
					       	recalc_int=1;
#endif
					}
				}
			}
			//Timer-B event count mode
			if (mfp_tbcr==0x8) //mfp_bscale==1)
			{
				mfp_bcount-=1<<20; //(((hsync_add*2)-hsync)*2048); //1048576;
				if (mfp_bcount<=0) {
					mfp_bcount+=mfp_tbdr;
					oldpend=mfp_ipra; newpend=(oldpend|0x1)&mfp_iera;
					if (newpend!=oldpend) {mfp_ipra=newpend;
#ifndef USE_FAME_CORE
					       	recalc_int=1;
#endif
					}
				}
			}
		}
		//Vertical blank?
		else
		if (hbl>=313)
		{
			  Draw_border(draw_border);
			  do_events();
#ifdef DEBUG_FRAMESKIP
			  total_frames++;
#endif
			  if (!frameskip)
			  {
#if defined(USE_DOUBLE_BUFFER) || defined(ALWAYS_LOW)
			  	render_status();
#endif
			  }
#ifdef DEBUG_FRAMESKIP
			  else
				total_frameskip++;
			   calcule_framerate();
#endif

			frameskip--;
			Sound_Update_VBL();
    			if (fdc_commands_executed)
    			{
	    			fdc_commands_executed--;
				if (!fdc_commands_executed)
					drawDiskEmpty();
#if 0
				else
	    				drawDisk();
#endif
    			}
			if (maxframeskip<0)
				frameskip=autoframeskip();
			else
				if (frameskip<0)
					frameskip=maxframeskip;
#ifndef NO_RENDER
			if (!frameskip){
			  if (vid_shiftmode==COL2)
				  change_redraw_func(Redraw_med);
			  else
				  change_redraw_func(Redraw);
			}
			else
				change_redraw_func(redraw_dummy);
#endif

			hbl=0;
			if (mainMenu_savedisk)
				check_disc_write();
			//Generate vsync interrupt
#ifndef USE_FAME_CORE
			vsyncpend=1; recalc_int=1;
#else
			Interrupt(AUTOINT4, 4);
#endif
			//Do fdc spinup
			if (fdc_motor){
				if (delay_fdc_motor>150) {
					fdc_status &= ~0x80;
					delay_fdc_motor=0;
					fdc_motor=0;
				}
				else delay_fdc_motor++;
			}
			if (vid_syncmode & 2)
			{
				hsync_add=512;
				timeframe=AUTOFRAME_50;
				nScreenRefreshRate=50;
				vid_cycle=(unsigned char *)&vid_cycles_pal;
			}
			else
			{
				hsync_add=427;
				timeframe=AUTOFRAME_60;
				nScreenRefreshRate=60;
				vid_cycle=(unsigned char *)&vid_cycles_ntsc;
			}
			if (maybe_border>1)
			{
				if (draw_border<16)
					draw_border+=4;
			}
			else
				if (draw_border)
					draw_border--;
			if (draw_border)
			{
				max_scanline=304;
				sub_scanline=84;
			}
			else
			{
				max_scanline=264;
				sub_scanline=64;
			}
			maybe_border=0;
		}
	}
	//Recalculate interrupts?
#ifndef USE_FAME_CORE
	if (recalc_int==1)
#endif
	{
		int mfp_int;
		mfp_int=0;
		if (6>GetI()) //Mfp interrupt
		{
			int number;
			uint16 imr, ipr, isr, irr;
			int in_request;
			//Find in_request and in_service
			imr = (mfp_imra<<8)+mfp_imrb;
			ipr = (mfp_ipra<<8)+mfp_iprb;
			irr = imr & ipr;
			isr = (mfp_isra<<8) + mfp_isrb;
			//in_request higher than in_service?
			if (irr>isr) {
				//Find highest set bit
				for (in_request = 15; in_request > 0; in_request--) {
					if (irr & 0x8000) break;
					irr <<= 1;
				}
				isr = 1 << in_request;
				//Set interrupt in service bits in MFP
				if (mfp_ivr & 0x8) {
					mfp_isra |= isr >> 8;
					mfp_isrb |= isr;
				}else{
					mfp_isra &= (~isr) >> 8;
					mfp_isrb &= ~isr;
				}
				//Clear interrupt pending bits in MFP
				mfp_ipra &= ~(isr >> 8);
				mfp_iprb &= ~isr;
				//Pass interrupt to cpu
				number = in_request | (mfp_ivr & 0xf0);
				Interrupt(number, 6);
				mfp_int=1;
			}
		}
#ifndef USE_FAME_CORE
		if (!mfp_int){
			if (vsyncpend==1 && 4>GetI())
			{
				Interrupt(AUTOINT4, 4);
				vsyncpend=0;
			}else	if (hsyncpend==1 && 2>GetI())
			{
				Interrupt(AUTOINT2, 2);
				hsyncpend=0;
			}
			
		}
		if (!vsyncpend&&!hsyncpend) recalc_int=0;
#endif
	}
#ifdef USE_SHORT_SLICE
	if (hsync<cyclenext)
#endif
		cyclenext=hsync;
    }
}

void dcastaway(void)
{
    Init();
    render_init();
    events_init();
    if (mainMenu_sound)
   	audio_init();
    else
	SDL_PauseAudio(1);
    maxframeskip=mainMenu_frameskip;

    render_blank_screen();
    emulate();

    if (mainMenu_sound)
    	audio_stop();
}


void emergency_reset(void)
{
	MemReInit();
	IOInit();
  	Ikbd_Reset();
    	HWReset();
	if (mainMenu_sound)
		audio_stop();
	dcastaway();
}
