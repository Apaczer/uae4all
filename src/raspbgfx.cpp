 /*
  * UAE - The Un*x Amiga Emulator
  *
  * SDL interface
  *
  * Copyright 2001 Bernd Lachner (EMail: dev@lachner-net.de)
  *  Raspberry Pi dispmanx by Chips
  * 
  */



#include "sysconfig.h"
#include "sysdeps.h"

#include <unistd.h>
#include <signal.h>

#include <SDL.h>
#include <SDL_endian.h>

#include "config.h"
#include "uae.h"
#include "options.h"
#include "memory.h"
#include "xwin.h"
#include "custom.h"
#include "drawing.h"
#include "keyboard.h"
#include "keybuf.h"
#include "gui.h"
#include "debug.h"
#include "savestate.h"
#include "menu/menu.h"
#include "vkbd/vkbd.h"

#include "debug_uae4all.h"

#include "thread.h"

#include "vkbd.h"
#include "bcm_host.h"
extern int drawfinished;

extern int mainMenu_screenformat;

int prefs_gfx_framerate, changed_gfx_framerate;

uae_sem_t vsync_wait_sem;

extern Uint32 proximo_frameskip;

char *gfx_mem=NULL;
unsigned gfx_rowbytes=0;

Uint32 uae4all_numframes=0;


static __inline__ RETSIGTYPE sigbrkhandler (int foo) {}


DISPMANX_DISPLAY_HANDLE_T   dispmanxdisplay;
DISPMANX_MODEINFO_T         dispmanxdinfo;
DISPMANX_RESOURCE_HANDLE_T  dispmanxresource_amigafb_1;
DISPMANX_RESOURCE_HANDLE_T  dispmanxresource_amigafb_2;
DISPMANX_ELEMENT_HANDLE_T   dispmanxelement;
DISPMANX_UPDATE_HANDLE_T    dispmanxupdate;
VC_RECT_T       src_rect;
VC_RECT_T       dst_rect;
VC_RECT_T       blit_rect;

DISPMANX_RESOURCE_HANDLE_T  dispmanxresource_menu;
DISPMANX_RESOURCE_HANDLE_T  dispmanxresource_menu2;
DISPMANX_ELEMENT_HANDLE_T   dispmanxelement_menu;


VC_RECT_T       src_rect_menu;
VC_RECT_T       dst_rect_menu;
VC_RECT_T       blit_rect_menu;

unsigned char current_resource_amigafb = 0;

static int red_bits, green_bits, blue_bits;
static int red_shift, green_shift, blue_shift;

static int current_width, current_height;
extern int emulated_mouse, emulated_mouse_button1, emulated_mouse_button2;

/* Keyboard and mouse */
int uae4all_keystate[256];


int vsync_frequency = 0;
int old_time = 0;
int need_frameskip = 0;

void vsync_callback(unsigned int a, void* b)
{
	proximo_frameskip=SDL_GetTicks();


	vsync_frequency = proximo_frameskip - old_time;
	old_time = proximo_frameskip;
	need_frameskip =  ( vsync_frequency > 31 ) ? (need_frameskip+1) : need_frameskip;
	//printf("d: %i", vsync_frequency     );

	uae_sem_post (&vsync_wait_sem);
}



#ifdef USE_RASTER_DRAW
void flush_block (int ystart, int ystop)
#else
void flush_screen (void)
#endif
{
    uae4all_prof_start(13);
#ifdef DEBUG_GFX
    dbgf("Function: flush_block %d %d\n", ystart, ystop);
#endif
	if (current_resource_amigafb == 1)
	{
		current_resource_amigafb = 0;
		vc_dispmanx_resource_write_data(  dispmanxresource_amigafb_1,
	                                    VC_IMAGE_RGB565,
	                                    current_width * 2,
	                                    gfx_mem,
	                                    &blit_rect );
		dispmanxupdate = vc_dispmanx_update_start( 10 );
		vc_dispmanx_element_change_source(dispmanxupdate,dispmanxelement,dispmanxresource_amigafb_1);

		vc_dispmanx_update_submit(dispmanxupdate,vsync_callback,NULL);

	}
	else
	{
		current_resource_amigafb = 1;
		vc_dispmanx_resource_write_data(  dispmanxresource_amigafb_2,
	                                    VC_IMAGE_RGB565,
	                                    current_width * 2,
	                                    gfx_mem,
	                                    &blit_rect );
		dispmanxupdate = vc_dispmanx_update_start( 10 );
		vc_dispmanx_element_change_source(dispmanxupdate,dispmanxelement,dispmanxresource_amigafb_2);

		vc_dispmanx_update_submit(dispmanxupdate,vsync_callback,NULL);
	}

       /*vc_dispmanx_resource_write_data(  dispmanxresource_amigafb_1,
                                            VC_IMAGE_RGB565,
                                            current_width * 2,
                                            gfx_mem,
                                            &blit_rect );*/




#ifndef RASPBERRY
	if (show_message)
	{
		show_message--;
		if (!show_message) {
			notice_screen_contents_lost();
		} else {
			_write_text_inv_n(prSDLScreen,0,29,30,show_message_str);
		}
	}
#endif
	if (emulated_mouse)
		vkbd_mouse();
	if (vkbd_mode)
		vkbd_key=vkbd_process();
    uae4all_prof_end(13);
}


