 /* 
  * Libretro sound.c implementation
  * (c) Chips 2022
  */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "uae.h"
#include "options.h"
#include "memory.h"
#include "audio.h"
#include "gensound.h"
#include "sound.h"
#include "custom.h"

extern unsigned long next_sample_evtime;

int produce_sound=0;
int changed_produce_sound=0;

unsigned int sound_rate=DEFAULT_SOUND_FREQ;
int mainMenu_soundStereo = 1;

#define DEFAULT_SOUND_CHANNELS 2
#define SOUND_BUFFERS_COUNT 1

uae_u16 sndbuffer[SOUND_BUFFERS_COUNT][(SNDBUFFER_LEN+32)*DEFAULT_SOUND_CHANNELS];
unsigned n_callback_sndbuff, n_render_sndbuff;
uae_u16 *sndbufpt = sndbuffer[0];
uae_u16 *render_sndbuff = sndbuffer[0];

int cnt = 0;
int wrcnt = 0;

extern void retro_audiocb(signed short int *sound_buffer,int sndbufsize);

#ifdef NO_SOUND


void finish_sound_buffer (void) {  }

int setup_sound (void) { sound_available = 0; return 0; }

void close_sound (void) { }

int init_sound (void) { return 0; }

void pause_sound (void) { }

void resume_sound (void) { }


void uae4all_init_sound(void) { }

void uae4all_play_click(void) { }

void uae4all_pause_music(void) { }

void uae4all_resume_music(void) { }

#else 

void sound_default_evtime(void)
{
    int pal = beamcon0 & 0x20;

    switch(m68k_speed)
    {
        case 6:
            scaled_sample_evtime=(unsigned)(MAXHPOS_PAL*MAXVPOS_PAL*VBLANK_HZ_PAL*CYCLE_UNIT/1.86)/sound_rate;
            break;

        case 5:
        case 4: // ~4/3 234
            if (pal)
                scaled_sample_evtime=(MAXHPOS_PAL*244*VBLANK_HZ_PAL*CYCLE_UNIT)/sound_rate; // ???
            else
                scaled_sample_evtime=(MAXHPOS_NTSC*255*VBLANK_HZ_NTSC*CYCLE_UNIT)/sound_rate;
            break;

        case 3:
        case 2: // ~8/7 273
            if (pal)
                scaled_sample_evtime=(MAXHPOS_PAL*270*VBLANK_HZ_PAL*CYCLE_UNIT)/sound_rate;
            else
                scaled_sample_evtime=(MAXHPOS_NTSC*255*VBLANK_HZ_NTSC*CYCLE_UNIT)/sound_rate;
            break;

        case 1:
        default: // MAXVPOS_PAL?
            if (pal)
                scaled_sample_evtime=(MAXHPOS_PAL*313*VBLANK_HZ_PAL*CYCLE_UNIT)/sound_rate;
            else
                scaled_sample_evtime=(MAXHPOS_NTSC*MAXVPOS_NTSC*VBLANK_HZ_NTSC*CYCLE_UNIT)/sound_rate + 1;
            break;
    }
    schedule_audio();
}

void finish_sound_buffer (void)
{
    retro_audiocb((short int *)sndbuffer[0], SNDBUFFER_LEN / 2 );
    sndbufpt = render_sndbuff = sndbuffer[0];
}

/* Try to determine whether sound is available.  This is only for GUI purposes.  */
int setup_sound (void)
{
    return 1;
}


void close_sound (void)
{
}

int init_sound (void)
{
    sndbufpt = sndbuffer[0];
    render_sndbuff = sndbuffer[0];

    scaled_sample_evtime_ok = 1;
    sound_available = 1;

    sound_default_evtime();

    return 1;
}

void flush_audio(void)
{

    // Flush audio buffer in order to render all audio samples for a given frame. It's better for some frontend

    retro_audiocb((short int*) sndbuffer[wrcnt%SOUND_BUFFERS_COUNT], (sndbufpt - render_sndbuff)/2);

    sndbufpt = sndbuffer[0];
    render_sndbuff = sndbuffer[0];

}

void pause_sound (void)
{
}

void resume_sound (void)
{
}

void uae4all_init_sound(void)
{
}

void uae4all_pause_music(void)
{
}

void uae4all_resume_music(void)
{
}

void uae4all_play_click(void)
{
}


#endif

