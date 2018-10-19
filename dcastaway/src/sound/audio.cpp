#ifndef NO_SOUND

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<SDL.h>

#include "config.h"
#include "st.h"
#include "mem.h"
#include "m68k_intrf.h"

#include "sound.h"

#ifdef SOUND_16BIT
#define SOUND_FORMAT AUDIO_S16
#else
#define SOUND_FORMAT AUDIO_S8
#endif



#ifdef MENU_MUSIC
#include <SDL_mixer.h>


enum{
	SAMPLE_CLICK,
	SAMPLE_FILE,
	SAMPLE_MAIN,
	SAMPLE_WAIT,
	SAMPLE_GOODBYE,
	SAMPLE_SAVE,
	SAMPLE_ERROR,
	NUM_SAMPLES
};

static char *sample_filename[NUM_SAMPLES]={
	DATA_PREFIX "click.wav",
	DATA_PREFIX "file.wav",
	DATA_PREFIX "main.wav",
	DATA_PREFIX "wait.wav",
	DATA_PREFIX "goodbye.wav",
	DATA_PREFIX "save.wav",
	DATA_PREFIX "error.wav"
};

static Mix_Chunk *sample_wave[NUM_SAMPLES];

#define play_sample(NSAMPLE) Mix_PlayChannel(0,sample_wave[(NSAMPLE)],0)
#define play_sampleS(NSAMPLE) Mix_PlayChannel(-1,sample_wave[(NSAMPLE)],0)

#endif


bool bSoundWorking = TRUE;                /* Is sound OK */
volatile bool bPlayingBuffer = FALSE;     /* Is playing buffer? */
int CompleteSndBufIdx;                    /* Replay-index into MixBuffer */


#ifdef __cplusplus
extern "C" {
static void Audio_CallBack(void *userdata, Uint8 *stream, int len);
}
#endif

/*-----------------------------------------------------------------------*/
/*
  SDL audio callback function - copy emulation sound to audio system.
*/
static void Audio_CallBack(void *userdata, Uint8 *stream, int len)
{
  int i;
#ifdef SOUND_16BIT
  Uint16 *pBuffer;
  len/=2;
#else
  Uint8 *pBuffer;
#endif

  /* If there are only some samples missing to have a complete buffer,
   * we generate them here (sounds much better then!). However, if a lot of
   * samples are missing, then the system is probably too slow, so we don't
   * generate more samples to not make things worse... */

  if(nGeneratedSamples < len)
    Sound_UpdateFromAudioCallBack();

  /* Pass completed buffer to audio system: Write samples into sound buffer
   * and convert them from 'signed' to 'unsigned' */
#ifdef SOUND_16BIT
  pBuffer = (Uint16 *)stream;
#else
  pBuffer = stream;
#endif
  for(i = 0; i < len; i++)
  {
#ifdef SOUND_16BIT
	*pBuffer++ = ((int)
#ifdef USE_BIG_ENDIAN
			MixBuffer[(CompleteSndBufIdx + i) % MIXBUFFER_SIZE]);
#else
		((char)MixBuffer[(CompleteSndBufIdx + i) % MIXBUFFER_SIZE]))*256;
#endif
#else
	*pBuffer++ = MixBuffer[(CompleteSndBufIdx + i) % MIXBUFFER_SIZE]; // ^ 128;
#endif
  }

  /* We should now have generated a complete frame of samples.
   * However, for slow systems we have to check how many generated samples
   * we may advance... */
  if(nGeneratedSamples >= len)
  {
    CompleteSndBufIdx += len;
    nGeneratedSamples -= len;
  }
  else
  {
    CompleteSndBufIdx += nGeneratedSamples;
    nGeneratedSamples = 0;
  }
  CompleteSndBufIdx = CompleteSndBufIdx % MIXBUFFER_SIZE;
}


void audio_init_music(void)
{
#ifdef MENU_MUSIC
	unsigned i;
	Mix_OpenAudio(SOUND_FREQ, SOUND_FORMAT, 1, SOUND_BUFFER_SIZE);
	for(i=0;i<NUM_SAMPLES;i++)
		sample_wave[i]=Mix_LoadWAV(sample_filename[i]);
	Mix_PlayMusic(Mix_LoadMUS(DATA_PREFIX "music.mod"),-1);
	Mix_VolumeMusic(MUSIC_VOLUME);
#endif
}

