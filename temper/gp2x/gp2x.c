#include "../common.h"

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>


s32 gp2x_audio_volume = 60;
u32 gp2x_dev_audio = 0;
u32 gp2x_dev_mem = 0;

volatile u32 *gp2x_memregs32;
volatile u16 *gp2x_memregs16;

#define gp2x_reg16(reg)                                                       \
  (gp2x_memregs16[reg >> 1])                                                  \

#define gp2x_reg32(reg)                                                       \
  (gp2x_memregs32[reg >> 2])                                                  \


s32 gp2x_load_mmuhack()
{
  s32 mmufd = open("/dev/mmuhack", O_RDWR);

  if(mmufd < 0)
  {
    system("/sbin/insmod ./mmuhack.o");
    mmufd = open("/dev/mmuhack", O_RDWR);
  }

  if(mmufd < 0)
    return -1;

  close(mmufd);
  return 0;
}

u32 gp2x_joystick_read()
{
  u32 value = (gp2x_reg16(0x1198) & 0x00FF);

  value = ~((gp2x_reg16(0x1184) & 0xFF00) | value |
   (gp2x_reg16(0x1186) << 16));

  if(value & GP2X_UP_LEFT)
    value |= GP2X_UP | GP2X_LEFT;

  if(value & GP2X_DOWN_LEFT)
    value |= GP2X_DOWN | GP2X_LEFT;

  if(value & GP2X_DOWN_RIGHT)
    value |= GP2X_DOWN | GP2X_RIGHT;

  if(value & GP2X_UP_RIGHT)
    value |= GP2X_UP | GP2X_RIGHT;

  if((value & (GP2X_LEFT | GP2X_RIGHT)) == (GP2X_LEFT | GP2X_RIGHT))
    value &= ~GP2X_LEFT;

  if((value & (GP2X_UP | GP2X_DOWN)) == (GP2X_UP | GP2X_DOWN))
    value &= ~GP2X_DOWN;

  value &=
   (GP2X_UP | GP2X_LEFT | GP2X_RIGHT | GP2X_DOWN | GP2X_START |
   GP2X_SELECT | GP2X_L | GP2X_R | GP2X_A | GP2X_B | GP2X_X | GP2X_Y |
   GP2X_VOL_DOWN | GP2X_VOL_UP | GP2X_VOL_MID | GP2X_PUSH);

  return value;
}

#define NUM_FRAMEBUFFERS   4
#define FRAMEBUFFERS_SIZE  (0x30000 * NUM_FRAMEBUFFERS)
#define FRAMEBUFFER_OFFSET (0x4000000 - FRAMEBUFFERS_SIZE)

u32 single_buffer_mode = 0;

u32 gp2x_framebuffer_offsets[4] =
{
  FRAMEBUFFER_OFFSET,
  FRAMEBUFFER_OFFSET + 0x30000,
  FRAMEBUFFER_OFFSET + 0x60000,
  FRAMEBUFFER_OFFSET + 0x90000
};

u16 *gp2x_framebuffer_ptrs[4];
u32 current_framebuffer = 0;

u16 gp2x_290E_old, gp2x_2910_old, gp2x_2912_old, gp2x_2914_old;

void gp2x_set_framebuffer(u32 framebuffer_offset)
{
  u32 framebuffer_offset_high = (u32)(framebuffer_offset) >> 16;
  u32 framebuffer_offset_low = (u32)(framebuffer_offset) & 0xFFFF;

  gp2x_reg16(0x2910) = framebuffer_offset_high;
  gp2x_reg16(0x2914) = framebuffer_offset_high;
  gp2x_reg16(0x290E) = framebuffer_offset_low;
  gp2x_reg16(0x2912) = framebuffer_offset_low;
}

