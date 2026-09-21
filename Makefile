NAME    = uae4all
O       = o
RM      = rm -f

# ---------------------------------------------------------------------------
# Build mode: LIBRETRO=1 builds the libretro core, default builds standalone
# ---------------------------------------------------------------------------

# for libretro
LIBRETRO      ?= 0
STATIC_LINKING ?=0

INSTALLDIR ?= $(HOME)

# Possible values: 0, YES, APPLY
PROFILE ?= 0

ifeq ($(LIBRETRO), 1)
  ifeq ($(STATIC_LINKING), 1)
    PROG = $(NAME)_libretro.a
  else
    PROG = $(NAME)_libretro.so
  endif
else
  PROG       = $(NAME)
  VERSION   ?= $(shell date +%Y-%m-%d\ %H:%M)
  ASSETSDIR  = assets
  OPKG_ASSETSDIR = opkg_assets
  LINK       = $(NAME).lnk
  DESTDIR    = emus
  SECTION    = emulators
  ALIASES    = aliases.txt
  RELEASEDIR = package
  DATADIR    = data
  OPKDIR     = opk_data
endif

# ---------------------------------------------------------------------------
# Toolchain: platform=miyoo (cross-compile); no platform (native host)
# ---------------------------------------------------------------------------
ifeq ($(platform), miyoo)
  INSTALLDIR = /mnt
  CHAINPREFIX  ?= /opt/miyoo
  CROSS_COMPILE = $(CHAINPREFIX)/usr/bin/arm-linux-
endif

ifneq ($(CROSS_COMPILE),)
  ifeq (,$(findstring arm,$(CC)))
    CC    = $(CROSS_COMPILE)gcc
  endif
  ifeq (,$(findstring arm,$(CXX)))
    CXX   = $(CROSS_COMPILE)g++
  endif
  ifeq (,$(findstring arm,$(STRIP)))
    STRIP = $(CROSS_COMPILE)strip
  endif
  ifeq (,$(findstring arm,$(AS)))
    AS    = $(CROSS_COMPILE)as
  endif
  ifeq (,$(findstring arm,$(AR)))
    AR    = $(CROSS_COMPILE)ar
  endif
else
  CC    ?= gcc
  CXX   ?= g++
  STRIP ?= strip
  AS    ?= as
  AR    ?= ar
endif

$(info CC=$(CC))
$(info CXX=$(CXX))
$(info AS=$(AS))
$(info STRIP=$(STRIP))
$(info AR=$(AR))

all: $(PROG)

# ---------------------------------------------------------------------------
# Platform / mode configuration
# ---------------------------------------------------------------------------
ifeq ($(LIBRETRO), 1)
  ## --- Libretro build ---

  # Pick correct default M68K core by platform
  ifeq ($(platform), miyoo)
    LDFLAGS         = -lz -lpthread
    OPTIMIZE_CFLAGS = -g -O3 -fno-PIC
    MORE_CFLAGS    += -DHAVE_KBDJOYPAD -DUSE_CAST_UNSIGNED
    ifndef UAE_CORE
      #MORE_CFLAGS += -DFAST_COMPILER # doesn't seem to help and seems to brake on UAE_CORE
      ifndef FAME_CORE
        CYCLONE_CORE = 1
      endif
    endif
  else
    # Native / other host
    LDFLAGS         = -lz -lpthread
    OPTIMIZE_CFLAGS = -g -O0 -fPIC
    ifndef UAE_CORE
      ifndef CYCLONE_CORE  # only applicable to arm
        FAME_CORE   = 1
        FAME_CORE_C = 1
      endif
    endif
  endif

  ifeq ($(STATIC_LINKING), 0)
    SHARED = -shared -Wl,--version-script=libretro/link.T
  else
    MORE_CFLAGS += -DNO_MAIN_IN_MAIN_C
  endif

  MORE_CFLAGS += -Ilibretro/include/ -Ilibretro/core/
  MORE_CFLAGS += -DROM_PATH_PREFIX=\"./\" -DDATA_PREFIX=\"./data/\" -DSAVE_PREFIX=\"./\"
  MORE_CFLAGS += -D__LIBRETRO__ -DNO_VKBD
  MORE_CFLAGS += -DGCCCONSTFUNC="__attribute__((const))" -DUSE_UNDERSCORE -DOPTIMIZED_FLAGS -DSHM_SUPPORT_LINKS=0

