//
//   mapgen.c
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
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include <SDL.h>
#include "save.h"
#include "levelblit.h"

void NewLevel();
	
void SaveLevel();

int Generate();

int DoRepeat = 0;

int place_of_power = 0;

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

GameLevel map;
Room rooms[3000];
int total_rooms = 0;

int rdir = 0;

int next_check = 30;

int GetRoom(int x, int y);

int rndnum(int max)
{
	return rand() % (max+1);
}

int r_fails[4] = {0};
int r_successes[4] = {0};

unsigned char floortiles[4] = {12, 18, 19, 20};

void WriteRoomConnectionData(Room* rm, struct RoomConnection* rc) {
	if (rc->y == rm->y) {
		FWChar(0); // At the top of the room
		FWInt(rc->x - rm->x);
	} else if (rc->y == rm->y + rm->h - 1) {
		FWChar(1); // At the bottom of the room
		FWInt(rc->x - rm->x);
	} else if (rc->x == rm->x) {
		FWChar(2); // At the left of the room
		FWInt(rc->y - rm->y);
	} else if (rc->x == rm->x + rm->w - 1) {
		FWChar(3); // At the right of the room
		FWInt(rc->y - rm->y);
	}
	FWInt(rc->c);
}

void ReadRoomConnectionData(Room* rm, int n, struct RoomConnection* rc) {
	unsigned char direction = FRChar();
	int offset = FRInt();

	switch (direction) {
	case 0:
		if (offset <= 0 || offset >= rm->w - 1) {
			fprintf(stderr, "Room #%d in the save file has a connection that is out of bounds\nAborting\n", n);
			exit(2);
		}
		rc->x = rm->x + offset;
		rc->y = rm->y;
		rc->x2 = rc->x;
		rc->y2 = rc->y - 1;
		break;

	case 1:
		if (offset <= 0 || offset >= rm->w - 1) {
			fprintf(stderr, "Room #%d in the save file has a connection that is out of bounds\nAborting\n", n);
			exit(2);
		}
		rc->x = rm->x + offset;
		rc->y = rm->y + rm->h - 1;
		rc->x2 = rc->x;
		rc->y2 = rc->y + 1;
		break;

	case 2:
		if (offset <= 0 || offset >= rm->h - 1) {
			fprintf(stderr, "Room #%d in the save file has a connection that is out of bounds\nAborting\n", n);
			exit(2);
		}
		rc->x = rm->x;
		rc->y = rm->y + offset;
		rc->x2 = rc->x - 1;
		rc->y2 = rc->y;
		break;

	case 3:
		if (offset <= 0 || offset >= rm->h - 1) {
			fprintf(stderr, "Room #%d in the save file has a connection that is out of bounds\nAborting\n", n);
			exit(2);
		}
		rc->x = rm->x + rm->w - 1;
		rc->y = rm->y + offset;
		rc->x2 = rc->x + 1;
		rc->y2 = rc->y;
		break;

	default:
		fprintf(stderr, "Room #%d in the save file has a connection with a corrupt direction\nAborting\n", n);
		exit(2);
		break;
	}
	rc->c = FRInt();
	if (rc->c < 0 || rc->c >= map.totalRooms) {
		fprintf(stderr, "Room #%d in the save file has a connection to a room that doesn't exist\nAborting\n", n);
		exit(2);
	}
}

void WriteRoomData(Room *rm)
{
	struct RoomConnection *rt;
	FWInt(rm->x);
	FWInt(rm->y);
	FWInt(rm->w);
	FWInt(rm->h);
	FWChar((rm->visited & 1) | ((rm->checkpoint & 1) << 1));
	FWInt(rm->s_dist);
	FWInt(rm->connections);
	FWInt(rm->room_type);
	FWInt(rm->room_param);
	rt = rm->con;
	while (rt != NULL) {
		WriteRoomConnectionData(rm, rt);
		rt = rt->n;
	}
}

