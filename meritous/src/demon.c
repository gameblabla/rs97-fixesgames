//
//   demon.c
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
#include <assert.h>

#include "levelblit.h"
#include "mapgen.h"
#include "save.h"
#include "audio.h"
#include "boss.h"
#include "tiles.h"

SDL_Surface *reticle;
SDL_Surface *inrange;
SDL_Surface *invis_enemy;

int searched[3000];
int searchdist[3000];
int csearch;
int room_active[3000];

int fc_open = 0;

int max_activate_dist = 0;

// enemy

int sqr(int x)
{
	return x*x;
}

struct enemy {
	int x, y;
	int room;
	int lives;
	int deaths;
	int str;
	int curr_follow;
	
	int followdepth;
	
	int enemy_type;
	
	SDL_Surface *image;
	int blit_pos;

	int creationcost;

	int teleport_v;
	int speed;
	int t;
	int active;
	int fire_rate;
	
	int last_room;
	int p_last_room;
	
	float move_dir;

	int min_gems;
	int max_gems;

	struct enemy *next;
	struct enemy *next_active;

	int delete_me;
	int dying;
	
	struct RoomConnection *m_exit;
};

// shot

struct bullet {
	float x, y;
	struct enemy *firer;
	struct bullet *parent;
	int room;
	float speed;
	float dir;
	int img;
	// img:	0 = bullet
	//		1 = star
	//		2 = laser
	// 		3 = ministar
	//		4 = rocket
	//		5 = replishot
	//		6 = laserstar
	//		7 = proxy
	//		8 = remote laser
	int t;
	
	int invuln;
	float natural_dir;
	
	int fire_time;
	int duration;
	float turn;
	
	int shield_damage;

	struct bullet *next;

	int dying;
	int delete_me;
};

struct diamond {
	int x, y;
	int room;
	int t;
	struct diamond *next;
	struct diamond *next_in_room;
	struct diamond *prv_in_room;
	int value;
	int delete_me;
};

struct enemyloc {
	struct enemy *e;
	struct enemyloc *n;
};

int total_enemies = 0;
int killed_enemies = 0;
int total_bullets = 0;
int active_enemies = 0;
int total_gems = 0;

struct enemy *enemy_stack = NULL;
struct bullet *bullet_stack = NULL;
struct enemy *active_stack = NULL;
struct diamond *gem_stack = NULL;

struct diamond *room_gems[3000] = {NULL};

struct enemyloc *enemy_loc_stack[20][20] = {{NULL}};

struct enemy *CreateEnemy(int enemy_x, int enemy_y, int enemy_room);
struct enemy *CreateEnemyEx(int enemy_x, int enemy_y, int enemy_room, int enemy_type);
void SCreateGem(int x, int y, int r, int v);
void ActivateEnemies(int room);
void SoupUpEnemies();

void SpawnLaser(int x, int y, float dir, int fire_time, int duration, float turn, int dmg);

void DestroyThings()
{
	struct enemy *ec, *ed;
	struct bullet *bc, *bd;
	struct diamond *dc, *dd;
	int i;
	
	ec = enemy_stack;
	bc = bullet_stack;
	dc = gem_stack;
	
	while (ec) {
		ed = ec;
		ec = ec->next;
		free(ed);
	}
	while (bc) {
		bd = bc;
		bc = bc->next;
		free(bd);
	}
	while (dc) {
		dd = dc;
		dc = dc->next;
		free(dd);
	}
	
	enemy_stack = NULL;
	bullet_stack = NULL;
	gem_stack = NULL;
	active_stack = NULL;
	
	for (i = 0; i < 3000; i++) {
		room_gems[i] = NULL;
	}
}

struct GRD_Box {
	int d[60][60];
};
int GetRoomDist(int room1, int room2);
int FindRoomDist(int room1, int room2)
{
	struct RoomConnection *follow;
	int mdist = 1000000;
	int cdist;
	
	if (room1 == room2) return 0;
	fc_open++;
	
	searched[room1] = csearch;
	
	follow = rooms[room1].con;
	while (follow != NULL) {
		if (searched[follow->c] != csearch) {
			cdist = GetRoomDist(follow->c, room2);
		
			if (cdist < mdist) {
				mdist = cdist;
			}
		}
		follow = follow->n;
	}
	fc_open--;
	return mdist;
}

int GetRoomDist(int room1, int room2)
{
	int gx, gy, dx, dy;
	int temp;
	int ix, iy;
	static struct GRD_Box *g[50][50] = {{NULL}};
	
	fc_open++;
	
	if (room2 < room1) {
		temp = room2;
		room2 = room1;
		room1 = temp;
	}
	
	gx = room1 / 60;
	dx = room1 % 60;
	gy = room2 / 60;
	dy = room2 % 60;
	
	if (g[gx][gy] == NULL) {
		g[gx][gy] = malloc(sizeof(struct GRD_Box));
		for (iy = 0; iy < 60; iy++) {
			for (ix = 0; ix < 60; ix++) {
				g[gx][gy]->d[ix][iy] = -1;
			}
		}
	}
	
	if (g[gx][gy]->d[dx][dy] == -1) {
		g[gx][gy]->d[dx][dy] = FindRoomDist(room1, room2);
	}
	fc_open--;
	return g[gx][gy]->d[dx][dy];
}

void AddEnemyLoc(struct enemy *e, int x, int y)
{
	struct enemyloc *next;
	x+=1;
	y+=1;
	next = enemy_loc_stack[x][y];
	enemy_loc_stack[x][y] = malloc(sizeof(struct enemyloc));
	enemy_loc_stack[x][y]->n = next;
	enemy_loc_stack[x][y]->e = e;
}

struct enemyloc *GetEnemyLoc(int s_x, int s_y)
{
	return enemy_loc_stack[s_x / 1100 + 1][s_y / 1100 + 1];
}

void AddEnemyPos(struct enemy *e)
{
	int x_loc, y_loc, ix, iy;
	x_loc = (e->x) / 1100;
	y_loc = (e->y) / 1100;
	AddEnemyLoc(e, x_loc, y_loc);
	ix = ((e->x - screen->w) / 1100);
	iy = ((e->y - screen->h) / 1100);
	if (x_loc != ix) {
		AddEnemyLoc(e, ix, y_loc);
	}
	if (y_loc != iy) {
		AddEnemyLoc(e, x_loc, iy);
	}
	if ((x_loc != ix) && (y_loc != iy)) {
		AddEnemyLoc(e, ix, iy);
	}
}

void WriteEnemyData()
{
	struct enemy *ptr;
	int i = 0, n = 0;
	Uint32 last_ticks = SDL_GetTicks(), cur_ticks;

	ptr = enemy_stack;
	while (ptr != NULL) {
		if (!ptr->delete_me) n++;
		ptr = ptr->next;
	}
	FWInt(n);
	ptr = enemy_stack;
	while (ptr != NULL) {
		if (!ptr->delete_me) {
			FWInt(ptr->x);
			FWInt(ptr->y);
			FWInt(ptr->enemy_type);
			i++;
			if ((cur_ticks = SDL_GetTicks()) >= last_ticks + PROGRESS_DELAY_MS) {
				SavingScreen(2, (float)i / (float)n);
				last_ticks = cur_ticks;
			}
		}
		ptr = ptr->next;
	}
}

void WriteGemData()
{
	struct diamond *ptr;
	int i = 0, n = 0;
	Uint32 last_ticks = SDL_GetTicks(), cur_ticks;

	ptr = gem_stack;
	while (ptr != NULL) {
		if (!ptr->delete_me) n++;
		ptr = ptr->next;
	}
	FWInt(n);
	ptr = gem_stack;
	while (ptr != NULL) {
		if (!ptr->delete_me) {
			FWInt(ptr->x);
			FWInt(ptr->y);
			FWInt(ptr->value);
			
			i++;
			if ((cur_ticks = SDL_GetTicks()) >= last_ticks + PROGRESS_DELAY_MS) {
				SavingScreen(3, (float)i / (float)n);
				last_ticks = cur_ticks;
			}
		}
		ptr = ptr->next;
	}
}

void WriteCreatureData()
{
	FWInt(total_enemies);
	FWInt(killed_enemies);

	WriteEnemyData();
	WriteGemData();
}

void ReadEnemyData()
{
	int i, n;
	int x, y, room, t;
	Uint32 last_ticks = SDL_GetTicks(), cur_ticks;

	n = FRInt();
	
	for (i = 0; i < n; i++) {
		x = FRInt();
		y = FRInt();
		/* x and y are in pixels; map tiles are in units of 32x32 */
		if (x < 0 || x >= map.w * 32 || y < 0 || y >= map.h * 32
		 || (room = map.r[(y / 32) * map.w + (x / 32)]) == -1) {
			fprintf(stderr, "An enemy in the save file is out of bounds\n");
			exit(2);
		}
		t = FRInt();
		CreateEnemyEx(x, y, room, t);
		total_enemies--;
		
		if ((cur_ticks = SDL_GetTicks()) >= last_ticks + PROGRESS_DELAY_MS) {
			LoadingScreen(2, (float)i / (float)n);
			last_ticks = cur_ticks;
		}
	}
	LoadingScreen(2, 1);
}

void ReadGemData()
{
	int i, n;
	int x, y, room, value;
	Uint32 last_ticks = SDL_GetTicks(), cur_ticks;

	n = FRInt();
	
	for (i = 0; i < n; i++) {
		x = FRInt();
		y = FRInt();
		/* x and y are in pixels; map tiles are in units of 32x32 */
		if (x < 0 || x >= map.w * 32 || y < 0 || y >= map.h * 32
		 || (room = map.r[(y / 32) * map.w + (x / 32)]) == -1) {
			fprintf(stderr, "A PSI crystal in the save file is out of bounds\n");
			exit(2);
		}
		value = FRInt();
		SCreateGem(x, y, room, value);
		
		if ((cur_ticks = SDL_GetTicks()) >= last_ticks + PROGRESS_DELAY_MS) {
			LoadingScreen(3, (float)i / (float)n);
			last_ticks = cur_ticks;
		}
	}
	LoadingScreen(3, 1);
}

