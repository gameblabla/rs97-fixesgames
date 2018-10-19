#ifdef DREAMCAST
#include <kos.h>
extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);
#include<SDL.h>
#include<SDL_dreamcast.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include "dcastaway.h"
#include "menu.h"
#include "mem.h"
#include "disk.h"
#include "vkbd.h"

#ifdef DREAMCAST
#include<dirent.h>
extern "C" { void fs_sdcard_shutdown(void); void fs_sdcard_init(void); int fs_sdcard_unmount(void); int fs_sdcard_mount(void); void sci_init(void); }

int sdcard_exists=0;
void reinit_sdcard(void)
{
	static uint32 last=(uint32)-5000;
	uint32 now=(((unsigned long long)timer_us_gettime64())>>10);
	if (now-last>5000) {
		char *dir="/sd/dcastaway";
		DIR *d=NULL;
		fs_sdcard_shutdown();
		timer_spin_sleep(111);
		fs_sdcard_init();
		timer_spin_sleep(111);
		fs_mkdir(dir);
		d=opendir(dir);
		sdcard_exists=(d!=NULL);
		if (d)
	 		closedir(d);
		last=now;
	}
}
#endif

#if defined(DREAMCAST) && !defined(DEBUG_FAME) 
static uint32 dcastaway_dc_args[4]={ 0, 0, 0, 0};
static void dcastaway_dreamcast_handler(irq_t source, irq_context_t *context)
{
	irq_create_context(context,context->r[15], (uint32)&emergency_reset, (uint32 *)&dcastaway_dc_args[0],0);
}

static void init_dreamcast_handlers(void)
{
    irq_set_handler(EXC_USER_BREAK_PRE,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_INSTR_ADDRESS,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_ILLEGAL_INSTR,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_SLOT_ILLEGAL_INSTR,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_GENERAL_FPU,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_SLOT_FPU,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_DATA_ADDRESS_WRITE,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_DTLB_MISS_WRITE,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_000,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_100,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_400,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_600,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_FPU,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_TRAPA,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_RESET_UDI,&dcastaway_dreamcast_handler);
    irq_set_handler(EXC_UNHANDLED_EXC,&dcastaway_dreamcast_handler);
}
#else
#define init_dreamcast_handlers()
#endif


int main(int argc, char *argv[])
{
#ifdef DREAMCAST
#if defined(DEBUG_FRAMESKIP) || defined(DEBUG_SAVESTATE) || defined(DEBUG_FILEMANAGER) || defined(DEBUG_FAME)
	SDL_DC_ShowAskHz(SDL_FALSE);
	puts("Main!!!!");
#endif
#endif
#ifdef DEBUG_FAME_FILE
	extern FILE *fame_debug_file;
	fame_debug_file=fopen(DEBUG_FAME_FILE,"wb");
	if (!fame_debug_file){
		puts("NO SE PUEDE ABRIR " DEBUG_FAME_FILE);
		return 1;
	}
#endif

#ifdef DREAMCAST
#ifdef USE_DOUBLE_BUFFER
	SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
#else
	SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
#endif
#endif

#ifndef NO_MENU
	init_text(1);
#else
	video_change_to_menu();
#endif
#ifdef DREAMCAST
	while (MemInit())
		drawNoRom();
#else
	if (MemInit())
	{
		drawNoRom();
		return 1;
	}
#endif
	initDisk();
	vkbd_init();
#ifdef MACOSX
	chdir(ROM_PATH_PREFIX);
#endif
#ifdef AUTODIR
	chdir(AUTODIR);
#endif
	run_mainMenu();
	quit_text();
	init_dreamcast_handlers();
	while(1)
		dcastaway();
#if 0
	quitDisk();
    	MemQuit();
#endif
	vkbd_quit();
	return 0;
}

int run_menuGame() { return 0; }
int run_menuControl() { return 0; }