else
  ## --- Standalone build ---

  ## use 7z savestate compression for *.asf files
  LIB7Z   ?= 1
  HOME_DIR ?= 1
  #SOUND_NEW=1  # minimal audio by notaz via gp2x (experimental)

  SYSROOT      := $(shell $(CC) --print-sysroot)
  PKGS          = sdl SDL_image zlib SDL_mixer
  PKGS_CFLAGS   = $(shell $(SYSROOT)/../../usr/bin/pkg-config --cflags $(PKGS))
  PKGS_LIBS     = $(shell $(SYSROOT)/../../usr/bin/pkg-config --libs $(PKGS))

  DEFAULT_CFLAGS = $(PKGS_CFLAGS) -DMIYOO
  LDFLAGS        = $(PKGS_LIBS)

  ifndef FAME_CORE
    LDFLAGS += -lm
  endif

  OPTIMIZE_CFLAGS = -O2 -fno-rtti

  MORE_CFLAGS += -DUSE_SDL -DDOUBLEBUFFER -DNO_DEFAULT_THROTTLE
  MORE_CFLAGS += -DROM_PATH_PREFIX=\"./\" -DSAVE_PREFIX=\"./\"

  ifeq ($(platform), miyoo)
    MORE_CFLAGS += -DDATA_PREFIX=\"./data/\"
    MORE_CFLAGS += -DUSE_CAST_UNSIGNED
    ## pick default core unless overridden
    ifndef CYCLONE_CORE
      ifndef FAME_CORE
        #UAE_CORE = 1
      endif
    endif
  else
    # Native host standalone
    MORE_CFLAGS += -DDATA_PREFIX=\"./assets/data/\"
    MORE_CFLAGS += -DCWD_MENU_DIR
    ifndef FAME_CORE
      #UAE_CORE   = 1
    endif
  endif

  MORE_CFLAGS += -DUSE_BLIT_MASKTABLE

  ifndef SOUND_NEW
    MORE_CFLAGS += -DMENU_MUSIC
    MORE_CFLAGS += -DNO_THREADS
  endif

endif  # LIBRETRO / standalone

ifeq ($(LTO), yes)
LDFLAGS += -flto
endif

ifeq ($(PROFILE), YES)
LDFLAGS += -lgcov
OPTIMIZE_CFLAGS += -fprofile-generate=$(INSTALLDIR)/profile
else ifeq ($(PROFILE), APPLY)
OPTIMIZE_CFLAGS += -fprofile-use=profile -fbranch-probabilities
endif

# ---------------------------------------------------------------------------
# Common flags (both modes)
# ---------------------------------------------------------------------------
MORE_CFLAGS += -Isrc/ -Isrc/include/ -Isrc/menu -Isrc/vkbd
MORE_CFLAGS += -fomit-frame-pointer -Wno-unused -Wno-format -fno-exceptions
MORE_CFLAGS += -DUNALIGNED_PROFITABLE -DREGPARAM="__attribute__((regparm(3)))" -D__inline__=__inline__
MORE_CFLAGS += -DOS_WITHOUT_MEMORY_MANAGEMENT -DVKBD_ALWAYS

ifdef CYCLONE_CORE
MORE_CFLAGS += -fno-threadsafe-statics
endif

ifeq ($(DEBUG), YES)
MORE_CFLAGS += -g3
endif

# Common optional features & debug defines
#MORE_CFLAGS += -DSTATUS_ALWAYS
#MORE_CFLAGS += -DUSE_MAYBE_BLIT
#MORE_CFLAGS += -DUSE_BLITTER_DELAYED
#MORE_CFLAGS += -DUSE_BLIT_FUNC
#MORE_CFLAGS += -DUSE_LARGE_BLITFUNC
#MORE_CFLAGS += -DUSE_VAR_BLITSIZE
#MORE_CFLAGS += -DUSE_SHORT_BLITTABLE
#MORE_CFLAGS += -DUSE_RASTER_DRAW
MORE_CFLAGS += -DUSE_ALL_LINES
#MORE_CFLAGS += -DUSE_LINESTATE
#MORE_CFLAGS += -DUSE_DISK_UPDATE_PER_LINE
#MORE_CFLAGS += -DDOUBLEBUFFER
#MORE_CFLAGS += -DUSE_AUTOCONFIG
#MORE_CFLAGS += -DUAE_CONSOLE

