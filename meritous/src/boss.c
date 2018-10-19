//
//   boss.c
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
#include <SDL.h>
#include <SDL_image.h>

#include "levelblit.h"
#include "mapgen.h"
#include "demon.h"
#include "gamemap.h"
#include "audio.h"

char *boss_names[] = {	"MERIDIAN",
			"ATARAXIA",
			"MERODACH",
			"WERVYN ANIXIL" };

int boss_fight_mode = 0;
// 0 - no boss fight
// 1 - entering boss room
// 2 - fiting teh boss
// 3 - boss dying
// 4-23 - final boss dying sequence

int current_boss_room = 0;
int current_boss = 0;
int boss_x, boss_y;
int boss_hp;
int boss_lives;
int boss_tail_len = 0;
int boss_breakpoint;
int boss_flash = 0;
int boss_new_life = 0;
int boss_ox, boss_oy;
int boss_dlg = 0;
int resetboss = 0;
int final_boss_dlg = 0;
float boss_dmgrate = 1;
float boss_dir;

int proxy_seek = 0;

int boss_m_heads;
int boss_m_hx[4];
int boss_m_hy[4];
float boss_m_hd[4];

float boss_2h_dir = 0;
int boss_2h_dst = 64;

int percent_required[4] = {15, 40, 60, 75};

char *artifact_names[] = { 	"Holy Sword 'Balmung'",
							"Mystic Halberd 'Amenonuhoko'",
							"Divine Bow 'Gandiva'",
							"Cursed Seal of Yahveh"};
							
int CanGetArtifact();
void DrawArtifactOverhead(int p_obj);

struct dlg_box {
	int pos;
	int lines;
	int last;
	char *txt;
};

struct dlg_box dtext[5][12] =
//      "This string of text is precisely seventy eight characters in length, usefully."
{
  { // Meridian
	{0, 5, 0,
		"MERIDIAN:\n"
		"\n"
		"Different...\n"
		"\n"
		"Your PSI is different. Your magic is perverse. Poisonous. Treacherous."
	},
	{1, 5, 0,
		"MERIT:\n"
		"\n"
		"Nothing but darkness carried on the wind of a curse. It's amazing that you\n"
		"are able to communicate with me at all. Surely a statement on the perversity\n"
		"of my magic, coming from you, is more than a little bit hypocritical."
	},
	{0, 6, 1,
		"MERIDIAN:\n"
		"\n"
		"So. You've even brought that tainted thing with you.\n"
		"That does not really surprise me, but nonetheless, your weak, impure PSI\n"
		"will do you little good against one manifest of shadows. Pitiful thing from\n"
		"the surface, MERIDIAN shall return you to the dust from whence you came!"
	}
  },
  
  { // Ataraxia
	{0, 5, 0,
		"ATARAXIA:\n"
		"\n"
		"It felt nearly as inconsequential as a stilling in the breeze, but there was\n"
		"more to it, wasn't it? It feels as though I lost a part of myself. Was that\n"
		"your doing?"
	},
	{1, 6, 0,
		"MERIT:\n"
		"\n"
		"Moving Ochra's Keys from the ley lines was only the start, am I correct?\n"
		"Nonetheless, what you've already caused is enough. The Keys were placed to\n"
		"prevent corruption in the PSI. Moving them renders the PSI unusable. There\n"
		"is nothing to be gained from THAT."
	},
	{0, 8, 0,
		"ATARAXIA:\n"
		"\n"
		"Statements like that belie your youthful naivety and ignorance. There is no\n"
		"way to make magic unusable. What you see now is the other side of PSI that\n"
		"pitiful beings such as yourself turn a blind eye to. Like the sun at dawn\n"
		"the sun at dusk, it is nonetheless the same sun, and each beautiful in its\n"
		"own way. PSI isn't just for things like you, but for the entire cosmos.\n"
		"Nothing can be gained by preventing it from taking its natural course."
	},
	{1, 8, 0,
		"MERIT:\n"
		"\n"
		"You delude yourself with the crazed whisperings from the muck that spawned\n"
		"you. You say that this PSI is still PSI, but it is not. PSI is the energy of\n"
		"creation. This is the energy of destruction. PSI is a pure force, gathered\n"
		"from around me and targeted with my mind. This is a foul mixture, existing\n"
		"as half-mad corruptions like you. It can only break apart when confronted\n"
		"with true power."
	},
	{0, 6, 1,
		"ATARAXIA:\n"
		"\n"
		"You realise that not even your fellow PSI users agree with you? But, it is\n"
		"of little consequence. I'll show you the true beauty of that which you shun\n"
		"by smothering you in it and turning your flesh into fuel for the PSI that\n"
		"runs through the veins of this Dome!"
	}
  },
  
  { // Merodach
	{0, 9, 0,
		"MERODACH:\n"
		"\n"
		"E\1v\1e\1r\1y\1t\1h\1i\1n\1g   h\1a\1s   t\1u\1r\1n\1e\1d   t\1o   d\1a\1r\1k\1n\1e\1s\1s\1.\n"
		"\n"
		"N\1o   s\1o\1u\1n\1d   n\1o   l\1i\1g\1h\1t   n\1o   s\1e\1n\1s\1a\1t\1i\1o\1n\1.\n"
		"\n"
		"W\1a\1s   t\1h\1i\1s   w\1h\1a\1t   y\1o\1u   w\1e\1r\1e   a\1f\1t\1e\1r   f\1r\1o\1m\n"
		"\n"
		"t\1h\1e   b\1e\1g\1i\1n\1n\1i\1n\1g\1?"
	},
	{0, 7, 1,
		"MERODACH:\n"
		"\n"
		"I\1'\1l\1l   w\1r\1a\1p   m\1y   r\1e\1m\1a\1i\1n\1i\1n\1g   P\1S\1I   a\1r\1o\1u\1n\1d   y\1o\1u\n"
		"\n"
		"a\1n\1d   e\1x\1t\1i\1n\1g\1u\1i\1s\1h   t\1h\1a\1t   l\1i\1f\1e   o\1f   y\1o\1u\1r\1s\1.\n"
		"\n"
		"L\1e\1t\1'\1s   s\1e\1e   h\1o\1w   y\1o\1u   e\1n\1j\1o\1y   e\1t\1e\1r\1n\1a\1l   r\1e\1s\1t\1!"
	}
  },
  
  { // Wervyn Anixil
    {1, 6, 0,
		"MERIT:\n"
		"\n"
		"ANIXIL? So, that's how it is. I assume you worked out a way of using the\n"
		"PSI in this state. So, not happy with your allotted PSI from the Dome, you\n"
		"decided to render it in this form and take it all for yourself.\n"
		"What was your reasoning for doing something this rash, though?"
	},
	
	{0, 9, 0,
		"WERVYN ANIXIL:\n"
		"\n"
		"All you seem to do is complain. That's what I've discovered through this,\n"
		"and it's definitely part of the reason I did it. I do creative things like\n"
		"this sort of 'punishment' because it lets me better understand the nature of\n"
		"all the forces involved. It tells me that you, MERIT, really do not have a\n"
		"clue how to relate to other PSI users, and that for the most part you're\n"
		"entirely self-absorbed. It tells me that, while PSI users in general doesn't\n"
		"really 'get' the ethos of the forces involved here, and that the rerendered"
	},
	
	{0, 9, 0,
		"WERVYN ANIXIL:\n"
		"\n"
		"PSI, as intelligent as it is, pretty much entirely understands it, though it\n"
		"was mistaken that the rendering of this form was intentional.\n"
		"\n"
		"Regarding that, by the way, THAT is the reason I thought you needed a\n"
		"punishment. Making ridiculously inefficient, wasteful use of the limited PSI\n"
		"we have available wouldn't have been a problem if it was just the normal PSI\n"
		"like we'd planned it to be. But you knew full well that PSI users who wanted"
	},
	
	{0, 9, 0,
	    "WERVYN ANIXIL:\n"
		"\n"
		"nothing to do with it were being greatly inconvenienced by usage clogging up\n"
		"the limited ley-lines we have. That makes you an asshole, a deliberate prick\n"
		"to everyone in the world because you thought you could get away with it, and\n"
		"that it wouldn't matter.\n"
		"\n"
		"I also take issue with this nonsense about your punishment being severe,\n"
		"because it's not. You can still use your PSI as you used it before, to a"
	},
	
	{0, 9, 0,
		"WERVYN ANIXIL:\n"
		"\n"
		"similar level of effectiveness. I specifically allowed access to THIS form\n"
		"of PSI so that you couldn't complain that you couldn't use magic. Now\n"
		"exiling you to the Dome had multiple meanings. One was to effectively tell\n"
		"you, 'I don't think you have anything of value to do, so I'm putting you in\n"
		"a place where no one does anything of value.' I still think that, by the\n"
		"way. Another was, like I said, to see what you would do. Maybe you would\n"
		"actually cast something worth the PSI, then. Or maybe you would take the"
	},
	
	{0, 8, 0,
		"WERVYN ANIXIL:\n"
		"\n"
		"hint and just stop it for the week until I decided you'd had enough. But you\n"
		"couldn't do either of those things, because all you do is complain.\n"
		"\n"
		"I want you to take a look at yourself and ask what you want to accomplish\n"
		"here, and what you think is keeping you here. Obviously you have some\n"
		"attachment, even though you keep saying the new PSI is worthless. This isn't"
	},
	
	{0, 9, 0,	
		"WERVYN ANIXIL:\n"
		"\n"
		"true, of course, ATARAXIA really captures a good point here. For the most\n"
		"part, PSI users just find you extremely annoying because you continually set\n"
		"yourself at odds with pretty much every kind of PSI usage but your own, and\n"
		"then complain about it. Do you want things to be different? They certainly\n"
		"can be. 'EVEN IF I DO CHANGE, THEN MY PSI WON'T BE GOOD ENOUGH!' That's\n"
		"bullshit. All other PSI users seem to be on the path to figuring out how to\n"
		"just use it. You can too."
	},
	
	{0, 9, 0,
		"WERVYN ANIXIL:\n"
		"\n"
		"Your 'punishment' is up tomorrow, I'd only planned to do it for a week. Of\n"
		"course, you can continue complaining, and I may have to think of something\n"
		"else to do to you. But I'm not going to kill you. If you want to leave this\n"
		"mortal coil because you can't take it anymore, then leave. If you don't want\n"
		"to leave, then start working out your issues with the new PSI, I actually\n"
		"would love to help. And if you don't want to do that, then I'm sorry, but\n"
		"you're going to continue being useless at magic, because the way you are"
	},
	
	{0, 4, 0,
		"WERVYN ANIXIL:\n"
		"\n"
		"now, it's not surprising at all that your PSI ability is as stunted as it\n"
		"is."
	},
	
	{1, 4, 0,
		"MERIT:\n"
		"\n"
		"The tainted PSI has driven even you mad. Mad with power. This is a blow I\n"
		"I must strike, for everyone. For humanity."
	},
	
	{0, 3, 1,
		"WERVYN ANIXIL:\n"
		"\n"
		"I'll meet you at the point of diminishing returns."
	}
  },
  
  { // Wervyn Anixil???? (with Agate Knife)
    {1, 8, 0,
		"MERIT:\n"
		"\n"
		"ANIXIL? So, that's how it is. I assume you worked out a way of using the\n"
		"PSI in this state. So, not happy with your allotted PSI from the Dome, you\n"
		"decided to render it in this form and take it all for yourself.\n"
		"What was your reasoning for doing something this rash, though?\n"
		"\n"
		"That was what you wanted to hear, right?"
	},
	
	{0, 7, 0,
		"WERVYN ANIXIL????:\n"
		"\n"
		"All you seem to do is complain. That's what I've discovered through this,\n"
		"and it's definitely part of the\n"
		"... ... ... ... ... ... ... ...\n"
		"... ... ... ... ... ... ... ... ...\n"
		"... ... oh."
	},
	
	{1, 6, 0,
		"MERIT:\n"
		"\n"
		"If I've learned anything in life, it's that things are often not quite as\n"
		"they seem. You couldn't use the Agate Knife, so you attempted to hide it.\n"
		"\n"
		"Unfortunately for you, I found it and recognised it."
	},
	
	{0, 7, 0,
	    "WERVYN ANIXIL????:\n"
		"\n"
		". . . . no. It's not just recognising the Knife at all. You were actually\n"
		"able to use it, too.\n"
		"\n"
		"So. You too knew the secret?"
	},
	
	{1, 5, 0,
		"MERIT:\n"
		"\n"
		"I was one of the few that he told.\n"
		"\n"
		"And so I know."
	},
	
	{0, 3, 0,
		"????:\n"
		"\n"
		"Umm..."
	},
	
	{1, 7, 0,	
		"MERIT:\n"
		"\n"
		"Well, it wasn't a bad attempt. If I hadn't found THAT, I would never have\n"
		"known.\n"
		"\n"
		"Now that that's taken care of, I might as well finish what I came here for.\n"
		"Prepare yourself."
	},
	
	{0, 3, 1,
		"????:\n"
		"\n"
		"I won't hold back! There's not a lot of point, now."
	}
  }
};

