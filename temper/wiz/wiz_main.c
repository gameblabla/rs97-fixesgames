#include "wiz_common.h"

void get_ticks_us(u64 *ticks_return)
{
  struct timeval current_time;
  gettimeofday(&current_time, NULL);

  *ticks_return =
   ((u64)current_time.tv_sec * 1000000ll) + current_time.tv_usec;
}

void delay_us(u32 us_count)
{
  u64 start;
  u64 current;

  get_ticks_us(&start);

  do
  {
    get_ticks_us(&current);
  } while((s64)(current - start) < us_count);
}

void delay_until(u64 us_point)
{
  u64 current;
  return;

  get_ticks_us(&current);
  printf("delay %d until %d\n", (u32)current, (u32)us_point);

  do
  {
    get_ticks_us(&current);
  } while(current < us_point);
}

config_struct config =
{
  // u32 pad[16];
  {
    CONFIG_BUTTON_UP, CONFIG_BUTTON_DOWN, CONFIG_BUTTON_LEFT,
    CONFIG_BUTTON_RIGHT, CONFIG_BUTTON_NONE, CONFIG_BUTTON_I,
    CONFIG_BUTTON_II, CONFIG_BUTTON_MENU, CONFIG_BUTTON_LOAD_STATE,
    CONFIG_BUTTON_SAVE_STATE, CONFIG_BUTTON_SELECT, CONFIG_BUTTON_RUN,
    CONFIG_BUTTON_VOLUME_DOWN, CONFIG_BUTTON_VOLUME_UP, CONFIG_BUTTON_MENU,
    CONFIG_BUTTON_NONE
  },
  0,                 // u32 show_fps;
  1,                 // u32 enable_sound;
  0,                 // u32 fast_forward;
  44100,             // u32 audio_output_frequency;
  1,                 // u32 patch_idle_loops;
  SS_SNAPSHOT_OFF,   // u32 snapshot_format;
  0,                 // u32 force_usa;

  533,               // u32 clock_speed;
  0,                 // u32 ram_timings;
  100,               // u32 gamma_percent;
  0,                 // u32 six_button_pad;
  CD_SYSTEM_TYPE_V3, // cd_system_type_enum cd_system_type;
  1,                 // u32 bz2_savestates;
  0,                 // u32 per_game_bram;
  0,                 // u32 sound_double;
  0,                 // u32 scale_factor;
  0,                 // u32 fast_cd_access;
  0,                 // u32 fast_cd_load;
  0,                 // u32 scale_width;
  0,                 // u32 unlimit_sprites
  0,                 // u32 compatibility_mode
};

void synchronize()
{
  static u64 frame_wait = 0;

  frame_wait += 16667;

  if(config.fast_forward == 0)
    sched_yield();
}

void platform_initialize()
{
  wiz_initialize();
}

void platform_quit()
{
  usleep(1000000);
  wiz_quit();
}

void set_clock_speed(u32 clock_speed)
{
  printf("Setting clock speed to %d\n", clock_speed);
  wiz_set_clock_speed(clock_speed);
}

