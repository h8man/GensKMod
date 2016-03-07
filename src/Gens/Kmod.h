#ifndef KMOD_H
#define KMOD_H


#define GENS_KMOD 	1		// define it to enable Kaneda mod
							// define it ALSO on nasmhead.inc
#define NO_MEMORY_LIMIT	0

#include "kmod/common.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef GENS_KMOD

void Init_KMod( );
void ResetDebug_KMod( );
void Update_KMod( );
void kmod_close();
void ToggleWindow_KMod( HWND hWnd, int Debug_Mode);
BOOL IsDialogMsg_KMod( LPMSG lpMsg );
void FrameStep_KMod( );
void ToggleAutoShot_KMod( );

void SpecialReg( unsigned char a, unsigned char b);

void SpyYM2612DAC( );

#endif //GENS_KMOD

#ifdef __cplusplus
};
#endif

#endif