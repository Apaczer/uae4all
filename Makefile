TARGET			= uae4all
VERSION			?= $(shell date +%Y-%m-%d\ %H:%M)
ASSETSDIR		= assets
OPKG_ASSETSDIR	= opkg_assets
LINK			= $(TARGET).lnk
DESTDIR			= emus
SECTION			= emulators
ALIASES			= aliases.txt

NAME	= $(TARGET)
O		= o
RM 		= rm -f

PROG		= $(NAME)
RELEASEDIR	= package
DATADIR		= data
OPKDIR		= opk_data

ifneq ($(LINUX), YES)
CHAINPREFIX ?= /opt/miyoo
CROSS_COMPILE ?= $(CHAINPREFIX)/usr/bin/arm-linux-
endif

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip

SYSROOT		:= $(shell $(CC) --print-sysroot)

PKGS		= sdl SDL_image zlib SDL_mixer
PKGS_CFLAGS	= $(shell $(SYSROOT)/../../usr/bin/pkg-config --cflags $(PKGS))
PKGS_LIBS	= $(shell $(SYSROOT)/../../usr/bin/pkg-config --libs $(PKGS))

all: $(PROG)

# Possible values : 0, YES, APPLY
PROFILE ?= 0

## either FAME or UAE
#FAME_CORE=1
## use 7z savestate compression for *.asf files
LIB7Z=1
HOME_DIR=1

DEFAULT_CFLAGS = $(PKGS_CFLAGS) -DMIYOO
LDFLAGS = $(PKGS_LIBS)

ifndef FAME_CORE
LDFLAGS += -lm
endif

ifeq ($(PROFILE), YES)
LDFLAGS		+= -lgcov
endif

MORE_CFLAGS = -Isrc/ -Isrc/include/ -Isrc/menu -Isrc/vkbd

MORE_CFLAGS += -DUSE_SDL -DDOUBLEBUFFER -DNO_DEFAULT_THROTTLE -DUNALIGNED_PROFITABLE -DREGPARAM="__attribute__((regparm(3)))" -D__inline__=__inline__
MORE_CFLAGS += -DOS_WITHOUT_MEMORY_MANAGEMENT -DVKBD_ALWAYS
MORE_CFLAGS += -DROM_PATH_PREFIX=\"./\" -DSAVE_PREFIX=\"./\"
ifeq ($(LINUX), YES)
MORE_CFLAGS += -DDATA_PREFIX=\"./assets/data/\"
MORE_CFLAGS += -DCWD_MENU_DIR
else
MORE_CFLAGS += -DDATA_PREFIX=\"./data/\"
endif

ifeq ($(DEBUG),YES)
MORE_CFLAGS += -g3
endif

OPTIMIZE_CFLAGS = -Ofast -fno-exceptions -fno-rtti -fomit-frame-pointer -Wno-unused -Wno-format

ifeq ($(PROFILE), YES)
OPTIMIZE_CFLAGS		+= -fprofile-generate=/mnt/profile
else ifeq ($(PROFILE), APPLY)
OPTIMIZE_CFLAGS		+= -fprofile-use=profile -fbranch-probabilities
OPTIMIZE_CFLAGS		+= -flto
endif

#MORE_CFLAGS+= -DSTATUS_ALWAYS
#MORE_CFLAGS+= -DUSE_MAYBE_BLIT
#MORE_CFLAGS+= -DUSE_BLITTER_DELAYED
#MORE_CFLAGS+= -DUSE_BLIT_FUNC
#MORE_CFLAGS+= -DUSE_LARGE_BLITFUNC
#MORE_CFLAGS+= -DUSE_VAR_BLITSIZE
#MORE_CFLAGS+= -DUSE_SHORT_BLITTABLE
MORE_CFLAGS+= -DUSE_BLIT_MASKTABLE
#MORE_CFLAGS+= -DUSE_RASTER_DRAW
#MORE_CFLAGS+= -DUSE_ALL_LINES
#MORE_CFLAGS+= -DUSE_LINESTATE
#MORE_CFLAGS+= -DUSE_DISK_UPDATE_PER_LINE
MORE_CFLAGS+= -DMENU_MUSIC
#MORE_CFLAGS+= -DUSE_AUTOCONFIG
#MORE_CFLAGS+= -DUAE_CONSOLE

MORE_CFLAGS+= -DUSE_ZFILE

#MORE_CFLAGS+= -DUAE4ALL_NO_USE_RESTRICT

#MORE_CFLAGS+= -DNO_SOUND
MORE_CFLAGS+= -DNO_THREADS

#MORE_CFLAGS+= -DDEBUG_TIMESLICE

MORE_CFLAGS+= -DEMULATED_JOYSTICK
#MORE_CFLAGS+= -DFAME_INTERRUPTS_PATCH
#MORE_CFLAGS+= -DFAME_INTERRUPTS_SECURE_PATCH
#MORE_CFLAGS+= -DSECURE_BLITTER

#MORE_CFLAGS+= -DUAE_MEMORY_ACCESS
#MORE_CFLAGS+= -DSAFE_MEMORY_ACCESS
#MORE_CFLAGS+= -DERROR_WHEN_MEMORY_OVERRUN

