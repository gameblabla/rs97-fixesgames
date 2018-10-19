#include	"compiler.h"
#include	"np2.h"
#include	"sysmng.h"
#include	"cpucore.h"
#include	"pccore.h"

UINT sys_updates;

#ifdef PSPDBG

// ----

static struct {
	UINT32	tick;
	UINT32	clock;
	UINT32	draws;
	SINT32	fps;
	SINT32	khz;
	UINT32 tick0;
} workclock;

void sysmng_workclockreset(void) {

	workclock.tick = GETTICK();
	workclock.clock = CPU_CLOCK;
	workclock.draws = drawcount;
        workclock.tick0 = GETTICK();
}

void sysmng_workclock_tickreset(void)
{
    workclock.tick0 = GETTICK();
}

BOOL sysmng_workclockrenewal(void) {

	SINT32	tick;

	tick = GETTICK() - workclock.tick;
        /* 0.5sec*/ 
	if (tick < 500) {
		return(FALSE);
	}
	workclock.tick += tick;
	workclock.fps = ((drawcount - workclock.draws) * 10000) / tick;
	workclock.draws = drawcount;
	workclock.khz = (CPU_CLOCK - workclock.clock) / tick;
	workclock.clock = CPU_CLOCK;
	return(TRUE);
}

void sysmng_updatecaption(void)
{
    char work[64];

    clock_buf[0] = '\0';

    if (workclock.fps) {
        OEMSPRINTF(clock_buf, " - %u.%1uFPS",
                   workclock.fps / 10, workclock.fps % 10);
    } else {
        milstr_ncpy(clock_buf, " - 0FPS", NELEMENTS(clock_buf));
    }

    OEMSPRINTF(work, OEMTEXT(" %2u.%03uMHz"),
               workclock.khz / 1000, workclock.khz % 1000);
    if (clock_buf[0] == '\0') {
        milstr_ncpy(clock_buf, " -", NELEMENTS(clock_buf));
    }
    milstr_ncat(clock_buf, work, sizeof(clock_buf));

    OEMSPRINTF(work, OEMTEXT(" %ld ticks"), GETTICK() - workclock.tick0);
    milstr_ncat(clock_buf, work, sizeof(clock_buf));
}

#endif /* PSPDBG */