void black_screen_now(void)
{
	// Avoid to draw next frame
	extern int framecnt   ,framecnt_hack ;
	framecnt = 1;
#ifndef USE_ALL_LINES
	framecnt_hack = 0;
#endif

	// Clear screen
	memset (gfx_mem, 0 , current_width * current_height * 2 );
	vc_dispmanx_resource_write_data(  dispmanxresource_amigafb_1,
	                                    VC_IMAGE_RGB565,
	                                    current_width * 2,
	                                    gfx_mem,
	                                    &blit_rect );
	vc_dispmanx_resource_write_data(  dispmanxresource_amigafb_2,
	                                    VC_IMAGE_RGB565,
	                                    current_width * 2,
	                                    gfx_mem,
	                                    &blit_rect );
	dispmanxupdate = vc_dispmanx_update_start( 10 );
	vc_dispmanx_element_change_source(dispmanxupdate,dispmanxelement,dispmanxresource_amigafb_1);
	vc_dispmanx_update_submit(dispmanxupdate,NULL,NULL);
	//vc_dispmanx_update_submit_sync(dispmanxupdate);
	need_frameskip =  1;
}

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
#ifdef DEBUG_GFX
    dbg("Function: graphics_setup");
#endif
    bcm_host_init();
    // FDA should above gfxmem used ?
    gfx_mem = (char *) calloc( 1, PREFS_GFX_WIDTH * PREFS_GFX_HEIGHT* 2 );
    return 1;
}


