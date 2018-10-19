// primitive graphics for Hello World sce

#include "pg.h"

#include "font.c"

#define pg_vramtop ((char *)0x04000000)
long pg_screenmode;
long pg_showframe;
static unsigned int pg_draw_buffer;
long pg_drawframe;

void pgSetDrawBuffer(void *frame_buf)
{
}

char *pgGetVramAddr(unsigned long x, unsigned long y)
{
	return 0;
}

void pgScreenFrame(long mode, long frame)
{
}

void pgScreenFlip()
{
}

/*****************************************************************************/
int pga_ready = 0;
int pga_pause = 1;
int pga_handle;

void *(*pga_callback)(void);
int pga_threadhandle;
volatile int pga_terminate = 0;


static int pgaOutBlocking(unsigned long vol1, unsigned long vol2, void *buf)
{
}

static int pga_channel_thread(int args, void *argp)
{
}

void pgaSetChannelCallback(void *callback)
{
}

int pgaInit(int samples)
{
	return 0;
}

void pgaTermPre()
{
    pga_ready = 0;
    pga_terminate = 1;
}

void pgaTerm()
{
}

void pgaPause(int n)
{
}

void pgGeInit()
{
}