void ReadRoomData(Room *rm, int n)
{
	int i, x, y;
	unsigned char data;
	struct RoomConnection *rt;
	
	rm->x = FRInt();
	rm->y = FRInt();
	rm->w = FRInt();
	rm->h = FRInt();

	if (rm->x < 0 || rm->x >= map.w || rm->y < 0 || rm->y >= map.h
	 || rm->w < 0 || rm->w >= map.w || rm->h < 0 || rm->h >= map.h
	 || rm->x + rm->w > map.w || rm->y + rm->h > map.h) {
		fprintf(stderr, "Room #%d in the save file is out of bounds\nAborting\n", n);
		exit(2);
	}

	for (y = rm->y; y < rm->y + rm->h; y++) {
		for (x = rm->x; x < rm->x + rm->w; x++) {
			if (map.r[y * map.w + x] != -1) {
				fprintf(stderr, "Rooms #%d and #%d in the save file overlap with each other\nAborting\n", map.r[y * map.w + x], n);
				exit(2);
			}
			map.r[y * map.w + x] = n;
		}
	}

	data = FRChar();
	if (data & ~0x3) {
		fprintf(stderr, "Room #%d in the save file has reserved flags set\nAborting\n", n);
		exit(2);
	}
	rm->visited = data & 1;
	rm->checkpoint = (data >> 1) & 1;
	rm->s_dist = FRInt();
	rm->connections = FRInt();
	rm->room_type = FRInt();
	rm->room_param = FRInt();
	
	rm->con = NULL;
	
	rm->enemies = 0;
	
	for (i = 0; i < rm->connections; i++) {
		rt = rm->con;
		rm->con = malloc(sizeof(struct RoomConnection));
		ReadRoomConnectionData(rm, n, rm->con);
		rm->con->n = rt;
	}
}

void WriteMapData()
{
	int i;
	Uint32 last_ticks = SDL_GetTicks(), cur_ticks;

	FWInt(place_of_power);
	for (i = 0; i < map.w*map.h; i++) {
		FWChar(map.m[i]);
		if ((cur_ticks = SDL_GetTicks()) >= last_ticks + PROGRESS_DELAY_MS) {
			SavingScreen(0, (float)i / (float)(map.w*map.h));
			last_ticks = cur_ticks;
		}
	}
	for (i = 0; i < map.totalRooms; i++) {
		WriteRoomData(&rooms[i]);
		if ((cur_ticks = SDL_GetTicks()) >= last_ticks + PROGRESS_DELAY_MS) {
			SavingScreen(1, (float)i / (float)map.totalRooms);
			last_ticks = cur_ticks;
		}
	}
}

void ReadMapData()
{
	int i;
	Uint32 last_ticks = SDL_GetTicks(), cur_ticks;

	map.totalRooms = total_rooms = 3000;
	place_of_power = FRInt();
	if (place_of_power < 0 || place_of_power >= map.totalRooms) {
		fprintf(stderr, "The Place of Power in the save file refers to a room that doesn't exist\nAborting\n");
		exit(2);
	}
	for (i = 0; i < map.w*map.h; i++) {
		if ((cur_ticks = SDL_GetTicks()) >= last_ticks + PROGRESS_DELAY_MS) {
			LoadingScreen(0, (float)i / (float)(map.w*map.h));
			last_ticks = cur_ticks;
		}
		map.m[i] = FRChar();
	}
	LoadingScreen(0, 1);
	for (i = 0; i < map.totalRooms; i++) {
		ReadRoomData(&rooms[i], i);
		if ((cur_ticks = SDL_GetTicks()) >= last_ticks + PROGRESS_DELAY_MS) {
			LoadingScreen(1, (float)i / (float)map.totalRooms);
			last_ticks = cur_ticks;
		}
	}
	LoadingScreen(1, 1);
}

int rndval(int a, int b)
{
	int temp;
	
	if (a == b) {
		return a;
	}
	
	if (b < a) {
		temp = a;
		a = b;
		b = temp;
	}
	
	temp = rndnum(b - a);
	
	return temp + a;
}

void RandomGenerateMap()
{
	int trying = 1;
	if (game_load) {
		NewLevel();
		ReadMapData();
	} else {
		NewLevel();
		while (trying) {
		
			trying = !Generate();
		}
	}
	//SaveLevel();
}

void NewLevel()
{
	int x, y;
	unsigned char *map_p;
	
	map.w = 512;
	map.h = 512;
	
	map.m = malloc(map.w * map.h * sizeof(unsigned char));
	map.r = malloc(map.w * map.h * sizeof(int));
	map_p = map.m;
	
	for (y = 0; y < map.h; y++) {
		for (x = 0; x < map.w; x++) {
			*(map_p++) = 17;
			map.r[y*map.w+x] = -1;
		}
	}
}

