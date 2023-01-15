#ifndef LIBRETRO_CORE_H
#define LIBRETRO_CORE_H 1

#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


#include <stdbool.h>


#define UINT16 uint16_t
#define UINT32 uint32_t

#define RENDER16B
#ifdef  RENDER16B
#define PIXEL_BYTES 1
#define PIXEL_TYPE UINT16
#define PITCH 2	
#else
#define PIXEL_BYTES 2
#define PIXEL_TYPE UINT32
#define PITCH 4	
#endif 

extern char Key_State[512];

extern int pauseg; 

extern void update_prefs_retrocfg(struct uae_prefs *);

#if  defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define LOG_TAG "RetroArch.UAE4ARM"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#else
#define LOGI printf
#endif

#if 0
#define NPLGN 12
#define NLIGN 5
#define NLETT 5

#define STAT_DECX 120
#define STAT_YSZ  20
#else
#define NPLGN 20
#define NLIGN 6
#define NLETT 5
#endif

#ifndef  RENDER16B
#define RGB565(r, g, b)  (((r) << (5+16)) | ((g) << (5+8)) | (b<<5))
#else
#define RGB565(r, g, b)  (((r) << (5+6)) | ((g) << 6) | (b))
#endif
#define uint32 unsigned int
#define uint8 unsigned char
#endif
