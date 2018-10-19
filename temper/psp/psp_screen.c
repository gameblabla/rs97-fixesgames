#include <pspctrl.h>

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>

#include <pspgu.h>
#include <psppower.h>
#include <psprtc.h>

static float *screen_vertex = (float *)0x441FC100;
static u32 *ge_cmd = (u32 *)0x441FC000;
static u16 *psp_gu_vram_base = (u16 *)(0x44000000);
static u32 *ge_cmd_ptr = (u32 *)0x441FC000;
static u32 ge_callback_id;
static u32 video_direct = 0;

static u32 __attribute__((aligned(16))) display_list[32];

#define PCE_SCREEN_WIDTH 320
#define PCE_SCREEN_HEIGHT 240

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
#define PSP_LINE_SIZE 512

#define GE_CMD_FBP    0x9C
#define GE_CMD_FBW    0x9D
#define GE_CMD_TBP0   0xA0
#define GE_CMD_TBW0   0xA8
#define GE_CMD_TSIZE0 0xB8
#define GE_CMD_TFLUSH 0xCB
#define GE_CMD_CLEAR  0xD3
#define GE_CMD_VTYPE  0x12
#define GE_CMD_BASE   0x10
#define GE_CMD_VADDR  0x01
#define GE_CMD_IADDR  0x02
#define GE_CMD_PRIM   0x04
#define GE_CMD_FINISH 0x0F
#define GE_CMD_SIGNAL 0x0C
#define GE_CMD_NOP    0x00

