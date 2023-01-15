



                UAE4ALL Amiga emulator for Raspberry Pi



                     Version 0.7 By Chips in 2015




This emulator is based on UAE4All rc3 which itself is an highly optimized (means un-accurate) Amiga emulator.
Audio is emulated, if it doesn't work try to play with Raspberry Pi configuration or use the jack.

Do not set GPU memory size too low in memory split configuration of Raspberry PI otherwise you'll get black screen.

Since PAL amiga is emulated, you will have best result with a 50Hz screen mode on your Raspberry...
In /boot/config.txt, following lines do the trick (1080p 50Hz mode):
hdmi_group=1
hdmi_mode=31

Following parameters are supported:
 -df0 <name of the adf>
 -df1 <name of the adf>
 -kick <name of the kickstart>
 -frameskip <nb of frameskip>

New in v0.7:
- Put back the callback mecanism which re-enable the double buffering since it's corrected in latest Raspberry Firmware. Don't forget to update it if UAE4ALL freeze.
- Add parameters management.


New in v0.6:
- Remove the callback mecanism since on latest Raspberry Pi firmware submit one has a strange behavior... framerate could be worst sometimes..



New in v0.5:

- Menu resolution is increased: now more files are displayed in adf loader and name are longer.
- Add new options in menu: second drive (DF1) adf load and 4/3 or fullscreen display selection.



New in v0.4:

- Now joystick can be used to go back from emulation mode to menu. If you have a joystick with more than 4 buttons, any button except the four first one will trigger the main menu. 



New in v0.3:
- Now Amiga Vsync is synchronized with Raspberry Pi one. It enables perfectly smooth emulation. Since emulated Amiga is Pal, ensure to set Raspberry Pi to 50Hz display mode otherwise graphics jitter will occurs.
- Since Vsync management is completely rewritten, Auto frame skip is less effective than in previous version. However it looks like enough for Rapberry Pi (especially with cyclone core).
- Improved double buffering: CPU is no more blocked on Vsync waiting.
- Menu can be driven using Joystick.



New in v0.2:
- Vsync is activated with a double buffering implementation. The double buffering is simple but efficient: one dispmanX element but two resources are used.
- 2 different binary: one with cyclone core (less accurate but faster), another one with fame core (slower but more accurate)
- Everything is now displayed using dispmanX: no more black screen should occurs in menu due to broken Raspberry Pi SDL implementation.


Original v0.1 status:

- Full Speed A500 achieved (except few rare cases).
- Works in command line.
- Sound is OK.
- Up-scaling is done using Dispmanx.
- This version is not so accurate, so don't blame if some demos aren't working.
- VSync is not activated.
- Sometime SDL fails to display the menu so you got black screen. Just remember how to quit the menu (5 time down then enter) and re-start the emulator.



How to compile:
Enter following command from a terminal:

sudo apt-get update
sudo apt-get install libsdl1.2-dev
sudo apt-get install libsdl-mixer1.2-dev

then make :)



