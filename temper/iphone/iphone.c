#include "../common.h"
#include <fcntl.h>
#include <pthread.h>

float __audioVolume = 1.0;
int soundBufferSize = 0;
int soundInit = 0;
unsigned long gp2x_pad_status = 0;
int __saved = 0;
char link_filename[1024];
char save_filename[1024];
char rom_filename[1024];
int SaveFrames = 0;

u32 iphone_joystick_read()
{
  return gp2x_pad_status;
}

void iphone_initialize_screen()
{
}

u16 *iphone_get_screen_ptr()
{
  return BaseAddress;
}

u32 iphone_get_screen_pitch()
{
  return (320 * 2);
}

void iphone_update_screen()
{
  static int updateFrames = 0;
  if(iphone_layout < 2)
  {
    // Portrait
    updateFrames++;
    if(updateFrames >= 2+ (iphone_touches > 1 ? 1 : 0))
    {
      updateFrames = 0;
      updateScreen();
    }
  }
  else
  {
    // Ugh landscape!
    updateFrames++;
    if(updateFrames >= 2+(iphone_touches+1))
    {
      updateFrames = 0;
      updateScreen();
    }
  }
  if(__autosave && ++SaveFrames >= 300)
  {
    char tmpfilename[1024];
    sprintf(tmpfilename, "%s-last-autosave.svs", rom_filename);
    save_state(tmpfilename, NULL);
    SaveFrames = 0;
  }

  if(!__emulation_run || (__emulation_saving != 0))
  {
    char bram_path[MAX_PATH];
    char buffer[512];
    char tmpfilename[512];
    time_t curtime;
    struct tm *loctime;

    if(__emulation_saving != 0)
    {
      if((__saved != 0) && (__emulation_saving == 2))
      {
        sprintf(tmpfilename, "%s", save_filename);
      }
      else
      {
        curtime = time (NULL);
        loctime = localtime (&curtime);
        strftime (buffer, 260, "%y%m%d-%I%M%S", loctime);
        sprintf(tmpfilename, "%s-%s.svs", rom_filename, buffer);
      }
      save_state(tmpfilename, NULL);
    }

    get_bram_path(bram_path);
    save_bram(bram_path);

    chdir(config.main_path);

    save_directory_config_file("temper.cf2");
    audio_exit();

    if(link_filename[0] != '\0')
    {
      unlink(link_filename);
      link_filename[0] = '\0';
    }
    __emulation_saving = 0;
    pthread_exit(NULL);
  }
}

void iphone_clear_line_edges(u32 line_number, u32 color, u32 edge, u32 middle)
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

  dest += middle;

  for(i = 0; i < edge; i++)
  {
    *dest = color;
    dest++;
  }
}

void iphone_sound_volume(s32 volume_change)
{
  __audioVolume += (float)volume_change / 100.0;

  if(__audioVolume < 0)
    __audioVolume = 0;

  if(__audioVolume > 1.0)
    __audioVolume = 1.0;
}

void iphone_initialize()
{
  chdir("/var/mobile/Media/ROMs/TEMPER");

  SaveFrames = 0;
  __saved = 0;
  config.enable_sound = (iphone_soundon ? 1: 0);
  config.show_fps = (gp2x_fps_debug ? 1: 0);

  config.fast_forward = 0;

  if(tArgc > 2)
  {
    if((!strcasecmp(tArgv[1] + (strlen(tArgv[1]) - 4), ".svs")))
    {
      unsigned long pos;
      char cmdname[1024];
      sprintf(save_filename, "%s", tArgv[1]);
      pos = strlen(tArgv[1]) - 18;
      tArgv[1][pos] = 0;
      sprintf(rom_filename, "%s", tArgv[1]);
      sprintf(link_filename, "/var/mobile/Media/ROMs/TEMPER/save_states/%s",
       rom_filename);
      *(char *)(strrchr(link_filename, '.')) = 0;
      sprintf(link_filename, "%s_0.svs", link_filename);
      unlink(link_filename);
      sprintf(cmdname,
       "cp \"/var/mobile/Media/ROMs/TEMPER/save_states/%s\" \"%s\"",
       save_filename, link_filename);
      system(cmdname);
      __saved = 1;
    }
  }
  else
  {
    sprintf(rom_filename, "%s", tArgv[1]);
  }
}

void iphone_quit()
{
  pthread_exit(NULL);
}

