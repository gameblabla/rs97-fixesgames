
enum {
	MENUTYPE_NORMAL	= 0
};

extern int help_key_sending;

#ifdef __cplusplus
extern "C" {
#endif

BOOL sysmenu_create(void);
void sysmenu_destroy(void);

BOOL sysmenu_menuopen(UINT menutype, int x, int y);

#define sdlkbd_resetf12()

#ifdef __cplusplus
}
#endif

