## UAE4ALL
UAE4All is a "Lite" Amiga Emulator. It is based on E-UAE core and it can utilize FAME for Motorola 68000 microprocessor emulation. It emulates Commodore Amiga 500 hardware with 1 MB Chip RAM, OCS and up to 4 floppy drive, utilizes Cyclone emulator of Motorola 68000 microprocessor for ARM hardware. It emulates most Commodore Amiga 500 hardware.

## UAE4ALL for MiyooCFW
Revamped version of UAE4ALL for MiyooCFW, supported devices: Bittboy, PocketGo, Powkiddy V90 / Q90 / Q20 etc.
This build is based of a port to _GCW0_ made by zear & Nebuleon (with smurline and goldmojo changes for other platforms). 

Stock emulator for 1.3.3 MiyooCFW was based (possibly) of upstream by zear (https://github.com/zear/uae4all). Initial (stock) port made by @jamesofarrell, compiled with FAME core (no src available).

This revision replaces FAME mk68 emulation library with UAE core. Major advantages would be wider game compatibility, however some games may produce fps drawback due to UAE being less optimized.

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
- Zool II - less optimized
- Jim Power - less optimized
- Xenon II - crashes after sprite collision with bomb

Below titles will play better on UAE core comparable to FAME:
- SuperFrog - no video jittery on "auto" frameskip
- Alien Breed - no video jittery on "auto" frameskip
- JamesPond - boot ups and doesn't crash
- Great Giana Sisters - boot ups and doesn't crash
- Nicky II - boot ups and doesn't crash
- AdamsFamily - no sprite freezes during gameplay