void graphics_subinit (void)
{
	VC_DISPMANX_ALPHA_T alpha = { (DISPMANX_FLAGS_ALPHA_T ) (DISPMANX_FLAGS_ALPHA_FROM_SOURCE | DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS), 
                             255, /*alpha 0->255*/
                             0 };

	uint32_t                    vc_image_ptr;
//#ifdef DEBUG_GFX
#if 1
	dbg("Function: graphics_subinit");
	dbgf("Emulation Resolution: %d x %d\n", current_width, current_height);
#endif

	//prSDLScreen = SDL_SetVideoMode(current_width, current_height, 16, uiSDLVidModFlags|VIDEO_FLAGS);

	if (dispmanxresource_amigafb_1 == 0)
	{
		dispmanxdisplay = vc_dispmanx_display_open( 0 );
		vc_dispmanx_display_get_info( dispmanxdisplay, &dispmanxdinfo);

		dispmanxresource_amigafb_1 = vc_dispmanx_resource_create( VC_IMAGE_RGB565,  current_width,   current_height,  &vc_image_ptr);
		dispmanxresource_amigafb_2 = vc_dispmanx_resource_create( VC_IMAGE_RGB565,  current_width,   current_height,  &vc_image_ptr);

		// FDA should above gfxmem used ?
		//gfx_mem = (char *) calloc( 1, current_width * current_height * 2 );


		vc_dispmanx_rect_set( &blit_rect, 0, 0, current_width,current_height);

		vc_dispmanx_resource_write_data(  dispmanxresource_amigafb_1,
	                                    VC_IMAGE_RGB565,
	                                    current_width *2,
	                                    gfx_mem,
	                                    &blit_rect );

		vc_dispmanx_rect_set( &src_rect, 0, 0, current_width << 16, current_height << 16 );



		//vc_dispmanx_rect_set( &dst_rect, (dispmanxdinfo.width /2),
	        //                     (dispmanxdinfo.height * 3)/100 ,
	        //                     (dispmanxdinfo.width - (dispmanxdinfo.width * 6)/100 )/2,
	        //                     (dispmanxdinfo.height - (dispmanxdinfo.height * 7)/100 )/2);

	}


	if (mainMenu_screenformat == 0)
	{
		printf("Fullscreen\n");
		vc_dispmanx_rect_set( &dst_rect, (dispmanxdinfo.width * 3)/100,
								(dispmanxdinfo.height * 3)/100 ,
								dispmanxdinfo.width - (dispmanxdinfo.width * 6)/100 ,
								dispmanxdinfo.height - (dispmanxdinfo.height * 7)/100 );
	}
	else
	{
		vc_dispmanx_rect_set( &dst_rect, ((dispmanxdinfo.width * 17)/100) ,
								(dispmanxdinfo.height * 3)/100 ,
								(dispmanxdinfo.width - ((dispmanxdinfo.width * 36)/100)) ,
								dispmanxdinfo.height - (dispmanxdinfo.height * 7)/100 );
	}



	dispmanxupdate = vc_dispmanx_update_start( 10 );
	dispmanxelement = vc_dispmanx_element_add( dispmanxupdate,
	                                        dispmanxdisplay,
	                                        2000,               // layer
	                                        &dst_rect,
	                                        dispmanxresource_amigafb_1,
	                                        &src_rect,
	                                        DISPMANX_PROTECTION_NONE,
	                                        &alpha,
	                                        NULL,             // clamp
	                                        DISPMANX_NO_ROTATE );

	vc_dispmanx_update_submit_sync(dispmanxupdate);
	//dispmanxupdate = vc_dispmanx_update_start( 10 );
	  
//#ifdef DEBUG_GFX
#if 1
	dbgf("Res dispmanX: %d %d\n", dispmanxdinfo.width ,dispmanxdinfo.height );
#endif

	/* Initialize structure for Amiga video modes */
	//gfx_mem = (char *)prSDLScreen->pixels;
	//gfx_rowbytes = prSDLScreen->pitch;
	gfx_rowbytes = current_width *2;

#ifdef DEBUG_GFX
	dbgf("current_height=%i\n",current_height);
#endif
	lastmx = lastmy = 0;
	newmousecounters = 0;
}


int graphics_init (void)
{
	int i,j;

#ifdef DEBUG_GFX
	dbg("Function: graphics_init");
#endif

	current_width = PREFS_GFX_WIDTH;
	current_height = PREFS_GFX_HEIGHT;

	uae_sem_init (&vsync_wait_sem, 0, 1);

	graphics_subinit ();


	if (!init_colors ())
		return 0;

	buttonstate[0] = buttonstate[1] = buttonstate[2] = 0;
	for (i = 0; i < 256; i++)
		uae4all_keystate[i] = 0;

	return 1;
}


void graphics_dispmanshutdown (void)
{
    dispmanxupdate = vc_dispmanx_update_start( 10 );
    vc_dispmanx_element_remove( dispmanxupdate, dispmanxelement);
    vc_dispmanx_update_submit_sync(dispmanxupdate);
}


static void graphics_subshutdown (void)
{
#ifdef DEBUG_GFX
    dbg("Function: graphics_subshutdown");
#endif
    graphics_dispmanshutdown();
    vc_dispmanx_resource_delete( dispmanxresource_amigafb_1 );
	vc_dispmanx_resource_delete( dispmanxresource_amigafb_2 );
    vc_dispmanx_display_close( dispmanxdisplay );
    SDL_FreeSurface(prSDLScreen);
    #if DEBUG_GFX
    dbgf("Killed %d %d %d %d\n", dispmanxupdate , dispmanxelement, dispmanxresource_amigafb_1 , dispmanxdisplay);
    #endif
}

void graphics_leave (void)
{
#ifdef DEBUG_GFX
    dbg("Function: graphics_leave");
#endif

    graphics_subshutdown ();

	SDL_VideoQuit();

    dumpcustom ();
}

/* Decode KeySyms. This function knows about all keys that are common
 * between different keyboard languages. */
