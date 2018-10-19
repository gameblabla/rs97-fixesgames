#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#define DIR_SEPARATOR_CHAR '\\'
#define DIR_SEPARATOR_CHAR_STR "\\"

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;
typedef unsigned long long int u64;
typedef signed char s8;
typedef signed short int s16;
typedef signed long int s32;
typedef signed long long int s64;

typedef enum
{
  TRACK_FILE_TYPE_BINARY,
  TRACK_FILE_TYPE_WAVE,
} track_file_type_enum;

typedef struct
{
  u32 file_number;
  u32 physical_offset;

  u32 sector_offset;
  u32 sector_count;
  u32 pregap_offset;

  u32 sector_size;
  u32 format_type;
} cd_track_struct;

typedef struct
{
  track_file_type_enum type;
  FILE *file_handle;

  u32 current_offset;
} cd_track_file_struct;

typedef struct
{
  FILE *bin_file;
  cd_track_file_struct track_files[100];
  u32 num_files;

  s32 first_track;
  s32 last_track;
  u32 num_physical_tracks;
  u32 num_sectors;
  s32 last_seek_track;

  cd_track_struct physical_tracks[100];
  cd_track_struct *logical_tracks[100];
} cd_bin_struct;


cd_bin_struct cd_bin;

char *skip_whitespace(char *str)
{
  while(*str == ' ')
    str++;

  return str;
}