void BossDialog()
{
	int ypos;
	int dialog_set;

	dialog_set = current_boss;
	
	if (current_boss == 3) {
		if (player_shield == 30) {
			dialog_set = 4;
		}
	}
	
	if (enter_pressed) {
		if (dtext[dialog_set][boss_dlg-1].last) {
			boss_dlg = 0;
			boss_fight_mode = 2;
		} else {
			boss_dlg++;
		}
		enter_pressed = 0;
	}
	if (boss_dlg > 0) {
		ypos = 34;

		DrawBorder(40, 16);
		draw_text(12, ypos+8, 8, dtext[dialog_set][boss_dlg-1].txt, 255);
	}
}

void InitBossVars()
{
	current_boss_room = 0;
	current_boss = 0;
	boss_tail_len = 0;
	boss_fight_mode = 0;
	boss_flash = 0;
	boss_new_life = 0;
	final_boss_dlg = 0;
}

void Curse()
{
	int i;
	int x, y;
	unsigned char tile;
	struct RoomConnection *rc;
	// Upon taking the cursed seal

	// Make the place of power your checkpoint
	
	checkpoint_x = rooms[place_of_power].w * 16 + rooms[place_of_power].x * 32;
	checkpoint_y = rooms[place_of_power].h * 16 + rooms[place_of_power].y * 32;
	
	// Turn the start room into a boss room, and clear it
	
	rooms[0].room_type = 2;
	Paint(rooms[0].x+1, rooms[0].y+1, rooms[0].w-2, rooms[0].h-2, "dat/d/fbossroom.loc");
	
	// Lock all unvisited rooms off
	
	for (i = 0; i < 3000; i++) {
		if (!rooms[i].visited) {
			rc = rooms[i].con;
			
			while (rc != NULL) {
				x = rc->x2;
				y = rc->y2;
				tile = Get(x, y);
				if ((tile >= 38)&&(tile <= 41)) tile = tile - 38 + 4;
				if ((tile >= 21)&&(tile <= 24)) tile = tile - 21 + 4;
				if ((tile >= 13)&&(tile <= 16)) tile = tile - 13 + 4;
				Put(x, y, tile, GetRoom(x, y));
				rc = rc->n;
			}
			rooms[i].con = NULL;
			rooms[i].connections = 0;
		}
	}
	
	// Every 25th monster becomes a curse. All other monsters are deleted
	
	CurseEnemies();
}

