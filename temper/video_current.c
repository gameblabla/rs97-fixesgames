#include "common.h"

// Put temp debug vars here

u64 scanline_cycle_counter;

// ARM versions are using this, but it doesn't really hurt to
// have it here like this
u32 sp_storage;

video_debug_mode_enum video_debug_mode = NO_DEBUG;

#ifdef ARM_ARCH
u16 scanline_buffer[512] __attribute__ ((aligned(32)));
#else
u16 scanline_buffer[512] __attribute__ ((aligned(16)));
#endif

vdc_struct vdc;

void update_satb(u16 *satb_location);
void vram_dma_line(u32 scanline_cycles);

u32 output_width = 320;
u32 output_height = 240;

#define update_screen_dimensions(type)                                        \
  vdc.center_##type##_offset = 0;                                             \
  vdc.overdraw_##type##_offset = 0;                                           \
  if(vdc.screen_##type > output_##type)                                       \
  {                                                                           \
    vdc.overdraw_##type##_offset =                                            \
     (vdc.screen_##type - output_##type) / 2;                                 \
    vdc.screen_##type = output_##type;                                        \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    if(vdc.screen_##type < output_##type)                                     \
    {                                                                         \
      vdc.center_##type##_offset =                                            \
       (output_##type - vdc.screen_##type) / 2;                               \
    }                                                                         \
  }                                                                           \

u32 vdc_status()
{
  memory.io_buffer = vdc.status;

  irq.status &= ~IRQ_VDC;
  vdc.status &= ~0x3F;

  return memory.io_buffer;
}


void vdc_register_select(u32 value)
{
  vdc.register_select = value & 0x1F;
}

u32 vdc_data_read_low()
{
  return (vdc.read_latch & 0xFF);
}

u32 vdc_data_read_high()
{
  u32 read_value = vdc.read_latch >> 8;
  if(vdc.register_select == 0x2)
  {
    vdc.marr = (vdc.marr + vdc.increment_value) & 0xFFFF;

    if(vdc.marr < 0x8000)
      vdc.read_latch = vdc.vram[vdc.marr];
  }

  return read_value;
}

void vdc_data_write_low(u32 value)
{
  switch(vdc.register_select)
  {
    case 0x0:
      vdc.mawr = (vdc.mawr & 0xFF00) | value;
      break;

    case 0x1:
      vdc.marr = (vdc.marr & 0xFF00) | value;
      break;

    case 0x2:
      vdc.write_latch = value;
      break;

    case 0x5:
      // control register, low byte
      // D7 : 1= Background enable
      // D6 : 1= Sprite enable
      // D5 : External sync (bit 1)
      // D4 : External sync (bit 0)
      // D3 : 1= Enable interrupt for vertical blanking
      // D2 : 1= Enable interrupt for raster compare
      // D1 : 1= Enable interrupt for sprite overflow
      // D0 : 1= Enable interrupt for sprite #0 collision
      vdc.cr = (vdc.cr & 0xFF00) | value;
      // vce.frames_rendered, vce.frame_counter);
      break;

    case 0x6:
      vdc.rcr = (vdc.rcr & 0xFF00) | value;
      break;

    case 0x7:
      vdc.bxr = (vdc.bxr & 0xFF00) | value;
      break;

    case 0x8:
      vdc.byr = (vdc.byr & 0xFF00) | value;
      vdc.effective_byr = vdc.byr;
      break;

    case 0x9:
      // mwr
      // D7 : CG mode
      // D6 : Virtual screen height
      // D5 : Virtual screen width (bit 1)
      // D4 : Virtual screen width (bit 0)
      // D3 : Sprite dot period (bit 1)
      // D2 : Sprite dot period (bit 0)
      // D1 : VRAM dot width (bit 1)
      // D0 : VRAM dot width (bit 0)
      vdc.latched_mwr = value;
      break;

    case 0xA:
      vdc.hsw = value & 0x1F;
      break;

    case 0xB:
    {
      vdc.hdw = value & 0x7F;
      vdc.screen_width = (vdc.hdw + 1) * 8;
      update_screen_dimensions(width);

      if(vdc.screen_width != vdc.sat_screen_width)
        update_satb(vdc.sat);
      break;
    }

    case 0xC:
      vdc.vsw = value & 0x1F;
      break;

    case 0xD:
      vdc.vdw = (vdc.vdw & 0xFF00) | value;
      vdc.screen_height = vdc.vdw + 1;
      if(vdc.screen_height > 242)
        vdc.screen_height = 242;
      update_screen_dimensions(height);
      break;

    case 0xE:
      vdc.vcr = value;
      break;

    case 0xF:
      vdc.dcr = value;
      break;

    case 0x10:
      vdc.sour = (vdc.sour & 0xFF00) | value;
      break;

    case 0x11:
      vdc.desr = (vdc.desr & 0xFF00) | value;
      break;

    case 0x12:
      vdc.lenr = (vdc.lenr & 0xFF00) | value;
      break;

    case 0x13:
      vdc.satb = (vdc.satb & 0xFF00) | value;
      vdc.satb_dma_trigger = 1;
      break;

    case 0x3:
    case 0x4:
      break;
  }
}

void vdc_data_write_high(u32 value)
{
  switch(vdc.register_select)
  {
    case 0x0:
      vdc.mawr = (vdc.mawr & 0xFF) | (value << 8);
      break;

    case 0x1:
      vdc.marr = (vdc.marr & 0xFF) | (value << 8);
      vdc.read_latch = vdc.vram[vdc.marr];
      break;

    case 0x2:
    {
      if(vdc.mawr < 0x8000)
      {
        u32 write_value = (value << 8) | vdc.write_latch;
        vdc.vram[vdc.mawr] = write_value;
        vdc.dirty_tiles[vdc.mawr / 16] = 1;
        vdc.dirty_patterns[vdc.mawr / 64] = 1;

        if(vdc.satb_dma_irq_lines)
        {
          cpu.alert = 1;
          cpu.vdc_stalled = 1;
        }
      }

      vdc.mawr = ((vdc.mawr + vdc.increment_value) & 0xFFFF);

      break;
    }

    case 0x5:
      // control register, high byte
      // D15-D13 : Unused
      // D12 : Increment width select (bit 1)
      // D11 : Increment width select (bit 0)
      // D10 : 1= DRAM refresh enable
      // D9 : DISP terminal output mode (bit 1)
      // D8 : DISP terminal output mode (bit 0)
      value &= 0x1F;
      vdc.cr = (vdc.cr & 0xFF) | (value << 8);
      switch(value >> 3)
      {
        case 0x0:
          vdc.increment_value = 0x01;
          break;

        case 0x1:
          vdc.increment_value = 0x20;
          break;

        case 0x2:
          vdc.increment_value = 0x40;
          break;

        case 0x3:
          vdc.increment_value = 0x80;
          break;
      }
      break;

    case 0x6:
      vdc.rcr = (vdc.rcr & 0xFF) | ((value & 0x3) << 8);
      break;

    case 0x7:
      vdc.bxr = (vdc.bxr & 0xFF) | ((value & 0x3) << 8);
      break;

    case 0x8:
    {
      vdc.byr = (vdc.byr & 0xFF) | ((value & 0x1) << 8);
      vdc.effective_byr = vdc.byr;
      break;
    }

    case 0xA:
      vdc.hds = value & 0x7F;
      break;

    case 0xB:
      vdc.hde = value & 0x7F;
      break;

    case 0xC:
      vdc.vds = value;
      break;

    case 0xD:
      vdc.vdw = (vdc.vdw & 0xFF) | ((value & 0x1) << 8);
      update_screen_dimensions(height);
      break;

    case 0x10:
      vdc.sour = (vdc.sour & 0xFF) | (value << 8);
      break;

    case 0x11:
      vdc.desr = (vdc.desr & 0xFF) | (value << 8);
      break;

    case 0x12:
      vdc.lenr = (vdc.lenr & 0xFF) | (value << 8);
      vdc.vram_dma_trigger = 1;

      // This is a hack.
      if(vdc.burst_mode && ((vdc.dcr & 0x02) == 0))
        vram_dma_line(1364);
      break;

    case 0x13:
      vdc.satb = (vdc.satb & 0xFF) | (value << 8);
      vdc.satb_dma_trigger = 1;
      break;

    default:
      break;
  }
}

void initialize_vdc()
{
  u32 i;

  for(i = 0; i < 240; i++)
  {
    vdc.scanline_widths[i] = 256;
  }
}

void reset_vdc()
{
  u32 i;

  vdc.status = 0;
  vdc.register_select = 0;
  vdc.mawr = 0;
  vdc.marr = 0;
  vdc.cr = 0;
  vdc.rcr = 0;
  vdc.mwr = 0;
  vdc.bxr = 0;
  vdc.byr = 0;
  vdc.dcr = 0;
  vdc.sour = 0;
  vdc.desr = 0;
  vdc.lenr = 0;
  vdc.satb = 0;

  vdc.write_latch = 0xFF;
  vdc.read_latch = 0xFF;

  vdc.raster_line = 0;
  vdc.effective_byr = 0;

  vdc.increment_value = 1;

  vdc.satb_dma_trigger = 0;
  vdc.satb_dma_irq_lines = 0;
  vdc.vram_dma_trigger = 0;
  vdc.vram_dma_irq_lines = 0;

  vdc.hsw = 0x1F;
  vdc.hds = 0x00;
  vdc.hdw = 0x7F;
  vdc.hde = 0x00;

  vdc.vsw = 0x02;
  vdc.vds = 0x0F;
  vdc.vdw = 0xFF;
  vdc.vcr = 0xFF;

  vdc.display_counter = 0;
  vdc.burst_mode = 0;

  vdc.center_width_offset = 0;
  vdc.center_height_offset = 0;
  vdc.overdraw_width_offset = 0;
  vdc.overdraw_height_offset = 0;
  //vdc.screen_width = 320;
  vdc.screen_width = 256;
  vdc.sat_screen_width = 320;
  vdc.screen_height = 240;

  memset(vdc.dirty_tiles, 0, 2048);
  memset(vdc.dirty_patterns, 0, 512);
  memset(vdc.vram, 0, 64 * 1024);
  memset(vdc.tile_cache, 0, 64 * 1024);
  memset(vdc.pattern_cache, 0, 64 * 1024);

  memset(vdc.sat, 0xFF, 64 * 8);
  update_satb(vdc.sat);

  memset(scanline_buffer, 0, 320 * 2);

  for(i = 0; i < 243; i++)
  {
    sat_cache.lines[i].num_active = 0;
  }

  clear_screen();
}

vce_struct vce;

u16 palette_convert[512];

void initialize_palette_convert()
{
  s32 color, r, g, b;
  s32 y, ry, by;
  double f_y, f_ry, f_by;
  double f_r, f_g, f_b;

  for(color = 0; color < 512; color++)
  {
    g = ((color >> 6) & 0x7) ;
    r = ((color >> 3) & 0x7) ;
    b = (color & 0x7) ;

    y = (r * 133) + (g * 261) + (b * 49);
    ry = (r * 214) + (g * -180) + (b * -34);
    by = (r * -72) + (g * -142) + (b * 214);

    y += 50;

    if(ry < 0)
      ry -= 50;
    else
      ry += 50;

    if(by < 0)
      by -= 50;
    else
      by += 50;

    y /= 100;
    ry /= 100;
    by /= 100;

    f_y = y / 31.0;
    f_ry = (ry / 15.0) * 0.701 * 0.75;
    f_by = (by / 15.0) * 0.886 * 0.75;

    f_r = f_y + f_ry;
    f_g = f_y - ((0.114 / 0.587) * f_by) - ((0.229 / 0.587) * f_ry);
    f_b = f_y + f_by;

    r = (u32)(floor(f_r * 31.0) + 0.5);
    b = (u32)(floor(f_b * 31.0) + 0.5);

    if(r < 0)
      r = 0;

    if(b < 0)
      b = 0;

    if(r > 31)
      r = 31;

    if(b > 31)
      b = 31;

#ifdef COLOR_RGB_555
    g = (u32)(floor(f_g * 31.0) + 0.5);

    if(g > 31)
      g = 31;
#else
    g = (u32)(floor(f_g * 63.0) + 0.5);

    if(g > 63)
      g = 63;
#endif

    if(g < 0)
      g = 0;

#ifdef COLOR_BGR_565
    palette_convert[color] = (b << 11) | (g << 5) | r;
#endif

#ifdef COLOR_RGB_565
    palette_convert[color] = (r << 11) | (g << 5) | b;
#endif

#ifdef COLOR_RGB_555
    palette_convert[color] = (r << 10) | (g << 5) | b;
#endif
  }
}

void vce_control_write(u32 value)
{
  vce.control = value;

  // Interlace bit determines how many scanlines there are.
  if(value & 0x4)
    vce.num_lines = 263;
  else
    vce.num_lines = 262;
}

void vce_address_write_low(u32 value)
{
  vce.palette_offset = (vce.palette_offset & 0xFF00) | value;
}

void vce_address_write_high(u32 value)
{
  vce.palette_offset = (vce.palette_offset & 0xFF) | ((value & 0x1) << 8);
}

u32 vce_data_read_low()
{
  return vce.palette[vce.palette_offset] & 0xFF;
}

u32 vce_data_read_high()
{
  u32 palette_entry = (vce.palette[vce.palette_offset] >> 8) | 0xFE;
  vce.palette_offset = (vce.palette_offset + 1) & 0x1FF;

  return palette_entry;
}

void vce_data_write_low(u32 value)
{
  u32 palette_entry = vce.palette[vce.palette_offset];
  palette_entry = (palette_entry & 0xFF00) | value;

  vce.palette[vce.palette_offset] = palette_entry;

  if(((vce.palette_offset & 0x0F) == 0) && (vce.palette_offset < 0x100))
  {
    if(vce.palette_offset == 0)
    {
      u32 i;
      u16 *palette_ptr = vce.palette_cache +
       (vce.palette_offset & 0x100);
      palette_entry = palette_convert[palette_entry];

      for(i = 0; i < 16; i++)
      {
        *palette_ptr = palette_entry;
        palette_ptr += 16;
      }
    }
  }
  else
  {
    if(vce.palette_offset == 0x100)
      vce.clear_edges = 1;

    vce.palette_cache[vce.palette_offset] =
     palette_convert[palette_entry];
  }
}

void vce_data_write_high(u32 value)
{
  u32 palette_entry = vce.palette[vce.palette_offset];
  palette_entry = (palette_entry & 0xFF) | ((value & 0x1) << 8);

  vce.palette[vce.palette_offset] = palette_entry;

  if(((vce.palette_offset & 0x0F) == 0) && (vce.palette_offset < 0x100))
  {
    if(vce.palette_offset == 0)
    {
      u32 i;
      u16 *palette_ptr = vce.palette_cache +
       (vce.palette_offset & 0x100);
      palette_entry = palette_convert[palette_entry];

      for(i = 0; i < 16; i++)
      {
        *palette_ptr = palette_entry;
        palette_ptr += 16;
      }
    }
  }
  else
  {
    vce.palette_cache[vce.palette_offset] =
     palette_convert[palette_entry];
  }

  vce.palette_offset = (vce.palette_offset + 1) & 0x1FF;
}

void reset_vce()
{
  vce.control = 0;
  vce.palette_offset = 0;
  vce.num_lines = 262;

  vce.frame_counter = 0;

  memset(vce.palette, 0, 512 * 2);
  memset(vce.palette_cache, 0, 512 * 2);
}

void initialize_vce()
{
}


void reset_video()
{
  reset_vdc();
  reset_vce();
}


void initialize_video()
{
  initialize_palette_convert();
  initialize_vdc();
  initialize_vce();

  vdc.center_width_offset = 0;
  vdc.center_height_offset = 0;
  vdc.overdraw_width_offset = 0;
  vdc.overdraw_height_offset = 0;
  vdc.screen_width = 320;
  vdc.screen_height = 240;

  set_screen_resolution(output_width, output_height);
}

void cache_tile(u32 tile_number)
{
  // Tiles consist of 2 16byte half-tiles contiguously in VRAM.
  // Each half-tile consists of 2 planes interleaved in 8 rows.

  u16 *vram_offset = vdc.vram + (tile_number * 16);
  u32 *tile_cache_offset = vdc.tile_cache + (tile_number * 8);
  u16 plane_row_a, plane_row_b;
  u32 output_row;
  u32 x, y;

  for(y = 0; y < 8; y++)
  {
    plane_row_a = vram_offset[y];
    plane_row_b = vram_offset[y + 8];
    // 16bits -> 8 pixels (32bits packed 4bpp)
    output_row = 0;
    for(x = 0; x < 8; x++)
    {
      output_row |= ((plane_row_a >> (7 - x)) & 0x1) << (x * 4);
      output_row |= ((plane_row_a >> ((7 - x) + 8)) & 0x1) << ((x * 4) + 1);
      output_row |= ((plane_row_b >> (7 - x)) & 0x1) << ((x * 4) + 2);
      output_row |= ((plane_row_b >> ((7 - x) + 8)) & 0x1) << ((x * 4) + 3);
    }

    tile_cache_offset[y] = output_row;
  }
}

void cache_pattern(u32 pattern_number)
{
  // Pattern consist of 4 32byte quarter-patterns contiguously in VRAM.
  // Each quarter-tile consists of 1 plane in 16 rows.
  u16 *vram_offset = vdc.vram + (pattern_number * 64);
  u32 *pattern_cache_offset = vdc.pattern_cache + (pattern_number * 32);
  u16 plane_row_a, plane_row_b, plane_row_c, plane_row_d;
  u32 output_row_a, output_row_b;
  u32 x, y;

  for(y = 0; y < 16; y++)
  {
    plane_row_a = vram_offset[y];
    plane_row_b = vram_offset[y + 16];
    plane_row_c = vram_offset[y + 32];
    plane_row_d = vram_offset[y + 48];

    // 16bits -> 16 pixels (32bits packed 4bpp)
    output_row_a = 0;
    for(x = 0; x < 8; x++)
    {
      output_row_a |= ((plane_row_a >> (15 - x)) & 0x1) << (x * 4);
      output_row_a |= ((plane_row_b >> (15 - x)) & 0x1) << ((x * 4) + 1);
      output_row_a |= ((plane_row_c >> (15 - x)) & 0x1) << ((x * 4) + 2);
      output_row_a |= ((plane_row_d >> (15 - x)) & 0x1) << ((x * 4) + 3);
    }

    output_row_b = 0;
    for(x = 0; x < 8; x++)
    {
      output_row_b |= ((plane_row_a >> (15 - (x + 8))) & 0x1) << (x * 4);
      output_row_b |= ((plane_row_b >> (15 - (x + 8))) & 0x1) << ((x * 4) + 1);
      output_row_b |= ((plane_row_c >> (15 - (x + 8))) & 0x1) << ((x * 4) + 2);
      output_row_b |= ((plane_row_d >> (15 - (x + 8))) & 0x1) << ((x * 4) + 3);
    }

    pattern_cache_offset[(y * 2)] = output_row_a;
    pattern_cache_offset[(y * 2) + 1] = output_row_b;
  }
}


void cache_tile_range(u32 tile_number_start, u32 tile_number_end)
{
  while(tile_number_start != tile_number_end)
  {
    cache_tile(tile_number_start);
    tile_number_start++;
  }
}

#define rgb16(r, g, b)                                                        \
  ((r << 11) | (g << 5) | b)                                                  \

#define grey16(c)                                                             \
  rgb16(c, (c * 2), c)                                                        \

u16 debug_palette[16] =
{
  grey16(0), grey16(1), grey16(2), grey16(3), grey16(4), grey16(5),
  grey16(6), grey16(7), grey16(8), grey16(9), grey16(10), grey16(11),
  grey16(12), grey16(13), grey16(14), grey16(15),
};

void display_tileset(u32 palette_number)
{
  u32 tile_number;
  u32 x, y;
  u32 x2, y2;
  u16 *screen_ptr;
  u32 screen_pitch;
  u32 *tile_ptr;
  u32 tile_row;

  set_screen_resolution(512, 256);
  screen_ptr = get_screen_ptr();
  screen_pitch = get_screen_pitch();

  palette_number <<= 4;

  for(y = 0, tile_number = 0; y < 32; y++)
  {
    for(x = 0; x < 64; x++, tile_number++)
    {
      if(vdc.dirty_tiles[tile_number])
      {
        cache_tile(tile_number);
        vdc.dirty_tiles[tile_number] = 0;
      }
      tile_ptr = vdc.tile_cache + (tile_number * 8);
      for(y2 = 0; y2 < 8; y2++)
      {
        tile_row = tile_ptr[y2];
        for(x2 = 0; x2 < 8; x2++)
        {
          screen_ptr[x2] =
           debug_palette[tile_row & 0xF];
           //vce.palette_cache[palette_number | (tile_row & 0xF)];
          tile_row >>= 4;
        }
        screen_ptr += screen_pitch;
      }
      screen_ptr -= (screen_pitch * 8);
      screen_ptr += 8;
    }
    screen_ptr -= (8 * 64);
    screen_ptr += (screen_pitch * 8);
  }
}

void display_tilemap(s32 palette_number_override)
{
  u32 tile_number, tile_offset;
  u32 palette_number = palette_number_override << 4;
  u32 bg_width = (((vdc.mwr >> 4) & 0x3) + 1) << 5;
  u32 bg_height = (((vdc.mwr >> 6) & 0x1) + 1) << 5;

  u32 x, y;
  u32 x2, y2;
  u16 *screen_ptr;
  u32 screen_pitch;
  u32 *tile_ptr;
  u32 tile_row;

  set_screen_resolution(bg_width * 8, bg_height * 8);
  screen_ptr = get_screen_ptr();
  screen_pitch = get_screen_pitch();

  for(y = 0, tile_offset = 0; y < bg_height; y++)
  {
    for(x = 0; x < bg_width; x++, tile_offset++)
    {
      tile_number = vdc.vram[tile_offset] & 0x7FF;

      if(palette_number_override == -1)
        palette_number = (vdc.vram[tile_offset] >> 12) << 4;

      if(vdc.dirty_tiles[tile_number])
      {
        cache_tile(tile_number);
        vdc.dirty_tiles[tile_number] = 0;
      }
      tile_ptr = vdc.tile_cache + (tile_number * 8);
      for(y2 = 0; y2 < 8; y2++)
      {
        tile_row = tile_ptr[y2];
        for(x2 = 0; x2 < 8; x2++)
        {
          screen_ptr[x2] =
           vce.palette_cache[palette_number | (tile_row & 0xF)];
          tile_row >>= 4;
        }
        screen_ptr += screen_pitch;
      }
      screen_ptr -= (screen_pitch * 8);
      screen_ptr += 8;
    }
    screen_ptr -= (8 * bg_width);
    screen_ptr += (screen_pitch * 8);
  }
}


void display_patternset(u32 palette_number)
{
  u32 pattern_number;
  u32 x, y;
  u32 x2, y2;
  u16 *screen_ptr;
  u32 screen_pitch;
  u32 *pattern_ptr;
  u32 pattern_half_row;

  set_screen_resolution(512, 256);
  screen_ptr = get_screen_ptr();
  screen_pitch = get_screen_pitch();

  palette_number <<= 4;

  for(y = 0, pattern_number = 0; y < 16; y++)
  {
    for(x = 0; x < 32; x++, pattern_number++)
    {
      if(vdc.dirty_patterns[pattern_number])
      {
        cache_pattern(pattern_number);
        vdc.dirty_patterns[pattern_number] = 0;
      }
      pattern_ptr = vdc.pattern_cache + (pattern_number * 32);
      for(y2 = 0; y2 < 16; y2++)
      {
        pattern_half_row = pattern_ptr[y2 * 2];
        for(x2 = 0; x2 < 8; x2++)
        {
          screen_ptr[x2] =
           vce.palette_cache[256 + (palette_number |
            (pattern_half_row & 0xF))];
          pattern_half_row >>= 4;
        }

        pattern_half_row = pattern_ptr[(y2 * 2) + 1];
        for(x2 = 0; x2 < 8; x2++)
        {
          screen_ptr[x2 + 8] =
           vce.palette_cache[256 + (palette_number |
           (pattern_half_row & 0xF))];
          pattern_half_row >>= 4;
        }

        screen_ptr += screen_pitch;
      }
      screen_ptr -= (screen_pitch * 16);
      screen_ptr += 16;
    }
    screen_ptr -= (16 * 32);
    screen_ptr += (screen_pitch * 16);
  }
}

#define render_pixel_direct()                                                 \
  *dest = vce.palette_cache[current_pixel | current_palette]                  \

#define render_pixel_indirect()                                               \
  *dest = current_pixel | current_palette                                     \

#define render_pixel_direct_spr_high()                                        \
  if(current_pixel)                                                           \
    *dest = vce.palette_cache[256 + (current_pixel | current_palette)]        \

#define render_pixel_indirect_spr_high()                                      \
  if(current_pixel)                                                           \
    *dest = (*dest & 0xFF) | ((current_pixel | current_palette) << 8)         \

#define render_pixel_indirect_spr_low()                                       \
  if(current_pixel)                                                           \
  {                                                                           \
    *dest &= 0xFF;                                                            \
    if((*dest & 0xF) == 0)                                                    \
      *dest |= ((current_pixel | current_palette) << 8);                      \
  }                                                                           \

#define render_pixel_collision_check_spr_high()                               \
  if(current_pixel)                                                           \
  {                                                                           \
    if(*dest & 0xFF00)                                                        \
      spr_collide = 1;                                                        \
                                                                              \
    *dest = (*dest & 0xFF) | ((current_pixel | current_palette) << 8);        \
  }                                                                           \

#define render_pixel_collision_check_spr_low()                                \
  if(current_pixel)                                                           \
  {                                                                           \
    if(*dest & 0xFF00)                                                        \
    {                                                                         \
      spr_collide = 1;                                                        \
      *dest &= 0xFF;                                                          \
    }                                                                         \
    if((*dest & 0xF) == 0)                                                    \
      *dest |= ((current_pixel | current_palette) << 8);                      \
  }                                                                           \


#define render_bg_get_bat_entry()                                             \
{                                                                             \
  u32 tile_number;                                                            \
  bat_entry = bat_offset[x_tile_offset];                                      \
  tile_number = bat_entry & 0x7FF;                                            \
                                                                              \
  x_tile_offset = (x_tile_offset + 1) & bg_width;                             \
  current_palette = (bat_entry >> 12) << 4;                                   \
  if(vdc.dirty_tiles[tile_number])                                            \
  {                                                                           \
    cache_tile(tile_number);                                                  \
    vdc.dirty_tiles[tile_number] = 0;                                         \
  }                                                                           \
  current_row = tile_base[tile_number * 8];                                   \
}                                                                             \


#define render_bg_row_pixels(count, render_type)                              \
  for(i = 0; i < (count); i++)                                                \
  {                                                                           \
    current_pixel = current_row & 0xF;                                        \
    render_pixel_##render_type();                                             \
    dest++;                                                                   \
                                                                              \
    current_row >>= 4;                                                        \
  }                                                                           \

#define render_bg_line_maker(render_type)                                     \
void render_bg_line_##render_type(u16 *dest)                                  \
{                                                                             \
  u32 bg_width = (vdc.mwr >> 4) & 0x3;                                        \
  u32 bg_height = (vdc.mwr >> 6) & 0x1;                                       \
  u32 screen_width = vdc.screen_width / 8;                                    \
  u32 i;                                                                      \
                                                                              \
  u32 bat_entry;                                                              \
  u32 current_palette;                                                        \
  u32 current_pixel;                                                          \
  u32 current_row;                                                            \
  u32 x_scroll = vdc.bxr + vdc.overdraw_width_offset;                         \
  u32 x_tile_offset = x_scroll / 8;                                           \
  u32 x_pixel_offset = x_scroll % 8;                                          \
  u32 y_tile_offset = vdc.effective_byr / 8;                                  \
  u32 y_pixel_offset = vdc.effective_byr % 8;                                 \
                                                                              \
  u16 *bat_offset;                                                            \
  u32 *tile_base = vdc.tile_cache + y_pixel_offset;                           \
                                                                              \
  if(bg_width == 2)                                                           \
    bg_width = 128;                                                           \
  else                                                                        \
    bg_width = (bg_width + 1) * 32;                                           \
                                                                              \
  bg_height = ((bg_height + 1) * 32) - 1;                                     \
                                                                              \
  y_tile_offset &= bg_height;                                                 \
                                                                              \
  bat_offset = vdc.vram + (y_tile_offset * bg_width);                         \
                                                                              \
  bg_width--;                                                                 \
                                                                              \
  x_tile_offset &= bg_width;                                                  \
                                                                              \
  if(x_pixel_offset)                                                          \
  {                                                                           \
    render_bg_get_bat_entry();                                                \
    current_row >>= (4 * x_pixel_offset);                                     \
    render_bg_row_pixels(8 - x_pixel_offset, render_type);                    \
    screen_width--;                                                           \
  }                                                                           \
                                                                              \
  while(screen_width)                                                         \
  {                                                                           \
    render_bg_get_bat_entry();                                                \
    render_bg_row_pixels(8, render_type);                                     \
    screen_width--;                                                           \
  }                                                                           \
                                                                              \
  if(x_pixel_offset)                                                          \
  {                                                                           \
    render_bg_get_bat_entry();                                                \
    render_bg_row_pixels(x_pixel_offset, render_type);                        \
  }                                                                           \
}                                                                             \

#ifdef ARM_ARCH

void render_bg_line_direct(u16 *dest);
void render_bg_line_indirect(u16 *dest);

#else

render_bg_line_maker(direct)
render_bg_line_maker(indirect)

#endif

sat_cache_struct sat_cache;

#define render_spr_half_row_pixels_no_flip(count, render_type, priority)      \
  for(i2 = 0; i2 < (count); i2++)                                             \
  {                                                                           \
    current_pixel = current_half_row & 0xF;                                   \
    render_pixel_##render_type##_spr_##priority();                            \
                                                                              \
    dest++;                                                                   \
    current_half_row >>= 4;                                                   \
  }                                                                           \

#define render_spr_half_row_pixels_flip(count, render_type, priority)         \
  for(i2 = 0; i2 < (count); i2++)                                             \
  {                                                                           \
    current_pixel = current_half_row >> 28;                                   \
    render_pixel_##render_type##_spr_##priority();                            \
                                                                              \
    dest++;                                                                   \
    current_half_row <<= 4;                                                   \
  }                                                                           \


#define render_spr_check_dirty(offset)                                        \
  if(vdc.dirty_patterns[(line_entry->pattern_offset + offset) / 32])          \
  {                                                                           \
    cache_pattern((line_entry->pattern_offset + offset) / 32);                \
    vdc.dirty_patterns[(line_entry->pattern_offset + offset) / 32] = 0;       \
  }                                                                           \

#define render_spr_row_ordered_normal(offset_a, offset_b, render_type,        \
 priority, flip_type)                                                         \
  render_spr_check_dirty(offset_a);                                           \
  current_half_row = vdc.pattern_cache[line_entry->pattern_offset +           \
   (offset_a)];                                                               \
  render_spr_half_row_pixels_##flip_type(8, render_type, priority);           \
                                                                              \
  current_half_row = vdc.pattern_cache[line_entry->pattern_offset +           \
   (offset_b)];                                                               \
  render_spr_half_row_pixels_##flip_type(8, render_type, priority)            \


#define render_spr_row_partial_shift_left_no_flip()                           \
  current_half_row >>= (8 - partial_x) * 4                                    \

#define render_spr_row_partial_shift_left_flip()                              \
  current_half_row <<= (8 - partial_x) * 4                                    \

#define render_spr_row_ordered_partial_left(offset_a, offset_b, render_type,  \
 priority, flip_type)                                                         \
  u32 partial_x = 16 + x;                                                     \
  render_spr_check_dirty(offset_a);                                           \
  if(partial_x > 8)                                                           \
  {                                                                           \
    partial_x -= 8;                                                           \
    current_half_row = vdc.pattern_cache[line_entry->pattern_offset +         \
     offset_a];                                                               \
    render_spr_row_partial_shift_left_##flip_type();                          \
    render_spr_half_row_pixels_##flip_type(partial_x, render_type,            \
     priority);                                                               \
    current_half_row = vdc.pattern_cache[line_entry->pattern_offset +         \
     offset_b];                                                               \
    render_spr_half_row_pixels_##flip_type(8, render_type, priority)          \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    current_half_row = vdc.pattern_cache[line_entry->pattern_offset +         \
     offset_b];                                                               \
    render_spr_row_partial_shift_left_##flip_type();                          \
    render_spr_half_row_pixels_##flip_type(partial_x, render_type,            \
     priority)                                                                \
  }                                                                           \


