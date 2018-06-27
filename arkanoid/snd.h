int boucle_son_gameover;
int boucle_son_levelstart;
int boucle_son_boss;

//La musique qui sera jouée
Mix_Music *musique;
Mix_Music *victory_musique;

//Les effets sonores que nous allons utiliser
Mix_Chunk *gameover_sound;
Mix_Chunk *intro_sound;
Mix_Chunk *levelstart_sound;
Mix_Chunk *menu_sound;
Mix_Chunk *tir_sound;
Mix_Chunk *ennemieexplosion_sound;
Mix_Chunk *tirboss_sound;
Mix_Chunk *bossexplosion_sound;
Mix_Chunk *selectionstage_sound;
Mix_Chunk *collisionbrique1_sound;
Mix_Chunk *collisionmur_sound;
Mix_Chunk *collisionraquette_sound;
Mix_Chunk *collisionboss_sound;
Mix_Chunk *intro_music_sound;

int load_sounds();
void jouer_son(Mix_Chunk *son);
void jouer_musique(Mix_Music *musique);
void clean_up();
