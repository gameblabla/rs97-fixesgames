/// \file
/// \brief Handles emulation speed throttling using the DINGOO timing functions.

#include <sys/time.h>
#include <SDL/SDL.h>

#include "dingoo.h"
#include "throttle.h"

static const double Slowest = 0.015625; // 1/64x speed (around 1 fps on NTSC)
static const double Fastest = 32;       // 32x speed   (around 1920 fps on NTSC)
static const double Normal  = 1.0;      // 1x speed    (around 60 fps on NTSC)

uint64 Lasttime, Nexttime;
static long double desired_frametime;
static uint64 desired_frametime_us;
static int InFrame;
double g_fpsScale = Normal; // used by dingoo.cpp
bool MaxSpeed = false;

uint64 get_ticks_us()
{
  struct timeval current_time;
  gettimeofday(&current_time, NULL);

  return (uint64)current_time.tv_sec * 1000000 + current_time.tv_usec;
}

void delay_us(uint64 us_count)
{
#if 1 // busy wait
    uint64 start;
    uint64 current;

    start = get_ticks_us();

    do {
        current = get_ticks_us();
    } while((uint64)(current - start) < us_count);
#else // normal wait, not reliable enough
    usleep(us_count);
#endif
}

/* LOGMUL = exp(log(2) / 3)
 *
 * This gives us a value such that if we do x*=LOGMUL three times,
 * then after that, x is twice the value it was before.
 *
 * This gives us three speed steps per order of magnitude.
 *
 */
#define LOGMUL 1.259921049894873

/**
 * Refreshes the FPS throttling variables.
 */
void RefreshThrottleFPS()
{
    uint64 fps = FCEUI_GetDesiredFPS(); // Do >> 24 to get in Hz
    desired_frametime = 16777216.0l / (fps * g_fpsScale);
    desired_frametime_us = (uint64)(desired_frametime * 1000000.0l);

    Lasttime=0;   
    Nexttime=0;
    InFrame=0;
    printf("desired_frametime: %i\n", desired_frametime_us);
}

/**
 * Perform FPS speed throttling by delaying until the next time slot.
 */
int SpeedThrottle()

{
    //puts("SpeedThrottle");
    if(g_fpsScale >= 32)
    {
        return 0; /* Done waiting */
    }
    uint64 time_left;
    uint64 cur_time;
    
    if(!Lasttime) Lasttime = get_ticks_us();
    
    if(!InFrame) {
        InFrame = 1;
        Nexttime = Lasttime + desired_frametime_us;
    }
    
    cur_time  = get_ticks_us();

    if(cur_time >= Nexttime)
        time_left = 0;
    else
        time_left = Nexttime - cur_time;
    
    if(time_left > 50000) {
        time_left = 50000;
        /* In order to keep input responsive, don't wait too long at once */
        /* 50 ms wait gives us a 20 Hz responsetime which is nice. */
    } else InFrame = 0;
    
    //printf("attempting to sleep %Ld ms, frame complete=%i\n", time_left, InFrame);
    delay_us(time_left);
    
    if(!InFrame)
    {
        Lasttime = get_ticks_us();
        return 0; /* Done waiting */
    }
    return 1; /* Must still wait some more */
}

/**
 * Set the emulation speed throttling to the next entry in the speed table.
 */
void IncreaseEmulationSpeed(void)
{
puts("IncreaseEmulationSpeed");
    g_fpsScale *= LOGMUL;
    
    if(g_fpsScale > Fastest) g_fpsScale = Fastest;

    RefreshThrottleFPS();
     
    FCEU_DispMessage("emulation speed %.1f%%", g_fpsScale*100.0);
}

/**
 * Set the emulation speed throttling to the previous entry in the speed table.
 */
void DecreaseEmulationSpeed(void)
{
puts("DecreaseEmulationSpeed");
    g_fpsScale /= LOGMUL;
    if(g_fpsScale < Slowest)
        g_fpsScale = Slowest;

    RefreshThrottleFPS();

    FCEU_DispMessage("emulation speed %.1f%%", g_fpsScale*100.0);
}

/**
 * Set the emulation speed throttling to a specific value.
 */
void
FCEUD_SetEmulationSpeed(int cmd)
{
puts("SetEmulationSpeed");
    MaxSpeed = false;
    
    switch(cmd) {
    case EMUSPEED_SLOWEST:
        g_fpsScale = Slowest;
        break;
    case EMUSPEED_SLOWER:
        DecreaseEmulationSpeed();
        break;
    case EMUSPEED_NORMAL:
        g_fpsScale = Normal;
        break;
    case EMUSPEED_FASTER:
        IncreaseEmulationSpeed();
        break;
    case EMUSPEED_FASTEST:
        g_fpsScale = Fastest;
        MaxSpeed = true;
        break;
    default:
        return;
    }

    RefreshThrottleFPS();

    FCEU_DispMessage("emulation speed %.1f%%", g_fpsScale*100.0);
}