#define render_spr_row_ordered_partial_right(offset_a, offset_b, render_type, \
 priority, flip_type)                                                         \
  u32 partial_x = vdc.screen_width - x;                                       \
  render_spr_check_dirty(offset_a);                                           \
  current_half_row = vdc.pattern_cache[line_entry->pattern_offset +           \
   offset_a];                                                                 \
                                                                              \
  if(partial_x > 8)                                                           \
  {                                                                           \
    render_spr_half_row_pixels_##flip_type(8, render_type, priority);         \
    current_half_row = vdc.pattern_cache[line_entry->pattern_offset +         \
     offset_b];                                                               \
    render_spr_half_row_pixels_##flip_type(partial_x - 8, render_type,        \
     priority)                                                                \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    render_spr_half_row_pixels_##flip_type(partial_x, render_type,            \
     priority)                                                                \
  }                                                                           \

#define render_spr_row_wide_a_flip(type, render_type, priority)               \
  render_spr_row_ordered_##type(33, 32, render_type, priority, flip)          \

#define render_spr_row_wide_b_flip(type, render_type, priority)               \
  render_spr_row_ordered_##type(1, 0, render_type, priority, flip)            \

#define render_spr_row_wide_a_no_flip(type, render_type, priority)            \
  render_spr_row_ordered_##type(0, 1, render_type, priority, no_flip)         \