/*-----------------------------------------------------------------------*/
/*
  Initialize the audio subsystem. Return TRUE if all OK.
  We use direct access to the sound buffer, set to a unsigned 8-bit mono stream.
*/
void audio_init(void)
{
#ifndef MENU_MUSIC
  static int first=0;
  SDL_AudioSpec desiredAudioSpec;/* We fill in the desired SDL audio options here */

  /* Init the SDL's audio subsystem: */
  if( SDL_WasInit(SDL_INIT_AUDIO)==0 )
  {
    if( SDL_InitSubSystem(SDL_INIT_AUDIO)<0 )
    {
      fprintf(stderr, "Could not init audio: %s\n", SDL_GetError() );
      bSoundWorking = FALSE;
      return;
    }
  }

  /* Set up SDL audio: */
  desiredAudioSpec.freq = SOUND_FREQ;
  desiredAudioSpec.format = SOUND_FORMAT;       /* Sound Bits */
  desiredAudioSpec.channels = 1;                /* Mono */
  desiredAudioSpec.samples = SOUND_BUFFER_SIZE; /* Buffer size */
  desiredAudioSpec.callback = Audio_CallBack;
  desiredAudioSpec.userdata = NULL;

  if (!first)
  if( SDL_OpenAudio(&desiredAudioSpec, NULL) )  /* Open audio device */
  {
    fprintf(stderr, "Can't use audio: %s\n", SDL_GetError());
    bSoundWorking = FALSE;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return;
  }
  first=-1;
#else
#ifdef NO_MENU
  Mix_OpenAudio(SOUND_FREQ, SOUND_FORMAT, 1, SOUND_BUFFER_SIZE);
#endif
#endif

  Sound_Init();

#ifdef MENU_MUSIC
  SDL_PauseAudio(1);
  Mix_HookMusic(&Audio_CallBack,NULL);
#endif
  SDL_PauseAudio(0);

  /* All OK */
  bSoundWorking = TRUE;
  /* And begin */
  Audio_EnableAudio(TRUE);
}


/*
  Free audio subsystem
*/
void audio_stop(void)
{
  if (bSoundWorking)
  {
#ifndef MENU_MUSIC
    /* Stop */
    Audio_EnableAudio(FALSE);

//    SDL_CloseAudio();
#else
    SDL_PauseAudio(1);
    Mix_HookMusic(NULL,NULL);
    Mix_VolumeMusic(0); //MUSIC_VOLUME);
    SDL_PauseAudio(0);
#endif

    bSoundWorking = FALSE;
  }
}


/*-----------------------------------------------------------------------*/
/*
  Lock the audio sub system so that the callback function will not be called.
*/
void Audio_Lock(void)
{
  SDL_LockAudio();
}


/*-----------------------------------------------------------------------*/
/*
  Unlock the audio sub system so that the callback function will be called again.
*/
void Audio_Unlock(void)
{
  SDL_UnlockAudio();
}


/*-----------------------------------------------------------------------*/
/*
  Start/Stop sound buffer
*/
void Audio_EnableAudio(bool bEnable)
{
  if(bEnable && !bPlayingBuffer)
  {
    /* Start playing */
    SDL_PauseAudio(FALSE);
    bPlayingBuffer = TRUE;
  }
  else if(!bEnable && bPlayingBuffer)
  {
    /* Stop from playing */
    SDL_PauseAudio(TRUE);
    bPlayingBuffer = FALSE;
  }
}

void audio_play_click(void)
{
#ifdef MENU_MUSIC
	play_sampleS(SAMPLE_CLICK);
#endif
}

void audio_play_file(void)
{
#ifdef MENU_MUSIC
	play_sample(SAMPLE_FILE);
#endif
}

void audio_play_main(void)
{
#ifdef MENU_MUSIC
	play_sampleS(SAMPLE_MAIN);
#endif
}

void audio_play_wait(void)
{
#ifdef MENU_MUSIC
	play_sample(SAMPLE_WAIT);
#endif
}

void audio_play_goodbye(void)
{
#ifdef MENU_MUSIC
	play_sampleS(SAMPLE_GOODBYE);
#endif
}

void audio_play_save(void)
{
#ifdef MENU_MUSIC
	play_sample(SAMPLE_SAVE);
#endif
}

void audio_play_error(void)
{
#ifdef MENU_MUSIC
	play_sampleS(SAMPLE_ERROR);
#endif
}

#endif
