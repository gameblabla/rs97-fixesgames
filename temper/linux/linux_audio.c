#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "../common.h"

u32 audio_dev;
u32 audio_paused = 1;
u32 audio_exit_thread = 0;

pthread_cond_t audio_cv;
pthread_mutex_t audio_mutex;
pthread_t audio_thread;

#define DSP_AUDIO_BUFFER_BITS 10
#define DSP_AUDIO_BUFFER_SIZE (1 << DSP_AUDIO_BUFFER_BITS)

// Do not do either of these two without first locking/unlocking audio
// (see functions below)

void audio_callback(s16 *stream, int length)
{
  u32 sample_length = length / 2;
  u32 _length;
  s16 *stream_base = (s16 *)stream;
  s32 *source;
  s32 current_sample;

  u32 i;

  u64 ticks;
  get_ticks_us(&ticks);

  pthread_mutex_lock(&audio_mutex);

  while(((audio.buffer_index - audio.buffer_base) %
   AUDIO_BUFFER_SIZE) < sample_length)
  {
    // Pump remaining cycles if you can.
    pthread_cond_wait(&audio_cv, &audio_mutex);
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
    pthread_cond_signal(&audio_cv);

  get_ticks_us(&ticks);

  pthread_mutex_unlock(&audio_mutex);
}

void *audio_update_thread(void *thread_data)
{
  u32 length = DSP_AUDIO_BUFFER_SIZE * 4;
  s16 stream[DSP_AUDIO_BUFFER_SIZE * 2];

  //struct sched_param sched_param = { 99 };
  //pthread_setschedparam(audio_thread, SCHED_RR, &sched_param);

  while(!audio_exit_thread)
  {
    if(!audio_paused)
      audio_callback(stream, length);
    else
      memset(stream, 0, length);

    write(audio_dev, stream, length);
    sched_yield();
  }

  return NULL;
}

void initialize_audio()
{
  u32 channels = 2;
  u32 format = AFMT_S16_LE;
  // Use 4 1024 b  sample fragments.
  u32 fragment_setting = (4 << 16) | DSP_AUDIO_BUFFER_BITS;
  u32 flags;

  audio.output_frequency = 44100;
  audio.playback_buffer_size = DSP_AUDIO_BUFFER_SIZE * 4;

  audio_dev = open("/dev/dsp", O_WRONLY);

  flags = fcntl(audio_dev, F_GETFL);
  flags &= ~O_NONBLOCK;
  fcntl(audio_dev, F_SETFL, flags);

  ioctl(audio_dev, SNDCTL_DSP_SETFMT, &format);
  ioctl(audio_dev, SNDCTL_DSP_CHANNELS, &channels);
  ioctl(audio_dev, SNDCTL_DSP_SPEED, &audio.output_frequency);

  printf("set fragment ioctl: %d\n",
   ioctl(audio_dev, SNDCTL_DSP_SETFRAGMENT, &fragment_setting));

  printf("Got frequency %d\n", audio.output_frequency);
  printf("Got %d channels\n", channels);
  printf("Got format %d\n", format);
  printf("Got fragment %x\n", fragment_setting);

  pthread_cond_init(&audio_cv, NULL);
  pthread_mutex_init(&audio_mutex, NULL);
  pthread_create(&audio_thread, NULL, audio_update_thread, NULL);
}

// Do not do either of these two without first locking/unlocking audio
// (see functions below)

void audio_signal_callback()
{
  pthread_cond_signal(&audio_cv);
}

void audio_wait_callback()
{
  if((((audio.buffer_index - audio.buffer_base) %
   AUDIO_BUFFER_SIZE) > (audio.playback_buffer_size * 3 / 4)) &&
   (config.fast_forward == 0))
  {
    while(((audio.buffer_index - audio.buffer_base) %
     AUDIO_BUFFER_SIZE) > (audio.playback_buffer_size * 3 / 4))
    {
      pthread_cond_wait(&audio_cv, &audio_mutex);
    }
  }
}


void audio_lock()
{
  pthread_mutex_lock(&audio_mutex);
}

void audio_unlock()
{
  pthread_mutex_unlock(&audio_mutex);
}

u32 audio_pause()
{
  u32 current_audio_pause = audio.pause_state;

  audio.pause_state = 1;
  return current_audio_pause;
}

void audio_unpause()
{
  audio_paused = 0;
}

void audio_exit()
{
  audio.buffer_index = AUDIO_BUFFER_SIZE - 1;
  audio.buffer_base = 0;
  audio_exit_thread = 1;
  pthread_cond_signal(&audio_cv);
  pthread_join(audio_thread, NULL);
  close(audio_dev);
}