void ActivateVisited()
{
	int i;
	Uint32 last_ticks = SDL_GetTicks(), cur_ticks;

	for (i = 0; i < 3000; i++) {
		if (rooms[i].visited) {
			ActivateEnemies(i);
		}
		if ((cur_ticks = SDL_GetTicks()) >= last_ticks + 50) {
			LoadingScreen(4, (float)i / 3000.0);
			last_ticks = cur_ticks;
		}
	}
	LoadingScreen(4, 1);
}

void ReadCreatureData()
{
	total_enemies = FRInt();
	killed_enemies = FRInt();

	ReadEnemyData();
	ReadGemData();
	ActivateVisited();
}

SDL_Surface *enemy_sprites[10];

struct enemy * AllocateEnemy()
{
	return malloc(sizeof(struct enemy));
}

struct bullet * AllocateBullet()
{
	return malloc(sizeof(struct bullet));
}

struct diamond * AllocateGem()
{
	return malloc(sizeof(struct diamond));
}

struct enemy *CreateEnemy(int enemy_x, int enemy_y, int enemy_room)
{
	int enemy_type;
	
	enemy_type = rand() % (rooms[enemy_room].s_dist / 5 + 1);
	if (rooms[enemy_room].room_type == 5) enemy_type += rand()%3;
	if (enemy_type > 8) enemy_type = rand()%3+6;
	
	if (rooms[enemy_room].s_dist >= 15) {
		if (rand()%64 == 0) {
			enemy_type = 9;
		}
	}
	
	
	return CreateEnemyEx(enemy_x, enemy_y, enemy_room, enemy_type);
}

struct enemy *CreateEnemyEx(int enemy_x, int enemy_y, int enemy_room, int enemy_type)
{
	struct enemy *new_enemy;

	new_enemy = AllocateEnemy();
	new_enemy->x = enemy_x;
	new_enemy->y = enemy_y;
	new_enemy->room = enemy_room;
	
	rooms[enemy_room].enemies++;
	new_enemy->deaths = 0;
	new_enemy->t = rand() % 65536;
	new_enemy->active = 0;
	new_enemy->curr_follow = -1;
	new_enemy->teleport_v = 0;

	new_enemy->m_exit = NULL;
	
	new_enemy->dying = 0;
	new_enemy->delete_me = 0;
	
	new_enemy->last_room = -1;
	new_enemy->p_last_room = -1;
	
	new_enemy->enemy_type = enemy_type;
	
	new_enemy->followdepth = 4;

	switch (enemy_type) {
		case 0:
			new_enemy->image = enemy_sprites[0];
			new_enemy->lives = 1;
			new_enemy->str = 20;
			new_enemy->speed = 3;
			new_enemy->fire_rate = 20;
			new_enemy->min_gems = 0;
			new_enemy->max_gems = 3;
			new_enemy->creationcost = 1;
			break;
		case 1:
			new_enemy->image = enemy_sprites[1];
			new_enemy->lives = 1;
			new_enemy->str = 50;
			new_enemy->speed = 4;
			new_enemy->fire_rate = 25;
			new_enemy->min_gems = 2;
			new_enemy->max_gems = 6;
			new_enemy->creationcost = 1;
			break;
		case 2:
			new_enemy->image = enemy_sprites[2];
			new_enemy->lives = 1;
			new_enemy->str = 180;
			new_enemy->speed = 5;
			new_enemy->fire_rate = 40;
			new_enemy->min_gems = 8;
			new_enemy->max_gems = 15;
			new_enemy->creationcost = 1;
			break;
		case 3:
			new_enemy->image = enemy_sprites[3];
			new_enemy->lives = 2;
			new_enemy->str = 220;
			new_enemy->speed = 3;
			new_enemy->fire_rate = 24;
			new_enemy->min_gems = 12;
			new_enemy->max_gems = 20;
			new_enemy->creationcost = 1;
			break;
		case 4:
			new_enemy->image = enemy_sprites[4];
			new_enemy->lives = 1;
			new_enemy->str = 360;
			new_enemy->speed = 3;
			new_enemy->fire_rate = 32;
			new_enemy->min_gems = 18;
			new_enemy->max_gems = 32;
			new_enemy->creationcost = 2;
			break;
		case 5:
			new_enemy->image = enemy_sprites[5];
			new_enemy->lives = 1;
			new_enemy->str = 450;
			new_enemy->speed = 3;
			new_enemy->fire_rate = 2;
			new_enemy->min_gems = 32;
			new_enemy->max_gems = 64;
			new_enemy->creationcost = 2;
			break;
		case 6:
			new_enemy->image = enemy_sprites[6];
			new_enemy->lives = 2;
			new_enemy->str = 450;
			new_enemy->speed = 4;
			new_enemy->fire_rate = 10;
			new_enemy->min_gems = 50;
			new_enemy->max_gems = 100;
			new_enemy->creationcost = 2;
			break;
		case 7:
			new_enemy->image = enemy_sprites[7];
			new_enemy->lives = 1;
			new_enemy->str = 500;
			new_enemy->speed = 4;
			new_enemy->fire_rate = 27;
			new_enemy->min_gems = 80;
			new_enemy->max_gems = 160;
			new_enemy->creationcost = 3;
			break;
		case 8:
			new_enemy->image = enemy_sprites[8];
			new_enemy->lives = 4;
			new_enemy->str = 500;
			new_enemy->speed = 2;
			new_enemy->fire_rate = 8;
			new_enemy->min_gems = 200;
			new_enemy->max_gems = 400;
			new_enemy->creationcost = 4;
			break;
		case 9:
			new_enemy->image = enemy_sprites[0];
			new_enemy->lives = 1;
			new_enemy->str = rooms[enemy_room].s_dist * 20;
			new_enemy->speed = 3;
			new_enemy->fire_rate = 21;
			new_enemy->min_gems = 300;
			new_enemy->max_gems = 600;
			new_enemy->creationcost = 3;
			break;
		case 10:
			new_enemy->image = enemy_sprites[9];
			new_enemy->lives = 8;
			new_enemy->str = 500;
			new_enemy->speed = 1;
			new_enemy->fire_rate = 4;
			new_enemy->min_gems = 5000;
			new_enemy->max_gems = 6000;
			new_enemy->followdepth = 8;
			new_enemy->creationcost = 6;
			break;
	}
	
	if (training) {
		new_enemy->str = new_enemy->str * 4 / 5;
		new_enemy->fire_rate += (new_enemy->fire_rate / 2);
		new_enemy->speed *= 2;
	}
	
	if (rooms[new_enemy->room].room_type == 5) {
		new_enemy->str = new_enemy->str * 3 / 2;
		if (new_enemy->str > 1500) new_enemy->str = 1500;
		new_enemy->fire_rate = new_enemy->fire_rate - 1;
	}

	new_enemy->blit_pos = (rand()%(new_enemy->image->w / (new_enemy->image->h/new_enemy->lives)))*(new_enemy->image->h/new_enemy->lives);
	new_enemy->next_active = NULL;

	new_enemy->next = enemy_stack;
	enemy_stack = new_enemy;

	AddEnemyPos(new_enemy);

	total_enemies++;
	
	return new_enemy;
}

void SCreateGem(int x, int y, int r, int v)
{
	struct diamond *new_gem;

	if (TileData[Get(x / 32, y / 32)].Is_Solid) {
		return;
	}
	if (GetRoom(x / 32, y / 32) != r) {
		return;
	}
	if (v == 0) {
		return;
	}
	
	new_gem = AllocateGem();
	
	new_gem->x = x;
	new_gem->y = y;
	new_gem->room = r;
new_gem->delete_me = 0;
	new_gem->t = rand()%65536;
	new_gem->next = gem_stack;
	new_gem->next_in_room = room_gems[r];
	
	if (room_gems[r] != NULL) {
		room_gems[r]->prv_in_room = new_gem;
	}
	
	new_gem->value = v;
	new_gem->prv_in_room = NULL;
	gem_stack = new_gem;
	room_gems[r] = new_gem;

	total_gems++;	
}

void CreateGem(int x, int y, int r, int v)
{
	if (v == 0) return;
	if ( (rand()%1000) < ((int)log(v)/4 + (player_hp == 1)*5 + 2) ) {
		SCreateGem(x, y, r, 31337);
	} else {
		SCreateGem(x, y, r, v);
	}
}

float PlayerDir(int x, int y)
{
	float dy = player_y+12 - y;
	float dx = player_x+8 - x;
	return atan2(dy, dx);
}

int PlayerDist(int x, int y)
{
	int d = sqrt(sqr(x-(player_x+8))+sqr(y-(player_y+12)));
	return d;
}

struct bullet *CreateBullet(int x, int y, struct enemy *firer, int bullet_type, float dir, float spd)
{
	struct bullet *new_shot;

	new_shot = AllocateBullet();
	new_shot->x = x;
	new_shot->y = y;
	new_shot->firer = firer;
	if (firer != NULL) {
		new_shot->room = firer->room;
	} else {
		new_shot->room = GetRoom(x / 32, y / 32);
	}
	new_shot->dying = 0;
	new_shot->delete_me = 0;
	new_shot->t = rand() % 65536;
	new_shot->dir = dir;
	new_shot->speed = spd;
	new_shot->invuln = 0;
	new_shot->parent = NULL;

	switch (bullet_type) {
		case 0:
			new_shot->img = 0;
			break;
		case 1:
			new_shot->img = 1;
			new_shot->invuln = 1;
			break;
		case 2:
			new_shot->img = 2;
			new_shot->invuln = 1;
			new_shot->t = 0;
			new_shot->fire_time = 30;
			new_shot->duration = 30;
			new_shot->turn = 0.0;
			new_shot->speed = 0.0;
			break;
		case 3:
			new_shot->img = 3;
			break;
		case 4:
			new_shot->img = 4;
			new_shot->invuln = 1;
			break;
		case 5:
			new_shot->img = 5;
			new_shot->t = 0;
			break;
		case 6:
			new_shot->img = 6;
			new_shot->invuln = 1;
			break;
		case 7:
			new_shot->img = 7;
			new_shot->invuln = 1;
			break;
		case 8:
			new_shot->img = 8;
			new_shot->invuln = 1;
			break;
	}
	if (training) {
		new_shot->speed *= 0.8;
	}
	new_shot->next = bullet_stack;
	bullet_stack = new_shot;