MORE_CFLAGS += -DUSE_ZFILE

#MORE_CFLAGS += -DUAE4ALL_NO_USE_RESTRICT

#MORE_CFLAGS += -DNO_SOUND
#MORE_CFLAGS += -DEXACT_AUDIO
#MORE_CFLAGS += -DSOUND_AHI
#MORE_CFLAGS += -DCUT_COPPER
#MORE_CFLAGS += -DEXACT_CURRENT_HPOS
#MORE_CFLAGS += -DUSE_SPECIAL_MEM

#MORE_CFLAGS += -DDEBUG_TIMESLICE

MORE_CFLAGS += -DEMULATED_JOYSTICK
#MORE_CFLAGS += -DFAME_INTERRUPTS_PATCH  # brakes in e.g. "Lost Vikings"
#MORE_CFLAGS += -DFAME_INTERRUPTS_SECURE_PATCH
#MORE_CFLAGS += -DSECURE_BLITTER

#MORE_CFLAGS += -DUAE_MEMORY_ACCESS
#MORE_CFLAGS += -DSAFE_MEMORY_ACCESS
#MORE_CFLAGS += -DERROR_WHEN_MEMORY_OVERRUN

# Debug may not compile for UAE core so disable,
# also it brakes argc parsing see gui_init()
#MORE_CFLAGS += -DDEBUG_UAE4ALL

#MORE_CFLAGS += -DDEBUG_UAE4ALL_FFLUSH
#MORE_CFLAGS += -DDEBUG_M68K
#MORE_CFLAGS += -DDEBUG_INTERRUPTS
#MORE_CFLAGS += -DDEBUG_CYCLES
#MORE_CFLAGS += -DDEBUG_CIA
#MORE_CFLAGS += -DDEBUG_SOUND
#MORE_CFLAGS += -DDEBUG_MEMORY
#MORE_CFLAGS += -DDEBUG_MAPPINGS
#MORE_CFLAGS += -DDEBUG_DISK
#MORE_CFLAGS += -DDEBUG_CUSTOM
#MORE_CFLAGS += -DDEBUG_EVENTS
#MORE_CFLAGS += -DDEBUG_SAVESTATE
#MORE_CFLAGS += -DDEBUG_GFX -DDEBUG_BLITTER
#MORE_CFLAGS += -DDEBUG_FRAMERATE
#MORE_CFLAGS += -DAUTO_FRAMERATE=1400
#MORE_CFLAGS += -DMAX_AUTO_FRAMERATE=4400
#MORE_CFLAGS += -DAUTO_FRAMERATE_SOUND
#MORE_CFLAGS += -DSTART_DEBUG=588
#MORE_CFLAGS += -DMAX_AUTOEVENTS=1856
#MORE_CFLAGS += -DSTART_DEBUG_SAVESTATE
#MORE_CFLAGS += -DAUTO_SAVESTATE=101
#MORE_CFLAGS += -DMAX_AUTOEVENTS=589
#MORE_CFLAGS += -DAUTO_RUN
#MORE_CFLAGS += -DAUTOEVENTS
#MORE_CFLAGS += -DPROFILER_UAE4ALL
#MORE_CFLAGS += -DAUTO_PROFILER=4000
#MORE_CFLAGS += -DMAX_AUTO_PROFILER=5000

CFLAGS   = $(DEFAULT_CFLAGS) $(OPTIMIZE_CFLAGS) $(MORE_CFLAGS)

# ---------------------------------------------------------------------------
# Object files
# ---------------------------------------------------------------------------
OBJS = \
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
	src/writelog.o \
	src/zfile.o \
	src/menu/fade.o \
	src/vkbd/vkbd.o

ifeq ($(LIBRETRO), 1)
OBJS += \
	src/sound_retro.o \
	src/retrogfx.o \
	libretro/core/libretro-core.o \
	libretro/core/core-mapper.o \
	libretro/core/graph.o \
	libretro/core/vkbd.o
else
OBJS += \
	src/od-joy.o \
	src/sdlgfx.o \
	src/dingoo.o \
	src/menu/menu.o \
	src/menu/menu_save.o \
	src/menu/menu_load.o \
	src/menu/menu_df_selection.o \
	src/menu/menu_main.o

ifdef SOUND_NEW
OBJS += src/sound_sdl_new.o
else
OBJS += src/sound.o
endif

