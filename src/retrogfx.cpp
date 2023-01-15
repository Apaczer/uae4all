 /*
  * UAE - The Un*x Amiga Emulator
  *
  * uae to libretro graphic interface
  *
  * Copyright 2021 Chips
  *
  */

#include "sysconfig.h"
#include "sysdeps.h"

#include <unistd.h>
#include <signal.h>

#include "uae.h"
#include "options.h"

#include "xwin.h"

#include "SDL.h"

SDL_Surface *prSDLScreen = NULL;

// Dummy menu part

int mainMenu_vpos=1;
int mainMenu_sound=-1;
int mainMenu_frameskip=0;
int mainMenu_throttle=0;
int mainMenu_autosave=-1;
int saveMenu_n_savestate=0;

void init_text(int splash)
{
}

void quit_text(void)
{
}

int run_mainMenu()
{
    return 0;
}

void _text_draw_window_bar(SDL_Surface *sf, int x, int y, int w, int h, int per, int max, char *title)
{
}

// Display

int uae4all_keystate[256];
int need_frameskip = 0;
unsigned int uae4all_numframes=0;
int prefs_gfx_framerate, changed_gfx_framerate;
char *gfx_mem=NULL;
unsigned gfx_rowbytes=0;

static int red_bits,  green_bits,  blue_bits;
static int red_shift, green_shift, blue_shift;

int libretro_frame_end=0;

void flush_screen (void)
{
    libretro_frame_end = 1;
}

int lockscr (void)
{
    return 1;
}

int needmousehack (void)
{
    return 1;
}


void handle_events (void)
{
}

#if 0
int graphics_init (void)
{
    if (!init_colors ())
        return 0;

    return(1);
}
#endif

static __inline__ int bitsInMask (unsigned long mask)
{
    /* count bits in mask */
    int n = 0;
    while (mask)
    {
        n += mask & 1;
        mask >>= 1;
    }
    return n;
}

static __inline__ int maskShift (unsigned long mask)
{
    /* determine how far mask is shifted */
    int n = 0;
    while (!(mask & 1))
    {
        n++;
        mask >>= 1;
    }
    return n;
}


static int init_colors (void)
{
    int i;

#ifdef DEBUG_GFX
    dbg("Function: init_colors");
#endif

    /* Truecolor: */
    //red_bits = bitsInMask(prSDLScreen->format->Rmask);
    //green_bits = bitsInMask(prSDLScreen->format->Gmask);
    //blue_bits = bitsInMask(prSDLScreen->format->Bmask);
    //red_shift = maskShift(prSDLScreen->format->Rmask);
    //green_shift = maskShift(prSDLScreen->format->Gmask);
    //blue_shift = maskShift(prSDLScreen->format->Bmask);
    red_bits = 5;
    green_bits = 6;
    blue_bits = 5;
    red_shift = 5 + 6;
    green_shift = 5;
    blue_shift = 0;
    alloc_colors64k (red_bits, green_bits, blue_bits, red_shift, green_shift, blue_shift);
    for (i = 0; i < 4096; i++)
        xcolors[i] = xcolors[i] * 0x00010001;

    return 1;
}

int graphics_setup (void)
{
    gfx_mem = (char *) calloc( 1, PREFS_GFX_WIDTH * PREFS_GFX_HEIGHT* 2 );
    gfx_rowbytes = PREFS_GFX_WIDTH *2;

    if (!init_colors ())
        return 0;

    return(1);
}

void graphics_leave (void)
{
}

int check_prefs_changed_gfx (void)
{
    extern int mainMenu_vpos;
    static int last_vpos=0;
    int ret=(last_vpos!=mainMenu_vpos);
    last_vpos=mainMenu_vpos;
    return ret;
}

void gui_purge_events(void)
{
}


