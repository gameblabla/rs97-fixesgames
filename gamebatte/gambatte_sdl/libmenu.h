/* libmenu.h
 * code for generating simple menus
 * public domain
 * by abhoriel
 */
 
#ifndef _LIBMENU_H
#define _LIBMENU_H

#ifdef __cplusplus
extern "C" {
#endif


#include <SDL/SDL.h>
#include "SFont.h"

typedef struct Menu_t menu_t;

typedef struct {
	char *text;
	char **entries;
	int is_shiftable;
	int n_entries;
	int selected_entry;
	void (*callback)(menu_t *);
} menu_entry_t;

struct Menu_t {
	char *title;
	menu_entry_t **entries;
	int n_entries;
	int selected_entry;
	int i;
	int quit;
	void (*back_callback)(menu_t *);
};

void libmenu_set_screen(SDL_Surface *set_screen);
void libmenu_set_font(SFont_Font *set_font);
int menu_main(menu_t *menu);
void set_active_menu(menu_t *menu);
menu_t *new_menu();
void delete_menu(menu_t *menu);
void menu_set_title(menu_t *menu, const char *title);
void menu_add_entry(menu_t *menu, menu_entry_t *entry);
menu_entry_t *new_menu_entry(int is_shiftable);
void delete_menu_entry(menu_entry_t *entry);
void menu_entry_set_text(menu_entry_t *entry, const char *text);
void menu_entry_add_entry(menu_entry_t *entry, const char* text);
void callback_menu_quit(menu_t *menu);


#ifdef __cplusplus
}
#endif


#endif
