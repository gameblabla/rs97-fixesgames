//
//   save.c
//
//   Copyright 2007, 2008 Lancer-X/ASCEAI
//
//   This file is part of Meritous.
//
//   Meritous is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Meritous is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Meritous.  If not, see <http://www.gnu.org/licenses/>.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <zlib.h>

#include "levelblit.h"
#include "mapgen.h"
#include "demon.h"
#include "tiles.h"

#if defined(WITH_HOME_DIR)
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

char *homeDir = NULL;

gzFile Filefp;
int game_load = 0;

void getHomeDir()
{
#if defined(WITH_HOME_DIR)
	if(homeDir != NULL)
	{
		free(homeDir);
		homeDir = NULL;
	}

	homeDir = (char *)malloc(strlen(getenv("HOME")) + strlen("/.meritous") + 1);
	strcpy(homeDir, getenv("HOME"));
	strcat(homeDir, "/.meritous");
	mkdir(homeDir, 0755); // create $HOME/.meritous if it doesn't exist
#endif
}

void freeHomeDir()
{
	if(homeDir != NULL)
	{
		free(homeDir);
		homeDir = NULL;
	}
}

void FWChar(unsigned char i)
{
	gzputc(Filefp, i);
}

unsigned char FRChar()
{
	return gzgetc(Filefp);
}

void FWInt(int val)
{
	unsigned char data[sizeof(int) * 2];
	size_t byte_count = 0;
	int sign = val < 0;

	/* Little-endian (least significant bits first) groups of 7 bits with the
	 * high bit set to indicate continuation bytes that are worth 128 times
	 * more than the previous byte. The last byte has 6 useful bits, bit 7
	 * unset, and bit 6 is set if the number is negative (< 0), otherwise
	 * unset (>= 0). */

	if (sign) val = -val;

	while (val >= 128) {
		data[byte_count++] = 0x80 | (val & 0x7F);
		val >>= 7;
	}

	if (val >= 64) {
		/* The last byte would need to represent 7 bits, but it may only
		 * represent 6, the sign bit, and the ending marker. We must put 7
		 * bits into one byte, then make another with 6 unset bits and the
		 * ending marker stuff. */
		data[byte_count++] = 0x80 | (val & 0x7F);
		data[byte_count++] = sign << 6;
	} else {
		data[byte_count++] = (sign << 6) | (val & 0x3F);
	}

	gzwrite(Filefp, data, byte_count);
}

int FRInt()
{
	int val = 0;
	int bit_count = 0;
	unsigned char c;

	while (((c = gzgetc(Filefp)) & 0x80) && bit_count < sizeof(int) * 8) {
		val |= (c & 0x7F) << bit_count;
		bit_count += 7;
	}

	if (c & 0x80) {
		fprintf(stderr, "An integer in the save file is too large for this system\nAborting\n");
		exit(2);
	}

	val |= (c & 0x3F) << bit_count;
	if (c & 0x40) val = -val;

	return val;
}

void SaveGame(char *filename)
{
	Filefp = gzopen(filename, "wb4");
	FWChar(0x7E);
	WriteMapData();
	WriteCreatureData();
	WritePlayerData();

	gzclose(Filefp);
}

void LoadGame(char *filename)
{
	unsigned char parity;
#if defined(WITH_HOME_DIR)
	char filePath[50];

	if(homeDir == NULL)
	{
		getHomeDir();
	}

	strcpy(filePath, homeDir);
	strcat(filePath, "/");
	strcat(filePath, filename);
	Filefp = gzopen(filePath, "rb");

#else
	Filefp = gzopen(filename, "rb");
#endif
	parity = FRChar();
	if (parity != 0x7E) {
		fprintf(stderr, "Parity byte in error (%x != 0x7E)\nAborting\n", parity);
		exit(2);
	}
	game_load = 1;
}

void CloseFile()
{
	gzclose(Filefp);
}

void DoSaveGame()
{
#if defined(WITH_HOME_DIR)
	char filePath[50];

	if(homeDir == NULL)
	{
		getHomeDir();
	}

	strcpy(filePath, homeDir);
	strcat(filePath, "/SaveFile.sav");
	SavingScreen(0, 0.0);
	SaveGame(filePath);
#else
	SavingScreen(0, 0.0);
	SaveGame("SaveFile.sav");
#endif
}

int IsSaveFile()
{
	FILE *fp;
#if defined(WITH_HOME_DIR)
	char filePath[50];

	if(homeDir == NULL)
	{
		getHomeDir();
	}

	strcpy(filePath, homeDir);
	strcat(filePath, "/SaveFile.sav");

	if ((fp = fopen(filePath, "rb")) != NULL) {
		fclose(fp);
		return 1;
	}
#else
	if ((fp = fopen("SaveFile.sav", "rb")) != NULL) {
		fclose(fp);
		return 1;
	}
#endif
	return 0;
}
