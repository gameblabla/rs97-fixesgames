#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include "define.h"
#include "function.h"
#include "util_snd.h"
#include "extern.h"
#include "option.h" 

#include "refresh.h"

void option_main( void );
void option_init( void );
void option_relese( void );
void option_keys( void );
void option_drow( void );
void option_init_save_data( void );
void option_kane_set( int x, int y );
void option_kane_disp( void );
void option_k_jmp( int i );

static int scene_exit;

static int mode;	
static int ag = 0;
static int selct[15];
static int bak_scl[64];
static int bak_count[64];
static int a[2] = {0,0};
static int b[2] = {0,0};
static int tolal_time[5];
static int tolal_time2[5];
static int tolal_time3[5];
static char string[1024];

void option_main( void )
{
	int exit_code;
	
	exit_code = 0;
	
	option_init( );		
	
	while( scene_exit )
	{
		option_keys( );		
		option_drow( );		
		
		RefreshScreen( g_screen );	
		FPSWait( );		

		exit_code = system_keys( ); 
		if ( exit_code == 0 )
		{
			scene_exit = 0;
		}
	}
	
	option_relese( );		
}

void option_init( void )
{
	int i;
	int wk;
	
	scene_exit = 1;

	mode = 0;	
	ag = 0;
	memset( selct, 0, sizeof( selct ) );
	memset( bak_scl, 0, sizeof( bak_scl ) );
	memset( bak_count, 0, sizeof( bak_count ) );
	memset( a, 0, sizeof( a ) );
	memset( b, 0, sizeof( b ) );
	memset( tolal_time, 0, sizeof( tolal_time ) );
	memset( tolal_time2, 0, sizeof( tolal_time2 ) );
	memset( tolal_time3, 0, sizeof( tolal_time3 ) );
	memset( string, 0, sizeof( string ) );

	LoadBitmap("image/title/title2.bmp",2,true);				
	LoadBitmap("image/title/scl.bmp",3,true);		
	LoadBitmap("image/title/title5.bmp",4,true);	
	LoadBitmap("image/title/potion_bak.bmp",5, false );	
	LoadBitmap("image/sys/fonts2.bmp",6,true);	

	if ( gameflag[126] == 1 )
	{
		LoadBitmap("image/title/option2.bmp",7,1);				
	}
	else 
	{
		LoadBitmap("image/title/option1.bmp",7,1);			
	}
	
	LoadBitmap("image/title/lvel.bmp",8,1);			
	LoadBitmap("image/title/on_off.bmp",9,1);		
	LoadBitmap("image/title/title_mask.bmp",11,1);
	LoadBitmap("image/title/none.bmp",12,1);		
	LoadBitmap("image/title/sima.bmp",13,1);		

	/*LoadBitmap("image/sys/waku.bmp",109,0);*/
	
	tolal_time[0] = 0;
	tolal_time[1] = 0;
	tolal_time[2] = 0;
	for ( i = 1; i <= 50; i++ )
	{
		wk = get2keta( gameflag[200 + i], 1 );
		tolal_time[0] = tolal_time[0] + wk;
		if ( tolal_time[0] >= 60 )
		{
			tolal_time[0] = tolal_time[0] - 60;
			tolal_time[1]++; 
		}
		wk = get2keta( gameflag[200 + i], 100 );
		tolal_time[1] = tolal_time[1] + wk;
		if ( tolal_time[1] >= 60 )
		{
			tolal_time[1] = tolal_time[1] - 60;
			tolal_time[2]++; 
		}
		
		wk = get2keta( gameflag[200 + i], 10000 );
		tolal_time[2] = tolal_time[2] + wk;
		if ( tolal_time[2] >= 999 )
		{
			tolal_time[2] = 999;
			tolal_time[1] = 59;
			tolal_time[0] = 59;
		}
		
		wk = get2keta( gameflag[250 + i], 1 );
		tolal_time2[0] = tolal_time2[0] + wk;
		if ( tolal_time2[0] >= 60 )
		{
			tolal_time2[0] = tolal_time2[0] - 60;
			tolal_time2[1]++; 
		}

		wk = get2keta( gameflag[250 + i], 100 );
		tolal_time2[1] = tolal_time2[1] + wk;
		if ( tolal_time2[1] >= 60 )
		{
			tolal_time2[1] = tolal_time2[1] - 60;
			tolal_time2[2]++; 
		}
		
		wk = get2keta( gameflag[250 + i], 10000 );
		tolal_time2[2] = tolal_time2[2] + wk;
		if ( tolal_time2[2] >= 999 )
		{
			tolal_time2[2] = 999;
			tolal_time2[1] = 59;
			tolal_time2[0] = 59;
		}
		
	}

	tolal_time[3] = 0;
	for ( i = 1; i <= 50; i++ )
	{
		tolal_time[3] = tolal_time[3] + gameflag[300 + i];
		tolal_time2[3] = tolal_time2[3] + gameflag[350 + i];
	}


	tolal_time3[0] = 0;
	tolal_time3[1] = 0;
	tolal_time3[2] = 0;
	tolal_time3[3] = 0;

	wk = get2keta( gameflag[137], 1 );
	tolal_time3[0] = wk;

	wk = get2keta( gameflag[137], 100 );
	tolal_time3[1] = wk;

	wk = get2keta( gameflag[137], 10000 );
	tolal_time3[2] = wk;

	tolal_time3[3] = gameflag[138];

	selct[2] = gameflag[133];

}

