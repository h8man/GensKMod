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
	
struct ConfigKMod_struct {
	BOOL	bAutoWatch;
	BOOL	Spy;
	BOOL	SpyReg;
	BOOL	SpySpr;
	BOOL	SpyRW;
	BOOL	SpyDMA;
	BOOL	CDBios;
	BOOL	noCDBSTAT;
	BOOL	noCDCSTAT;
	BOOL	bBluePause;
	BOOL	pausedAtStart;
	BOOL	singleInstance;
	BOOL	Special;
	BOOL	useGDB;
    unsigned int gdb_m68kport;
    unsigned int gdb_s68kport;
    unsigned int gdb_msh2port;
    unsigned int gdb_ssh2port;
    char	logfile[MAX_PATH];
};

void Init_KMod( );
void ResetDebug_KMod( );
void Update_KMod( );
void kmod_close();
void ToggleWindow_KMod( HWND hWnd, int Debug_Mode);
BOOL IsDialogMsg_KMod( LPMSG lpMsg );
void FrameStep_KMod( );
void ToggleAutoShot_KMod( );

void SpecialReg( unsigned char a, unsigned char b);

//TODO remove
void JumpM68KRam_KMod(DWORD adr);

void SpyYM2612DAC( );



void Config_KMod( );
void LoadConfig_KMod();

void GMVRecord_KMod( );
void GMVStop_KMod( );
void GMVPlay_KMod( );
void GMVUpdatePlay_KMod( );
void GMVTools_KMod( );

extern struct ConfigKMod_struct KConf;

extern int AutoShot_KMod;
extern BOOL	EnabledChannels[6];

#endif //GENS_KMOD

#ifdef __cplusplus
};
#endif

#endif