#define render_spr_row_wide_b_no_flip(type, render_type, priority)            \
  render_spr_row_ordered_##type(32, 33, render_type, priority,                \
   no_flip)                                                                   \


#define render_spr_wide_normal(render_type, priority, flip_type)              \
  render_spr_row_wide_a_##flip_type(normal, render_type, priority);           \
  render_spr_row_wide_b_##flip_type(normal, render_type, priority)            \

#define render_spr_wide_partial_left(render_type, priority, flip_type)        \
  render_spr_row_wide_a_##flip_type(partial_left, render_type, priority);     \
  render_spr_row_wide_b_##flip_type(normal, render_type, priority)            \

#define render_spr_wide_partial_right(render_type, priority, flip_type)       \
  render_spr_row_wide_a_##flip_type(normal, render_type, priority);           \
  x += 16;                                                                    \
  render_spr_row_wide_b_##flip_type(partial_right, render_type, priority)     \


#define render_spr_row_ordered(type, render_type, priority)                   \
  if(line_entry->attributes & SPRITE_ATTRIBUTE_HFLIP)                         \
  {                                                                           \
    if(line_entry->attributes & SPRITE_ATTRIBUTE_WIDE)                        \
    {                                                                         \
      render_spr_wide_##type(render_type, priority, flip);                    \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      render_spr_row_ordered_##type(1, 0, render_type, priority, flip);       \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    if(line_entry->attributes & SPRITE_ATTRIBUTE_WIDE)                        \
    {                                                                         \
      render_spr_wide_##type(render_type, priority, no_flip);                 \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      render_spr_row_ordered_##type(0, 1, render_type, priority, no_flip);    \
    }                                                                         \
  }                                                                           \


#define render_spr_row(render_type, priority)                                 \
  if(line_entry->attributes &                                                 \
   (SPRITE_ATTRIBUTE_CLIP_LEFT | SPRITE_ATTRIBUTE_CLIP_RIGHT))                \
  {                                                                           \
    if(line_entry->attributes & SPRITE_ATTRIBUTE_CLIP_LEFT)                   \
    {                                                                         \
      dest = base;                                                            \
      render_spr_row_ordered(partial_left, render_type, priority);            \
    }                                                                         \
    else                                                                      \
                                                                              \
    if(line_entry->attributes & SPRITE_ATTRIBUTE_CLIP_RIGHT)                  \
    {                                                                         \
      dest = base + x;                                                        \
      render_spr_row_ordered(partial_right, render_type, priority);           \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    dest = base + x;                                                          \
    render_spr_row_ordered(normal, render_type, priority);                    \
  }                                                                           \

#define render_spr_row_decision_direct()                                      \
  render_spr_row(direct, high)                                                \

#define render_spr_row_decision(type)                                         \
  if(line_entry->attributes & SPRITE_ATTRIBUTE_PRIORITY)                      \
  {                                                                           \
    render_spr_row(type, high);                                               \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    render_spr_row(type, low);                                                \
  }                                                                           \

#define render_spr_row_decision_indirect()                                    \
  render_spr_row_decision(indirect)                                           \


#define render_spr_prelude()                                                  \
  line_entry = sat_cache.lines[line].entries + i;                             \
  current_palette = (line_entry->attributes & 0xF) << 4                       \


#define render_spr_line_body(render_type, end_spr)                            \
  sat_cache_line_entry_struct *line_entry;                                    \
  u32 current_half_row;                                                       \
  u32 current_pixel;                                                          \
  u32 current_palette;                                                        \
  s32 i;                                                                      \
  u32 i2;                                                                     \
  u16 *dest;                                                                  \
  s32 x = 0;                                                                  \
                                                                              \
  if((vdc.cr & 0x2) && (sat_cache.lines[line].overflow))                      \
  {                                                                           \
    vdc.status |= VDC_STATUS_OVERFLOW_IRQ;                                    \
    raise_interrupt(IRQ_VDC);                                                 \
  }                                                                           \
                                                                              \
  for(i = sat_cache.lines[line].num_active - 1; i >= end_spr; i--)            \
  {                                                                           \
    render_spr_prelude();                                                     \
    x = line_entry->x;                                                        \
    render_spr_row_decision_##render_type();                                  \
  }                                                                           \


#define render_spr_line_maker(render_type)                                    \
void render_spr_line_##render_type(u16 *base, u32 line)                       \
{                                                                             \
  render_spr_line_body(render_type, 0);                                       \
}                                                                             \

#ifdef ARM_ARCH

void render_spr_line_direct(u16 *base, u32 line);
void render_spr_line_indirect(u16 *base, u32 line);

#else

render_spr_line_maker(direct)
render_spr_line_maker(indirect)

#endif



u32 render_spr_line_collision_check(u16 *base, u32 line)                      \
{                                                                             \
  u32 spr_collide = 0;                                                        \
  render_spr_line_body(indirect, 1);                                          \
                                                                              \
  render_spr_prelude();                                                       \
  x = line_entry->x;                                                          \
  render_spr_row_decision(collision_check);                                   \
                                                                              \
  return spr_collide;                                                         \
}                                                                             \



#define update_satb_add_entry_limit(line, offset, _x, _attributes)            \
  if(sat_cache.lines[line].num_present < 16)                                  \
  {                                                                           \
    line_entry = sat_cache.lines[line].entries +                              \
     sat_cache.lines[line].num_active;                                        \
    line_entry->x = _x;                                                       \
    line_entry->attributes = _attributes;                                     \
    line_entry->pattern_offset = (offset);                                    \
    sat_cache.lines[line].num_active++;                                       \
    sat_cache.lines[line].num_present++;                                      \
    sat_cache.lines[line].flags |= flags;                                     \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    sat_cache.lines[line].overflow = 1;                                       \
  }                                                                           \


#define _update_satb_add_entry_wide_limit(line, offset, offset_b, _x)         \
  if(sat_cache.lines[line].num_present < 15)                                  \
  {                                                                           \
    line_entry = sat_cache.lines[line].entries +                              \
     sat_cache.lines[line].num_active;                                        \
    line_entry->x = _x;                                                       \
    line_entry->attributes = attributes_wide;                                 \
    line_entry->pattern_offset = (offset);                                    \
    sat_cache.lines[line].num_active++;                                       \
    sat_cache.lines[line].num_present += 2;                                   \
    sat_cache.lines[line].flags |= flags;                                     \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    if(sat_cache.lines[line].num_present < 16)                                \
    {                                                                         \
      line_entry = sat_cache.lines[line].entries +                            \
       sat_cache.lines[line].num_active;                                      \
      line_entry->x = _x;                                                     \
      line_entry->attributes = attributes & ~SPRITE_ATTRIBUTE_WIDE;           \
      line_entry->pattern_offset = (offset);                                  \
      sat_cache.lines[line].num_active++;                                     \
      sat_cache.lines[line].num_present++;                                    \
      sat_cache.lines[line].flags |= flags;                                   \
    }                                                                         \
                                                                              \
    sat_cache.lines[line].overflow = 1;                                       \
  }                                                                           \


#define update_satb_add_offscreen_limit()                                     \
  if(sat_cache.lines[i2].num_present < 16)                                    \
    sat_cache.lines[i2].num_present++;                                        \
  else                                                                        \
    sat_cache.lines[i2].overflow = 1                                          \


#define update_satb_add_entry_no_limit(line, offset, _x, _attributes)         \
  line_entry = sat_cache.lines[line].entries +                                \
   sat_cache.lines[line].num_active;                                          \
  line_entry->x = _x;                                                         \
  line_entry->attributes = _attributes;                                       \
  line_entry->pattern_offset = (offset);                                      \
  sat_cache.lines[line].num_active++;                                         \
  sat_cache.lines[line].flags |= flags                                        \


#define _update_satb_add_entry_wide_no_limit(line, offset, offset_b, _x)      \
  line_entry = sat_cache.lines[line].entries +                                \
   sat_cache.lines[line].num_active;                                          \
  line_entry->x = _x;                                                         \
  line_entry->attributes = attributes_wide;                                   \
  line_entry->pattern_offset = (offset);                                      \
  sat_cache.lines[line].num_active++;                                         \
  sat_cache.lines[line].flags |= flags                                        \


#define update_satb_add_offscreen_no_limit()                                  \



#define pattern_offset_calculate_normal()                                     \
  pattern_offset_sub = pattern_offset + ((i2 / 16) * 64) + ((i2 % 16) * 2)    \

#define pattern_offset_calculate_flip()                                       \
  pattern_offset_sub = pattern_offset + (((height - i2 - 1) / 16) * 64) +     \
   (((height - i2 - 1) % 16) * 2)                                             \


#define update_satb_fill_sprite(direction, limit_check)                       \
{                                                                             \
  u32 pattern_offset_sub;                                                     \
  u32 iterations = height;                                                    \
  i2 = 0;                                                                     \
                                                                              \
  if(y < 0)                                                                   \
    i2 -= y;                                                                  \
                                                                              \
  if((y + height) > 263)                                                      \
    iterations = 263 - y;                                                     \
                                                                              \
  for(; i2 < iterations; i2++)                                                \
  {                                                                           \
    pattern_offset_calculate_##direction();                                   \
    update_satb_add_entry_##limit_check(i2 + y, pattern_offset_sub, x,        \
     attributes);                                                             \
  }                                                                           \
}                                                                             \

#define update_satb_add_entry_half(offset, x_off, _attributes, limit_check)   \
  update_satb_add_entry_##limit_check(i2 + y,                                 \
   pattern_offset_sub + offset, x + x_off, _attributes);                      \
  sat_cache.lines[i2 + y].num_present++                                       \

