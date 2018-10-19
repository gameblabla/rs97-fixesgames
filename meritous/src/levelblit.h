//
//   levelblit.h
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

// Exposes levelblit.c functionality and types

#ifndef LEVELBLIT_H
#define LEVELBLIT_H

#define PLAYERW 16
#define PLAYERH 24

#define SCREEN_W 320
#define SCREEN_H 240

#define JOY_DEADZONE 1000

extern SDL_Joystick *joy;

extern SDL_Surface *screen;

extern int player_x, player_y;
extern int scroll_x, scroll_y;
extern int player_room;
extern int prv_player_room;

extern int magic_circuit;
extern int circuit_size;
extern int circuit_range;
void DrawBorder(int len, int lines);
void DrawBorderAtCustomHeight(int len, int lines, int height);
void DrawCircle(int x, int y, int r, unsigned char c);
void DrawCircleEx(int x, int y, int r, int r2, unsigned char c);
void DrawRect(int x, int y, int w, int h, unsigned char c);
int IsSolid(unsigned char tile);
void draw_char(int cur_x, int cur_y, int c, Uint8 tcol);
void draw_text(int x, int y, int right_margin, char *str, Uint8 tcol);
void draw_text_ex(int x, int y, char *str, Uint8 tcol, SDL_Surface *srf);

extern int player_shield;
extern int shield_hp;
extern int shield_recover;
extern int player_hp;
extern int player_lives;
extern int player_lives_part;
extern int enter_room_x, enter_room_y;

extern int player_dying;

extern int checkpoint_x;
extern int checkpoint_y;

extern int player_gems;

extern int specialmessage;
extern int specialmessagetimer;

extern int tele_select;

void WritePlayerData();
void ReadPlayerData();

extern int artifacts[];

void LoadingScreen(int part, float progress);
void SavingScreen(int part, float progress);

void ThinLine(SDL_Surface *scr, int x1, int y1, int x2, int y2, Uint8 col);
float RandomDir();

void Arc(SDL_Surface *s, int x, int y, int r, float dir);

extern SDL_Surface *artifact_spr;
extern SDL_Surface *artifact_spr_large;

void VideoUpdate();
void EndCycle(int n);

extern int enter_pressed;

extern int game_paused;

extern int key_held[];

extern int training;
extern int show_ending;

void DrawLevel(int off_x, int off_y, int hide_not_visited, int fog_of_war);
void DrawPlayer(int x, int y, int pl_dir, int pl_frm);
int GetNearestCheckpoint(int x, int y);
int dist(int x1, int y1, int x2, int y2);
#define K_UP 0
#define K_DN 1
#define K_LT 2
#define K_RT 3
#define K_SP 4

#endif
