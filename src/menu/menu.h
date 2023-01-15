#include<stdio.h>
#include<SDL.h>

extern SDL_Surface *prSDLScreen;

#define MENU_FILE_SPLASH DATA_PREFIX "splash.bmp"
#define MENU_FILE_BACKGROUND DATA_PREFIX "background.bmp"
#define MENU_FILE_WINDOW DATA_PREFIX "window.bmp"
#define MENU_FILE_TEXT DATA_PREFIX "text.bmp"
#ifdef DREAMCAST
#ifdef AUTO_RUN
#define MENU_DIR_DEFAULT "/cd/"
#else
#define MENU_DIR_DEFAULT "/"
#endif
#else
#define MENU_DIR_DEFAULT "."
#endif


//#define MENU_GFX_WIDTH    464
//#define MENU_GFX_HEIGHT   304

// Menu resolution

#define MENU_GFX_WIDTH    480
#define MENU_GFX_HEIGHT   352

// Main menu position definition

#define MENU_BORDER_LEFT  110
#define MENU_BORDER_RIGHT 210
#define MENU_BORDER_UP    40
#define MENU_BORDER_DOWN  80

#define MENU_FIRST_CHAR   14

// Load menu position definition

#define MENU_LOAD_BORDER_LEFT  20
#define MENU_LOAD_BORDER_RIGHT 20
#define MENU_LOAD_BORDER_UP    20
#define MENU_LOAD_BORDER_DOWN  40

#define SHOW_MAX_FILES 19

#define MAX_FILELEN 52

#define MENU_LOAD_STARTING_LINE  2


void text_draw_background();
void init_text(int splash);
void quit_text(void);
void write_text(int x, int y, char * str);
void write_text_inv(int x, int y, char * str);
void write_text_inv_n(int x, int y, int n, char * str);
void write_centered_text(int y, char * str);
void write_num(int x, int y, int v);
void write_num_inv(int x, int y, int v);
void text_draw_window(int x, int y, int w, int h, char *title);
void text_draw_barra(int x, int y, int w, int h, int per, int max);
void text_draw_window_bar(int x, int y, int w, int h, int per, int max, char *title);
void _write_text(SDL_Surface *sf, int x, int y, char * str);
void _write_text_inv(SDL_Surface *sf, int x, int y, char * str);
void _write_text_inv_n(SDL_Surface *sf, int x, int y, int n, char * str);
void _write_centered_text(SDL_Surface *sf, int y, char * str);
void _write_num(SDL_Surface *sf, int x, int y, int v);
void _write_num_inv(SDL_Surface *sf, int x, int y, int v);
void _text_draw_window(SDL_Surface *sf, int x, int y, int w, int h, char *title);
void _text_draw_window_bar(SDL_Surface *sf, int x, int y, int w, int h, int per, int max, char *title);
// void text_draw_menu_msg();
void text_flip(void);

void drawPleaseWait(void);
void menu_raise(void);
void menu_unraise(void);

int run_mainMenu();
int run_menuLoad(char FloppyID);
int run_menuSave();
int run_menuGame();
int run_menuControl();

