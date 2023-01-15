#ifdef DREAMCAST
#include <kos.h>
extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);
#endif

/*
  * UAE - The Un*x Amiga Emulator
  *
  * Main program
  *
  * Copyright 1995 Ed Hanway
  * Copyright 1995, 1996, 1997 Bernd Schmidt
  */
#include "sysconfig.h"
#include "sysdeps.h"
#include <assert.h>

#include "config.h"
#include "uae.h"
#include "options.h"
#include "thread.h"
#include "debug_uae4all.h"
#include "gensound.h"
#include "events.h"
#include "memory.h"
#include "audio.h"
#include "sound.h"
#include "custom.h"
#include "m68k/m68k_intrf.h"
#include "disk.h"
#include "debug.h"
#include "xwin.h"
#include "joystick.h"
#include "keybuf.h"
#include "gui.h"
#include "zfile.h"
#include "autoconf.h"
#include "osemu.h"
#include "exectasks.h"
#include "compiler.h"
#include "bsdsocket.h"
#include "drawing.h"

#ifdef USE_SDL
#include "SDL.h"
#endif
#ifdef DREAMCAST
#include<SDL_dreamcast.h>
#endif
long int version = 256*65536L*UAEMAJOR + 65536L*UAEMINOR + UAESUBREV;

int no_gui = 0;
int joystickpresent = 0;
int cloanto_rom = 0;

struct gui_info gui_data;

char warning_buffer[256];

char optionsfile[256];

/* If you want to pipe printer output to a file, put something like
 * "cat >>printerfile.tmp" above.
 * The printer support was only tested with the driver "PostScript" on
 * Amiga side, using apsfilter for linux to print ps-data.
 *
 * Under DOS it ought to be -p LPT1: or -p PRN: but you'll need a
 * PostScript printer or ghostscript -=SR=-
 */

/* Slightly stupid place for this... */
/* ncurses.c might use quite a few of those. */
char *colormodes[] = { "256 colors", "32768 colors", "65536 colors",
    "256 colors dithered", "16 colors dithered", "16 million colors",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
};

#ifdef DINGOO
void dingoo_set_clock(unsigned int mhz);
#else
#define dingoo_set_clock()
#endif

void discard_prefs ()
{
}

extern char uae4all_image_file[128];
void default_prefs ()
{
#ifdef NO_SOUND
    produce_sound = 0;
#else
    produce_sound = 2;
#endif

    prefs_gfx_framerate = -1;


    strcpy (prefs_df[0], "df0.adf");
    strcpy (prefs_df[1], ROM_PATH_PREFIX "df1.adf");
    real_changed_df[0]=1;
    strcpy (uae4all_image_file, "df0.adf");

#ifdef DREAMCAST
    strcpy (romfile, ROM_PATH_PREFIX "kick.rom");
    strcpy (romfile_sd, "/sd/uae4all/" "kick.rom");
#else
//    strcpy (romfile, "/cdrom/kick.rom");
    strcpy (romfile, "kick.rom");
#endif

    prefs_chipmem_size=0x00100000;
}

int quit_program = 0;

void uae_reset (void)
{
    gui_purge_events();
/*
    if (quit_program == 0)
	quit_program = -2;
*/  
//    black_screen_now();
    quit_program = 2;
    set_special (SPCFLAG_BRK);
}

void uae_quit (void)
{
    if (quit_program != -1)
	quit_program = -1;
}

void reset_all_systems (void)
{
    init_eventtab ();

    memory_reset ();
}

/* Okay, this stuff looks strange, but it is here to encourage people who
 * port UAE to re-use as much of this code as possible. Functions that you
 * should be using are do_start_program() and do_leave_program(), as well
 * as real_main(). Some OSes don't call main() (which is braindamaged IMHO,
 * but unfortunately very common), so you need to call real_main() from
 * whatever entry point you have. You may want to write your own versions
 * of start_program() and leave_program() if you need to do anything special.
 * Add #ifdefs around these as appropriate.
 */

