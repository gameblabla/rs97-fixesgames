#include <stdio.h>

int 
main(int argc, char**argv)
{
  cpc_dsk_init();

  int index;
  for (index = 1; index < argc; index++) {
    cpc_dsk_dir(argv[index]);
  }
  return 0;
}
