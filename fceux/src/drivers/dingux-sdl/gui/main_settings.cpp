// Externals
extern Config *g_config;

/* MENU COMMANDS */

// Use PAL or NTSC rate
static void pal_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.PAL", val);
}

// TODO - Open game genie screen
static void gg_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.GameGenie", val);
}

static void sprite_limit_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.DisableSpriteLimit", val);
}

static void throttle_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.FPSThrottle", val);
}

static void showfps_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.ShowFPS", val);
}

static void show_mouse_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.ShowMouseCursor", val);
}

static void mouse_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.MouseSpeed", &val);

	if (key == DINGOO_RIGHT)
		val = val < 8 ? val + 1 : 8;
	if (key == DINGOO_LEFT)
		val = val > 0 ? val - 1 : 0;

	g_config->setOption("SDL.MouseSpeed", val);
}

// Frameskip
#if 0 //def FRAMESKIP
static void frameskip_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.Frameskip", &val);

	if (key == DINGOO_RIGHT) val = val < 9 ? val+1 : 9;
	if (key == DINGOO_LEFT) val = val > 0 ? val-1 : 0;

	g_config->setOption("SDL.Frameskip", val);
}
#endif

// Custom palette
static void custom_update(unsigned long key) {
	const char *types[] = { ".pal", NULL };
	char palname[128] = "";

	#ifdef WIN32
	if (!RunFileBrowser("d:\\", palname, types, "Choose nes palette (.pal)")) 
	#else
	if (!RunFileBrowser(NULL, palname, types, "Choose nes palette (.pal)")) 
	#endif
	{
		CloseGame();
		SDL_Quit();
		exit(-1);
	}

	std::string cpalette = std::string(palname);
	g_config->setOption("SDL.Palette", cpalette);
}

/* MAIN SETTINGS MENU */

static SettingEntry
	st_menu[] = {
		{ "PAL", "Use PAL timing", "SDL.PAL", pal_update },
		{ "Game Genie", "Emulate Game Genie", "SDL.GameGenie", gg_update },
		{ "No sprite limit", "Disable sprite limit", "SDL.DisableSpriteLimit", sprite_limit_update },
		{ "FPS Throttle", "Use fps throttling", "SDL.FPSThrottle", throttle_update },
		{ "Show FPS", "Show frames per second", "SDL.ShowFPS", showfps_update },
		{ "Show mouse", "Show/hide mouse cursor", "SDL.ShowMouseCursor", show_mouse_update },
		{ "Mouse speed", "Mouse cursor speed", "SDL.MouseSpeed", mouse_update },
#if 0 //def FRAMESKIP
		{ "Frameskip", "Frameskip [0-9]", "SDL.Frameskip", frameskip_update},
#endif
		{ "Custom palette", "Load custom palette", "SDL.Palette", custom_update },
};

int RunMainSettings() {
	static int index = 0;
	static int spy = 72;
	int done = 0, y, i;

	int max_entries = 8;
#if 0 //def FRAMESKIP
	int menu_size = 9;
#else
	int menu_size = 8;
#endif

	static int offset_start = 0;
	static int offset_end = menu_size > max_entries ? max_entries : menu_size;

	char tmp[32];
	int itmp;

	g_dirty = 1;
	while (!done) {
		// Parse input
		readkey();
		if (parsekey(DINGOO_B))
			done = 1;
		if (parsekey(DINGOO_UP, 1)) {
			if (index > 0) {
				index--;

				if (index >= offset_start)
					spy -= 15;

				if ((offset_start > 0) && (index < offset_start)) {
					offset_start--;
					offset_end--;
				}
			} else {
				index = menu_size-1;
				offset_end = menu_size;
				offset_start = menu_size <= max_entries ? 0 : offset_end - max_entries;
				spy = 72 + 15*(index - offset_start);
			}
		}

		if (parsekey(DINGOO_DOWN, 1)) {
			if (index < (menu_size - 1)) {
				index++;

				if (index < offset_end)
					spy += 15;

				if ((offset_end < menu_size) && (index >= offset_end)) {
					offset_end++;
					offset_start++;
				}
			} else {
				index = 0;
				offset_start = 0;
				offset_end = menu_size <= max_entries ? menu_size : max_entries;
				spy = 72;
			}
		}

		if (parsekey(DINGOO_LEFT, 1) || parsekey(DINGOO_RIGHT, 1) || parsekey(
				DINGOO_A))
			st_menu[index].update(g_key);

		// Draw stuff
		if (g_dirty) {
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

			DrawText(gui_screen, "Main Settings", 8, 37);

			// Draw menu
			for (i = offset_start, y = 72; i < offset_end; i++, y += 15) {
				DrawText(gui_screen, st_menu[i].name, 60, y);

				g_config->getOption(st_menu[i].option, &itmp);

				if (!strncmp(st_menu[i].name, "Custom palette", 6)) {
					std::string palname;
					g_config->getOption(st_menu[i].option, &palname);

					// Remove path of string
					const int sz = static_cast<int> (palname.size());
					const int path_sz = palname.rfind("/", palname.size());

					if (path_sz == sz)
						strncpy(tmp, palname.c_str(), 32);
					else
						strncpy(tmp, palname.substr(path_sz + 1, sz - 1
								- path_sz).c_str(), 32);
				} else if (!strncmp(st_menu[i].name, "Mouse speed", 11)) {
					sprintf(tmp, "%d", itmp);
				} else
					sprintf(tmp, "%s", itmp ? "on" : "off");
				DrawText(gui_screen, tmp, 210, y);
			}

			// Draw info
			DrawText(gui_screen, st_menu[index].info, 8, 225);

			// Draw offset marks
			if (offset_start > 0)
				DrawChar(gui_screen, SP_UPARROW, 274, 62);
			if (offset_end < menu_size)
				DrawChar(gui_screen, SP_DOWNARROW, 274, 203);

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
