#include "../common.h"

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>
#include <fcntl.h>

#define SYS_CLK_FREQ 27

int wiz_mmuhack(int mem_fd);

int mmuhack_fd;

s32 wiz_audio_volume = 60;
u32 wiz_dev_audio = 0;
u32 wiz_dev_mem = 0;

volatile u32 *wiz_memregs32;
volatile u16 *wiz_memregs16;

#define wiz_reg16(reg)                                                        \
  (wiz_memregs16[reg >> 1])                                                   \

#define wiz_reg32(reg)                                                        \
  (wiz_memregs32[reg >> 2])                                                   \

typedef enum
{
  MLC_CONTROLT    = 0x4000,
  MLC_SCREEN_SIZE = 0x4004,
  MLC0_LEFT_RIGHT = 0x400C,
  MLC0_TOP_BOTTOM = 0x4010,
  MLC0_CONTROL    = 0x4024,
  MLC0_HSTRIDE    = 0x4028,
  MLC0_VSTRIDE    = 0x402C,
  MLC0_ADDRESS    = 0x4038,
  MLC1_CONTROL    = 0x4058
} wiz_mlc_regs_enum;

typedef enum
{
  DPC_HTOTAL      = 0x307C,
  DPC_VTOTAL      = 0x3084,
  DPC_VSWIDTH     = 0x3086,
  DPC_VASTART     = 0x3088,
  DPC_VAEND       = 0x308A,
  DPC_CONTROL0    = 0x308C
} wiz_dplc_regs_enum;

typedef enum
{
  GPIO_APAD       = 0xA018,
  GPIO_BPAD       = 0xA058,
  GPIO_CPAD       = 0xA098
} wiz_gpio_regs_enum;

typedef enum
{
  PLL_SETREG0     = 0xF004,
  PWR_MODE        = 0xF07C
} wiz_pll_regs_enum;

typedef enum
{
  MEM_CFG         = 0x14800,
  MEM_TIME0       = 0x14802,
  MEM_TIME1       = 0x14804,
  MEM_REFRESH     = 0x14808,
  MEM_CONTROL     = 0x1480A
} wiz_memory_regs_enum;

typedef enum
{
  VIDEO_MODE_RGB565   = 0x4432,
  VIDEO_MODE_BGR565   = 0xC432,
  VIDEO_MODE_XRGB1555 = 0x4342,
  VIDEO_MODE_XBGR1555 = 0xC342,
  VIDEO_MODE_XRGB4444 = 0x4211,
  VIDEO_MODE_XBGR4444 = 0xC211,
  VIDEO_MODE_XRGB8332 = 0x4120,
  VIDEO_MODE_XBGR8332 = 0xC120,
  VIDEO_MODE_ARGB1555 = 0x3342,
  VIDEO_MODE_ABGR1555 = 0xB342,
  VIDEO_MODE_ARGB4444 = 0x2211,
  VIDEO_MODE_ABGR4444 = 0xA211,
  VIDEO_MODE_ARGB8332 = 0x1120,
  VIDEO_MODE_ABGR8332 = 0x9120,
  VIDEO_MODE_RGB888   = 0x4653,
  VIDEO_MODE_BGR888   = 0xC653,
  VIDEO_MODE_XRGB8888 = 0x4653,
  VIDEO_MODE_XBGR8888 = 0xC653,
  VIDEO_MODE_ARGB8888 = 0x0653,
  VIDEO_MODE_ABGR8888 = 0x8653,
  VIDEO_MODE_PTRGB565 = 0x443A
} video_mode_enum;

typedef struct
{
  u32 mlc_controlt;
  u32 mlc_screen_size;
  u32 mlc0_left_right;
  u32 mlc0_top_bottom;
  u32 mlc0_control;
  u32 mlc0_hstride;
  u32 mlc0_vstride;
  u32 mlc0_address;
  u32 mlc1_control;

  u32 pll_setreg0;
  u32 pwr_mode;

  u16 dpc_htotal;
} wiz_reg_backup_struct;

static wiz_reg_backup_struct wiz_reg_backup;

