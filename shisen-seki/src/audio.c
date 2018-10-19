#include "audio.h"

#include <SDL_mixer.h>

int hasAudio;
int enableMusic = 1;
int enableSfx = 1;
Mix_Music *bgdMusic = NULL;
Mix_Chunk *clearSfx = NULL;

int initAudio()
{
	int flags = MIX_INIT_OGG;

	if ((Mix_Init(flags)&flags) != flags)
	{
		fprintf(stderr, "ERROR: Failed to initialize the audio engine: No OGG support detected.");
		return -1;
	}

	if (Mix_OpenAudio(AUDIO_SAMPLE_RATE, MIX_DEFAULT_FORMAT, 2, AUDIO_CHUNK_SIZE))
	{
		fprintf(stderr, "ERROR: Failed to initialize the audio engine.");
		return -1;
	}

	return 0;
}

void deinitAudio()
{
	if (!hasAudio)
	{
		return;
	}

	unloadMusic(&bgdMusic);

	Mix_CloseAudio();
	Mix_Quit();
}

Mix_Music *loadMusic(Mix_Music *track, char *fileName)
{
	if (!hasAudio)
	{
		return NULL;
	}

	track = Mix_LoadMUS(fileName);

	return track;
}

void unloadMusic(Mix_Music **track)
{
	if (!hasAudio)
	{
		return;
	}

	Mix_FreeMusic(*track);
	*track = NULL;
}

void playMusic(Mix_Music *track)
{
	if (!hasAudio || !enableMusic)
	{
		return;
	}

	if (!Mix_PlayingMusic())
	{
		Mix_PlayMusic(track, -1);
	}
}

void resumeMusic()
{
	if (!hasAudio)
	{
		return;
	}

	if (Mix_PausedMusic())
	{
		Mix_ResumeMusic();
	}
	else if (!Mix_PlayingMusic())
	{
		if(bgdMusic)
		{
			playMusic(bgdMusic);
		}
	}
}

void pauseMusic()
{
	if (!hasAudio)
	{
		return;
	}

	if (Mix_PlayingMusic())
	{
		Mix_PauseMusic();
	}
}

Mix_Chunk *loadSfx(Mix_Chunk *effect, char *fileName)
{
	if (!hasAudio)
	{
		return NULL;
	}

	effect = Mix_LoadWAV(fileName);

	return effect;
}

void unloadSfx(Mix_Chunk **effect)
{
	if (!hasAudio)
	{
		return;
	}

	Mix_FreeChunk(*effect);
	*effect = NULL;
}

void playSfx(Mix_Chunk *effect)
{
	if (!hasAudio || !enableSfx)
	{
		return;
	}

	Mix_PlayChannel(-1, effect, 0);
}