void DrawPowerObject()
{
	int p_x, p_y;
	int p_obj = rooms[player_room].room_param;
	static int tick = 0;
	static int collect = 0;
	int i;
	int required_enemies;
	int n_artifacts;
	int dx, dy;
	int hover_v, off_v;
	float ddir;
	int dmag;
	
	if (place_of_power == player_room) p_obj = 3;
	
	n_artifacts = current_boss + artifacts[8] + artifacts[9] + artifacts[10];
	
	required_enemies = total_enemies * (percent_required[n_artifacts]) / 100;
	
	if (rooms[player_room].room_type == 4) required_enemies = 0;
	
	SDL_Rect from, to;
	
	hover_v = 16;
	off_v = 48;
	
	p_x = (rooms[player_room].w * 32 / 2 - 16) + rooms[player_room].x * 32;

	if (!game_paused) {
		if ((rooms[player_room].room_type == 5) || (rooms[player_room].room_type == 6)) {
			if (CanGetArtifact()) {
				if ((Get((player_x+PLAYERW/2)/32, (player_y+PLAYERH/2)/32)==42) ||
					(PlayerDist(rooms[player_room].w * 16 + rooms[player_room].x * 32,
								rooms[player_room].h * 16 + rooms[player_room].y * 32) < 32)) {
					if (rooms[player_room].enemies == 0) {
						off_v = 48 - (collect * 48 / 100);
						hover_v = 16 - (collect * 16 / 100);
						
						DrawCircle(player_x + PLAYERW/2 - scroll_x, player_y + PLAYERH/2- scroll_y, (100-collect) * 4 + 1, rand()%192+64);
						collect++;
						if (collect == 1) {
							SND_Pos("dat/a/crystal2.wav", 100, 0);
						}
						if (collect > 100) {
							collect = 0;
							rooms[player_room].room_type = 4;
							artifacts[8 + p_obj] = 1;
							specialmessage = 30 + p_obj;
							specialmessagetimer = 120;
							
							if (p_obj == 3) {
								Curse();
							}
						}
					}
				}
			}
		}
	}
	p_y = (rooms[player_room].h * 32 / 2 - 16) + rooms[player_room].y * 32 - off_v + sin((float)tick / 20.0)*hover_v;
	
	from.x = (8 + p_obj) * 32;
	from.y = 0;
	from.w = 32;
	from.h = 32;
	
	to.x = p_x - scroll_x;
	to.y = p_y - scroll_y;
	
	if (killed_enemies >= required_enemies) {
		DrawCircle(p_x + 16 - scroll_x, p_y + 16 - scroll_y, 34+rand()%5 + collect * 4 + 1, rand()%128+128);
	}
	SDL_BlitSurface(artifact_spr_large, &from, screen, &to);
	//printf("Required: %d   Killed %d   Value: %d\n", required_enemies, killed_enemies, (int)(sqrt((required_enemies - killed_enemies)/4)+5));

	if(required_enemies - killed_enemies > 0)
	{
		for (i = 0; i < (int)(sqrt((required_enemies - killed_enemies))+5); i++) { // !! Converting a sqrt from a negative number (-nan) to int is evil as it gives different results on different cpu architectures!
			ddir = (float)(rand()%256) * M_PI * 2 / 256;
			dmag = rand()%20;
			dx = p_x + 16 + cos(ddir)*dmag;
			dy = p_y + 16 + sin(ddir)*dmag;
			DrawCircle(dx - scroll_x, dy - scroll_y, rand()%5+1, 0);
			DrawCircle(dx - scroll_x, dy - scroll_y, rand()%3+1, 64);
		}
	}
	
	tick++;
}

void DrawBossHP(int bar_length)
{
	static SDL_Surface *boss_hp_icon = NULL;
	int draw_amt;
	Uint8 draw_col;
	SDL_Rect to;
	to.x = 0;
	to.y = 29;
	
	if (boss_hp_icon == NULL) {
		boss_hp_icon = IMG_Load("dat/i/boss_icon.png");
	}
	SDL_BlitSurface(boss_hp_icon, NULL, screen, &to);
	DrawRect(16, 28, SCREEN_W - 16, 17, 0);
	DrawRect(17, 29, SCREEN_W - 18, 15, 32);
	DrawRect(18, 30, SCREEN_W - 20, 13, 64);
	
	draw_col = 128 + (boss_hp * 127 / 1000);

	if (bar_length < 100) {
		draw_amt = bar_length * (SCREEN_W - 38) / 100;
	} else {
		draw_amt = boss_hp * (SCREEN_W - 38) / 1000;
	}

	DrawRect(20, 32, draw_amt+2, 9, draw_col * 2 / 3);
	DrawRect(21, 33, draw_amt, 7, draw_col);
	
	
	if ( (current_boss == 3) && (player_shield == 30) ) {
		draw_text(22, 33, 0, "??????????????", 0);
	} else {
		draw_text(22, 33, 0, boss_names[current_boss], 0);
	}
}

int PDir(int x1, int y1, int x2, int y2)
{
	int dx, dy;
	
	dx = x2 - x1;
	dy = y2 - y1;
	
	return atan2(dy, dx);
}

float CHDir(float original_dir, float new_dir, float dir_delta)
{
	while (original_dir < 0) original_dir += M_PI * 2;
	while (new_dir < 0) new_dir += M_PI * 2;
	
	original_dir = fmodf(original_dir, M_PI * 2) + M_PI * 2;
	new_dir = fmodf(new_dir, M_PI * 2) + M_PI * 2;

	if (fabs(original_dir - new_dir) <= dir_delta) return new_dir;
	if (fabs((original_dir + M_PI * 2) - new_dir) <= dir_delta) return new_dir;
	if (fabs(original_dir - (new_dir + M_PI * 2)) <= dir_delta) return new_dir;	
	
	if (original_dir > new_dir) {
		if ((original_dir - new_dir) <= M_PI) {
			return original_dir - dir_delta;
		}
		if (((new_dir + M_PI*2) - original_dir) <= M_PI) {
			return original_dir + dir_delta;
		}
	} else {
		if ((new_dir - original_dir) <= M_PI) {
			return original_dir + dir_delta;
		}
		if (((original_dir + M_PI*2) - new_dir) <= M_PI) {
			return original_dir - dir_delta;
		}
	}
	
	exit(1);
	
	return original_dir;
}

void TryHurtBoss(int x, int y, int range, int power)
{
	int atk_power;
	if (boss_new_life) return;
	if (dist(x, y, boss_x, boss_y) <= range) {
		atk_power = 400 * power / circuit_size + power / 75;
		if (atk_power > boss_breakpoint) {
			boss_hp -= (atk_power - boss_breakpoint) * boss_dmgrate;
			SND_Pos("dat/a/enemyhit.wav", 128, dist(x, y, boss_x, boss_y) / 4);
			boss_flash = 40;
			if (boss_hp <= 0) {
				boss_new_life = 1;
				boss_hp = 0;
			}
		}
	}
}

int BossMovement(int move_x, int move_y)
{
	if (!IsSolid(Get( (move_x - 12 + (current_boss==3)*6)/32, (move_y - 12 + (current_boss==3)*4)/32))) {
		if (!IsSolid(Get( (move_x + 12 - (current_boss==3)*6)/32, (move_y - 12 + (current_boss==3)*4)/32))) {
			if (!IsSolid(Get( (move_x - 12 + (current_boss==3)*6)/32, (move_y + 12 - (current_boss==3)*4)/32))) {
				if (!IsSolid(Get( (move_x + 12 - (current_boss==3)*6)/32, (move_y + 12 - (current_boss==3)*4)/32))) {
					return 1;
				}
			}
		}
	}
	return 0;
}