static int kc_decode (SDL_keysym *prKeySym)
{
    switch (prKeySym->sym)
    {
    case SDLK_b: return AK_B;
    case SDLK_c: return AK_C;
    case SDLK_d: return AK_D;
    case SDLK_e: return AK_E;
    case SDLK_f: return AK_F;
    case SDLK_g: return AK_G;
    case SDLK_h: return AK_H;
    case SDLK_i: return AK_I;
    case SDLK_j: return AK_J;
    case SDLK_k: return AK_K;
    case SDLK_l: return AK_L;
    case SDLK_n: return AK_N;
    case SDLK_o: return AK_O;
    case SDLK_p: return AK_P;
    case SDLK_r: return AK_R;
    case SDLK_s: return AK_S;
    case SDLK_t: return AK_T;
    case SDLK_u: return AK_U;
    case SDLK_v: return AK_V;
    case SDLK_x: return AK_X;

    case SDLK_0: return AK_0;
    case SDLK_1: return AK_1;
    case SDLK_2: return AK_2;
    case SDLK_3: return AK_3;
    case SDLK_4: return AK_4;
    case SDLK_5: return AK_5;
    case SDLK_6: return AK_6;
    case SDLK_7: return AK_7;
    case SDLK_8: return AK_8;
    case SDLK_9: return AK_9;

    case SDLK_KP0: return AK_NP0;
    case SDLK_KP1: return AK_NP1;
    case SDLK_KP2: return AK_NP2;
    case SDLK_KP3: return AK_NP3;
    case SDLK_KP4: return AK_NP4;
    case SDLK_KP5: return AK_NP5;
    case SDLK_KP6: return AK_NP6;
    case SDLK_KP7: return AK_NP7;
    case SDLK_KP8: return AK_NP8;
    case SDLK_KP9: return AK_NP9;
    case SDLK_KP_DIVIDE: return AK_NPDIV;
    case SDLK_KP_MULTIPLY: return AK_NPMUL;
    case SDLK_KP_MINUS: return AK_NPSUB;
    case SDLK_KP_PLUS: return AK_NPADD;
    case SDLK_KP_PERIOD: return AK_NPDEL;
    case SDLK_KP_ENTER: return AK_ENT;

    case SDLK_F1: return AK_F1;
    case SDLK_F2: return AK_F2;
    case SDLK_F3: return AK_F3;
    case SDLK_F4: return AK_F4;
    case SDLK_F5: return AK_F5;
    case SDLK_F6: return AK_F6;
    case SDLK_F7: return AK_F7;
    case SDLK_F8: return AK_F8;
    case SDLK_F9: return AK_F9;
    case SDLK_F10: return AK_F10;

    case SDLK_BACKSPACE: return AK_BS;
    case SDLK_DELETE: return AK_DEL;
    case SDLK_LCTRL: return AK_CTRL;
    case SDLK_RCTRL: return AK_RCTRL;
    case SDLK_TAB: return AK_TAB;
    case SDLK_LALT: return AK_LALT;
    case SDLK_RALT: return AK_RALT;
    case SDLK_RMETA: return AK_RAMI;
    case SDLK_LMETA: return AK_LAMI;
    case SDLK_RETURN: return AK_RET;
    case SDLK_SPACE: return AK_SPC;
    case SDLK_LSHIFT: return AK_LSH;
    case SDLK_RSHIFT: return AK_RSH;
    case SDLK_ESCAPE: return AK_ESC;

    case SDLK_INSERT: return AK_HELP;
    case SDLK_HOME: return AK_NPLPAREN;
    case SDLK_END: return AK_NPRPAREN;
    case SDLK_CAPSLOCK: return AK_CAPSLOCK;

    case SDLK_UP: return AK_UP;
    case SDLK_DOWN: return AK_DN;
    case SDLK_LEFT: return AK_LF;
    case SDLK_RIGHT: return AK_RT;

    case SDLK_PAGEUP: return AK_RAMI;          /* PgUp mapped to right amiga */
    case SDLK_PAGEDOWN: return AK_LAMI;        /* PgDn mapped to left amiga */

    default: return -1;
    }
}

static int decode_us (SDL_keysym *prKeySym)
{
    switch(prKeySym->sym)
    {
	/* US specific */
    case SDLK_a: return AK_A;
    case SDLK_m: return AK_M;
    case SDLK_q: return AK_Q;
    case SDLK_y: return AK_Y;
    case SDLK_w: return AK_W;
    case SDLK_z: return AK_Z;
    case SDLK_LEFTBRACKET: return AK_LBRACKET;
    case SDLK_RIGHTBRACKET: return AK_RBRACKET;
    case SDLK_COMMA: return AK_COMMA;
    case SDLK_PERIOD: return AK_PERIOD;
    case SDLK_SLASH: return AK_SLASH;
    case SDLK_SEMICOLON: return AK_SEMICOLON;
    case SDLK_MINUS: return AK_MINUS;
    case SDLK_EQUALS: return AK_EQUAL;
	/* this doesn't work: */
    case SDLK_BACKQUOTE: return AK_QUOTE;
    case SDLK_QUOTE: return AK_BACKQUOTE;
    case SDLK_BACKSLASH: return AK_BACKSLASH;
    default: return -1;
    }
}

