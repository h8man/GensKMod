#include "Rom.h"
#include "kmod.h"

#ifndef _GENS_H
#define _GENS_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

//#define CLOCK_NTSC 53700000			// More accurate for division round
//#define CLOCK_PAL  53200000

#define CLOCK_NTSC 53693175
#define CLOCK_PAL  53203424

#define GENS_DEBUG 	1

#define DMODE_68K		1
#define DMODE_Z80		2
#define DMODE_VDP		3
#define	DMODE_CD_68K	4
#define DMODE_CD_CDC	5
#define DMODE_CD_GFX	6
#define DMODE_32_MSH2	7
#define DMODE_32_SSH2	8
#define DMODE_32_VDP	9
#define DMODE_PREVIEW	0x1000

#define DMODE_VDP_REG	10
#define DMODE_SPRITES	11
#define DMODE_YM2612	12
#define DMODE_PSG		13
#define DMODE_WATCHERS	14
#define DMODE_LAYERS	15
#define DMODE_MSG		16
#define DMODE_CD_REG	17
#define DMODE_32_REG	18
#define DMODE_PLANEEXPLORER 19

#define WIN_NUMBER	DMODE_PLANEEXPLORER

extern int Debug;
extern int Frame_Skip;
extern int Frame_Number;
extern int DAC_Improv;
extern int RMax_Level;
extern int GMax_Level;
extern int BMax_Level;
extern int Contrast_Level;
extern int Brightness_Level;
extern int Greyscale;
extern int Invert_Color;

int Round_Double(double val);
void Init_Tab(void);
void Recalculate_Palettes(void);
void Check_Country_Order(void);
char* Detect_Country_SegaCD(void);
void Detect_Country_Genesis(void);

void Init_Genesis_Bios(void);
int Init_Genesis(struct Rom *MD_Rom);
void Reset_Genesis();
int Do_VDP_Only(void);
int Do_Genesis_Frame_No_VDP(void);
int Do_Genesis_Frame(void);

int Init_32X(struct Rom *MD_Rom);
void Reset_32X();
int Do_32X_Frame_No_VDP(void);
int Do_32X_Frame(void);

int Init_SegaCD(const char *iso_name);
int Reload_SegaCD(const char *iso_name);
void Reset_SegaCD();
int Do_SegaCD_Frame_No_VDP(void);
int Do_SegaCD_Frame(void);
int Do_SegaCD_Frame_Cycle_Accurate(void);
int Do_SegaCD_Frame_No_VDP_Cycle_Accurate(void);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif