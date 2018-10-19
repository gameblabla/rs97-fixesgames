#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <SDL_mixer.h>

#define AUDIO_SAMPLE_RATE	44100
#define AUDIO_CHUNK_SIZE	1024

extern int hasAudio;
extern int enableMusic;
extern int enableSfx;
extern Mix_Music *bgdMusic;
extern Mix_Chunk *clearSfx;

int initAudio();
void deinitAudio();
Mix_Music *loadMusic(Mix_Music *track, char *fileName);
void unloadMusic(Mix_Music **track);
void playMusic(Mix_Music *track);
void resumeMusic();
void pauseMusic();
Mix_Chunk *loadSfx(Mix_Chunk *effect, char *fileName);
void unloadSfx(Mix_Chunk **effect);
void playSfx(Mix_Chunk *effect);

#endif /* _AUDIO_H_ */
