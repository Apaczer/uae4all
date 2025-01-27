## UAE4ALL Libretro

This is downstream fork of https://github.com/Chips-fr/uae4all-rpi aimed at Libretro implementation of a "Lite" Amiga Emulator. It is based on E-UAE core and it can utilize FAME (no Cyclone or UAE currently supported in ARMv5) for Motorola 68000 microprocessor emulation. It emulates most Commodore Amiga 500 hardware with OSC 1MB Chip.

## Native build

```
no instructions available
```

## Cross-Compiling instructions

1. Clone this repo & set up your environment with docker:
```
git clone -b libretro --single-branch https://github.com/Apaczer/uae4all
docker run --volume ./:/src/ -it miyoocfw/toolchain-shared-uclibc:latest
cd /src
```
2. Compile `uae4all_libretro.so` core
``` 
make -j$(nproc) -f Makefile.miyoo platform=miyoo
```

## Controls

|RetroPad button|Action|
|---|---|
|B|Fire button 1 / Red|
|A|Fire button 2 / Blue|
|L2|Left mouse button|
|R2|Right mouse button|
|L|Switch to previous disk|
|R|Switch to next disk|
|Select|Toggle virtual keyboard|
|Start|Toggle mouse emulation|

~~Right analog stick controls the mouse.~~

In mouse emulation dpad and fire buttons controls the mouse.

Two joysticks support. Switch automatically between mouse or second joystick when a mouse or 2nd joystick button is pressed.

L & R button can change DF0: current disk for multiple disk roms. Each disk should be named with "(Disk x of y)"

Kickstarts supported:

|System|Version|Filename|Size|MD5|
|---|---|---|---|---|
|A500|KS v1.3 rev 34.005|**kick34005.A500**|262 144|82a21c1890cae844b3df741f2762d48d|