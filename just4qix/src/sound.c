#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "misc.h"

#define numMusicTracks  10
#define numSoundEffects 7

enum musicTrack { menu, game1, game2, game3, game4, gameOver, game5, game6, game7, game8 };
char* musicFiles[numMusicTracks] = { "sound/alf_-_no-mercy.mod", "sound/beyond_music.mod", "sound/spacedeb.mod", "sound/enigma.mod", "sound/stardstm.mod", "sound/elysium.mod","sound/2unlimit.mod","sound/cream_of_the_earth.mod","sound/aurora.mod","sound/annamull.mod"};

Mix_Music* Music;  //only ever one music track playing at a time, load them as we need them

enum soundEffect { blip, fill, pops, whirly, levelcomplete, gameover, playerdeath };
char* soundFiles[numSoundEffects] = { "sound/blip1.wav","sound/fill.wav","sound/pop.wav","sound/whirly.wav","sound/excellnt.wav","sound/evil_laugh.wav","sound/playerdeath.wav" };
Mix_Chunk* soundEffects[numSoundEffects];



bool InitSound(){
  //initialize audio
  int audio_rate = 22050;
  Uint16 audio_format = AUDIO_S16SYS; /* 16-bit stereo */
  int audio_channels = 2;
  int audio_buffers = 1024;

  SDL_Init(SDL_INIT_AUDIO);	
	
   if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
    printf("Unable to open audio!\n");
    exit(1);
  }
  //load sound effects
  int i;
  for (i=0; i<numSoundEffects; i++){
	soundEffects[i] = Mix_LoadWAV(soundFiles[i]);
  }
  return true; //todo: test if audio worked
}

/*
bool IsMusicPlaying(){
	return !(currentMusic);
}
*/

void CleanUpSound(){
	 
	Mix_CloseAudio();
	int i;
	for (i=0; i<numSoundEffects; i++){
		Mix_FreeChunk(soundEffects[i]);
	}	
	
}

void StopMusic() {
  Mix_HaltMusic();
  Mix_FreeMusic(Music);
  Music = NULL;
}

void PlayIntroMusic() {
	if (( Music = Mix_LoadMUS(musicFiles[menu]) ) == NULL ) {
		printf("Couldn't load intro music\n");
		printf("%s",Mix_GetError());
	};
	Mix_PlayMusic(Music, -1); //loop continously 
}

void PlayGameMusic() {
	int select = rand() % 8;
	switch (select) {
	case 0:
		Music = Mix_LoadMUS(musicFiles[game1]);
		break;
	case 1:
		Music = Mix_LoadMUS(musicFiles[game2]);
		break;
	case 2:
		Music = Mix_LoadMUS(musicFiles[game3]);
		break;
	case 3:
		Music = Mix_LoadMUS(musicFiles[game4]);
		break;
	case 4:
		Music = Mix_LoadMUS(musicFiles[game5]);
		break;
	case 5:
		Music = Mix_LoadMUS(musicFiles[game6]);
		break;
	case 6:
		Music = Mix_LoadMUS(musicFiles[game7]);
		break;
	case 7:
		Music = Mix_LoadMUS(musicFiles[game8]);
		break;
	}
	Mix_PlayMusic(Music, -1); //loop continously 
}

void PlayGameOverMusic() {
	Music = Mix_LoadMUS(musicFiles[gameOver]) ;
	Mix_PlayMusic(Music, -1); //loop continously 
}

void playPop(){
	int channel = Mix_PlayChannel(-1,  soundEffects[pops], 0);
}
void playWhirly(){
	int channel = Mix_PlayChannel(-1, soundEffects[whirly], 0);
}


void playBlip(){
	int channel = Mix_PlayChannel(-1, soundEffects[blip], 0);
}

void playFill(){
	int channel = Mix_PlayChannel(-1, soundEffects[fill], 0);
}


void playLevelComplete(){
	int channel = Mix_PlayChannel(-1, soundEffects[levelcomplete], 0);
}
void playGameOver(){
	int channel = Mix_PlayChannel(-1, soundEffects[gameover], 0);
}

void playPlayerDeath(){
	int channel = Mix_PlayChannel(-1, soundEffects[playerdeath], 0);
}