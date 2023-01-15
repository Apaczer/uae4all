# uae4all for Raspberry Pi

Very fast amiga emulator suitable for low-end devices like raspberry pi one and zero.

Emulate an amiga 500.

Need kick.rom file containing kickstart 1.3

How to compile:

   Retrieve the source of this emulator:

      git clone https://github.com/Chips-fr/uae4all-rpi
      cd uae4all-rpi

   Install following packages:

      sudo apt-get install libsdl1.2-dev 
      sudo apt-get install libsdl-gfx1.2-dev

   Then :

      make

For even faster emulation speed (but lower compatibility) uncomment in makefile CYCLONE_CORE and comment FAME_CORE.

# uae4all-libretro core

How to compile:

     make -f Makefile.libretro

Then copy the core to retroarch core directory:

     cp uae4all_libretro.so ~/.config/retroarch/cores/

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

Right analog stick controls the mouse.

In mouse emulation dpad and fire buttons controls the mouse.

Two joysticks support. Switch automatically between mouse or second joystick when a mouse or 2nd joystick button is pressed.

L & R button can change DF0: current disk for multiple disk roms. Each disk should be named with "(Disk x of y)"

Kickstarts supported:

|System|Version|Filename|Size|MD5|
|---|---|---|---|---|
|A500|KS v1.3 rev 34.005|**kick34005.A500**|262 144|82a21c1890cae844b3df741f2762d48d|