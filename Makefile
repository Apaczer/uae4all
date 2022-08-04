NAME   = uae4all.bittboy
O      = o
RM     = rm -f
CC     = arm-linux-gcc
CXX    = arm-linux-g++
STRIP  = arm-linux-strip
SDL_BASE = /opt/miyoo/arm-miyoo-linux-uclibcgnueabi/sysroot/usr/bin/
PROFILE = APPLY


PROG   = $(NAME)

all: $(PROG)

FAME_CORE=1
LIB7Z=1


DEFAULT_CFLAGS = -I/opt/miyoo/arm-miyoo-linux-uclibcgnueabi/include -I/opt/miyoo/arm-miyoo-linux-uclibcgnueabi/sysroot/usr/include -I/opt/miyoo/arm-miyoo-linux-uclibcgnueabi/sysroot/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRAN -DINGUX
LDFLAGS        = -lSDL_mixer -lSDL -lz -lpthread
#-static -lmikmod -lz 


MORE_CFLAGS = -O3  -Isrc/ -Isrc/include/ -Isrc/menu -Isrc/vkbd -I/opt/miyoo/arm-miyoo-linux-uclibcgnueabi/include/c++/7.3.0/ -fomit-frame-pointer  -Wno-unused -Wno-format -DUSE_SDL -DGCCCONSTFUNC="__attribute__((const))" -DUSE_UNDERSCORE -fno-exceptions -DUNALIGNED_PROFITABLE -DREGPARAM="__attribute__((regparm(3)))" -DOPTIMIZED_FLAGS -D__inline__=__inline__ -DSHM_SUPPORT_LINKS=0 -DOS_WITHOUT_MEMORY_MANAGEMENT -DVKBD_ALWAYS -DMENU_MUSIC

MORE_CFLAGS += -falign-functions -falign-loops -falign-labels -falign-jumps \
-ffast-math -fsingle-precision-constant -funsafe-math-optimizations \
-fomit-frame-pointer -fno-builtin -fno-exceptions -fno-common \
-fstrict-aliasing  -fexpensive-optimizations -fno-rtti \
-finline -finline-functions -fpeel-loops

OPTIMISE= -D_ZAURUS -O2 -ffast-math -fstrict-aliasing -fomit-frame-pointer -ftree-vectorize -funroll-all-loops -fpeel-loops -ftracer -funswitch-loops -finline-functions

ifeq ($(PROFILE), YES)
OPTIMISE 		+= -fprofile-generate=/mnt/profile
else ifeq ($(PROFILE), APPLY)
OPTIMISE		+= -fprofile-use -fbranch-probabilities
endif


MORE_CFLAGS+= $(OPTIMISE) 
MORE_CFLAGS+= -DROM_PATH_PREFIX=\"./\" -DDATA_PREFIX=\"./data/\" -DSAVE_PREFIX=\"./\"

#MORE_CFLAGS+= -DUSE_AUTOCONFIG
#MORE_CFLAGS+= -DUAE_CONSOLE
#MORE_CFLAGS+= -DGP2X
MORE_CFLAGS+= -DUSE_ZFILE
#MORE_CFLAGS+= -DUAE4ALL_NO_USE_RESTRICT
#MORE_CFLAGS+= -DNO_SOUND
MORE_CFLAGS+= -DNO_THREADS
MORE_CFLAGS+= -DEMULATED_JOYSTICK
#MORE_CFLAGS+= -DDEBUG_TIMESLICE
#MORE_CFLAGS+= -DFAME_INTERRUPTS_PATCH
#MORE_CFLAGS+= -DFAME_INTERRUPTS_SECURE_PATCH

#MORE_CFLAGS+= -DUAE_MEMORY_ACCESS
#MORE_CFLAGS+= -DSAFE_MEMORY_ACCESS
#MORE_CFLAGS+= -DERROR_WHEN_MEMORY_OVERRUN