	total_bullets++;
	return new_shot;
}

struct bullet *FireLaser(int x, int y, struct enemy *firer, float dir, int fire_time, int duration, float turn, int dmg)
{
	int f_total;
	struct bullet *b;
	b = CreateBullet(x, y, firer, 2, dir, 0);
	b->fire_time = fire_time;
	b->duration = duration;
	b->turn = turn;
	b->shield_damage = dmg;
	
	if (training) {
		f_total = b->fire_time + b->duration;
		if (b->duration > 1) {
			b->duration /= 2;
			b->fire_time = f_total - b->duration;
		}
		b->shield_damage = (b->shield_damage + 1) / 2;
	}
	return b;
}

void InitEnemySprites()
{
	enemy_sprites[0] = IMG_Load("dat/i/mons1.png");
	SDL_SetColorKey(enemy_sprites[0], SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
	enemy_sprites[1] = IMG_Load("dat/i/mons2.png");
	SDL_SetColorKey(enemy_sprites[1], SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
	enemy_sprites[2] = IMG_Load("dat/i/mons3.png");
	SDL_SetColorKey(enemy_sprites[2], SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
	enemy_sprites[3] = IMG_Load("dat/i/mons4.png");
	SDL_SetColorKey(enemy_sprites[3], SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
	enemy_sprites[4] = IMG_Load("dat/i/mons5.png");
	SDL_SetColorKey(enemy_sprites[4], SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
	enemy_sprites[5] = IMG_Load("dat/i/mons6.png");
	SDL_SetColorKey(enemy_sprites[5], SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
	enemy_sprites[6] = IMG_Load("dat/i/mons7.png");
	SDL_SetColorKey(enemy_sprites[6], SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
	enemy_sprites[7] = IMG_Load("dat/i/mons8.png");
	SDL_SetColorKey(enemy_sprites[7], SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
	enemy_sprites[8] = IMG_Load("dat/i/mons9.png");
	SDL_SetColorKey(enemy_sprites[8], SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
	enemy_sprites[9] = IMG_Load("dat/i/mons10.png");
	SDL_SetColorKey(enemy_sprites[9], SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);

	reticle = IMG_Load("dat/i/reticle.png");
	SDL_SetColorKey(reticle, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);

	inrange = IMG_Load("dat/i/inrange.png");
	SDL_SetColorKey(inrange, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);

	invis_enemy = IMG_Load("dat/i/hidden_monster.png");
	SDL_SetColorKey(invis_enemy, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
}

void ActivateSingleEnemy(struct enemy *t)
{
	struct enemy *new_active;
	
	if (t->active) return;
	
	if (t->enemy_type != 10) {
		if (t->enemy_type == 9) {
			if (max_activate_dist < 10) return;
		} else {
		
			if (t->enemy_type > (max_activate_dist / 5 + 2)) return;
			
		}
	}
	
	new_active = t;
	new_active->next_active = active_stack;
	active_stack = new_active;
	
	active_enemies += 1;
	t->active = 1;
}

void XActivateSingleEnemy(struct enemy *t)
{
	if (rooms[t->room].room_type == 2) return;
	if (rooms[t->room].room_type == 3) return;
	
	ActivateSingleEnemy(t);
}

void InitEnemies()
{
	int c_room;
	int cr_w, cr_h, cr_x, cr_y;
	int room_size;
	int n_enemies;
	int i;
	int trying;
	int e_x, e_y;
	int nx, ny;
	
	max_activate_dist = 0;

	InitEnemySprites();
	
	for (ny = 0; ny < 20; ny++) {
		for (nx = 0; nx < 20; nx++) {
			enemy_loc_stack[nx][ny] = NULL;
		}
	}
	
	total_enemies = 0;
	killed_enemies = 0;
	total_bullets = 0;
	active_enemies = 0;
	total_gems = 0;
	
	for (i = 0; i < 3000; i++) {
		room_active[i] = 0;
	}

	if (game_load) {
		ReadCreatureData();
		if (current_boss > 0) SoupUpEnemies();
	} else {
		for (c_room = 1; c_room < 3000; c_room++) {
			cr_x = rooms[c_room].x + 1;
			cr_y = rooms[c_room].y + 1;
			cr_w = rooms[c_room].w - 2;
			cr_h = rooms[c_room].h - 2;
			room_size = cr_w * cr_h;
			
			n_enemies = rand() % ((room_size / 4) + 1);
			
			if (rooms[c_room].room_type == 2) {
				n_enemies = 0;
			}
			
			if (rooms[c_room].room_type == 3) {
				n_enemies += (n_enemies + room_size) / 2;
			}
			
			if (rooms[c_room].room_type == 5) {
				n_enemies = 50;
			}

			while (n_enemies > 0) {
				trying = 1;
				while (trying) {
					e_x = cr_x * 32 + 32 + rand() % (cr_w * 32 - 64 + 1);
					e_y = cr_y * 32 + 32 + rand() % (cr_h * 32 - 64 + 1);

					if ((!IsSolid(Get( (e_x-16) /32, (e_y-16) /32)))&&(!IsSolid(Get( (e_x+16) /32, (e_y-16) /32)))) {
						if ((!IsSolid(Get( (e_x-16) /32, (e_y+16) /32)))&&(!IsSolid(Get( (e_x+16) /32, (e_y+16) /32)))) {
							n_enemies -= (CreateEnemy(e_x, e_y, c_room))->creationcost;
							trying = 0;
						}
					}
				}
			}
			if (c_room % 100 == 99) {
				LoadingScreen(2, (float)c_room / 3000.0);
			}
		}
	}


}

int EnemyMovement(struct enemy *e, int move_x, int move_y)
{
	if (!IsSolid(Get( (move_x - 12)/32, (move_y - 12)/32))) {
		if (!IsSolid(Get( (move_x + 12)/32, (move_y - 12)/32))) {
			if (!IsSolid(Get( (move_x - 12)/32, (move_y + 12)/32))) {
				if (!IsSolid(Get( (move_x + 12)/32, (move_y + 12)/32))) {
					e->x = move_x;
					e->y = move_y;
					return 1;
				}
			}
		}
	}
	return 0;
}

// Only activate SOME enemies. A room can only be ZActivated once
void ZActivateEnemies(int room)
{
	struct enemy *t;
	struct RoomConnection *rc;
	
	if (room_active[room]) return;
	room_active[room] = 1;

	t = enemy_stack;

	while (t != NULL) {
		// 1/4 chance of activating each enemy
		if (rand()%4 == 0) {
			if (t->room == room) {
				if (t->active == 0) {
					XActivateSingleEnemy(t);
				}
			}
		}
		t = t->next;
	}
	
	// 1/3 chance of activating each adjacent room
	rc = rooms[room].con;
	while (rc != NULL) {
		if (rand()%3 == 0) {
			ZActivateEnemies(rc->c);
		}
		rc = rc->n;
	}
}

void ActivateEnemies(int room)
{
	struct enemy *t;
	struct RoomConnection *rc;

	t = enemy_stack;
	
	if (rooms[room].s_dist > max_activate_dist) {
		max_activate_dist = rooms[room].s_dist;
	}

	while (t != NULL) {
		if (t->room == room) {
			if (t->active == 0) {
				ActivateSingleEnemy(t);
				
				if (rooms[room].room_type == 3) {
					t->teleport_v = (rand() % 1500) + 50;
				}
			}
		}
		t = t->next;
	}
	
	// 1/2 chance of activating each adjacent room
	rc = rooms[room].con;
	while (rc != NULL) {
		if (rand()%2 == 0) {
			if ((rooms[rc->c].room_type != 2) && (rooms[rc->c].room_type != 3))  {
				ZActivateEnemies(rc->c);
			}
		}
		rc = rc->n;
	}
}

int CanEnterRoom(int room)
{
	if (room == 0) return 0;
	if (rooms[room].room_type == 2) return 0;
	if (rooms[room].room_type == 3) return 0;
	if (rooms[room].room_type == 5) return 0;
	if (rooms[room].room_type == 6) return 0;

	if (artifacts[11]) {
		if (rooms[room].enemies > 3) {
			return 0;
		}
	}
	
	return 1;
}
int CanLeaveRoom(int room)
{
	if (room == 0) return 0;
	if (rooms[room].room_type == 2) return 0;
	if (rooms[room].room_type == 3) return 0;
	if (rooms[room].room_type == 5) return 0;
	if (rooms[room].room_type == 6) return 0;

	return 1;
}

int RecurseFind(struct enemy *e, int room, int depth)
{
	struct RoomConnection *follow;
	int dpth;
	int mindpth = 1000000;
	follow = rooms[room].con;
	
	if (CanEnterRoom(room) == 0) return 0;
	
	if (room == player_room) return depth+1;
	if (searched[room] == csearch) {
		if (searchdist[room] < depth)
			return 0;
	}
	if (depth > e->followdepth) return 0;
	if ((e->last_room == room) && (e->p_last_room == player_room)) return 0;
	
	searched[room] = csearch;
	searchdist[room] = depth;

	while (follow != NULL) {
		if ((dpth = RecurseFind(e, follow->c, depth+1)) > 0) {
			if (dpth < mindpth) {
				mindpth = dpth;
			}
		}
		follow = follow->n;
	}
	
	if (mindpth != 1000000) {
		return mindpth;
	}
	
	return 0;
	
}

int FollowPlayer(struct enemy *e, struct RoomConnection **rcon)
{
	struct RoomConnection *follow;
	int mindepth = 1000000;
	int newdepth;
	int mdist = 1000000;
	int ndist = 0;
	int rdepth[4000];
	
	return 0;
	follow = rooms[e->room].con;
	
	while (follow != NULL) {
		if (follow->c == player_room) {
			if (CanEnterRoom(follow->c)) {
				*rcon = follow;
				e->curr_follow = (*rcon)->c;
				return 1;
			}
		}
		follow = follow->n;
	}
	
	// Recursively follow the player, to a certain depth
	follow = rooms[e->room].con;
	csearch = rand();
	
	// Are we already following the player into a room?
	
	/*if (e->curr_follow != -1) {
		// See if this room is the best FIRST
		newdepth = RecurseFind(e, e->curr_follow, 0);
		if (newdepth > 0) {
			mindepth = newdepth;
			*rcon = follow;
		}
	}*/
	
	while (follow != NULL) {
		if (CanEnterRoom(follow->c)) {
			newdepth = RecurseFind(e, follow->c, 0);
			rdepth[follow->c] = newdepth;
			if (newdepth > 0) {
				if (mindepth > newdepth) {
					mindepth = newdepth;
					*rcon = follow;
				}
			}
		}
		follow = follow->n;
	}
	if (mindepth != 1000000) {
		follow = rooms[e->room].con;
		while (follow != NULL) {
			if (CanEnterRoom(follow->c)) {
				newdepth = rdepth[follow->c];
				if (newdepth == mindepth) {
					ndist = PlayerDist(follow->x*32+16, follow->y*32+16);
					if (ndist < mdist) {
						mdist = ndist;
						*rcon = follow;
					}
				}
			}
			follow = follow->n;
		}
	
		e->curr_follow = (*rcon)->c;
		return mindepth;
	}
	return 0;
}

void KillEnemy(struct enemy *t)
{
	static int lastkill = 0;
	int ct;
	
	if (t->dying > 0) return;
	if (t->teleport_v > 0) return;
	if (t->delete_me) return;
	
	ct = SDL_GetTicks();
	
	if ((ct - lastkill) > 100) {
		SND_Pos("dat/a/enemyhit.wav", 128, PlayerDist(t->x, t->y));
		lastkill = ct;
	}
	
	t->dying = 1;
}

void ArtifactRoomUnlock(int room)
{
	struct enemy *e;
	int x, y, rx, ry, rt;
	int placed;
	int tot_treasures;
	
	e = active_stack;
	while (e != NULL) {
		if ((e->delete_me == 0) && (e->room == room)) {
			return;
		}
		e = e->next_active;
	}
	
	// unlock doors
	
	for (y = 0; y < rooms[room].h; y++) {
		for (x = 0; x < rooms[room].w; x++) {
			rx = x + rooms[room].x;
			ry = y + rooms[room].y;
			rt = Get(rx, ry);
			
			if ((rt >= 21) && (rt <= 24)) {
				Put(rx, ry, rt - 21 + 13, room);
			}
		}
	}
	
	// place treasure
	placed = 0;
	tot_treasures = 2 + rand() % (rooms[room].s_dist / 8 + 1);
	while (placed < tot_treasures) {
		x = rooms[room].x + (rand() % (rooms[room].w - 2));
		y = rooms[room].y + (rand() % (rooms[room].h - 2));
		//printf("Attempting %d, %d\n", x, y);
		if ((x+y)%2 == (placed>0)) {
			//printf("Correct placement type\n");
			if (!IsSolid(Get(x, y))) {
				//printf("Not solid\n");
				Put(x, y, 26, room);
				placed++;
			}
		}
		fflush(stdout);
	}
	
	// sign room
	rooms[room].room_type = 4;
}

void EnemySound(int t, int dist)
{
	static int last_e_sound = 0;
	static int last_delay = 150;
	int curr_e_sound = SDL_GetTicks();
	
	if ((curr_e_sound - last_delay) < last_e_sound) {
		return;
	}
	
	switch (t) {
		case 0:
			SND_Pos("dat/a/mons0shot.wav", 48, dist);
			last_delay = 200;
		break;
		case 1:
			SND_Pos("dat/a/mons1shot.wav", 112, dist);
			last_delay = 500;
		break;
		case 2:
			SND_Pos("dat/a/mons2shot.wav", 110, dist);
			last_delay = 1000;
		break;
		case 3:
			SND_Pos("dat/a/mons3shot.wav", 110, dist);
			last_delay = 500;
		break;
		case 4:
			SND_Pos("dat/a/mons4shot.wav", 110, dist);
			last_delay = 900;
		break;
		case 5:
			SND_Pos("dat/a/mons5shot.wav", 80, dist);
			last_delay = 60;
		break;
		case 6:
			SND_Pos("dat/a/mons6shot.wav", 110, dist);
			last_delay = 1000;
		break;
		case 7:
			SND_Pos("dat/a/mons7shot.wav", 110, dist);
			last_delay = 600;
		break;
		case 8:
			SND_Pos("dat/a/mons8shot.wav", 110, dist);
			last_delay = 700;
		break;
		case 9:
			SND_Pos("dat/a/mons9shot.wav", 110, dist);
			last_delay = 242;
		break;
		case 10:
			SND_Pos("dat/a/mons10shot.wav", 110, dist);
			last_delay = 250;
		break;
		
		
		default:
		break;
	}
	
	last_e_sound = curr_e_sound;
}

void MoveEnemy(struct enemy *e)
{
	int n_gems;
	int i;
	int move_x, move_y;
	int door_x=0, door_y=0;
	int enemy_fire_type;
	int actual_lives;
	int can_move = 0;
	struct RoomConnection *con_traverse;
	struct RoomConnection *rcon;
	int nearest;
	struct bullet *b;
	e->t++;
	float dp;
	float dpf;
	if (e->teleport_v > 0) {
		e->teleport_v--;
		return;
	}
	
	if (e->enemy_type < 10) {
		enemy_fire_type = e->enemy_type;
	} else {
		enemy_fire_type = rand()%10;
	}
	
	if (e->dying == 0) {
		if ((e->t % e->fire_rate) == 0) {
			if (e->room == player_room) {
				switch (enemy_fire_type) {
					case 0:
						EnemySound(0, PlayerDist(e->x, e->y));
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y), 1.5);
						if (e->enemy_type==10) e->fire_rate = 20;
					break;
					
					case 1:
						EnemySound(1, PlayerDist(e->x, e->y));
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)+0.1, 2.1);
						CreateBullet(e->x, e->y, e, 3, PlayerDir(e->x, e->y), 2.4);
						CreateBullet(e->x, e->y, e, 3, PlayerDir(e->x, e->y)-0.1, 2.1);
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y), 1.7);
						if (e->enemy_type==10) e->fire_rate = 25;
					break;
					
					case 2:
						EnemySound(2, PlayerDist(e->x, e->y));
						for (dp = 0; dp < M_PI*2; dp += 0.25) {
							CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)+dp, 1.1);
						}
						dp = RandomDir();
						for (i = 0; i < 20; i++) {
							CreateBullet(e->x, e->y, e, 0, dp, (float)i * 0.1 + 2.5);
						}
						if (e->enemy_type==10) e->fire_rate = 40;
					break;
					
					case 3:
						EnemySound(3, PlayerDist(e->x, e->y));
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y), 2.5);
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)+0.1, 2.5);
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)+0.2, 2.5);
						CreateBullet(e->x, e->y, e, 1, PlayerDir(e->x, e->y), 1.5);
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)-0.2, 2.2);
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)-0.3, 2.2);
						if (e->enemy_type==10) e->fire_rate = 24;
					break;
					
					case 4:
						EnemySound(4, PlayerDist(e->x, e->y));
						for (dp = 0; dp < M_PI * 0.66; dp += 0.2) {
							CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)+0.5 - dp, 1.8+(dp/2));
							CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)-0.4 + dp, 1.8+(dp/2));
						}
						FireLaser(e->x, e->y, e, PlayerDir(e->x, e->y), 24, 4, 20.0, 6);
						if (e->enemy_type==10) e->fire_rate = 32;
					break;
					
					case 5:
						EnemySound(5, PlayerDist(e->x, e->y));
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y), 4);
						if (e->enemy_type==10) e->fire_rate = 2;
					break;
					
					case 6:
						EnemySound(6, PlayerDist(e->x, e->y));
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)+0.25, 6);
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)+0.25, 4);
		
						CreateBullet(e->x, e->y, e, 5, PlayerDir(e->x, e->y) - 0.05, 4.99);
						CreateBullet(e->x, e->y, e, 5, PlayerDir(e->x, e->y), 4.99);
						CreateBullet(e->x, e->y, e, 5, PlayerDir(e->x, e->y) + 0.05, 4.99);
		
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)-0.25, 5);
						CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)-0.25, 3);
						if (e->enemy_type==10) e->fire_rate = 10;
					break;
					
					case 7:
						EnemySound(7, PlayerDist(e->x, e->y));
						for (dp = 0; dp < M_PI * 0.66; dp += 0.1) {
							CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)+0.5 - dp, 1.8+(dp/2));
							CreateBullet(e->x, e->y, e, 3, PlayerDir(e->x, e->y)-0.4 + dp, 1.8+(dp/2));
		
							CreateBullet(e->x, e->y, e, 3, PlayerDir(e->x, e->y)+0.5 - dp*2, 4+(dp*2));
							CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y)-0.4 + dp*2, 4+(dp*2));
						}
						FireLaser(e->x, e->y, e, (float)e->t / 25.0, 16, 10, 0.08, 4);
						FireLaser(e->x, e->y, e, (float)e->t / 25.0 + M_PI*2/4, 16, 10, 0.08, 4);
						FireLaser(e->x, e->y, e, (float)e->t / 25.0 + M_PI*4/4, 16, 10, 0.08, 4);
						FireLaser(e->x, e->y, e, (float)e->t / 25.0 + M_PI*6/4, 16, 10, 0.08, 4);
						if (e->enemy_type==10) e->fire_rate = 27;
					break;
		
					case 8:
						EnemySound(8, PlayerDist(e->x, e->y));
						for (dp = 0; dp < 1; dp += 0.1) {
							CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y) + M_PI/2 + dp, 3 + dp/2);
							CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y), 4 + dp);
							CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y) - M_PI/2 - dp, 3 + dp/2);
						}
						CreateBullet(e->x, e->y, e, 6, (e->t / e->fire_rate)*0.7, 2);
						if (e->enemy_type==10) e->fire_rate = 8;
					break;
					
					case 9:
					    i = rand()%((rand()%(PlayerDist(e->x, e->y)+1))+1);
					    if (i < 15) {
							EnemySound(9, PlayerDist(e->x, e->y));
							dpf = (float)(2000 - e->str) / 2500.0;
							for (dp = 0; dp < 1; dp += dpf) {
								(CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y) + 0.1 - dp/5.0, 3.5 + dp/3.0));
								CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y) + (2*M_PI / 3) + 0.1 - dp/5.0, 3.5 + dp/3.0);
								CreateBullet(e->x, e->y, e, 0, PlayerDir(e->x, e->y) + (4*M_PI / 3) + 0.1 - dp/5.0, 3.5 + dp/3.0);
								
								CreateBullet(e->x, e->y, e, 4, PlayerDir(e->x, e->y) + 0.1 - dp/5.0, 7 + dp*5);
								CreateBullet(e->x, e->y, e, 4, PlayerDir(e->x, e->y) + (2*M_PI / 3) + 0.1 - dp/5.0, 7 + dp*5);
								CreateBullet(e->x, e->y, e, 4, PlayerDir(e->x, e->y) + (4*M_PI / 3) + 0.1 - dp/5.0, 7 + dp*5);
							}
						}
						if (e->enemy_type==10) e->fire_rate = 21;
					break;
					
					default:
					break;
				}
			}
		}
	
		if ((e->t % e->speed) == 0) {
			if (player_room == e->room) {
				e->m_exit = NULL;
				if (e->t % (e->speed * (8 + rand()%6)) == 0) {
					e->move_dir = (float)(rand()%256) / 256.0 * M_PI * 2.0;
				}
				move_x = e->x + cos(e->move_dir)*5;
				move_y = e->y + sin(e->move_dir)*5;
				
				EnemyMovement(e, move_x, move_y);
			} else {
				if (CanLeaveRoom(e->room)) {
					// Try to follow the player into the next room
					
					// Are we already moving towards an exit?
					
					if (e->m_exit != NULL) {
						rcon = e->m_exit;
						
						door_x = (rcon->x + (rcon->x - rcon->x2))*32+16;
						door_y = (rcon->y + (rcon->y - rcon->y2))*32+16;
						
						can_move = 1;
					} else {
						con_traverse = rooms[e->room].con;
						nearest = PlayerDist(e->x, e->y);
						rcon = NULL;
						while (con_traverse != NULL) {
							i = PlayerDist(con_traverse->x2 * 32 + 16, con_traverse->y2 * 32 + 16);
							
							if ((i < nearest) && CanEnterRoom(GetRoom(con_traverse->x2, con_traverse->y2))) {
								nearest = i;
								rcon = con_traverse;
							}
							con_traverse = con_traverse->n;
						}
						
						if (rcon != NULL) {
							door_x = (rcon->x + (rcon->x - rcon->x2))*32+16;
							door_y = (rcon->y + (rcon->y - rcon->y2))*32+16;
							e->m_exit = rcon;
							can_move = 1;
						}
					}
					
					// So, can we move?
					if (can_move) {
						// Are we near the door?
						if (( abs(door_x - e->x) + abs(door_y - e->y))<6) {
							// Go through the door
							e->last_room = e->room;
							e->p_last_room = player_room;
							e->x = (rcon->x2 + (rcon->x2 - rcon->x))*32+16;
							e->y = (rcon->y2 + (rcon->y2 - rcon->y))*32+16;
							rooms[e->room].enemies--;
							e->room = (rcon->c);
							rooms[e->room].enemies++;
							e->curr_follow = -1;
							
							e->m_exit = NULL;
							
						} else {
							// Move towards the door
							e->move_dir = atan2(door_y - e->y, door_x - e->x);
							move_x = e->x + cos(e->move_dir)*5;
							move_y = e->y + sin(e->move_dir)*5;
								
							EnemyMovement(e, move_x, move_y);
						}
					}
				}
			}
		}
	}
	
	if (e->dying > 0) {
		e->dying++;
		if (e->dying == 3) {
			b = bullet_stack;
			while (b != NULL) {
				if (!b->delete_me) {
					if (b->firer == e) {
						b->delete_me = 1;
						CreateGem(b->x, b->y, b->room, (e->max_gems + e->min_gems) / 5 + (artifacts[2] * (e->max_gems + e->min_gems) / 4));
					}
				}
				b = b->next;
			}
		}
		if (e->dying >= 20) {
			if ((e->lives >= 4)&&training) {
				actual_lives = e->lives * 3 / 4;
			} else {
				actual_lives = e->lives;
			}
			
			e->deaths++;
			if (e->deaths >= actual_lives) {
				e->deaths--;
				e->delete_me = 1;
				e->dying = 0;
				killed_enemies++;
				rooms[e->room].enemies--;
				n_gems = e->min_gems + rand()%(e->max_gems - e->min_gems + 1);
				for (i = 0; i < n_gems; i++) {
					CreateGem(e->x - 16 + rand()%32, e->y - 16 + rand()%32, e->room, 1+rand()%4 + (artifacts[2]*rand()%3));
				}
				if (rooms[e->room].room_type == 3) {
					ArtifactRoomUnlock(e->room);
				}
			} else {
				e->dying = 0;
			}
		}
	}
}