void option_relese( void )
{
	int i;
	
	for ( i = 0; i < BMPBUFF_MAX; i++ )
	{
		ReleaseBitmap( i );
	}
	soundStopBgm(EN_BGM_GAME01);

}


void option_keys( void )
{
	if ( ag == 0 )
	{
		if ( IsPushKey( gameflag[0] ) )
		{
			soundPlaySe( EN_SE_SELECT );
			mode--;
			if ( mode == 14 )
			{
				mode = 4;
			}
			if ( mode <= -1 )
			{
				mode = 15;
			}
		}
		else if ( IsPushKey( gameflag[1] ) )
		{
			soundPlaySe( EN_SE_SELECT );
			mode++;
			if ( mode == 5 )
			{
				mode = 15;
			}
			if ( mode >= 16 )
			{
				mode = 0;
			}
		}
	}
	if ( IsPushKey( gameflag[2] ) )
	{
		soundPlaySe( EN_SE_SELECT );
		selct[mode]--;
		if ( selct[mode] < 0 )
		{
			selct[mode] = 0;
		}
		if( ( mode == 3 ) && ( ag == 1 ) )
		{
			sprintf(string,"image/option/%d_f.bmp", selct[mode] );
			LoadBitmap( string, 10 , false );
		}
	}
	else if ( IsPushKey( gameflag[3] ) )
	{
		soundPlaySe( EN_SE_SELECT );
		selct[mode]++;
		if ( mode == 0 )	/* BGM */
		{
			if ( selct[mode] > 6 )
			{
				selct[mode] = 6;
			}
		}
		if ( mode == 1 )	/* SE */
		{
			if ( selct[mode] > 8 )
			{
				selct[mode] = 8;
			}
		}
		if ( mode == 2 )
		{
			if ( selct[mode] > 8 )
			{
				selct[mode] = 8;
			}
		}
		if ( ( mode == 3 ) && ( ag == 1 ) )
		{
			if ( selct[mode] > 99 )
			{
				selct[mode] = 99;
			}
			sprintf(string,"image/option/%d_f.bmp", selct[mode] );
			LoadBitmap( string, 10 , false );
		}
	}
	
	
	if ( IsPushOKKey( ) )
	{
		if ( mode == 15 )	/* Exit */
		{
			gameflag[133] = selct[2];	
			SaveGameFlag("config/config.fig");

			gameflag[40] = 1;					
			g_scene = EN_SN_TITLE;
			scene_exit=0;
		}
		else if ( mode == 4 )
		{
			if ( gameflag[100] == 1 )
			{
				gameflag[40] = 5;						
				g_scene = EN_SN_ENDING;
				scene_exit=0;
			}
			else 
			{
				soundPlaySe( EN_SE_MSG );
			}
		}
		else if ( mode == 3 )
		{
			ag = 1;
			sprintf(string,"image/option/%d_f.bmp", selct[mode] );
			LoadBitmap( string, 10 , false );
		}
		else if ( mode == 1 )	/* se test */
		{
			soundPlaySe( selct[mode] );
		}
		else if ( mode == 0 )	/* bgm test */
		{
			soundPlayBgm( selct[mode] );
		}
	}
	
	if ( IsPushCancelKey(  ) )
	{
		if ( mode == 3 )
		{
			ag = 0;
		}
		else if ( mode == 2 )
		{
			selct[mode] = 0;
		}
		else if ( mode == 1 )	/* se test */
		{
			soundStopSeAll();	
		}
		else if ( mode == 0 )	/* bgm test */
		{
			soundStopBgm(0);
		}
		
		if ( ag == 1 )
		{
			ag = 0;
		}
	}

}

