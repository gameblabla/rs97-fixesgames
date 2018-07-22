#include "../config.h"

typedef struct _setting_entry {
	const char *name;
	const char *info;
	const std::string option;
	void (*update)(unsigned long);
} SettingEntry;

#include "main_settings.cpp"
#include "video_settings.cpp"
#include "sound_settings.cpp"
#include "control_settings.cpp"

#define SETTINGS_MENUSIZE 5

static int cmd_main_settings() {
	return RunMainSettings();
}

static int cmd_video_settings() {
	return RunVideoSettings();
}

static int cmd_sound_settings() {
	return RunSoundSettings();
}

static int cmd_control_settings() {
	return RunControlSettings();
}

static int cmd_config_save() {
	extern Config *g_config;
	g_config->save();
}

static MenuEntry
	settings_menu[] = {
		{ "Main Setup", "Change fceux main config", cmd_main_settings },
		{ "Video Setup", "Change video config", cmd_video_settings },
		{ "Sound Setup", "Change sound config", cmd_sound_settings },
		{ "Control Setup", "Change control config", cmd_control_settings },
		{ "Save config as default",	"Override default config", cmd_config_save } };

int RunSettingsMenu() {
	static int index = 0;
	static int spy = 72;
	int done = 0, y, i;

	g_dirty = 1;
	while (!done) {
		// Parse input
		readkey();
		if (parsekey(DINGOO_B))
			done = 1;

		if (parsekey(DINGOO_UP, 1)) {
			if (index > 0) {
				index--;
				spy -= 16;
			} else {
				index = SETTINGS_MENUSIZE - 1;
				spy = 72 + 16*index;
			}
		}

		if (parsekey(DINGOO_DOWN, 1)) {
			if (index < SETTINGS_MENUSIZE - 1) {
				index++;
				spy += 16;
			} else {
				index = 0;
				spy = 72;
			}
		}

		if (parsekey(DINGOO_A)) {
			done = settings_menu[index].command();
		}

		// Must draw bg only when needed
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

			DrawText(gui_screen, "Settings", 8, 37);

			// Draw menu
			for (i = 0, y = 72; i < SETTINGS_MENUSIZE; i++, y += 16) {
				DrawText(gui_screen, settings_menu[i].name, 60, y);
			}

			// Draw info
			DrawText(gui_screen, settings_menu[index].info, 8, 225);

			g_dirty = 0;
		}

		SDL_Delay(16);

		// Update real screen
		FCEUGUI_Flip();
	}

	// Must update emulation core and drivers
	UpdateEMUCore(g_config);
	FCEUD_DriverReset();

	// Clear screen
	dingoo_clear_video();

	g_dirty = 1;
}