void DrawBoss()
{
	int flash_coeff;
	static int t = 0;
	t++;
	
	flash_coeff = (boss_flash > 0)&&((boss_flash/3) % 2) ? 255 : 0;
	if (boss_flash > 0) boss_flash--;

	switch (current_boss) {
		case 0: {
			static SDL_Surface *boss_spr = NULL;
			static int tail_x[10], tail_y[10];
			int i;
			
			SDL_Rect drawpos;
			if (boss_spr == NULL) {
				boss_spr = IMG_Load("dat/i/boss1.png");
				SDL_SetColorKey(boss_spr, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
			}
			
			if (boss_tail_len == 0) {
				tail_x[boss_tail_len] = boss_x;
				tail_y[boss_tail_len] = boss_y;
				boss_tail_len++;
			} else {
				if (dist(tail_x[0], tail_y[0], boss_x, boss_y) >= 24) {
					if (boss_tail_len < 10) boss_tail_len++;
					for (i = 8; i >= 0; i--) {
						tail_x[i+1] = tail_x[i];
						tail_y[i+1] = tail_y[i];
					}
					tail_x[0] = boss_x;
					tail_y[0] = boss_y;
				} else {
					for (i = 0; i < boss_tail_len; i++) {
						tail_x[i] += -1 + rand() % 3;
						tail_y[i] += -1 + rand() % 3;
					}
				}
			}
			
			for (i = 0; i < boss_tail_len; i++) {
				DrawCircleEx(tail_x[i] - scroll_x, tail_y[i] - scroll_y, 48 - i * 3, 0, 96 ^ flash_coeff);
			}
			for (i = 0; i < boss_tail_len; i++) {
				DrawCircleEx(tail_x[i] - scroll_x, tail_y[i] - scroll_y, 44 - i * 3, 0, 0 ^ flash_coeff);
			}
			
			drawpos.x = boss_x - 16 - scroll_x;
			drawpos.y = boss_y - 16 - scroll_y;
			
			SDL_BlitSurface(boss_spr, NULL, screen, &drawpos);
			
			break;
		}
		case 1: {
			static SDL_Surface *boss_spr = NULL;
			float h_dist;
			float new_dst, new_dir;
			int i, j;
			int hx[4], hy[4];
			int heads;
			int mx, my;
			int check_pass;
			
			heads = 5 - boss_lives;
			h_dist = M_PI * 2 / heads;

			SDL_Rect drawpos;
			if (boss_spr == NULL) {
				boss_spr = IMG_Load("dat/i/boss2.png");
				SDL_SetColorKey(boss_spr, SDL_SRCCOLORKEY | SDL_RLEACCEL, 255);
			}
			
			new_dst = 64 + sin((float)t / 80.0) * 24;
			
			new_dir = boss_2h_dir;
			new_dir += (float)(rand() % 16) / 128.0;
			new_dir -= (float)(rand() % 16) / 128.0;
			
			// Check new dists
			check_pass = 1;
			for (i = 0; i < heads; i++) {
				mx = boss_x + cos(new_dir + h_dist * i) * new_dst;
				my = boss_y + sin(new_dir + h_dist * i) * new_dst;
				
				if (!BossMovement(mx, my))
					check_pass = 0;
			}
			
			if (check_pass) {
				boss_2h_dst = new_dst;
				boss_2h_dir = new_dir;
			}
			
			// Heads
			
			for (i = 0; i < heads; i++) {
				hx[i] = boss_x + cos(boss_2h_dir + h_dist * i) * boss_2h_dst;
				hy[i] = boss_y + sin(boss_2h_dir + h_dist * i) * boss_2h_dst;
			}
			
			// Membranes
			for (j = 0; j < heads; j++) {
				for (i = 0; i < 5; i++) {
					mx = hx[j] + (hx[(j+1)%heads] - hx[j]) * i / 4;
					my = hy[j] + (hy[(j+1)%heads] - hy[j]) * i / 4;
					
					DrawCircleEx(mx - scroll_x, my - scroll_y, 24 + abs(i - 2)*12 + rand()%6, 0, 96 ^ flash_coeff);
				}
			}
			for (j = 0; j < heads; j++) {
				for (i = 0; i < 5; i++) {
					mx = hx[j] + (hx[(j+1)%heads] - hx[j]) * i / 4;
					my = hy[j] + (hy[(j+1)%heads] - hy[j]) * i / 4;
					
					DrawCircleEx(mx - scroll_x, my - scroll_y, 20 + abs(i - 2)*10 + rand()%6, 0, 0 ^ flash_coeff);
				}
			}
			
			// Draw heads
			
			for (i = 0; i < heads; i++) {	
				drawpos.x = hx[i] - 16 - scroll_x;
				drawpos.y = hy[i] - 16 - scroll_y;
				
				SDL_BlitSurface(boss_spr, NULL, screen, &drawpos);
			}
			
			break;
		}
		case 2: {
			static SDL_Surface *boss_spr = NULL;
			int i;
			int mx, my;
			float md;
			SDL_Rect drawfrom, drawpos;
						
			if (boss_spr == NULL) {
				boss_spr = IMG_Load("dat/i/boss3.png");
				SDL_SetColorKey(boss_spr, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
			}
			
			boss_m_heads = boss_lives;
			
			DrawCircleEx(boss_x - scroll_x, boss_y - scroll_y, 48, 0, 0 ^ flash_coeff);
			// Draw appendages
			for (i = 0; i < boss_m_heads; i++) {
				mx = boss_x;
				my = boss_y;
				md = M_PI * 2.0 / (float)boss_m_heads * i + (M_PI / 4);
						
				while (dist(mx, my, boss_m_hx[i], boss_m_hy[i]) > 12) {
					md = CHDir(md, PDir(mx, my, boss_m_hx[i], boss_m_hy[i]), 0.5);
					//md = PDir(mx, my, boss_m_hx[i], boss_m_hy[i]);
					mx += cos(md) * 12;
					my += sin(md) * 12;

					DrawCircleEx(mx - scroll_x, my - scroll_y, 16 + rand()%8, 0, 0 ^ flash_coeff);
				}
			}
			for (i = 0; i < boss_m_heads; i++) {
				DrawCircleEx(boss_m_hx[i] - scroll_x, boss_m_hy[i] - scroll_y, 28, 0, 0 ^ flash_coeff);
			}
			for (i = 0; i < boss_m_heads; i++) {
				mx = boss_x;
				my = boss_y;
				md = M_PI * 2.0 / (float)boss_m_heads * i + (M_PI / 4);
						
				while (dist(mx, my, boss_m_hx[i], boss_m_hy[i]) > 12) {
					md = CHDir(md, PDir(mx, my, boss_m_hx[i], boss_m_hy[i]), 0.5);
					md = PDir(mx, my, boss_m_hx[i], boss_m_hy[i]);
					mx += cos(md) * 12;
					my += sin(md) * 12;
					drawfrom.x = 32;
					drawfrom.y = 32 * (flash_coeff > 0);
					drawfrom.w = 32;
					drawfrom.h = 32;
					drawpos.x = mx - 16 - scroll_x;
					drawpos.y = my - 16 - scroll_y;
				
					SDL_BlitSurface(boss_spr, &drawfrom, screen, &drawpos);
				}
			}
			// Draw heads
			drawfrom.x = 0;
			drawfrom.y = 32 * (flash_coeff > 0);
			drawfrom.w = 32;
			drawfrom.h = 32;
			for (i = 0; i < boss_m_heads; i++) {
				drawpos.x = boss_m_hx[i] - 16 - scroll_x;
				drawpos.y = boss_m_hy[i] - 16 - scroll_y;
				SDL_BlitSurface(boss_spr, &drawfrom, screen, &drawpos);
			}
			
			// Draw core
			for (i = 0; i < boss_m_heads; i++) {
				drawpos.x = boss_x - 16 - scroll_x;
				drawpos.y = boss_y - 16 - scroll_y;
				
				SDL_BlitSurface(boss_spr, &drawfrom, screen, &drawpos);
			}
			break;
		}
		case 3: {
			SDL_Rect drawto;
			static SDL_Surface *boss_spr = NULL;
			
			if (boss_spr == NULL) {
				boss_spr = IMG_Load("dat/i/boss4.png");
				SDL_SetColorKey(boss_spr, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
			}
			
			// Aura
			
			DrawCircleEx(boss_x - scroll_x, boss_y - scroll_y, 64+rand()%4, 0, (192+rand()%64) ^ flash_coeff);
			DrawCircleEx(boss_x - scroll_x, boss_y - scroll_y, 48+rand()%4, 0, (rand()%64) ^ flash_coeff);
			
			drawto.x = boss_x - 16 - scroll_x;
			drawto.y = boss_y - 16 - scroll_y;
			
			SDL_BlitSurface(boss_spr, NULL, screen, &drawto);
			break;
		}
	}
}

void BC_BossIntro()
{
	static int boss_bar_fill = 0;
	static int boss_circle = 0, circle_reduce = 128;
	
	if (boss_dlg != 0) {
		DrawBoss();
		DrawBossHP(100);
		return;
	}
	
	if (boss_bar_fill < 100) {
		boss_bar_fill += 2;
		DrawBossHP(boss_bar_fill);
	} else {
		if (boss_circle < 128) {
			boss_circle += 2;
			DrawCircleEx(boss_x - scroll_x, boss_y - scroll_y, boss_circle, 0, 0);
			DrawCircle(boss_x - scroll_x, boss_y - scroll_y, boss_circle, 96);
		} else {
			if (circle_reduce > 4) {
				circle_reduce -= 2;
				DrawCircleEx(boss_x - scroll_x, boss_y - scroll_y, circle_reduce, 0, 0);
				DrawCircle(boss_x - scroll_x, boss_y - scroll_y, circle_reduce, 96);
				DrawBoss();
			} else {
				DrawBoss();
				if (final_boss_dlg == 0) {
					boss_dlg = 1;
					if (current_boss == 3) {
						final_boss_dlg = 1;
					}
				} else {
					boss_fight_mode = 2;
				}
				boss_circle = 0;
				boss_bar_fill = 0;
				circle_reduce = 128;
			}
		}
		DrawBossHP(100);
	}
}

void BC_BossCombat()
{
	static int t = 0;
	int move_x, move_y;
	float pdir;
	float pdir_x, pdir_y;
	int i, j;
	t++;
	
	pdir = PlayerDir(boss_x, boss_y);
	pdir_x = cos(pdir);
	pdir_y = sin(pdir);
	
	move_x = boss_x;
	move_y = boss_y;
	DrawBoss();
	switch (current_boss) {
		case 0: {
			static float boss_dir = 0.0;
			static float boss_dir_offset = 1.0;
			
			move_x += cos(boss_dir) * (4 + (3 - boss_lives)/2);
			move_y += sin(boss_dir) * (4 + (3 - boss_lives)/2);
			
			move_x += pdir_x * (2 + (3 - boss_lives)/2);
			move_y += pdir_y * (2 + (3 - boss_lives)/2);
			
			boss_dir += (float)(rand() % 64)/256 * boss_dir_offset;
			if ((rand()%48) == 0) {
				boss_dir_offset *= -1.0;
			}
			while (!BossMovement(move_x, move_y)) {
				boss_dir += (float)(rand() % 64)/256 * boss_dir_offset;
				if ((rand()%48) == 0) {
					boss_dir_offset *= -1.0;
				}
				move_x = boss_x;
				move_y = boss_y;
				
				move_x += cos(boss_dir) * (4 + (3 - boss_lives)/2);
				move_y += sin(boss_dir) * (4 + (3 - boss_lives)/2);
				move_x += pdir_x * (2 + (3 - boss_lives)/2);
				move_y += pdir_y * (2 + (3 - boss_lives)/2);
			}
			if (BossMovement(move_x, move_y)) {
				boss_x = move_x;
				boss_y = move_y;
			}
			if (boss_flash > 15) return;
			
			switch (boss_lives) {
				case 3: {
					int laser_dmg;
					float laser_1_dir, laser_2_dir;
					
					laser_dmg = (rand()%(player_shield/2+1))/2;
					
					for (i = 0; i < player_shield / 5 + 1; i++) {
						SpawnBullet(boss_x + (rand() % 48)*pdir_x, boss_y + (rand() % 48)*pdir_y, 3, pdir + M_PI / 4, 2, 0);
						SpawnBullet(boss_x + (rand() % 48)*pdir_x, boss_y + (rand() % 48)*pdir_y, 0, pdir, 4.5, 0);
					}
					laser_1_dir = pdir + 0.3 + sin((float)t / 8)*0.2;
					laser_2_dir = pdir - 0.3 + sin((float)t / 8)*0.2;
					SpawnLaser(boss_x + pdir_x * 24, boss_y + pdir_y * 8, laser_1_dir, 0, 1, 0, laser_dmg);
					SpawnLaser(boss_x + pdir_x * 24, boss_y + pdir_y * 8, laser_2_dir, 0, 1, 0, laser_dmg);
					break;
				}
				case 2: {
					int laser_dmg;
					float laser_1_dir, laser_2_dir, laser_3_dir;
					
					laser_dmg = (rand()%(player_shield/2+1))/2;
					for (i = 0; i < player_shield / 3 + 1; i++) {
						SpawnBullet(boss_x + (rand() % 48)*pdir_x, boss_y + (rand() % 48)*pdir_y, 3, RandomDir(), 3.5, 0);
						SpawnBullet(boss_x + (rand() % 48)*pdir_x, boss_y + (rand() % 48)*pdir_y, 0, RandomDir(), 4.5, 0);
					}
					laser_1_dir = pdir + 0.4 + sin((float)t / 8)*0.2;
					laser_2_dir = pdir - 0.4 + sin((float)t / 8)*0.2;
					laser_3_dir = RandomDir();
					SpawnLaser(boss_x + pdir_x * 24, boss_y + pdir_y * 8, laser_1_dir, 0, 1, 0, laser_dmg);
					SpawnLaser(boss_x + pdir_x * 24, boss_y + pdir_y * 8, laser_2_dir, 0, 1, 0, laser_dmg);
					if ((t % 12) == 0)
						SpawnLaser(boss_x + pdir_x * 24, boss_y + pdir_y * 8, laser_3_dir, 24, 8, 0, laser_dmg * 4);
					break;
				}
				case 1: {
					int laser_dmg;
					float laser_1_dir, laser_2_dir;
					
					laser_dmg = (rand()%(player_shield/2+1))/2;
					for (i = 0; i < player_shield / 4 + 1; i++) {
						SpawnBullet(boss_x + (rand() % 48)*pdir_x, boss_y + (rand() % 48)*pdir_y, 3, RandomDir(), 3.5, 0);
						SpawnBullet(boss_x + (rand() % 48)*pdir_x, boss_y + (rand() % 48)*pdir_y, 0, pdir, 5.0, 0);
						SpawnBullet(boss_x + (rand() % 48)*pdir_x, boss_y + (rand() % 48)*pdir_y, 0, RandomDir(), 4.5, 0);
					}
					laser_1_dir = pdir + 0.4 + sin((float)t / 8)*0.3;
					laser_2_dir = pdir - 0.4 + sin((float)t / 8)*0.3;
					SpawnLaser(boss_x + pdir_x * 24, boss_y + pdir_y * 8, laser_1_dir, 0, 1, 0, laser_dmg);
					SpawnLaser(boss_x + pdir_x * 24, boss_y + pdir_y * 8, laser_2_dir, 0, 1, 0, laser_dmg);
					if ((t % 5) == 0)
						SpawnLaser(boss_x + pdir_x * 24, boss_y + pdir_y * 8, RandomDir(), 16, 16, 0, laser_dmg * 4);
					if ((t % 10) == 0)
						SpawnBullet(boss_x + (rand() % 48)*pdir_x, boss_y + (rand() % 48)*pdir_y, 5, RandomDir(), 5, 0);
					break;
				}
			}
			break;
		}
		case 1: {
			float h_dist;
			int i, j;
			int hx[4], hy[4];
			int heads;
			int mx, my;
			int check_pass;
			float hd2;

			heads = 5 - boss_lives;
			h_dist = M_PI * 2 / heads;
			
			// Heads
			
			for (i = 0; i < heads; i++) {
				hx[i] = boss_x + cos(boss_2h_dir + h_dist * i) * boss_2h_dst;
				hy[i] = boss_y + sin(boss_2h_dir + h_dist * i) * boss_2h_dst;
			}
			
			move_x += -6 + rand() % 13;
			move_y += -6 + rand() % 13;
			
			move_x += pdir_x * (2 + (3 - boss_lives)/2);
			move_y += pdir_y * (2 + (3 - boss_lives)/2);
			
			// Check movement
			check_pass = 1;
			for (i = 0; i < heads; i++) {
				mx = move_x + cos(boss_2h_dir + h_dist * i) * boss_2h_dst;
				my = move_y + sin(boss_2h_dir + h_dist * i) * boss_2h_dst;
				
				if (!BossMovement(mx, my))
					check_pass = 0;
			}
			
			if (check_pass) {
				boss_x = move_x;
				boss_y = move_y;
			}
			
			if (boss_flash > 30) return;
			
			// Main cannons
			if ((t % (10 + boss_lives * 6)) == 1) {
				i = (t / (10 + boss_lives * 6)) % heads;
				for (j = 0; j < player_shield / 2 + 1; j++) {
					SpawnBullet(hx[i] + (rand() % 48)*pdir_x, hy[i] + (rand() % 48)*pdir_y, 0, pdir, 7 + ((float)(rand()%16))/10.0, 0);
				}
				SpawnBullet(hx[i], hy[i], 4, pdir, 10, 0);
			}
			
			// Barrage launcher
			for (i = 0; i < player_shield / 4 + heads; i++) {
				SpawnBullet(boss_x, boss_y, 0, RandomDir(), 3 + (float)(rand()%16)/10.0, 0);
			}
			
			if (boss_lives == 2) {
				// Splitters
				if ((t % 20) == 1) {
					SpawnBullet(boss_x, boss_y, 5, pdir, 5, 0);
				}
			}
			
			// Central laser cannon
			
			if ((t % (100 + boss_lives * 50)) == ((100 + boss_lives * 50)-1)) {
				i = player_shield / 3 + 2;
				hd2 = M_PI * 2.0 / (float)i;
				for (j = 0; j < i; j++) {
					SpawnLaser(boss_x + cos(hd2*j) * 20, boss_y + sin(hd2*j) * 20, pdir, 15 + rand()%16, 5, 0, (player_shield / 3 + 1));
				}
			}
			
			// Star vomit
			
			if (boss_lives == 2) {
				for (i = 0; i < heads; i++) {
					for (j = 0; j < player_shield / 6 + 1; j++) {
						SpawnBullet(hx[i], hy[i], 3, RandomDir(), 6, 0);
					}
				}
			}
			
			// Fusion cannon
			
			if (boss_lives <= 2) {
				if ((t % (12 + (boss_lives - 1) * 30)) == 9) {
					for (i = 0; i < heads; i++) {
						SpawnLaser(hx[i], hy[i], pdir, 6, 6, 0, (player_shield / 6 + 1));
					}
				}
			}
			break;
		}
		case 2: {
			int mx, my;
			static int t2 = 0;
			float md;
			int trying;

			boss_m_heads = boss_lives;
			
			for (i = 0; i < boss_m_heads; i++) {
				mx = boss_m_hx[i];
				my = boss_m_hy[i];
				
				boss_m_hd[i] = CHDir(boss_m_hd[i], PlayerDir(mx, my), 0.2);
				md = M_PI * 2.0 / (float)boss_m_heads * i + (M_PI / 4);
				boss_m_hd[i] = CHDir(boss_m_hd[i], md, 0.1);
				
				md = boss_m_hd[i];
				
				if ( (t % boss_m_heads) == i) {
					mx += cos(md) * 4 + (boss_m_heads == 1) * 2;
					my += sin(md) * 4 + (boss_m_heads == 1) * 2;
				}
				
		
				for (j = 0; j < boss_m_heads; j++) {
					if (j != i) {
						if (dist(boss_m_hx[j], boss_m_hy[j], boss_m_hx[i], boss_m_hy[i]) < 200) {
							md = PDir(boss_m_hx[j], boss_m_hy[j], boss_m_hx[i], boss_m_hy[i]);
							mx += cos(md) * 2;
							my += sin(md) * 2;
							boss_m_hd[i] = CHDir(boss_m_hd[i], md, 0.1);
						}
					}
				}
				
				trying = 10;
				
				while (!BossMovement(mx, my)) {
					trying--;
					if (trying == 0) break;
					md = PlayerDir(boss_m_hx[i], boss_m_hy[i]);
					md = md - 0.3 + (float)(rand()%16) * 0.6;
					
					mx = boss_m_hx[i] + cos(md) * 5;
					my = boss_m_hy[i] + sin(md) * 5;
					
					if (boss_m_heads == 1) {
						boss_m_hd[i] = CHDir(boss_m_hd[i], PlayerDir(mx, my), 0.2);
						md = boss_m_hd[i];
						mx += cos(md) * 3;
						my += sin(md) * 3;
					}
				}
				if (trying > 0) {
					boss_m_hx[i] = mx;
					boss_m_hy[i] = my;
				}
			}
			
			if (boss_flash > 30) return;
			
			if (boss_m_heads > 1) {
				if ((t % 2) == 0) {
					i = t % boss_m_heads;
					SpawnBullet(boss_m_hx[i] + cos(boss_m_hd[i])*8, boss_m_hy[i] + sin(boss_m_hd[i])*8, 0, boss_m_hd[i], 10, 0);
				} else {
					i = (t+2) % boss_m_heads;
					md = PlayerDir(boss_m_hx[i], boss_m_hy[i]);
					SpawnBullet(boss_m_hx[i] + cos(boss_m_hd[i])*8, boss_m_hy[i] + sin(boss_m_hd[i])*8, 0, md, 10, 0);
				}
			} else {
				md = CHDir(RandomDir(), PlayerDir(boss_m_hx[0], boss_m_hy[0]), 1.5);
				SpawnBullet(boss_m_hx[0] + cos(boss_m_hd[0])*8, boss_m_hy[0] + sin(boss_m_hd[0])*8, 0, md, 10, 0);
			}
			// Barrage
			
			for (i = 0; i < 24; i++) {
				if ((t2 % (6 - (player_shield / 6) + boss_m_heads)) == 0) {
					md = RandomDir();
					if (boss_m_heads > 2) {
						SpawnBullet(boss_x + cos(md)*8, boss_y + sin(md)*8, 0, md, 5, 0);
					} else {
						if (boss_m_heads > 1) {
							SpawnBullet(boss_x + cos(md)*24, boss_y + sin(md)*24, 4, md, 12, 0);
						} else {
							md = CHDir(md, pdir, 0.66);
							SpawnBullet(boss_x + cos(md)*24, boss_y + sin(md)*24, 4, md, 12, 0);
						}
					}
				}
				t2++;
			}
			
			// Beams
			if (boss_m_heads < 4) {
				if ((t % 5) == 0) {
					i = (t / 5)%boss_m_heads;
					md = boss_m_hd[i];
					SpawnLaser(boss_m_hx[i], boss_m_hy[i], md, 1, 10, 0.05, (player_shield / 7) + 3 - boss_m_heads);
					md += M_PI / 2;
					SpawnLaser(boss_m_hx[i], boss_m_hy[i], md, 1, 10, 0.05, (player_shield / 7) + 3 - boss_m_heads);
					md += M_PI / 2;
					SpawnLaser(boss_m_hx[i], boss_m_hy[i], md, 1, 10, 0.05, (player_shield / 7) + 3 - boss_m_heads);
					md += M_PI / 2;
					SpawnLaser(boss_m_hx[i], boss_m_hy[i], md, 1, 10, 0.05, (player_shield / 7) + 3 - boss_m_heads);
				}
			}
			
			if (boss_m_heads == 1) {
				if ( (t % 30) < player_shield) {
					md = CHDir(RandomDir(), pdir, 1.0);
					SpawnBullet(boss_m_hx[0] + cos(md)*24, boss_m_hy[0] + sin(md)*24, 3, md, 11, 0);
				}
			}
			break;
		}
		case 3:
		{
			int boss_loop;
			int boss_loop_total;
			int mx, my;
			int npattern;
			int i;
			float firedir;
			float flp;
			float cboss_dir;
			int ls_x, ls_y;
			
						
			mx = boss_x + cos(boss_dir)*(4 + (player_shield==30));
			my = boss_y + sin(boss_dir)*(4 + (player_shield==30));
			
			if (BossMovement(mx, my)) {
				boss_x = mx;
				boss_y = my;
				boss_dir = CHDir(boss_dir, pdir, 0.3);
			} else {
				boss_dir = CHDir(boss_dir, RandomDir(), 1);
			}
			
			if ((boss_lives == 1) && (player_shield == 30)) {
				{
					int room_w, room_h;
					int room_x, room_y;
					float tmr_t;
					float b_m_dir;
					
					float boss_x_bias, boss_y_bias;
					
					boss_x_bias = 1.33 - ((float)(boss_hp) / 1200.0);
					boss_y_bias = 0.7 + ((float)(boss_hp) / 1500.0);
					
					room_x = rooms[player_room].x * 32 + 64;
					room_y = rooms[player_room].y * 32 + 64;
					room_w = rooms[player_room].w * 32 - 128;
					room_h = rooms[player_room].h * 32 - 128;
					
					tmr_t = (float)t / 30.0;
					
					mx = (int)((sin(tmr_t * boss_x_bias)*0.5+0.5) * (float)room_w) + room_x;
					my = (int)((cos(tmr_t * boss_y_bias)*0.5+0.5) * (float)room_h) + room_y;
					
					if ( dist(mx, my, boss_x, boss_y) < 24) {
						boss_x = mx;
						boss_y = my;
					} else {
						b_m_dir = PDir(boss_x, boss_y, mx, my);
						boss_x += cos(b_m_dir)*24;
						boss_y += sin(b_m_dir)*24;
					}
				}
				if ((t % 50) < 49) {
					proxy_seek = 0;
					for (i = 0; i < 20; i++) {
						firedir = RandomDir();
						SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 7, firedir, 1, 0);
					}
				} else {
					proxy_seek = 1;
				}
				if ( ((t / 50) % 4) == 3) {
					proxy_seek = 1;
				}
				
				for (i = 0; i < 24; i++) {
					firedir = M_PI / 15 * (float)i + (float)t / 33.0 * M_PI;
					SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 0, firedir, 10, 0);
				}
			
				break;
			}
			
			boss_loop_total = 1 + (player_shield == 30)*2;
			
			cboss_dir = boss_dir;
			
			for (boss_loop = 0; boss_loop < boss_loop_total; boss_loop++) {
				if (player_shield == 30) {
					npattern = (3 - boss_lives) * 3 + 2 - (boss_hp / 334);
				} else {
					npattern = (2 - boss_lives) * 3 + 2 - (boss_hp / 334);
				}
				
				switch (npattern) {
					case 0:
						// Spirally pattern
						for (i = 0; i < 4; i++) {
							firedir = M_PI / 2 * (float)i + (float)t / 33.0 * M_PI;
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 0, firedir, 5, 0);
						}
						for (i = 0; i < 4; i++) {
							firedir = M_PI / 2 * (float)i + (float)t / 33.0 * M_PI * 2.0;
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 0, firedir, 8, 0);
						}
						for (i = 0; i < 4; i++) {
							firedir = RandomDir();
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 0, firedir, 12, 0);
						}
					break;
					case 1:
						// Proxies
						if ((t % 100) < 80) {
							for (i = 0; i < 6; i++) {
								firedir = RandomDir();
								SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 7, firedir, 1, 0);
							}
						} else {
							if ((t % 100) == 88) {
								proxy_seek = 1;
							}
							if ((t % 100) == 92) {
								proxy_seek = 0;
							}
						}
						for (i = 0; i < 8; i++) {
							firedir = M_PI / 4 * (float)i + (float)t / 33.0 * M_PI;
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 0, firedir, 10, 0);
						}
					break;
					case 2:
						// Laserwalls
						if ((t % 4) == 2) {
							firedir = cboss_dir + (float)t / 33.0 * M_PI;
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 10, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 5, 1);
						}
						
						for (i = 0; i < 4; i++) {
							firedir = cboss_dir + M_PI / 6 * (float)i + (float)t / 33.0 * M_PI;
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 0, firedir, 5, 0);
						}
					break;
					case 3:
						if ((t % 30) == 29) {
							firedir = RandomDir();
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 10, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 9.99, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 9.98, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 9.97, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 9.96, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 9.95, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 9.94, 1);
						}
						if ((t % 40) == 39) {
							firedir = RandomDir();
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 1, firedir, 6, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 5, firedir+1, 6, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 6, firedir+1, 6, 1);
						}
						if ((t % 50) == 49) {
							proxy_seek = 0;
							firedir = RandomDir();
							for (i = 0; i < 60; i++) {
								SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 7, firedir, 1, 1);
								firedir += M_PI * 2.0 * (float)firedir / 60.0;
							}
						}
						if ((t % 50) == 45) {
							proxy_seek = 1;
						}
						
						if ((t % 10) == 2) {
							firedir = cboss_dir + (float)t / 33.0 * M_PI;
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 10, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 7.5, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 5, 1);
						}
					break;
					case 4:
						if ((t % 80) == 79) {
							firedir = RandomDir();
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 10, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 9.98, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 9.96, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 9.94, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 9.92, 1);
						}
						if ((t % 90) == 89) {
							firedir = RandomDir();
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 1, firedir, 6, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 5, firedir+1, 6, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 6, firedir+1, 6, 1);
						}
						if ((t % 100) == 99) {
							proxy_seek = 0;
							firedir = RandomDir();
							for (i = 0; i < 60; i++) {
								SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 7, firedir, 1, 1);
								SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 7, firedir, 2, 1);
								SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 7, firedir, 3, 1);
								SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 7, firedir, 4, 1);
								firedir += M_PI * 2.0 * (float)firedir / 60.0;
							}
						}
						if ((t % 100) == 95) {
							proxy_seek = 1;
						}
						
						flp = (float)(t % 50) / 49.0;
						
						if ((t % 10) == 3) {
							i = (t / 10) % 4;
							switch (i) {
								case 0:
									ls_x = 7904 + flp * 576;
									ls_y = 8000;
									break;
								case 1:
									ls_x = 8480 - flp * 576;
									ls_y = 8416;
									break;
								case 2:
									ls_x = 7904;
									ls_y = 8000 + flp * 416;
									break;
								case 3:
								default:
									ls_x = 8480;
									ls_y = 8416 - flp * 416;
									break;
							}
							firedir = PlayerDir(ls_x, ls_y) - 0.1 + ((float)(rand()%16))/8.0;
							SpawnLaser(ls_x, ls_y, firedir, 8, 2, 0, player_shield/2+1);
						}
						
						if ((t % 6) == 2) {
							firedir = cboss_dir + (float)t / 33.0 * M_PI;
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 10, 1);
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 8, firedir, 5, 1);
						}
					break;
					case 5:
						flp = (float)(t % 50) / 49.0;
						
						if ((t % 8) == 7) {
							i = (t / 8) % 4;
							switch (i) {
								case 0:
									ls_x = 7904 + flp * 576;
									ls_y = 8000;
									break;
								case 1:
									ls_x = 8480 - flp * 576;
									ls_y = 8416;
									break;
								case 2:
									ls_x = 7904;
									ls_y = 8000 + flp * 416;
									break;
								case 3:
								default:
									ls_x = 8480;
									ls_y = 8416 - flp * 416;
									break;
							}
							firedir = PlayerDir(ls_x, ls_y) - 0.1 + ((float)(rand()%16))/8.0;
							SpawnLaser(ls_x, ls_y, firedir - 0.03*6, 5, 5, 0.03, player_shield/3+1);
						}
						
						for (i = 0; i < 7; i++) {
							firedir = cboss_dir + M_PI/2 * (float)i + (float)t / 33.0 * M_PI;
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 0, firedir, 8, 0);
						}
						for (i = 0; i < 7; i++) {
							firedir = cboss_dir + M_PI/4 + M_PI/2 * (float)i + (float)t / 33.0 * M_PI;
							SpawnBullet(boss_x + cos(firedir)*12, boss_y + sin(firedir)*12, 3, firedir, 8, 0);
						}
					break;
				}
				
				cboss_dir += -0.1 + ((float)(rand()%16))/8.0;
			}
		break;
		}
	}
}

