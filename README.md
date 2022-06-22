## UAE4ALL
UAE4All is a "Lite" Amiga Emulator. It is based on E-UAE core and it can utilize FAME for Motorola 68000 microprocessor emulation. It emulates Commodore Amiga 500 hardware with 1 MB Chip RAM, OCS and up to 4 floppy drive, utilizes Cyclone emulator of Motorola 68000 microprocessor for ARM hardware. It emulates most Commodore Amiga 500 hardware.

## UAE4ALL for MiyooCFW
Revamped version of UAE4ALL for MiyooCFW, supported devices: Bittboy, PocketGo, Powkiddy V90 / Q90 / Q20 etc.
This build is based of a port to _GCW0_ made by zear & Nebuleon (with smurline and goldmojo changes for other platforms). 

Stock emulator for 1.3.3 MiyooCFW was based (possibly) of upstream by zear (https://github.com/zear/uae4all). Initial (stock) port made by @jamesofarrell, compiled with FAME core (no src available).

This revision replaces FAME m68k emulation library with UAE core. Major advantages would be wider game compatibility, however some games may produce fps drawback due to UAE being less optimized.

### Changelog:
**UAE4ALL rev. 1.1**
- the "Status bar" option is now visible in main menu and uae4all.cfg file ( ``STATUS_BAR 0`` or ``-1``)
- added hotkeys to increase/decrease "Throttle" (SELECT+X/Y)

**UAE4ALL rev. 1.0**
- load DF0 as first arg. parameter (you can load ROM file directly from your console's frontend)
- show/hide status bar option (editable through *.cfg or in emu's GUI)
- disabled splash screen by default (faster init load- show/hide status bar option (editable through *.cfg or in emu's GUI)
- disabled splash screen by default (faster init load speed)
- semi-darkmode for all notifications due to above commit (unintentional, but I like it!)
- fixed A/B button mapping for V90/Q90 while in menu screen (to be cohesive with gmenu2x/SM).
- frame throttling fine tuning now works on "auto" frameskip mode
- lib7z savestate compression method (will not work with jamesofarrell's stock version)
- compatibility increase (revert from FAME m68k library to UAE core, may introduce slight performance drawback) speed)
- semi-darkmode for all notifications due to above commit (unintentional, but I like it!)
- fixed A/B button mapping for V90/Q90 while in menu screen (to be cohesive with gmenu2x/SM).
- frame throttling fine tuning now works on "auto" frameskip mode
- lib7z savestate compression method (will not work with jamesofarrell's stock version)
- compatibility increase (revert from FAME m68k library to UAE core, may introduce slight performance drawback)

## Compiling instructions
1. Set up your environment with @steward-fu toolchain (preferably in Debian 9 distro): 
```
$ cd
$ wget https://github.com/steward-fu/miyoo/releases/download/v1.0/toolchain.7z
$ 7za x toolchain.7z
$ sudo cp -a miyoo /opt/
```
2. Copy this repo & compile
``` 
git clone https://github.com/Apaczer/uae4all
cd uae4all
make clean
make -f Makefile.miyoo
```
## PGO instructions :
All builds (from release section) have Profile-Guided Optimization applied, which gives approx 5-10% performance boost.
- set ``PROFILE = YES`` in Makefile
- compile
- run the emulator for a few minutes on your device 
- dump the *.gcda files dropped in ``/mnt/profile`` and copy them back into your ``/src`` folder in a repo
- set ``PROFILE = APPLY``
- compile & enjoy optimized build!

While gathering profiling data on target platform, use the app how normally you would and look for parts that might introduce perf. challenge. Possibly try to run emulator in one take and exit normally through GUI's menu to correctly collect information.

## Compatibility list
There are some drawbacks when FAME is not used:
- _Zool II_ - less optimized
- _Jim Power in Mutant Planet_ - less optimized
- _Xenon 2: Megablast_ - crashes after sprite collision with bomb

Below titles will play better on UAE core comparable to FAME:
- _Superfrog_ - no video jittery on "auto" frameskip
- _Alien Breed_ - no video jittery on "auto" frameskip
- _James Pond_ - boot ups and doesn't crash (CR PDX by TIC)
- _The Great Giana Sisters_ - boot ups and doesn't crash
- _Nicky II_ - boot ups and doesn't crash
- _The Addams Family Mansion Mayhem_ - no sprite freezes during gameplay
- _Moonstone: A Hard Days Knight_ - passes loading screen when disk B is being read on DF1 drive

UAE4ALL emulation related issues:
- _James Pond_ - player's sprite can pass through walls
- _Moonstone: A Hard Days Knight_ - will play only with first (DF0) and second (DF1) disk mounted simultaneously

## FAQ
Q) _What's the difference between stock UAE4ALL (from 1.3.3 MiyooCFW)?_  
A: This revision is more compatible and have additional user-friendly features described in changelog. The previous src was also unavailable, so this port is a chance for other people to work of it and modify this emulator to their needs. 

Q) _How to set it up in console?_  
A: Place binary file in _/mnt/emus/uae4all/_ and make new link to it if you don't have one already. You will also need to have there _/data_ folder with necessary [assets](https://github.com/Apaczer/uae4all/tree/master/data).

Q) _Does this emu need any BIOS file?_  
A: Yes, the emulator will recognize it by placing kick.rom (rename your kickstarter BIOS to match) in _/mnt/.uae4all/_ location