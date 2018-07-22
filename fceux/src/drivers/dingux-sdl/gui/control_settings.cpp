// Externals
extern Config *g_config;

#define CONTROL_MENUSIZE 6

/* MENU COMMANDS */

int keyCheck(unsigned long key)
{
	if (key == DINGOO_B) return DefaultGamePad[0][1];
	if (key == DINGOO_A) return DefaultGamePad[0][0];
	if (key == DINGOO_Y) return DefaultGamePad[0][9];
	if (key == DINGOO_X) return DefaultGamePad[0][8];

	return -1;	
}

//!!! BUG LOCATED IN SDL LIBRARY GUI.CPP 
//!!! << configGamePadButton prefix creation SDL.Input.GamePad.%d missing "." at end >> 
//!!! FOLLOWING STRINGS MAY BE DEPRECATED IF FIXED IN FUTURE RELEASE
static void setA(unsigned long key)
{   	
	g_config->setOption("SDL.Input.GamePad.0A", keyCheck(key));
	UpdateInput(g_config);
}

static void setB(unsigned long key)
{   	
	g_config->setOption("SDL.Input.GamePad.0B", keyCheck(key));
	UpdateInput(g_config);
}

static void setTurboB(unsigned long key)
{   	
	g_config->setOption("SDL.Input.GamePad.0TurboB", keyCheck(key));
	UpdateInput(g_config);
}

static void setTurboA(unsigned long key)
{   	
	g_config->setOption("SDL.Input.GamePad.0TurboA", keyCheck(key));
	UpdateInput(g_config);
}

static void MergeControls(unsigned long key)
{
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.MergeControls", val);
}

static void resetMappings(unsigned long key)
{
	g_config->setOption("SDL.Input.GamePad.0A", DefaultGamePad[0][0]);
	g_config->setOption("SDL.Input.GamePad.0B", DefaultGamePad[0][1]);
	g_config->setOption("SDL.Input.GamePad.0TurboA", DefaultGamePad[0][8]);
	g_config->setOption("SDL.Input.GamePad.0TurboB", DefaultGamePad[0][9]);
	g_config->setOption("SDL.MergeControls", 0);
	UpdateInput(g_config);
}
/* CONTROL SETTING MENU */

static SettingEntry cm_menu[] = 
{
	{"Button B", "Map input for B", "SDL.Input.GamePad.0B", setB},
	{"Button A", "Map input for A", "SDL.Input.GamePad.0A", setA},
	{"Turbo B", "Map input for Turbo B", "SDL.Input.GamePad.0TurboB", setTurboB},
	{"Turbo A", "Map input for Turbo A", "SDL.Input.GamePad.0TurboA", setTurboA},
	{"Merge P1/P2", "Control both players at once", "SDL.MergeControls", MergeControls},
	{"Reset defaults", "Reset default control mappings", "", resetMappings},
};

