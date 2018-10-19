#include <stdlib.h>
#include <SDL.h>

#include "def.h"
#include "device.h"
#include "hardware.h"

void fill_audio(void *udata, Uint8 *stream, int len);
samp getsample(void);

Uint4 buf[1024];
Uint4 bsize = 0;

bool wave_device_available = FALSE;

bool initsounddevice(void)
{
	return(TRUE);
}

bool setsounddevice(int base, int irq, int dma, Uint4 samprate, Uint4 bufsize)
{
	if(bufsize*2 > sizeof(buf))
		return FALSE;
	SDL_AudioSpec wanted;
	wanted.freq = samprate;
	wanted.samples = bufsize;
	wanted.channels = 1;
	wanted.format = AUDIO_S16SYS;
	wanted.userdata = NULL;
	wanted.callback = fill_audio;

#ifdef _VGL
	restorekeyb();
#endif

	bool result = FALSE;
	if ((SDL_InitSubSystem(SDL_INIT_AUDIO)) >= 0)
		if ((SDL_OpenAudio(&wanted, NULL)) >= 0)
			result = TRUE;
	if (result == FALSE)
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
	else {
//		buf = malloc(bufsize*2);
		bsize = bufsize*2;
		wave_device_available = TRUE;
	}

#ifdef _VGL
	initkeyb();
#endif

	return(result);
}

void fill_audio(void *udata, Uint8 *stream, int len)
{
	if(!buf)
		return;
	int i;

	if (len > bsize)
		len = bsize;
	int l;
	l = len/2;
	Uint4* b = buf;
	for(i = 0; i < l; i++)
	{
		Uint4 s = getsample();
		s *= 8;
		*b++ = s;
	}
	SDL_MixAudio(stream, (void*)buf, len, SDL_MIX_MAXVOLUME);
}


void killsounddevice(void)
{
	SDL_PauseAudio(1);
}

