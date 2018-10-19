//
//   demon.h
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

// Exposes demon.c functionality and types

#ifndef DEMON_H
#define DEMON_H

void InitEnemies();
void ActivateEnemies(int room);
void DrawEntities();
void MoveEntities();
void HurtEnemies(int x, int y, int range, int power);
void CircuitBullets(int x, int y, int r);
void CreateGem(int x, int y, int r, int v);

extern int total_enemies, killed_enemies, total_gems;

void WriteCreatureData();
void WriteEnemyData();
void WriteGemData();
void ReadCreatureData();
void ReadEnemyData();
void ReadGemData();
void CrystalSummon();

void ActivateRand();

void DestroyThings();

void EnemySound(int t, int dist);

void SpawnBullet(int x, int y, int bullet_type, float dir, float spd, int invuln);
void SpawnLaser(int x, int y, float dir, int fire_time, int duration, float turn, int dmg);

void ClearBossBullets();

float PlayerDir(int x, int y);
int PlayerDist(int x, int y);

void CullEnemies(int nth);
void SoupUpEnemies();

void CurseEnemies();
#endif
