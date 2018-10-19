#include <stdio.h>

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;
typedef unsigned long long int u64;
typedef signed char s8;
typedef signed short int s16;
typedef signed long int s32;
typedef signed long long int s64;

u8 hucard_rom[1024 * 1024];

int main(int argc, char *argv[])
{
  FILE *rom_file = fopen(argv[1], "rb");
  u32 file_size;
  u32 header_size;
  u32 rom_size, rom_pages;
  u32 i;
  u32 current_byte;
  u32 new_byte;
  u32 temp;

  fseek(rom_file, 0, SEEK_END);
  file_size = ftell(rom_file);

  header_size = file_size & 0x1FFF;
  rom_size = file_size - header_size;
  rom_pages = rom_size / 0x2000;

  printf("header size: %x, rom size %x (%x pages)\n", header_size, rom_size,
   rom_pages);

  fseek(rom_file, header_size, SEEK_SET);

  fread(hucard_rom, rom_size, 1, rom_file);
  fclose(rom_file);

  printf("Unencrypting encrypted ROM.\n");

  char encryption_table[16] =
  {
    0, 8, 4, 12, 2, 10, 6, 14,
    1, 9, 5, 13, 3, 11, 7, 15
  };

  for(i = 0; i < rom_size; i++)
  {
    current_byte = hucard_rom[i];
    temp = hucard_rom[i] & 0xF;

    new_byte = encryption_table[current_byte >> 4];
    new_byte |= encryption_table[current_byte & 0xF] << 4;

    hucard_rom[i] = new_byte;
  }

  rom_file = fopen(argv[2], "wb");
  fwrite(hucard_rom, rom_size, 1, rom_file);

  return 0;
}