void DestroyDungeon()
{
	int i;
	struct RoomConnection *c, *d;
	
	// Destroy map
	free(map.m);
	free(map.r);
	
	// Destroy rooms
	for (i = 0; i < total_rooms; i++) {
		c = rooms[i].con;
		while (c != NULL) {
			d = c;
			c = c->n;
			free(d);
		}
	}
	total_rooms = 0;
}

void ResetLevel()
{
	int x, y;
	unsigned char *map_p;
	
	map.w = 512;
	map.h = 512;
	map_p = map.m;
	
	total_rooms = 0;

	rdir = 0;

	next_check = 30;
	
	for (y = 0; y < map.h; y++) {
		for (x = 0; x < map.w; x++) {
			*(map_p++) = 17;
			map.r[y*map.w+x] = -1;
		}
	}
}

void SaveLevel()
{
	int x, y, i;
	SDL_Surface *map_surf;
	char cs[2] = ".";
	char rnum[5] = "0000";
	unsigned char ch;
	unsigned char *map_p;
	SDL_Color cpalette[4];
	Uint8 cl;
	
	map_surf = SDL_CreateRGBSurface(0, 4096, 4096, 8, 0, 0, 0, 0);
	
	map_p = map.m;
	
	cpalette[0].r = cpalette[0].g = cpalette[0].b = 0;
	cpalette[1].r = cpalette[1].g = cpalette[1].b = 255;
	cpalette[2].r = 255; cpalette[2].g = 0; cpalette[2].b = 255;
	cpalette[3].r = 0; cpalette[3].g = 255; cpalette[3].b = 128;
	
	SDL_SetPalette(map_surf, SDL_LOGPAL | SDL_PHYSPAL, cpalette, 0, 4);
	
	for (y = 0; y < map.h; y++) {
		for (x = 0; x < map.w; x++) {
			ch = *(map_p++);
			
			if (IsSolid(ch))
				*cs = 4;
			else
				*cs = 5;
				
			if (ch == 17)
				*cs = 0;
				
			cl = 1;
			if (rooms[GetRoom(x, y)].room_type == 2) cl = 2;
			if (rooms[GetRoom(x, y)].room_type == 3) cl = 3;
			
			draw_text_ex(x*8, y*8, cs, cl, map_surf);
		}
	}
	for (i = 0; i < 3000; i++) {
		sprintf(rnum, "%d", i);
		draw_text_ex(rooms[i].x * 8, rooms[i].y * 8, rnum, 0, map_surf);
	}
	
	SDL_SaveBMP(map_surf, "map.bmp");
}

void CreateRoomDimensions(int *w, int *h)
{
	*w = rndval(5, 12);
	*h = rndval(5, 12);
	
	if (*w == 12) {
		*w = rndval(12, 15);
	}
	if (*h == 12) {
		*h = rndval(12, 15);
	}
}

void Put(int x, int y, unsigned char tile, int room)
{
	map.m[map.w*y+x] = tile;
	map.r[map.w*y+x] = room;
}

unsigned char Get(int x, int y)
{
	if (x < 0) return 17;
	if (y < 0) return 17;
	if (x >= map.w) return 17;
	if (y >= map.h) return 17;
	
	return map.m[map.w*y+x];
}

int GetRoom(int x, int y)
{
	if (x < 0) return -1;
	if (y < 0) return -1;
	if (x >= map.w) return -1;
	if (y >= map.h) return -1;
	
	return map.r[map.w*y+x];
}

int GetVisited(int x, int y)
{
	if (x < 0) return 0;
	if (y < 0) return 0;
	if (x >= map.w) return 0;
	if (y >= map.h) return 0;
	
	return rooms[GetRoom(x, y)].visited;
}

void Paint(int xp, int yp, int w, int h, char *fname)
{
	FILE *fp;
	int x, y;
	fp = fopen(fname, "rb");
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			Put(x+xp, y+yp, fgetc(fp), GetRoom(x+xp, y+yp));
		}
	}
	fclose(fp);
}

