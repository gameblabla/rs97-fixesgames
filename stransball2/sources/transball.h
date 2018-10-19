#ifndef __SUPER_TRANSBALL
#define __SUPER_TRANSBALL

#define MAX_ATRACTOR_P	64

int replay_parameters(char *file, int *ship, int *length, char *levelname);
void replay_copy(char *f1,char *f2);

bool load_configuration(void);
void save_configuration(void);


class SHIP_BULLET {
public:
	int x,y;
	int speed_x,speed_y;
	int state;
};


class TRANSBALL {
public:
	TRANSBALL(char *gpath,char *spath,char *mpath,int fuel,char *map,int ship_type);
	~TRANSBALL();

	int cycle(unsigned char *keyboard);
	void render(SDL_Surface *screen,int sx,int sy);

	int get_statistics(int num);

	void free_sounds();
	void load_sounds();

/*	SDLKey get_thrustkey(void) {return THRUST_KEY;}
	SDLKey get_antithrustkey(void) {return ANTITHRUST_KEY;}
	SDLKey get_leftkey(void) {return LEFT_KEY;}
	SDLKey get_rightkey(void) {return RIGHT_KEY;}
	SDLKey get_firekey(void) {return FIRE_KEY;}
	SDLKey get_atractorkey(void) {return ATRACTOR_KEY;}*/ 
	
private:
	
	char *g_filename(char *file);
	char *s_filename(char *file);
	char *m_filename(char *file);

	bool ship_map_collision(void);
	bool tile_map_collision(TILE *t,int bx,int by);

	char *m_gpath,*m_spath,*m_mpath;
	
	// SDLKey THRUST_KEY,ANTITHRUST_KEY,LEFT_KEY,RIGHT_KEY,FIRE_KEY,ATRACTOR_KEY;
	unsigned char old_keyboard[SDLK_LAST];

	SDL_Surface *tiles_sfc,*mask_sfc;
//	SDL_Surface *rotate_sfc,*rotate_sfc2;
	
	TILE **tiles,**tiles_mask;
	Mix_Chunk *S_start,*S_shipshot,*S_shot,*S_switch,*S_takeball,*S_explosion,*S_enemyhit,*S_thrust,*S_fuel;
	int thrust_channel;
	TRANSBALL_MAP *map;

	/* ship_state: 0 - normal, 1 - exploding	*/ 
	int ship_state,ship_anim;		
	int ship_x,ship_y,ship_angle;
	int ship_speed_x,ship_speed_y;
	int ship_atractor;
	int ship_type;
	int atractor_particles;
	int atractor_p_x[MAX_ATRACTOR_P],atractor_p_y[MAX_ATRACTOR_P];
	float atractor_p_speed[MAX_ATRACTOR_P];
	Uint32 atractor_p_color[MAX_ATRACTOR_P];

	/* ball_state: < 0 - inicial, 0 - atraida por la nave	*/ 
	int ball_state;
	int ball_x,ball_y;
	int ball_speed_x,ball_speed_y;

	int map_x,map_y;

	int fuel;
//	int ships;

	List<SHIP_BULLET> bullets;

	float fade_factor;
	int fade_state;

	bool start;

	int nstars;
	int *star_x,*star_y;
	int *star_color;

	/* statistics: */ 
	int fuel_used,n_shots,n_hits,enemies_destroyed;
};


#endif