int keycode2amiga(SDL_keysym *prKeySym)
{
    int iAmigaKeycode = kc_decode(prKeySym);
    if (iAmigaKeycode == -1)
            return decode_us(prKeySym);
    return iAmigaKeycode;
}

static int refresh_necessary = 0;

void handle_events (void)
{
    SDL_Event rEvent;
    int iAmigaKeyCode;
    int i, j;
    int iIsHotKey = 0;
#ifdef DEBUG_EVENTS
    dbg("Function: handle_events");
#endif

    /* Handle GUI events */
    gui_handle_events ();

#ifdef EMULATED_JOYSTICK
	{
		if ((vkbd_button3==(SDLKey)0)&&(!vkbd_mode))
			buttonstate[0]=emulated_mouse_button1;
		if ((vkbd_button4==(SDLKey)0)&&(!vkbd_mode))
			buttonstate[2]=emulated_mouse_button2;
	}
#endif

    while (SDL_PollEvent(&rEvent))
    {
	switch (rEvent.type)
	{
	case SDL_QUIT:
#ifdef DEBUG_EVENTS
	    dbg("Event: quit");
#endif
	    uae_quit();
	    break;
	    break;
	case SDL_JOYBUTTONDOWN:
	    if (vkbd_mode) break;
	    if ((rEvent.jbutton.button==6) && (vkbd_button2!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button2;
	    else if ((rEvent.jbutton.button==5) && (vkbd_button3!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button3;
	    else if ((rEvent.jbutton.button==1) && (vkbd_button4!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button4;
	    else
	    	break;
        case SDL_KEYDOWN:
#ifdef DEBUG_EVENTS
	    dbg("Event: key down");
#endif
	    if ((rEvent.key.keysym.sym!=SDLK_F11)&&(rEvent.key.keysym.sym!=SDLK_F12)&&(rEvent.key.keysym.sym!=SDLK_PAGEUP)
#ifdef EMULATED_JOYSTICK
		&&(rEvent.key.keysym.sym!=SDLK_ESCAPE)&&((rEvent.key.keysym.sym!=SDLK_SPACE)||((rEvent.key.keysym.sym==SDLK_SPACE)&&(vkbd_button3!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_LCTRL)&&((rEvent.key.keysym.sym!=SDLK_LALT)||((rEvent.key.keysym.sym==SDLK_LALT)&&(vkbd_button2!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_RETURN)&&((rEvent.key.keysym.sym!=SDLK_LSHIFT)||((rEvent.key.keysym.sym==SDLK_LSHIFT)&&(vkbd_button4!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_TAB)&&(rEvent.key.keysym.sym!=SDLK_BACKSPACE)&&(rEvent.key.keysym.sym!=SDLK_UP)&&(rEvent.key.keysym.sym!=SDLK_DOWN)&&(rEvent.key.keysym.sym!=SDLK_LEFT)&&(rEvent.key.keysym.sym!=SDLK_RIGHT)
#endif
			    )
	    {
		    if ((rEvent.key.keysym.sym==SDLK_LALT)&&(vkbd_button2!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button2;
		    else
		    if ((rEvent.key.keysym.sym==SDLK_LSHIFT)&&(vkbd_button4!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button4;
		    else
		    if ((rEvent.key.keysym.sym==SDLK_SPACE)&&(vkbd_button3!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button3;

		iAmigaKeyCode = keycode2amiga(&(rEvent.key.keysym));
		if (iAmigaKeyCode >= 0)
		{
		    if (!uae4all_keystate[iAmigaKeyCode])
		    {
			uae4all_keystate[iAmigaKeyCode] = 1;
			record_key(iAmigaKeyCode << 1);
		    }
		}
	    }
	    break;
	case SDL_JOYBUTTONUP:
	    if (vkbd_mode) break;
	    if ((rEvent.jbutton.button==6) && (vkbd_button2!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button2;
	    else if ((rEvent.jbutton.button==5) && (vkbd_button3!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button3;
	    else if ((rEvent.jbutton.button==1) && (vkbd_button4!=(SDLKey)0))
		    rEvent.key.keysym.sym=vkbd_button4;
	    else
	    	break;
	case SDL_KEYUP:
#ifdef DEBUG_EVENTS
	    dbg("Event: key up");
#endif
	    if ((rEvent.key.keysym.sym!=SDLK_F11)&&(rEvent.key.keysym.sym!=SDLK_F12)&&(rEvent.key.keysym.sym!=SDLK_PAGEUP)
#ifdef EMULATED_JOYSTICK
		&&(rEvent.key.keysym.sym!=SDLK_ESCAPE)&&((rEvent.key.keysym.sym!=SDLK_SPACE)||((rEvent.key.keysym.sym==SDLK_SPACE)&&(vkbd_button3!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_LCTRL)&&((rEvent.key.keysym.sym!=SDLK_LALT)||((rEvent.key.keysym.sym==SDLK_LALT)&&(vkbd_button2!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_RETURN)&&((rEvent.key.keysym.sym!=SDLK_LSHIFT)||((rEvent.key.keysym.sym==SDLK_LSHIFT)&&(vkbd_button4!=(SDLKey)0)&&(!vkbd_mode)))&&(rEvent.key.keysym.sym!=SDLK_TAB)&&(rEvent.key.keysym.sym!=SDLK_BACKSPACE)&&(rEvent.key.keysym.sym!=SDLK_UP)&&(rEvent.key.keysym.sym!=SDLK_DOWN)&&(rEvent.key.keysym.sym!=SDLK_LEFT)&&(rEvent.key.keysym.sym!=SDLK_RIGHT)
#endif
			    )
	    {
		    if ((rEvent.key.keysym.sym==SDLK_LALT)&&(vkbd_button2!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button2;
		    else
		    if ((rEvent.key.keysym.sym==SDLK_LSHIFT)&&(vkbd_button4!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button4;
		    else
		    if ((rEvent.key.keysym.sym==SDLK_SPACE)&&(vkbd_button3!=(SDLKey)0)&&(!vkbd_mode))
			    rEvent.key.keysym.sym=vkbd_button3;

		iAmigaKeyCode = keycode2amiga(&(rEvent.key.keysym));
		if (iAmigaKeyCode >= 0)
		{
		    uae4all_keystate[iAmigaKeyCode] = 0;
		    record_key((iAmigaKeyCode << 1) | 1);
		}
	    }
	    break;
	case SDL_MOUSEBUTTONDOWN:
#ifdef DEBUG_EVENTS
	    dbg("Event: mouse button down");
#endif
	    	buttonstate[(rEvent.button.button-1)%3] = 1;
	    break;
	case SDL_MOUSEBUTTONUP:
#ifdef DEBUG_EVENTS
	    dbg("Event: mouse button up");
#endif

	    	buttonstate[(rEvent.button.button-1)%3] = 0;
	    break;
	case SDL_MOUSEMOTION:
#ifdef DEBUG_EVENTS
	    dbg("Event: mouse motion");
#endif
	    lastmx += rEvent.motion.xrel<<1;
	    lastmy += rEvent.motion.yrel<<1;
	    newmousecounters = 1;
	    break;
	}
    }


    /* Handle UAE reset */
/*
    if ((uae4all_keystate[AK_CTRL] || uae4all_keystate[AK_RCTRL]) && uae4all_keystate[AK_LAMI] && uae4all_keystate[AK_RAMI])
	uae_reset ();
*/
#ifdef DEBUG_EVENTS
    dbg(" handle_events -> terminado");
#endif
}

int check_prefs_changed_gfx (void)
{
	extern int mainMenu_vpos;
	static int last_vpos=0;
	int ret=(last_vpos!=mainMenu_vpos);
	last_vpos=mainMenu_vpos;
	return ret;
}

int debuggable (void)
{
    return 1;
}

int needmousehack (void)
{
    return 1;
}



#if !defined(DREAMCAST) && !defined(DINGOO)
int lockscr (void)
{
#ifdef DEBUG_GFX
    dbg("Function: lockscr");
#endif
    return 1;
}

void unlockscr (void)
{
#ifdef DEBUG_GFX
    dbg("Function: unlockscr");
#endif
}
#endif

void gui_purge_events(void)
{
	SDL_Event event;
	SDL_Delay(150);
	while(SDL_PollEvent(&event))
		SDL_Delay(10);
	keybuf_init();
}

