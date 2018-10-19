#include "../common.h"

SDL_mutex *audio_mutex;
SDL_cond *audio_cv;

void audio_callback(void *userdata, Uint8 *stream, int length)
{
  u32 sample_length = length / 2;
  u32 _length;
  s16 *stream_base = (s16 *)stream;
  s32 *source;
  s32 current_sample;

  u32 i;

  SDL_LockMutex(audio_mutex);

  while(((audio.buffer_index - audio.buffer_base) %
   AUDIO_BUFFER_SIZE) < length)
  {
    // Pump remaining cycles if you can.
    SDL_CondWait(audio_cv, audio_mutex);
  }

  if(config.enable_sound)
  {
    if((audio.buffer_base + sample_length) >= AUDIO_BUFFER_SIZE)
    {
      u32 partial_length = (AUDIO_BUFFER_SIZE - audio.buffer_base) * 2;
      sound_copy(audio.buffer_base, partial_length, normal);
      sound_copy(0, length - partial_length, normal);
      audio.buffer_base = (length - partial_length) / 2;
    }
    else
    {
      sound_copy(audio.buffer_base, length, normal);
      audio.buffer_base += sample_length;
    }
  }
  else
  {
    if((audio.buffer_base + sample_length) >= AUDIO_BUFFER_SIZE)
    {
      u32 partial_length = (AUDIO_BUFFER_SIZE - audio.buffer_base) * 2;
      audio.buffer_base = (length - partial_length) / 2;
    }
    else
    {
      audio.buffer_base += sample_length;
    }
    memset(stream, 0, length);
  }

  if(config.fast_forward == 0)
    SDL_CondSignal(audio_cv);

  SDL_UnlockMutex(audio_mutex);
}

void initialize_audio()
{
  printf("initializing audio with frequency %d\n",
   config.audio_output_frequency);

  switch(config.audio_output_frequency)
  {
    case 8000:
    case 11025:
      audio.playback_buffer_size = 512;
      break;

    case 16000:
    case 22050:
      audio.playback_buffer_size = 1024;
      break;

    case 44100:
      audio.playback_buffer_size = 2048;
      break;

    default:
      audio.playback_buffer_size = 4096;
      break;
  }

  audio.pause_state = 1;
  audio.playback_buffer_size = 4096;
  //audio.playback_buffer_size = 2048;
  //audio.playback_buffer_size = 4096;
  //audio.playback_buffer_size = 16384;

  SDL_AudioSpec desired_spec =
  {
    config.audio_output_frequency, AUDIO_S16SYS, 2, 0,
    audio.playback_buffer_size / 4, 0, 0, audio_callback, NULL
  };

  SDL_AudioSpec audio_settings;
  SDL_OpenAudio(&desired_spec, &audio_settings);
  SDL_PauseAudio(1);

  audio.output_frequency = audio_settings.freq;
  audio_mutex = SDL_CreateMutex();
  audio_cv = SDL_CreateCond();
}

void audio_exit()
{
  audio.buffer_index = AUDIO_BUFFER_SIZE - 1;
  audio.buffer_base = 0;
  SDL_CondSignal(audio_cv);
  SDL_LockMutex(audio_mutex);
  SDL_UnlockMutex(audio_mutex);
  SDL_DestroyCond(audio_cv);
  SDL_DestroyMutex(audio_mutex);
  SDL_CloseAudio();
}

// Do not do either of these two without first locking/unlocking audio
// (see functions below)

void audio_signal_callback()
{
  SDL_CondSignal(audio_cv);
}

void audio_wait_callback()
{
  if((((audio.buffer_index - audio.buffer_base) %
   AUDIO_BUFFER_SIZE) > (audio.playback_buffer_size * 3 / 2)) &&
   (config.fast_forward == 0))
  {
    while(((audio.buffer_index - audio.buffer_base) %
     AUDIO_BUFFER_SIZE) > (audio.playback_buffer_size * 3 / 2))
    {
      SDL_CondWait(audio_cv, audio_mutex);
    }
  }
}

void audio_lock()
{
  SDL_LockMutex(audio_mutex);
}

void audio_unlock()
{
  SDL_UnlockMutex(audio_mutex);
}

u32 audio_pause()
{
  u32 current_audio_pause = audio.pause_state;

  if(current_audio_pause == 0)
    SDL_PauseAudio(1);

  audio.pause_state = 1;
  return current_audio_pause;
}

void audio_unpause()
{
  if(audio.pause_state)
  {
    audio.pause_state = 0;
    SDL_PauseAudio(0);
  }
}