void do_start_program (void)
{
    /* Do a reset on startup. Whether this is elegant is debatable. */
#if defined(DREAMCAST) && !defined(DEBUG_UAE4ALL)
	while(1)
#endif
	{
		quit_program = 2;
		reset_frameskip();
		m68k_go (1);
	}
}

void do_leave_program (void)
{
    graphics_leave ();
    close_joystick ();
    close_sound ();
    dump_counts ();
    zfile_exit ();
#ifdef USE_SDL
    SDL_Quit ();
#endif
    memory_cleanup ();
}

#if defined(DREAMCAST) && !defined(DEBUG_UAE4ALL)
static uint32 uae4all_dc_args[4]={ 0, 0, 0, 0};
static void uae4all_dreamcast_handler(irq_t source, irq_context_t *context)
{
	irq_create_context(context,context->r[15], (uint32)&do_start_program, (uint32 *)&uae4all_dc_args[0],0);
}
#endif

#ifdef DREAMCAST
#include<dirent.h>
extern "C" { void fs_sdcard_shutdown(void); void fs_sdcard_init(void); int fs_sdcard_unmount(void); int fs_sdcard_mount(void); void sci_init(void); }

int sdcard_exists=0;
void reinit_sdcard(void)
{
	static uint32 last=(uint32)-5000;
	uint32 now=(((unsigned long long)timer_us_gettime64())>>10);
	if (now-last>5000) {
		char *dir="/sd/uae4all";
		DIR *d=NULL;
		fs_sdcard_shutdown();
		timer_spin_sleep(111);
		fs_sdcard_init();
		timer_spin_sleep(111);
		fs_mkdir(dir);
		d=opendir(dir);
		sdcard_exists=(d!=NULL);
		if (d)
			closedir(d);
		last=now;
	}
}
#endif

void start_program (void)
{
#if defined(DREAMCAST) && !defined(DEBUG_UAE4ALL)
    irq_set_handler(EXC_USER_BREAK_PRE,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_INSTR_ADDRESS,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_ILLEGAL_INSTR,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_SLOT_ILLEGAL_INSTR,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_GENERAL_FPU,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_SLOT_FPU,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_DATA_ADDRESS_WRITE,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_DTLB_MISS_WRITE,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_000,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_100,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_400,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_OFFSET_600,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_FPU,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_TRAPA,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_RESET_UDI,&uae4all_dreamcast_handler);
    irq_set_handler(EXC_UNHANDLED_EXC,&uae4all_dreamcast_handler);
#endif
    do_start_program ();
}

void leave_program (void)
{
    do_leave_program ();
}

typedef struct _cmdline_opt
{
	char *optname;
	int len;
	void *opt;
} cmdline_opt;

extern int  mainMenu_throttle, mainMenu_frameskip, mainMenu_sound, mainMenu_case, mainMenu_autosave, mainMenu_vpos;
extern unsigned int sound_rate;

extern char uae4all_image_file[128];
extern char uae4all_image_file2[128];

static cmdline_opt cmdl_opts[] =
{
//	{ "-statusln",        0, &mainMenu_showStatus },
//	{ "-mousemultiplier", 0, &mainMenu_mouseMultiplier },
	{ "-sound",           0, &mainMenu_sound },
	{ "-soundrate",       0, &sound_rate },
	{ "-autosave",        0, &mainMenu_autosave },
	{ "-systemclock",     0, &mainMenu_throttle },
//	{ "-syncthreshold",   0, &timeslice_mode },
	{ "-frameskip",       0, &mainMenu_frameskip },
//	{ "-skipintro",       0, &skipintro },
#ifdef ANDROIDSDL
	{ "-onscreen",       0, &mainMenu_onScreen },
#endif
//	{ "-ntsc",            0, &mainMenu_ntsc },
//	{ "-joyconf",            0, &mainMenu_joyConf },
//	{ "-use1mbchip",            0, &mainMenu_chipMemory },
//	{ "-autofire",            0, &mainMenu_autofire },
//	{ "-drives",            0, &mainMenu_drives },
//	{ "-script",            0, &mainMenu_enableScripts},
//	{ "-screenshot",            0, &mainMenu_enableScreenshots},
	{ "-kick",            sizeof(romfile), romfile },
	{ "-df0",             sizeof(uae4all_image_file), uae4all_image_file },
	{ "-df1",             sizeof(uae4all_image_file2), uae4all_image_file2 },
//	{ "-df2",             sizeof(uae4all_image_file2), uae4all_image_file2 },
//	{ "-df3",             sizeof(uae4all_image_file2), uae4all_image_file3 },
};

