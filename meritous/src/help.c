//
//   help.c
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
#include <string.h>

#include "levelblit.h"
extern SDL_Surface *ScreenSurface;
struct help_line {
	char *t;
};

struct help_section {
	int lines;
	char *identifier;
	struct help_line *l[256];
};

struct help_file {
	int sections;
	struct help_section *s[256];
};

struct help_file *hlp = NULL;
int my_line;
int my_sec;
int my_cursor;
int my_link;
int return_top;
void InitHelp()
{
	FILE *fp;
	struct help_section *current_sec = NULL;
	struct help_line *current_line = NULL;
	char linebuf[80];
	hlp = malloc(sizeof(struct help_file));
	hlp->sections = 0;

	fp = fopen("dat/d/helpfile.txt", "r");
	while (fgets(linebuf, 79, fp)) {
		if (linebuf[strlen(linebuf)-1] == '\n')
			linebuf[strlen(linebuf)-1] = 0;

		if (linebuf[0] == '\'') {
			// comment
			continue;
		}
		if (linebuf[0] == ':') {
			// section
			hlp->s[hlp->sections] = malloc(sizeof(struct help_section));
			current_sec = hlp->s[hlp->sections];
			hlp->sections++;
			current_sec->identifier = malloc(strlen(linebuf));
			current_sec->lines = 0;
			strcpy(current_sec->identifier, linebuf+1);
			continue;
		}

		// different line
		if (current_sec != NULL) {
			int i;
			int char_in_line;
			int last_space = -1;
			int start = 0;
			int line_len = (SCREEN_W - 40-4)/8;
			int text_len;

			char *temp_line = malloc(strlen(linebuf)+1);
			if (!temp_line)
			{
				fclose(fp);
				return;
			}

			strcpy(temp_line, linebuf);

			for (i = 0, char_in_line = 0; temp_line[i] != '\0'; ++i, ++char_in_line) {
				if (temp_line[i] == ' ') {
					last_space = i;
				}
				else if (temp_line[i] == '\n') {
					last_space = -1;
					char_in_line = -1;
				}

				if (char_in_line >= line_len) {
					char_in_line = i - last_space -1;
					if (last_space >= 0) {
						temp_line[last_space] = '\n';
						last_space = -1;
					}
				}
			}

			text_len = strlen(temp_line);
			last_space = -1;

			for (i = 0; i < text_len; ++i) {
				if (temp_line[i] == '\n') {
					temp_line[i] = '\0';

					current_sec->l[current_sec->lines] = malloc(sizeof(struct help_line));
					current_line = current_sec->l[current_sec->lines];
					current_sec->lines++;
					current_line->t = malloc(strlen(&temp_line[start])+1);
					strcpy(current_line->t, &temp_line[start]);

					start = i + 1;
				}
			}

			current_sec->l[current_sec->lines] = malloc(sizeof(struct help_line));
			current_line = current_sec->l[current_sec->lines];
			current_sec->lines++;
			current_line->t = malloc(strlen(&temp_line[start])+1);
			strcpy(current_line->t, &temp_line[start]);

			free(temp_line);
		}
	}
	fclose(fp);
}

void DisplayHelp()
{
	static int tick = 0;
	int i;
	struct help_section *current_sec = NULL;
	char *ltext;
	char c_ident[20];
	int line_num;
	int follow_link = 0;
	char linkfollow[20] = "";
	int offset_x = 8;
	int offset_y = 5;

	DrawRect(0, 0, SCREEN_W, SCREEN_H, 0);
	DrawRect(1, 1, SCREEN_W - 2, SCREEN_H - 2, 200);
	DrawRect(2, 2, SCREEN_W - 4, SCREEN_H - 4, 255);
	DrawRect(3, 3, SCREEN_W - 6, SCREEN_H - 6, 200);
	DrawRect(4, 4, SCREEN_W - 8, SCREEN_H - 8, 100);
	DrawRect(7, 7, SCREEN_W - 14, SCREEN_H - 14, 20);
	DrawRect(12, 12, SCREEN_W - 24, SCREEN_H - 24, 60);

	// 70x40 display
	current_sec = hlp->s[my_sec];

	my_line = my_cursor - 22;
	if (my_line < 0) my_line = 0;
	if (my_line >= (current_sec->lines)) my_line = current_sec->lines - 1;
	for (i = 0; i < 2; i++) {
		draw_text(23+i - offset_x, (my_cursor - my_line)*10 - offset_y, 0, "->", 255);
	}

	for (i = 2; i < 23; i++) {
		line_num = my_line + i;
		if (line_num >= 0) {
			if (line_num < current_sec->lines) {
				ltext = current_sec->l[line_num]->t;

				switch (ltext[0]) {
					case '!':
						draw_text((SCREEN_W-strlen(ltext+1)*8)/2, i*10 - offset_y, 0, ltext+1, 255);
						break;
					case '?':
						if (i < 23) {
							strncpy(c_ident, ltext+1, strchr(ltext+1, '?')-ltext-1);
							c_ident[strchr(ltext+1, '?')-ltext-1] = 0;

							draw_text(40 - offset_x, i*10 - offset_y, 0, strchr(ltext+1, '?')+1, my_cursor == line_num ? 200+(tick%16)*3 : 150);
							if ((my_link == 1)&&(my_cursor == line_num)) {
								follow_link = 1;
								strcpy(linkfollow, c_ident);
							}
						}
						break;
					default:
						draw_text(40 - offset_x, i*10 - offset_y, 0, ltext, 200);
						break;
				}
			}
		}
	}
	tick++;
	//SDL_Flip(screen);
	SDL_SoftStretch(screen, NULL, ScreenSurface, NULL);
	SDL_Flip(ScreenSurface);

	if (return_top)
	{
		follow_link = 1;
		strcpy(linkfollow, "default");
	}

	if (follow_link) {
		for (i = 0; i < hlp->sections; i++) {
			if (strcmp(linkfollow, hlp->s[i]->identifier) == 0) {
				my_sec = i;
				my_cursor = (my_sec == 0 ? 11 : 6);
				break;
			}
		}
		my_link = 0;
		return_top = 0;
	}
}

