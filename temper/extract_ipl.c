#include <stdio.h>

int main(int argc, char *argv[])
{
  FILE *bin_file = fopen(argv[1], "rb");
  FILE *ipl_file = fopen("ipl.bin", "wb");
  char ipl_sector[2048];

  fseek(bin_file, 0x78c400, SEEK_SET);
  fread(ipl_sector, 2048, 1, bin_file);
  fwrite(ipl_sector, 2048, 1, ipl_file);

  fclose(bin_file);
  fclose(ipl_file);
  return 0;
}