void gp2x_initialize_screen()
{
  gp2x_290E_old = gp2x_reg16(0x290E);
  gp2x_2910_old = gp2x_reg16(0x2910);
  gp2x_2912_old = gp2x_reg16(0x2912);
  gp2x_2914_old = gp2x_reg16(0x2914);

  // mmap the framebuffers
  gp2x_framebuffer_ptrs[0] = mmap(0, FRAMEBUFFERS_SIZE, PROT_WRITE,
   MAP_SHARED, gp2x_dev_mem, gp2x_framebuffer_offsets[0]);

  printf("mapped framebuffers from virtual address %p to physical\n"
   "location %x\n", gp2x_framebuffer_ptrs[0], gp2x_framebuffer_offsets[0]);

  gp2x_framebuffer_ptrs[1] = gp2x_framebuffer_ptrs[0] + 0x18000;
  gp2x_framebuffer_ptrs[2] = gp2x_framebuffer_ptrs[1] + 0x18000;
  gp2x_framebuffer_ptrs[3] = gp2x_framebuffer_ptrs[2] + 0x18000;

  // Set the video mode to 16bpp, 320 wide
  gp2x_reg16(0x28DA) = (((16 + 1) / 8) << 9) | 0xAB;
  gp2x_reg16(0x290C) = 320 * ((16 + 1) / 8);

  memset(gp2x_framebuffer_ptrs[0], 0, FRAMEBUFFERS_SIZE);

  gp2x_set_framebuffer(gp2x_framebuffer_offsets[0]);

  gp2x_load_mmuhack();
}

u16 *gp2x_get_screen_ptr()
{
  return gp2x_framebuffer_ptrs[current_framebuffer];
}

u32 gp2x_get_screen_pitch()
{
  return (320 * 2);
}

void gp2x_update_screen()
{
  // Set the framebuffer to the backbuffer

  //invalidate_cache_region(gp2x_framebuffer_ptrs[current_framebuffer],
  // 320 * 240 * 2);

  gp2x_set_framebuffer(gp2x_framebuffer_offsets[current_framebuffer]);

  if(single_buffer_mode == 0)
  {
    // Set the backbuffer to the next buffer

    current_framebuffer++;

    if(current_framebuffer == NUM_FRAMEBUFFERS)
      current_framebuffer = 0;
  }
}

void gp2x_clear_all_buffers()
{
  u32 i;

  for(i = 0; i < 4; i++)
  {
    memset(gp2x_framebuffer_ptrs[i], 0, 320 * 240 * 2);
  }
}

