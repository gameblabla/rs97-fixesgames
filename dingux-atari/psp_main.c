/*
   PSPATARI: Porting of Atari800
   Ludovic Jacomme <Ludovic.Jacomme@gmail.com>
*/

#include <stdio.h>
#include <zlib.h>
#include "global.h"

#include <stdlib.h>
#include <stdio.h>

#define STDOUT_FILE  "stdout.txt"
#define STDERR_FILE  "stderr.txt"

extern int SDL_main(int argc, char *argv[]);

static void cleanup_output(void);

/* Remove the output files if there was no output written */
static void cleanup_output(void)
{
#ifndef NO_STDIO_REDIRECT
  FILE *file;
  int empty;
#endif

  /* Flush the output in case anything is queued */
  fclose(stdout);
  fclose(stderr);

#ifndef NO_STDIO_REDIRECT
  /* See if the files have any output in them */
  file = fopen(STDOUT_FILE, "rb");
  if ( file ) {
    empty = (fgetc(file) == EOF) ? 1 : 0;
    fclose(file);
    if ( empty ) {
      remove(STDOUT_FILE);
    }
  }
  file = fopen(STDERR_FILE, "rb");
  if ( file ) {
    empty = (fgetc(file) == EOF) ? 1 : 0;
    fclose(file);
    if ( empty ) {
      remove(STDERR_FILE);
    }
  }
#endif
# ifdef GP2X_MODE
  gp2xRmmodMMUhack();
# endif
  // TODO jz4740
  //cpu_deinit();
#if defined(GP2X_MODE) || defined(WIZ_MODE)
  chdir("/usr/gp2x");
  execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif
}

int
main(int argc, char *argv[])
{

  // TODO jz4740
  //cpu_init();

#ifndef NO_STDIO_REDIRECT
  /* Redirect standard output and standard error. */
  /* TODO: Error checking. */
  freopen(STDOUT_FILE, "w", stdout);
  freopen(STDERR_FILE, "w", stderr);
  setvbuf(stdout, NULL, _IOLBF, BUFSIZ);  /* Line buffered */
  setbuf(stderr, NULL);          /* No buffering */
#endif /* NO_STDIO_REDIRECT */

  atexit(cleanup_output);

#ifdef GP2X_MODE
  gp2xInsmodMMUhack();
#endif

  SDL_main(argc,argv);

  return 0;
}
