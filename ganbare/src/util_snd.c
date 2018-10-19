
#include <SDL/SDL.h>
#ifndef NOSOUND
#include <SDL/SDL_mixer.h>
#endif
#include "include/general.h"
#include "include/util_snd.h"
#include "define.h"

#ifdef DREAMCAST
#include <kos.h>
#endif

#ifdef GP2X
enum{
	SND_RATE = 44100, 
	SND_CHANNEL = 2, 
	SND_BUFFER = 128
};
#else
enum{
	SND_RATE = 44100, 
	SND_CHANNEL = 2, 
	SND_BUFFER = 1024
};
#endif

void soundInitBuffer(void);
void soundRelease(void);
void soundLoadBuffer(int num, UINT8 *fname, int loop);
void soundLoadBuffer2(int num, UINT8 *fname1, UINT8 *fname2);
void soundLoadBufferSE(int num, UINT8 *fname);
void soundStopBgm(int num);
void soundStopBgmPlaying(void);
int soundIsPlayBgm(void);
void soundPlayBgm(int num);
void soundPlayFadeFlag(int flag, int time);
void soundPlayCtrl(void);
void soundSetVolumeMaster(int vol);
void soundSetVolumeBgm(int vol, int num);
void soundSetVolumeAll(int vol);
void soundStopSe(int num);
void soundPlaySe(int num);
int soundIsPlaySe(int num);
void soundStopSeAll(void);

#ifdef NOSOUND
	typedef int Mix_Music;
	typedef int Mix_Chunk;
	static int master_vol;
	static int play_bgmvol;
#else

static int play_bgmvol;
static int play_sevol;
static int fade_ctrl;
static int fade_vol;
static int fade_time;
static int fade_rate;
static int master_vol;

static unsigned char sound_use = FALSE;
static Mix_Chunk *chunk[SOUND_CHUNKBANK];
static int chunkChannel[SOUND_MIXBANK];
static Mix_Music *music[SOUND_MUSBANK];
static Mix_Music *music_loop[SOUND_MUSBANK];
static int musicIntor[SOUND_MUSBANK];
static int musicLoop[SOUND_MUSBANK];
static int musicPlayNum;
static int musicIsLoop;

#endif

void soundInitBuffer(void)
{
#ifndef NOSOUND
	int i;
    int audio_rate;
    UINT16 audio_format;
    int audio_channels;
    int audio_buffers;

	master_vol = 100;
	play_bgmvol = MIX_MAX_VOLUME;
	play_sevol = MIX_MAX_VOLUME;
	soundSetVolumeAll(play_bgmvol);

	if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0){
		return;
    }

	audio_rate = SND_RATE;
	audio_format = AUDIO_S16LSB;
	audio_channels = SND_CHANNEL;
	audio_buffers = SND_BUFFER;
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0){
		sound_use = FALSE;
	}else{
		sound_use = TRUE;
	}
	Mix_Init(MIX_INIT_OGG);
	//Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

	fade_vol = 0;
	fade_ctrl = 0;
	musicPlayNum = -1;
	musicIsLoop = 0;

	for(i = 0; i < SOUND_MUSBANK; i++){
		music[i] = NULL;
		music_loop[i] = NULL;
		musicIntor[i] = 0;
		musicLoop[i] = 0;
	}

	for(i = 0; i < SOUND_CHUNKBANK; i++){
		chunk[i] = NULL;
	}

	for(i = 0; i < SOUND_MIXBANK; i++){
		chunkChannel[i] = -1;
	}
#endif
}

void soundRelease(void)
{
#ifndef NOSOUND
	int i;

	for(i = 0; i < SOUND_MUSBANK; i++){
		if(music[i]) Mix_FreeMusic(music[i]);
		if(music_loop[i]) Mix_FreeMusic(music_loop[i]);
		music[i] = NULL;
		music_loop[i] = NULL;
	}

	for(i = 0; i < SOUND_CHUNKBANK; i++){
		if(chunk[i]) Mix_FreeChunk(chunk[i]);
		chunk[i] = NULL;
	}

	Mix_CloseAudio();
#endif
}

void soundLoadBuffer(int num, UINT8 *fname, int loop)
{
#ifndef NOSOUND
	if(!music[num]){
		music[num] = Mix_LoadMUS((char *)fname);
		musicIntor[num] = 0;
		musicLoop[num] = loop;
	}
#endif
}

void soundLoadBuffer2(int num, UINT8 *fname1, UINT8 *fname2)
{
#ifndef NOSOUND
	if(!music[num]){
		music[num] = Mix_LoadMUS((char *)fname1);
		musicIntor[num] = 1;
	}
	if(!music_loop[num]){
		music_loop[num] = Mix_LoadMUS((char *)fname2);
	}
#endif
}

void soundLoadBufferSE(int num, UINT8 *fname)
{
#ifndef NOSOUND
	chunk[num] = Mix_LoadWAV((char *)fname);
	if(!chunk[num]){
		sound_use = FALSE;
	}
#endif
}

void soundStopBgm(int num)
{
#ifndef NOSOUND
	if(!sound_use){
		return;
	}

	if(music[num]){
		fade_vol = 0;
		fade_ctrl = 0;
		musicPlayNum = -1;
		musicIsLoop = 0;
		Mix_HaltMusic();
	}
#endif
}

