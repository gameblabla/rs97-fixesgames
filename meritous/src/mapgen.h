//
//   mapgen.h
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

// Exposes mapgen.c functionality and types

#ifndef MAPGEN_H
#define MAPGEN_H

struct RoomConnection {
	int x, y;
	int x2, y2;
	int c;
	struct RoomConnection *n;
};

typedef struct {
	int x, y;
	int w, h;
	int creator;
	int visited;
	int checkpoint;
	int s_dist;
	int connections;
	int room_type;
	int room_param;
	int enemies;
	struct RoomConnection *con;
} Room;

typedef struct {
	int w, h;
	unsigned char *m;
	int *r;
	int totalRooms;
	//Room *rooms;
} GameLevel;

extern GameLevel map;
extern Room rooms[3000];

void RandomGenerateMap();

void Put(int x, int y, unsigned char tile, int room);
unsigned char Get(int x, int y);
int GetRoom(int x, int y);
int GetVisited(int x, int y);
extern int place_of_power;

void WriteMapData();
void ReadMapData();

void DestroyDungeon();

void Paint(int xp, int yp, int w, int h, char *fname);

#endif
