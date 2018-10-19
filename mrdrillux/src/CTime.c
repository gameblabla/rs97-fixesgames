#include "CTime.h"

void CTimeWait(CTime *p) {
	Uint32 lefttime;

	p->nowtime=SDL_GetTicks();
	p->under+=p->interval;
	lefttime=p->lasttime+(p->under>>16)-p->nowtime;
	if(p->lasttime+(p->under>>16)>p->nowtime){
		SDL_Delay(lefttime);
		p->isDelay=0;
		p->framecount++;
	}else{
		p->isDelay=1;
	}
	p->lasttime+=(p->under>>16);
	p->clock+=(p->under>>16);
	p->fpsclock+=(p->under>>16);
	if(p->fpsclock>1000){
		p->fps=p->framecount;
		p->framecount=0;
		p->fpsclock-=1000;
	}
	p->under &= 0x0ffff;
}

void CTimeReset(CTime *p) {
	p->nowtime=SDL_GetTicks();
	p->lasttime=p->nowtime;
	p->under=0;
	p->clock=0;
	p->isDelay=0;

	p->fps=0;
	p->fpsclock=0;
	p->framecount=0;
}

void CTimeChangeFPS(CTime *p, int fpsmax) {
	p->interval = (65536 * 1000) / fpsmax;
}
