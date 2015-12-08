#ifndef KMOD_H
#define KMOD_H


#define GENS_KMOD 	1		// define it to enable Kaneda mod
							// define it ALSO on nasmhead.inc
#define NO_MEMORY_LIMIT	0


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
void CloseDebug_KMod();
void ToggleWindow_KMod( HWND hWnd, int Debug_Mode);
BOOL IsDialogMsg_KMod( LPMSG lpMsg );
void FrameStep_KMod( );
void ToggleAutoShot_KMod( );

void SpecialReg( unsigned char a, unsigned char b);
void IncTimer_KMod( unsigned odom);

void SpyReg( unsigned char a, unsigned char b);
void SpySpr( );
void SpyZ80NotPaused(unsigned int address );
void SpyZ80NotPaused_DMA(unsigned int address );
void SpyBadByteRead(unsigned int address );
void SpyBadWordRead(unsigned int address );
void SpyBadByteWrite(unsigned int address );
void SpyBadWordWrite(unsigned int address );

void SpyCDBiosCall( );

void SpyYM2612DAC( );

void Watchers_KMod( );
void AutoWatcher_KMod();

void Config_KMod( );
void LoadConfig_KMod();

void GMVRecord_KMod( );
void GMVStop_KMod( );
void GMVPlay_KMod( );
void GMVUpdatePlay_KMod( );
void GMVTools_KMod( );

extern struct ConfigKMod_struct KConf;
extern UCHAR ActiveLayer;
extern UCHAR ActivePal;
extern int AutoShot_KMod;
extern BOOL	EnabledChannels[6];

#endif //GENS_KMOD

#ifdef __cplusplus
};
#endif

#endif