void gp2x_clear_line_edges_all_buffers(u32 line_number,
 u32 color, u32 edge, u32 middle)
{
  u32 i2;
  u32 i;
  u32 *dest;

  color |= (color << 16);

  edge /= 2;
  middle /= 2;

  for(i2 = 0; i2 < 4; i2++)
  {
    dest = (u32 *)(gp2x_framebuffer_ptrs[i2] + (line_number * 320));

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


void gp2x_set_single_buffer_mode()
{
  single_buffer_mode = 1;
}

void gp2x_set_multi_buffer_mode()
{
  single_buffer_mode = 0;
}

void gp2x_sound_volume(s32 volume_change)
{
  u32 volume;

  gp2x_audio_volume += volume_change;

  if(gp2x_audio_volume < 0)
    gp2x_audio_volume = 0;

  if(gp2x_audio_volume > 80)
    gp2x_audio_volume = 80;

  volume = (gp2x_audio_volume << 8) | gp2x_audio_volume;
  ioctl(gp2x_dev_audio, SOUND_MIXER_WRITE_PCM, &volume);
}

#include <sys/mman.h>
#include <math.h>

#define GP2X_CLK_FREQ 7372800

u16 sys_cset_reg;
u16 fpl_lvset_reg;
u16 sys_clk_en_reg;
u16 mem_ram_timings_0;
u16 mem_ram_timings_1;
u16 display_clock_div;

typedef enum
{
  GP2X_REG_INT_MASK          = 0x0808,
  GP2X_REG_CLK_CHG_ST        = 0x0902,
  GP2X_REG_SYS_CLK_EN        = 0x0904,
  GP2X_REG_F_PLL_SET_V       = 0x0910,
  GP2X_REG_SYS_C_SET         = 0x091c,
  GP2X_REG_DISP_C_SET        = 0x0924,
  GP2X_REG_MLC_OVLAY_CTR     = 0x2880,
  GP2X_REG_MLC_GAMMA_A       = 0x295C,
  GP2X_REG_MLC_GAMMA_D       = 0x295E,
  GP2X_REG_BANK_TIMEX_0      = 0x3802,
  GP2X_REG_BANK_TIMEX_1      = 0x3804
} gp2x_io_reg_enum;

void gp2x_cpu_ctrl_initialize(void)
{
  sys_clk_en_reg = gp2x_reg16(GP2X_REG_SYS_CLK_EN);
  fpl_lvset_reg = gp2x_reg16(GP2X_REG_F_PLL_SET_V);
  sys_cset_reg = gp2x_reg16(GP2X_REG_SYS_C_SET);

  display_clock_div = gp2x_reg16(GP2X_REG_DISP_C_SET);
  mem_ram_timings_0 = gp2x_reg16(GP2X_REG_BANK_TIMEX_0);
  mem_ram_timings_1 = gp2x_reg16(GP2X_REG_BANK_TIMEX_1);
}

void gp2x_cpu_ctrl_quit(void)
{
  gp2x_reg16(GP2X_REG_SYS_CLK_EN) = sys_clk_en_reg;
  gp2x_reg16(GP2X_REG_F_PLL_SET_V) = fpl_lvset_reg;
  gp2x_reg16(GP2X_REG_SYS_C_SET) = sys_cset_reg;

  gp2x_reg16(GP2X_REG_DISP_C_SET) = display_clock_div;
  gp2x_reg16(GP2X_REG_BANK_TIMEX_0) = mem_ram_timings_0;
  gp2x_reg16(GP2X_REG_BANK_TIMEX_1) = mem_ram_timings_1;
}

void gp2x_set_display_clock_div(u32 div)
{
  div = (div & 0x40) | 0x40;
  gp2x_reg16(GP2X_REG_DISP_C_SET) =
   (gp2x_reg16(GP2X_REG_DISP_C_SET) & 0xFF) | (div << 8);
}

u32 gp2x_disable_interrupts()
{
  u32 interrupt_state = gp2x_reg32(GP2X_REG_INT_MASK);
  gp2x_reg32(GP2X_REG_INT_MASK) = 0xFF8FFFE7;

  return interrupt_state;
}

void gp2x_enable_interrupts(u32 interrupt_state)
{
  gp2x_reg32(GP2X_REG_INT_MASK) = interrupt_state;
}

void gp2x_set_flck(u32 mhz)
{
  printf("Setting GP2X clock speed to %d MHz\n", mhz);

  u32 mdiv;
  u32 pdiv = 3;
  u32 scale = 0;
  u32 khz = mhz * 1000000;

  u32 interrupt_state;

  mdiv = (khz * pdiv) / GP2X_CLK_FREQ;
  mdiv = ((mdiv - 8) & 0xFF) << 8;
  pdiv = ((pdiv - 2) & 0x3F) << 2;

  interrupt_state = gp2x_disable_interrupts();
  gp2x_reg16(GP2X_REG_F_PLL_SET_V) = mdiv | pdiv | scale;

  // Wait for this to take effect.
  while(gp2x_reg16(GP2X_REG_CLK_CHG_ST) & 1);

  // Turn interrupts back to what they were.
  gp2x_enable_interrupts(interrupt_state);
}

void gp2x_set_920_div(u32 div)
{
  gp2x_reg16(GP2X_REG_SYS_C_SET) =
   (gp2x_reg16(GP2X_REG_SYS_C_SET) & ~0x3) | (div & 0x3);
}

void gp2x_set_dlck_div(u32 div)
{
  gp2x_reg16(GP2X_REG_SYS_C_SET) =
   (gp2x_reg16(GP2X_REG_SYS_C_SET) & ~(0x7 << 6)) | ((div & 0x7) << 6);
}

void gp2x_set_ram_timings(u32 tRC, u32 tRAS, u32 tWR, u32 tMRD, u32 tRFC,
 u32 tRP, u32 tRCD)
{
  //u32 interrupt_state;
  printf("Setting RAM timings to:\n"
   "  tRC %d\n  tRAS %d\n  tWR %d\n  tMRD %d\n  tRFC %d\n  tRP %d\n"
   "  tRCD\n", tRC, tRAS, tWR, tMRD, tRFC, tRP, tRCD);

  tRC--;
  tRAS--;
  tWR--;
  tMRD--;
  tRFC--;
  tRP--;
  tRCD--;

  //interrupt_state = gp2x_disable_interrupts();
  gp2x_reg16(GP2X_REG_BANK_TIMEX_0) = ((tMRD & 0xF) << 12) |
   ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
  gp2x_reg16(GP2X_REG_BANK_TIMEX_1) = ((tRC & 0xF) << 8) |
   ((tRAS & 0xF) << 4) | (tWR & 0xF);
  //gp2x_enable_interrupts(interrupt_state);
}

void gp2x_default_ram_timings()
{
  //u32 interrupt_state;
  printf("Resetting RAM timings to startup values.\n");

  //interrupt_state = gp2x_disable_interrupts();
  gp2x_reg16(GP2X_REG_BANK_TIMEX_0) = mem_ram_timings_0;
  gp2x_reg16(GP2X_REG_BANK_TIMEX_1) = mem_ram_timings_1;
  //gp2x_enable_interrupts(interrupt_state);
}

void gp2x_set_gamma(u32 gamma_value_percent)
{
  //u32 interrupt_state;
  float gamma_value_f;
  u32 i;
  u8 gamma_value_u8;

  if(gamma_value_percent > 300)
    gamma_value_percent = 300;

  gamma_value_f = (float)gamma_value_percent / 100.0;
  gamma_value_f = 1.0 / gamma_value_f;

  //interrupt_state = gp2x_disable_interrupts();
  // Enable gamma.
  gp2x_reg16(GP2X_REG_MLC_OVLAY_CTR) &= ~(1 << 12);
  gp2x_reg16(GP2X_REG_MLC_GAMMA_A) = 0;

  for(i = 0; i < 256; i++)
  {
    gamma_value_u8 = (u8)(255.0 * pow(i / 255.0, gamma_value_f));

    gp2x_reg16(GP2X_REG_MLC_GAMMA_D) =
     (gamma_value_u8 << 8) | gamma_value_u8;
    gp2x_reg16(GP2X_REG_MLC_GAMMA_D) = gamma_value_u8;
  }
  //gp2x_enable_interrupts(interrupt_state);
}

void gp2x_set_60hz_mode()
{

}

char min_readahead_str[256];
char max_readahead_str[256];

void gp2x_initialize()
{
  FILE *readahead_fp;

  gp2x_dev_mem = open("/dev/mem", O_RDWR);
  gp2x_dev_audio = open("/dev/mixer", O_RDWR);

  gp2x_memregs32 =
   (u32 *)mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED,
   gp2x_dev_mem, 0xc0000000);
  gp2x_memregs16 = (u16 *)gp2x_memregs32;

  readahead_fp = fopen("/proc/sys/vm/min-readahead", "rb");
  fgets(min_readahead_str, 255, readahead_fp);
  fclose(readahead_fp);

  readahead_fp = fopen("/proc/sys/vm/max-readahead", "rb");
  fgets(max_readahead_str, 255, readahead_fp);
  fclose(readahead_fp);

  printf("min readahead: %s\nmax readahead: %s\n",
   min_readahead_str, max_readahead_str);

  readahead_fp = fopen("/proc/sys/vm/min-readahead", "wb");
  fputs("0", readahead_fp);
  fclose(readahead_fp);

  readahead_fp = fopen("/proc/sys/vm/max-readahead", "wb");
  fputs("0", readahead_fp);
  fclose(readahead_fp);

  gp2x_cpu_ctrl_initialize();

  if(config.ram_timings)
    gp2x_set_ram_timings(6, 4, 1, 1, 1, 2, 2);
  else
    gp2x_default_ram_timings();

  gp2x_set_flck(config.clock_speed);
  gp2x_set_gamma(config.gamma_percent);

  gp2x_sound_volume(0);
}

void gp2x_quit()
{
  FILE *readahead_fp;
  gp2x_cpu_ctrl_quit();

  gp2x_reg16(0x290E) = gp2x_290E_old;
  gp2x_reg16(0x2910) = gp2x_2910_old;
  gp2x_reg16(0x2912) = gp2x_2912_old;
  gp2x_reg16(0x2914) = gp2x_2914_old;

  gp2x_set_gamma(100);

  munmap((void *)gp2x_memregs32, 0x10000);
  munmap((void *)gp2x_framebuffer_ptrs[0], FRAMEBUFFERS_SIZE);
  close(gp2x_dev_audio);
  close(gp2x_dev_mem);

  system("/sbin/rmmod mmuhack");
  chdir("/usr/gp2x");

  readahead_fp = fopen("/proc/sys/vm/min-readahead", "wb");
  fputs(max_readahead_str, readahead_fp);
  fclose(readahead_fp);

  readahead_fp = fopen("/proc/sys/vm/max-readahead", "wb");
  fputs(max_readahead_str, readahead_fp);
  fclose(readahead_fp);

  if(config.relaunch_shell_on_quit)
    execl("gp2xmenu", "gp2xmenu", NULL);
}

