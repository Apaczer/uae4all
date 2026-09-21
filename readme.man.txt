[About]

UAE4ALL is an Amiga 500 emulator based on UAE 0.8.22 core.

[Controls] (during emulation):
D-PAD - control amiga joystick (if mouse emulation turned off) / amiga mouse (if mouse emulation turned on)
A - joystick button #1
B - joystick button #2 (fire button)
Y - mouse button #1 (left)
X - mouse button #2 (right)
L - enable/disable mouse emulation (when using mouse press R to change the mouse speed)
R - enable/disable on-screen-keyboard ( use the B button to press a key; A, X & Y can be used to map a key to, return to the main menu to reset keymaps).
HOME - emulator menu
START - SuperThrottle on/off
SELECT + R - Quick load state
SELECT + L - Quick save state
SELECT + Y - increase Throttle
SELECT + X - decrease Throttle

[Menu/CFG Options]
"Throttle" - set optimization level (may brake emualtion)
"Frameskip" - adjust frameskipping for better performance (see Usage)
"Save disks" - write/read savedisk's "temporary" save data to/from <8hex>.ADS compressed file (*)
"Screen pos" - adjust screen vertically
"Use analog" - use real hardware joystick inputs (if connected)
"Status bar" - show/hide lower status bar
"2MB RAM" - increase Chip default RAM (512kB) to 2MB

*) the compressed savedata filename e.g. "6E22D1ED.ads" can be represended with cmd: 
    printf "%s%08X.ads\n" "$SAVE_PREFIX" "$crc"
    SAVE_PREFIX=<CWD of savedisk>
    crc=<8-HEX savedisk's custom uae4all_checksum>

[Usage]
Before first use:
Provide a valid Amiga 500 Kickstart bootstrap firmware and copy it to the following location:
/mnt/.uae4all/kick.rom
UAE4ALL will refuse to launch without this file present.

Save disk:
The program by design stores save data in virtual memory until closing emulator.
To preserve and use savedisk's data for future use, keep enabled "Save disks" feature in menu,
which will store/read memory in/from separate ADS file, however can severy slow down save process.
It is advised to use a pre-formatted for specific game [save disk], as using plain blank ADF file 
may not always work for above "Save disks" method.

Launching programs via menu:
UAE4ALL will work with disk images in "adf" and "adz" format.
1) Copy appropriate files to a location of your choice in the GCW Zero filesystem
2) Launch UAE4ALL emulator
3) Select "Load disk image" option
4) Choose "Load DF0 image" (Floppy Drive #1) or "Load DF1 image" (Floppy Drive #2) to load a disk image to the appropriate drive.
NOTE: Bootable disks need to be loaded to DF0.
5) Point the file selector to a directory location with the disk images and select the image you want to load.
NOTE: This directory location will be stored as a default directory for future UAE4ALL runs.
6) For bootable images, select "Start Amiga" or "Reset Amiga" to boot a new image. For non-bootable images, select "Return to Amiga" and follow instructions of the currently emulated program.

Adjusting the frame rate:
Setting the frameskip value to "auto" will limit the emulated program speed to 50 frames per second. Use this option for programs meant to be run on PAL Amiga machines. Most Amiga programs were designed for this frame rate.
Setting the frameskip value to "0" will limit the emulated program speed to 60 frames per second. Use this option for programs meant to be run on NTSC Amiga machines.
