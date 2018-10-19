# ifndef __SDL_JOY_H__
# define __SDL_JOY_H__

# ifdef __cplusplus
extern "C" {
# endif

#ifdef LINUX_MODE

//some keys of the keyboard to emulate sce

#define PSP_UPLEFT         79 //SDLK_KP7
#define PSP_UP             80 //SDLK_KP8
#define PSP_UPRIGHT        81 //SDLK_KP9

#define PSP_LEFT           83 //SDLK_KP4
#define PSP_RIGHT          85 //SDLK_KP6

#define PSP_DOWNLEFT       87 //SDLK_KP1
#define PSP_DOWN           88 //SDLK_KP2
#define PSP_DOWNRIGHT      89 //SDLK_KP3

#define PSP_TRIANGLE       25
#define PSP_SQUARE         38
#define PSP_CROSS          53
#define PSP_CIRCLE         40

#define PSP_L              46 //SDLK_l
#define PSP_R              27 //SDLK_r
#define PSP_FIRE           65 //SDLK_SPACE
#define PSP_START          36 //SDLK_RETURN
#define PSP_SELECT         39 //SDLK_s

#define PSP_JOY_UP         98
#define PSP_JOY_DOWN      104
#define PSP_JOY_LEFT      100
#define PSP_JOY_RIGHT     102

#endif

#define PSP_NOEVENT -1

#define DELAY_KEY_FIRST 250
#define DELAY_KEY_REPEAT 80
    

# ifdef __cplusplus
}
# endif

# endif