void wiz_backup_hardware_registers()
{
  wiz_reg_backup.mlc_controlt    = wiz_reg32(MLC_CONTROLT);
  wiz_reg_backup.mlc_screen_size = wiz_reg32(MLC_SCREEN_SIZE);
  wiz_reg_backup.mlc0_left_right = wiz_reg32(MLC0_LEFT_RIGHT);
  wiz_reg_backup.mlc0_top_bottom = wiz_reg32(MLC0_TOP_BOTTOM);
  wiz_reg_backup.mlc0_control    = wiz_reg32(MLC0_CONTROL);
  wiz_reg_backup.mlc0_hstride    = wiz_reg32(MLC0_HSTRIDE);
  wiz_reg_backup.mlc0_vstride    = wiz_reg32(MLC0_VSTRIDE);
  wiz_reg_backup.mlc0_address    = wiz_reg32(MLC0_ADDRESS);
  wiz_reg_backup.mlc1_control    = wiz_reg32(MLC1_CONTROL);
  wiz_reg_backup.pll_setreg0     = wiz_reg32(PLL_SETREG0);
  wiz_reg_backup.pwr_mode        = wiz_reg32(PWR_MODE);

  wiz_reg_backup.dpc_htotal      = wiz_reg16(DPC_HTOTAL);
}

void wiz_restore_hardware_registers()
{
  wiz_reg32(MLC_CONTROLT)     = wiz_reg_backup.mlc_controlt;
  wiz_reg32(MLC_SCREEN_SIZE)  = wiz_reg_backup.mlc_screen_size;
  wiz_reg32(MLC0_LEFT_RIGHT)  = wiz_reg_backup.mlc0_left_right;
  wiz_reg32(MLC0_TOP_BOTTOM)  = wiz_reg_backup.mlc0_top_bottom;
  wiz_reg32(MLC0_CONTROL)     = wiz_reg_backup.mlc0_control;
  wiz_reg32(MLC0_HSTRIDE)     = wiz_reg_backup.mlc0_hstride;
  wiz_reg32(MLC0_VSTRIDE)     = wiz_reg_backup.mlc0_vstride;
  wiz_reg32(MLC0_ADDRESS)     = wiz_reg_backup.mlc0_address;
  wiz_reg32(MLC1_CONTROL)     = wiz_reg_backup.mlc1_control;
  wiz_reg32(PLL_SETREG0)      = wiz_reg_backup.pll_setreg0;
  wiz_reg32(PWR_MODE)         = wiz_reg_backup.pwr_mode;

  wiz_reg16(DPC_HTOTAL)       = wiz_reg_backup.dpc_htotal;

  wiz_reg32(MLC0_CONTROL) |= (1 << 4);
  wiz_reg32(MLC1_CONTROL) |= (1 << 4);
  wiz_reg32(MLC_CONTROLT) |= (1 << 3);
}


u32 wiz_joystick_read()
{
  u32 value = ~((wiz_reg16(GPIO_CPAD) << 16) | wiz_reg16(GPIO_BPAD));

  value &=
   (WIZ_UP | WIZ_LEFT | WIZ_RIGHT | WIZ_DOWN | WIZ_MENU |
   WIZ_SELECT | WIZ_L | WIZ_R | WIZ_A | WIZ_B | WIZ_X | WIZ_Y |
   WIZ_VOL_DOWN | WIZ_VOL_UP);

  return value;
}

void wiz_wait_vsync_poll()
{
  static u64 last_ticks = 0;
  u64 current_ticks;
  u32 ticks_delta;
  u32 wait_for = 1;

  get_ticks_us(&current_ticks);
  // Ticks_delta is the number of ticks since the last vsync began. It gives
  // us a good idea of when the next vsync will begin - since we're at 120Hz
  // we should wait for either 1 or 2 vsyncs, or if things have gotten slow
  // wait for none.
  ticks_delta = current_ticks - last_ticks;

  if(ticks_delta > 16000)
    wait_for = 0;

  if(ticks_delta < 8000)
    wait_for = 2;

  while(wait_for)
  {
    while((wiz_reg16(DPC_CONTROL0) & (1 << 10)) == 0);
    wiz_reg16(DPC_CONTROL0) |= (1 << 10);
    wait_for--;
  }
  get_ticks_us(&last_ticks);
}

#define NUM_FRAMEBUFFERS   2
#define FRAMEBUFFERS_SIZE  (0x30000 * NUM_FRAMEBUFFERS)
#define FRAMEBUFFER_OFFSET 0x2A00000

u32 single_buffer_mode = 0;

u32 wiz_framebuffer_offsets[NUM_FRAMEBUFFERS] =
{
  FRAMEBUFFER_OFFSET,
  FRAMEBUFFER_OFFSET + 0x30000,
  //FRAMEBUFFER_OFFSET + 0x60000,
  //FRAMEBUFFER_OFFSET + 0x90000
};

u16 *wiz_framebuffer_ptrs[NUM_FRAMEBUFFERS];
u32 current_framebuffer = 0;

void wiz_set_framebuffer(u32 framebuffer_offset)
{
  wiz_reg32(MLC0_ADDRESS) = framebuffer_offset;
  // Set bit dirty so it takes effect
  wiz_reg32(MLC0_CONTROL) |= (1 << 4);
}