void MoveBullet(struct bullet *e)
{
	int pdist;
	struct bullet *n;
	float fx, fy;
	static int last_shield_hit_sound = 0;
	int c_shield_hit_sound;
	
	e->t++;
	
	if ( (boss_fight_mode != 0) && (boss_fight_mode != 2) && (e->dying == 0) ) {
		e->dying = 1;
	}

	if (e->dying == 0) {
		if (e->img == 7) {
			if (proxy_seek) {
				e->speed = 10;
				e->img = 4;
				e->dir = (e->dir + PlayerDir(e->x, e->y)) / 2;
			}
		}
		
		e->x += cos(e->dir) * e->speed;
		e->y += sin(e->dir) * e->speed;
		
		if (e->img == 1) {
			if (e->t % 20 == 19) {
				if (e->t % 40 >= 20)
					CreateBullet(e->x, e->y, e->firer, 3, e->dir + M_PI/2, e->speed * 0.75);
				else
					CreateBullet(e->x, e->y, e->firer, 3, e->dir - M_PI/2, e->speed * 0.75);
			}
		}
		if (e->img == 6) {
			if (e->t % 40 == 39) {
				(FireLaser(e->x, e->y, e->firer, (float)e->t / 30.0, 20, 10, 0.05, 2))->parent = e;
				(FireLaser(e->x, e->y, e->firer, (float)e->t / 30.0 + M_PI*2/3, 20, 10, 0.05, 2))->parent = e;
				(FireLaser(e->x, e->y, e->firer, (float)e->t / 30.0 + M_PI*4/3, 20, 10, 0.05, 2))->parent = e;
			}
		}
		
		
		if (e->img == 5) {
			if (e->t < 100) {
				if (e->t % 20 == 9) {
					n = CreateBullet(e->x, e->y, e->firer, 5, e->dir + M_PI/4, e->speed);
					n->natural_dir = e->dir;
					n->t = e->t;
					e->natural_dir = e->dir;
					
					e->dir -= M_PI / 4;
				}
				if (e->t % 20 == 19) {
					e->dir = e->natural_dir;
				}
			}
			if (e->t == 100) {
				e->dir = PlayerDir(e->x, e->y);
				e->speed *= 1.5;
			}
		}

		if (IsSolid(Get(e->x/32, e->y/32))) {
			if (e->img == 3) {
				if (e->room == player_room) {
					e->dir = PlayerDir(e->x, e->y);
					
					if (IsSolid(Get((e->x + cos(e->dir) * e->speed * 1.5)/32, (e->y + sin(e->dir) * e->speed * 1.5)/32))) e->dying = 1;
					
				} else {
					e->dying = 1;
				}
			} else {
				e->dying = 1;
			}
		}
		
		if (e->img == 2) {
			if (e->parent != NULL) {
				e->x = e->parent->x;
				e->y = e->parent->y;
			} else {
				if (e->firer != NULL) {
					e->x = e->firer->x;
					e->y = e->firer->y;
				}
			}
			if (e->dying == 0) {
				if ((e->t > e->fire_time)&&(e->t <= (e->fire_time + e->duration))) {
					fx = e->x;
					fy = e->y;
					while (!IsSolid(Get((fx)/32, (fy)/32))) {
						if (player_dying == 0) {
							if (PlayerDist(fx, fy) < 30) {
								// hits player shield
								if ((player_shield > 0)&&(shield_hp > 0)) {
									shield_hp -= e->shield_damage;
									if (shield_hp >= 0) {
										e->dying = 1;
										break;
									} else {
										shield_hp = 0;
									}
								}
								
								if (PlayerDist(fx, fy) < 4 - (2 * artifacts[5])) {
									player_dying = 1;
									SND_Pos("dat/a/playerhurt.wav", 128, 0);
									e->dying = 1;
									break;
								}
							}
						}
						fx += cos(e->dir)*2;
						fy += sin(e->dir)*2;
					}
				}
				if (e->turn > 10) {
					if (e->dir > PlayerDir(e->x, e->y)) {
						e->dir -= (e->dir - PlayerDir(e->x, e->y)) / (e->turn - 9.00);
						e->dir -= (e->dir - PlayerDir(e->x, e->y)) / (e->turn - 9.00);
					} else {
						e->dir += (PlayerDir(e->x, e->y) - e->dir) / (e->turn - 9.00);
					}
				} else {
					e->dir += e->turn;
				}
				if (e->t > (e->fire_time + e->duration)) {
					e->dying = 1;
				}
			}
		} else {
	
			pdist = PlayerDist(e->x, e->y);
		
			if (pdist < 30) {
				if (player_dying == 0) {
					if (player_shield > 0) {
						if (shield_hp > 0) {
							shield_hp--;
							if (e->img == 4) {
								e->dying = 1;
							} else {
								e->dir += M_PI;
								c_shield_hit_sound = SDL_GetTicks();
								
								if ((c_shield_hit_sound - 150) > last_shield_hit_sound) {
									SND_Pos("dat/a/shieldhit.wav", 50, 0);
									last_shield_hit_sound = c_shield_hit_sound;
								}
								while (PlayerDist(e->x, e->y) < 30) {
									e->x += cos(e->dir) * e->speed;
									e->y += sin(e->dir) * e->speed;
								}
							}
						}
					}
				}
			}
			if (e->dying == 0) {
				if (pdist < 6 - (2 * artifacts[5])) {
					if (player_dying == 0) {
						SND_Pos("dat/a/playerhurt.wav", 128, 0);
						player_dying = 1;
					}
				}
			}
		}
	}
	
	if ((e->dying == 1) && (e->img == 8)) {
		SpawnLaser(e->x - cos(e->dir) * e->speed, e->y - sin(e->dir) * e->speed, PlayerDir(e->x, e->y), 10, 10, 0.0, player_shield / 6);
	}

	if (GetRoom((e->x)/32, (e->y)/32) != e->room) {
		e->delete_me = 1;
	}
	
	if (e->dying > 0) {
		e->dying++;

		if (e->dying >= 10) {
			e->delete_me = 1;
			e->dying = 0;
		}
	}
}