MORE_CFLAGS+= -DDEBUG_UAE4ALL
MORE_CFLAGS+= -DDEBUG_FILE=\"stdout.txt\"
##MORE_CFLAGS+= -DDEBUG_UAE4ALL_FFLUSH
#MORE_CFLAGS+= -DDEBUG_M68K
#MORE_CFLAGS+= -DDEBUG_INTERRUPTS
#MORE_CFLAGS+= -DDEBUG_CIA
###MORE_CFLAGS+= -DDEBUG_SOUND
#MORE_CFLAGS+= -DDEBUG_MEMORY
###MORE_CFLAGS+= -DDEBUG_MAPPINGS
#MORE_CFLAGS+= -DDEBUG_DISK
#MORE_CFLAGS+= -DDEBUG_CUSTOM
###MORE_CFLAGS+= -DDEBUG_EVENTS
###MORE_CFLAGS+= -DDEBUG_GFX -DDEBUG_BLITTER
###MORE_CFLAGS+= -DDEBUG_FRAMERATE
###MORE_CFLAGS+= -DAUTO_FRAMERATE=1400
###MORE_CFLAGS+= -DMAX_AUTO_FRAMERATE=4400
###MORE_CFLAGS+= -DAUTO_FRAMERATE_SOUND
#MORE_CFLAGS+= -DSTART_DEBUG=999999
#MORE_CFLAGS+= -DMAX_AUTOEVENTS=999990
#MORE_CFLAGS+= -DAUTO_RUN
MORE_CFLAGS+= -DHOME_DIR


#MORE_CFLAGS+= -DPROFILER_UAE4ALL

CFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)

OBJS =	\
	src/savestate.o \
	src/audio.o \
	src/autoconf.o \
	src/blitfunc.o \
	src/blittable.o \
	src/blitter.o \
	src/cia.o \
	src/savedisk.o \
	src/compiler.o \
	src/custom.o \
	src/disk.o \
	src/drawing.o \
	src/ersatz.o \
	src/gfxutil.o \
	src/keybuf.o \
	src/main.o \
	src/md-support.o \
	src/memory.o \
	src/missing.o \
	src/gui.o \
	src/od-joy.o \
	src/sound.o \
	src/sdlgfx.o \
	src/writelog.o \
	src/zfile.o \
	src/menu/fade.o \
	src/menu/menu.o \
	src/menu/menu_save.o \
	src/menu/menu_load.o \
	src/menu/menu_main.o \
	src/vkbd/vkbd.o \
	src/dingoo.o \


ifdef FAME_CORE
CFLAGS+=-DUSE_FAME_CORE -DUSE_FAME_CORE_C -DFAME_IRQ_CLOCKING -DFAME_CHECK_BRANCHES -DFAME_EMULATE_TRACE -DFAME_DIRECT_MAPPING -DFAME_BYPASS_TAS_WRITEBACK -DFAME_ACCURATE_TIMING -DFAME_GLOBAL_CONTEXT -DFAME_FETCHBITS=8 -DFAME_DATABITS=8 -DFAME_GOTOS -DFAME_EXTRA_INLINE=__inline__ -DINLINE=__inline__ -DFAME_NO_RESTORE_PC_MASKED_BITS

src/m68k/fame/famec.o: src/m68k/fame/famec.cpp
OBJS += \
	src/m68k/fame/famec.o \
	src/m68k/fame/m68k_intrf.o
else
OBJS += \
	src/m68k/uae/newcpu.o \
	src/m68k/uae/readcpu.o \
	src/m68k/uae/cpudefs.o \
	src/m68k/uae/fpp.o \
	src/m68k/uae/cpustbl.o \
	src/m68k/uae/cpuemu.o

endif

CPPFLAGS  = $(CFLAGS)

$(PROG): $(OBJS) 
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)
	$(STRIP) $(PROG)


run: $(PROG)
	./$(PROG)

clean:
	$(RM) $(PROG) $(OBJS)