void option_drow( void )
{
	int i;
	int x, y;

	ClearSecondary();

	a[0]++;
	if ( a[0] > 100 )
	{
		a[0] = 0;
	}
	for ( x = 0; x < 5; x++ )
	{
		for ( y = 0; y < 4; y++ )
		{
			Blt( 5, ( x * 100 )- a[0], ( y * 100 ) - a[0] );
		} 
	}

	Blt( 1, 10, 90 );
	
	if ( ag == 0 )
	{
		Blt( 7, 0, 0 );
		BltRect( 3, 50, 58 + ( mode * 8 ) , 32 + ( a[1] * 5 ) , 0, 5, 7 );
		
		for ( i = 0; i < 3 ; i++ )
		{
			BltNumericImage( selct[i], 2, 200, 56 + ( i * 8 ), 6, 0, 0, 10, 8 );
		}

		BltNumericImage2( gameflag[110], 3, 180, 64 + ( 5 * 8 ), 6, 0, 0, 10, 8 );
		BltNumericImage( gameflag[109], 2, 200 + 20 + 10, 64 + ( 5 * 8 ), 6, 0, 0, 10, 8 );
		BltNumericImage( gameflag[108], 2, 200 + 40 + 20, 64 + ( 5 * 8 ), 6, 0, 0, 10, 8 );

		BltNumericImage2( tolal_time[2], 3, 180, 64 + ( 6 * 8 ), 6, 0, 0, 10, 8 );	
		BltNumericImage( tolal_time[1], 2, 200 + 20 + 10, 64 + ( 6 * 8 ), 6, 0, 0, 10, 8 );
		BltNumericImage( tolal_time[0], 2, 200 + 40 + 20, 64 + ( 6 * 8 ), 6, 0, 0, 10, 8 );

		BltNumericImage2( tolal_time[3], 4, 170, 64 + ( 7 * 8 ), 6, 0, 0, 10, 8 );	

		BltNumericImage2( tolal_time3[2], 3, 180, 64 + ( 9 * 8 ), 6, 0, 0, 10, 8 );
		BltNumericImage( tolal_time3[1], 2, 200 + 20 + 10, 64 + ( 9 * 8 ), 6, 0, 0, 10, 8 );	
		BltNumericImage( tolal_time3[0], 2, 200 + 40 + 20, 64 + ( 9 * 8 ), 6, 0, 0, 10, 8 );	

		BltNumericImage2( tolal_time3[3], 4, 170, 64 + ( 10 * 8 ), 6, 0, 0, 10, 8 );	

		if ( gameflag[126] == 1 )
		{
			BltNumericImage2( tolal_time2[2], 3, 180, 64 + ( 12 * 8 ), 6, 0, 0, 10, 8 );	
			BltNumericImage( tolal_time2[1], 2, 200 + 20 + 10, 64 + ( 12 * 8 ), 6, 0, 0, 10, 8 );
			BltNumericImage( tolal_time2[0], 2, 200 + 40 + 20, 64 + ( 12 * 8 ), 6, 0, 0, 10, 8 );

			BltNumericImage2( tolal_time2[3], 4, 170, 64 + ( 13 * 8 ), 6, 0, 0, 10, 8 );
		}

	}
	else 
	{
		Blt( 10, 0, 0 );
		BltNumericImage( selct[mode], 2, 300, 230, 6, 0, 0, 10, 8 );	/* No. */

	}
	

	KeyInput();				
}