void BC_BossDying()
{
	int x, y, rx, ry, rt;
	static int t_timer = 0;
	static int bxp, bdef;
	int i;
	static SDL_Surface *endpics[1] = {NULL};
	static float dr = 0;
	
	if (current_boss < 3) {
		specialmessage = 40 + rooms[player_room].room_param;
		specialmessagetimer = 120;
		rooms[player_room].room_type = 4;
		boss_fight_mode = 0;
		current_boss += 1;
		artifacts[8 + rooms[player_room].room_param] = 0;
		
		CullEnemies(4);
		
		// unlock doors
		
		for (y = 0; y < rooms[player_room].h; y++) {
			for (x = 0; x < rooms[player_room].w; x++) {
				rx = x + rooms[player_room].x;
				ry = y + rooms[player_room].y;
				rt = Get(rx, ry);
				
				if ((rt >= 21) && (rt <= 24)) {
					Put(rx, ry, rt - 21 + 13, player_room);
					
					if (rt == 21) {
						Put(rx, ry+1, 14, GetRoom(rx, ry+1));
					}
					if (rt == 22) {
						Put(rx, ry-1, 13, GetRoom(rx, ry-1));
					}
					if (rt == 23) {
						Put(rx+1, ry, 16, GetRoom(rx+1, ry));
					}
					if (rt == 24) {
						Put(rx-1, ry, 15, GetRoom(rx-1, ry));
					}
				}
			}
		}
		
		if (current_boss > 0) {
			SoupUpEnemies();
		}
	} else {
		if (boss_fight_mode == 3) {
			t_timer = 0;
			boss_fight_mode = 4;
			magic_circuit = 0;
			bxp = 4;
			bdef = 256;
			dr = RandomDir();
			
			if (training) t_timer = 2000;
		} else {
			DrawArtifactOverhead(3);
			if (bdef > 0) {
				DrawCircleEx(boss_x-scroll_x, boss_y-scroll_y, bxp, bxp-bdef, 0);
				bxp += 4;
				bdef -= 2;
			} else {
				if (t_timer < 220) {
					draw_text(SCREEN_W/2 - 76, 100, 0, "*** Divine Seal ***", 1);
					draw_text(SCREEN_W/2 - 76, SCREEN_H - 100, 0, "*** Divine Seal ***", 1);
					magic_circuit = circuit_size * 0.75 * t_timer / 220;
					for (i = 0; i < 10; i++) {
						rt = (rand() % 350) + 50;
						DrawCircle(player_x + PLAYERW/2 - scroll_x + cos(dr)*rt, player_y - scroll_y + PLAYERH/2
						+ sin(dr)*rt, rand()%30+5, rand()%128+128);
					}
					
					dr -= 0.025;
					for (i = 0; i < 5; i++) {
						Arc(screen, player_x + PLAYERW/2 - scroll_x, player_y + PLAYERH/2 - scroll_y, 450, dr);
						dr += 0.01;
					}
					dr += 0.03;
					
					if ((t_timer % 30) == 29) {
						dr = RandomDir();
					}
					
					DrawCircle(player_x - scroll_x + PLAYERW/2, player_y - scroll_y + PLAYERH/2, (t_timer % 30) * 15, 255);
					DrawCircle(player_x - scroll_x + PLAYERW/2, player_y - scroll_y + PLAYERH/2, ( (t_timer + 15) % 30) * 15, 255);
				} else {
					magic_circuit = 0;
					if (boss_fight_mode < 23) {
						if ( (t_timer % 4)==3) {
							boss_fight_mode++;
						}
					} else {
						if (endpics[0] == NULL) {
							if (training) {
								endpics[0] = IMG_Load("dat/i/wuss_ending.png");
							} 
						}
						
						if (training) {
							SDL_BlitSurface(endpics[0], NULL, screen, NULL);
						} else {
							show_ending = 1;
						}
					}
				}
				
				if (t_timer == 200) {
					CullEnemies(1);
				}
				
				t_timer++;
			}
		}
	}
}

