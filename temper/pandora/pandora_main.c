#include "../common.h"
#include "pandora_screen.h"

void get_ticks_us(u64 *ticks_return)
{
  struct timeval current_time;
  gettimeofday(&current_time, NULL);

  *ticks_return =
   ((u64)current_time.tv_sec * 1000000ll) + current_time.tv_usec;
}

void delay_us(u32 delay)
{
  usleep(delay);
}

config_struct config =
{
  // u32 pad[16];
  {
    CONFIG_BUTTON_UP, CONFIG_BUTTON_DOWN, CONFIG_BUTTON_LEFT,
    CONFIG_BUTTON_RIGHT, CONFIG_BUTTON_RUN, CONFIG_BUTTON_SELECT,
    CONFIG_BUTTON_MENU, CONFIG_BUTTON_NONE, CONFIG_BUTTON_II,
    CONFIG_BUTTON_I, CONFIG_BUTTON_NONE, CONFIG_BUTTON_NONE,
    CONFIG_BUTTON_NONE, CONFIG_BUTTON_NONE,
  },
  0,                 // u32 show_fps;
  1,                 // u32 enable_sound;
  0,                 // u32 fast_forward;
  44100,             // u32 audio_output_frequency;
  1,                 // u32 patch_idle_loops;
  SS_SNAPSHOT_OFF,   // u32 snapshot_format;
  0,                 // u32 force_usa;

  500,               // u32 clock_speed;
  0,                 // u32 gp2x_ram_timings;
  100,               // u32 gp2x_gamma_percent;
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
  if(config.fast_forward == 0)
  {
    wait_for_vsync();
  }
}

void platform_initialize()
{
  
  SDL_Init(SDL_INIT_NOPARACHUTE | SDL_INIT);
}

void platform_quit()
{
  reset_screen();
}

