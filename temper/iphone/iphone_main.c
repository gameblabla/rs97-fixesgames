#include "iphone_common.h"

#define main temper_main

static unsigned long Frames=0;
static unsigned long long Timer=0;
static unsigned long long tick=0;
static unsigned long long tickframe = 0;
static unsigned long frame_ticks_total = 0;

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
    CONFIG_BUTTON_II, CONFIG_BUTTON_MENU, CONFIG_BUTTON_VOLUME_DOWN,
    CONFIG_BUTTON_VOLUME_UP, CONFIG_BUTTON_RUN, CONFIG_BUTTON_SELECT,
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

  200,               // u32 clock_speed;
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
#if 0
  static u64 frame_wait = 0;

  frame_wait += 16667;

  //if(config.fast_forward == 0)
    sched_yield();
#else	
	const unsigned long frame_speed = 16;
	unsigned long frame_ticks;
	struct timeval current_time;
	gettimeofday(&current_time, NULL);

	Timer = (((unsigned long long)current_time.tv_sec * 1000LL) + ((unsigned long)current_time.tv_usec / 1000LL));
	
	if(tickframe == 0)
  {
    tickframe = Timer;
  }
	frame_ticks = Timer - tickframe;
	frame_ticks_total += frame_ticks;
	tickframe = Timer;
	Frames++;
	
	if(/*(Frames % 2 == 0) &&*/ frame_ticks_total < ((frame_speed)*Frames))
	{
		usleep((((frame_speed)*Frames) - frame_ticks_total) * 1000);
		//tickframe = gp2x_timer_read();
		Frames = 0;
  	gettimeofday(&current_time, NULL);
  	Timer = (((unsigned long long)current_time.tv_sec * 1000LL) + ((unsigned long)current_time.tv_usec / 1000LL));
		tick = Timer;
    tickframe = Timer;
		frame_ticks_total = 0;
	}

	if(Frames >= 600)
	{
    //char myfps[256];
    //sprintf(myfps, "%u ", Frames);
	  //print_string(myfps, 0xFFFF, 0x000, 0, 20);
		Frames = 0;
  	gettimeofday(&current_time, NULL);
  	Timer = (((unsigned long long)current_time.tv_sec * 1000LL) + ((unsigned long)current_time.tv_usec / 1000LL));
		tick = Timer;
    tickframe = Timer;
		frame_ticks_total = 0;
	}
#endif
}

void platform_initialize()
{
  Frames=0;
  Timer=0;
  tick=0;
  tickframe = 0;
  frame_ticks_total = 0;
  iphone_initialize();
}

void platform_quit()
{
  iphone_quit();
}

void set_clock_speed(int clock_speed)
{
}

void set_fast_ram_timings()
{
}

void set_default_ram_timings()
{
}

void set_gamma()
{
}