#define FBIO_MAGIC                'D'
#define FBIO_LCD_CHANGE_CONTROL   _IOW(FBIO_MAGIC, 90, unsigned int[2])
#define LCD_DIRECTION_ON_CMD      5   // 320x240
#define LCD_DIRECTION_OFF_CMD     6   // 240x320

void wiz_set_portrait_orientation(u32 on)
{
  u32 lcd_ioctl_settings[2] = { 0 };
  s32 fb_file_handle = open("/dev/fb0", O_RDWR);

  if(on)
    lcd_ioctl_settings[1] = LCD_DIRECTION_ON_CMD;
  else
    lcd_ioctl_settings[1] = LCD_DIRECTION_OFF_CMD;

  ioctl(fb_file_handle, FBIO_LCD_CHANGE_CONTROL, &lcd_ioctl_settings);
  close(fb_file_handle);
}

u16 *wiz_get_screen_ptr()
{
  return wiz_framebuffer_ptrs[current_framebuffer];
}

u32 wiz_get_screen_pitch()
{
#ifdef FRAMEBUFFER_PORTRAIT_ORIENTATION
  return (240 * 2);
#else
  return (320 * 2);
#endif
}


void wiz_update_screen()
{
  // Set the framebuffer to the backbuffer
  if(!config.fast_forward)
    wiz_wait_vsync_poll();

  wiz_set_framebuffer(wiz_framebuffer_offsets[current_framebuffer]);

  if(single_buffer_mode == 0)
  {
    // Set the backbuffer to the next buffer
    current_framebuffer++;

    if(current_framebuffer == NUM_FRAMEBUFFERS)
      current_framebuffer = 0;
  }
}

void wiz_clear_all_buffers()
{
  u32 i;

  for(i = 0; i < NUM_FRAMEBUFFERS; i++)
  {
    memset(wiz_framebuffer_ptrs[i], 0, 320 * 240 * 2);
  }
}

void wiz_clear_line_edges_all_buffers(u32 line_number,
 u32 color, u32 edge, u32 middle)
{
  u32 i2;
  u32 i;
  u32 *dest;

  color |= (color << 16);

  edge /= 2;
  middle /= 2;

  for(i2 = 0; i2 < NUM_FRAMEBUFFERS; i2++)
  {
    dest = (u32 *)(wiz_framebuffer_ptrs[i2] + (line_number * 320));

    for(i = 0; i < edge; i++)
    {
      *dest = color;
      dest++;
    }

    dest += middle;

    for(i = 0; i < edge; i++)
    {
      *dest = color;
      dest++;
    }
  }
}


void wiz_set_single_buffer_mode()
{
  single_buffer_mode = 1;
}

void wiz_set_multi_buffer_mode()
{
  single_buffer_mode = 0;
}

void wiz_sound_volume(s32 volume_change)
{
  u32 volume;

  wiz_audio_volume += volume_change;

  if(wiz_audio_volume < 0)
    wiz_audio_volume = 0;

  if(wiz_audio_volume > 80)
    wiz_audio_volume = 80;

  volume = (wiz_audio_volume << 8) | wiz_audio_volume;
  ioctl(wiz_dev_audio, SOUND_MIXER_WRITE_PCM, &volume);
}