void parse_cmdline(int argc, char **argv)
{
	int arg, i, found;
	printf("Parsing %i parameters.\n",argc);

	for (arg = 1; arg < argc-1; arg++)
	{
		for (i = found = 0; i < sizeof(cmdl_opts) / sizeof(cmdl_opts[0]); i++)
		{
			if (strcmp(argv[arg], cmdl_opts[i].optname) == 0)
			{
				arg++;
				if (cmdl_opts[i].len == 0)
					*(int *)(cmdl_opts[i].opt) = atoi(argv[arg]);
				else
				{
					strncpy((char *)cmdl_opts[i].opt, argv[arg], cmdl_opts[i].len);
					((char *)cmdl_opts[i].opt)[cmdl_opts[i].len-1] = 0;
				}
				found = 1;
				break;
			}
		}
		if (!found) printf("skipping unknown option: \"%s\"\n", argv[arg]);
	}
}

extern void update_prefs_retrocfg(void);

void real_main (int argc, char **argv)
{
#ifdef USE_SDL
//    SDL_Init (SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE);
    SDL_Init (SDL_INIT_VIDEO | SDL_INIT_JOYSTICK 
#ifndef NO_SOUND
 			| SDL_INIT_AUDIO
#endif
	);
#endif

    default_prefs ();

    parse_cmdline(argc, argv);
#ifdef __LIBRETRO__
    update_prefs_retrocfg();
#endif
    if (! graphics_setup ()) {
	exit (1);
    }
    rtarea_init ();

    machdep_init ();

    if (! setup_sound ()) {
	write_log ("Sound driver unavailable: Sound output disabled\n");
	produce_sound = 0;
    }
    init_joystick ();
    int err = gui_init ();
    if (err == -1) {
        write_log ("Failed to initialize the GUI\n");
    } else if (err == -2) {
        exit (0);
    }
    if (sound_available && produce_sound > 1 && ! init_audio ()) {
	write_log ("Sound driver unavailable: Sound output disabled\n");
	produce_sound = 0;
    }

    /* Install resident module to get 8MB chipmem, if requested */
    rtarea_setup ();

    keybuf_init (); /* Must come after init_joystick */

    memory_init ();

    custom_init (); /* Must come after memory_init */
    DISK_init ();

    init_m68k();
#ifndef USE_FAME_CORE
    compiler_init ();
#endif
    gui_update ();

//    dingoo_set_clock(430);
#ifndef __LIBRETRO__
    if (graphics_init ())
		start_program ();
    leave_program ();
#endif
}

#ifndef NO_MAIN_IN_MAIN_C
int main (int argc, char **argv)
{
#ifdef DREAMCAST
#if defined(DEBUG_UAE4ALL) || defined(DEBUG_FRAMERATE) || defined(PROFILER_UAE4ALL) || defined(AUTO_RUN)
	{
		SDL_DC_ShowAskHz(SDL_FALSE);
    		puts("MAIN !!!!");
	}
#endif
#endif
#ifdef DEBUG_FILE
    DEBUG_STR_FILE=fopen(DEBUG_FILE,"wt");
#endif
    real_main (argc, argv);
    return 0;
}

#endif
