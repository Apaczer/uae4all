/* 
 * Uae4all libretro core implementation
 * (c) Chips 2022
 */

#include "libretro.h"
#include "libretro-core.h"

#include "uae.h"

#include "sysconfig.h"
#include "sysdeps.h"
#include "options.h"
#include "custom.h"

#include "m68k/uae/newcpu.h"

#include "savestate.h"

#include "zlib.h"
#include "fsdb.h"
#include "filesys.h"
#include "autoconf.h"

unsigned int VIRTUAL_WIDTH=PREFS_GFX_WIDTH;
unsigned int retrow=PREFS_GFX_WIDTH;
unsigned int retroh=PREFS_GFX_HEIGHT;

extern char *gfx_mem;

extern char uae4all_image_file[];
extern char uae4all_image_file2[];

extern int mainMenu_throttle;

extern void DISK_GUI_change (void);

static unsigned msg_interface_version = 0;


#define RETRO_DEVICE_AMIGA_KEYBOARD RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_KEYBOARD, 0)
#define RETRO_DEVICE_AMIGA_JOYSTICK RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 1)

unsigned amiga_devices[ 2 ];

extern int SHIFTON,pauseg;
char RPATH[512];
extern int SHOWKEY;

#include "cmdline.cpp"

extern void update_input(void);
extern void texture_init(void);
extern void texture_uninit(void);
extern void input_gui(void);
extern void retro_virtualkb(void);

extern void flush_audio(void);

const char *retro_save_directory;
const char *retro_system_directory;
const char *retro_content_directory;

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;

struct zfile *retro_deserialize_file = NULL;
static size_t save_state_file_size = 0;

int libretroreset = 1;

// Amiga default kickstarts

#define A500_ROM        "kick34005.A500"


#ifdef _WIN32
#define RETRO_PATH_SEPARATOR            "\\"
#else
#define RETRO_PATH_SEPARATOR            "/"
#endif

void path_join(char* out, const char* basedir, const char* filename)
{
   snprintf(out, 64, "%s%s%s", basedir, RETRO_PATH_SEPARATOR, filename);
}


static void trimwsa (char *s)
{
  /* Delete trailing whitespace.  */
  int len = strlen (s);
  while (len > 0 && strcspn (s + len - 1, "\t \r\n") == 0)
    s[--len] = '\0';
}

// Very light management of .uae file...

void cfgfile_load(const char *filename)
{
   FILE * fh;
   char line[128];

   fh = fopen(filename,"rb");
   if (!fh)
      return;

   while (fgets (line, sizeof (line), fh) != 0)
   {
      if ((strlen(line) >= strlen("floppy") + 3))
         if (!strncmp(line, "floppy", 6))
         {
            if ((line[6] == '0') && (line[7] == '='))
            {
               strncpy(uae4all_image_file, &line[8], 127);
               trimwsa(uae4all_image_file);
            }
            if ((line[6] == '1') && (line[7] == '='))
            {
               strncpy(uae4all_image_file2, &line[8], 127);
               trimwsa(uae4all_image_file2);
            }
         }   
   }
   fclose(fh);
}



void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

  static const struct retro_controller_description p1_controllers[] = {
    { "AMIGA Joystick", RETRO_DEVICE_AMIGA_JOYSTICK },
    { "AMIGA Keyboard", RETRO_DEVICE_AMIGA_KEYBOARD },
  };
  static const struct retro_controller_description p2_controllers[] = {
    { "AMIGA Joystick", RETRO_DEVICE_AMIGA_JOYSTICK },
    { "AMIGA Keyboard", RETRO_DEVICE_AMIGA_KEYBOARD },
  };

  static const struct retro_controller_info ports[] = {
    { p1_controllers, 2  }, // port 1
    { p2_controllers, 2  }, // port 2
    { NULL, 0 }
  };

  cb( RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports );

  struct retro_variable variables[] = {
//      { "uae4all_fastmem",        "Fast Mem; None|1 MB|2 MB|4 MB|8 MB", },
//      { "uae4all_resolution",     "Internal resolution; 320x240", },
//      { "uae4all_leds_on_screen", "Leds on screen; on|off", },
//      { "uae4all_floppy_speed",   "Floppy speed; 100|200|400|800", },
      { "uae4all_throttle",   "Optimize level; none|1|2|3|4|5", },
      { NULL, NULL },
   };

   cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
}