void BC_NewLife()
{
	static int circle_size = 0;
	static int circle_size2 = 128;
	
	if (boss_flash > 0) {
		DrawBoss();
		return;
	}
	if (boss_new_life == 1) {
		circle_size = 0;
		circle_size2 = 128;
		boss_ox = rooms[current_boss_room].w * 16 + rooms[current_boss_room].x * 32;
		boss_oy = rooms[current_boss_room].h * 16 + rooms[current_boss_room].y * 32;
		boss_new_life = 2;
	} else {
		if (circle_size < 128) {
			circle_size += 4;
			DrawCircleEx(boss_x - scroll_x, boss_y - scroll_y, circle_size, 0, 0);
			DrawCircle(boss_x - scroll_x, boss_y - scroll_y, circle_size, 96);
			DrawBoss();
			
			if (boss_lives > 1) {
				DrawCircleEx(boss_ox - scroll_x, boss_oy - scroll_y, circle_size, 0, 0);
				DrawCircle(boss_ox - scroll_x, boss_oy - scroll_y, circle_size, 96);
			}
		} else {
			if (circle_size2 == 128) {
				boss_ox = boss_x;
				boss_oy = boss_y;
				boss_x = rooms[current_boss_room].w * 16 + rooms[current_boss_room].x * 32;
				boss_y = rooms[current_boss_room].h * 16 + rooms[current_boss_room].y * 32;
				boss_tail_len = 0;
				boss_lives--;
			}
			if (circle_size2 > 4) {
				circle_size2 -= 4;
				if (boss_lives > 0) {
					DrawCircleEx(boss_x - scroll_x, boss_y - scroll_y, circle_size2, 0, 0);
					DrawCircle(boss_x - scroll_x, boss_y - scroll_y, circle_size2, 96);
					DrawBoss();
				}
				
				DrawCircleEx(boss_ox - scroll_x, boss_oy - scroll_y, circle_size2, 0, 0);
				DrawCircle(boss_ox - scroll_x, boss_oy - scroll_y, circle_size2, 96);
			} else {
				if (boss_lives > 0) {
					boss_new_life = 0;
					boss_hp = 1000;
					if ( (boss_lives == 1) && (current_boss == 3) && (player_shield == 30) ) {
						player_hp = 6;
						boss_dmgrate = 0.2;
					}
				} else {
					boss_new_life = 0;
					boss_fight_mode = 3;
				}
			}
		}
	}
}

