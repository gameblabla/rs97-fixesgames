/*  
    util.c

    Copyright (C) 2010 Amf

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libintl.h>
#include <locale.h>

#ifdef __WIN32__
#include "windows.h"
#include "shlobj.h"
#endif

#include "chroma.h"
#include "level.h"
#include "menu.h"
#include "display.h"
#include "util.h"

void display_quit();

void file_readline(FILE *file, char *buffer, size_t size)
{
    char c;
    size_t pos = 0;
    int ok = 1;

    while(ok)
    {
        c=fgetc(file);
        if(!feof(file) && c!=13 && c!=10 && c!=0 && pos<(size-1))
            buffer[pos++]=c; 
        else
            ok=0; 
    }
    buffer[pos]=0;
}

int isfile(char *filename)
{
    struct stat fst;

    if(stat(filename, &fst) == -1)
        return 0;

    if(S_ISREG(fst.st_mode))
        return 1;
      
      return 0;
}

int isdirectory(char *filename)
{
    struct stat fst;

    if(stat(filename, &fst) == -1)
        return 0;

    if(S_ISDIR(fst.st_mode))
        return 1;

    return 0;
}

void ensuredirectory(char *directory)
{
    char buffer[FILENAME_MAX + 4096];
    if(isdirectory(directory))
        return;

    errno = 0;
#ifdef __WIN32__
    if(mkdir(directory) && errno != EEXIST)
#else
    if(mkdir(directory, 0755) && errno != EEXIST)
#endif
    {
        sprintf(buffer, gettext("Unable to create directory '%s': %s"), directory, strerror(errno));
        warning(buffer);
    }
}

void getfilename(char *filename, char *fullfilename, int create, int location)
{
    char base[FILENAME_MAX];

#ifdef __WIN32__
#ifndef CSIDL_FLAG_CREATE
#define CSIDL_FLAG_CREATE 0x8000
#endif

#ifndef CSIDL_PERSONAL 
#define CSIDL_PERSONAL 0x05
#endif

#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA 0x1A
#endif

#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif

    typedef HRESULT (WINAPI *SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPTSTR);

    HINSTANCE shfolder_dll;
    SHGETFOLDERPATH SHGetFolderPath ;
#endif

    if(location == LOCATION_SYSTEM)
    {
#ifdef CHROMA_DATA_DIR
#ifndef __WIN32__
        sprintf(fullfilename, "%s%s", CHROMA_DATA_DIR, filename);
#else
	strcpy(fullfilename, filename);
#endif
#else
	strcpy(fullfilename, filename);
#endif
	return;
    }

    if(location == LOCATION_DOCUMENTS)
    {
#ifdef __WIN32__
        if((shfolder_dll = LoadLibrary("shfolder.dll")) != NULL)
        {
            SHGetFolderPath = (void *)GetProcAddress(shfolder_dll, "SHGetFolderPathA");
            if(SHGetFolderPath != NULL)
            {

                if(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, base) != S_OK)
                    strcpy(base, ".");
            }
            FreeLibrary(shfolder_dll);

            if(create)
                ensuredirectory(base);
        }
#endif

#ifndef __WIN32__
        if(getenv("HOME") != NULL)
        {
    	    snprintf(base, FILENAME_MAX, "%s", getenv("HOME"));
	    base[FILENAME_MAX - 1] = 0;

    	    if(create)
	        ensuredirectory(base);
        }
        else
        {
	    strcpy(base, ".");
        }
#endif

        snprintf(fullfilename, FILENAME_MAX, "%s/%s", base, filename);
        fullfilename[FILENAME_MAX - 1] = 0;

        if(create)
	    ensuredirectory(fullfilename);
    }

    if(location == LOCATION_LOCAL)
    {
#ifdef __WIN32__
        if((shfolder_dll = LoadLibrary("shfolder.dll")) != NULL)
        {
            SHGetFolderPath = (void *)GetProcAddress(shfolder_dll, "SHGetFolderPathA");
            if(SHGetFolderPath != NULL)
            {

                if(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, base) == S_OK)
                    strcat(base, "\\chroma");
                else
                    strcpy(base, "chroma");
            }
            FreeLibrary(shfolder_dll);

            if(create)
                ensuredirectory(base);
        }
#endif

#ifndef __WIN32__
        if(getenv("HOME") != NULL)
        {
    	    snprintf(base, FILENAME_MAX, "%s/.chroma", getenv("HOME"));
	    base[FILENAME_MAX - 1] = 0;

    	    if(create)
	        ensuredirectory(base);
        }
        else
        {
	    strcpy(base, ".chroma");
        }
#endif

        snprintf(fullfilename, FILENAME_MAX, "%s/%s", base, filename);
        fullfilename[FILENAME_MAX - 1] = 0;

        if(strcmp(filename, "curses.chroma") == 0)
	    create = 0;
        if(strcmp(filename, "sdl.chroma") == 0)
	    create = 0;

        if(create)
	    ensuredirectory(fullfilename);
    }
}

void warning(char *error)
{
#ifdef __WIN32__
    if(display_type() == DISPLAY_SDL)
        MessageBox(0,error,"Chroma",MB_OK);
    else
    {
        fprintf(stderr, "chroma: %s\n", error);
        sleep(5);
    }
#else
    fprintf(stderr, "chroma: %s\n", error);
#endif
}

void fatal(char *error)
{
    display_quit();
#ifdef __WIN32__
    if(display_type() == DISPLAY_SDL)
        MessageBox(0,error,"Chroma",MB_OK);
    else
    {
        fprintf(stderr, "chroma: %s\n", error);
        sleep(5);
    }
#else
    fprintf(stderr, "chroma: %s\n", error);
#endif

    exit(1);
}

int utf8strlen(char *string)
{
    int i, j;

    i = 0; j = 0;
    while(string[i])
    {
        if((string[i] & 0xc0) != 0x80)
            j ++;
        i++;
    }

    return j;
}

void utf8strncpy(char *out, char *in, int l)
{
    int i, x;

    i = 0; x = 0;
    while(in[i] != 0 && x < l)
    {
        out[i] = in[i];
        if((out[i] & 0xc0) != 0x80)
            x ++;
        i ++;
    }
    while(in[i] != 0 && (in[i] & 0xc0) == 0x80)
    {
        out[i] = in[i];
        i ++;
    }
    out[i] = 0; /* unlike the real strncpy! */
}