#define update_satb_add_entry_wide(pattern_offset, x_offset, limit_check)     \
  _update_satb_add_entry_wide_##limit_check(i2 + y, pattern_offset_sub,       \
   pattern_offset, x + x_offset)                                              \

#define update_satb_add_entry_double_normal_view_yes_no(limit_check)          \
  update_satb_add_entry_half(0, 0, attributes, limit_check)                   \

#define update_satb_add_entry_double_normal_view_no_yes(limit_check)          \
  update_satb_add_entry_half(32, 16, attributes_b, limit_check)               \

#define update_satb_add_entry_double_normal_view_yes_yes(limit_check)         \
  update_satb_add_entry_wide(0, 0, limit_check)                               \


#define update_satb_add_entry_double_flip_view_yes_no(limit_check)            \
  update_satb_add_entry_half(32, 0, attributes, limit_check)                  \

#define update_satb_add_entry_double_flip_view_no_yes(limit_check)            \
  update_satb_add_entry_half(0, 16, attributes_b, limit_check)                \

#define update_satb_add_entry_double_flip_view_yes_yes(limit_check)           \
  update_satb_add_entry_wide(32, 0, limit_check)                              \


#define update_satb_add_entry_double_normal(l_on, r_on, limit_check)          \
  update_satb_add_entry_double_normal_view_##l_on##_##r_on(limit_check)       \