void wiz_initialize_screen()
{
  u32 i;

  // mmap the framebuffers
  wiz_framebuffer_ptrs[0] = mmap(0, FRAMEBUFFERS_SIZE, PROT_READ | PROT_WRITE,
   MAP_SHARED, wiz_dev_mem, wiz_framebuffer_offsets[0]);

  printf("mapped framebuffers from virtual address %p to physical\n"
   "location %x\n", wiz_framebuffer_ptrs[0], wiz_framebuffer_offsets[0]);

  for(i = 1; i < NUM_FRAMEBUFFERS; i++)
  {
    wiz_framebuffer_ptrs[i] = wiz_framebuffer_ptrs[i - 1] + 0x18000;
  }

#ifdef FRAMEBUFFER_PORTRAIT_ORIENTATION
  // Set the resolution to 240x320, 16bpp
  wiz_reg32(MLC_SCREEN_SIZE) = ((320 - 1) << 16) | (240 - 1);
  // Set the dirty bit so that it takes effect.
  wiz_reg32(MLC_CONTROLT) |= (1 << 3);
  wiz_reg32(MLC0_HSTRIDE) = 2;
  wiz_reg32(MLC0_VSTRIDE) = 240 * 2;

  // Set the layer position to 0, 0 to 239, 319
  wiz_reg32(MLC0_LEFT_RIGHT) = (0 << 16) | (240 - 1);
  wiz_reg32(MLC0_TOP_BOTTOM) = (0 << 16) | (320 - 1);

  wiz_set_portrait_orientation(0);
#else
  // Set the resolution to 320x240, 16bpp
  wiz_reg32(MLC_SCREEN_SIZE) = ((240 - 1) << 16) | (320 - 1);
  // Set the dirty bit so that it takes effect.
  wiz_reg32(MLC_CONTROLT) |= (1 << 3);
  wiz_reg32(MLC0_HSTRIDE) = 2;
  wiz_reg32(MLC0_VSTRIDE) = 320 * 2;

  // Set the layer position to 0, 0 to 319, 239
  wiz_reg32(MLC0_LEFT_RIGHT) = (0 << 16) | (320 - 1);
  wiz_reg32(MLC0_TOP_BOTTOM) = (0 << 16) | (240 - 1);

  wiz_set_portrait_orientation(1);
#endif

  // Clear the framebuffers
  wiz_clear_all_buffers();
  wiz_set_framebuffer(wiz_framebuffer_offsets[0]);

  // Enable the first layer and disable the second layer.
  wiz_reg32(MLC0_CONTROL) = ((1 << 4) | (1 << 5)) | (VIDEO_MODE_RGB565 << 16);
  wiz_reg32(MLC1_CONTROL) = (1 << 4);

  printf("Loading mmuhack...\n");
  mmuhack_fd = open("/dev/mmuhack", O_RDWR);
  if(mmuhack_fd < 0)
  {
    printf("Loading mmuhack module.\n");
    system("insmod mmuhack.ko");
    mmuhack_fd = open("/dev/mmuhack", O_RDWR);

    if(mmuhack_fd < 0)
      printf("Failed to load mmuhack.\n");
  }

  printf("DPC_HTOTAL: %x\n", wiz_reg16(DPC_HTOTAL));
  printf("DPC_VTOTAL: %x\n", wiz_reg16(DPC_VTOTAL));
  printf("DPC_VSWIDTH: %x\n", wiz_reg16(DPC_VSWIDTH));
  printf("DPC_VASTART: %x\n", wiz_reg16(DPC_VASTART));
  printf("DPC_VAEND: %x\n", wiz_reg16(DPC_VAEND));

  wiz_reg16(DPC_HTOTAL) = 0xf3;
}

void wiz_initialize()
{
  printf("Initializing Wiz.\n");

  wiz_dev_mem = open("/dev/mem", O_RDWR);
  wiz_dev_audio = open("/dev/mixer", O_RDWR);

  printf("Got mem device %d, audio device %d\n",  wiz_dev_mem,
   wiz_dev_audio);

  wiz_memregs32 =
   (u32 *)mmap(0, 0x20000, PROT_READ | PROT_WRITE, MAP_SHARED,
   wiz_dev_mem, 0xC0000000);
  wiz_memregs16 = (u16 *)wiz_memregs32;

  printf("Got hardware register pointer %p\n", wiz_memregs32);

  printf("Backing up hardware registers.\n");
  wiz_backup_hardware_registers();

  printf("Setting volume.\n");
  wiz_sound_volume(0);

  printf("MEM_CFG: %x\n", wiz_reg16(MEM_CFG));
  printf("MEM_TIME0: %x\n", wiz_reg16(MEM_TIME0));
  printf("MEM_TIME1: %x\n", wiz_reg16(MEM_TIME1));
  printf("MEM_REFRESH: %x\n", wiz_reg16(MEM_REFRESH));
  printf("MEM_CONTROL: %x\n", wiz_reg16(MEM_CONTROL));
}

void wiz_quit()
{
  wiz_restore_hardware_registers();

  munmap((void *)wiz_memregs32, 0x10000);
  munmap((void *)wiz_framebuffer_ptrs[0], FRAMEBUFFERS_SIZE);
  close(wiz_dev_audio);
  close(wiz_dev_mem);

  if(mmuhack_fd < 0)
  {
    printf("Closing mmuhack.\n");
    close(mmuhack_fd);
  }

  if(config.relaunch_shell_on_quit)
  {
    printf("Relaunching Wiz menu.\n");
    chdir("/usr/gp2x");
    execl("gp2xmenu", "gp2xmenu", NULL);
  }
}

void wiz_set_clock_speed(u32 clock_speed)
{
  u32 mdiv;
  u32 pdiv = 9;
  u32 sdiv = 0;

  mdiv = ((clock_speed * pdiv) / SYS_CLK_FREQ) & 0x3FF;
  wiz_reg32(PLL_SETREG0) = (pdiv << 18) | (mdiv << 8) | sdiv;
  wiz_reg32(PWR_MODE) |= 0x8000;
}