void DrawEnemy(struct enemy *e, SDL_Surface *scr)
{
	SDL_Rect draw_pos;
	SDL_Rect surf_pos;
	static SDL_Surface *teleflash = NULL;
	
	if (e->delete_me) return;
	draw_pos.x = e->x - e->image->h/2/e->lives - scroll_x;
	draw_pos.y = e->y - e->image->h/2/e->lives - scroll_y;

	surf_pos.x = e->blit_pos;
	surf_pos.y =  e->image->h/e->lives*e->deaths;
	surf_pos.w = e->image->h/e->lives;
	surf_pos.h = e->image->h/e->lives;
	
	if (e->teleport_v < 8) {
	
		if (e->dying == 0) {
			SDL_BlitSurface(e->image, &surf_pos, scr, &draw_pos);
		} else {
			if ((e->deaths+1) >= e->lives) {
				surf_pos.w = e->image->h/e->lives * (20 - e->dying) / 20;
				surf_pos.h = surf_pos.w;
				surf_pos.x += (e->image->h/e->lives - surf_pos.w)/2;
				surf_pos.y += (e->image->h/e->lives - surf_pos.w)/2;
				draw_pos.x += (e->image->h/e->lives - surf_pos.w)/2;
				draw_pos.y += (e->image->h/e->lives - surf_pos.w)/2;		
			} else {
				surf_pos.w = e->image->h/e->lives * (20 - e->dying/2) / 20;
				surf_pos.h = surf_pos.w;
				surf_pos.x += (e->image->h/e->lives - surf_pos.w)/2;
				surf_pos.y += (e->image->h/e->lives - surf_pos.w)/2;
				draw_pos.x += (e->image->h/e->lives - surf_pos.w)/2;
				draw_pos.y += (e->image->h/e->lives - surf_pos.w)/2;		
			}
			SDL_BlitSurface(e->image, &surf_pos, scr, &draw_pos);
		}
	
		if (((e->t % 8) == 1) && (!game_paused)) {
			e->blit_pos = (e->blit_pos + e->image->h/e->lives) % e->image->w;
		}
	
		draw_pos.x = e->x - e->image->h/e->lives/2 - scroll_x;
		draw_pos.y = e->y - e->image->h/e->lives/2 - scroll_y;
	
		draw_pos.x -= (128-e->image->h/e->lives)/2;
		draw_pos.y -= (128-e->image->h/e->lives)/2;
	
		if (magic_circuit >= e->str) {
			SDL_BlitSurface(reticle, NULL, scr, &draw_pos);
		}
		
		draw_pos.x = e->x - e->image->h/e->lives/2 - scroll_x;
		draw_pos.y = e->y - e->image->h/e->lives/2 - scroll_y;
	
		draw_pos.x -= (128-e->image->h/e->lives)/2;
		draw_pos.y -= (128-e->image->h/e->lives)/2;
	
		if (sqrt(sqr(e->x - player_x) + sqr(e->y - player_y)) < circuit_range) {
			SDL_BlitSurface(inrange, NULL, scr, &draw_pos);
		}
	}
	
	if (e->teleport_v < 24) {
		if (teleflash == NULL) {
			teleflash = IMG_Load("dat/i/teleflash.png");
			SDL_SetColorKey(teleflash, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
		}

		surf_pos.x = 48*((e->teleport_v) / 3);
		surf_pos.y = 0;
		surf_pos.w = 48;
		surf_pos.h = 48;
		
		draw_pos.x = e->x - 24 - scroll_x;
		draw_pos.y = e->y - 24 - scroll_y;
		SDL_BlitSurface(teleflash, &surf_pos, scr, &draw_pos);
	}
}

void DrawBullet(struct bullet *b)
{
	int i;
	int x1, y1, x2, y2, xo1, yo1;
	int z;
	float fx, fy;
	static SDL_Surface *d_star_small = NULL, *d_star_big = NULL, *d_star_ls = NULL;
	SDL_Rect draw_pos, surf_pos;
	if (b->delete_me) return;
	if (b->img == 0) {
		if (b->dying > 0) {
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 6+(b->dying / 2), b->dying*10);
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 3+(b->dying / 2), 255-b->dying*10);
		} else {
			if ((b->t % 8) < 4) {
				DrawCircle(b->x - scroll_x, b->y - scroll_y, 6, 0);
				DrawCircle(b->x - scroll_x, b->y - scroll_y, 4, 255);
			} else {
				DrawCircle(b->x - scroll_x, b->y - scroll_y, 6, 255);
				DrawCircle(b->x - scroll_x, b->y - scroll_y, 4, 0);
			}
		}
	}
	if (b->img == 1) {
		if (d_star_big == NULL) {
			d_star_big = IMG_Load("dat/i/star1.png");
			SDL_SetColorKey(d_star_big, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
		}
		
		if (b->dying == 0) {
			surf_pos.x = b->x - 16 - scroll_x;
			surf_pos.y = b->y - 16 - scroll_y;
			draw_pos.x = (b->t % 8)*32;
			draw_pos.y = 0;
			draw_pos.w = 32;
			draw_pos.h = 32;
			SDL_BlitSurface(d_star_big, &draw_pos, screen, &surf_pos);
		} else {
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 4+(b->dying), b->dying*10);
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 2+(b->dying), 255-b->dying*10);
		}
	}
	if (b->img == 2) {
		// IMMA CHARGIN MAH LAZER
		if (b->t <= b->fire_time) {
			z = (b->t * 150 / (b->fire_time+1)) + 80;
			
			fx = b->x;
			fy = b->y;
			while (!IsSolid(Get((fx)/32, (fy)/32))) {
				DrawRect(fx-1-scroll_x, fy-1-scroll_y, 4, 4, z/2);
				fx += cos(b->dir)*2;
				fy += sin(b->dir)*2;
				
				if ((player_shield > 0)&&(shield_hp > 0)&&(PlayerDist(fx, fy) < 30)) break;
				if (PlayerDist(fx, fy) < 4) break;
			}
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 5, (z+255)/2.4);
			DrawCircle(fx-cos(b->dir)*2 - scroll_x, fy-sin(b->dir)*2 - scroll_y, 4, z/2);
			
			fx = b->x;
			fy = b->y;
			while (!IsSolid(Get((fx)/32, (fy)/32))) {
				DrawRect(fx-scroll_x, fy-scroll_y, 2, 2, z);
				fx += cos(b->dir)*2;
				fy += sin(b->dir)*2;
				
				if ((player_shield > 0)&&(shield_hp > 0)&&(PlayerDist(fx, fy) < 30)) break;
				if (PlayerDist(fx, fy) < 4) break;
			}
			
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 3, (z+255)/2);
			DrawCircle(fx-cos(b->dir)*2 - scroll_x, fy-sin(b->dir)*2 - scroll_y, 2, z);
		}
		
		// SHOOP DA WHOOP
		if ((b->t > b->fire_time) && (b->t <= b->fire_time + b->duration)) {
			z = 255 - ((b->t - b->fire_time) * 200 / (b->duration+1));
			
			fx = b->x;
			fy = b->y;
			while (!IsSolid(Get((fx)/32, (fy)/32))) {
				DrawRect(fx-3-scroll_x, fy-3-scroll_y, 8, 8, z*2/3);
				fx += cos(b->dir)*2;
				fy += sin(b->dir)*2;
				
				if ((player_shield > 0)&&(shield_hp > 0)&&(PlayerDist(fx, fy) < 30)) break;
				if (PlayerDist(fx, fy) < 4) break;
			}
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 9, (z+255)/2.4);
			DrawCircle(fx-cos(b->dir)*2 - scroll_x, fy-sin(b->dir)*2 - scroll_y, 8, z*2/3);
			fx = b->x;
			fy = b->y;
			while (!IsSolid(Get((fx)/32, (fy)/32))) {
				DrawRect(fx-2-scroll_x, fy-2-scroll_y, 6, 6, z);
				fx += cos(b->dir)*2;
				fy += sin(b->dir)*2;
				
				x1 = fx + rand()%12 - rand()%12 - scroll_x;
				y1 = fy + rand()%12 - rand()%12 - scroll_y;
				DrawRect(x1, y1, 2, 2, rand()%(z+1));
				
				if ((player_shield > 0)&&(shield_hp > 0)&&(PlayerDist(fx, fy) < 30)) break;
				if (PlayerDist(fx, fy) < 4) break;
			}
		
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 7, (z+255)/2);
			DrawCircle(fx-cos(b->dir)*2 - scroll_x, fy-sin(b->dir)*2 - scroll_y, 6, z);
		}
	}
	if (b->img == 3) {
		if (d_star_small == NULL) {
			d_star_small = IMG_Load("dat/i/star2.png");
			SDL_SetColorKey(d_star_small, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
		}
		if (b->dying == 0) {
			surf_pos.x = b->x - 8 - scroll_x;
			surf_pos.y = b->y - 8 - scroll_y;
			draw_pos.x = (b->t % 8)*16;
			draw_pos.y = 0;
			draw_pos.w = 16;
			draw_pos.h = 16;
			SDL_BlitSurface(d_star_small, &draw_pos, screen, &surf_pos);
		} else {
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 4+(b->dying/2), b->dying*10);
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 2+(b->dying/2), 255-b->dying*10);
		}
	}
	if (b->img == 4) {
		if (b->dying == 0) {
			x1 = b->x + cos(b->dir)*(2) - scroll_x;
			y1 = b->y + sin(b->dir)*(2) - scroll_y;
				
			x2 = b->x - cos(b->dir)*(80) - scroll_x;
			y2 = b->y - sin(b->dir)*(80) - scroll_y;
			for (i = 0; i < 40; i++) {
				xo1 = (x2 - x1)*i/40 + x1;
				yo1 = (y2 - y1)*i/40 + y1;
				
				z = (i/2);
				if (i > 10) z = (40 - i)/7;
				
				DrawRect(xo1 - z - 2, yo1 - z - 2, z*2 + 4, z*2 + 4, 200);
			}
			for (i = 0; i < 40; i++) {
				xo1 = (x2 - x1)*i/40 + x1;
				yo1 = (y2 - y1)*i/40 + y1;
				
				z = (i/2);
				if (i > 10) z = (40 - i)/7;
				
				DrawRect(xo1 - z, yo1 - z, z*2, z*2, 255);
			}
		} else{
			DrawCircleEx(b->x - scroll_x, b->y - scroll_y, b->dying * 2 + 1, b->dying * 1.75 - 1, 230);
			DrawCircleEx(b->x - scroll_x, b->y - scroll_y, b->dying * 2, b->dying * 1.75, 255);
		}
	}
	if (b->img == 5) {
		if (b->dying > 0) {
			i = 4+(b->dying / 5);
			DrawRect(b->x - scroll_x - (i-1/2), b->y - scroll_y - (i-1/2), i, i, b->dying*10);
			i = 2+(b->dying / 5);
			DrawRect(b->x - scroll_x - (i-1/2), b->y - scroll_y - (i-1/2), i, i, 255-b->dying*10);
		} else {
			if ((b->t % 8) < 4) {
				DrawRect(b->x - scroll_x - 1, b->y - scroll_y - 1, 4, 4, 0);
				DrawRect(b->x - scroll_x, b->y - scroll_y, 2, 2, 255);
			} else {
				DrawRect(b->x - scroll_x - 1, b->y - scroll_y - 1, 4, 4, 255);
				DrawRect(b->x - scroll_x, b->y - scroll_y, 2, 2, 0);
			}
		}
	}
	if (b->img == 6) {
		if (d_star_ls == NULL) {
			d_star_ls = IMG_Load("dat/i/star3.png");
			SDL_SetColorKey(d_star_ls, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
		}
		if (b->dying == 0) {
			surf_pos.x = b->x - 16 - scroll_x;
			surf_pos.y = b->y - 16 - scroll_y;
			draw_pos.x = (b->t % 8)*32;
			draw_pos.y = 0;
			draw_pos.w = 32;
			draw_pos.h = 32;
			SDL_BlitSurface(d_star_ls, &draw_pos, screen, &surf_pos);
		} else {
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 4+(b->dying), 255-b->dying*10);
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 2+(b->dying), b->dying*10);
		}
	}
	
	if (b->img == 7) {
		if ((b->t % 8) < 4) {
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 8, 0);
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 5, 255);
		} else {
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 8, 255);
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 5, 0);
		}
	}
	if (b->img == 8) {
		if (b->dying > 0) {
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 12+(b->dying), b->dying*10);
			DrawCircle(b->x - scroll_x, b->y - scroll_y, 6+(b->dying), 255-b->dying*10);
		} else {
			if ((b->t % 6) < 3) {
				DrawCircle(b->x - scroll_x, b->y - scroll_y, 12, 0);
				DrawCircle(b->x - scroll_x, b->y - scroll_y, 8, 255);
			} else {
				DrawCircle(b->x - scroll_x, b->y - scroll_y, 12, 255);
				DrawCircle(b->x - scroll_x, b->y - scroll_y, 8, 0);
			}
		}
	}
}

