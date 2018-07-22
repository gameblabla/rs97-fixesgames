#ifndef _gui_h
#define _gui_h

int FCEUGUI_Init(FCEUGI *gi);
void FCEUGUI_Reset(FCEUGI *gi);
void FCEUGUI_Close();
void FCEUGUI_Run();

int InitFileBrowser();
void KillFileBrowser();
int RunFileBrowser(char *source, char *romname, const char *types[], const char *info = NULL);

#endif