void Retro_Msg(const char * msg_str)
{
   if (msg_interface_version >= 1)
   {
      struct retro_message_ext msg = {
         msg_str,
         3000,
         3,
         RETRO_LOG_WARN,
         RETRO_MESSAGE_TARGET_ALL,
         RETRO_MESSAGE_TYPE_NOTIFICATION,
         -1
      };
      environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE_EXT, &msg);
   }
   else
   {
      struct retro_message msg = {
         msg_str,
         180
      };
      environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &msg);
   }
}


void update_prefs_retrocfg(void)
{
   struct retro_variable var;

   var.key = "uae4all_throttle";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (strcmp(var.value, "none")  == 0)
         mainMenu_throttle = 0;
      else
         mainMenu_throttle = atoi(var.value);
      
   }

#if 0

   var.key = "uae4all_resolution";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      char *pch;
      char str[100];
      snprintf(str, sizeof(str), var.value);

      pch = strtok(str, "x");
      if (pch)
         retrow = strtoul(pch, NULL, 0);
      pch = strtok(NULL, "x");
      if (pch)
         retroh = strtoul(pch, NULL, 0);

      prefs->gfx_size.width  = retrow;
      prefs->gfx_size.height = retroh;
      prefs->gfx_resolution  = prefs->gfx_size.width > 600 ? 1 : 0;

      LOGI("[libretro-uae4all]: Got size: %u x %u.\n", retrow, retroh);

      VIRTUAL_WIDTH = retrow;
      texture_init();

   }

   var.key = "uae4all_leds_on_screen";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (strcmp(var.value, "on")  == 0) prefs->leds_on_screen = 1;
      if (strcmp(var.value, "off") == 0) prefs->leds_on_screen = 0;
   }

   var.key = "uae4all_fastmem";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (strcmp(var.value, "None") == 0)
      {
         prefs->fastmem[0].size = 0;
      }
      if (strcmp(var.value, "1 MB") == 0)
      {
         prefs->fastmem[0].size = 0x100000;
      }
      if (strcmp(var.value, "2 MB") == 0)
      {
         prefs->fastmem[0].size = 0x100000 * 2;
      }
      if (strcmp(var.value, "4 MB") == 0)
      {
         prefs->fastmem[0].size = 0x100000 * 4;
      }
      if (strcmp(var.value, "8 MB") == 0)
      {
         prefs->fastmem[0].size = 0x100000 * 8;
      }
   }


   var.key = "uae4all_floppy_speed";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      prefs->floppy_speed=atoi(var.value);
   }
#endif

   if (strcasestr(RPATH,".uae"))
      cfgfile_load(RPATH);

   path_join(romfile, retro_system_directory, A500_ROM);
}


void retro_shutdown_core(void)
{
   LOGI("SHUTDOWN\n");

   texture_uninit();
   environ_cb(RETRO_ENVIRONMENT_SHUTDOWN, NULL);
}

void retro_reset(void)
{
   libretroreset = 1;
   uae_reset();
}

void retro_init(void)
{
   const char *system_dir = NULL;

   msg_interface_version = 0;
   environ_cb(RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION, &msg_interface_version);

   if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &system_dir) && system_dir)
   {
      // if defined, use the system directory
      retro_system_directory=system_dir;
   }

   const char *content_dir = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY, &content_dir) && content_dir)
   {
      // if defined, use the system directory
      retro_content_directory=content_dir;
   }		

   const char *save_dir = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &save_dir) && save_dir)
   {
      // If save directory is defined use it, otherwise use system directory
      retro_save_directory = *save_dir ? save_dir : retro_system_directory;      
   }
   else
   {
      // make retro_save_directory the same in case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY is not implemented by the frontend
      retro_save_directory=retro_system_directory;
   }

   LOGI("Retro SYSTEM_DIRECTORY %s\n", retro_system_directory );
   LOGI("Retro SAVE_DIRECTORY %s\n",   retro_save_directory   );
   LOGI("Retro CONTENT_DIRECTORY %s\n",retro_content_directory);

