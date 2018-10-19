#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int height;
  FILE *input_file, *output_file;
  int i, i2;

  height = atoi(argv[3]);

  input_file = fopen(argv[1], "rb");
  output_file = fopen(argv[2], "wb");

  fprintf(output_file, "  {\n");

  for(i = 0; i < 256; i++)
  {
    for(i2 = 0; i2 < height; i2++)
    {
      fprintf(output_file, "    0x%02x00,\n", fgetc(input_file));
    }

    fseek(input_file, 14 - height, SEEK_CUR);
  }

  fprintf(output_file, "  },\n\n  {\n");

  for(i = 0; i < 256; i++)
  {
    fprintf(output_file, "    %d,\n", i * height);
  }
  fprintf(output_file, "  }\n");
}