#MORE_CFLAGS+= -DDEBUG_UAE4ALL
#MORE_CFLAGS+= -DDEBUG_UAE4ALL_FFLUSH
#MORE_CFLAGS+= -DDEBUG_M68K
#MORE_CFLAGS+= -DDEBUG_INTERRUPTS
##MORE_CFLAGS+= -DDEBUG_CIA
#MORE_CFLAGS+= -DDEBUG_SOUND
#MORE_CFLAGS+= -DDEBUG_MEMORY
###MORE_CFLAGS+= -DDEBUG_MAPPINGS
##MORE_CFLAGS+= -DDEBUG_DISK
##MORE_CFLAGS+= -DDEBUG_CUSTOM
###MORE_CFLAGS+= -DDEBUG_EVENTS
###MORE_CFLAGS+= -DDEBUG_GFX -DDEBUG_BLITTER
#MORE_CFLAGS+= -DDEBUG_FRAMERATE
#MORE_CFLAGS+= -DAUTO_FRAMERATE=1400
#MORE_CFLAGS+= -DMAX_AUTO_FRAMERATE=4400
###MORE_CFLAGS+= -DAUTO_FRAMERATE_SOUND
##MORE_CFLAGS+= -DSTART_DEBUG=11554
##MORE_CFLAGS+= -DMAX_AUTOEVENTS=11560
#MORE_CFLAGS+= -DSTART_DEBUG=11554
#MORE_CFLAGS+= -DMAX_AUTOEVENTS=5000
#MORE_CFLAGS+= -DAUTO_RUN
#MORE_CFLAGS+= -DAUTOEVENTS
#MORE_CFLAGS+= -DPROFILER_UAE4ALL
#MORE_CFLAGS+= -DAUTO_PROFILER=4000
#MORE_CFLAGS+= -DMAX_AUTO_PROFILER=5000

#MORE_CFLAGS+= -DPROFILER_UAE4ALL

ifneq ($(LINUX),YES)
MORE_CFLAGS+= -DUSE_CAST_UNSIGNED
endif

CFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) $(OPTIMIZE_CFLAGS)

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
	src/menu/menu_df_selection.o \
	src/menu/menu_main.o \
	src/vkbd/vkbd.o \
	src/dingoo.o \

ifdef LIB7Z
CFLAGS+=-DUSE_LIB7Z
OBJS+= \
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

ifdef HOME_DIR
CFLAGS+=-DHOME_DIR
OBJS+= \
	src/homedir.o
endif

ifdef FAME_CORE
CFLAGS+=-DUSE_FAME_CORE -DUSE_FAME_CORE_C -DFAME_IRQ_CLOCKING -DFAME_CHECK_BRANCHES -DFAME_EMULATE_TRACE -DFAME_DIRECT_MAPPING -DFAME_BYPASS_TAS_WRITEBACK -DFAME_ACCURATE_TIMING -DFAME_GLOBAL_CONTEXT -DFAME_FETCHBITS=8 -DFAME_DATABITS=8 -DFAME_GOTOS -DFAME_EXTRA_INLINE=__inline__ -DINLINE=__inline__ -DFAME_NO_RESTORE_PC_MASKED_BITS
#CFLAGS+-DFAME_INLINE_LOOP
src/m68k/fame/famec.o: src/m68k/fame/famec.cpp
OBJS += src/m68k/fame/famec.o src/m68k/fame/m68k_intrf.o
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
ifneq ($(DEBUG),YES)
	$(STRIP) $(PROG)
endif

run: $(PROG)
	./$(PROG)

package: $(PROG)
	@mkdir -p $(RELEASEDIR)
	@cp *$(TARGET) $(RELEASEDIR)/
	@mkdir -p $(RELEASEDIR)/mnt/$(DESTDIR)/$(TARGET)
	@mkdir -p $(RELEASEDIR)/mnt/gmenu2x/sections/$(SECTION)
	@mv $(RELEASEDIR)/*$(TARGET) $(RELEASEDIR)/mnt/$(DESTDIR)/$(TARGET)/
	@cp -r $(ASSETSDIR)/* $(RELEASEDIR)/mnt/$(DESTDIR)/$(TARGET)
	@cp $(LINK) $(RELEASEDIR)/mnt/gmenu2x/sections/$(SECTION)
	-@cp $(OPKG_ASSETSDIR)/$(ALIASES) $(RELEASEDIR)/mnt/$(DESTDIR)/$(TARGET)

zip: package
	@cd $(RELEASEDIR) && zip -rq $(TARGET)$(VERSION).zip ./* && mv *.zip ..
	@rm -rf $(RELEASEDIR)

ipk: package
	@mkdir -p $(RELEASEDIR)/data
	@mv $(RELEASEDIR)/mnt $(RELEASEDIR)/data/
	@cp -r $(OPKG_ASSETSDIR)/CONTROL $(RELEASEDIR)
	@sed "s/^Version:.*/Version: $(VERSION)/" $(OPKG_ASSETSDIR)/CONTROL/control > $(RELEASEDIR)/CONTROL/control
	@echo 2.0 > $(RELEASEDIR)/debian-binary
	@tar --owner=0 --group=0 -czvf $(RELEASEDIR)/data.tar.gz -C $(RELEASEDIR)/data/ . >/dev/null 2>&1
	@tar --owner=0 --group=0 -czvf $(RELEASEDIR)/control.tar.gz -C $(RELEASEDIR)/CONTROL/ . >/dev/null 2>&1
	@ar r $(TARGET).ipk $(RELEASEDIR)/control.tar.gz $(RELEASEDIR)/data.tar.gz $(RELEASEDIR)/debian-binary
	@rm -rf $(RELEASEDIR)

gm2xpkg-ipk: $(PROG)
	gm2xpkg -i -f pkg.cfg

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
	$(RM) $(PROG) $(OBJS)
	rm -rf $(RELEASEDIR)
	rm -f *.ipk
	rm -f *.zip
