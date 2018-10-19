/*  
    util.h

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

void file_readline(FILE *file, char *buffer, size_t size);

int isfile(char *filename);
int isdirectory(char *filename);

void getfilename(char *filename, char *fullfilename, int create, int system);

void fatal(char *);
void warning(char *);

int utf8strlen(char *string);
void utf8strncpy(char *out, char *in, int l);

#define LOCATION_LOCAL     0
#define LOCATION_SYSTEM    1
#define LOCATION_DOCUMENTS 2
