#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "menu.h"

#include <sys/stat.h>
#include <unistd.h>
#include<dirent.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "uae.h"
#include "options.h"
#include "sound.h"
#include <SDL.h>
#include "gp2x.h"
#include <SDL_ttf.h>
static char *text_str_status_line="Status ln";

char *text_str_misc_separator="----------------------------------";
char *text_str_misc_title=    "            Miscellanous         -";
int menuMisc = 0;
extern int mainMenu_ntsc;
extern int mainMenu_useExternalDrive;
extern int mainMenu_useOneMegaChip;
extern int mainMenu_joyConf;
extern int mainMenu_autosave;
extern int mainMenu_autofire;
extern int mainMenu_showStatus;
extern int skipintro;
extern int enableHWscaling;

extern int mainMenu_enableScreenshots;
extern int mainMenu_enableScripts;
extern int nr_drives;
extern int kickstart;
extern SDL_Joystick *uae4all_joy0;

static void draw_miscMenu(int c)
{
	int leftMargin=3;
	int tabstop1 = 17;
	int tabstop2 = 19;
	int tabstop3 = 21;
	int tabstop4 = 23;
	int tabstop5 = 25;
	int tabstop6 = 27;
	int tabstop7 = 29;
	int tabstop8 = 31;
	int tabstop9 = 33;

	int menuLine = 3;
	static int b=0;
	int bb=(b%6)/3;
	SDL_Rect r;
	extern SDL_Surface *text_screen;
	r.x=80-64; r.y=0; r.w=110+64+64; r.h=240;

	text_draw_background();
	text_draw_window(2,2,40,30,text_str_misc_title);

	menuLine = +3;
	write_text(3, menuLine, "Misc Options");
	menuLine++;
	write_text(3, menuLine, text_str_misc_separator);
	
	menuLine+=2;
	if (menuMisc == 0 && bb)
		write_text_inv(3, menuLine, "Return to main menu");
	else
		write_text(3, menuLine, "Return to main menu");

	menuLine+=2;
	write_text(leftMargin,menuLine,"Video Mode");
	if ((!mainMenu_ntsc)&&((menuMisc!=1)||(bb)))
		write_text_inv(tabstop1,menuLine,"Pal");
	else
		write_text(tabstop1,menuLine,"Pal");

	if ((mainMenu_ntsc)&&((menuMisc!=1)||(bb)))
		write_text_inv(tabstop3,menuLine,"Ntsc");
	else
		write_text(tabstop3,menuLine,"Ntsc");


	menuLine+=2;
	write_text(leftMargin,menuLine,"Auto Save");
	if ((!mainMenu_autosave)&&((menuMisc!=2)||(bb)))
		write_text_inv(tabstop1,menuLine,"On");
	else
		write_text(tabstop1,menuLine,"On");

	if ((mainMenu_autosave)&&((menuMisc!=2)||(bb)))
		write_text_inv(tabstop3,menuLine,"Off");
	else
		write_text(tabstop3,menuLine,"Off");


	menuLine+=2;

	write_text(leftMargin,menuLine,"Memory Chip Size");
	if ((mainMenu_useOneMegaChip)&&((menuMisc!=3)||(bb)))
		write_text_inv(tabstop3,menuLine,"1MB");
	else
		write_text(tabstop3,menuLine,"1MB");

	if ((!mainMenu_useOneMegaChip)&&((menuMisc!=3)||(bb)))
		write_text_inv(tabstop6,menuLine,"512KB");
	else
		write_text(tabstop6,menuLine,"512KB");

	menuLine+=2;

	write_text(leftMargin,menuLine,"Number of drives:");
	
	if ((nr_drives==1)&&((menuMisc!=4)||(bb)))
		write_text_inv(tabstop3,menuLine,"1");
	else
		write_text(tabstop3,menuLine,"1");

	if ((nr_drives==2)&&((menuMisc!=4)||(bb)))
		write_text_inv(tabstop4,menuLine,"2");
	else
		write_text(tabstop4,menuLine,"2");

	if ((nr_drives==3)&&((menuMisc!=4)||(bb)))
		write_text_inv(tabstop5,menuLine,"3");
	else
		write_text(tabstop5,menuLine,"3");

	if ((nr_drives==4)&&((menuMisc!=4)||(bb)))
		write_text_inv(tabstop6,menuLine,"4");
	else
		write_text(tabstop6,menuLine,"4");

	menuLine+=2;

	write_text(leftMargin,menuLine,"Use Autofire");
	if ((mainMenu_autofire)&&((menuMisc!=5)||(bb)))
		write_text_inv(tabstop5,menuLine,"Yes");
	else
		write_text(tabstop5,menuLine,"Yes");

	if ((!mainMenu_autofire)&&((menuMisc!=5)||(bb)))
		write_text_inv(tabstop7,menuLine,"No");
	else
		write_text(tabstop7,menuLine,"No");

	menuLine+=2;
	write_text(leftMargin, menuLine,text_str_status_line);
	if ((!mainMenu_showStatus)&&((menuMisc!=6)||(bb)))
		write_text_inv(tabstop1,menuLine, "Off");
	else
		write_text(tabstop1, menuLine, "Off");
	if ((mainMenu_showStatus)&&((menuMisc!=6)||(bb)))
		write_text_inv(tabstop3, menuLine,"On");
	else
		write_text(tabstop3, menuLine,"On");

	menuLine+=2;

	write_text(leftMargin,menuLine,"Enable Screenshot");
	if ((mainMenu_enableScreenshots)&&((menuMisc!=7)||(bb)))
		write_text_inv(tabstop5,menuLine,"Yes");
	else
		write_text(tabstop5,menuLine,"Yes");

	if ((!mainMenu_enableScreenshots)&&((menuMisc!=7)||(bb)))
		write_text_inv(tabstop7,menuLine,"No");
	else
		write_text(tabstop7,menuLine,"No");

	menuLine+=2;

	write_text(leftMargin,menuLine,"Enable scripts");
	if ((mainMenu_enableScripts)&&((menuMisc!=8)||(bb)))
		write_text_inv(tabstop5,menuLine,"Yes");
	else
		write_text(tabstop5,menuLine,"Yes");

	if ((!mainMenu_enableScripts)&&((menuMisc!=8)||(bb)))
		write_text_inv(tabstop7,menuLine,"No");
	else
		write_text(tabstop7,menuLine,"No");

	


	menuLine+=2;

	write_text(leftMargin,menuLine,"Skip Intro");
	if ((skipintro)&&((menuMisc!=9)||(bb)))
		write_text_inv(tabstop5,menuLine,"Yes");
	else
		write_text(tabstop5,menuLine,"Yes");

	if ((!skipintro)&&((menuMisc!=9)||(bb)))
		write_text_inv(tabstop7,menuLine,"No");
	else
		write_text(tabstop7,menuLine,"No");



	menuLine+=2;

	write_text(leftMargin,menuLine,"Kickstart");
	if ((kickstart==1)&&((menuMisc!=10)||(bb)))
		write_text_inv(tabstop3,menuLine,"1.3");
	else
		write_text(tabstop3,menuLine,"1.3");

	if ((kickstart==2)&&((menuMisc!=10)||(bb)))
		write_text_inv(tabstop5,menuLine,"2.0");
	else
		write_text(tabstop5,menuLine,"2.0");

	if ((kickstart==3)&&((menuMisc!=10)||(bb)))
		write_text_inv(tabstop7,menuLine,"3.1");
	else
		write_text(tabstop7,menuLine,"3.1");


		menuLine+=2;

	write_text(leftMargin,menuLine,"Enable HW scaling");
	if ((enableHWscaling)&&((menuMisc!=11)||(bb)))
		write_text_inv(tabstop5,menuLine,"Yes");
	else
		write_text(tabstop5,menuLine,"Yes");

	if ((!enableHWscaling)&&((menuMisc!=11)||(bb)))
		write_text_inv(tabstop7,menuLine,"No");
	else
		write_text(tabstop7,menuLine,"No");

	menuLine+=2;

	write_text(leftMargin,menuLine,"Control config");
	if ((mainMenu_joyConf==0)&&((menuMisc!=12)||(bb)))
		write_text_inv(tabstop5,menuLine,"0");
	else
		write_text(tabstop5,menuLine,"0");

	if ((mainMenu_joyConf==1)&&((menuMisc!=12)||(bb)))
		write_text_inv(tabstop6,menuLine,"1");
	else
		write_text(tabstop6,menuLine,"1");

	if ((mainMenu_joyConf==2)&&((menuMisc!=12)||(bb)))
		write_text_inv(tabstop7,menuLine,"2");
	else
		write_text(tabstop7,menuLine,"2");

	
	menuLine+=2;
	if (mainMenu_joyConf==0) write_text(5,menuLine,"Standard buttons");
	else if (mainMenu_joyConf==1) write_text(5,menuLine,"Jump=X - Button 1=B");
	else if (mainMenu_joyConf==2) write_text(5,menuLine,"Jump=B - Button 1=X");
	text_flip();
	b++;
}

