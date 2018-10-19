/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include "cap32.h"
#include "z80.h"
#include "psp_run.h"

  static psp_run_t *psp_run_head = (psp_run_t *)0;

int
psp_run_load_file(void)
{
  psp_run_t *psp_run_new;
  char       FileName[MAX_PATH];
  char       Buffer[512];
  FILE      *RunFile;
  char      *DiskName;
  char      *RunName;
  char      *Scan;
  int        error = 0;

  strcpy(FileName, "./run.txt");

  RunFile = fopen(FileName, "r");
  error   = 1;

  if (RunFile != (FILE*)0) {

    while (fgets(Buffer,512,RunFile) != (char *)0) {

      Scan = strchr(Buffer,'\n');
      if (Scan) *Scan = '\0';
      /* For this #@$% of windows ! */
      Scan = strchr(Buffer,'\r');
      if (Scan) *Scan = '\0';
      if (Buffer[0] == '#') continue;

      Scan = strchr(Buffer,'=');
      if (! Scan) continue;

      *Scan = '\0';
      DiskName = strdup(Buffer);
      RunName  = strdup(Scan + 1);
    
      psp_run_new = (psp_run_t *)malloc(sizeof(psp_run_t));

      psp_run_new->next = psp_run_head;
      psp_run_new->disk_name = DiskName;
      psp_run_new->run_name  = RunName;
      psp_run_head           = psp_run_new;
    }

    error = 0;
    fclose(RunFile);
  }

  return error;
}

char *
psp_run_search(char *DiskName)
{
  psp_run_t *Scan;

  if (DiskName != (char *)0 )
  {
    for (Scan  = psp_run_head; 
         Scan != (psp_run_t *)0;
         Scan  = Scan->next)
    {
      if (!strcasecmp(Scan->disk_name, DiskName)) {
        return Scan->run_name;
      }
    }
  }
  return NULL;
}