void DrawRoom(int place_x, int place_y, int room_w, int room_h, int room_id)
{
	int x, y, i;
	int f_type;
	
	f_type = rand()%4;
	// Corners
	Put(place_x, place_y, 11, room_id);
	Put(place_x + room_w - 1, place_y, 10, room_id);
	Put(place_x, place_y + room_h - 1, 9, room_id);
	Put(place_x + room_w - 1, place_y + room_h - 1, 8, room_id);
	
	// Walls
	
	for (i = 0; i < room_w - 2; i++) {
		Put(place_x + 1 + i, place_y + room_h - 1, 4, room_id);
		if (rand() % 16 == 0) Put(place_x + 1 + i, place_y + room_h - 1, 45 + (rand()%2)*4, room_id);
		Put(place_x + 1 + i, place_y, 5, room_id);
		if (rand() % 16 == 0) Put(place_x + 1 + i, place_y, 46 + (rand()%2)*4, room_id);
	}
	for (i = 0; i < room_h - 2; i++) {
		Put(place_x + room_w - 1, place_y + 1 + i, 6, room_id);
		if (rand() % 16 == 0) Put(place_x + room_w - 1, place_y + 1 + i, 47 + (rand()%2)*4, room_id);
		Put(place_x, place_y + 1 + i, 7, room_id);
		if (rand() % 16 == 0) Put(place_x, place_y + 1 + i, 48 + (rand()%2)*4, room_id);
	}
	
	// Floor
	
	for (y = 0; y < room_h - 2; y++) {
		for (x = 0; x < room_w - 2; x++) {
			Put(place_x + 1 + x, place_y + 1 + y, floortiles[f_type], room_id);
		}
	}

	// Magic Tiles

	if ((room_id % 30) == 29) {
		if (Get(place_x + 1 + rand()%(room_w-2), place_y + 1 + rand()%(room_h-2)) == floortiles[f_type]) {
			Put(place_x + 1 + rand()%(room_w-2), place_y + 1 + rand()%(room_h-2), 28+rand()%3, room_id);
		}
	}

	// Save tiles
	
	if ((room_id % 25) == 20) {
		x = place_x + 1 + rand()%(room_w-2);
		y = place_y + 1 + rand()%(room_h-2);
		if (Get(x, y) == floortiles[f_type]) {
			Put(x, y, 31, room_id);
		}
	}

	// Summon tiles
	if ((room_id % 75) == 48) {
		x = place_x + 1 + rand()%(room_w-2);
		y = place_y + 1 + rand()%(room_h-2);
		if (Get(x, y) == floortiles[f_type]) {
			Put(x, y, 32, room_id);
		}
	}
	
	// Compass tile
	
	if ((room_id % 20) == 19) {
		x = place_x + 1 + rand()%(room_w-2);
		y = place_y + 1 + rand()%(room_h-2);
		if (Get(x, y) == floortiles[f_type]) {
			Put(x, y, 53, room_id);
		}
	}
	
	// First room
	if (room_id == 0) {
		Paint(place_x+1, place_y+1, room_w-2, room_h-2, "dat/d/centre.loc");
	}
	// Power object rooms
	if ((room_id % 1000) == 499) {
		Paint(place_x+1, place_y+1, room_w-2, room_h-2, "dat/d/weapon.loc");
	}
	// Boss rooms
	if ((room_id % 1000) == 999) {
		Paint(place_x+1, place_y+1, room_w-2, room_h-2, "dat/d/bossroom.loc");
	}
}

int NoRoomCollision(int place_x, int place_y, int room_w, int room_h)
{
	int x, y;
	
	if (place_x < 0) return 0;
	if (place_y < 0) return 0;
	if ((place_x+room_w) > map.w) return 0;
	if ((place_y+room_h) > map.h) return 0;

	for (y = 0; y < room_h; y++) {
		for (x = 0; x < room_w; x++) {
			if (Get(place_x + x, place_y + y) != 17) return 0;
		}
	}
	
	return 1;
}