ifdef HOME_DIR
CFLAGS += -DHOME_DIR
OBJS   += src/homedir.o
endif
endif  # LIBRETRO / standalone objs

ifdef LIB7Z
CFLAGS += -DUSE_LIB7Z
OBJS   += \
	src/lib7z/7zAlloc.o \
	src/lib7z/7zBuf2.o \
	src/lib7z/7zBuf.o \
	src/lib7z/7zCrc.o \
	src/lib7z/7zDecode.o \
	src/lib7z/7zExtract.o \
	src/lib7z/7zFile.o \
	src/lib7z/7zHeader.o \
	src/lib7z/7zIn.o \
	src/lib7z/7zItem.o \
	src/lib7z/7zStream.o \
	src/lib7z/Alloc.o \
	src/lib7z/Bcj2.o \
	src/lib7z/Bra86.o \
	src/lib7z/BraIA64.o \
	src/lib7z/Bra.o \
	src/lib7z/LzFind.o \
	src/lib7z/LzmaDec.o \
	src/lib7z/LzmaEnc.o \
	src/lib7z/lzma.o
endif

# ---------------------------------------------------------------------------
# M68K core selection
# ---------------------------------------------------------------------------
ifdef FAME_CORE
ifdef FAME_CORE_C
#CFLAGS += -DUSE_FAME_CORE -DUSE_FAME_CORE_C -DFAME_INLINE_LOOP -DFAME_IRQ_CLOCKING -DFAME_CHECK_BRANCHES -DFAME_EMULATE_TRACE -DFAME_DIRECT_MAPPING -DFAME_BYPASS_TAS_WRITEBACK -DFAME_ACCURATE_TIMING -DFAME_GLOBAL_CONTEXT -DFAME_FETCHBITS=8 -DFAME_DATABITS=8 -DFAME_GOTOS -DFAME_EXTRA_INLINE=__inline__ -DFAME_NO_RESTORE_PC_MASKED_BITS
CFLAGS += -DUSE_FAME_CORE -DUSE_FAME_CORE_C -DFAME_IRQ_CLOCKING -DFAME_CHECK_BRANCHES -DFAME_EMULATE_TRACE -DFAME_DIRECT_MAPPING -DFAME_BYPASS_TAS_WRITEBACK -DFAME_ACCURATE_TIMING -DFAME_GLOBAL_CONTEXT -DFAME_FETCHBITS=8 -DFAME_DATABITS=8 -DFAME_NO_RESTORE_PC_MASKED_BITS
src/m68k/fame/famec.o: src/m68k/fame/famec.cpp
OBJS += src/m68k/fame/famec.o
else
CFLAGS += -DUSE_FAME_CORE
src/m68k/fame/fame.o: src/m68k/fame/fame.asm
	nasm -f elf src/m68k/fame/fame.asm
OBJS += src/m68k/fame/fame.o
endif
OBJS += src/m68k/fame/m68k_intrf.o
else  # FAME_CORE
ifdef CYCLONE_CORE
# use all FAME hacks in uae code for Cyclone too
CFLAGS += -DUSE_FAME_CORE
CFLAGS += -DUSE_CYCLONE_CORE
#ASFLAGS += -mfloat-abi=soft -mcpu=arm920t
ifeq ($(platform), miyoo)
ASFLAGS += -mcpu=arm926ej-s
endif
OBJS += src/m68k/cyclone/cyclone.o
OBJS += src/m68k/m68k_cmn_intrf.o
OBJS += src/m68k/cyclone/m68k_intrf.o
CFLAGS += -DUSE_CYCLONE_MEMHANDLERS
OBJS += src/m68k/cyclone/memhandlers.o
else  # UAE core
OBJS += \
	src/m68k/uae/newcpu.o \
	src/m68k/uae/readcpu.o \
	src/m68k/uae/cpudefs.o \
	src/m68k/uae/fpp.o \
	src/m68k/uae/cpustbl.o \
	src/m68k/uae/cpuemu.o
endif  # UAE core
endif  # FAME_CORE

CPPFLAGS = $(CFLAGS)

# ---------------------------------------------------------------------------
# Link rules
# ---------------------------------------------------------------------------
ifeq ($(LIBRETRO), 1)
ifeq ($(STATIC_LINKING), 1)
$(PROG): $(OBJS)
	$(AR) rcs $@ $(OBJS)