void soundStopBgmPlaying(void)
{
#ifndef NOSOUND
	if(!sound_use){
		return;
	}

	if(musicPlayNum != -1){
		fade_vol = 0;
		fade_ctrl = 0;
		musicPlayNum = -1;
		musicIsLoop = 0;
		Mix_HaltMusic();
	}
#endif
}

int soundIsPlayBgm(void)
{
#ifndef NOSOUND
	if(!sound_use){
		return 0;
	}

	if(Mix_PlayingMusic() == SDL_TRUE) return 1;
#endif
	return	0;
}

void soundPlayBgm(int num)
{
#ifndef NOSOUND
	if(num < 0 || !music[num]){
		return;
	}

	if(!sound_use){
		return;
	}

	fade_ctrl = 0;
    musicPlayNum = num;
	if(musicIntor[num] == 0){
		Mix_PlayMusic(music[num], musicLoop[num]);
		musicIsLoop = 0;
	}else{
		Mix_PlayMusic(music[num],  1);
		musicIsLoop = 2;
	}
	Mix_VolumeMusic(play_bgmvol * master_vol / 100);
#endif
}

void soundPlayBgm2(int num)
{
#ifndef NOSOUND
	if(num < 0 || !music_loop[num]){
		return;
	}

	if(!sound_use){
		return;
	}

    Mix_PlayMusic(music_loop[num], -1);
#endif
}

void soundLoadPlayBgm(UINT8 *fname, int loop)
{
//	soundStopBgmPlaying();
//	if(music[BGM_TEST]){
//		Mix_FreeMusic(music[BGM_TEST]);
//		music[BGM_TEST] = NULL;
//	}
//	soundLoadBuffer(BGM_TEST, fname, -1);
//	soundPlayBgm(BGM_TEST);
}

void soundPlayFadeFlag(int flag, int time)
{
#ifndef NOSOUND
	fade_ctrl = flag;
	fade_time = time;
#endif
}

void soundPlayCtrl(void)
{
#ifndef NOSOUND
	int	i;

	for(i = 0; i < SOUND_MIXBANK; i++){
		if(chunkChannel[i] != -1){
		    if(Mix_Playing(i) == 0) chunkChannel[i] = -1;
		}
	}

	if(musicPlayNum == -1){
		return;
	}

	if(musicIsLoop){
		if(musicIsLoop == 2){
			if(Mix_PlayingMusic() == SDL_TRUE) musicIsLoop--;
		}else{
			if(Mix_PlayingMusic() == SDL_FALSE){
				musicIsLoop = 0;
				soundPlayBgm2(musicPlayNum);
			}
		}
	}

	if(!fade_ctrl){
		return;
	}

	if(fade_ctrl == 1){
		fade_ctrl = 2;
		fade_vol = play_bgmvol;
		fade_rate = fade_vol / fade_time;
	}

	if(fade_ctrl == 2){
		fade_time--;
		if(fade_time){
			fade_vol -= fade_rate;
			Mix_VolumeMusic(fade_vol * master_vol / 100);
		}else{
			fade_vol = 0;
			fade_ctrl = 0;
			musicPlayNum = -1;
			musicIsLoop = 0;
			Mix_VolumeMusic(fade_vol);
		}
	}
#endif
}

void soundSetVolumeMaster(int vol)
{
	master_vol = vol;
}

void soundSetVolumeBgm(int vol, int num)
{
	play_bgmvol = vol;
}

void soundSetVolumeAll(int vol)
{
#ifndef NOSOUND
	int	i;

	vol = vol * master_vol / 100;
	play_sevol = vol;
	
	for(i = 0; i < SOUND_CHUNKBANK; i++){
		if(chunk[i]){
			Mix_VolumeChunk(chunk[i], vol);
		}
	}

	Mix_VolumeMusic(vol);
#endif
}

void soundStopSe(int num)
{
#ifndef NOSOUND
	int	i;

	for(i = 0; i < SOUND_MIXBANK; i++){
		if(chunkChannel[i] == num){
			Mix_HaltChannel(i);
			chunkChannel[i] = -1;
		}
	}
#endif
}

void soundPlaySe(int num)
{
#ifndef NOSOUND
	int	i;
	int vol = play_sevol * master_vol / 100;

	if(!sound_use){
		return;
	}

	soundStopSe(num);

	for(i = 0; i < SOUND_MIXBANK; i++){
		if(chunkChannel[i] == -1){
		    Mix_PlayChannel(i, chunk[num], 0);
			Mix_VolumeChunk(chunk[i], vol);
			chunkChannel[i] = num;
			return;
		}
	}
#endif
}

int soundIsPlaySe(int num)
{
#ifndef NOSOUND
	int	i;

	for(i = 0; i < SOUND_MIXBANK; i++){
		if(chunkChannel[i] == num){
			if(Mix_Playing(i) != 0) return 1;
		}
	}
#endif
	return 0;
}

void soundStopSeAll(void)
{
#ifndef NOSOUND
	int	i;

	for(i = 0; i < SOUND_MIXBANK; i++){
		chunkChannel[i] = -1;
	}
	Mix_HaltChannel(-1);
#endif
}