s32 load_bin_cue(char *cue_file_name)
{
  FILE *cue_file = fopen(cue_file_name, "rb");

  printf("loading cue file %s\n", cue_file_name);

  if(cue_file)
  {
    char line_buffer[256];
    char *line_buffer_ptr;

    char bin_file_name[MAX_PATH];
    char *separator_pos;
    s32 current_physical_track_number = -1;
    u32 current_physical_offset;
    u32 current_pregap = 0;
    u32 bin_file_size;

    cd_track_struct *current_physical_track = NULL;

    u32 i;

    // First, get filename. Only support binary right now.
    fgets(line_buffer, 255, cue_file);
    separator_pos = strchr(line_buffer, ' ');
    // Now find the first non-space.
    separator_pos = skip_whitespace(separator_pos);
    // Now see if what's there is a quote.
    if(*separator_pos == '"')
    {
      strcpy(bin_file_name, separator_pos + 1);
      separator_pos = strrchr(bin_file_name, '"');
      *(separator_pos) = 0;
    }
    else
    {
      // Otherwise go to the next space.
      strcpy(bin_file_name, separator_pos);
      separator_pos = strrchr(bin_file_name, ' ');
      *(separator_pos) = 0;
    }

    // Might have to change directory first.
    separator_pos = strrchr(cue_file_name, DIR_SEPARATOR_CHAR);

    if(separator_pos)
    {
      char current_dir[MAX_PATH];
      getcwd(current_dir, MAX_PATH);

      *separator_pos = 0;

      chdir(cue_file_name);

#ifdef GP2X_OR_WIZ_BUILD
      cd_bin.bin_file = open(bin_file_name, O_RDONLY);
#else
      cd_bin.bin_file = fopen(bin_file_name, "rb");
#endif

      printf("loaded bin file %s (%p)\n", bin_file_name, cd_bin.bin_file);

      *separator_pos = DIR_SEPARATOR_CHAR;
      chdir(current_dir);
    }
    else
    {
#ifdef GP2X_OR_WIZ_BUILD
      cd_bin.bin_file = open(bin_file_name, O_RDONLY);
#else
      cd_bin.bin_file = fopen(bin_file_name, "rb");
#endif
    }

    for(i = 0; i < 100; i++)
    {
      cd_bin.logical_tracks[i] = NULL;
    }

    cd_bin.first_track = -1;
    cd_bin.last_track = -1;
    cd_bin.num_physical_tracks = 0;
    cd_bin.num_sectors = 0;

    // Get line
    while(fgets(line_buffer, 256, cue_file))
    {
      // Skip trailing whitespace
      line_buffer_ptr = skip_whitespace(line_buffer);

      // Dirty, but should work - switch on first character.
      switch(line_buffer_ptr[0])
      {
        // New track number
        case 'T':
        {
          u32 new_track_number;
          char track_type[64];

          sscanf(line_buffer_ptr, "TRACK %d %s", &new_track_number,
           track_type);

          current_physical_track_number++;
          current_physical_track =
           cd_bin.physical_tracks + current_physical_track_number;

          current_physical_track->sector_size = 2352;

          if(!strcmp(track_type, "AUDIO"))
          {
            current_physical_track->format_type = 0;
            current_physical_track->sector_size = 2352;
          }

          if(!strcmp(track_type, "MODE1/2352"))
          {
            current_physical_track->format_type = 4;
            current_physical_track->sector_size = 2352;
          }

          if(!strcmp(track_type, "MODE1/2048"))
          {
            current_physical_track->format_type = 4;
            current_physical_track->sector_size = 2048;
          }

          cd_bin.logical_tracks[new_track_number] = current_physical_track;
          cd_bin.num_physical_tracks++;

          if((cd_bin.first_track == -1) ||
           (new_track_number < cd_bin.first_track))
          {
            cd_bin.first_track = new_track_number;
          }

          if((cd_bin.last_track == -1) ||
           (new_track_number > cd_bin.last_track))
          {
            cd_bin.last_track = new_track_number;
          }

          break;
        }

        // Pregap
        case 'P':
        {
          u32 minutes, seconds, frames;

          sscanf(line_buffer_ptr, "PREGAP %d:%d:%d", &minutes,
           &seconds, &frames);

          current_pregap += frames + (seconds * 75) + (minutes * 75 * 60);
          break;
        }

        // Index
        case 'I':
        {
          u32 index_number;
          u32 minutes, seconds, frames;
          u32 sector_offset;

          sscanf(line_buffer_ptr, "INDEX %d %d:%d:%d", &index_number,
           &minutes, &seconds, &frames);

          sector_offset = frames + (seconds * 75) + (minutes * 75 * 60);

          if(index_number == 1)
          {
            current_physical_track->pregap_offset = current_pregap;
            current_physical_track->sector_offset = sector_offset;
          }

          break;
        }
      }
    }

    current_physical_offset = 0;

    for(i = 0; i < cd_bin.num_physical_tracks - 1; i++)
    {
      cd_bin.physical_tracks[i].sector_count =
       cd_bin.physical_tracks[i + 1].sector_offset -
       cd_bin.physical_tracks[i].sector_offset;

      cd_bin.physical_tracks[i].physical_offset = current_physical_offset;
      current_physical_offset += (cd_bin.physical_tracks[i].sector_count *
       cd_bin.physical_tracks[i].sector_size);

      cd_bin.physical_tracks[i].sector_offset +=
       cd_bin.physical_tracks[i].pregap_offset;

      cd_bin.num_sectors += cd_bin.physical_tracks[i].sector_count;
    }

#ifdef GP2X_OR_WIZ_BUILD
    bin_file_size = lseek(cd_bin.bin_file, 0, SEEK_END);
    lseek(cd_bin.bin_file, 0, SEEK_SET);
#else
    fseek(cd_bin.bin_file, 0, SEEK_END);
    bin_file_size = ftell(cd_bin.bin_file);
    fseek(cd_bin.bin_file, 0, SEEK_SET);
#endif

    // Set the last track data
    cd_bin.physical_tracks[i].physical_offset = current_physical_offset;
    cd_bin.physical_tracks[i].sector_offset +=
     cd_bin.physical_tracks[i].pregap_offset;
    cd_bin.physical_tracks[i].sector_count =
     (bin_file_size - current_physical_offset) /
     cd_bin.physical_tracks[i].sector_size;

    cd_bin.num_sectors += cd_bin.physical_tracks[i].sector_count;

    printf("finished loading cue %s\n", cue_file_name);
    printf("bin file: %s (%p)\n", bin_file_name, cd_bin.bin_file);
    printf("first track: %d, last track: %d\n", cd_bin.first_track,
     cd_bin.last_track);

    for(i = cd_bin.first_track; i <= cd_bin.last_track; i++)
    {
      printf("track %d (%p):\n", i, cd_bin.logical_tracks[i]);
      if(cd_bin.logical_tracks[i] == NULL)
      {
        printf("  (invalid)\n");
      }
      else
      {
        printf("  physical offset 0x%x\n",
         cd_bin.logical_tracks[i]->physical_offset);
        printf("  sector offset 0x%x\n",
         cd_bin.logical_tracks[i]->sector_offset);
        printf("  sector size %d\n",
         cd_bin.logical_tracks[i]->sector_size);
      }
    }

    cd_bin.last_seek_track = 0;

    fclose(cue_file);
    return 0;
  }

  return -1;
}

