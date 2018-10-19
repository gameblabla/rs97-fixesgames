#include <SDL/SDL.h>
extern "C" {
#include "get_resolution.h"
#include "define.h"
}

#ifdef SYLLABLE
#include <gui/desktop.h>
#endif

extern struct scaling screen_scale;

void Get_Resolution(void)
{
#ifdef SCALING
	#ifdef SYLLABLE
		os::Desktop cDesktop;
		os::IPoint point = cDesktop.GetResolution();
		screen_scale.w_display = point.x;
		screen_scale.h_display = point.y;
	#else
		const SDL_VideoInfo* info = SDL_GetVideoInfo();
		screen_scale.w_display = info->current_w;
		screen_scale.h_display = info->current_h; 
	#endif
	
	screen_scale.w_scale = screen_scale.w_display / DISPLY_WIDTH;
	screen_scale.h_scale = screen_scale.h_display / DISPLY_HEIGHT;
  
	screen_scale.w_scale_size = screen_scale.w_scale * DISPLY_WIDTH;
	screen_scale.h_scale_size = screen_scale.h_scale * DISPLY_HEIGHT;
	
	screen_position.x = (screen_scale.w_display - screen_scale.w_scale_size)/2;
	screen_position.y = (screen_scale.h_display - screen_scale.h_scale_size)/2;

	
#endif
}