#define update_satb_add_entry_double_flip(l_on, r_on, limit_check)            \
  update_satb_add_entry_double_flip_view_##l_on##_##r_on(limit_check)         \


#define update_satb_fill_sprite_double(dir, hflip, l_on, r_on, limit_check)   \
{                                                                             \
  u32 pattern_offset_sub;                                                     \
  u32 iterations = height;                                                    \
  i2 = 0;                                                                     \
                                                                              \
  if(y < 0)                                                                   \
    i2 -= y;                                                                  \
                                                                              \
  if((y + height) > 263)                                                      \
    iterations = 263 - y;                                                     \
                                                                              \
  for(; i2 < iterations; i2++)                                                \
  {                                                                           \
    pattern_offset_calculate_##dir();                                         \
    update_satb_add_entry_double_##hflip(l_on, r_on, limit_check);            \
  }                                                                           \
}                                                                             \

#define update_satb_fill_sprite_double_option(l_on, r_on, limit_check)        \
  if(attributes & SPRITE_ATTRIBUTE_VFLIP)                                     \
  {                                                                           \
    if(attributes & SPRITE_ATTRIBUTE_HFLIP)                                   \
    {                                                                         \
      update_satb_fill_sprite_double(flip, flip, l_on, r_on, limit_check);    \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      update_satb_fill_sprite_double(flip, normal, l_on, r_on, limit_check);  \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    if(attributes & SPRITE_ATTRIBUTE_HFLIP)                                   \
    {                                                                         \
      update_satb_fill_sprite_double(normal, flip, l_on, r_on, limit_check);  \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      update_satb_fill_sprite_double(normal, normal, l_on, r_on, limit_check);\
    }                                                                         \
  }                                                                           \

#ifdef DEBUGGER_ON

#define mask_sprite()                                                         \
  if(debug.sprite_mask & (1ULL << i))                                         \
    continue                                                                  \

#else

#define mask_sprite()                                                         \

#endif

#define update_satb_maker(limit_check)                                        \
void update_satb_##limit_check(u16 *satb_location)                            \
{                                                                             \
  u16 *current_sprite = satb_location;                                        \
  s32 x, y, width, height;                                                    \
  sat_cache_line_entry_struct *line_entry;                                    \
  u32 pattern_offset;                                                         \
  u32 pattern_number;                                                         \
  u32 cgx, cgy, attributes, attributes_b, attributes_wide;                    \
  u32 i, i2;                                                                  \
  u32 flags = 0;                                                              \
                                                                              \
  vdc.sat_screen_width = vdc.screen_width;                                    \
                                                                              \
  /* Cycle through each sprite and place it on the appropriate sat cache    */\
  /* line if it's visible and within 16 active (and increase                */\
                                                                              \
  for(i = 0; i < 263; i++)                                                    \
  {                                                                           \
    sat_cache.lines[i].flags = 0;                                             \
    sat_cache.lines[i].num_active = 0;                                        \
    sat_cache.lines[i].num_present = 0;                                       \
    sat_cache.lines[i].overflow = 0;                                          \
  }                                                                           \
                                                                              \
  for(i = 0; i < 64; i++, current_sprite += 4)                                \
  {                                                                           \
    mask_sprite();                                                            \
                                                                              \
    y = (current_sprite[0] & 0x3FF) - 64;                                     \
    x = (current_sprite[1] & 0x3FF) - 32 - vdc.overdraw_width_offset;         \
    attributes = current_sprite[3] & 0xB98F;                                  \
    cgx = ((attributes >> 8) & 0x1);                                          \
    cgy = ((attributes >> 12) & 0x3);                                         \
                                                                              \
    /* cgy 2 is not supposed to be valid, but really it's like 3            */\
    if(cgy == 2)                                                              \
      cgy = 3;                                                                \
                                                                              \
    width = (cgx + 1) * 16;                                                   \
    height = (cgy + 1) * 16;                                                  \
                                                                              \
    /* Only count sprites that fall on some scanline.                       */\
    if(((y + height) > 0) && (y < 263))                                       \
    {                                                                         \
      /* Sprites that are on the visible portion of the screen are added    */\
      /* to the list of sprites to draw; ones that aren't only increment    */\
      /* the number of sprites present on the line. This contributes to     */\
      /* the number of sprites which may be visible on a given line.        */\
                                                                              \
      if(((x + width) > 0) && (x < (s32)vdc.screen_width))                    \
      {                                                                       \
        pattern_number = (current_sprite[2] >> 1) & 0x3FF;                    \
                                                                              \
        pattern_number &= ~cgx;                                               \
                                                                              \
        if(cgy > 0)                                                           \
          pattern_number &= ~0x2;                                             \
                                                                              \
        if(cgy > 1)                                                           \
          pattern_number &= ~0x4;                                             \
                                                                              \
        if(pattern_number >= 0x200)                                           \
          continue;                                                           \
                                                                              \
        flags = (attributes & SPRITE_ATTRIBUTE_PRIORITY) == 0;                \
                                                                              \
        pattern_offset = (pattern_number * 32);                               \
                                                                              \
        if(current_sprite[2] & 0x1)                                           \
          attributes |= SPRITE_ATTRIBUTE_CG;                                  \
                                                                              \
        if(i == 0)                                                            \
          flags |= LINE_CACHE_FLAGS_SPR0;                                     \
                                                                              \
        if(cgx)                                                               \
        {                                                                     \
          attributes_b = attributes;                                          \
          if(x < 0)                                                           \
          {                                                                   \
            if(x <= -16)                                                      \
            {                                                                 \
              if(x < -16)                                                     \
                attributes_b |= SPRITE_ATTRIBUTE_CLIP_LEFT;                   \
                                                                              \
              attributes_b &= ~SPRITE_ATTRIBUTE_WIDE;                         \
              update_satb_fill_sprite_double_option(no, yes, limit_check);    \
              continue;                                                       \
            }                                                                 \
                                                                              \
            attributes |= SPRITE_ATTRIBUTE_CLIP_LEFT;                         \
          }                                                                   \
                                                                              \
          if((x + 32) >= vdc.screen_width)                                    \
          {                                                                   \
            if((x + 16) >= vdc.screen_width)                                  \
            {                                                                 \
              if((x + 16) > vdc.screen_width)                                 \
                attributes |= SPRITE_ATTRIBUTE_CLIP_RIGHT;                    \
                                                                              \
              attributes &= ~SPRITE_ATTRIBUTE_WIDE;                           \
              update_satb_fill_sprite_double_option(yes, no, limit_check);    \
              continue;                                                       \
            }                                                                 \
                                                                              \
            attributes_b |= SPRITE_ATTRIBUTE_CLIP_RIGHT;                      \
          }                                                                   \
                                                                              \
          attributes_wide = attributes | attributes_b;                        \
          update_satb_fill_sprite_double_option(yes, yes, limit_check);       \
        }                                                                     \
        else                                                                  \
        {                                                                     \
          if(x < 0)                                                           \
            attributes |= SPRITE_ATTRIBUTE_CLIP_LEFT;                         \
                                                                              \
          attributes &= ~SPRITE_ATTRIBUTE_CLIP_RIGHT;                         \
          if((x + width) >= vdc.screen_width)                                 \
            attributes |= SPRITE_ATTRIBUTE_CLIP_RIGHT;                        \
                                                                              \
          if(attributes & SPRITE_ATTRIBUTE_VFLIP)                             \
          {                                                                   \
            update_satb_fill_sprite(flip, limit_check);                       \
          }                                                                   \
          else                                                                \
          {                                                                   \
            update_satb_fill_sprite(normal, limit_check);                     \
          }                                                                   \
        }                                                                     \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        if(y < 0)                                                             \
        {                                                                     \
          height += y;                                                        \
          y = 0;                                                              \
        }                                                                     \
                                                                              \
        if((y + height) > 263)                                                \
          height = (263 - y);                                                 \
                                                                              \
        if(cgx)                                                               \
        {                                                                     \
          for(i2 = y; i2 < (height + y); i2++)                                \
          {                                                                   \
            update_satb_add_offscreen_##limit_check();                        \
            update_satb_add_offscreen_##limit_check();                        \
          }                                                                   \
        }                                                                     \
        else                                                                  \
        {                                                                     \
          for(i2 = y; i2 < (height + y); i2++)                                \
          {                                                                   \
            update_satb_add_offscreen_##limit_check();                        \
          }                                                                   \
        }                                                                     \
      }                                                                       \
    }                                                                         \
  }                                                                           \
}                                                                             \

update_satb_maker(limit)
update_satb_maker(no_limit)

void update_satb(u16 *satb_location)
{
  if(config.unlimit_sprites)
    update_satb_no_limit(satb_location);
  else
    update_satb_limit(satb_location);
}


#ifdef ARM_ARCH

void render_line_expand(u16 *dest, u16 *src);
void render_line_fill(u16 *dest, u32 color);
void render_line_scale_width_256(u16 *dest, u16 *src);
void render_line_expand_scale_width_256(u16 *dest, u16 *src);

#else

#define expand_pixel(_pixel)                                                  \
  if(_pixel & 0xFF00)                                                         \
    _pixel = 256 + (_pixel >> 8);                                             \
  _pixel = vce.palette_cache[_pixel]                                          \

void render_line_expand(u16 *dest, u16 *src)
{
  u32 i;
  u32 current_pixel;

  for(i = 0; i < vdc.screen_width; i++)
  {
    current_pixel = *src;
    expand_pixel(current_pixel);
    *dest = current_pixel;

    src++;
    dest++;
  }
}

void render_line_fill(u16 *dest, u32 color)
{
  u32 i;

  for(i = 0; i < vdc.screen_width; i++)
  {
    *dest = color;
    dest++;
  }
}

#define color_blend_25_75(dest, _a, _b)                                       \
  current_color = ((_a + (_b * 3)) >> 2) & g_rb_mask;                         \
  dest = current_color | (current_color >> 16)                                \

#define color_blend_50_50(dest, _a, _b)                                       \
  current_color = ((_a + _b) >> 1) & g_rb_mask;                               \
  dest = current_color | (current_color >> 16)                                \


#define color_blend_var(dest, _a, _b, a_coeff, b_coeff, shift)                \
  current_color = (((_a * a_coeff) + (_b * b_coeff)) >> shift) & g_rb_mask;   \
  dest = current_color | (current_color >> 16)                                \


void render_line_scale_width_256(u16 *dest, u16 *src)
{
  u32 i;
  u32 width = vdc.screen_width / 4;
  u32 color_a, color_b, color_c, color_d;
  u32 current_color;

  u32 rb_mask = 0x1F | (0x1F << 11);
  u32 g_mask = 0x3F << 5;
  u32 g_rb_mask = (g_mask << 16) | rb_mask;
  u32 color_a_expand, color_b_expand, color_c_expand, color_d_expand;

  for(i = 0; i < width; i++)
  {
    color_a = src[0];
    color_b = src[1];
    color_c = src[2];
    color_d = src[3];

    color_a_expand = (color_a | (color_a << 16)) & g_rb_mask;
    color_b_expand = (color_b | (color_b << 16)) & g_rb_mask;
    color_c_expand = (color_c | (color_c << 16)) & g_rb_mask;
    color_d_expand = (color_d | (color_d << 16)) & g_rb_mask;

    dest[0] = color_a;
    color_blend_25_75(dest[1], color_a_expand, color_b_expand);
    color_blend_50_50(dest[2], color_b_expand, color_c_expand);
    color_blend_25_75(dest[3], color_d_expand, color_c_expand);
    dest[4] = color_d;

    src += 4;
    dest += 5;
  }
}

/*

void render_line_scale_width_352(u16 *dest, u16 *src)
{
  u32 i;
  u32 width = vdc.screen_width / 4;
  u32 colors[16];
  u32 current_color;

  u32 rb_mask = 0x1F | (0x1F << 11);
  u32 g_mask = 0x3F << 5;
  u32 g_rb_mask = (g_mask << 16) | rb_mask;
  u32 color_a_expand, color_b_expand, color_c_expand, color_d_expand;

  for(i = 0; i < width; i++)
  {
    colors[0] = src[0];
    colors[1] = src[1];
    colors[2] = src[2];
    colors[3] = src[3];
    colors[4] = src[3];
    colors[5] = src[3];

    color_a_expand = (color_a | (color_a << 16)) & g_rb_mask;
    color_b_expand = (color_b | (color_b << 16)) & g_rb_mask;
    color_c_expand = (color_c | (color_c << 16)) & g_rb_mask;
    color_d_expand = (color_d | (color_d << 16)) & g_rb_mask;

    dest[0] = color_a;
    color_blend_25_75(dest[1], color_a_expand, color_b_expand);
    color_blend_50_50(dest[2], color_b_expand, color_c_expand);
    color_blend_25_75(dest[3], color_d_expand, color_c_expand);
    dest[4] = color_d;

    src += 4;
    dest += 5;
  }
}

*/

void render_line_expand_scale_width_256(u16 *dest, u16 *src)
{
  u32 i;
  u32 width = vdc.screen_width / 4;
  u32 color_a, color_b, color_c, color_d;
  u32 current_color;

  u32 rb_mask = 0x1F | (0x1F << 11);
  u32 g_mask = 0x3F << 5;
  u32 g_rb_mask = (g_mask << 16) | rb_mask;
  u32 color_a_expand, color_b_expand, color_c_expand, color_d_expand;

  rb_mask |= rb_mask << 16;
  g_mask |= g_mask << 16;

  for(i = 0; i < width; i++)
  {
    color_a = src[0];
    color_b = src[1];
    color_c = src[2];
    color_d = src[3];

    expand_pixel(color_a);
    expand_pixel(color_b);
    expand_pixel(color_c);
    expand_pixel(color_d);

    color_a_expand = (color_a | (color_a << 16)) & g_rb_mask;
    color_b_expand = (color_b | (color_b << 16)) & g_rb_mask;
    color_c_expand = (color_c | (color_c << 16)) & g_rb_mask;
    color_d_expand = (color_d | (color_d << 16)) & g_rb_mask;

    dest[0] = color_a;
    color_blend_25_75(dest[1], color_a_expand, color_b_expand);
    color_blend_50_50(dest[2], color_b_expand, color_c_expand);
    color_blend_25_75(dest[3], color_d_expand, color_c_expand);
    dest[4] = color_d;

    src += 4;
    dest += 5;
  }
}


#endif

#define adjust_dma_ptr_up(ptr)                                                \
  ptr = (ptr + 1) & 0x7FFF                                                    \

#define adjust_dma_ptr_down(ptr)                                              \
  ptr = (ptr - 1) & 0x7FFF                                                    \

#define vram_dma_loop(source_direction, dest_direction)                       \
  while(lenr >= 0)                                                            \
  {                                                                           \
    value = vdc.vram[sour];                                                   \
    vdc.vram[desr] = value;                                                   \
                                                                              \
    adjust_dma_ptr_##source_direction(sour);                                  \
    adjust_dma_ptr_##dest_direction(desr);                                    \
                                                                              \
    lenr--;                                                                   \
  }                                                                           \

#define update_tile_cache(start, end)                                         \
{                                                                             \
  u32 start_region = (start) / 16;                                            \
  u32 end_region = (end) / 16;                                                \
  u32 i;                                                                      \
                                                                              \
  for(i = start_region; i <= end_region; i++)                                 \
  {                                                                           \
    vdc.dirty_tiles[i] = 1;                                                   \
    vdc.dirty_patterns[i / 4] = 1;                                            \
  }                                                                           \
}                                                                             \
                                                                              \

void vram_dma_line(u32 scanline_cycles)
{
  u32 sour = vdc.sour;
  u32 desr = vdc.desr;
  s32 lenr = vdc.lenr;
  u32 value;

  if(lenr > (scanline_cycles / 6))
    lenr = scanline_cycles / 6;

  vdc.lenr -= lenr + 1;

  switch((vdc.dcr >> 2) & 0x3)
  {
    case 0:
      update_tile_cache(desr, desr + lenr);
      vram_dma_loop(up, up);
      break;

    case 1:
      update_tile_cache(desr, desr + lenr);
      vram_dma_loop(down, up);
      break;

    case 2:
      update_tile_cache(desr - lenr, desr);
      vram_dma_loop(up, down);
      break;

    case 3:
      update_tile_cache(desr - lenr, desr);
      vram_dma_loop(down, down);
      break;
  }

  vdc.desr = desr;
  vdc.sour = sour;

  if(vdc.lenr == -1)
  {
    vdc.vram_dma_trigger = 0;

    if(vdc.dcr & 0x2)
    {
      vdc.status |= VDC_STATUS_DMA_IRQ;
      raise_interrupt(IRQ_VDC);
    }
  }
}

void render_line(u32 start_line, u32 vblank_line)
{
  static u32 last_scale_width = 0;

  if((vce.frame_counter >= 14) &&
   (vce.frame_counter < (14 + output_height)))
  {
    u16 *dest = get_screen_ptr();
    u16 *_dest;
    u32 screen_line = vdc.raster_line;
    u32 scanline_line = vce.frame_counter - 14;
    u32 scale_width = 0;

    if(config.scale_width && ((vce.control & 0x3) == 0))
      scale_width = 1;

    if(scale_width != last_scale_width)
    {
      if(scale_width)
        output_width = 256;
      else
        output_width = 320;

      vdc.screen_width = (vdc.hdw + 1) * 8;
      update_screen_dimensions(width);
    }

    last_scale_width = scale_width;

#ifdef FRAMEBUFFER_PORTRAIT_ORIENTATION
    if((vdc.display_counter >= start_line) &&
     (vdc.display_counter < vblank_line) &&
     (vdc.burst_mode == 0))
    {
      // Draw scanline
      if((sat_cache.lines[screen_line].flags & LINE_CACHE_FLAGS_SPR0) &&
       (vdc.cr & 0x01))
      {
        if((vdc.cr & 0x40) && (config.spr_off == 0))
        {
          if((vdc.cr & 0x80) && (config.bg_off == 0))
            render_bg_line_indirect(scanline_buffer);
          else
            render_line_fill(scanline_buffer, 0);

          if(render_spr_line_collision_check(scanline_buffer, screen_line))
          {
            vdc.status |= VDC_STATUS_SPRITE_COLLISION_IRQ;
            raise_interrupt(IRQ_VDC);
          }
        }
        else
        {
          if((vdc.cr & 0x80) && (config.bg_off == 0))
            render_bg_line_indirect(scanline_buffer);
          else
            render_line_fill(scanline_buffer, 0);
        }
      }
      else
      {
        if((vdc.cr & 0x80) && (config.bg_off == 0))
          render_bg_line_indirect(scanline_buffer);
        else
          render_line_fill(scanline_buffer, 0);

        if((vdc.cr & 0x40) && (config.spr_off == 0))
          render_spr_line_indirect(scanline_buffer, screen_line);
      }
    }
    else
    {
      render_line_fill(scanline_buffer, 256);
    }

    if(scale_width & 0x1)
      render_line_expand_scale_width_256(dest, scanline_buffer);
    else
      render_line_expand(dest, scanline_buffer);
#else
    dest += (scanline_line * get_screen_pitch()) + vdc.center_width_offset;
    _dest = dest;

    if((vdc.display_counter >= start_line) &&
     (vdc.display_counter < vblank_line) &&
     (vdc.burst_mode == 0))
    {
      // Draw scanline
      if((sat_cache.lines[screen_line].flags & LINE_CACHE_FLAGS_SPR0) &&
       (vdc.cr & 0x01))
      {
        if((vdc.cr & 0x40) && (config.spr_off == 0))
        {
          scale_width |= 0x2;

          if((vdc.cr & 0x80) && (config.bg_off == 0))
            render_bg_line_indirect(scanline_buffer);
          else
            render_line_fill(scanline_buffer, 0);

          if(render_spr_line_collision_check(scanline_buffer, screen_line))
          {
            vdc.status |= VDC_STATUS_SPRITE_COLLISION_IRQ;
            raise_interrupt(IRQ_VDC);
          }
        }
        else
        {
          if(scale_width)
            dest = scanline_buffer;

          if((vdc.cr & 0x80) && (config.bg_off == 0))
            render_bg_line_direct(dest);
          else
            render_line_fill(dest, vce.palette_cache[0]);
        }
      }
      else

      if(sat_cache.lines[screen_line].flags & LINE_CACHE_FLAGS_LOW_PRIORITY)
      {
        if((vdc.cr & 0x40) && (config.spr_off == 0))
        {
          scale_width |= 0x2;

          if((vdc.cr & 0x80) && (config.bg_off == 0))
            render_bg_line_indirect(scanline_buffer);
          else
            render_line_fill(scanline_buffer, 0);

          render_spr_line_indirect(scanline_buffer, screen_line);
        }
        else
        {
          if(scale_width)
            dest = scanline_buffer;

          if((vdc.cr & 0x80) && (config.bg_off == 0))
            render_bg_line_direct(dest);
          else
            render_line_fill(dest, vce.palette_cache[0]);
        }
      }
      else
      {
        if(scale_width)
          dest = scanline_buffer;

        if((vdc.cr & 0x80) && (config.bg_off == 0))
          render_bg_line_direct(dest);
        else
          render_line_fill(dest, vce.palette_cache[0]);

        if((vdc.cr & 0x40) && (config.spr_off == 0))
          render_spr_line_direct(dest, screen_line);
      }
    }
    else
    {
      if(scale_width)
        dest = scanline_buffer;

      render_line_fill(dest, vce.palette_cache[256]);
    }

    if(scale_width & 0x1)
    {
      if(scale_width & 0x2)
        render_line_expand_scale_width_256(dest, scanline_buffer);
      else
        render_line_scale_width_256(_dest, dest);
    }
    else
    {
      if(scale_width & 0x2)
        render_line_expand(dest, scanline_buffer);
    }
#endif

    if((vdc.screen_width < vdc.scanline_widths[scanline_line])
     || (vce.clear_edges == 1))
    {
      vce.clear_edges = 0;
      clear_line_edges(scanline_line, vce.palette_cache[256],
       vdc.center_width_offset, vdc.screen_width);
    }

    vdc.scanline_widths[scanline_line] = vdc.screen_width;
  }
}

void update_frame(u32 skip)
{
  static u32 vds, vsw, vdw, vcr, vblank_line, start_line;
  static u32 check_idle_loop_line = 0;
  static u32 input_message_last;

  u32 vblank_active;

  s32 scanline_cycles = 1364;
  s32 hds_cycles = 0;

  // Check up to three RCRs for idle loops
  u32 check_idle_loops = 3;

  do
  {
    if((check_idle_loops) && (vce.frame_counter == check_idle_loop_line))
    {
      patch_idle_loop();
      check_idle_loops--;
    }

    if(vce.frame_counter == 0)
    {
      // vsr high
      vds = vdc.vds;
      // vsr low
      vsw = vdc.vsw;
      vdw = vdc.vdw;
      vcr = vdc.vcr;

      start_line = vds + vsw;
      vblank_line = start_line + vdw + 1;

      if(vblank_line > 261)
        vblank_line = 261;

      vdc.display_counter = 0;

      // If SPR/BG are both off at the first scanline then the VDC enters
      // burst mode, freeing it up from accessing memory for the entire
      // frame (so nothing is rendered, but you can do DMA)
      if(vdc.cr & 0xC0)
        vdc.burst_mode = 0;
      else
        vdc.burst_mode = 1;
    }

    vblank_active = 0;

    if(vdc.vram_dma_trigger &&
     (vdc.burst_mode || (vdc.display_counter < start_line) ||
     (vdc.display_counter >= vblank_line)))
    {
      vram_dma_line(scanline_cycles);
    }

    if(vdc.display_counter == vblank_line)
    {
      // Most games will be waiting before IRQs, so check here.
      if(check_idle_loops)
      {
        patch_idle_loop();
        check_idle_loops--;
      }

      vblank_active = 1;

      vdc.mwr = vdc.latched_mwr;

      if(vdc.satb_dma_trigger || (vdc.dcr & 0x10))
      {
        vdc.satb_dma_trigger = 0;
        vdc.satb_dma_irq_lines = 2;

        if(vdc.satb < 0x8000)
        {
          memcpy(vdc.sat, vdc.vram + vdc.satb, 512);
          update_satb(vdc.sat);
        }
      }
    }

    switch(vce.control & 0x3)
    {
      case 0:
        scanline_cycles = 1364;

        hds_cycles = ((0x27 - vdc.hdw) * 8) * 4;

        if(vdc.hds >= 0x9)
          hds_cycles += ((vdc.hds - 0x8) * 8) * 4;
        else

        if(vdc.hds >= 0x6)
          hds_cycles -= 0x15;

        break;

      case 1:
        scanline_cycles = 1365;
        hds_cycles = ((0x35 - vdc.hdw) * 8) * 3;
        if(vdc.hds >= 0xC)
          hds_cycles += (((vdc.hds - 0xB) * 8) - 0x7) * 3;
        else

        if(vdc.hds >= 0x7)
          hds_cycles -= 0x15;

        if(vdc.hds >= 0x12)
          hds_cycles += ((vdc.hds - 0x11) * 8) * 4;

        break;

      case 2:
        hds_cycles = ((0x4E - vdc.hdw) * 8) * 2;
        if(vdc.hds >= 0x12)
          hds_cycles += ((vdc.hds - 0x11) * 8) * 4;
        else

        if(vdc.hds >= 0x18)
          hds_cycles -= 0x15;

        break;

      case 3:
        if(vce.frame_counter & 1)
          scanline_cycles = 1366;
        else
          scanline_cycles = 1364;

        if(vdc.hds >= 0x12)
          hds_cycles += ((vdc.hds - 0x11) * 8) * 4;
        else

        if(vdc.hds >= 0xC)
          hds_cycles -= 0x15;

        break;
    }

    if(((s32)vdc.raster_line == ((s32)vdc.rcr - 0x40)) && (vdc.cr & 0x04))
    {
      // Sometimes idle loops happen before RCR IRQs, but we don't want to
      // do this constantly - just once per frame should be good.
      if(check_idle_loops != 0)
      {
        patch_idle_loop();
        check_idle_loops--;
      }

      vdc.status |= VDC_STATUS_RASTER_IRQ;
      raise_interrupt(IRQ_VDC);
    }

    //scanline_cycle_counter = cpu.global_cycles;

    if(hds_cycles < 0)
      hds_cycles = 0;
    else
      execute_instructions_timer(hds_cycles);

    if(vdc.display_counter == (vds + vsw))
      vdc.effective_byr = vdc.byr;
    else
      vdc.effective_byr++;

    if(!skip)
      render_line(start_line, vblank_line);

    if(vblank_active && (vdc.cr & 0x08))
    {
      // Execute the rest of the cycles. If a vlank IRQ is actually occuring
      // then this means we should execute 6 cycles first before actually
      // raising it (giving the game a chance to reset it)

      vdc.status |= VDC_STATUS_VBLANK_IRQ;

      execute_instructions_timer(6);
      hds_cycles += 6;

      if(vdc.status & VDC_STATUS_VBLANK_IRQ)
        raise_interrupt(IRQ_VDC);
    }

    // Active display
    execute_instructions_timer(scanline_cycles - hds_cycles);

    vdc.raster_line++;
    vdc.display_counter++;

    // End of scanline
    if(vdc.satb_dma_irq_lines)
    {
      vdc.satb_dma_irq_lines--;
      if(vdc.satb_dma_irq_lines == 0)
      {
        cpu.vdc_stalled = 0;

        if(vdc.dcr & 0x01)
        {
          vdc.status |= VDC_STATUS_SATB_IRQ;
          raise_interrupt(IRQ_VDC);
        }
      }
    }

    vce.frame_counter++;

    if(vce.frame_counter == vce.num_lines)
      vce.frame_counter = 0;

    if(vdc.display_counter == start_line)
      vdc.raster_line = 0;

    if(vdc.display_counter == (vds + vsw + vdw + 3 + vcr))
      vdc.display_counter = 0;

  } while(vce.frame_counter != vblank_line);

  set_font_wide();
  char print_buffer[128];

  if(config.fast_forward)
    print_string("--FF--", 0xFFFF, 0x000,
     320 - vdc.center_width_offset - (6 * 8), 0);

  if(netplay.type != NETPLAY_TYPE_NONE)
  {
    char print_buffer[128];
    sprintf(print_buffer, "latency: %d, (%1.2lf, %d) -> (%1.2lf, %d)",
     netplay.frame_latency, (double)netplay.period_latency /
     LATENCY_CALCULATION_PERIOD, netplay.period_stalls,
     (double)netplay.remote_latency_report / LATENCY_CALCULATION_PERIOD,
     netplay.remote_stalls_report);

    set_font_narrow();
    print_string(print_buffer, 0xFFFF, 0x000, vdc.center_width_offset, 0);
    set_font_wide();
  }

  if(config.show_fps)
  {
    static u32 frames = 0;
    static u32 fps = 60;
    const u32 frame_interval = 60;

    static u64 fps_timestamp;
    static u64 last_fps_timestamp = 0;

    frames++;

    if(frames == frame_interval)
    {
      u32 time_delta;

      get_ticks_us(&fps_timestamp);
      if(last_fps_timestamp == 0)
        time_delta = 1000000;
      else
        time_delta = fps_timestamp - last_fps_timestamp;
      last_fps_timestamp = fps_timestamp;

      fps = (u64)((u64)1000000 * (u64)frame_interval) / time_delta;

      frames = 0;
    }

    sprintf(print_buffer, "%02d/60", fps);
    print_string(print_buffer, 0xFFFF, 0x000, vdc.center_width_offset, 0);
  }

  if(config.status_message_counter)
  {
    u32 message_line = 228 - (config.status_message_lines * 8);
    u32 i;

    set_font_narrow();

    for(i = 0; i < config.status_message_lines; i++)
    {
      print_string(config.status_message[i], 0xFFFF, 0x000, 6, message_line);
      message_line += 8;  
    }

    config.status_message_counter--;
    if(config.status_message_counter == 0)
    {
      config.status_message_lines = 0;
      for(i = 228 - (STATUS_MESSAGE_LINES * 8); i < 228; i++)
      {
        clear_line_edges(i, vce.palette_cache[256], vdc.center_width_offset,
         vdc.screen_width);
      }
    }

    set_font_wide();
  }

  if(config.input_message)
  {
    set_font_narrow();
    print_string(config.input_message, 0xFFFF, 0x000, 6, 228);
    set_font_wide();
    input_message_last = 1;
  }
  else

  if(input_message_last)
  {
    u32 i;

    for(i = 228; i < 228 + 10; i++)
    {
      clear_line_edges(i, vce.palette_cache[256], vdc.center_width_offset,
       vdc.screen_width);
    }

    input_message_last = 0;
  }

  vce.frames_rendered++;

  check_idle_loop_line += 16;
  if(check_idle_loop_line >= vce.num_lines)
    check_idle_loop_line -= vce.num_lines;

  if(skip != 1)
    update_screen();
}

void dump_vram(u32 start, u32 size)
{
  u32 i, i2, offset;

  for(i = 0, offset = start; i < size / 8; i++)
  {
    for(i2 = 0; i2 < 8; i2++, offset++)
    {
      printf("%04x ", vdc.vram[offset]);
    }
    printf("\n");
  }
}

void dump_palette(u32 start, u32 size)
{
  u32 i, i2, offset;

  for(i = 0, offset = start; i < size / 8; i++)
  {
    for(i2 = 0; i2 < 8; i2++, offset++)
    {
      printf("%04x ", vce.palette[offset]);
    }
    printf("\n");
  }
}

void dump_spr(u32 start, u32 size)
{
  u32 i, offset;
  u32 spr_a, spr_b, spr_c, spr_d;

  char *ny = "ny";

  for(i = 0, offset = start; i < size; i++)
  {
    spr_a = vdc.sat[(i * 4)];
    spr_b = vdc.sat[(i * 4) + 1];
    spr_c = vdc.sat[(i * 4) + 2];
    spr_d = vdc.sat[(i * 4) + 3];

    printf("spr %x: (%02x %02x %02x %02x)\n", i, spr_a, spr_b, spr_c, spr_d);
    printf("  size: %d by %d\n", (((spr_d >> 8) & 0x1) + 1) * 16,
     (((spr_d >> 12) & 0x3) + 1) * 16);
    printf("  position: %d, %d\n", (spr_b & 0x3FF) - 32,
     (spr_a & 0x3FF) - 64);
    printf("  pattern: %x\n", (spr_c >> 1) & 0x1FF);
    printf("  palette: %x\n", spr_d & 0xF);
    printf("  hf: %c  vf %c  hp %c\n",
     ny[(spr_d & SPRITE_ATTRIBUTE_HFLIP) != 0],
     ny[(spr_d & SPRITE_ATTRIBUTE_VFLIP) != 0],
     ny[(spr_d & SPRITE_ATTRIBUTE_PRIORITY) != 0]);
    printf("\n");
  }
}

void dump_sprites_per_line(u32 line)
{
  printf("%d sprites on line %d\n",
   sat_cache.lines[line].num_present, line);
}

void dump_video_status()
{
  printf("vdc.status: %04x\n", vdc.status);
  printf("vdc.register_select: %04x\n", vdc.register_select);
  printf("vdc.mawr: %04x\n", vdc.mawr);
  printf("vdc.marr: %04x\n", vdc.marr);
  printf("vdc.cr: %04x\n", vdc.cr);
  printf("vdc.rcr: %04x\n", vdc.rcr);
  printf("vdc.mwr: %04x\n", vdc.mwr);
  printf("vdc.bxr: %04x\n", vdc.bxr);
  printf("vdc.byr: %04x\n", vdc.byr);
  printf("vdc.hds: %04x\n", vdc.hds);
  printf("vdc.hsw: %04x\n", vdc.hsw);
  printf("vdc.hdw: %04x\n", vdc.hdw);
  printf("vdc.hde: %04x\n", vdc.hde);
  printf("vdc.vds: %04x\n", vdc.vds);
  printf("vdc.vsw: %04x\n", vdc.vsw);
  printf("vdc.vdw: %04x\n", vdc.vdw);
  printf("vdc.dcr: %04x\n", vdc.dcr);
  printf("vdc.vcr: %04x\n", vdc.vcr);
  printf("vdc.sour: %04x\n", vdc.sour);
  printf("vdc.desr: %04x\n", vdc.desr);
  printf("vdc.lenr: %04x\n", vdc.lenr);
  printf("vdc.satb: %04x\n", vdc.satb);
  printf("vdc.write_latch: %04x\n", vdc.write_latch);
  printf("vdc.read_latch: %04x\n", vdc.read_latch);
  printf("vdc.raster_line: %04x\n", vdc.raster_line);
  printf("vdc.effective_byr: %04x\n", vdc.effective_byr);

  printf("vce.control: %04x\n", vce.control);
  printf("vce.frame_counter: %04x\n", vce.frame_counter);
  printf("vce.palette_offset: %04x\n", vce.palette_offset);
}

// Tile cache and pattern cache shouldn't be saved, so either make all the
// data dirty or recache it (the first option is probably better)

// Also, sat should be dealt with again upon load.

#define video_savestate_extra_load()                                          \
{                                                                             \
  u32 i, i2;                                                                  \
  u16 *palette_ptr = vce.palette;                                             \
  u16 *palette_cache_ptr = vce.palette_cache;                                 \
  u16 zero_entry;                                                             \
                                                                              \
  memset(vdc.dirty_tiles, 1, 2048);                                           \
  memset(vdc.dirty_patterns, 1, 512);                                         \
  update_satb(vdc.sat);                                                       \
                                                                              \
  zero_entry = palette_convert[palette_ptr[0]];                               \
                                                                              \
  for(i = 0; i < 16; i++)                                                     \
  {                                                                           \
    palette_cache_ptr[0] = zero_entry;                                        \
    for(i2 = 1; i2 < 16; i2++)                                                \
    {                                                                         \
      palette_cache_ptr[i2] = palette_convert[palette_ptr[i2]];               \
    }                                                                         \
    palette_ptr += 16;                                                        \
    palette_cache_ptr += 16;                                                  \
  }                                                                           \
                                                                              \
  palette_cache_ptr[0] = palette_convert[palette_ptr[0]];                     \
  for(i = 0; i < 16; i++)                                                     \
  {                                                                           \
    for(i2 = 1; i2 < 16; i2++)                                                \
    {                                                                         \
      palette_cache_ptr[i2] = palette_convert[palette_ptr[i2]];               \
    }                                                                         \
    palette_ptr += 16;                                                        \
    palette_cache_ptr += 16;                                                  \
  }                                                                           \
                                                                              \
  vdc.screen_width = (vdc.hdw + 1) * 8;                                       \
  update_screen_dimensions(width);                                            \
  for(i = 0; i < 240; i++)                                                    \
  {                                                                           \
    vdc.scanline_widths[i] = vdc.screen_width;                                \
  }                                                                           \
}                                                                             \

#define video_savestate_default_v1_load()                                     \
  vdc.latched_mwr = vdc.mwr                                                   \

#define video_savestate_default_v1_store()                                    \

#define video_savestate_extra_store()                                         \

#define video_savestate_builder(type, type_b, version_gate)                   \
void video_##type_b##_savestate(savestate_##type_b##_type savestate_file)     \
{                                                                             \
  file_##type##_array(savestate_file, vdc.vram);                              \
  file_##type##_array(savestate_file, vdc.sat);                               \
                                                                              \
  file_##type##_variable(savestate_file, vdc.status);                         \
  file_##type##_variable(savestate_file, vdc.register_select);                \
  file_##type##_variable(savestate_file, vdc.mawr);                           \
  file_##type##_variable(savestate_file, vdc.marr);                           \
  file_##type##_variable(savestate_file, vdc.cr);                             \
  file_##type##_variable(savestate_file, vdc.rcr);                            \
  file_##type##_variable(savestate_file, vdc.mwr);                            \
  file_##type##_variable(savestate_file, vdc.bxr);                            \
  file_##type##_variable(savestate_file, vdc.byr);                            \
  file_##type##_variable(savestate_file, vdc.hds);                            \
  file_##type##_variable(savestate_file, vdc.hsw);                            \
  file_##type##_variable(savestate_file, vdc.hde);                            \
  file_##type##_variable(savestate_file, vdc.hdw);                            \
  file_##type##_variable(savestate_file, vdc.vds);                            \
  file_##type##_variable(savestate_file, vdc.vsw);                            \
  file_##type##_variable(savestate_file, vdc.vdw);                            \
  file_##type##_variable(savestate_file, vdc.dcr);                            \
  file_##type##_variable(savestate_file, vdc.vcr);                            \
  file_##type##_variable(savestate_file, vdc.sour);                           \
  file_##type##_variable(savestate_file, vdc.desr);                           \
  file_##type##_variable(savestate_file, vdc.lenr);                           \
  file_##type##_variable(savestate_file, vdc.satb);                           \
                                                                              \
  file_##type##_variable(savestate_file, vdc.write_latch);                    \
  file_##type##_variable(savestate_file, vdc.read_latch);                     \
                                                                              \
  file_##type##_variable(savestate_file, vdc.raster_line);                    \
  file_##type##_variable(savestate_file, vdc.effective_byr);                  \
                                                                              \
  file_##type##_variable(savestate_file, vdc.screen_width);                   \
  file_##type##_variable(savestate_file, vdc.screen_height);                  \
                                                                              \
  file_##type##_variable(savestate_file, vdc.center_width_offset);            \
  file_##type##_variable(savestate_file, vdc.center_height_offset);           \
  file_##type##_variable(savestate_file, vdc.overdraw_width_offset);          \
  file_##type##_variable(savestate_file, vdc.overdraw_height_offset);         \
                                                                              \
  file_##type##_variable(savestate_file, vdc.increment_value);                \
                                                                              \
  file_##type##_variable(savestate_file, vdc.satb_dma_trigger);               \
  file_##type##_variable(savestate_file, vdc.satb_dma_irq_lines);             \
                                                                              \
  file_##type##_variable(savestate_file, vdc.vram_dma_trigger);               \
  file_##type##_variable(savestate_file, vdc.vram_dma_irq_lines);             \
                                                                              \
  file_##type##_variable(savestate_file, vdc.display_counter);                \
  file_##type##_variable(savestate_file, vdc.burst_mode);                     \
                                                                              \
  file_##type##_array(savestate_file, vce.palette);                           \
                                                                              \
  file_##type##_variable(savestate_file, vce.control);                        \
  file_##type##_variable(savestate_file, vce.palette_offset);                 \
  file_##type##_variable(savestate_file, vce.num_lines);                      \
                                                                              \
  file_##type##_variable(savestate_file, vce.frame_counter);                  \
                                                                              \
  if(version_gate >= 2)                                                       \
  {                                                                           \
    file_##type##_variable(savestate_file, vdc.latched_mwr);                  \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    video_savestate_default_v1_##type_b();                                    \
  }                                                                           \
  video_savestate_extra_##type_b();                                           \
}                                                                             \

build_savestate_functions(video);

