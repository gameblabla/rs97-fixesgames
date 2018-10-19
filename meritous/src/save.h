//
//   save.h
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

// save.c header file

#ifndef SAVE_H
#define SAVE_H

#define PROGRESS_DELAY_MS 50

void getHomeDir();
void freeHomeDir();

void DoSaveGame();

void FWInt(int val);
void FWChar(unsigned char i);
unsigned char FRChar();
int FRInt();

extern int game_load;

void SaveGame(char *);
void LoadGame(char *);
void CloseFile();

int IsSaveFile();

#endif