#define address8(base, offset)                                                \
  *((u8 *)((u8 *)base + (offset)))                                            \

#define address16(base, offset)                                               \
  *((u16 *)((u8 *)base + (offset)))                                           \

#define address32(base, offset)                                               \
  *((u32 *)((u8 *)base + (offset)))                                           \

// This will only work on little endian platforms for now.

s32 convert_bin_to_wav(FILE *bin_file, char *output_dir, char *wav_file_name,
 u32 sector_count)
{
  FILE *wav_file;
  u8 wav_header[36];
  u8 *riff_header = wav_header + 0;
  u8 *fmt_header = wav_header + 0x0C;
  u8 sector_buffer[2352];
  u32 byte_length = sector_count * 2352;
  u32 i;

  chdir(output_dir);
  wav_file = fopen(wav_file_name, "wb");

  printf("writing wav %s, %x sectors\n", wav_file_name, sector_count);

  // RIFF type chunk
  memcpy(riff_header   + 0x00, "RIFF", 4);
  address32(riff_header, 0x04) = byte_length + 44 - 8;
  memcpy(riff_header   + 0x08, "WAVE", 4);

  // WAVE file chunk: format
  memcpy(fmt_header   + 0x00, "fmt ", 4);
  // Chunk data size
  address32(fmt_header, 0x04) = 16;
  // Compression code: PCM
  address16(fmt_header, 0x08) = 1;
  // Number of channels: Stereo
  address16(fmt_header, 0x0a) = 2;
  // Sample rate: 44100Hz
  address32(fmt_header, 0x0c) = 44100;
  // Average bytes per second: sample rate * 4
  address32(fmt_header, 0x10) = 44100 * 4;
  // Block align (bytes per sample)
  address16(fmt_header, 0x14) = 4;
  // Bit depth
  address16(fmt_header, 0x16) = 16;

  // Write out header
  fwrite(wav_header, 36, 1, wav_file);

  // DATA chunk
  fprintf(wav_file, "data");
  // length
  fwrite(&byte_length, 4, 1, wav_file);

  // Write out sectors
  for(i = 0; i < sector_count; i++)
  {
    fread(sector_buffer, 2352, 1, bin_file);
    fwrite(sector_buffer, 2352, 1, wav_file);
  }

  fclose(wav_file);
  chdir("..");
  return 0;
}

u32 convert_wav_to_ogg(char *wav_file_name, char *output_dir,
 char *ogg_file_name)
{
  char cmd_string[(MAX_PATH * 2) + 16];

  chdir(output_dir);
  sprintf(cmd_string, "oggenc %s", wav_file_name);
  system(cmd_string);

  unlink(wav_file_name);
  chdir("..");
}

s32 convert_bin_to_iso(FILE *bin_file, char *output_dir, char *iso_file_name,
 u32 sector_count)
{
  FILE *iso_file;
  u8 sector_buffer[2352];
  u32 i;

  chdir(output_dir);
  iso_file = fopen(iso_file_name, "wb");
  printf("writing iso %s, %x sectors\n", iso_file_name, sector_count);

  for(i = 0; i < sector_count; i++)
  {
    fread(sector_buffer, 2352, 1, bin_file);
    fwrite(sector_buffer + 16, 2048, 1, iso_file);
  }

  fclose(iso_file);
  chdir("..");
  return 0;
}

