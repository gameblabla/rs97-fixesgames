#ifndef VIDEO_H
#define VIDEO_H

typedef enum
{
  SPRITE_ATTRIBUTE_CLIP_LEFT  = 0x0010,
  SPRITE_ATTRIBUTE_CLIP_RIGHT = 0x0020,
  SPRITE_ATTRIBUTE_PRIORITY   = 0x0080,
  SPRITE_ATTRIBUTE_WIDE       = 0x0100,
  SPRITE_ATTRIBUTE_CG         = 0x0200,
  SPRITE_ATTRIBUTE_HFLIP      = 0x0800,
  SPRITE_ATTRIBUTE_VFLIP      = 0x8000
} sprite_attribute_enum;

typedef enum
{
  NO_DEBUG,
  DEBUG_TILES,
  DEBUG_PATTERNS
} video_debug_mode_enum;

typedef enum
{
  VDC_STATUS_SPRITE_COLLISION_IRQ = 0x01,
  VDC_STATUS_OVERFLOW_IRQ         = 0x02,
  VDC_STATUS_RASTER_IRQ           = 0x04,
  VDC_STATUS_SATB_IRQ             = 0x08,
  VDC_STATUS_DMA_IRQ              = 0x10,
  VDC_STATUS_VBLANK_IRQ           = 0x20,
  VDC_STATUS_BUSY                 = 0x40
} vdc_status_enum;

typedef enum
{
  LINE_CACHE_FLAGS_LOW_PRIORITY = 0x1,
  LINE_CACHE_FLAGS_SPR0         = 0x2
} line_cache_flags_enum;

// 41B + 64KB + 512B + 128KB + 3040B
// 192KB + 3593B

typedef struct
{
  u16 vram[1024 * 32];
  u16 sat[64 * 4];

  u32 status;
  u32 register_select;
  u32 mawr;
  u32 marr;
  u32 cr;
  u32 rcr;
  u32 mwr;
  u32 bxr;
  u32 byr;
  u32 hds;
  u32 hsw;
  u32 hde;
  u32 hdw;
  u32 vds;
  u32 vsw;
  u32 vdw;
  u32 dcr;
  u32 vcr;
  u32 sour;
  u32 desr;
  u32 lenr;
  u32 satb;

  u32 write_latch;
  u32 read_latch;

  u32 raster_line;
  u32 effective_byr;
  u32 latched_mwr;

  u32 screen_width;
  u32 screen_height;

  u32 center_width_offset;
  u32 center_height_offset;
  u32 overdraw_width_offset;
  u32 overdraw_height_offset;

  u32 increment_value;

  u32 satb_dma_trigger;
  s32 satb_dma_irq_lines;

  u32 vram_dma_trigger;
  s32 vram_dma_irq_lines;

  u32 display_counter;
  u32 burst_mode;

  // Don't save these in a savestate.
  u32 sat_screen_width;

  u32 tile_cache[1024 * 16];
  u32 pattern_cache[1024 * 16];

  u8 dirty_tiles[2048];
  u8 dirty_patterns[512];
  u16 scanline_widths[240];
} vdc_struct;

// 6 bytes, if we make it so with GCC

typedef struct __attribute__ ((__packed__))
{
  u16 pattern_offset;
  s16 x;
  u16 attributes;
} sat_cache_line_entry_struct;

// 388 (0x184) bytes
typedef struct __attribute__ ((__packed__))
{
  // entries: 384 (0x180) bytes
  sat_cache_line_entry_struct entries[64];
  u8 flags;
  u8 num_active;
  u8 num_present;
  u8 overflow;
} sat_cache_line_struct;

typedef struct
{
  sat_cache_line_struct lines[263];
} sat_cache_struct;

typedef struct
{
  u16 palette[512];

  // Can be recalculated, don't save in save state.
  u16 palette_cache[512];

  u32 control;
  u32 palette_offset;
  u32 num_lines;

  u32 frame_counter;

  // Inpersistent/debug info, don't save in save state.
  u32 clear_edges;
  u32 frames_rendered;
} vce_struct;

extern video_debug_mode_enum video_debug_mode;

extern vdc_struct vdc;
extern vce_struct vce;
extern sat_cache_struct sat_cache;

u32 vdc_status();
u32 vdc_data_read_low();
u32 vdc_data_read_high();

void vdc_register_select(u32 value);
void vdc_data_write_low(u32 value);
void vdc_data_write_high(u32 value);
void reset_video();
void initialize_video();

void vce_control_write(u32 value);
void vce_address_write_low(u32 value);
void vce_address_write_high(u32 value);
u32 vce_data_read_low();
u32 vce_data_read_high();
void vce_data_write_low(u32 value);
void vce_data_write_high(u32 value);
void initialize_vce();

void update_frame();

void render_bg_lines(u8 *dest, u32 start_y, u32 end_y);

void dump_vram(u32 start, u32 size);
void dump_palette(u32 start, u32 size);
void dump_video_status();

void dump_spr(u32 start, u32 size);
void dump_sprites_per_line(u32 line);

void video_load_savestate(savestate_load_type savestate_file);
void video_store_savestate(savestate_store_type savestate_file);

#endif