int RunControlSettings()
{
	static int index = 0;
	static int spy = 72;
	int done = 0, y, i;
	int err = 1;
	int editMode = 0;

	g_dirty = 1;
	while (!done) {
		// Parse input
		readkey();
		if (parsekey(DINGOO_SELECT)) {
			if(index < 4) // Allow edit mode only for button mapping menu items
			{
				editMode = 1;
				DrawText(gui_screen, ">>", 185, spy);
				g_dirty = 0;
			}
		}

		if (!editMode) {
			if (parsekey(DINGOO_A)) {
				if (index > 3) {
					cm_menu[index].update(g_key);
				}
			}
			if (parsekey(DINGOO_B)) {
				//ERROR CHECKING
				int iBtn1 = -1;
				int iBtn2 = -1;
				err = 1;
				for ( int i = 0; i < 5; i++ ) {
					for ( int y = 0; y < 5; y++ ) {
						g_config->getOption(cm_menu[i].option, &iBtn1);
						if (i != y) {
							g_config->getOption(cm_menu[y].option, &iBtn2);
							if (iBtn1 == iBtn2) {
								err = 0;
								g_dirty = 1;
							}
							
						}
					}
				}

				done= err;
			}
		}	
		if ( !editMode ) {
	   		if (parsekey(DINGOO_UP, 1)) {
				if (index > 0) {
					index--; 
					spy -= 15;
				} else {
					index = CONTROL_MENUSIZE - 1;
					spy = 72 + 15*index;
				}
			}

			if (parsekey(DINGOO_DOWN, 1)) {
				if (index < CONTROL_MENUSIZE - 1) {
					index++;
					spy += 15;
				} else {
					index = 0;
					spy = 72;
				}
			}

	   		if (parsekey(DINGOO_LEFT, 1)) {
				if (index == 4) {
					cm_menu[index].update(g_key);
				}
			}

	   		if (parsekey(DINGOO_RIGHT, 1)) {
				if (index == 4) {
					cm_menu[index].update(g_key);
				}
			}
		}

		if ( editMode ) {
			if (parsekey(DINGOO_A, 0) || parsekey(DINGOO_B, 0) || parsekey(DINGOO_X, 0) || parsekey(DINGOO_Y, 0)) {
				cm_menu[index].update(g_key);
				g_dirty = 1;
				editMode = 0;
			}
		}
  
		// Draw stuff
		if( g_dirty ) 
		{
			draw_bg(g_bg);
			
			//Draw Top and Bottom Bars
			DrawChar(gui_screen, SP_SELECTOR, 0, 37);
			DrawChar(gui_screen, SP_SELECTOR, 81, 37);
			DrawChar(gui_screen, SP_SELECTOR, 0, 225);
			DrawChar(gui_screen, SP_SELECTOR, 81, 225);
			DrawText(gui_screen, "B - Go Back", 235, 225);
			DrawChar(gui_screen, SP_LOGO, 12, 9);
			
			// Draw selector
			DrawChar(gui_screen, SP_SELECTOR, 56, spy);
			DrawChar(gui_screen, SP_SELECTOR, 77, spy);
			if (err == 0) {
				DrawText(gui_screen, "!!!Error - Duplicate Key Mapping!!! ", 8, 37);
			} else {
				DrawText(gui_screen, "Control Settings - Press select to edit", 8, 37);
			}

			// Draw menu
			for(i=0,y=72;i < CONTROL_MENUSIZE;i++,y+=15) {
				int iBtnVal = -1;
				char cBtn[32];
				int mergeValue;

				DrawText(gui_screen, cm_menu[i].name, 60, y);
				
				g_config->getOption(cm_menu[i].option, &iBtnVal);
				
				if (i == CONTROL_MENUSIZE-1)
					sprintf(cBtn, "%s", "");
				else if (i == CONTROL_MENUSIZE-2)
				{
					int mergeValue;
					g_config->getOption("SDL.MergeControls", &mergeValue);
					sprintf(cBtn, "%s", mergeValue ? "on" : "off");
				}
				else if (iBtnVal == DefaultGamePad[0][0])
					sprintf(cBtn, "%s", "GCW_A");
				else if (iBtnVal == DefaultGamePad[0][1])
					sprintf(cBtn, "%s", "GCW_B");
				else if (iBtnVal == DefaultGamePad[0][8])
					sprintf(cBtn, "%s", "GCW_Y");
				else if (iBtnVal == DefaultGamePad[0][9])
					sprintf(cBtn, "%s", "GCW_X");
				else
					sprintf(cBtn, "%s", "<empty>");


				DrawText(gui_screen, cBtn, 210, y);
				
			}

			// Draw info
			DrawText(gui_screen, cm_menu[index].info, 8, 225);

			g_dirty = 0;
		}

		SDL_Delay(16);
		
		// Update real screen
		FCEUGUI_Flip();
	}	

	// Clear screen
	dingoo_clear_video();

	g_dirty = 1;
	return 0;
}