void BossControl()
{
	if ((player_room != current_boss_room)) {
		// Player left, so roll back the boss
		resetboss = 0;
		current_boss_room = 0;
		boss_tail_len = 0;
		boss_fight_mode = 0;
		boss_new_life = 0;
	}
	
	if (boss_fight_mode == 1) {
		BC_BossIntro();
		return;
	}
	if (boss_fight_mode == 2) {
		if (boss_new_life > 0) {
			BC_NewLife();
		} else {
			BC_BossCombat();
		}
		return;
	}
	if (boss_fight_mode >= 3) {
		BC_BossDying();
		return;
	}
}

void DrawArtifactOverhead(int p_obj)
{
	int p_x, p_y;
	static int tick = 0;

	SDL_Rect from, to;
	
	p_x = player_x - 8;
	p_y = player_y - 36 + sin((float)tick / 20.0)*4;

	
	from.x = (8 + p_obj) * 32;
	from.y = 0;
	from.w = 32;
	from.h = 32;
	
	to.x = p_x - scroll_x;
	to.y = p_y - scroll_y;
	SDL_BlitSurface(artifact_spr_large, &from, screen, &to);

	
	tick++;
}

int CanGetArtifact()
{
	int required_enemies;
	int n_artifacts;
	n_artifacts = current_boss + artifacts[8] + artifacts[9] + artifacts[10];
	required_enemies = total_enemies * (percent_required[n_artifacts]) / 100;

	if (killed_enemies >= required_enemies) return 1;
	return 0;
}