void MakeConnect(int x, int y, int type)
{
	int nx, ny;
	int d1, d2;
	int room_1, room_2;
	struct RoomConnection *rconnect;
	
	switch (type) {
		case 0:
			nx = x;
			ny = y - 1;
			d1 = 14;
			d2 = 13;
			break;
		case 1:
			nx = x;
			ny = y + 1;
			d1 = 13;
			d2 = 14;
			break;
		case 2:
			nx = x - 1;
			ny = y;
			d1 = 16;
			d2 = 15;
			break;
		case 3:
			nx = x + 1;
			ny = y;
			d1 = 15;
			d2 = 16;
			break;
		default:
			nx = 0;
			ny = 0;
			d1 = 0;
			d2 = 0;
			break;
	}
	
	room_1 = GetRoom(x, y);
	room_2 = GetRoom(nx, ny);
	if ((room_1 % 1000) == 999) {
		d1 = d1 - 13 + 21;
		d2 = d2 - 13 + 38;
	} else {
		if ((room_2 % 1000) == 999) {
			d1 = d1 - 13 + 38;
			d2 = d2 - 13 + 21;
		}
	}
	Put(x, y, d1, GetRoom(x, y));
	Put(nx, ny, d2, GetRoom(nx, ny));

	rooms[room_1].connections++;
	rconnect = rooms[room_1].con;
	rooms[room_1].con = malloc(sizeof(struct RoomConnection));
	rooms[room_1].con->n = rconnect;
	rooms[room_1].con->x = x;
	rooms[room_1].con->y = y;
	rooms[room_1].con->x2 = nx;
	rooms[room_1].con->y2 = ny;
	rooms[room_1].con->c = room_2;
	
	rooms[room_2].connections++;
	rconnect = rooms[room_2].con;
	rooms[room_2].con = malloc(sizeof(struct RoomConnection));
	rooms[room_2].con->n = rconnect;
	rooms[room_2].con->x = nx;
	rooms[room_2].con->y = ny;
	rooms[room_2].con->x2 = x;
	rooms[room_2].con->y2 = y;
	rooms[room_2].con->c = room_1;

}

int SuitableConnection(int t)
{
	switch (t) {
		case 4:
		case 5:
		case 6:
		case 7:
		
		case 45:
		case 46:
		case 47:
		case 48:
		
		case 49:
		case 50:
		case 51:
		case 52:
			return 1;
			break;
			
		default:
			break;
	}
	return 0;
}

void NewRoom(int place_x, int place_y, int room_w, int room_h, int creator)
{
	int connect_points = 0;
	int cplist_x[100], cplist_y[100], cplist_r[100], cplist_t[100];
	
	int sr_cps = 0;
	int sr_cp[100];
	
	int sr_nps = 0;
	int sr_np[100];
	
	int i;
	
	// Draw this room
	rooms[total_rooms].checkpoint = 0;
	DrawRoom(place_x, place_y, room_w, room_h, total_rooms);
	
	rooms[total_rooms].x = place_x;
	rooms[total_rooms].y = place_y;
	
	rooms[total_rooms].w = room_w;
	rooms[total_rooms].h = room_h;
	
	rooms[total_rooms].room_type = 0;
	rooms[total_rooms].room_param = 0;
	
	rooms[total_rooms].creator = creator;
	
	rooms[total_rooms].connections = 0;
	rooms[total_rooms].con = NULL;
	rooms[total_rooms].enemies = 0;
	
	rooms[total_rooms].visited = 0;

	rooms[total_rooms].s_dist = -1;
	
	if (total_rooms == 0) {
		rooms[total_rooms].checkpoint = 1;
	}
	
	
	
	total_rooms++;

	if (creator == -1) return;
	
	// Find connection points
	
	for (i = 0; i < room_w - 2; i++) {
		if (SuitableConnection(Get(place_x + 1 + i, place_y - 1))) {
			cplist_x[connect_points] = place_x + 1 + i;
			cplist_y[connect_points] = place_y;
			cplist_r[connect_points] = GetRoom(place_x + 1 + i, place_y - 1);
			cplist_t[connect_points] = 0;
			connect_points++;
		}
		
		if (SuitableConnection(Get(place_x + 1 + i, place_y + room_h))) {
			cplist_x[connect_points] = place_x + 1 + i;
			cplist_y[connect_points] = place_y + room_h - 1;
			cplist_r[connect_points] = GetRoom(place_x + 1 + i, place_y + room_h);
			cplist_t[connect_points] = 1;
			connect_points++;
		}
	}
	for (i = 0; i < room_h - 2; i++) {
		if (SuitableConnection(Get(place_x - 1, place_y + 1 + i))) {
			cplist_x[connect_points] = place_x;
			cplist_y[connect_points] = place_y + 1 + i;
			cplist_r[connect_points] = GetRoom(place_x - 1, place_y + 1 + i);
			cplist_t[connect_points] = 2;
			connect_points++;
		}
		
		if (SuitableConnection(Get(place_x + room_w, place_y + 1 + i))) {
			cplist_x[connect_points] = place_x + room_w - 1;
			cplist_y[connect_points] = place_y + 1 + i;
			cplist_r[connect_points] = GetRoom(place_x + room_w, place_y + 1 + i);
			cplist_t[connect_points] = 3;
			connect_points++;
		}
	}
	
	for (i = 0; i < connect_points; i++) {
		if (cplist_r[i] == creator) {
			sr_cp[sr_cps++] = i;
		} else {
			sr_np[sr_nps++] = i;
		}
	}
	
	//printf("cps: %d      room: %d\n", sr_cps, total_rooms);

	assert(sr_cps > 0);
	
	i = rndval(0, sr_cps-1);
	MakeConnect(cplist_x[sr_cp[i]], cplist_y[sr_cp[i]], cplist_t[sr_cp[i]]);
	
	// one other connection (if we can)
	if (sr_nps > 0) {
		i = rndval(0, sr_nps-1);
		MakeConnect(cplist_x[sr_np[i]], cplist_y[sr_np[i]], cplist_t[sr_np[i]]);
	}

}

