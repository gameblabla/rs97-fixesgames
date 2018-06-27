#include "includes.h"

int load_sounds()
{
    //Chargement de la musique
    victory_musique = Mix_LoadMUS( "snd/Victory_Music.wav" );

    //S'il y a eu une erreur au chargement de la musique
    if( victory_musique == NULL )
    {
       	printf("Erreur lors du chargement d'une musique");
    }

    //Chargement des effets sonores
    	gameover_sound = Mix_LoadWAV( "snd/Game_Over.wav" );
 	intro_sound = Mix_LoadWAV( "snd/Intro.wav" );
	levelstart_sound = Mix_LoadWAV( "snd/Level_Start.wav" );
	menu_sound = Mix_LoadWAV( "snd/Menu.wav" );
	tir_sound = Mix_LoadWAV( "snd/Tir.wav" );
	ennemieexplosion_sound = Mix_LoadWAV( "snd/Ennemie_Explosion.wav" );
	tirboss_sound = Mix_LoadWAV( "snd/Tir_Boss.wav" );
	bossexplosion_sound = Mix_LoadWAV( "snd/Boss_Explosion.wav" );
	selectionstage_sound = Mix_LoadWAV( "snd/Selection_Stage.wav" );
	collisionbrique1_sound = Mix_LoadWAV( "snd/Collision_Brique.wav" );
	collisionmur_sound = Mix_LoadWAV( "snd/Collision_Mur.wav" );
	collisionraquette_sound = Mix_LoadWAV( "snd/Collision_Raquette.wav" );
	collisionboss_sound = Mix_LoadWAV( "snd/Collision_Boss.wav" );
	intro_music_sound = Mix_LoadWAV( "snd/Intro_Music_Sound.wav" );
    //S'il y a eu un problème au chargement des effets sonore
    if (intro_sound == NULL || gameover_sound == NULL || levelstart_sound == NULL || menu_sound == NULL || tir_sound == NULL || ennemieexplosion_sound == NULL || tirboss_sound == NULL || bossexplosion_sound == NULL || selectionstage_sound == NULL || collisionbrique1_sound == NULL || collisionmur_sound == NULL || collisionraquette_sound == NULL || collisionboss_sound == NULL || intro_music_sound == NULL )
    {
       	printf("Erreur lors du chargement d'un son");
    }
	//Initialisation des variable de boucles
	boucle_son_gameover = 1;
	boucle_son_levelstart = 1;
	boucle_son_boss = 1;
    //Si tout s'est bien chargé
    return 1;
}

void jouer_son(Mix_Chunk *son)
{
	if( Mix_PlayChannel( -1, son, 0 ) == -1 )
	{
		printf("Erreur lors de la lecture d'un son");
	}
}

void jouer_musique(Mix_Music *musique)
{
	if( Mix_PlayMusic( musique, -1 ) == -1 )
	{
		printf("Erreur lors de la lecture d'une musique");
	}
}

void clean_up()
{
    //Libération des effets sonores
    Mix_FreeChunk( gameover_sound );
    Mix_FreeChunk( levelstart_sound );
    Mix_FreeChunk( tir_sound );
    Mix_FreeChunk( ennemieexplosion_sound );
    Mix_FreeChunk( tirboss_sound );
    Mix_FreeChunk( selectionstage_sound );
    Mix_FreeChunk( collisionbrique1_sound );
    Mix_FreeChunk( collisionmur_sound );
    Mix_FreeChunk( collisionraquette_sound );
    Mix_FreeChunk( collisionboss_sound );
    //Libération de la musique
    Mix_FreeMusic( musique );

    //On quitte SDL_mixer
    Mix_CloseAudio();
}

