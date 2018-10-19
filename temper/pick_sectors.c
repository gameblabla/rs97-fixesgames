#include <stdio.h>
#include <stdlib.h>

int random_sector()
{
  return (((rand() & 0xFFFF) | (rand() << 16)) % 328501);
}

int main()
{
  int i, i2;

  for(i = 0; i < 3; i++)
  {
    printf("root sector %x\n", random_sector());

    for(i2 = 0; i2 < 9; i2++)
    {
      printf("sub-sector %x\n", random_sector());
    }
  }
}
