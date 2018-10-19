//
//   ending.c
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
#include "audio.h"
#include "boss.h"
#include "mapgen.h"

void DrawScrolly(int t);
void DrawPText(int t);
void DrawSText(int t);
void DrawSTextPartTwo(int t);
void DrawSTextV(int t);
void DrawCircuitFlash(int t, int method);
void DrawStream(int t);

void InitParticleStorm();
void RunParticleStorm(int offset);

SDL_Surface *streamspr = NULL, *glitter = NULL;

SDL_Color ending_pal[256];

void UpdatePalette()
{
	SDL_SetPalette(screen, SDL_PHYSPAL, ending_pal, 0, 256);
}

void DrawCredits();

int credits_scroll = 0;

int EndingEvents()
{
	static SDL_Event event;
	
	player_room = 0;
	current_boss = 3;
	boss_fight_mode = 4;
	
	MusicUpdate();
	
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					return 1;
					break;
				default:
					break;
			}
		}
		if (event.type == SDL_QUIT) {
			return 1;
		}
	}
	
	return 0;
}

void ShowEnding()
{
	int i;

	if (streamspr == NULL) {
		streamspr = IMG_Load("dat/i/stream.png");
		SDL_SetColorKey(streamspr, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
		glitter = IMG_Load("dat/i/glitter.png");
		SDL_SetColorKey(glitter, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
	}
	
	for (i = 0; i < 500; i += 1) {
		if (((i % 60) >= 24)&&((i % 60) < 34)) {
			DrawCircuitFlash((i % 60) - 24, 0);
		} else {
			DrawScrolly(i);
		}
		EndCycle(0);
		if (EndingEvents()) return;
	}
	for (i = 0; i < 30; i++) {
		DrawCircuitFlash(i, 1);
		
		EndCycle(0);
		if (EndingEvents()) return;
	}
	SDL_FillRect(screen, NULL, 255);
	for (i = 0; i < 350; i++) {
		DrawPText(i);
		EndCycle(0);
		if (EndingEvents()) return;
	}
	
	Paint(0, 0, 22, 27, "dat/d/cstream.loc");
	
	if (player_shield < 30) {
		for (i = 0; i < 400; i++) {
			DrawStream(i);
			EndCycle(0);
			if (EndingEvents()) return;
		}
		InitParticleStorm();
		for (i = 0; i < 240; i++) {
			RunParticleStorm(240-i);
			EndCycle(0);
			if (EndingEvents()) return;
		}
		for (i = 0; i < 60; i++) {
			RunParticleStorm(0);
			EndCycle(0);
			if (EndingEvents()) return;
		}
		for (i = 0; i < 180; i++) {
			RunParticleStorm(i*3);
			EndCycle(0);
			if (EndingEvents()) return;
		}
		for (i = 0; i < 500; i++) {
			DrawSText(i);
			EndCycle(0);
			if (EndingEvents()) return;
		}
		for (i = 0; i < 500; i++) {
			DrawSTextPartTwo(i);
			EndCycle(0);
			if (EndingEvents()) return;
		}
	} else {
		for (i = 0; i < 250; i++) {
			DrawStream(i);
			EndCycle(0);
			if (EndingEvents()) return;
		}
		for (i = 0; i < 500; i++) {
			DrawSTextV(i);
			EndCycle(0);
			if (EndingEvents()) return;
		}
	}
	
	credits_scroll = 0;
	for (;;) {
		DrawCredits();
		EndCycle(0);
		if (EndingEvents()) return;
	}
}

char *SText[15] = {	"Merit released the locks on the PSI flowing through the Dome,",
			"releasing the flow of PSI into the atmosphere.",
			"",
			"The Orcus Dome was originally built to centralise the limited",
			"PSI available to everyone. However, this made the existing",
			"reserves more vulnerable to malicious PSI users",
			"",
			"While other PSI users initially resented Merit for his rash",
			"behaviour, they eventually adjusted to the decentralisation.",
			"",
			"Eventually, PSI users grew so adept at manipulating the",
			"diluted flows of PSI that they were capable of the same things",
			"as before. Each PSI user would keep their own individual",
			"reserves of PSI for when they needed to weild greater power,",
			"and the balance of power was restored." };
			
char *STextV[15] = {	"Merit decided to assume the role of custodian over the Orcus",
			"Dome, in Wervyn Anixil's place. He resumed the experiments on",
			"PSI and found ways of making the Dome's remaining supply go as",
			"far as it could.",
			"",
			"Other PSI users were suspicious of MERIT, just as they were",
			"wary of Wervyn Anixil before him, but they soon adjusted.",
			"",
			"The balance of power was quickly restored, and stabilised for",
			"eternity due to the work of Wervyn Anixil and now MERIT.",
			"[[ BEST ENDING ]]",
			"",
			"",
			"",
			""};

void DrawSText(int t)
{
	int offset = 540 + (t / 2);
	int i, c;
	t = t * 350 / 500;
	int cl = 350 - t;
	
	for (i = 0; i < 64; i++) {
		DrawRect(0, i * 15 - offset, SCREEN_W, 15, (64 - i) * cl / 350);
	}
	for (i = 64; i < 128; i++) {
		DrawRect(0, i * 15 - offset, SCREEN_W, 15, (i - 64) * cl / 350);
	}
	
	if (t < 300) {
		for (i = 0; i < 10; i++) {
			c = (255 + (i * 100) - t*10);
			if (c < 0) c = 0;
			if (c > 255) c = 255;
			
			draw_text(10, 20+i*20, 0, SText[i], 255-c);
		}
	} else {
		for (i = 0; i < 10; i++) {
			c = 5 + (t-300) * 5;
			
			draw_text(10, 20+i*20, 0, SText[i], 255-c);
		}
	}
	
	UpdatePalette();
	VideoUpdate();
}

void DrawSTextPartTwo(int t)
{
	int offset = 540 + (t / 2);
	int i, c;
	t = t * 350 / 500;
	
	for (i = 0; i < 64; i++) {
		DrawRect(0, i * 15 - offset, SCREEN_W, 15, 0);
	}
	for (i = 64; i < 128; i++) {
		DrawRect(0, i * 15 - offset, SCREEN_W, 15, 0);
	}
	
	if (t < 300) {
		for (i = 10; i < 15; i++) {
			c = (255 + (i * 100) - t*10);
			if (c < 0) c = 0;
			if (c > 255) c = 255;
			
			draw_text(10, 20+(i-10)*20, 0, SText[i], 255-c);
		}
	} else {
		for (i = 10; i < 15; i++) {
			c = 5 + (t-300) * 5;
			
			draw_text(10, 20+(i-10)*20, 0, SText[i], 255-c);
		}
	}
	
	UpdatePalette();
	VideoUpdate();
}

void DrawSTextV(int t)
{
	int offset = 540 + (t / 2);
	int i, c;
	t = t * 350 / 500;
	int cl = 350 - t;
	
	for (i = 0; i < 64; i++) {
		DrawRect(0, i * 15 - offset, SCREEN_W, 15, (64 - i) * cl / 350);
	}
	for (i = 64; i < 128; i++) {
		DrawRect(0, i * 15 - offset, SCREEN_W, 15, (i - 64) * cl / 350);
	}
	
	if (t < 300) {
		for (i = 0; i < 15; i++) {
			c = (255 + (i * 100) - t*10);
			if (c < 0) c = 0;
			if (c > 255) c = 255;
			
			draw_text(10, 20+i*20, 0, STextV[i], 255-c);
		}
	} else {
		for (i = 0; i < 15; i++) {
			c = 5 + (t-300) * 5;
			
			draw_text(10, 20+i*20, 0, STextV[i], 255-c);
		}
	}
	
	UpdatePalette();
	VideoUpdate();
}


float pt_x[500];
float pt_y[500];
float pt_vx[500];
float pt_vy[500];
int pt_t[500];

void InitParticleStorm()
{
	int i;
	
	for (i = 0; i < 500; i++) {
		pt_x[i] = SCREEN_W/2;
		pt_y[i] = 960;
		pt_vx[i] = (float)(rand()%101) / 33.333 - 1.5;
		pt_vy[i] = (float)(rand()%101) / 10.0 - 16.1;
		pt_t[i] = rand()%100;
	}
}

char *credits[] = {
	"Concept:\nLancer-X/Asceai",
	"Game design:\nLancer-X/Asceai",
	"Graphics:\nLancer-X/Asceai",
	"Programming:\nLancer-X/Asceai",
	"Sound Effects:\nVarious (public domain) sources",
	"Beta testing:\nQuasar",
	"Beta testing:\nTerryn",
	"Beta testing:\nWervyn",
	"GCW Zero port:\nZear",
	"With additional patches by:\nNebuleon & David Knight"
};

void DrawCredits()
{
	static SDL_Surface *fin = NULL;
	static SDL_Surface *theend[2] = {NULL};
	SDL_Rect draw_to;
	int i;
	int ypos;
	int c;
	int n_credits = sizeof(credits)/sizeof(*credits);
	int finish_point;
	
	finish_point = 400 + (n_credits * 50);
	
	draw_to.x = 384;
	draw_to.y = 352;
	
	SDL_FillRect(screen, NULL, 0);
	
	if (fin == NULL) {
		fin = IMG_Load("dat/i/fin.png");
		
		theend[0] = IMG_Load("dat/i/theend.png");
		theend[1] = IMG_Load("dat/i/true_end.png");
	}
	
	if (credits_scroll >= (finish_point + 80)) {
		SDL_BlitSurface(theend[(player_shield == 30)], NULL, screen, NULL);
	} else {
		SDL_BlitSurface(fin, NULL, screen, &draw_to);
		
		// Show each line of credits
		
		for (i = 0; i < n_credits; i++) {
			ypos = 800 + (i * 100) - credits_scroll * 2;
			
			if ((ypos >= 0)&&(ypos < 480)) {
				c = 255 - abs(ypos - 240);
				draw_text(60, ypos, 0, credits[i], c);
			}
		}
		
	}
	
	for (i = 0; i < 128; i++) {
		ending_pal[i].r = 0;
		ending_pal[i].g = i;
		ending_pal[i].b = i*2;
	}
	for (i = 128; i < 256; i++) {
		ending_pal[i].r = (i - 128)*2+1;
		ending_pal[i].g = i;
		ending_pal[i].b = 255;
	}
	
	// Dim palette if we're just starting
	
	if (credits_scroll < 80) {
		for (i = 0; i < 256; i++) {
			ending_pal[i].r = ending_pal[i].r * credits_scroll / 80;
			ending_pal[i].g = ending_pal[i].g * credits_scroll / 80;
			ending_pal[i].b = ending_pal[i].b * credits_scroll / 80;
		}
	}
	
	// Also palette if we're finishing
	
	if ((credits_scroll >= (finish_point))&&(credits_scroll < (finish_point + 80))) {
		for (i = 0; i < 256; i++) {
			ending_pal[i].r = ending_pal[i].r * (finish_point+80-credits_scroll) / 80;
			ending_pal[i].g = ending_pal[i].g * (finish_point+80-credits_scroll) / 80;
			ending_pal[i].b = ending_pal[i].b * (finish_point+80-credits_scroll) / 80;
		}
	}
	
	if ((credits_scroll >= (finish_point + 80))&&(credits_scroll < (finish_point + 160))) {
		for (i = 0; i < 256; i++) {
			ending_pal[i].r = ending_pal[i].r * (credits_scroll - (finish_point + 80)) / 80;
			ending_pal[i].g = ending_pal[i].g * (credits_scroll - (finish_point + 80)) / 80;
			ending_pal[i].b = ending_pal[i].b * (credits_scroll - (finish_point + 80)) / 80;
		}
	}
	
	credits_scroll++;
	
	UpdatePalette();
	VideoUpdate();
}

void RunParticleStorm(int offset)
{
	SDL_Rect draw_from, draw_to;
	int i;
	
	for (i = 0; i < 64; i++) {
		DrawRect(0, i * 15 - offset, SCREEN_W, 15, 64 - i);
	}
	
	for (i = 0; i < 500; i++) {
		if (pt_t[i] > 0) {
			pt_t[i]--;
		} else {
			pt_vy[i] += 0.1;
			pt_x[i] += pt_vx[i];
			pt_y[i] += pt_vy[i];
		}
		
		draw_from.x = (rand()%3)*32;
		draw_from.y = 0;
		draw_from.w = 32;
		draw_from.h = 32;
	
		draw_to.x = (int)pt_x[i] - 16;
		draw_to.y = (int)pt_y[i] - 16 - offset;
		SDL_BlitSurface(glitter, &draw_from, screen, &draw_to);
	}

	for (i = 0; i < 128; i++) {
		ending_pal[i].r = i*2;
		ending_pal[i].g = i*2;
		ending_pal[i].b = 0;
	}
	for (i = 128; i < 256; i++) {
		ending_pal[i].r = 255;
		ending_pal[i].g = 255;
		ending_pal[i].b = (i - 128)*2+1;
	}
	
	UpdatePalette();
	VideoUpdate();
}

void DrawStream(int t)
{
	int i;
	int scr_x = 32;
	int scr_y = 0;
	int strm_scrl;
	SDL_Rect draw_from, draw_to;
	
	for (i = 0; i < 256; i++) {
		ending_pal[i].r = i;
		ending_pal[i].g = (i * 7 / 8) + 16 + sin( (float)t / 8 )*16;
		ending_pal[i].b = (i * 3 / 4) + 32 + sin( (float)t / 8 )*32;
	}
	

	if (t >= 300) {
		scr_x = 32 + rand()%32 - rand()%32;
		scr_y = rand()%8;
	}
	
	if (t < 10) {
		scr_y = (20 - t * 2);
	}
	
	DrawLevel(scr_x + SCREEN_W/2, scr_y, 0, 0);
	DrawPlayer(344 - scr_x - SCREEN_W/2, scr_y + SCREEN_H/2 + SCREEN_H/4, 0, 0);

	for (i = 0; i < 7; i++) {
		strm_scrl = (t * 20) % 128;
		draw_to.x = 0 - strm_scrl - scr_x + (128*i);
		draw_to.y = 19 - scr_y;
		
		if (i >= 300) {
			draw_to.y += rand()%4;
			draw_to.y -= rand()%4;
		}
		SDL_BlitSurface(streamspr, NULL, screen, &draw_to);
	}
	
	// glitter
	for (i = 0; i < 20; i++) {
		draw_from.x = (rand()%3)*32;
		draw_from.y = 0;
		draw_from.w = 32;
		draw_from.h = 32;
	
		draw_to.x = rand()%(SCREEN_W+32)-32;
		draw_to.y = (rand()%(124)) + 3;
		
		SDL_BlitSurface(glitter, &draw_from, screen, &draw_to);
	}
	
	if (t > 250) {
		if (t < 300) {
			if (t == 251) {
				SND_CircuitRelease(1000);
			}
			DrawCircle(344 - scr_x - SCREEN_W/2 + 8, scr_y + SCREEN_H/2 + SCREEN_H/4 + 12, (t - 254) * 10, 255);
			DrawCircle(344 - scr_x - SCREEN_W/2 + 8, scr_y + SCREEN_H/2 + SCREEN_H/4 + 12, (t - 252) * 10, 225);
			DrawCircle(344 - scr_x - SCREEN_W/2 + 8, scr_y + SCREEN_H/2 + SCREEN_H/4 + 12, (t - 250) * 10, 195);
		}
	}
	
	UpdatePalette();
	VideoUpdate();
}

char *PText[10] = {	"Activating the seal quickly unblocked the ley lines and allowed",
			"PSI to flow through the Dome again. The remaining shadows were",
			"quickly flushed out.",
			"",
			"Wervyn Anixil's unconventional use of the PSI resulted in him",
			"being burned out and rendered powerless. Merit will see to it",
			"that he faces judgement for his crimes.",
			"",
			"Neither of the two PSI weapons housed within the Dome had been",
			"touched. However . . ." };
char *PTextV[10] ={	"Activating the seal quickly unblocked the ley lines and allowed",
			"PSI to flow through the Dome again. The remaining shadows were",
			"quickly flushed out.",
			"",
			"The traitor, who was never identified, perished in the Sealing.",
			"It soon became clear that the traitor had managed to betray and",
			"kill the real Wervyn Anixil during his experiments on the PSI.",
			"If the Agate Knife was never found, nobody would have been any",
			"the wiser, and things could have turned out very differently.",
			"However, there was one last thing for MERIT to do."};

void DrawPText(int t)
{
	int i;
	int c;
	
	int x, y;
	
	for (i = 0; i < 256; i++) {
		ending_pal[i].r = i;
		ending_pal[i].g = i;
		ending_pal[i].b = (i * 3 / 4) + 64;
	}

	for (i = 0; i < (32 * 8); i++) {
		x = (i % 32)*20;
		y = (i / 32)*20;
		
		c = 237 + (i/32*2) + (rand()% (19 - (i/32) *2));
		DrawRect(x, y, 20, 20, c);
		c = 237 + (i/32*2) + (rand()% (19 - (i/32) *2));
		DrawRect(x, 460 - y, 20, 20, c);
	}
	
	if (t < 300) {
	
		for (i = 0; i < 10; i++) {
			c = (255 + (i * 100) - t*10);
			if (c < 0) c = 0;
			if (c > 255) c = 255;
			
			if (player_shield != 30) {
				draw_text(10, 20+i*20, 0, PText[i], c);
			} else {
				draw_text(10, 20+i*20, 0, PTextV[i], c);
			}
		}
	} else {
		for (i = 0; i < 10; i++) {
			c = 5 + (t-300) * 5;
			
			if (player_shield != 30) {
				draw_text(10, 20+i*20, 0, PText[i], c);
			} else {
				draw_text(10, 20+i*20, 0, PTextV[i], c);
			}
		}
	}
	
	UpdatePalette();
	VideoUpdate();
}

void DrawScrolly(int t)
{
	int xp;
	int yp;
	int i, j;
	float a_dir;
	float v_radius;
	int all_blue = 0;
	SDL_Rect draw_from, draw_to;
	
	int x, y, r;
	
	float bright;
	
	if (t < 795) {
		xp = 8192 - 320 - 3180 + (t * 4);
		yp = t * 20;
	} else {
		xp = 8192 - 320 + ( (t-795) * 10);
		yp = 795 * 20 - (t-795)*10;
	}
	
	// Palette
	
	
	if ((rand() % 10)==9) {
		all_blue = 1;
	}
	for (i = 0; i < 256; i++) {
		bright = sin((float)t / 10.0) * 0.2 + 0.4;
		ending_pal[i].r = (i * bright + (256*(1.0-bright))) * ((float)(all_blue == 0) * 0.5 + 0.5);
		ending_pal[i].g = (i * bright + (256*(1.0-bright))) * ((float)(all_blue == 0) * 0.5 + 0.5);
		ending_pal[i].b = i * bright + (256*(1.0-bright));
	}
	DrawLevel(xp, yp, 0, 0);
	
	v_radius = sin((float)t / 10.0)*20 + 100;
	
	for (i = 0; i < 5; i++) {
		x = rand()%SCREEN_W;
		y = rand()%SCREEN_H;
		r = rand()%500+100;
		
		DrawCircleEx(x, y, r+2, r-4, 128);
		DrawCircleEx(x, y, r, r-2, 255);
	}
	
	for (i = 0; i < 4; i++) {
		draw_from.x = (8 + i) * 32;
		draw_from.y = 0;
		draw_from.w = 32;
		draw_from.h = 32;
		
		a_dir = ((float)t / 10.0) + (M_PI*(float)i/2);
		
		for (j = 10; j >= 0; j--) {
			DrawCircleEx(SCREEN_W/2+cos(a_dir)*v_radius, SCREEN_H/2+sin(a_dir)*v_radius, 22 + j * 2, 0, abs(j-3) * 15);
		}
		DrawCircleEx(SCREEN_W/2+cos(a_dir)*v_radius, SCREEN_H/2+sin(a_dir)*v_radius, 20, 0, 0);
		
		draw_to.x = SCREEN_W/2 + cos(a_dir) * v_radius - 16;
		draw_to.y = SCREEN_H/2 + sin(a_dir) * v_radius - 16;
		SDL_BlitSurface(artifact_spr, &draw_from, screen, &draw_to);
	}
	
	UpdatePalette();
	VideoUpdate();
}

void DrawCircuitFlash(int t, int method)
{
	static SDL_Surface *circ = NULL;
	static int xpos, ypos;
	int i, j;
	SDL_Rect from;
	
	if (circ == NULL) {
		circ = IMG_Load("dat/i/circuits_1.png");
	}
	
	if (t == 0) {
		if (method == 0) {
			xpos = rand()%(641/2);
			ypos = rand()%(481/2);
		} else {
			xpos = SCREEN_W/2;
			ypos = SCREEN_H/2;
		}
	}
	
	from.x = xpos;
	from.y = ypos;
	from.w = SCREEN_W;
	from.h = SCREEN_H;
	
	SDL_BlitSurface(circ, &from, screen, NULL);
	
	for (i = 0; i < 256; i++) {
		if (method == 0) {
			j = i * t / 4;
		} else {
			j = i * t / 8;
			if (t >= 20) {
				j += t * 25;
			}
		}
		
		if (j > 255) j = 255;
		ending_pal[i].r = j;
		ending_pal[i].g = j;
		ending_pal[i].b = j;
	}
	
	UpdatePalette();
	VideoUpdate();
}