static int key_miscMenu(int *c)
{
	int end=0;
	static int delay=0;
	int left=0, right=0, up=0, down=0, hit0=0, hit1=0;
	delay ++;
	if (delay<5) return end;
	delay=0;
	SDL_Joystick *joy = uae4all_joy0 ;
	SDL_JoystickUpdate ();
	if (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT)) right=1;
	else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT)) left=1;
	else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_UP)) up=1;
	else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_DOWN)) down=1;
	else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_B)) hit0=1;
	else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_START)) {printf("Pressed Start\n");hit1=1;}
	else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_X)) hit0=1;

/*
	SDL_Event event;


	while (SDL_PollEvent(&event) > 0)
	{
		if (event.type == SDL_QUIT)
			end=-1;
		else if (event.type == SDL_JOYBUTTONDOWN)
		{
             switch (event.jbutton.button)
             {
                 case GP2X_BUTTON_UP: up=1; break;
				 case GP2X_BUTTON_LEFT: left=1; break;
				 case GP2X_BUTTON_RIGHT: right=1; break;
                 case GP2X_BUTTON_DOWN: down=1; break;
                 case GP2X_BUTTON_X: hit0=1; break;
				case GP2X_BUTTON_B: hit0=1; break;
				 case GP2X_BUTTON_START: hit1=1; break;
	          }
        }
*/
		if (hit0)
		{
			end =-1;
		}
		else if (hit1)
		{
			end=-1;
		}
		else if (up)
		{
			if (menuMisc==0) menuMisc=12;
			else menuMisc--;
		}
		else if (down)
		{
			if (menuMisc==12) menuMisc=0;
			else menuMisc++;	
		}
		switch (menuMisc)
		{
			case 1:
				if ((left)||(right))
				{
						mainMenu_ntsc = !mainMenu_ntsc;
					
				}
				break;
			case 2:
				if ((left)||(right))
				{
						mainMenu_autosave = (mainMenu_autosave == 0) ? 1 : 0;
				}
				break;
			case 3:
				if ((left)||(right))
				{
					mainMenu_useOneMegaChip = !mainMenu_useOneMegaChip;
						
				}
				break;

			case 4:
				if (left)
				{
					if (nr_drives>1)
						nr_drives--;
					else
						nr_drives=4;
				}
				else if (right)
				{
					if (nr_drives<4)
						nr_drives++;
					else
						nr_drives=1;
				}	
				break;
			case 5:
				if ((left)||(right))
				{
					mainMenu_autofire = !mainMenu_autofire;
				}
				break;
			case 6:
				{
					if ((left)||(right))
						mainMenu_showStatus=!mainMenu_showStatus;
				}   break;
			case 7:
				{
					if ((left)||(right))
						mainMenu_enableScreenshots=!mainMenu_enableScreenshots;
				}   break;
			case 8:
				{
					if ((left)||(right))
						mainMenu_enableScripts=!mainMenu_enableScripts;
				}   break;
			case 9:
				{
					if ((left)||(right))
						skipintro=!skipintro;
				}   break;
				case 10:
				if (left)
				{
					if (kickstart>1)
						kickstart--;
					else
						kickstart=3;
				}
				else if (right)
				{
					if (kickstart<3)
						kickstart++;
					else
						kickstart=1;
				}	break;
		
				case 11:
				{
					if ((left)||(right))
						enableHWscaling=!enableHWscaling;
				}   break;
			
			case 12:
				if (left)
				{
					if (mainMenu_joyConf>0)
						mainMenu_joyConf--;
					else
						mainMenu_joyConf=2;
				}
				else if (right)
				{
					if (mainMenu_joyConf<2)
						mainMenu_joyConf++;
					else
						mainMenu_joyConf=0;
				}	break;
				
		}
	//}

	return end;
}

static void raise_miscMenu()
{
	int i;

	text_draw_background();
	text_flip();
	for(i=0;i<10;i++)
	{
		text_draw_background();
		text_draw_window(80-64,(10-i)*24,160+64+64,220,text_str_misc_title);
		text_flip();
	}
}

static void unraise_miscMenu()
{
	int i;

	for(i=9;i>=0;i--)
	{
		text_draw_background();
		text_draw_window(80-64,(10-i)*24,160+64+64,220,text_str_misc_title);
		text_flip();
	}
	text_draw_background();
	text_flip();
}
int run_menuMisc()
{
	SDL_Event event;
	SDL_Delay(150);
	while(SDL_PollEvent(&event))
		SDL_Delay(10);
	int end=0,c=0;
	raise_miscMenu();
	while(!end)
	{
		draw_miscMenu(c);
		end=key_miscMenu(&c);
	}
	set_joyConf();
	unraise_miscMenu();
	return end;
}