int AddChild(int room_id)
{
	Room r = rooms[room_id];
	int place_x = r.x;
	int place_y = r.y;
	int room_w = r.w;
	int room_h = r.h;
	int new_w, new_h, new_x, new_y;
	int room_pos;
	
	int trying;
	int attempts;
	

	
	trying = 1;
	attempts = 0;
	while (trying) {
		attempts++;
		
		if (( (total_rooms+1) % 500)==0) {
			new_w = 20;
			new_h = 15;
		} else {
			CreateRoomDimensions(&new_w, &new_h);
		}
		
		room_pos = (rdir++)%4;
		
		if (room_pos < 2) {
			// vertical placement
			new_x = rndval(place_x - (new_w - 3), place_x + (room_w - 3));
			if (room_pos == 0) {
				new_y = place_y - new_h;
			} else {
				new_y = place_y + room_h;
			}
		} else {
			// horiz placement
			new_y = rndval(place_y - (new_h - 3), place_y + (room_h - 3));
			if (room_pos == 2) {
				new_x = place_x - new_w;
			} else {
				new_x = place_x + room_w;
			}
		}
		
		if (NoRoomCollision(new_x, new_y, new_w, new_h)) {
			//printf("SUCCESS\n");
			r_successes[room_pos]++;
			NewRoom(new_x, new_y, new_w, new_h, room_id);
			return 1;
		} else {
			//printf("FAIL %d\n", attempts);
			r_fails[room_pos]++;
			if (attempts > 20) return 0;
		}
	}
	return 0;
}

void RecurseSetDist()
{
	struct RoomConnection *rc;
	int queue[10000];
	int q_top = 1;
	int q_bot = 0;
	int rooms_left = 3000;
	int c_room;
	queue[0] = 0;
	
	if (rooms_left % 100 == 0) {
		LoadingScreen(1, 1.0 - ((float)rooms_left / 3000.0));
	}
	
	rooms[0].s_dist = 0;
	
	while ((rooms_left > 0)) {	
		c_room = queue[q_bot];
		q_bot++;
		rooms_left--;
		
		rc = rooms[c_room].con;
		
		while (rc != NULL) {
			//assert(qp < 3000);
			if (rooms[rc->c].s_dist == -1) {
				queue[q_top] = rc->c;
				q_top++;
				rooms[rc->c].s_dist = rooms[c_room].s_dist+1;
			}
			rc = rc->n;
		}
	}
}

int RoomSize(int c_room)
{
	return sqrt(rooms[c_room].w*rooms[c_room].w + rooms[c_room].h*rooms[c_room].h);
}

