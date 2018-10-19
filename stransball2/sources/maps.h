#ifndef TRANSBALL_MAPS
#define TRANSBALL_MAPS

class DOOR {
public:
	int x,y;
	int state;	/* state of the door: 0 closed - 14 open */ 
	int action; /* -1 : closing, 1: opening, 0: quiet	*/ 
	int event;  /* 0: activates when the ball is taken, !=0: activates when the right switch is pressed */ 

	void activate() 
	{
		if (action==0) {
			if (state==0) {
				action=1;
			} else {
				action=-1;
			} /* if */ 
		} else {
			action=-action;
		} /* if */ 
	} 
};


class SWITCH {
public:
	int x,y;
	int number;
	int state;
};


class FUELRECHARGE {
public:
	int x,y;
};

class SMOKE {
public:
	int x,y;
	int speed_x,speed_y;
	int desired_x,desired_y;
	int timer;
};

class SMOKESOURCE {
public:
	int x,y;
	int speed_x,speed_y;
	int timer;
};


class ENEMY {
public:
	bool collision(int strength) {life-=strength;if (life<=0) return true; else return false;}

	void draw_bullet(SDL_Surface *screen,TILE **tiles,int map_x,int map_y);
	void draw_directionalcanon(SDL_Surface *screen,TILE **tiles,int tile,int map_x,int map_y);
	void draw_directionalcanon2(SDL_Surface *screen,TILE **tiles,int tile,int map_x,int map_y);
	void draw_tank(SDL_Surface *screen,TILE **tiles,int map_x,int map_y);
	void draw_destroyedtank(SDL_Surface *screen,TILE **tiles,int map_x,int map_y);
	void draw_explosion(SDL_Surface *screen,TILE **tiles,int map_x,int map_y);

	bool cycle_bullet(int sx,int sy,bool collision);
	bool cycle_canon(int ship_x,int ship_y,List<ENEMY> *enemies,Mix_Chunk *S_shot);
	bool cycle_fastcanon(int ship_x,int ship_y,List<ENEMY> *enemies,Mix_Chunk *S_explosion);
	bool cycle_directionalcanon(int ship_x,int ship_y,List<ENEMY> *enemies,Mix_Chunk *S_shot);
	bool cycle_directionalcanon2(int ship_x,int ship_y,int ship_speedx,int ship_speedy,List<ENEMY> *enemies,Mix_Chunk *S_shot);
	bool cycle_tank(int ship_x,int ship_y,int ship_sx,int ship_sy,int gdist1,int gdist2,bool lcol,bool rcol,List<ENEMY> *enemies,Mix_Chunk *S_shot);
	bool cycle_explosion(void);

	/* type:			*/ 
	/* 0 : bullet		*/ 
	/* 1 : canon		*/ 
	/* 2 : fastcanon	*/ 
	/* 3 : directional canon	*/ 
	/* 4 : tank			*/ 
	/* 5 : destroyed tank	*/ 
	/* 6 : explosion	*/ 
	/* 7 : directional canon 2*/ 
	int type;

	int x,y;
	int life;
	int speed_x,speed_y;
	int direction;
	int state,state2;
	int tile;
	int tank_type;
	int tank_angle,turret_angle;


};


class TRANSBALL_MAP {
public:
	TRANSBALL_MAP(char *file);
	~TRANSBALL_MAP();

	void set_sounds(Mix_Chunk *S_shot,Mix_Chunk *S_explosion,Mix_Chunk *S_enemyhit,Mix_Chunk *S_switch);

	void cycle(int ship_x,int ship_y,int ship_speed_x,int ship_speed_y,TILE **masks);
	int  shipbullet_collision(int x,int y,int strength);
	void ball_collision(int x,int y);
	void ball_taken(void);
	void draw_map(SDL_Surface *surface,TILE **tiles,int x,int y,int ww,int wh);

	int get_sx(void) {return sx;};
	int get_sy(void) {return sy;};

	void get_ball_position(int *x,int *y);

	bool ship_in_fuel_recharge(int ship_x,int ship_y);

private:
	int animpiece(int piece);
	void draw_map_enemy(SDL_Surface *surface,TILE **tiles,int x,int y,int ww,int wh,ENEMY *e);
	void draw_map_noenemies(SDL_Surface *surface,TILE **tiles,int x,int y,int ww,int wh);

	int sx,sy;
	int animtimer,animflag;

	int *map;
	List<ENEMY> enemies;
	List<DOOR> doors;
	List<SWITCH> switches;
	List<FUELRECHARGE> fuel_recharges;
	List<SMOKE> smokes;
	List<SMOKESOURCE> smokesources;

	int background_type;

	Mix_Chunk *S_shot,*S_explosion,*S_enemyhit,*S_switch;
};

#endif

