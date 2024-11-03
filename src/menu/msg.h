static const char *menu_msg="                         Amiga emulator "
#ifdef DREAMCAST
 "for Dreamcast "
#else 
#ifdef DINGOO
 "for Dingoo "
#endif
#endif
#if defined(MIYOO)
 "UAE 0.8.22. reworked for MiyooCFW with changes by smurline and goldmojo. Final build and tests by Apacz rev. 1.2";
#else
 "by Chui - based on UAE 0.8.22. GCW Zero port by Nebuleon and Zear. UAE4ALL logo and minor gfx changes by Hi-Ban. Release 1.";
#endif
#define MAX_SCROLL_MSG (-1500)
