#include <stdio.h>
#include <stdlib.h>

#include <SDL/SDL.h>
#include "define.h"
#include "extern.h"
#include "scene.h"

#include "title.h"
#include "act.h"
#include "option.h"
#include "ending.h"
#include "logo.h"

void scenemanager( void );

void scenemanager( void )
{
	int exit;
	
	exit = true;
	while( exit )
	{
		switch( g_scene )
		{
		case EN_SN_TITLE:
			title_main( );
			break;
		case EN_SN_ACT:
			act_main( );
			break;
		case EN_SN_OPTION:
			option_main( );
			break;
		case EN_SN_ENDING:
			ending_main( );
			break;
		case EN_SN_LOGO:
			logo_main( );
			break;
		default:
			exit = false;
			break;
		}
	}

}