#define sector_offset_to_msf(offset, minutes, seconds, frames)                \
{                                                                             \
  u32 _offset = offset;                                                       \
  minutes = (_offset / 75) / 60;                                              \
  seconds = (_offset / 75) % 60;                                              \
  frames = _offset % 75;                                                      \
}                                                                             \


s32 convert_bin_cue(char *output_name_base)
{
  char output_file_name[MAX_PATH];
  FILE *output_cue_file;
  FILE *bin_file = cd_bin.bin_file;
  cd_track_struct *current_track;
  u32 m, s, f;
  u32 current_pregap = 0;
  u32 last_pregap = 0;
  u32 i;
  struct stat sb;

  if(stat(output_name_base, &sb))
    mkdir(output_name_base);

  sprintf(output_file_name, "%s.cue", output_name_base);
  chdir(output_name_base);
  output_cue_file = fopen(output_file_name, "wb");
  chdir("..");

  // Every track gets its own file. It's either going to be of type ISO
  // or of type WAV.

  for(i = 0; i < 100; i++)
  {
    current_track = cd_bin.logical_tracks[i];
    if(current_track != NULL)
    {
      switch(current_track->format_type)
      {
        // Audio
        case 0:
        {
          char output_name_wav[MAX_PATH];

          sprintf(output_file_name, "%s_t%d.ogg", output_name_base, i);
          sprintf(output_name_wav, "%s_t%d.wav", output_name_base, i);

          fprintf(output_cue_file, "FILE \"%s\" OGG\n", output_file_name);
          fprintf(output_cue_file, "  TRACK %02d AUDIO\n", i);
          current_pregap = current_track->pregap_offset - last_pregap;
          last_pregap = current_track->pregap_offset;
          if(current_pregap > 0)
          {
            sector_offset_to_msf(current_pregap, m, s, f);
            fprintf(output_cue_file, "    PREGAP %02d:%02d:%02d\n", m, s, f);
          }
          fprintf(output_cue_file, "    INDEX 01 00:00:00\n");

          fseek(bin_file, current_track->physical_offset, SEEK_SET);
          convert_bin_to_wav(bin_file, output_name_base, output_name_wav,
           current_track->sector_count);
          convert_wav_to_ogg(output_name_wav, output_name_base,
           output_file_name);
          break;
        }

        // Data
        default:
          sprintf(output_file_name, "%s_t%d.iso", output_name_base, i);
          fprintf(output_cue_file, "FILE \"%s\" BINARY\n", output_file_name);
          fprintf(output_cue_file, "  TRACK %02d MODE1/2048\n", i);
          current_pregap = current_track->pregap_offset - last_pregap;
          last_pregap = current_track->pregap_offset;
          if(current_pregap > 0)
          {
            sector_offset_to_msf(current_pregap, m, s, f);
            fprintf(output_cue_file, "    PREGAP %02d:%02d:%02d\n", m, s, f);
          }
          fprintf(output_cue_file, "    INDEX 01 00:00:00\n");

          fseek(bin_file, current_track->physical_offset, SEEK_SET);
          convert_bin_to_iso(bin_file, output_name_base, output_file_name,
           current_track->sector_count);
          break;
      }
    }
  }

  fclose(output_cue_file);

  return 0;
}

int main(int argc, char *argv[])
{
  if(argc < 3)
  {
    printf("usage: %s <input cue> <output base>\n", argv[0]);
    return 0;
  }

  if(load_bin_cue(argv[1]) == 0)
  {
    if(convert_bin_cue(argv[2]) == 0)
      return 0;
  }
  else
  {
    printf("error: could not load cue file %s\n", argv[1]);
  }

  return 1;
}

