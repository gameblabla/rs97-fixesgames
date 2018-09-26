#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
#include <SDL/SDL_image.h>

#include <gambatte.h>
#include "src/blitterwrapper.h"

#include "libmenu.h"
#include "font12px.h"


static SDL_Surface *screen;
static SFont_Font* font;

int init_menu() {
    SDL_Surface *font_bitmap_surface = NULL;
    SDL_RWops *RWops;
    
	RWops = SDL_RWFromMem(font_12px, 12415);
    font_bitmap_surface = IMG_LoadPNG_RW(RWops);
    SDL_FreeRW(RWops);
    if (!font_bitmap_surface) {
        fprintf(stderr, "menu: font load error\n");
        exit(1);
    }
    font = SFont_InitFont(font_bitmap_surface);
    if (!font) {
        fprintf(stderr, "menu: font init error\n");
        exit(1);
    }

	libmenu_set_font(font);
	return 0;
}


void menu_set_screen(SDL_Surface *set_screen) {
	screen = set_screen;
	libmenu_set_screen(screen);
}

/* ============================ MAIN MENU =========================== */

static void callback_quit(menu_t *caller_menu);
static void callback_return(menu_t *caller_menu);
static void callback_savestate(menu_t *caller_menu);
static void callback_loadstate(menu_t *caller_menu);
static void callback_selectstate(menu_t *caller_menu);
static void callback_selectedstate(menu_t *caller_menu);
static void callback_options(menu_t *caller_menu);
static void callback_restart(menu_t *caller_menu);

static gambatte::GB *gambatte_p;
static BlitterWrapper *blitter_p;

void main_menu(gambatte::GB *gambatte, BlitterWrapper *blitter) {
    menu_t *menu;
	menu_entry_t *menu_entry;
    enum {RETURN = 0, SAVE_STATE = 1, LOAD_STATE = 2, SELECT_STATE = 3, OPTIONS = 4, RESTART = 5, QUIT = 6};
    
    menu = new_menu();
	menu_set_title(menu, "Main Menu");
	menu->back_callback = callback_menu_quit;
	
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Return to game");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_return;

	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Save state");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_savestate;
    
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Load state");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_loadstate;
	
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Select state");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_selectstate;
    
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Options");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_options;

	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Restart emulator");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_restart;
    
	menu_entry = new_menu_entry(0);
	menu_entry_set_text(menu_entry, "Quit");
	menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_quit;

	gambatte_p = gambatte;
	blitter_p = blitter;

	menu_main(menu);
    
    delete_menu(menu);	
}


static void callback_quit(menu_t *caller_menu) {
    caller_menu->quit = 1;
	SDL_Quit();
    exit(0);
}

static void callback_return(menu_t *caller_menu) {
    caller_menu->quit = 1;
}

static void callback_savestate(menu_t *caller_menu) {
    gambatte_p->saveState(blitter_p->inBuf().pixels, blitter_p->inBuf().pitch);
    caller_menu->quit = 1;
}

static void callback_loadstate(menu_t *caller_menu) {
	gambatte_p->loadState();
    caller_menu->quit = 1;
}

static void callback_restart(menu_t *caller_menu) {
    gambatte_p->reset();
    caller_menu->quit = 1;
}

/* ==================== SELECT STATE MENU =========================== */

static void callback_selectstate(menu_t *caller_menu) {
    #define N_STATES 10
    menu_t *menu;
	menu_entry_t *menu_entry;
    int i;
    char buffer[64];
    (void) caller_menu;
    menu = new_menu();

    menu_set_title(menu, "Select State");
	menu->back_callback = callback_menu_quit;
	
    for (i = 0; i < N_STATES; i++) {
        menu_entry = new_menu_entry(0);
        sprintf(buffer, "State %d", i);
        menu_entry_set_text(menu_entry, buffer);
        menu_add_entry(menu, menu_entry);
        menu_entry->callback = callback_selectedstate;
    }
    menu->selected_entry = gambatte_p->currentState();
    
	menu_main(menu);
    
    delete_menu(menu);
}

static void callback_selectedstate(menu_t *caller_menu) {
	gambatte_p->selectState(caller_menu->selected_entry);
	caller_menu->quit = 1;
}


/* ==================== OPTIONS MENU ================================ */
#define SHOW_FPS 0
#define SCALER 1
#define BACK 2

static void callback_options_back(menu_t *caller_menu);
static int is_showing_fps = 0;

static void callback_options(menu_t *caller_menu) {
    menu_t *menu;
	menu_entry_t *menu_entry;
    (void) caller_menu;
    menu = new_menu();
        
    menu_set_title(menu, "Options");
	menu->back_callback = callback_options_back;
	
    menu_entry = new_menu_entry(1);
    menu_entry_set_text(menu_entry, "Show FPS");
    menu_add_entry(menu, menu_entry);
    menu_entry_add_entry(menu_entry, "Off");
    menu_entry_add_entry(menu_entry, "On");    
    menu_entry->selected_entry = is_showing_fps;

    menu_entry = new_menu_entry(1);
    menu_entry_set_text(menu_entry, "Scaler");
    menu_add_entry(menu, menu_entry);
    menu_entry_add_entry(menu_entry, "Fullscreen");
    menu_entry_add_entry(menu_entry, "1.5x");
    menu_entry_add_entry(menu_entry, "None");
    menu_entry->selected_entry = blitter_p->getScaler();


    menu_entry = new_menu_entry(0);
    menu_entry_set_text(menu_entry, "Back");
    menu_add_entry(menu, menu_entry);
    menu_entry->callback = callback_options_back;
    
	menu_main(menu);
    
    delete_menu(menu);
}

static void callback_options_back(menu_t *caller_menu) {
	is_showing_fps = caller_menu->entries[SHOW_FPS]->selected_entry;
	blitter_p->setScaler(caller_menu->entries[SCALER]->selected_entry);
    caller_menu->quit = 1;
}

#undef SHOW_FPS
#undef RETURN

void show_fps(SDL_Surface *surface, int fps) {
	char buffer[64];
	sprintf(buffer, "%d", fps);
	if (is_showing_fps) {
		SFont_Write(surface, font, 0, 0, buffer);
	}
}
