/* Caprice32 - Amstrad CPC Emulator

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <zlib.h>
#include "SDL.h"

#include <stdlib.h>
#include <stdio.h>

#include "cap32.h"

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

#if defined(GP2X_MODE) || defined(WIZ_MODE)
  chdir("/usr/gp2x");
  execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif
}

int
main(int argc, char *argv[])
{

#ifndef NO_STDIO_REDIRECT
  /* Redirect standard output and standard error. */
  /* TODO: Error checking. */
  freopen(STDOUT_FILE, "w", stdout);
  freopen(STDERR_FILE, "w", stderr);
  setvbuf(stdout, NULL, _IOLBF, BUFSIZ);  /* Line buffered */
  setbuf(stderr, NULL);          /* No buffering */
#endif /* NO_STDIO_REDIRECT */

  atexit(cleanup_output);

  SDL_main(argc,argv);

  return 0;
}
