#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include "define.h"
#include "function.h"
#include "util_snd.h"
#include "extern.h"
#include "logo.h" 

#include "refresh.h"

void logo_main( void );
void logo_init( void );
void logo_relese( void );
void logo_keys( void );
void logo_drow( void );
void logo_init_save_data( void );

static int scene_exit;
static int demo;


void logo_main( void )
{
	int exit_code;
	
	exit_code = 0;
	
	logo_init( );	
	
	while( scene_exit )
	{
		logo_keys( );	
		logo_drow( );		
		
		RefreshScreen( g_screen );
		FPSWait( );	

		exit_code = system_keys( ); 
		if ( exit_code == 0 )
		{
			scene_exit = 0;
		}
	}
	
	logo_relese( );	
}

void logo_init( void )
{
	scene_exit = 1;
	demo = 0;
	
	LoadBitmap("image/a_logo.bmp",2,true);	

	soundStopBgm(EN_BGM_GAME01);
}

void logo_relese( void )
{
	int i;
	
	for ( i = 0; i < BMPBUFF_MAX; i++ )
	{
		ReleaseBitmap( i );
	}
	soundStopBgm(EN_BGM_GAME01);

}


void logo_keys( void )
{
	if ( ( IsPushKey(gameflag[4]) ) || ( IsPushKey(gameflag[5]) ) || ( demo > ( 60 * 3 ) ) )
	{
		gameflag[40] = 0;	
		g_scene = EN_SN_TITLE;
		scene_exit=0;
		return;
	}
}

void logo_drow( void )
{
	ClearSecondary();
	demo++;
	
	Blt( 2, 0, 0 );		/* 320 * 240 */
	KeyInput();				
}