void DrawGem(struct diamond *g)
{
	static SDL_Surface *d_sprite = NULL;
	int gemtype = 0;
	static char hp_icon[2];
	SDL_Rect draw_pos;
	SDL_Rect surf_pos;
	unsigned char fxp = (SDL_GetTicks() / 300)%2 ? 255 : 0;
	
	if (g->delete_me) return;

	if (d_sprite == NULL) {
		d_sprite = IMG_Load("dat/i/gem.png");
		SDL_SetColorKey(d_sprite, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
		hp_icon[0] = 3;
		hp_icon[1] = 0;
	}

	if (g->value == 1) gemtype = 2;
	if ((g->value > 1)&&(g->value < 5)) gemtype = 1;
	if (g->value >= 5) gemtype = 0;
	
	if (g->value != 31337) {
	
		surf_pos.x = g->x - 4 - scroll_x;
		surf_pos.y = g->y - 4 - scroll_y;
		draw_pos.x = (g->t % 4) * 8 + 32 * gemtype;
		draw_pos.y = 0;
		draw_pos.w = 8;
		draw_pos.h = 8;
	
		SDL_BlitSurface(d_sprite, &draw_pos, screen, &surf_pos);
	} else {
		DrawCircle(g->x - scroll_x, g->y - scroll_y, 6, (rand()%64) ^ fxp);
		draw_text(g->x - 4 - scroll_x, g->y - 4 - scroll_y, 0, hp_icon, (200+rand()%56) ^ fxp);
	}

	g->t++;
}

void xprintf(char *s)
{
	printf(s);
	fflush(stdout);
}

void DrawInvisible(int x, int y)
{
	SDL_Rect dest;
	dest.x = x - scroll_x - 24;
	dest.y = y - scroll_y - 24;
	SDL_BlitSurface(invis_enemy, NULL, screen, &dest);
}

void DrawEntities()
{
	struct enemy *t;
	struct bullet *b;
	struct diamond *g;
	struct enemyloc *els;
	
	if ((rooms[player_room].room_type != 3)&&(rooms[player_room].room_type != 2)) {
		// Draw gems
	
		g = room_gems[player_room];
		while (g != NULL) {
			if ((g->room == player_room)&&(g->delete_me == 0)&&(g->value != 31337)) {
				DrawGem(g);
			}
			g = g->next_in_room;
		}
		g = room_gems[player_room];
		while (g != NULL) {
			if ((g->room == player_room)&&(g->delete_me == 0)&&(g->value == 31337)) {
				DrawGem(g);
			}
			g = g->next_in_room;
		}
	}
	
	t = active_stack;
	while (t != NULL) {
		if (!t->delete_me) {
			if ((t->room == player_room) && (t->enemy_type != 9)) {
				DrawEnemy(t, screen);
			}
		}
		t = t->next_active;
	}
	
	// Draw invisible enemies (if possible)
	if (artifacts[6]) {
		// Draw the actives
		t = active_stack;
		while (t != NULL) {
			if (!t->delete_me) 
			{
				if (((!t->delete_me)&&((t->room != player_room)||(t->enemy_type == 9)))) {
					DrawInvisible(t->x, t->y);
				}
			}
			t = t->next_active;
		}
		// Draw the inactives
		if (!artifacts[11]) {
			els = GetEnemyLoc(scroll_x, scroll_y);
			while (els != NULL) {
				t = els->e;
				if (((!t->delete_me)&&((t->room != player_room)||(t->enemy_type == 9)))&&(t->active == 0)) {
					if ((t->x+24 >= scroll_x) && (t->y+24 >= scroll_y)) {
						if (t->x-24 <= (scroll_x+639)) {
							if (t->y-24 <= (scroll_y+479)) {
								if ((rooms[t->room].room_type != 2) && (rooms[t->room].room_type != 3)) {
									DrawInvisible(t->x, t->y);
								}
							}
						}
					}
				}
				els = els->n;
			}
		}
	}
	
	b = bullet_stack;
	while (b != NULL) {
		if (!b->delete_me) {
			if (b->room == player_room) {
				DrawBullet(b);
			}
		}
		b = b->next;
	}
}

void MoveEntities()
{
	struct enemy *t;
	struct bullet *b;
	struct diamond *g;

	struct bullet *b_del;
	struct diamond *g_del;
	
	if ((rooms[player_room].room_type != 3)&&(rooms[player_room].room_type != 2)) {
		// gem stuff
		g = room_gems[player_room];
		while (g != NULL) {
			if ((g->room == player_room)&&(g->delete_me == 0)) {
				if (artifacts[7]) {
					g->x += (player_x+4 - g->x)/10;
					g->y += (player_y+12 - g->y)/10;
				}
				if (PlayerDist(g->x, g->y) < 20) {
					g->delete_me = 1;
					total_gems--;
					
					if (g->value == 31337) {
						if (player_hp < (3 + (player_shield == 30)*3)) {
							SND_Pos("dat/a/crystal.wav", 64, 0);
							player_hp++;
						} else {
							if (!training) {
								SND_Pos("dat/a/tone.wav", 64, 0);
								
								if (player_lives == 1) {
									player_lives_part += 15;
								} else {
									if (player_lives < 10) {
										player_lives_part += 4;
									} else {
										player_lives_part += 1;
									}
								}
								if (player_lives_part >= 88) {
									player_lives_part -= 88;
									player_lives += 1;
									SND_Pos("dat/a/crystal2.wav", 100, 0);
								}
							}
						}
						
					} else {
						player_gems += g->value;
					}
				}
			}
			g = g->next_in_room;
		}
	}
	
	t = active_stack;
	while (t != NULL) {
		if (!t->delete_me) {
			if ((rooms[t->room].room_type != 3) || (t->room == player_room))
				MoveEnemy(t);
		}
		t = t->next_active;
	}

	b = bullet_stack;
	while (b != NULL) {
		if (!b->delete_me) {
			MoveBullet(b);
		}
		b = b->next;
	}

	// delete old bullets
	b = bullet_stack;
	if ((b != NULL)&&(b->delete_me)) {
		b_del = b;
		b = b->next;
		free(b_del);
		bullet_stack = b;
	}
	while (b != NULL) {
		if (b->next != NULL) {
			if (b->next->delete_me) {
				b_del = b->next;

				b->next = b->next->next;
				free(b_del);
			}
		}
		b = b->next;
	}

	// delete old gems
	g = gem_stack;
	if ((g != NULL)&&(g->delete_me)) {
		// Remove room reference
		
		if (g == room_gems[g->room]) {
			room_gems[g->room] = g->next_in_room;
			if (room_gems[g->room] != NULL) {
				room_gems[g->room]->prv_in_room = NULL;
			}
		} else {
			if (g->prv_in_room != NULL) {
				g->prv_in_room->next_in_room = g->next_in_room;
			}
			
			if (g->next_in_room != NULL) {
				g->next_in_room->prv_in_room = g->prv_in_room;
			}
		}
	
		g_del = g;
		g = g->next;
		free(g_del);
		gem_stack = g;
	}
	while (g != NULL) {
		if (g->next != NULL) {
			if (g->next->delete_me) {
				// Remove room reference
				assert( (g->next->prv_in_room != NULL) || (room_gems[g->next->room] == g->next) );
				
				if (g->next == room_gems[g->next->room]) {
					room_gems[g->next->room] = g->next->next_in_room;
					if (room_gems[g->next->room] != NULL) {
						room_gems[g->next->room]->prv_in_room = NULL;
					}
				} else {
					g->next->prv_in_room->next_in_room = g->next->next_in_room;
					if (g->next->next_in_room != NULL) {
						g->next->next_in_room->prv_in_room = g->next->prv_in_room;
					}
				}
				g_del = g->next;

				g->next = g->next->next;
				free(g_del);
			}
		}
		g = g->next;
	}

	// delete old monsters
	t = active_stack;
	if (t != NULL) {
		while (t->delete_me) {
			t = t->next_active;
			active_stack = t;

			if (t == NULL) break;
		}
	}
	while (t != NULL) {
		if (t == active_stack)
			assert(t->delete_me == 0);
		else
			assert(!t->delete_me);

		if (t->next_active != NULL) {
			while (t->next_active->delete_me) {
				t->next_active = t->next_active->next_active;
				if (t->next_active == NULL) break;
			}
		}
		t = t->next_active;
	}
}

void HurtEnemies(int x, int y, int range, int power)
{
	struct enemy *t;
	int e_range;

	t = active_stack;
	while (t != NULL) {
		e_range = sqrt(sqr(t->x - x) + sqr(t->y - y));
		if (e_range < range) {
			if (power >= t->str) {
				if (t->room == GetRoom(x/32, y/32)) {
					KillEnemy(t);
				}
			}
		}
		t = t->next_active;
	}
}

void CircuitBullets(int x, int y, int r)
{
	struct bullet *b;
	b = bullet_stack;
	while (b != NULL) {
		if (!b->delete_me) {
			if (b->dying == 0) {
				if (b->invuln == 0) {
					if (sqrt(sqr(b->x - x) + sqr(b->y - y)) < r) {
						b->dying = 1;
					}
				}
			}
		}
		b = b->next;
	}
}

void CrystalSummon()
{
	struct diamond *g;
	int rg_x, rg_y;
	int i;

	g = gem_stack;
	
	for (i = 0; i < 3000; i++) {
		room_gems[i] = NULL;
	}
	
	while (g != NULL) {
		if (!g->delete_me) {
			if (rooms[g->room].room_type != 3) {
				g->room = player_room;
				rg_x = rooms[player_room].x * 32 + 32 + rand()%(rooms[player_room].w*32-64);
				rg_y = rooms[player_room].y * 32 + 32 + rand()%(rooms[player_room].h*32-64);
				if (player_room == 0) {
					rg_x = (rooms[player_room].x+5) * 32 + 32 + rand()%(8*32);
					rg_y = (rooms[player_room].y+5) * 32 + 32 + rand()%(5*32);
				}
				while (IsSolid(Get(rg_x/32, rg_y/32))) {
					rg_x = rooms[player_room].x * 32 + 32 + rand()%(rooms[player_room].w*32-64);
					rg_y = rooms[player_room].y * 32 + 32 + rand()%(rooms[player_room].h*32-64);
					if (player_room == 0) {
						rg_x = (rooms[player_room].x+5) * 32 + 32 + rand()%(8*32);
						rg_y = (rooms[player_room].y+5) * 32 + 32 + rand()%(5*32);
					}
				}
				g->x = rg_x;
				g->y = rg_y;
			}
		}
		g->next_in_room = room_gems[g->room];
		g->prv_in_room = NULL;
		if (room_gems[g->room] != NULL) {
			room_gems[g->room]->prv_in_room = g;
		}
		room_gems[g->room] = g;
		
		g = g->next;
	}
}

void ActivateRand()
{
	struct enemy *e;
	struct enemy *en[20000];
	int ent = 0;
	
	e = enemy_stack;
	
	while (e != NULL) {
		if (e->delete_me == 0) {
			if (e->active == 0) {
				en[ent++] = e;
			}
		}
		e = e->next;
	}
	if (ent > 0) {
		e = en[rand()%ent];
		
		XActivateSingleEnemy(e);
	}
}

void ClearBossBullets()
{
	struct bullet *b = bullet_stack;
	while (b != NULL) {
		if (!b->delete_me) {
			if (b->firer == NULL) {
				b->delete_me = 1;
			}
		}
		b = b->next;
	}
}

void SpawnBullet(int x, int y, int bullet_type, float dir, float spd, int invuln)
{
	struct bullet *b;
	
	if ( (current_boss == 3) && (player_shield == 30) && (boss_fight_mode == 2) ) {
		spd *= 1.2;
	}
	b = CreateBullet(x, y, NULL, bullet_type, dir, spd);
	if (invuln) {
		b->invuln = 1;
	}
}
void SpawnLaser(int x, int y, float dir, int fire_time, int duration, float turn, int dmg)
{
	FireLaser(x, y, NULL, dir, fire_time, duration, turn, dmg);
}

void CullEnemies(int nth)
{
	struct enemy *e;
	int i = 0;
	
	e = enemy_stack;
	
	while (e != NULL) {
		if (e->delete_me == 0) {
			if (rooms[e->room].room_type == 0) {
				if ( (i % nth) == (nth - 1)) {
					e->delete_me = 1;
					killed_enemies++;
					e->dying = 0;
					rooms[e->room].enemies--;
				}
				i++;
			}
		}
		e = e->next;
	}
}

void SoupUpEnemies()
{
	struct enemy *e;
	int str_limit;
	float str_multiplier;
	float fr_divider;
	
	e = enemy_stack;
	str_limit = 1500;
	if (circuit_size > 1500) {
		str_limit = 1500;
	}
	
	str_multiplier = 1.0 + (1.0/3.0)*(float)current_boss;
	fr_divider = 1.0 + (2.0/3.0)*(float)current_boss;
	
	while (e != NULL) {
		if (e->delete_me == 0) {
			if (e->enemy_type != 10) {
				if (e->str < str_limit) {
					if ((e->str * 2) < str_limit) {
						e->str = e->str * str_multiplier;
					} else {
						e->str = str_limit;
					}
				}
				e->fire_rate = (int)((float)e->fire_rate / fr_divider) + 1;
				e->speed = (int)((float)e->speed / fr_divider) + 1;
				e->min_gems *= str_multiplier;
				e->max_gems *= str_multiplier;
			}
		}
		e = e->next;
	}
}

void CurseSingleEnemy(struct enemy *e)
{
	static int ActiveRooms[3000];
	static int NActiveRooms = 0;
	int i;
	int rm;
	
	if (NActiveRooms == 0) {
		for (i = 0; i < 3000; i++) {
			if ((rooms[i].room_type == 0) || (rooms[i].room_type == 4)) {
				ActiveRooms[NActiveRooms++] = i;
			}
		}
	}

	rm = ActiveRooms[(NActiveRooms ? rand()%NActiveRooms : 0)];
	while ((rooms[rm].enemies > 3) || (rooms[rm].visited == 0)) {
		rm = ActiveRooms[rand()%NActiveRooms];
	}

	e->x = rooms[rm].w * 16 + rooms[rm].x * 32;
	e->y = rooms[rm].h * 16 + rooms[rm].y * 32;
	rooms[e->room].enemies--;
	e->room = rm;
	rooms[e->room].enemies++;
	
	e->image = enemy_sprites[9];
	e->lives = 8;
	e->str = 500;
	e->speed = 1;
	e->fire_rate = (rand()%4)+1;
	e->min_gems = 5000;
	e->max_gems = 6000;
	e->followdepth = 12;
	e->creationcost = 6;
	e->enemy_type = 10;
	
	ActivateSingleEnemy(e);
}

void CurseEnemies()
{
	struct enemy *e;
	int i = 0;
	
	e = enemy_stack;
	
	while (e != NULL) {
		if (e->delete_me == 0) {
			if ( (i % 5) == (4)) {
				CurseSingleEnemy(e);
			} else {
				e->delete_me = 1;
				killed_enemies++;
				
				e->dying = 0;
				rooms[e->room].enemies--;
			}
			i++;
		}
		e = e->next;
	}
}
