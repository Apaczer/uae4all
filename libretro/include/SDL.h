#ifndef __SDL_H
#define __SDL_H

#include "libretro.h"
#include "SDL_thread.h"

typedef struct sdl_surface {
	int w;
	int h;
	int pitch;
	unsigned char *pixels;
}SDL_Surface ;

//SDL_Surface *prSDLScreen;

#if 0
typedef struct{
  unsigned char  scancode;
  unsigned short sym;
  unsigned short mod;
  unsigned short unicode;
} SDL_keysym;
#endif

#define SDL_Quit()
#define SDL_Init(...)
//#define SDL_LockSurface
//#define SDL_UnlockSurface
#define SDL_INIT_JOYSTICK
#define SDL_INIT_NOPARACHUTE
#define SDL_INIT_VIDEO

typedef unsigned short SDLKey;

#define SDLK_HOME     RETROK_HOME
#define SDLK_END      RETROK_END
#define SDLK_PAGEDOWN RETROK_PAGEDOWN
#define SDLK_PAGEUP   RETROK_PAGEUP
#define SDLK_RSHIFT   RETROK_RSHIFT
#define SDLK_RCTRL    RETROK_RCTRL
#define SDLK_UP       RETROK_UP
#define SDLK_DOWN     RETROK_DOWN
#define SDLK_RIGHT    RETROK_RIGHT
#define SDLK_LEFT     RETROK_LEFT
#define SDLK_ESCAPE   RETROK_ESCAPE

#define SDLK_LAST     320

#endif // __SDL_H