else
$(PROG): $(OBJS)
	$(CC) $(SHARED) -o $@ $(OBJS) $(LDFLAGS)
#	$(STRIP) $(PROG)
endif
else  # standalone
$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)
ifneq ($(DEBUG), YES)
	$(STRIP) $(PROG)
endif
endif

# ---------------------------------------------------------------------------
# Convenience targets
# ---------------------------------------------------------------------------
libretro:
	$(MAKE) LIBRETRO=1

ifeq ($(LIBRETRO), 1)
install: $(PROG)
	cp $(PROG) ~/.config/retroarch/cores/
endif

package: $(PROG)
	@mkdir -p $(RELEASEDIR)
	@cp *$(NAME) $(RELEASEDIR)/
	@mkdir -p $(RELEASEDIR)$(INSTALLDIR)/$(DESTDIR)/$(NAME)
	@mkdir -p $(RELEASEDIR)$(INSTALLDIR)/gmenu2x/sections/$(SECTION)
	@mv $(RELEASEDIR)/*$(NAME) $(RELEASEDIR)$(INSTALLDIR)/$(DESTDIR)/$(NAME)/
	@cp -r $(ASSETSDIR)/* $(RELEASEDIR)$(INSTALLDIR)/$(DESTDIR)/$(NAME)
	@cp $(LINK) $(RELEASEDIR)$(INSTALLDIR)/gmenu2x/sections/$(SECTION)
	-@cp $(OPKG_ASSETSDIR)/$(ALIASES) $(RELEASEDIR)$(INSTALLDIR)/$(DESTDIR)/$(NAME)

zip: package
	@cd $(RELEASEDIR) && zip -rq $(NAME)$(VERSION).zip ./* && mv *.zip ..
	@rm -rf $(RELEASEDIR)

ipk: package
	@mkdir -p $(RELEASEDIR)/data
	@mv $(RELEASEDIR)$(INSTALLDIR) $(RELEASEDIR)/data/
	@cp -r $(OPKG_ASSETSDIR)/CONTROL $(RELEASEDIR)
	@sed "s/^Version:.*/Version: $(VERSION)/" $(OPKG_ASSETSDIR)/CONTROL/control > $(RELEASEDIR)/CONTROL/control
	@echo 2.0 > $(RELEASEDIR)/debian-binary
	@tar --owner=0 --group=0 -czvf $(RELEASEDIR)/data.tar.gz -C $(RELEASEDIR)/data/ . >/dev/null 2>&1
	@tar --owner=0 --group=0 -czvf $(RELEASEDIR)/control.tar.gz -C $(RELEASEDIR)/CONTROL/ . >/dev/null 2>&1
	@ar r $(NAME).ipk $(RELEASEDIR)/control.tar.gz $(RELEASEDIR)/data.tar.gz $(RELEASEDIR)/debian-binary
	@rm -rf $(RELEASEDIR)

ifeq ($(platform), miyoo)
gm2xpkg-ipk: $(PROG)
	gm2xpkg -i -f pkg.cfg
endif

opk: $(PROG)
	mkdir -p $(RELEASEDIR)
	cp $(PROG) $(RELEASEDIR)
	cp -R $(DATADIR) $(RELEASEDIR)
	rm $(RELEASEDIR)/$(DATADIR)/music.mod
	rm $(RELEASEDIR)/$(DATADIR)/click.wav
	cp $(OPKDIR)/* $(RELEASEDIR)
	cp -R ./docs/ $(RELEASEDIR)
	mksquashfs $(RELEASEDIR) uae4all.opk -all-root -noappend -no-exports -no-xattrs

almostclean:
	cp src/m68k/fame/famec.o src/m68k/fame/famec.preserved.o
	$(RM) $(PROG) $(OBJS)
	mv src/m68k/fame/famec.preserved.o src/m68k/fame/famec.o

clean:
	$(RM) $(PROG) $(OBJS) $(NAME) $(NAME)_libretro.so $(NAME)_libretro.a
	rm -rf $(RELEASEDIR)
	rm -f *.ipk
	rm -f *.zip

.PHONY: all clean almostclean opk libretro package zip ipk
ifeq ($(LIBRETRO), 1)
.PHONY: install
endif
ifeq ($(platform), miyoo)
.PHONY: gm2xpkg-ipk
endif