void MakeSpecialRooms()
{
	int i, j;
	int c_tier;
	int c_room;
	int biggest_room_sz = 0;
	int biggest_room_n = -1;
	int rtyp[8] = {0};
	int ctyp;
	int x, y;
	
	// Special rooms are:
	// - Boss rooms @ 500, 1000, 1500, 2000, 2500, 3000
	// - Artifact rooms (biggest non-boss room of a given tier)
	//		Tiers: 5-9  10-14  15-19  20-24  25-29  30-34  35-39  40-44
	
	// boss rooms
	for (i = 0; i < 3; i++) {
		c_room = i*1000+999;
		rooms[c_room].room_type = 2;
		rooms[c_room].room_param = i;
	}
	// power object rooms
	for (i = 0; i < 3; i++) {
		c_room = i*1000+499;
		rooms[c_room].room_type = 5;
		rooms[c_room].room_param = i;
	}
	
	// artifact rooms
	for (c_tier = 0; c_tier < 8; c_tier++) {
		biggest_room_sz = 0;
		for (c_room = 0; c_room < 3000; c_room++) {
			if (rooms[c_room].room_type == 0) {
				if (rooms[c_room].s_dist >= (c_tier*5+5)) {
					if (rooms[c_room].s_dist <= (c_tier*5+9)) {
						if (RoomSize(c_room) > biggest_room_sz) {
							biggest_room_sz = RoomSize(c_room);
							biggest_room_n = c_room;
						}
					}
				}
			}
		}
		rooms[biggest_room_n].room_type = 3;
		
		// pick a #
		for (;;) {
			ctyp = rand()%8;
			if (rtyp[ctyp] == 0) {
				rtyp[ctyp] = 1;
				break;
			}
		}
		
		rooms[biggest_room_n].room_param = ctyp;
		
		//printf("Artifact room for tier %d is room %d (size %d), with artifact %d\n", c_tier, biggest_room_n, biggest_room_sz, ctyp);
	}
	
	// place of power
	// The room with the highest s_dist that is not of any other type
	
	for (i = 0; i < 3000; i++) {
		if (rooms[i].s_dist > rooms[place_of_power].s_dist) {
			if (rooms[i].room_type == 0) {
				place_of_power = i;
			}
		}
	}

	rooms[place_of_power].room_type = 6;
	
	// Now place some checkpoints in the remaining rooms
	// Normally, we would have a checkpoint for every 30
	// rooms, BUT since we aren't using that method any
	// more, we will simply use an equivalent--namely, to
	// divide the map into an 8x8 grid and place one
	// checkpoint per square

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			j = -1;
			for (i = 0; i < 20; i++) {
				j = GetRoom(rand() % 64 + x * 64, rand() % 64 + y * 64);
                
                if (j >= 0) {
                    if (rooms[j].room_type == 0) {
                        Put(rooms[j].x + rooms[j].w / 2, rooms[j].y + rooms[j].h / 2, 25, j);
                        rooms[j].checkpoint = 1;
                        break;
                    }
                }
			}
		}
	}
	
	next_check--;
}

int Generate()
{
	int attempts = 0;
	int i;
	int correct_dist = 0;
	int maxdist = 0;
	Uint32 last_ticks = SDL_GetTicks(), cur_ticks;
	rdir = rand()%4;
	NewRoom(map.w / 2 - 20 / 2, map.h / 2 - 15 / 2, 20, 15, -1);
	
	for (attempts = 0; attempts < 100000; attempts++) {
        assert(map.w == 512);
		AddChild(rndval(rndval(0, total_rooms-1), total_rooms-1));
		if ((cur_ticks = SDL_GetTicks()) >= last_ticks + PROGRESS_DELAY_MS) {
			LoadingScreen(0, (float)total_rooms / 3000.0);
			last_ticks = cur_ticks;
		}
		if (total_rooms == 3000) break;
	}
	
	if ((total_rooms < 3000)||(DoRepeat == 1)) {
		DoRepeat = 0;
		ResetLevel();
		return 0;
	}
	
	RecurseSetDist();
	
	for (i = 0; i < 3000; i++) {
		if (rooms[i].s_dist > maxdist) {
			maxdist = rooms[i].s_dist;
		}
		
		if (rooms[i].s_dist >= 50) {
			correct_dist = 1;
		}
	}
	
	if (correct_dist == 0) {
		//printf("Dist fail (only %d)\n", maxdist);
		DoRepeat = 0;
		ResetLevel();
		return 0;
	}
	
	//printf("Rooms: %d\n", total_rooms);
	
	MakeSpecialRooms();
	
	map.totalRooms = total_rooms;
	return 1;
}