#define GE_CMD(cmd, operand)                                                \
  *ge_cmd_ptr = (((GE_CMD_##cmd) << 24) | (operand));                       \
  ge_cmd_ptr++                                                              \

static u16 *screen_texture = (u16 *)(0x04000000 + (512 * 272 * 2));
static u16 *current_screen_texture = (u16 *)(0x04000000 + (512 * 272 * 2));
static u16 *screen_pixels = (u16 *)(0x04000000 + (512 * 272 * 2));
static u32 screen_pitch = 320;

static void ge_finish_callback(int id, void *arg)
{
}


void set_screen_resolution(u32 width, u32 height)
{
  sceDisplaySetMode(0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);

  sceDisplayWaitVblankStart();
  sceDisplaySetFrameBuf((void *)psp_gu_vram_base, PSP_LINE_SIZE,
   PSP_DISPLAY_PIXEL_FORMAT_565, PSP_DISPLAY_SETBUF_NEXTFRAME);

  sceGuInit();

  sceGuStart(GU_DIRECT, display_list);
  sceGuDrawBuffer(GU_PSM_5650, (void *)0, PSP_LINE_SIZE);
  sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT,
   (void *)0, PSP_LINE_SIZE);
  sceGuClear(GU_COLOR_BUFFER_BIT);
  sceGuOffset(2048 - (PSP_SCREEN_WIDTH / 2), 2048 - (PSP_SCREEN_HEIGHT / 2));
  sceGuViewport(2048, 2048, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);

  sceGuScissor(0, 0, PSP_SCREEN_WIDTH + 1, PSP_SCREEN_HEIGHT + 1);
  sceGuEnable(GU_SCISSOR_TEST);

  u8 *weird_malloc = malloc(1024 * 4);
  free(weird_malloc);

  sceGuTexMode(GU_PSM_5650, 0, 0, GU_FALSE);
  sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);

  sceGuTexFilter(GU_LINEAR, GU_LINEAR);
  sceGuEnable(GU_TEXTURE_2D);

  sceGuFrontFace(GU_CW);
  sceGuDisable(GU_BLEND);

  sceGuFinish();
  sceGuSync(0, 0);

  sceDisplayWaitVblankStart();
  sceGuDisplay(GU_TRUE);

  PspGeCallbackData ge_callback;
  ge_callback.signal_func = NULL;
  ge_callback.signal_arg = NULL;
  ge_callback.finish_func = ge_finish_callback;
  ge_callback.finish_arg = NULL;
  ge_callback_id = sceGeSetCallback(&ge_callback);

  screen_vertex[0] = 0 + 0.5;
  screen_vertex[1] = 0 + 0.5;
  screen_vertex[2] = ((PSP_SCREEN_WIDTH - PCE_SCREEN_WIDTH) / 2) + 0.5;
  screen_vertex[3] = ((PSP_SCREEN_HEIGHT - PCE_SCREEN_HEIGHT) / 2) + 0.5;
  screen_vertex[4] = 0;
  screen_vertex[5] = PCE_SCREEN_WIDTH - 0.5;
  screen_vertex[6] = PCE_SCREEN_HEIGHT - 0.5;
  screen_vertex[7] = ((PSP_SCREEN_WIDTH - PCE_SCREEN_WIDTH) / 2) +
   PCE_SCREEN_WIDTH - 0.5;
  screen_vertex[8] = ((PSP_SCREEN_HEIGHT - PCE_SCREEN_HEIGHT) / 2) +
   PCE_SCREEN_HEIGHT - 0.5;
  screen_vertex[9] = 0;

  // Set framebuffer to PSP VRAM
  GE_CMD(FBP, ((u32)psp_gu_vram_base & 0x00FFFFFF));
  GE_CMD(FBW, (((u32)psp_gu_vram_base & 0xFF000000) >> 8) | PSP_LINE_SIZE);
  // Set texture 0 to the screen texture
  GE_CMD(TBP0, ((u32)screen_texture & 0x00FFFFFF));
  GE_CMD(TBW0, (((u32)screen_texture & 0xFF000000) >> 8) | PCE_SCREEN_WIDTH);
  // Set the texture size to 512 by 256 (2^8 by 2^9)
  GE_CMD(TSIZE0, (8 << 8) | 9);
  // Flush the texture cache
  GE_CMD(TFLUSH, 0);
  // Use 2D coordinates, no indeces, no weights, 32bit float positions,
  // 32bit float texture coordinates
  GE_CMD(VTYPE, (1 << 23) | (0 << 11) | (0 << 9) |
   (3 << 7) | (0 << 5) | (0 << 2) | 3);
  // Set the base of the index list pointer to 0
  GE_CMD(BASE, 0);
  // Set the rest of index list pointer to 0 (not being used)
  GE_CMD(IADDR, 0);
  // Set the base of the screen vertex list pointer
  GE_CMD(BASE, ((u32)screen_vertex & 0xFF000000) >> 8);
  // Set the rest of the screen vertex list pointer
  GE_CMD(VADDR, ((u32)screen_vertex & 0x00FFFFFF));
  // Primitive kick: render sprite (primitive 6), 2 vertices
  GE_CMD(PRIM, (6 << 16) | 2);
  // Done with commands
  GE_CMD(FINISH, 0);
  // Raise signal interrupt
  GE_CMD(SIGNAL, 0);
  GE_CMD(NOP, 0);
  GE_CMD(NOP, 0);
}

void update_screen()
{
  u32 *old_ge_cmd_ptr = ge_cmd_ptr;
  static u32 screen_flip = 0;

  // Render the current screen
  ge_cmd_ptr = ge_cmd + 2;
  GE_CMD(TBP0, ((u32)screen_pixels & 0x00FFFFFF));
  GE_CMD(TBW0, (((u32)screen_pixels & 0xFF000000) >> 8) |
   PCE_SCREEN_WIDTH);
  ge_cmd_ptr = old_ge_cmd_ptr;

  sceKernelDcacheWritebackAll();
  sceGeListEnQueue(ge_cmd, ge_cmd_ptr, ge_callback_id, NULL);

  // Flip to the next screen
  screen_flip ^= 1;

  if(screen_flip)
    screen_pixels = screen_texture + (320 * 240 * 2);
  else
    screen_pixels = screen_texture;
}

void *get_screen_ptr()
{
  return screen_pixels;
}

u32 get_screen_pitch()
{
  return 320;
}

void clear_screen()
{
  u32 i;
  u32 pitch = get_screen_pitch();
  u16 *pixels = get_screen_ptr();

  for(i = 0; i < 240; i++)
  {
    memset(pixels, 0, 320 * 2);
    pixels += pitch;
  }
}

void clear_line_edges(u32 line_number, u32 color, u32 edge, u32 middle)
{
  u32 *dest = (u32 *)((u16 *)get_screen_ptr() +
   (line_number * get_screen_pitch()));
  u32 i;

  color |= (color << 16);

  edge /= 2;
  middle /= 2;

  for(i = 0; i < edge; i++)
  {
    *dest = color;
    dest++;
  }

  // Marker for counting pixels

  dest += middle;

  for(i = 0; i < edge; i++)
  {
    *dest = color;
    dest++;
  }
}


