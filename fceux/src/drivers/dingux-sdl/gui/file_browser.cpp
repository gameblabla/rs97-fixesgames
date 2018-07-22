#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "file_list.h"

// Externals
extern SDL_Surface* screen;
extern Config *g_config;

static char s_LastDir[128] = "/";
int RunFileBrowser(char *source, char *outname, const char *types[],
		const char *info) {

	int size = 0;
	int index;
	int offset_start, offset_end;
	static int max_entries = 8;
	int scrollModifier = 4;
	int justsavedromdir = 0;
	int scrollMult;

	static int spy;
	int y, i;

	// Try to get a saved romdir from a config file
	char* home = getenv("HOME");
	char romcfgfile [128];
	sprintf (romcfgfile, "%s/.fceux/romdir.cfg", home);
	FILE * pFile;
	pFile = fopen (romcfgfile,"r+");
	if (pFile != NULL) {
		fgets (s_LastDir , 128 , pFile);
		fclose (pFile);
	}

	// Create file list
	FileList *list = new FileList(source ? source : s_LastDir, types);
	if (list == NULL)
		return 0;

	scrollModifier *= max_entries;

RESTART:
	size = list->Size();

	spy = 72;
	index = 0;
	offset_start = 0;
	offset_end = size > max_entries ? max_entries : size;

	g_dirty = 1;
	while (1) {
		// Parse input
		readkey();
		// TODO - put exit keys

		// Go to previous folder or return ...
		if (parsekey(DINGOO_B)) {
			list->Enter(-1);
			goto RESTART;
		}

		// Enter folder or select rom ...
		if (parsekey(DINGOO_A)) {
			if (list->GetSize(index) == -1) {
				list->Enter(index);
				goto RESTART;
			} else {
				strncpy(outname, list->GetPath(index), 128);
				break;
			}
		}

		if (parsekey(DINGOO_X)) {
			return 0;
		}

		if (parsekey(DINGOO_SELECT)) {
			// Save the current romdir in a config file
			char* home = getenv("HOME");
			char romcfgfile [128];
			strncpy(s_LastDir, list->GetCurDir(), 128);
			sprintf (romcfgfile, "%s/.fceux/romdir.cfg", home);
			FILE * pFile;
			pFile = fopen (romcfgfile,"w+");
			fputs (s_LastDir,pFile);
			fclose (pFile);
			justsavedromdir = 1;
		}

		if (size > 0) {
			// Move through file list

			if (parsekey(DINGOO_R, 0)) {
				int iSmartOffsetAdj = ((size <= max_entries) ? size : max_entries);
				index = size - 1;
				spy = 72 + 15*(iSmartOffsetAdj-1);
				offset_end = size;
				offset_start = offset_end - iSmartOffsetAdj;
			}

			if (parsekey(DINGOO_L, 0)) {
				goto RESTART;
			}

			if (parsekey(DINGOO_UP, 1)) {
				if (index > offset_start){
					index--;
					spy -= 15;
				} else if (offset_start > 0) {
					index--;
					offset_start--;
					offset_end--;
				} else {
					index = size - 1;
					offset_end = size;
					offset_start = size <= max_entries ? 0 : offset_end - max_entries;
					spy = 72 + 15*(index - offset_start);
				}
			}

			if (parsekey(DINGOO_DOWN, 1)) {
				if (index < offset_end - 1){
					index++;
					spy += 15;
				} else if (offset_end < size) {
					index++;
					offset_start++;
					offset_end++;
				} else {
					index = 0;
					offset_start = 0;
					offset_end = size <= max_entries ? size : max_entries;
					spy = 72;
				}
			}

			if (parsekey(DINGOO_LEFT, 1)) {
				if (index > offset_start) {
					index = offset_start;

					spy = 72;

				} else if (index - scrollModifier >= 0){
						index -= scrollModifier;
						offset_start -= scrollModifier;
						offset_end = offset_start + max_entries;
				} else
					goto RESTART;
			}

			if (parsekey(DINGOO_RIGHT, 1)) {
				if (index < offset_end-1) {
					index = offset_end-1;

					spy = 72 + 15*(index-offset_start);

				} else if (offset_end + scrollModifier <= size) {
						index += scrollModifier;
						offset_end += scrollModifier;
						offset_start += scrollModifier;
				} else {
					int iSmartOffsetAdj = ((size <= max_entries) ? size : max_entries);
					index = size - 1;
					spy = 72 + 15*(iSmartOffsetAdj-1);
					offset_end = size;
					offset_start = offset_end - iSmartOffsetAdj;
				}
			}
		}

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
			DrawChar(gui_screen, SP_SELECTOR, 4, spy);
			DrawChar(gui_screen, SP_SELECTOR, 81, spy);

			DrawText(gui_screen, "ROM Browser", 8, 37);

			// Draw file list
			for (i = offset_start, y = 72; i < offset_end; i++, y += 15) {
				DrawText(gui_screen, list->GetName(i), 8, y);
			}

			// Draw info
			if (info)
				DrawText(gui_screen, info, 8, 225);
			else {
				if (justsavedromdir == 1){
					DrawText(gui_screen, "ROM dir successfully saved!", 8, 225);
				} else {
					if (list->GetSize(index) == -1)
						DrawText(gui_screen, "SELECT - Save ROM dir", 8, 225);
					else
						DrawText(gui_screen, "SELECT - Save ROM dir", 8, 225);
				}
				justsavedromdir = 0;
			}

			// Draw offset marks
			if (offset_start > 0)
				DrawChar(gui_screen, SP_UPARROW, 157, 57);
			if (offset_end < list->Size())
				DrawChar(gui_screen, SP_DOWNARROW, 157, 197);

			g_dirty = 0;
		}

		SDL_Delay(4);

		// Update real screen
		FCEUGUI_Flip();

	}

	if (source == NULL)
		strncpy(s_LastDir, list->GetCurDir(), 128);
	delete list;

	return 1;
}