#ifndef RENDER16B
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
#else
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
#endif
   
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      fprintf(stderr, "PIXEL FORMAT is not supported.\n");
      LOGI("PIXEL FORMAT is not supported.\n");
      exit(0);
   }

   // Savestates
   // > Considered incomplete because runahead cannot
   //   be enabled until content is full loaded
   static uint64_t quirks = RETRO_SERIALIZATION_QUIRK_INCOMPLETE;
   environ_cb(RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS, &quirks);

#if 0
   // > Ensure save state de-serialization file
   //   is closed/NULL
   //   (redundant safety check, possibly required
   //   for static builds...)
   if (retro_deserialize_file)
   {
      zfile_fclose(retro_deserialize_file);
      retro_deserialize_file = NULL;
   }
#endif

   struct retro_input_descriptor inputDescriptors[] = {
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "A" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "B" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,      "X" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,      "Y" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,      "R" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,      "L" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2,     "R2" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2,     "L2" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3,     "R3" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3,     "L3" }
   };
   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, &inputDescriptors);

   memset(Key_State,0,512);

   texture_init();

}

extern void main_exit();
void retro_deinit(void)
{
   texture_uninit();
   uae_quit ();

   LOGI("Retro DeInit\n");
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}


void retro_set_controller_port_device( unsigned port, unsigned device )
{
  if ( port < 2 )
  {
    amiga_devices[ port ] = device;

    LOGI(" (%d)=%d \n",port,device);
  }
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "uae4all";
   info->library_version  = "0.7";
   info->valid_extensions = "adf|adz|zip";
   info->need_fullpath    = true;
   info->block_extract    = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   //FIXME handle vice PAL/NTSC
   struct retro_game_geometry geom = { retrow, retroh, retrow, retroh,4.0 / 3.0 };
   struct retro_system_timing timing = { 50.0, 44100.0 };

   info->geometry = geom;
   info->timing   = timing;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}


void retro_audiocb(signed short int *sound_buffer,int sndbufsize)
{
    if(pauseg==0)
        if (audio_batch_cb)
            audio_batch_cb(sound_buffer, sndbufsize);
}


extern int Retro_PollEvent();
extern unsigned long sample_evtime, scaled_sample_evtime;

void retro_run(void)
{
   int x;
   bool updated = false;
   static int Deffered = 0;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      update_prefs_retrocfg();

   // Too early event poll or savesate segfault since emu not initialized...
   if (Deffered == 0)
   {
      Deffered = 1;
   }
   else
   {
      if (Deffered == 1)
      {
#if 0
         // Save states
         // > Ensure that save state file path is empty,
         //   since we use memory based save states
         savestate_fname[0] = '\0';
         // > Get save state size
         //   Here we use initial size + 5%
         //   Should be sufficient in all cases
         // NOTE: It would be better to calculate the
         // state size based on current config parameters,
         // but while
         //   - currprefs.chipmem_size
         //   - currprefs.bogomem_size
         //   - currprefs.fastmem_size
         // account for *most* of the size, there are
         // simply too many other factors to rely on this
         // alone (i.e. mem size + 5% is fine in most cases,
         // but if the user supplies a custom uae config file
         // then this is not adequate at all). Untangling the
         // full set of values that are recorded is beyond
         // my patience...
         struct zfile *state_file = save_state("libretro", 0);

         if (state_file)
         {
            save_state_file_size  = (size_t)zfile_size(state_file);
            save_state_file_size += (size_t)(((float)save_state_file_size * 0.05f) + 0.5f);
            zfile_fclose(state_file);
         }
#endif
         Deffered = 2;
      }

      Retro_PollEvent();
   }


   m68k_go (1);


   if(pauseg==0)
      if(SHOWKEY)
         retro_virtualkb();

   flush_audio();

   DISK_GUI_change();

   video_cb(gfx_mem,retrow,retroh,retrow<<PIXEL_BYTES);

}


bool retro_load_game(const struct retro_game_info *info)
{
   const char *full_path;

   full_path = info->path;

   strcpy(RPATH,full_path);

   pre_main(RPATH);

   libretroreset = 1;

   quit_program = 2;

   return true;
}

