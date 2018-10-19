#include "../common.h"

#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#ifndef FBIO_WAITFORVSYNC
  #define FBIO_WAITFORVSYNC _IOW('F', 0x20, __u32)
#endif

u32 fbdev_handle;
u16 *fb_pixels[2];
u32 current_buffer = 1;
u32 double_buffer_mode = 1;

#define plot_pixel_2x(offset)                                                 \
  dest_pixels[offset] = current_pixel;                                        \
  dest_pixels[offset + 1] = current_pixel                                     \

#define plot_pixel_3x(offset)                                                 \
  plot_pixel_2x(offset);                                                      \
  dest_pixels[offset + 2] = current_pixel                                     \

#define plot_pixel_4x(offset)                                                 \
  plot_pixel_3x(offset);                                                      \
  dest_pixels[offset + 3] = current_pixel                                     \

void set_screen_buffer(u32 buffer_number)
{
  u32 screen_y = 0;
  struct fb_var_screeninfo fb_screen_info;

  if(buffer_number)
    screen_y = 240;

  ioctl(fbdev_handle, FBIOGET_VSCREENINFO, &fb_screen_info);
  fb_screen_info.yoffset = screen_y;
  ioctl(fbdev_handle, FBIOPAN_DISPLAY, &fb_screen_info);
}

void wait_for_vsync()
{
  int arg = 0;
  ioctl(fbdev_handle, FBIO_WAITFORVSYNC, &arg);
}

void update_screen()
{
  // Flip screen to use current buffer
  if(double_buffer_mode)
  {
    set_screen_buffer(current_buffer);
    current_buffer ^= 1;
  }
}

void set_screen_resolution(u32 width, u32 height)
{
  u16 *fb_pixels_double;

  // Set output screen to 640x480
  system("ofbset -fb /dev/fb1 -pos 80 0 -size 640 480 -mem 307200 -en 1");
  // Set input screen to 320x240x16bpp, 2 frames
  system("fbset -fb /dev/fb1 -g 320 240 320 480 16");

  fbdev_handle = open("/dev/fb1", O_RDWR);
  fflush(stdout);

  fb_pixels_double = mmap(0, 320 * 240 * 2 * 2, PROT_WRITE, MAP_SHARED,
   fbdev_handle, 0);
  fb_pixels[0] = fb_pixels_double;
  fb_pixels[1] = fb_pixels_double + (320 * 240);

  printf("Got buffers %p, %p\n", fb_pixels[0], fb_pixels[1]);

  set_screen_buffer(0);
}

void reset_screen(void)
{
  close(fbdev_handle);
  // Disable screen overlay
  system("ofbset -fb /dev/fb1 -pos 0 0 -size 0 0 -mem 0 -en 0");
}

void *get_screen_ptr()
{
  return fb_pixels[current_buffer];
}

u32 get_screen_pitch()
{
  return 320;
}

void clear_screen()
{
  memset(fb_pixels[current_buffer], 0, 320 * 240 * 2);
}

void clear_line_edges(u32 line_number, u32 color, u32 edge, u32 middle)
{
  u32 *dest_a = (u32 *)(fb_pixels[0] + (line_number * 320));
  u32 *dest_b = (u32 *)(fb_pixels[1] + (line_number * 320));
  u32 i;

  color |= (color << 16);

  edge /= 2;
  middle /= 2;

  for(i = 0; i < edge; i++)
  {
    dest_a[i] = color;
    dest_b[i] = color;
  }

  dest_a += middle + edge;
  dest_b += middle + edge;

  for(i = 0; i < edge; i++)
  {
    dest_a[i] = color;
    dest_b[i] = color;
  }
}

void set_single_buffer_mode()
{
  current_buffer = 0;
  set_screen_buffer(0);
  double_buffer_mode = 0;  
}

void set_multi_buffer_mode()
{
  double_buffer_mode = 1;
  current_buffer = 0;
  set_screen_buffer(1);
}

void clear_all_buffers()
{
  memset(fb_pixels[0], 0, 320 * 240 * 2);
  memset(fb_pixels[1], 0, 320 * 240 * 2);
}

