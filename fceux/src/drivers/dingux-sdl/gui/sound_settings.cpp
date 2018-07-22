
// Externals
extern Config *g_config;

/* MENU COMMANDS */

// Custom palette
static void sound_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.Sound", val);
}

// Sound rate
static void soundrate_update(unsigned long key) {
	const int rates[] = { 8000, 11025, 16000, 22050, 24000, 32000, 44100, 48000 };
	int i = 0, val;
	g_config->getOption("SDL.Sound.Rate", &val);

	for (i = 0; i < 8; i++)
		if (val == rates[i])
			break;

	if (key == DINGOO_RIGHT)
		i = i < 7 ? i + 1 : 0;
	if (key == DINGOO_LEFT)
		i = i > 0 ? i - 1 : 7;

	g_config->setOption("SDL.Sound.Rate", rates[i]);
}

// Sound quality
static void soundhq_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.Sound.Quality", &val);

	if (key == DINGOO_RIGHT)
		val = val < 2 ? val + 1 : 0;
	if (key == DINGOO_LEFT)
		val = val > 0 ? val - 1 : 2;

	g_config->setOption("SDL.Sound.Quality", val);
}

// Custom palette
static void lowpass_update(unsigned long key) {
	int val, tmp;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.Sound.LowPass", val);
}

// Sound volume
static void volume_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.Sound.Volume", &val);

	if (key == DINGOO_RIGHT)
		val = val < 256 ? val + 1 : 0;
	if (key == DINGOO_LEFT)
		val = val > 0 ? val - 1 : 256;

	g_config->setOption("SDL.Sound.Volume", val);
}

// Triangle volume
static void triangle_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.Sound.TriangleVolume", &val);

	if (key == DINGOO_RIGHT)
		val = val < 256 ? val + 1 : 0;
	if (key == DINGOO_LEFT)
		val = val > 0 ? val - 1 : 256;

	g_config->setOption("SDL.Sound.TriangleVolume", val);
}

// Square 1 volume
static void square1_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.Sound.Square1Volume", &val);

	if (key == DINGOO_RIGHT)
		val = val < 256 ? val + 1 : 0;
	if (key == DINGOO_LEFT)
		val = val > 0 ? val - 1 : 256;

	g_config->setOption("SDL.Sound.Square1Volume", val);
}

// Square 2 volume
static void square2_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.Sound.Square2Volume", &val);

	if (key == DINGOO_RIGHT)
		val = val < 256 ? val + 1 : 0;
	if (key == DINGOO_LEFT)
		val = val > 0 ? val - 1 : 256;

	g_config->setOption("SDL.Sound.Square2Volume", val);
}

// Noise volume
static void noise_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.Sound.NoiseVolume", &val);

	if (key == DINGOO_RIGHT)
		val = val < 256 ? val + 1 : 0;
	if (key == DINGOO_LEFT)
		val = val > 0 ? val - 1 : 256;

	g_config->setOption("SDL.Sound.NoiseVolume", val);
}

// PCM volume
static void pcm_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.Sound.PCMVolume", &val);

	if (key == DINGOO_RIGHT)
		val = val < 256 ? val + 1 : 0;
	if (key == DINGOO_LEFT)
		val = val > 0 ? val - 1 : 256;

	g_config->setOption("SDL.Sound.PCMVolume", val);
}

/* SOUND SETTINGS MENU */
static SettingEntry sd_menu[] = {
	{ "Toggle sound", "Enable sound", "SDL.Sound", sound_update },
	{ "Sound rate",	"Sound playback rate (Hz)", "SDL.Sound.Rate", soundrate_update },
	{ "Quality", "Sound quality", "SDL.Sound.Quality", soundhq_update},
	{ "Lowpass", "Enables low-pass filter",	"SDL.Sound.LowPass", lowpass_update },
	{ "Volume", "Sets global volume", "SDL.Sound.Volume", volume_update },
	{ "Triangle volume", "Sets Triangle volume", "SDL.Sound.TriangleVolume", triangle_update },
	{ "Square1 volume", "Sets Square 1 volume",	"SDL.Sound.Square1Volume", square1_update },
	{ "Square2 volume",	"Sets Square 2 volume", "SDL.Sound.Square2Volume", square2_update },
	{ "Noise volume", "Sets Noise volume", "SDL.Sound.NoiseVolume",	noise_update },
	{ "PCM volume", "Sets PCM volume", "SDL.Sound.PCMVolume", pcm_update }
};

int RunSoundSettings() {
	static int index = 0;
	static int spy = 72;
	int done = 0, y, i;

	int max_entries = 8;
	int menu_size = 10;

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

		if (parsekey(DINGOO_RIGHT, 1) || parsekey(DINGOO_LEFT, 1))
			sd_menu[index].update(g_key);

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

			DrawText(gui_screen, "Sound Settings", 8, 37);

			// Draw menu
			for (i = offset_start, y = 72; i < offset_end; i++, y += 15) {
				DrawText(gui_screen, sd_menu[i].name, 60, y);

				g_config->getOption(sd_menu[i].option, &itmp);

				if (!strncmp(sd_menu[i].name, "Toggle sound", 12) \
				|| !strncmp(sd_menu[i].name, "Lowpass", 7)) {
					sprintf(tmp, "%s", itmp ? "on" : "off");
				} else
					sprintf(tmp, "%d", itmp);
				DrawText(gui_screen, tmp, 210, y);
			}

			// Draw info
			DrawText(gui_screen, sd_menu[index].info, 8, 225);

			// Draw offset marks
			if (offset_start > 0)
				DrawChar(gui_screen, SP_UPARROW, 218, 57);
			if (offset_end < menu_size)
				DrawChar(gui_screen, SP_DOWNARROW, 218, 197);

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