int MoveCursor()
{
	SDL_Event ev;
	static int key_delay = 0;
	static int key_up = 0, key_down = 0, key_left = 0, key_right = 0;

	if (key_delay > 0) key_delay--;

	my_link = 0;
	return_top = 0;
	while (SDL_PollEvent(&ev)) {
		if (ev.type == SDL_KEYDOWN) {
			if (ev.key.keysym.sym == SDLK_DOWN) {
				key_down = 1;
				key_delay = 10;
				if (my_cursor < hlp->s[my_sec]->lines-1) my_cursor++;
			}
			if (ev.key.keysym.sym == SDLK_UP) {
				key_up = 1;
				key_delay = 10;
				if (my_cursor > (my_sec == 0 ? 11 : 6)) my_cursor--;
			}
			if (ev.key.keysym.sym == SDLK_LEFT) {
				key_left = 1;
				key_delay = 10;
				my_cursor-=10;
				if (my_cursor <= (my_sec == 0 ? 11 : 6)) my_cursor = (my_sec == 0 ? 11 : 6);
			}
			if (ev.key.keysym.sym == SDLK_RIGHT) {
				key_right = 1;
				key_delay = 10;
				my_cursor+=10;
				if (my_cursor >= hlp->s[my_sec]->lines-1) my_cursor = hlp->s[my_sec]->lines-1;
			}
			if (ev.key.keysym.sym == SDLK_ESCAPE) {
				key_delay = 0;
				key_up = 0;
				key_down = 0;
				key_left = 0;
				key_right = 0;
				return 0;
			}
			if (ev.key.keysym.sym == SDLK_BACKSPACE) {
				key_delay = 0;
				key_up = 0;
				key_down = 0;
				key_left = 0;
				key_right = 0;
				return 0;
			}
			if ((ev.key.keysym.sym == SDLK_LCTRL) || (ev.key.keysym.sym == SDLK_RETURN)) {
				my_link = 1;
			}
			if (ev.key.keysym.sym == SDLK_LALT) {
				return_top = 1;
			}
		}

		if (ev.type == SDL_KEYUP) {
			if (ev.key.keysym.sym == SDLK_DOWN) {
				key_down = 0;
			}
			if (ev.key.keysym.sym == SDLK_UP) {
				key_up = 0;
			}
			if (ev.key.keysym.sym == SDLK_LEFT) {
				key_left = 0;
			}
			if (ev.key.keysym.sym == SDLK_RIGHT) {
				key_right = 0;
			}
		}
		if (ev.type == SDL_QUIT) {
			key_delay = 0;
			key_up = 0;
			key_down = 0;
			key_left = 0;
			key_right = 0;
			return 0;
		}
	}

	if (key_delay == 0) {
		if (key_up == 1) {
			if (my_cursor > (my_sec == 0 ? 11 : 6)) my_cursor--;
		}
		if (key_down == 1) {
			if (my_cursor < hlp->s[my_sec]->lines-1) my_cursor++;
		}
		if (key_left == 1) {
			my_cursor-=10;
			if (my_cursor <= (my_sec == 0 ? 11 : 6)) my_cursor = (my_sec == 0 ? 11 : 6);
		}
		if (key_right == 1) {
			my_cursor+=10;
			if (my_cursor >= hlp->s[my_sec]->lines-1) my_cursor = hlp->s[my_sec]->lines-1;
		}
	}

	return 1;
}

void ShowHelp()
{
	int in_help = 1;
	if (hlp == NULL) {
		InitHelp();
	}
	my_line = 0;
	my_sec = 0;
	my_cursor = 11;
	my_link = 0;
	return_top = 0;

	while (in_help)
	{
		DisplayHelp();
		in_help = MoveCursor();
		SDL_Delay(30);
	}
}