void BossRoom(int room)
{
	int i;
	
	boss_fight_mode = 1;
	current_boss_room = room;
	boss_x = rooms[room].w * 16 + rooms[room].x * 32;
	boss_y = rooms[room].h * 16 + rooms[room].y * 32;
	boss_hp = 1000;
	boss_flash = 0;
	magic_circuit = 0;
	boss_dlg = 0;
	boss_new_life = 0;
	
	switch (current_boss) {
		case 0:
			boss_lives = 3;
			boss_breakpoint = 75;
			boss_dmgrate = 1.00;
		break;
		case 1:
			boss_lives = 3;
			boss_breakpoint = 75;
			boss_dmgrate = 0.80;
		break;
		case 2:
			boss_lives = 4;
			boss_breakpoint = 90;
			boss_dmgrate = 1.25;
			boss_m_heads = 4;
			for (i = 0; i < boss_m_heads; i++) {
				boss_m_hd[i] = M_PI / 2 * i + (M_PI / 4);
				boss_m_hx[i] = boss_x + cos(boss_m_hd[i]) * 128;
				boss_m_hy[i] = boss_y + sin(boss_m_hd[i]) * 128;
			}
		break;
		case 3:
			boss_lives = 2 + (player_shield == 30);
			boss_breakpoint = 120;
			boss_dmgrate = 0.40 + 0.10*(player_shield == 30);
			boss_dir = M_PI * 3.0 / 2.0;
		break;
	}
	
	if (training) {
		boss_dmgrate *= 1.2;
		boss_breakpoint *= 0.8;
	}
}