void retro_unload_game(void)
{
#if 0
   // Ensure save state de-serialization file
   // is closed/NULL
   // Note: Have to do this here (not in retro_deinit())
   // since leave_program() calls zfile_exit()
   if (retro_deserialize_file)
   {
      zfile_fclose(retro_deserialize_file);
      retro_deserialize_file = NULL;
   }
#endif
   pauseg=0;
   leave_program ();
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_PAL;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
   (void)type;
   (void)info;
   (void)num;
   return false;
}

size_t retro_serialize_size(void)
{
#if 0
   return save_state_file_size;
#else
   return 0;
#endif
}

bool retro_serialize(void *data_, size_t size)
{
#if 0
   struct zfile *state_file = save_state("libretro", (uae_u64)save_state_file_size);
   bool success = false;

   if (state_file)
   {
      uae_s64 state_file_size = zfile_size(state_file);

      if (size >= state_file_size)
      {
         size_t len = zfile_fread(data_, 1, state_file_size, state_file);

         if (len == state_file_size)
            success = true;
      }

      zfile_fclose(state_file);
   }

   return success;
#else
   return false;
#endif
}

bool retro_unserialize(const void *data_, size_t size)
{
#if 0
   // TODO: When attempting to use runahead, CD32
   // and WHDLoad content will hang on boot. It seems
   // we cannot restore a state until the system has
   // passed some level of initialisation - but the
   // point at which a restore becomes 'safe' is
   // unknown (for CD32 content, for example, we have
   // to wait ~300 frames before runahead can be enabled)
   bool success = false;

   // Cannot restore state while any 'savestate'
   // operation is underway
   // > Actual restore is deferred until m68k_go(),
   //   so we have to use a shared shared state file
   //   object - this cannot be modified until the
   //   restore is complete
   // > Note that this condition should never be
   //   true - if a save state operation is underway
   //   at this point then we are dealing with an
   //   unknown error
   if (!savestate_state)
   {
      // Savestates also save CPU prefs, therefore refresh core options, but skip it for now
      //request_check_prefs_timer = 4;

      if (retro_deserialize_file)
      {
         zfile_fclose(retro_deserialize_file);
         retro_deserialize_file = NULL;
      }

      retro_deserialize_file = zfile_fopen_empty(NULL, "libretro", size);

      if (retro_deserialize_file)
      {
         size_t len = zfile_fwrite(data_, 1, size, retro_deserialize_file);

         if (len == size)
         {
            unsigned frame_counter = 0;
            unsigned max_frames    = 50;

            zfile_fseek(retro_deserialize_file, 0, SEEK_SET);
            savestate_state = STATE_DORESTORE;

            // For correct operation of the frontend,
            // the save state restore must be completed
            // by the time this function returns.
            // Since P-UAE requires several (2) frames to get
            // itself in order during a restore event, we
            // have to keep emulating frames until the
            // restore is complete...
            // > Note that we set a 'timeout' of 50 frames
            //   here (1s of emulated time at 50Hz) to
            //   prevent lock-ups in the event of unexpected
            //   errors
            // > Temporarily 'deactivate' runloop - this lets
            //   us call m68k_go() without accessing frontend
            //   features - specifically, it disables the audio
            //   callback functionality
#if 0 // TEMP
            libretro_runloop_active = 0;
            while (savestate_state && (frame_counter < max_frames))
            {
               // Note that retro_deserialize_file will be
               // closed inside m68k_go() upon successful
               // completion of the restore event
               restart_pending = m68k_go(1, 1);
               frame_counter++;
            }
            libretro_runloop_active = 1;

            // If the above while loop times out, then
            // everything is completely broken. We cannot
            // handle this here, so just assume the restore
            // completed successfully...
            request_reset_drawing = true;
            success               = true;
#endif
         }
         else
         {
            zfile_fclose(retro_deserialize_file);
            retro_deserialize_file = NULL;
         }
      }
   }

   return success;
#else
   return false;
#endif
}

void *retro_get_memory_data(unsigned id)
{
   (void)id;
   return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
   (void)id;
   return 0;
}

void retro_cheat_reset(void) {}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}

