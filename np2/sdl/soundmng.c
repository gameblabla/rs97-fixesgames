#include	"compiler.h"
#include	"parts.h"
#include	"soundmng.h"
#include	"sound.h"
#include "pg.h"

#include <SDL.h>

#if !defined(DISABLE_SOUND)

#define	NSNDBUF 32

typedef struct {
    BOOL opened;
    int nsndbuf;
    int samples;
    int out_samples;
    int rate;
    SINT16 *buf[NSNDBUF];
} SOUNDMNG;

static SOUNDMNG soundmng;

static void *sound_play_cb(void)
{
    int length;
    SINT16 *dst;
    const SINT32 *src;

    length = (int)(soundmng.out_samples * 2 * sizeof(SINT16));
    dst = soundmng.buf[soundmng.nsndbuf];
    src = sound_pcmlock();
    if (src) {
        if (soundmng.rate == 44100) {
            satuation_s16(dst, src, length);
        } else if (soundmng.rate == 22050) {
            satuation_s16_22k(dst, src,length);
        } else if (soundmng.rate == 11025) {
            satuation_s16_11k(dst, src,length);
        }
        sound_pcmunlock(src);
    }
    else {
        ZeroMemory(dst, length);
    }
    soundmng.nsndbuf = (soundmng.nsndbuf + 1) % NSNDBUF;

    return dst;
}

void soundmng_cb_sdl(void *userdata,Uint8 *stream,int length)
{	
    SINT16 *dst;
    const SINT32 *src;
//	SOUNDMNG *psm = (SOUNDMNG *)userdata;

	dst = (SINT16 *)stream;
    src = sound_pcmlock();
    if (src) {
		satuation_s16(dst, src, length);
		
        sound_pcmunlock(src);
    }
    else {
        ZeroMemory(dst, length);
    }	
}

UINT soundmng_create(UINT rate, UINT ms)
{
    UINT s;
    UINT samples, out_samples;
    SINT16 *tmp;
	SDL_AudioSpec audio;

    if (soundmng.opened) {
        goto smcre_err1;
    }

    // ms = 500?
    s = rate * ms / (NSNDBUF * 1000);
    samples = 1;
    while(s > samples) {
        samples <<= 1;
    }
    // o—Í‘¤‚Í44100ŒÅ’è
    s = 44100 * ms / (NSNDBUF * 1000);
    out_samples = 1;
    while(s > out_samples) {
        out_samples <<= 1;
    }

    soundmng.nsndbuf = 0;
    soundmng.samples = samples;
    soundmng.out_samples = out_samples;
    for (s = 0; s < NSNDBUF; s++) {
        tmp = (SINT16 *)_MALLOC(out_samples * 2 * sizeof(SINT16), "buf");
        if (tmp == NULL) {
            goto smcre_err2;
        }
        soundmng.buf[s] = tmp;
        ZeroMemory(tmp, out_samples * 2 * sizeof(SINT16));
    }

//	printf("rate = %d\n",rate);
	
	audio.channels = 2;
	audio.freq = rate;
	audio.format = AUDIO_S16;
	audio.samples = samples;
	audio.callback = soundmng_cb_sdl;
	audio.userdata = &soundmng;
	
	SDL_OpenAudio(&audio,NULL);
//    pgaSetChannelCallback(sound_play_cb);
//    pgaInit(out_samples);

    soundmng.rate = rate;
    soundmng.opened = TRUE;

    return(samples);

 smcre_err2:
    for (s = 0; s < NSNDBUF; s++) {
        tmp = soundmng.buf[s];
        soundmng.buf[s] = NULL;
        if (tmp) {
            _MFREE(tmp);
        }
    }

 smcre_err1:
    return(0);
}

void soundmng_destroy(void) {

	int		i;
	SINT16	*tmp;

	if (soundmng.opened) {
		soundmng.opened = FALSE;
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		for (i=0; i<NSNDBUF; i++) {
			tmp = soundmng.buf[i];
			soundmng.buf[i] = NULL;
			_MFREE(tmp);
		}
	}
}

void soundmng_play(void) {

    pgaPause(0);
	if (soundmng.opened) {
		SDL_PauseAudio(0);
	}
}

void soundmng_stop(void) {

    pgaPause(1);
	if (soundmng.opened) {
		SDL_PauseAudio(1);
	}
}

#endif

