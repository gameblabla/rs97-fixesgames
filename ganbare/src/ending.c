#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include "define.h"
#include "function.h"
#include "util_snd.h"
#include "extern.h"
#include "ending.h" 

#include "refresh.h"

void ending_main( void );
void ending_init( void );
void ending_relese( void );
void ending_keys( void );
void ending_drow( void );
void ending_init_save_data( void );


static int scene_exit;

void ending_main( void )
{
	int exit_code;
	
	exit_code = 0;
	
	ending_init( );	
	
	while( scene_exit )
	{
		ending_keys( );	
		ending_drow( );
		
		RefreshScreen( g_screen );
		FPSWait( );	

		exit_code = system_keys( );
		if ( exit_code == 0 )
		{
			scene_exit = 0;
		}
	}
	
	ending_relese( );		// 終了
}

void ending_init( void )
{
	scene_exit = 1;

	LoadBitmap("image/bak/1.bmp",1,false);
	LoadBitmap("image/end/1.bmp",2,true);	
	
	if ( gameflag[126] == 0 )
	{ 
		LoadBitmap("image/end/2.bmp",3,true);
	}
	else 
	{
		LoadBitmap("image/end/3.bmp",3,true);
	}

	/*LoadBitmap("image/sys/waku.bmp",109,true);*/
	soundPlayBgm( EN_BGM_GAME02 );

}

void ending_relese( void )
{
	int i;
	
	for ( i = 0; i < BMPBUFF_MAX; i++ )
	{
		ReleaseBitmap( i );
	}
	soundStopBgm(EN_BGM_GAME01);

}


void ending_keys( void )
{
	if ( ( IsPushKey(gameflag[4]) ) || ( IsPushKey(gameflag[5]) ) )
	{
		gameflag[40] = 0;					/* タイトル */
		g_scene = EN_SN_TITLE;
		scene_exit=0;
		return;
	}

}

void ending_drow( void )
{
	Blt( 1 , 0, 0 );
	Blt( 3 , 0, 0 );
	Blt( 2 , 0, 0 );
	
	KeyInput();				
}


