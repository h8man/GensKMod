#ifndef KMOD_CONFIG_H
#define KMOD_CONFIG_H


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



#ifdef __cplusplus
extern "C" {
#endif

extern struct ConfigKMod_struct KConf;
extern int AutoShot_KMod;
extern int AutoPause_KMod;
extern BOOL	EnabledChannels[6];

void Config_KMod( );
void LoadConfig_KMod();

#ifdef __cplusplus
};
#endif

#endif //KMOD_CONFIG_H