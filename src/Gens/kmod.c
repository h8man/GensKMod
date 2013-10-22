#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "gens.h"
#include "resource.h"
#include "G_main.h"
#include "keycode.h"
#include "G_ddraw.h" // for Put_Info
#include "Misc.h" // for byte swap

#include "M68KD.h"
#include "Cpu_68k.h"
#include "Star_68k.h"
#include "mem_M68K.h"
#include "mem_S68K.h"

#include "mem_Z80.h"
#include "z80.h"
#include "z80dis.h"

#include "vdp_rend.h"
#include "vdp_io.h"

#include "ym2612.h"

#include "psg.h"

#include "LC89510.h"

#include "Mem_SH2.h"
#include "SH2.h"
#include "SH2D.h"
#include "vdp_32X.h"

#include "io.h"



#include "parser.h" //used for struct watchers

/*********************************************

  This is a mod I, Kaneda, tried to apply
  to Gens.
  It's Genny Dev optimized so it could slow
  down the game emulation (sound?)
  If you're a player, don't apply it but use
  my corrected version (look for KANEDA_BUG)

 *********************************************/

/*********************************************
 **
 ** Mod done to original Gens
 ** bug-> bug correction
 ** fix-> optimization, adjust
 **
 ****** 0.0
 ** - replace Debug == X by Debug == DMODE_XXXXX for easier reading
 ** - call debug = screen minimize (if fullscreen)
 ** - some mod to Build_Main_Menu
 ** - debug M68K
 ** - debug z80
 ** - debug VDP (pal/tile)
 ** - debug VDP Registers
 ** - debug sprites
 ** - debug ym2612
 ** - debug PSG
 ** - watchers
 ****** 0.1
 ** - scroll tiles by line
 ** - pal indicator (red arrow)
 ** - select current pal in VDP debug by click
 ** - show/hide layers
 ** - save/load watcher (gamename.wch)
 ** - create watchers directory on init
 ****** 0.2
 ** - Debug Config window with "Spy strange registers val", autoload watch
 ** - spy strange register data
 ** - spy full sprite limit
 ** - log/watcher name ok when loading from history rom
 ** - spy bad read (68K Byte, 68K Word)
 ** - spy bad write (68K Byte, 68K Word)
 ****** 0.3
 ** - bug : on pause mode, if debug window called, no redraw (black screen)
 ** - bug : YM2612 now updated while game paused
 ** - bug : pal's red arrow now centered
 ** - bug : correct YM2612 TimerB value
 ** - fix : VDP redraw optimized (thx Fonzie)
 ** - fix : Sprite redraw optimized
 ** - 'blue pause' optionnel (Fonzie)
 ** - dump VDP tiles to bmp
 ** - dump sprite to bmp
 ** - dump ym2612 instr (y12 or ff)
 ** - supp key to delete a watcher (Varcies)
 ** - ins key to insert a watcher
 ** - active/desactive any pal (color or black)
 ** - sprite zoom
 ** - tile zoom
 ****** 0.4
 ** - bug : memory fault on sprite dump (RedAngel)
 ** - bug : bad tile selection
 ** - bug : no tiles refresh while paused (tomman)
 ** - Spy/Message on separate window
 ** - debug CD 68k
 ** - debug CD CDC
 ** - debug CD Reg
 ** - debug CD GFX
 ** - debug VDP Registers more readable
 ** - spy DMA to hack gfx
 ** - spy DMA limit error (CMD)
 ** - dump M68K rom (interesting when CD or 32X game)
 ** - dump S68K rom (interesting when CD or 32X game)
 ** - Register 31: Timer
 ** - Register 30: Message
 ** - Register 29: Pause Gens
 ** - special registers calls in zip file available in Debug...
 **
 ****** 0.5
 ** - bug : bad 68k memory dump (swap)
 ** - bug : bad MCD reg list
 ** - bug : rename s68k buttons
 ** - GMV record/play
 ** - Register 29: Interface with Gens
 **				0x00: pause
 **				0x01: show debug 68k window
 **				0x02: show debug z80 window
 **				0x03: show debug VDP window
 **				0x04: show debug sub68k window
 **				0x05: show debug CDC window
 **				0x06: show debug CD Gfx window
 **				0x07: show debug 32X main window
 **				0x08: show debug 32X sub window
 **				0x09: show debug 32X VDP window
 **				0x0A: show debug VDP register window
 **				0x0B: show debug Sprite window
 **				0x0C: show debug YM2612 window
 **				0x0D: show debug PSG window
 **				0x0E: show watchers window
 **				0x0F: show Layers window
 **				0x10: show message window
 **				0x11: show debug CD register window
 **				0x50: dump 68k ram
 **				0x51: dump z80 ram
 **				0x52: dump CD68k ram
 **				0x60: screenshot
 **				0x62: start record gmv
 **				0x63: stop record gmv
 **				0x64: start gmv
 **				0x65: stop gmv						
 ** - spy unpaused Z80 access
 ** - debug.s updated
 ** - GMV Tools to compress GMV for use in demos replay
 ** - Pause on menu
 ** - Render frame per frame
 ** - Auto screenshot
 **
 ****** 0.6
 ** - bug : bad redraw when shadow/highlight and layer hide
 ** - bug : correct DAC value
 ** - bug : screenshot takes the message too
 ** - fix : change scroll button with true scrollbar on CD GFX
 ** - fix : change scroll button with scrollbar on VDP (and remove up/down bitmap resource)
 ** - show/hide 32X layer
 ** - Z80 memory viewer and linked disassembly (no more 'current' PC)
 ** - M68K ROM/RAM viewer and linked disassembly (no more 'current' PC)
 ** - S68K PRAM/WRAM viewer and linked disassembly (no more 'current' PC)
 ** - debug.s/.h updaded (warning! func name updated too...sorry for that, but it's now more 'pro')
 ** - dump CD Word RAM to BMP 
 **
 ****** 0.7
 ** - bug : create file and directories at ROM place when 'opened from..' (Pascal/TmEE)
 ** - bug : bad redraw when shadow/highlight and layer hide (GrayLight)
 ** - bug : wrong redraw of S68K PRam adresses in RAM mode (Fonzie)
 ** - bug : screenshot 32X not working
 ** - bug : pause 32X unperfect (since KMod 0.3!)
 ** - fix : changed debug font for W98 user
 ** - better PSG debug window by Edge
 ** - DAC frequency info
 ** - MSH2 ROM/RAM/cache viewer and linked disassembly (thx Fonzie)
 ** - SSH2 ROM/RAM/cache viewer and linked disassembly (thx Fonzie)
 ** - 32x registers
 ** - 32x VDP(s)
 ** - CPUs debug windows are on the same template
 ** - Spy CD Bios call
 ** - Autolog message to file, so no more limited to memory
 ** - add tfi dump (Shiru)
 ** - 5th pal for debug (GrayLight)
 **
 ****** 0.7b
 ** - better watchers, to use nm.exe output
 **
 ****** 0.7c
 ** - resizable watchers window
 ** - structures handling (keywords supported : STRUCT, CHAR, SHORT, LONG, END
 **
 ****** 0.7.1
 ** - VS2012 compile
 ** - enable/disable YM2612 sound channel (TmEE)
 ** - better message view (hangs if too much messages!)
 ** - bug : SH2 disassembler in 0.7c has the registers "shifted" one step. So the value displayed for R0 is actually the value of R1, the value displayed for R1 is the value of R2, and so on. (ob1,_mic)
 *********************************************/

/*********************************************
 **
 ** Mod to do
 **
 ** - bug : Timer not working
 ** - bug : Fifa comes as MD not 32X 
 ** - debug Genesis - Scroll (editable on pause)
 ** - 32x VDP modes handle
 ** - add "jump to"/PC in mem/disasm views
 ** - optimize CD GFX and VDP (a memory DC bitblt to screen when curTile = 0)
 ** - config GYM dumping (only dac, only PSG,...)
 ** - memory viewer/editor (haroldoop)
 ** - GENS source code comments (!!)
 ** - spy DMA (68k->VRAm) while Z80 running
 ** - bugs fonzie
 ** - show FM-status in DebugYM2612
 ** - support VGM dump (see Maxim of SMS Power)
 ** - keyon/off per channel
 ** - update VDP Register (see CMD genvdp.txt)
 ** - show/hide (active/desactive) MCD 256K GFX Ram
 ** - optimize CD Debug (not only original Gens debug on windows)
 ** - spy bad read (S68K Byte, S68K Word, SH2)
 ** - spy bad write (S68K Byte, S68K Word, SH2)
 ** - spy odd/even RW error
 ** - Flux (MD+MCD)
 ** - true disasm
 ** - breakpoint
 ** - add Halt when value is xxx on Watchers
 ** - symbol file (haroldoop)
 ** - add line sprites limit spy
 **
 *********************************************/

/*********************************************
 **
 ** Mod I'm unable to do
 **
 ** - spy DMA odd address error while RAM transfert
 **    -> it's the half of the adress which is written on Reg21->23, it's dever responsability to not make the mistake
 **
 *********************************************/

/* bug from Fonzie 

Dans les adresses VRAM (Debug>Genesis>VDP)
L'adresse s'incremente de 0x10 par ligne, ça devrai être 0x200 par ligne il me semble
-------------

*/ 

/*********************************************
Bug found by bensimauru (SEF)
When sound is enabled, the frame rate looks a little choppy, even though my CPU
isn't maxed and the FPS indicator is hovering around 60.  If I disable sound, it looks much smoother.
It's pretty visible with Sonic.  Anyone else notice this?  

It seems to have to do with the audio buffering - if I change
 spec.samples = 1024;
to
 spec.samples = 256;
in src/gens/sdllayer/g_sdlsound.c, it looks pretty much as smooth as without sound.  I also changed
 while (audio_len > 1024 * 2 * 2 * 4)
to
 while (audio_len > 256 * 2 * 2 * 4)
in Write_Sound_Buffer to prevent crashing on exit.

So I think an audio buffer of 1024 samples blocks for too long and throws off frame timing.


reply from Rock2000 (SEF):
I'm not familiar with the SDL version of Gens, but I assume its the same issue I found.
The problem is that the sound rate is faster then the video rate (or slower, I forget the details).
So Gens skips a frame to get the two back into sink when they diverge too much.
With no auto frame skip (DirectX version) the sound will periodically wrap around the circular buffer and
you'll get lots of static and scratches for awhile. Auto frame skip fixes that, but you're stuck with
a frame skip every so often instead. I just added some brute force code for myself that determines how
close the sound buffer is to wrapping around on itself, and toggle the playback frequency between
44100 and 43900 to ensure it doesn't wrap. Works for me with no scratchy sound and no frame skip.  
 *********************************************/

/*
#define ITEM_BAD_TYPE		0x00000000
#define ITEM_CHAR_TYPE		0x01000000
#define ITEM_INT_TYPE		0x02000000
#define ITEM_LONG_TYPE		0x04000000
*/

#define TIMER_CYCLES		66480	/* cycles used by timer call */

#define WIN_NUMBER	18

struct oper {
	unsigned char op_data[8];
	long op_padding[2];
};

struct voice {
	struct oper op[4];
	unsigned char	algo;
	unsigned char	fback;		
	unsigned char 	padding[14];
};

struct ym2612_KMod {
	struct 		voice ym;
	unsigned char	name[16];
	unsigned char	dumper[16];
	unsigned char	game[16];
}KM2612;




char *mcd_bios_name[]={
"UNKNOWN 0x00   ",
"UNKNOWN 0x01   ",
"MSCSTOP        ",
"MSCPAUSEON     ",
"MSCPAUSEOFF    ",
"MSCSCANFF      ",
"MSCSCANFR      ",
"MSCSCANOFF     ",
"ROMPAUSEON     ",
"ROMPAUSEOFF    ",
"DRVOPEN        ",
"UNKNOWN 0x0b   ",
"UNKNOWN 0x0c   ",
"UNKNOWN 0x0d   ",
"UNKNOWN 0x0e   ",
"UNKNOWN 0x0f   ",
"DRVINIT        ",
"MSCPLAY        ",
"MSCPLAY1       ",
"MSCPLAYR       ",
"MSCPLAYT       ",
"MSCSEEK        ",
"MSCSEEKT       ",
"ROMREAD        ",
"ROMSEEK        ",
"MSCSEEK1       ",
"UNKNOWN 0x1a   ",
"UNKNOWN 0x1b   ",
"UNKNOWN 0x1c   ",
"UNKNOWN 0x1d   ",
"TESTENTRY      ",
"TESTENTRYLOOP  ",
"ROMREADN       ",
"ROMREADE       ",
"UNKNOWN 0x22   ",
"UNKNOWN 0x23   ",
"UNKNOWN 0x24   ",
"UNKNOWN 0x25   ",
"UNKNOWN 0x26   ",
"UNKNOWN 0x27   ",
"UNKNOWN 0x28   ",
"UNKNOWN 0x29   ",
"UNKNOWN 0x2a   ",
"UNKNOWN 0x2b   ",
"UNKNOWN 0x2c   ",
"UNKNOWN 0x2d   ",
"UNKNOWN 0x2e   ",
"UNKNOWN 0x2f   ",
"UNKNOWN 0x30   ",
"UNKNOWN 0x31   ",
"UNKNOWN 0x32   ",
"UNKNOWN 0x33   ",
"UNKNOWN 0x34   ",
"UNKNOWN 0x35   ",
"UNKNOWN 0x36   ",
"UNKNOWN 0x37   ",
"UNKNOWN 0x38   ",
"UNKNOWN 0x39   ",
"UNKNOWN 0x3a   ",
"UNKNOWN 0x3b   ",
"UNKNOWN 0x3c   ",
"UNKNOWN 0x3d   ",
"UNKNOWN 0x3e   ",
"UNKNOWN 0x3f   ",
"UNKNOWN 0x40   ",
"UNKNOWN 0x41   ",
"UNKNOWN 0x42   ",
"UNKNOWN 0x43   ",
"UNKNOWN 0x44   ",
"UNKNOWN 0x45   ",
"UNKNOWN 0x46   ",
"UNKNOWN 0x47   ",
"UNKNOWN 0x48   ",
"UNKNOWN 0x49   ",
"UNKNOWN 0x4a   ",
"UNKNOWN 0x4b   ",
"UNKNOWN 0x4c   ",
"UNKNOWN 0x4d   ",
"UNKNOWN 0x4e   ",
"UNKNOWN 0x4f   ",
"UNKNOWN 0x50   ",
"UNKNOWN 0x51   ",
"UNKNOWN 0x52   ",
"UNKNOWN 0x53   ",
"UNKNOWN 0x54   ",
"UNKNOWN 0x55   ",
"UNKNOWN 0x56   ",
"UNKNOWN 0x57   ",
"UNKNOWN 0x58   ",
"UNKNOWN 0x59   ",
"UNKNOWN 0x5a   ",
"UNKNOWN 0x5b   ",
"UNKNOWN 0x5c   ",
"UNKNOWN 0x5d   ",
"UNKNOWN 0x5e   ",
"UNKNOWN 0x5f   ",
"UNKNOWN 0x60   ",
"UNKNOWN 0x61   ",
"UNKNOWN 0x62   ",
"UNKNOWN 0x63   ",
"UNKNOWN 0x64   ",
"UNKNOWN 0x65   ",
"UNKNOWN 0x66   ",
"UNKNOWN 0x67   ",
"UNKNOWN 0x68   ",
"UNKNOWN 0x69   ",
"UNKNOWN 0x6a   ",
"UNKNOWN 0x6b   ",
"UNKNOWN 0x6c   ",
"UNKNOWN 0x6d   ",
"UNKNOWN 0x6e   ",
"UNKNOWN 0x6f   ",
"UNKNOWN 0x70   ",
"UNKNOWN 0x71   ",
"UNKNOWN 0x72   ",
"UNKNOWN 0x73   ",
"UNKNOWN 0x74   ",
"UNKNOWN 0x75   ",
"UNKNOWN 0x76   ",
"UNKNOWN 0x77   ",
"UNKNOWN 0x78   ",
"UNKNOWN 0x79   ",
"UNKNOWN 0x7a   ",
"UNKNOWN 0x7b   ",
"UNKNOWN 0x7c   ",
"UNKNOWN 0x7d   ",
"UNKNOWN 0x7e   ",
"UNKNOWN 0x7f   ",
"CDBCHK         ",
"CDBSTAT        ",
"CDBTOCWRITE    ",
"CDBTOCREAD     ",
"CDBPAUSE       ",
"FDRSET         ",
"FDRCHG         ",
"CDCSTART       ",
"CDCSTARTP      ",
"CDCSTOP        ",
"CDCSTAT        ",
"CDCREAD        ",
"CDCTRN         ",
"CDCACK         ",
"SCDINIT        ",
"SCDSTART       ",
"SCDSTOP        ",
"SCDSTAT        ",
"SCDREAD        ",
"SCDPQ          ",
"SCDPQL         ",
"LEDSET         ",
"CDCSETMODE     ",
"WONDERREQ      ",
"WONDERCHK      "
};



struct _32X_register_struct {
	BYTE side; //bit 0:MD, bit 1:32X
	WORD adr;
	char *descriptionMD;
	char *description32X;
};



struct _32X_register_struct _32X_register[]={
	{3, 0x00, "Adapter control register", "Interrupt Mask"},
	{3, 0x02, "Interrupt control register", "Stand By change"},
	{3, 0x04, "Bank set register", "H Count"},
	{3, 0x06, "DREQ control register", "DREQ control register"},
	{3, 0x08, "DREQ source MSB", "DREQ source MSB"},
	{3, 0x0A, "DREQ source LSB", "DREQ source LSB"},
	{3, 0x0C, "DREQ destination MSB", "DREQ destination MSB"},
	{3, 0x0E, "DREQ destination LSB", "DREQ destination LSB"},
	{3, 0x10, "DREQ lenght", "DREQ lenght"},
	{3, 0x12, "FIFO", "FIFO"},
	{2, 0x14, "", "VRES Interrupt clear"},
	{2, 0x16, "", "V Interrupt clear"},
	{2, 0x18, "", "H Interrupt clear"},
	{3, 0x1A, "SEGA TV register", "CMD Interrupt clear"},
	{2, 0x1C, "", "PWM Interrupt clear"},
	{3, 0x20, "Comm Port 0", "Comm Port 0"},
	{3, 0x22, "Comm Port 1", "Comm Port 1"},
	{3, 0x24, "Comm Port 2", "Comm Port 2"},
	{3, 0x26, "Comm Port 3", "Comm Port 3"},
	{3, 0x28, "Comm Port 4", "Comm Port 4"},
	{3, 0x2A, "Comm Port 5", "Comm Port 5"},
	{3, 0x2C, "Comm Port 6", "Comm Port 6"},
	{3, 0x2E, "Comm Port 7", "Comm Port 7"},
	{3, 0x30, "PWM control", "PWM control"},
	{3, 0x32, "Cycle register", "Cycle register"},
	{3, 0x34, "L ch Pulse Width", "L ch Pulse Width"},
	{3, 0x36, "R ch Pulse Width", "R ch Pulse Width"},
	{3, 0x38, "Mono Pulse Width", "Mono Pulse Width"},
	{2, 0x100, "", "Bitmap mode"},
	{2, 0x102, "", "Packed Pixel Control"},
	{2, 0x104, "", "Auto fill lenght"},
	{2, 0x106, "", "Auto fill start"},
	{2, 0x108, "", "Auto fill data"},
	{2, 0x10A, "", "Frame Buffer control"},
	{0xff,-1,""}
};

#define WATCHER_SUBDATA		0x8000
#define WATCHER_STRUCT_MASK	0x7F00 // 126 struct max
#define WATCHER_SIZE_MASK	0x00FF

struct str_Watcher {
	DWORD	adr;
	char	name[50];
	WORD	size;
};

struct str_StructElement {
	char	name[20];
	UCHAR	size;
};

struct str_Structure {
	char	name[20];
	UCHAR	nbElements;
	struct str_StructElement	elements[10];
} StructureKMod[126];


/* Dirty way to know which window is open, using original Gens Debug values */
UCHAR OpenedWindow_KMod[ WIN_NUMBER ]; /* 0: Window Closed, 1: Window Opened */
HWND  HandleWindow_KMod[ WIN_NUMBER ];

HWND hM68K, hZ80, hVDP, hMisc, hSprites, hYM2612, hPSG;
HWND hCD_68K, hCD_CDC, hCD_GFX, hCD_Reg;
HWND hMSH2, hSSH2, h32X_VDP, h32X_Reg;
HWND hWatchers;
HWND hLayers;
HWND hDMsg;

struct ConfigKMod_struct KConf;
char szWatchDir[MAX_PATH];
char szKModLog[MAX_PATH];

CHAR debug_string[1024];

int AutoPause_KMod;
int AutoShot_KMod;

//TODO could be handle by typedef struct channel__ .enabled ?
BOOL	EnabledChannels[5];

// defined on VDP_REND.ASM
//UCHAR ActiveLayer; /* 0003 ABSW */
//UCHAR ActivePal; /* 0000 3210 */


void Dump68K_KMod(HWND hwnd);
void DumpS68KPRam_KMod(HWND hwnd);
void DumpZ80_KMod(HWND hwnd);
void GMVRecord_KMod( );
void GMVStop_KMod( );
void GMVPlay_KMod( );
void JumpM68KRam_KMod( DWORD adr );

/******* PRIVATE *********/
void CloseWindow_KMod( UCHAR mode )
{
	if (mode < 1)	return;
	if (mode > WIN_NUMBER)	return;

	OpenedWindow_KMod[ mode-1 ] = 0;
	ShowWindow( HandleWindow_KMod[mode-1], SW_HIDE );
}

void OpenWindow_KMod( UCHAR mode )
{
	if (mode < 1)	return;
	if (mode > WIN_NUMBER)	return;

	OpenedWindow_KMod[ mode-1 ] = 1;
	ShowWindow( HandleWindow_KMod[mode-1], SW_SHOW	 );
	Update_KMod( );
}


/// for all memory views
void Hexview( unsigned char *addr, unsigned char *dest)
{
	wsprintf(dest, "%.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
}


void Ansiview( unsigned char *addr, unsigned char *dest)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		if (addr[i] == 0)
			wsprintf(dest+i, "%c", '.');
		else
			wsprintf(dest+i, "%c", addr[i]);
	}
}

/************ Message **************/
ULONG	timer_KMod;
HANDLE	KMsgLog;
UCHAR	msgIdx_KMod, msg_KMod[255];
char	*errorText_KMod="** Too many messages **";

BOOL Msg_KMod( char *msg)
{
	CHAR *editText;
	CHAR *editCutText;
	UINT nSize, nSizeToAdd, nMaxSize;
	DWORD dwBytesToWrite, dwBytesWritten;

	if (KMsgLog)
	{
		dwBytesToWrite = strlen( msg );
		if (dwBytesToWrite)		WriteFile(KMsgLog, msg, dwBytesToWrite, &dwBytesWritten, NULL) ;
	}

	nSizeToAdd = strlen(msg);
	nSize = (UINT) SendDlgItemMessage(hDMsg, IDC_MSG_EDIT, WM_GETTEXTLENGTH, (WPARAM) 0, (LPARAM) 0);
	nMaxSize =  (UINT) SendDlgItemMessage(hDMsg, IDC_MSG_EDIT, EM_GETLIMITTEXT, (WPARAM) 0, (LPARAM) 0);

	nSize += nSizeToAdd+1;
	editText = (CHAR *) LocalAlloc( LPTR, nSize);
	if (editText == NULL)	return FALSE;

	GetDlgItemText(hDMsg, IDC_MSG_EDIT, editText, nSize);	
	strcat(editText, msg);
	
	nSize = strlen(editText);
	editCutText = editText;
	while ( nSize >=  nMaxSize)	
	{
		editCutText = strstr(editText, "\r\n");
		editCutText+=2;
		nSize = strlen(editCutText);
	}

	SetDlgItemText(hDMsg, IDC_MSG_EDIT, editCutText);
	SendDlgItemMessage(hDMsg, IDC_MSG_EDIT, EM_LINESCROLL, (WPARAM) 0, (LPARAM) SendDlgItemMessage(hDMsg, IDC_MSG_EDIT, EM_GETLINECOUNT, (WPARAM) 0, (LPARAM) 0) );
	LocalFree( (HLOCAL) editText );
	editText = NULL;

	return TRUE;
}



void MsgBrowse_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;

	unsigned char Conf_File[MAX_PATH];

	SetCurrentDirectory(Gens_Path);
	strcpy(Conf_File, Gens_Path);
	strcat(Conf_File, "GensKMod.cfg");

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_msg");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "Log file (*.log)\0*.log\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Message Log file";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "log";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;
	
    CloseHandle(hFr);

	strcpy(KConf.logfile ,szFileName);

	WritePrivateProfileString("Debug", "file", KConf.logfile, Conf_File);	

	if (KMsgLog)	CloseHandle( KMsgLog );
	//FILE_SHARE_READ pour pouvoir l'ouvrir dans notepad
	KMsgLog = CreateFile (KConf.logfile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (KMsgLog != INVALID_HANDLE_VALUE)	SetFilePointer(KMsgLog, 0, 0, FILE_END);

	SetDlgItemText(hwnd, IDC_MSG_FILE, KConf.logfile);
}


void IncTimer_KMod( unsigned odom)
{
	if (timer_KMod)	timer_KMod += odom;	
}

void SpecialReg( unsigned char a, unsigned char b)
{
	
	/* Special new registers :
	 31 :	sr000000	Timer register, start and output a timer (based on m68k cycles)
				s=0		Counter output 
				s=1		Counter start
				r=0		<undefined>
				r=1		reset counter

	 30 :	xxxxxxxx	Message register
						char to add to string (>= 0x20)
						automatically shown if 255th char
						if 0, show and reset message string
	 
	 29 :	xxxxxxxx	Ask Gens to ....
			00000000	0x00 : pause
			00xxxxxx    0x?? : open debug window
			01010000	0x50 : dump 68k ram
			01010001	0x51 : dump Z80 ram
			01010010	0x52 : dump S68k ram
			01100000	0x60 : take screenshot
			01100010	0x62 : start record gmv
			01100011	0x63 : stop record gmv
			01100100	0x64 : start gmv
			01100101	0x65 : stop gmv
						
	*/
		
	if (!Game)	return;

	if (!KConf.Special)	return;

	switch(a)
	{
		case 31:
			if (b&0x80)
			{
				/* starttimer */
				wsprintf(debug_string,"Timer started\r\n");
				Msg_KMod(debug_string);
				timer_KMod = 1;
			}
			else
			{
				wsprintf(debug_string,"Timer : %.10d cycles elapsed\r\n", main68k_readOdometer() + timer_KMod -1 - TIMER_CYCLES);
				Msg_KMod(debug_string);

				if (b&0x40)	timer_KMod = 0;
			}			
			break;
		
		case 30:
			if (b == 0)
			{
				if (msgIdx_KMod > 0)
				{
					wsprintf(debug_string,"Message : %s\r\n", msg_KMod);
					Msg_KMod(debug_string);

					msgIdx_KMod = 0;
					ZeroMemory(msg_KMod, 255);
				}
			}
			else if (b<0x20)
			{
				return;
			}
			else
			{
				msg_KMod[msgIdx_KMod++] = b;
				if (msgIdx_KMod == 255)	
					SpecialReg(30, 0); /* flush to msgbox */
			}
			break;

		case 29:
			switch(b)
			{
				case 0: // pause
					if (Paused)
					{
						Msg_KMod("Pause requested by already paused game\r\n");
					}
					else
					{
						Paused = 1;
						Pause_Screen( );
						Msg_KMod("Pause requested by game\r\n");
					}
					break;

				case 0x50: // dump 68K ram
					Dump68K_KMod( hM68K );
					break;

				case 0x51: // dump z80 ram
					DumpZ80_KMod( hZ80 );
					break;
				
				case 0x52: // dump s68k program ram
					DumpS68KPRam_KMod( hCD_68K );
					break;

				case 0x60:
					Take_Shot( );
					Msg_KMod("Screenshot requested by game\r\n");
					break;

				case 0x62:
					Msg_KMod("Start recording a GMV file");
					GMVRecord_KMod( );
					break;

				case 0x63:
					Msg_KMod("Stop recording a GMV file");
					GMVStop_KMod( );
					break;

				case 0x64:
					Msg_KMod("Start playing a GMV file");
					GMVPlay_KMod( );
					break;
				
				case 0x65:
					Msg_KMod("Stop playing a GMV file");
					GMVStop_KMod( );
					break;

				default:
					if ( b < 20)
					{
						OpenWindow_KMod( b );
					}
			}
			break;
	}

}

void MsgOpen_KMod( HWND hwnd )
{
	ShellExecute(hwnd, "open", "notepad", KConf.logfile, NULL, SW_SHOW);
}

void MsgInit_KMod( HWND hwnd )
{
	unsigned char Conf_File[MAX_PATH];

	SetCurrentDirectory(Gens_Path);
	strcpy(Conf_File, Gens_Path);
	strcat(Conf_File, "GensKMod.cfg");

	GetPrivateProfileString("Log", "file", szKModLog, KConf.logfile, MAX_PATH, Conf_File);
	WritePrivateProfileString("Debug", "file", KConf.logfile, Conf_File);	

	if (KMsgLog)	CloseHandle( KMsgLog );
	//FILE_SHARE_READ pour pouvoir l'ouvrir dans notepad
	KMsgLog = CreateFile (szKModLog, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (KMsgLog != INVALID_HANDLE_VALUE)	SetFilePointer(KMsgLog, 0, 0, FILE_END);

	SetDlgItemText(hwnd, IDC_MSG_FILE, szKModLog);

	if (!Game)	return;

	Msg_KMod("*******************");
	Msg_KMod(Rom_Name);
	Msg_KMod("\r\n");

	msgIdx_KMod = 0;
	ZeroMemory(msg_KMod, 255);
	
}


BOOL CALLBACK MsgDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;

	switch(Message)
    {
		case WM_INITDIALOG:
			hFont = (HFONT) GetStockObject(OEM_FIXED_FONT);
		//	SendDlgItemMessage(hwnd, IDC_MSG_EDIT, WM_SETFONT, (WPARAM)hFont, TRUE);

			MsgInit_KMod( hwnd );
			break;
		
		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case IDC_MSG_CLEAR:
					SetDlgItemText(hwnd, IDC_MSG_EDIT,"");
					break;

				case IDC_MSG_OPEN:
					MsgOpen_KMod( hwnd );
					break;

				case IDC_MSG_BROWSE:
					MsgBrowse_KMod( hwnd );
					break;
			}
			break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_MSG );
			break;

		case WM_DESTROY:
			if (KMsgLog)
				CloseHandle( KMsgLog );

			if (hFont != NULL)
				DeleteObject( (HGDIOBJ) hFont );

			DestroyWindow( hDMsg );
			PostQuitMessage(0);
			break;
		
		default:
            return FALSE;
    }
    return TRUE;
}
/************ Spy **************/

void Spy_KMod( char *log )
{
	Msg_KMod("Spy : ");
	Msg_KMod(log);
	Msg_KMod("\r\n");
}


void SpyReg( unsigned char a, unsigned char b)
{
	unsigned short VDPS;

	if (!Game)	return;

	if (!KConf.Spy )	return;

	if (!KConf.SpyReg )	return;

	VDPS = Read_VDP_Status( );

	switch(a)
	{
		case 0:
			if ( (b&0x04) == 0)
			{
				wsprintf(debug_string,"%.5X Reg. %d : bit 2 must be 1", main68k_context.pc, a);
				Spy_KMod( debug_string );
			}

			if ( b&0xE9 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value doesn't follow the schema 000?01?0", main68k_context.pc, a);
				Spy_KMod( debug_string );
			}
			break;

		case 1:
			if ( (b&0x04) == 0)
			{
				wsprintf(debug_string,"%.5X Reg. %d : bit 2 must be 1", a);
				Spy_KMod( debug_string );
			}

			if ( b&0x83 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 0????100", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}

			if ((b&0x08) && ( (VDPS&1) == 0))
			{
				wsprintf(debug_string,"%.5X Reg. %d : V30 cell mode (0x08) impossible in NTSC mode", main68k_context.pc, a);
				Spy_KMod( debug_string );
			}
			break;

		case 2:
			if ( b&0xC7 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 00???000", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 3:
			if ( (b&0xC1) && (H_Cell==32) )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 00?????0", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			else if ( (b&0xC3) && (H_Cell==40) )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 00????00", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 4:
			if ( b&0xF8 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 00000???", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 5:
			if ( (b&0x80) && (H_Cell==32) )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 0???????", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			else if ( (b&0x81) && (H_Cell==40) )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 0??????0", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 6:
			if (b)
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X must be 0", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 7:
			if ( b&0xC0 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 00??????", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 8:
			if (b)
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X must be 0", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 9:
			if (b)
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X must be 0", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 10:
			// nothing to check
			break;

		case 11:
			if ( b&0xF0 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 0000????", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}

			if ( (b&0x03) == 0x01 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : wrong HSCroll mode (0x01)", main68k_context.pc, a);
				Spy_KMod( debug_string );
			}
			break;

		case 12:
			if ( b&0x70 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema ?000????", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}

			if ( ((b&0x81) == 0x10) || ((b&0x81) == 0x01) )
			{
				wsprintf(debug_string,"%.5X Reg. %d : wrong H cell mode (%.2X)", main68k_context.pc, a, b&0x81);
				Spy_KMod( debug_string );
			}

			if ( (b&0x06) == 0x40 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : wrong interlace mode (2)", main68k_context.pc, a);
				Spy_KMod( debug_string );
			}
			break;

		case 13:
			if ( b&0xC0 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 00??????", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 14:
			if ( b )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X must be 0", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 15:
			if ( !b )
			{
				wsprintf(debug_string,"%.5X Reg. %d : an auto increment data of 0 is a bad idea!", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 16:
			if ( b&0xCC )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema 00??00??", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}

			if ( (b&0x03) == 0x02 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : wrong H scroll mode (2)", main68k_context.pc, a);
				Spy_KMod( debug_string );
			}			

			if ( (b&0x30) == 0x20 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : wrong V scroll mode (2)", main68k_context.pc, a);
				Spy_KMod( debug_string );
			}			
			break;

		case 17:
			if ( b&0x60 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema ?00?????", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 18:
			if ( b&0x60 )
			{
				wsprintf(debug_string,"%.5X Reg. %d : value %.2X doesn't follow the schema ?00?????", main68k_context.pc, a, b);
				Spy_KMod( debug_string );
			}
			break;

		case 19:
			// nothing
			break;

		case 20:
			// nothing
			break;

		case 21:
			// nothing
			break;

		case 22:
			// nothing
			break;

		case 23:
			// nothing
			break;

		default:
			wsprintf(debug_string,"%.5X Wrong register (%d) called with value %.2X", main68k_context.pc, a, b);
			Spy_KMod( debug_string );
			break;
	}
}


void SpySpr( )
{
	if (!Game)	return;

	if (!KConf.Spy )	return;

	if (!KConf.SpySpr )	return;
	
	Spy_KMod( "More than 80 sprites to draw" );
}

void SpyZ80NotPaused(unsigned int address )
{
	if (!Game)	return;

	if (!KConf.Spy )	return;

	if (!KConf.SpyRW )	return;

	wsprintf(debug_string,"%.5X want to access unpaused Z80 at %.8X", main68k_context.pc, address);
	Spy_KMod( debug_string );
}

void SpyZ80NotPaused_DMA(unsigned int address )
{
	if (!Game)	return;

	if (!KConf.Spy )	return;

	if (!KConf.SpyRW )	return;

	wsprintf(debug_string,"%.5X want to DMA while Z80 is running, at %.8X", main68k_context.pc, address);
	Spy_KMod( debug_string );
}


void SpyBadByteRead(unsigned int address )
{
	if (!Game)	return;

	if (!KConf.Spy )	return;

	if (!KConf.SpyRW )	return;

	wsprintf(debug_string,"%.5X want to read a Byte at %.8X", main68k_context.pc, address);
	Spy_KMod( debug_string );
}

void SpyBadWordRead(unsigned int address )
{
	if (!Game)	return;

	if (!KConf.Spy )	return;
	if (!KConf.SpyRW )	return;

	wsprintf(debug_string,"%.5X want to read a Word at %.8X", main68k_context.pc, address);
	Spy_KMod( debug_string );
}

void SpyBadByteWrite(unsigned int address )
{
	if (!Game)	return;

	if (!KConf.Spy )	return;
	if (!KConf.SpyRW )	return;

	wsprintf(debug_string,"%.5X want to write a Byte at %.8X", main68k_context.pc, address);
	Spy_KMod( debug_string );
}

void SpyBadWordWrite(unsigned int address )
{
	if (!Game)	return;

	if (!KConf.Spy )	return;
	if (!KConf.SpyRW )	return;

	wsprintf(debug_string,"%.5X want to write a Word at %.8X", main68k_context.pc, address);
	Spy_KMod( debug_string );
}


void SpyDMA( )
{
	if (!Game)	return;

	if (!KConf.Spy )	return;

	if (!KConf.SpyDMA )	return;

	if (!(VDP_Reg.Set2 & 0x10) )	return;

	switch( VDP_Reg.DMA_Src_Adr_H >> 6 )
	{

		case 0x02:
			wsprintf(debug_string, "VRAM Fill of %d bytes into %0.4X", VDP_Reg.DMA_Lenght, Ctrl.Address&0xFFFF);
			break;
		
		case 0x03:
			wsprintf(debug_string, "VRAM copy of %d bytes from %0.6X to %0.4X", VDP_Reg.DMA_Lenght, VDP_Reg.DMA_Address, Ctrl.Address&0xFFFF);
			if ( (VDP_Reg.DMA_Address/0x020000) != ((VDP_Reg.DMA_Lenght+VDP_Reg.DMA_Address) / 0x020000) )
				lstrcat(debug_string, "\r\nError : DMA must stay between a same 0x020000 block");
			break;

		case 0x00:
		case 0x01:
			// 68k -> VRAM
			wsprintf(debug_string, "68k -> VRAM of %d bytes from %0.6X to %0.4X", VDP_Reg.DMA_Lenght, VDP_Reg.DMA_Address*2, Ctrl.Address&0xFFFF);		


			switch(Ctrl.Access & 0x03)
			{
				case 0x00:
					lstrcat(debug_string, " (Wrong access?)");
					break;
				case 0x01:
					lstrcat(debug_string, " (VRAM access)");
					break;
				case 0x02:
					lstrcat(debug_string, " (CRAM access)");
					break;
				case 0x03:
					lstrcat(debug_string, " (VSRAM access)");
					break;
			}

			if ( (VDP_Reg.DMA_Address/0x020000) != ((VDP_Reg.DMA_Lenght+VDP_Reg.DMA_Address) / 0x020000) )
				lstrcat(debug_string, "\r\nError : DMA must stay between a same 0x020000 block");
			break;
		default:
			wsprintf(debug_string, "Invalide DMA mode");
	}

	Spy_KMod( debug_string );
}


void SpyCDBiosCall( )
{
	unsigned newPC;
	short int function;

	if (!Game)	return;
 
	if (!KConf.CDBios )	return;

	newPC = sub68k_readPC( );
	if (newPC == 0x5f22)
	{
		function = (short int) (sub68k_context.dreg[0]&0xFFFF);
		if ((function == 0x81) && KConf.noCDBSTAT)	return;

		if ((function == 0x8a) && KConf.noCDCSTAT)	return;

		//wsprintf(debug_string,"jumped at %.X: a0=%.8X a1=%.8X d0=%.4X d1=%.8X", newPC, sub68k_context.areg[0], sub68k_context.areg[1], sub68k_context.dreg[0]&0xFFFF, sub68k_context.dreg[1]);
		wsprintf(debug_string,"Call %s: a0=%.8X a1=%.8X d0=%.4X d1=%.8X", mcd_bios_name[function], sub68k_context.areg[0], sub68k_context.areg[1], sub68k_context.dreg[0]&0xFFFF, sub68k_context.dreg[1]);
		Spy_KMod( debug_string );
   	}
}


void UpdateSpy_KMod( )
{
	if (!Game)	return;

	if (!KConf.Spy )	return;
}


/******* WATCHERS *****************/
/// MSDN : You cannot set the state or lParam members for subitems because subitems do not have these attributes
HWND hWatchList;

char *GetZoneWatch_KMod( unsigned long adr )
{
	if ( adr < 0x00400000 )
		return "ROM";
	else if ( adr < 0x00800000 )
		return "Sega 1";
	else if ( adr < 0x00A00000 )
		return "Sega 2";
	else if ( adr < 0x00A10000 )
		return "Z80";
	else if ( adr < 0x00A11000 )
		return "IO";
	else if ( adr < 0x00A12000 )
		return "Control";
	else if ( adr < 0x00B00000 )
		return "Sega 3";
	else if ( adr < 0x00C00000 )
		return "Sega 4";
	else if ( adr < 0x00E00000 )
		return "VDP";
	else if ( adr < 0x00FF0000 )
		return "!! WARNING !!";
	else
		return "RAM";

}


void UpdateWatchers_KMod( )
{
	int         i, nbItems;
	LVITEM		lvItem;
	char	buf[64];    
	DWORD		value;
	struct str_Watcher *watcher;

	if ( OpenedWindow_KMod[ 13 ] == FALSE )	return;

	nbItems = ListView_GetItemCount(hWatchList);
	if (nbItems < 1)	return;

	for(i=0; i<nbItems; i++)
	{
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		ListView_GetItem(hWatchList, &lvItem);
		watcher = (struct str_Watcher *) lvItem.lParam;

		if (watcher->size & 1)
			wsprintf(buf, "0x%0.2X",  M68K_RB(watcher->adr));
		else if (watcher->size & 2)
			wsprintf(buf, "0x%0.4X",  M68K_RW(watcher->adr));
		else 
		{
			value =  M68K_RW(watcher->adr);
			value <<= 16;
			value |=  M68K_RW(watcher->adr+2);
			wsprintf(buf, "0x%0.8X", value);
		}
		/*
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 2;
		lvItem.pszText = buf;
		ListView_SetItem(hWatchList, &lvItem);
*/
		
		ListView_SetItemText(hWatchList, i, 1, watcher->name);
		ListView_SetItemText(hWatchList, i, 2, buf);
		ListView_SetItemText(hWatchList, i, 3, GetZoneWatch_KMod(watcher->adr ));
	}
}



void AddWatcher_KMod(  struct str_Watcher newWatcher, WORD pos )//unsigned long adrHexa, char *wchname )
{
	LVITEM		lvItem;
    char		buf[64];
	struct str_Watcher* watcher;

	watcher = (struct str_Watcher *) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct str_Watcher));
	if (!watcher)	return;

	watcher->adr = newWatcher.adr;
	watcher->size = newWatcher.size;
	strcpy(watcher->name, newWatcher.name);


	/* add a watcher to 0 (user must mod it later) */
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.iItem = pos;
	lvItem.iSubItem = 0;
	wsprintf(buf, "%0.8X", watcher->adr);
	lvItem.pszText = buf;
	lvItem.lParam = (LPARAM) (watcher);
	ListView_InsertItem(hWatchList, &lvItem);
}



void DeleteWatcher_KMod( int  i )
{
	LVITEM		lvItem;
	struct str_Watcher* watcher;

	/* delete the first selected watch */ 
	if (i==-1)	i = ListView_GetNextItem(hWatchList, -1, LVNI_ALL | LVNI_SELECTED);

	if (i==-1)	return;

	do
	{
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		if (!ListView_GetItem(hWatchList, &lvItem))		return; //happens if i<0 for example...but it's impossible unless bug

		watcher = (struct str_Watcher*) lvItem.lParam;
		i--;
	}
	while(watcher->size & WATCHER_SUBDATA); //if subdata, move back to main watcher

	i++;
	HeapFree(GetProcessHeap(), 0, watcher);
	ListView_DeleteItem(hWatchList, i);


	//if ( !(watcher->size & WATCHER_STRUCT_MASK) )	return; //no sub element to delete

	do
	{
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		if (!ListView_GetItem(hWatchList, &lvItem))		return;

		watcher = (struct str_Watcher*) lvItem.lParam;
		if (watcher->size & WATCHER_SUBDATA)	ListView_DeleteItem(hWatchList, i);

	}
	while(watcher->size & WATCHER_SUBDATA); //delete every subdata
}

void SaveWatchers_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten;
	LVITEM		lvItem;
	WORD	i, nbItems;
	struct str_Watcher* tmpWatcher;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, szWatchDir);
	strcat(szFileName, Rom_Name);
	strcat(szFileName, ".wch");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "Watchers (*.wch)\0*.wch\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = szWatchDir;//(LPSTR)NULL;
    szFile.lpstrTitle = "Save Watchers";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "wch";


	SetCurrentDirectory(Gens_Path);

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	
	nbItems = ListView_GetItemCount(hWatchList);
	if (nbItems < 1)
	{
		MessageBox(NULL, "No watcher to save", NULL, MB_OK);
	    CloseHandle(hFr);
		return;
	}

	dwBytesToWrite = sizeof(nbItems);
	WriteFile(hFr, &nbItems, dwBytesToWrite, &dwBytesWritten, NULL) ;


	dwBytesToWrite = sizeof(struct str_Watcher);
	for(i=0; i<nbItems; i++)
	{
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		ListView_GetItem(hWatchList, &lvItem);
		tmpWatcher = (struct str_Watcher*) lvItem.lParam;


		//only save main watcher
		if ( !(tmpWatcher->size&WATCHER_SUBDATA) )
		{
			//WATCHER_STRUCT_MASK is saved with watcher, assuming user won't change the structures !!
			WriteFile(hFr, tmpWatcher, dwBytesToWrite, &dwBytesWritten, NULL) ;
		}
	}

	CloseHandle(hFr);

	Put_Info("Watchers saved", 1500);
}


BOOL ImportWatchers_KMod( HANDLE source)
{
	DWORD dwBytesToRead, dwBytesRead;
	WORD	i, nbItems;
	struct str_Watcher tmpWatcher;

	ListView_DeleteAllItems(hWatchList);

	dwBytesToRead = sizeof(nbItems);
	ReadFile(source, &nbItems, dwBytesToRead, &dwBytesRead, NULL) ;
	if (nbItems < 1)	return (FALSE);

	dwBytesToRead = sizeof(tmpWatcher);
	for(i=0; i<nbItems; i++)
	{
		ReadFile(source, &tmpWatcher, dwBytesToRead, &dwBytesRead, NULL) ;
		AddWatcher_KMod( tmpWatcher, i );
	}

	return (TRUE);

}

void LoadWatchers_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    


    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "Watchers (*.wch)\0*.wch\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = szWatchDir;//(LPSTR)NULL;
    szFile.lpstrTitle = "Load Watchers";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                     OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "wch";


	SetCurrentDirectory(Gens_Path);

    if (GetOpenFileName(&szFile)!=TRUE)   return;


    hFr = CreateFile (szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)	return;
	
	ImportWatchers_KMod( hFr );

	CloseHandle(hFr);

	Put_Info("Watchers loaded", 1500);
}


void ImportStructures_KMod( HANDLE hFr )
{
	DWORD i;

	CHAR type[6], name[20];
	BOOL	inStruct=FALSE;
	CHAR	curStruct=0, curElement = 0;


	ZeroMemory( StructureKMod, sizeof(StructureKMod) );

	if (!Parser_Load( hFr ))	return;

	for(i=0; i< Parser_NumLine( ); i++)
	{
		sscanf( (char *) trim( Parser_Line(i), " \t" ) , "%s %s", type, name);

		if ( lstrcmpi( type, "STRUCT" ) == 0 )
		{
			//new struct

			//Close previous one
			if (inStruct)	curStruct++;

			lstrcpyn( StructureKMod[curStruct].name, name, 20);
			StructureKMod[curStruct].nbElements = 0;

			inStruct = TRUE;			
			curElement= 0;
		}
		else if ( lstrcmpi( type, "END" ) == 0 )
		{
			//end struct
			if (inStruct)	curStruct++;
			inStruct = FALSE;
		}
		else if ( lstrcmpi( type, "CHAR" ) == 0 )
		{
			//char	
			if (inStruct)
			{
				lstrcpyn( StructureKMod[curStruct].elements[curElement].name, name, 20);
				StructureKMod[curStruct].elements[curElement].size = 1;

				curElement++;
				StructureKMod[curStruct].nbElements++;
			}
		}
		else if ( lstrcmpi( type, "SHORT" ) == 0 )
		{
			//short	
			if (inStruct)
			{
				lstrcpyn( StructureKMod[curStruct].elements[curElement].name, name, 20);
				StructureKMod[curStruct].elements[curElement].size = 2;

				curElement++;
				StructureKMod[curStruct].nbElements++;
			}			
		}
		else if ( lstrcmpi( type, "LONG" ) == 0 )
		{
			//long	
			if (inStruct)
			{
				lstrcpyn( StructureKMod[curStruct].elements[curElement].name, name, 20);
				StructureKMod[curStruct].elements[curElement].size = 4;

				curElement++;
				StructureKMod[curStruct].nbElements++;
			}			
		}

	}

	Parser_Unload( );
}

void AutoLoadWatchers_KMod( )
{
    char szFileName[MAX_PATH];
    HANDLE hFr;

	strcpy(szFileName, Rom_Dir);
	strcat(szFileName, Rom_Name);
	strcat(szFileName, ".wch");

    hFr = CreateFile (szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
	{
		//try in watcher dir
		strcpy(szFileName, szWatchDir);
		strcat(szFileName, Rom_Name);
		strcat(szFileName, ".wch");

		hFr = CreateFile (szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
		if (hFr == INVALID_HANDLE_VALUE)	return;
	}

	ImportWatchers_KMod( hFr );

	CloseHandle(hFr);

	strcpy(szFileName, Rom_Dir);
	strcat(szFileName, Rom_Name);
	strcat(szFileName, ".str");

    hFr = CreateFile (szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
	{
		//try in watcher dir
		strcpy(szFileName, szWatchDir);
		strcat(szFileName, Rom_Name);
		strcat(szFileName, ".str");

		hFr = CreateFile (szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
	
	}

	if (hFr != INVALID_HANDLE_VALUE)	ImportStructures_KMod( hFr );

	CloseHandle(hFr);


	Put_Info("Watchers auto loaded", 1500);
}


void ChangeWatcherType_KMod( int item, WORD size)
{
	LVITEM		lvItem;
	struct str_Watcher*	watcher;
	struct str_Watcher newWatcher;

	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = item;
	lvItem.iSubItem = 0;
	ListView_GetItem(hWatchList, &lvItem);
	watcher = (struct str_Watcher*) lvItem.lParam;
	if (!watcher)	return;

	//subelement can't be changed to struct
	if ( watcher->size & WATCHER_SUBDATA)	return;

	newWatcher.adr = watcher->adr;
	newWatcher.size = size;
	lstrcpy( newWatcher.name, watcher->name);


	DeleteWatcher_KMod( item ); //delete it (with all its possible sub data)

	AddWatcher_KMod( newWatcher, item); //recreate it
}

void ChangeWatcherStruct_KMod( int item, WORD structIdx)
{
/*
WATCHER_SUBDATA		0x8000
WATCHER_STRUCT_MASK	0x7F00 
WATCHER_SIZE_MASK	0x00FF
*/
     

	LVITEM		lvItem;
	struct str_Watcher*	watcher;
	struct str_Watcher newWatcher;

	DWORD	adr;
	CHAR	idx;
	CHAR	basename[71]; //50+20+.

	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = item;
	lvItem.iSubItem = 0;
	ListView_GetItem(hWatchList, &lvItem);
	watcher = (struct str_Watcher*) lvItem.lParam;
	if (!watcher)	return;

	//subelement can't be changed to struct
	if ( watcher->size & WATCHER_SUBDATA)	return;

	lstrcpy( basename, watcher->name);
	lstrcat( basename, ".");

	adr = watcher->adr;

	newWatcher.adr = adr;
	newWatcher.size = (structIdx+1) << 8;
	newWatcher.size |= 4; //to be update like a dword
	lstrcpy( newWatcher.name, watcher->name);


	DeleteWatcher_KMod( item ); //delete it (with all its possible sub data)

	AddWatcher_KMod( newWatcher, item); //recreate it

	//TODO move this to AddWatcher ?
	for (idx = 0; idx < StructureKMod[structIdx].nbElements; idx++)
	{
		ZeroMemory( &newWatcher, sizeof(newWatcher) );

		newWatcher.adr = adr;
		newWatcher.size = StructureKMod[structIdx].elements[idx].size | WATCHER_SUBDATA;
		lstrcpy( newWatcher.name, basename);
		lstrcat( newWatcher.name, StructureKMod[structIdx].elements[idx].name );

		AddWatcher_KMod( newWatcher, item+1+idx );

		adr += StructureKMod[structIdx].elements[idx].size;		
	}
}

void SeeWatcher_KMod( int item )
{
	LVITEM		lvItem;
	struct str_Watcher*	watcher;

	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = item;
	lvItem.iSubItem = 0;
	ListView_GetItem(hWatchList, &lvItem);
	watcher = (struct str_Watcher*) lvItem.lParam;
	if (!watcher)	return;

	OpenWindow_KMod( DMODE_68K );
	JumpM68KRam_KMod( watcher->adr - 0x00FF0000 );
}


void ResizeWatcher_KMod( HWND hDlg, WORD newWidth, WORD newHeight )
{

	HWND hwndButton;

	MoveWindow( hWatchList, 5, 5, newWidth-10, newHeight-40, TRUE);

	hwndButton = GetDlgItem( hDlg, IDC_WATCHER_ADD);
	MoveWindow( hwndButton, 5, newHeight-30, 90, 25, TRUE);
	hwndButton = GetDlgItem( hDlg, IDC_WATCHER_DEL);
	MoveWindow( hwndButton, 100, newHeight-30, 90, 25, TRUE);

	hwndButton = GetDlgItem( hDlg, IDC_WATCHER_SAVE);
	MoveWindow( hwndButton, newWidth-190, newHeight-30, 90, 25, TRUE);
	hwndButton = GetDlgItem( hDlg, IDC_WATCHER_LOAD);
	MoveWindow( hwndButton, newWidth-95, newHeight-30, 90, 25, TRUE);

	RedrawWindow( hDlg, NULL, NULL, RDW_INVALIDATE);
}


void WatcherInit_KMod( HWND hDlg)
{
	LV_COLUMN   lvColumn;
	int         i;
	TCHAR       szString[4][20] = {"Address (hex)", "Name", "Value", "Type"};

	hWatchList = GetDlgItem(hDlg, IDC_WATCHER_LIST);
	ListView_DeleteAllItems(hWatchList);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 84;
	for(i = 0; i < 4; i++)
	{
	   lvColumn.pszText = szString[i];
	   ListView_InsertColumn(hWatchList, i, &lvColumn);
	}

	ListView_SetExtendedListViewStyle(hWatchList, /*LVS_EX_FULLROWSELECT |*/ LVS_EX_GRIDLINES );
}


LRESULT ProcessCustomDraw (LPARAM lParam)
{
	HFONT hFontBold, hOldFont, hFont;
                        
    LOGFONT lf = { 0 };
    

    LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

	LVITEM		lvItem;
	struct str_Watcher*	watcher;

    switch(lplvcd->nmcd.dwDrawStage) 
    {
        case CDDS_PREPAINT : //Before the paint cycle begins
            //request notifications for individual listview items
            return CDRF_NOTIFYITEMDRAW;
            
        case CDDS_ITEMPREPAINT: //Before an item is drawn
			lvItem.mask = LVIF_PARAM;
			lvItem.iItem = (int)lplvcd->nmcd.dwItemSpec;
			lvItem.iSubItem = 0;
			ListView_GetItem(hWatchList, &lvItem);
			watcher = (struct str_Watcher*) lvItem.lParam;

			hFont = (HFONT) SendMessage(hWatchList, WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(LOGFONT), &lf);
			lf.lfItalic = TRUE;

			hFontBold = CreateFontIndirect(&lf);

            //if (((int)lplvcd->nmcd.dwItemSpec%2)==0)
			if (watcher->size & WATCHER_SUBDATA)
            {
                hOldFont = (HFONT) SelectObject(lplvcd->nmcd.hdc, hFontBold);
				DeleteObject(hFontBold);
                return CDRF_NEWFONT;
            }
            else{  
				DeleteObject(hFontBold);
                return CDRF_DODEFAULT;
            }
            break;

        //Before a subitem is drawn
        case CDDS_SUBITEM | CDDS_ITEMPREPAINT: 
			/*
            if ( ListView_GetNextItem(hWatchList, -1, LVNI_ALL | LVNI_SELECTED) == (int)lplvcd->nmcd.dwItemSpec)
            {
                if (0 == lplvcd->iSubItem)
                {
                    //customize subitem appearance for column 0
                    lplvcd->clrText   = RGB(255,0,0);
                    lplvcd->clrTextBk = RGB(255,255,255);

                    //To set a custom font:
                    //SelectObject(lplvcd->nmcd.hdc, 
                    //    <your custom HFONT>);

                    return CDRF_NEWFONT;
                }
                else if (1 == lplvcd->iSubItem)
                {
                    //customize subitem appearance for columns 1..n
                    //Note: setting for column i 
                    //carries over to columnn i+1 unless
                    //      it is explicitly reset
                    lplvcd->clrTextBk = RGB(255,0,0);
                    lplvcd->clrTextBk = RGB(255,255,255);

                    return CDRF_NEWFONT;
                }
            }*/
			break;
    }
    return CDRF_DODEFAULT;
}

BOOL CALLBACK WatcherDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LVITEM		lvItem;
    NMHDR		*pInfo;
	LPNMITEMACTIVATE lpnmitem;
	LVHITTESTINFO lvhi;
	static	int	curItem;

	HMENU		hPopup,hSubPopup;
	POINT		pt;
	WORD		sizeItem;

	int         i,idStruct;
    char		buf[64];
    static		HWND hEdit;
	unsigned long adrHexa;

	struct str_Watcher*	watcher;
	struct str_Watcher	defWatcher;
	
	switch(Message)
    {
		case WM_INITDIALOG:
			WatcherInit_KMod( hwnd );
			break;
		
		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case IDC_WATCHER_ADD:
					defWatcher.adr = 0;
					defWatcher.size = 4;
					strcpy(defWatcher.name, "New watcher");
					AddWatcher_KMod( defWatcher, 0 );
					break;

				case IDC_WATCHER_DEL:
					DeleteWatcher_KMod( -1 );
					break;

				case IDC_WATCHER_SAVE:
					SaveWatchers_KMod( hwnd );
					break;

				case IDC_WATCHER_LOAD:
					LoadWatchers_KMod( hwnd );
					break;
				
				case IDC_WATCHER_BYTE:
					ChangeWatcherType_KMod( curItem, 1);
					break;
				case IDC_WATCHER_WORD:
					ChangeWatcherType_KMod( curItem, 2);
					break;
				case IDC_WATCHER_DWORD:
					ChangeWatcherType_KMod( curItem, 4);
					break;
				case IDC_WATCHER_PTR:
					SeeWatcher_KMod( curItem );
					break;

				default:
					if ( (LOWORD(wParam) & 0x9900) == 0x9900 )
					{
						ChangeWatcherStruct_KMod( curItem, LOWORD(wParam) & 0x7F );
					}
			}
			break;
	   case WM_NOTIFY:
			pInfo = (NMHDR *) lParam;
			i = ListView_GetNextItem(hWatchList, -1, LVNI_ALL | LVNI_SELECTED);
			switch(pInfo->code)
			{	
				case NM_CUSTOMDRAW:
					SetWindowLong(hwnd, DWL_MSGRESULT, (LONG)ProcessCustomDraw(lParam));
					break;

				case NM_DBLCLK:
					if	((pInfo->hwndFrom == hWatchList) && (i>=0) )
					{
						lvItem.mask = LVIF_PARAM;
						lvItem.iItem = i;
						lvItem.iSubItem = 0;
						ListView_GetItem(hWatchList, &lvItem);
						watcher = (struct str_Watcher*) lvItem.lParam;
						if (watcher->size & WATCHER_SUBDATA)	return TRUE; //can't edit subdata watcher

						ListView_EditLabel(hWatchList, i);
					}
					break;

				case NM_RCLICK:
					if	(pInfo->hwndFrom == hWatchList)
					{ 
						lpnmitem = (LPNMITEMACTIVATE) lParam;
						 
						lvhi.pt = lpnmitem->ptAction; 
						ListView_SubItemHitTest(hWatchList , &lvhi); 
						if (lvhi.iItem !=-1)
						{
							curItem = lvhi.iItem;

							lvItem.mask = LVIF_PARAM;
							lvItem.iItem = curItem;
							lvItem.iSubItem = 0;
							ListView_GetItem(hWatchList, &lvItem);
							watcher = (struct str_Watcher*) lvItem.lParam;
							

							if (lvhi.iSubItem == 2)
							{
								// popupmenu on size
								if (watcher->size & WATCHER_SUBDATA)	return TRUE; //can't change subdata size

								sizeItem = (watcher->size & WATCHER_SIZE_MASK) -1;
								if (sizeItem == 3)	sizeItem--;
								

								idStruct = 0;								
								if (StructureKMod[idStruct].nbElements)	hSubPopup=CreatePopupMenu();

								for (idStruct=0; idStruct<126, StructureKMod[idStruct].nbElements; idStruct++)
								{
									AppendMenu(hSubPopup, MF_STRING, 0x9900+idStruct,  StructureKMod[idStruct].name);
									if (watcher->size & WATCHER_STRUCT_MASK)	CheckMenuRadioItem(hSubPopup, 0, 1, (watcher->size & WATCHER_STRUCT_MASK >> 8) -1, MF_BYPOSITION);

								}

								hPopup=CreatePopupMenu();
								AppendMenu(hPopup, MF_STRING, IDC_WATCHER_BYTE,  "Byte");
								AppendMenu(hPopup, MF_STRING, IDC_WATCHER_WORD,  "Word");
								AppendMenu(hPopup, MF_STRING, IDC_WATCHER_DWORD, "Dword");
								if ( !(watcher->size & WATCHER_STRUCT_MASK) )	CheckMenuRadioItem(hPopup, 0, 2, sizeItem, MF_BYPOSITION);

								if (hSubPopup) AppendMenu(hPopup, MF_POPUP, (UINT) hSubPopup,"Structure"); //for imported from NM2WCH

							}
							else if ( (watcher->adr >= 0x00FF0000) && (lvhi.iSubItem == 0) )
							{
								// popupmenu on address
								hPopup=CreatePopupMenu();
								AppendMenu(hPopup, MF_STRING, IDC_WATCHER_PTR, "Jump to...");
							}

							if (hPopup)
							{
								pt = lvhi.pt;
								ClientToScreen(pInfo->hwndFrom,&pt);

								TrackPopupMenu(hPopup, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);

								DestroyMenu(hPopup);
							}
						}
                   }
					// use GetCursorPos()+ScreenToClient() to get subitem or save it throught LVN_COLUMNCLICK or 
					// then ListView_SubItemHitTest(), ListView_GetSubItemRect(), ... to display it at the right position) 

					break;

                case LVN_BEGINLABELEDIT:
					if (i>=0)
					{
						lvItem.mask = LVIF_PARAM;
						lvItem.iItem = i;
						lvItem.iSubItem = 0;
						ListView_GetItem(hWatchList, &lvItem);
						watcher = (struct str_Watcher*) lvItem.lParam;
						if (!watcher)
						{
							SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
							return TRUE;
						}
						if (watcher->size & WATCHER_SUBDATA)
						{
							SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
							return TRUE;
						}
						hEdit = ListView_GetEditControl(hWatchList);
					}
					else
					{
						SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
						return TRUE;
					}
                    break;

                case LVN_ENDLABELEDIT:
					if (i>=0)
					{
						lvItem.mask = LVIF_PARAM;
						lvItem.iItem = i;
						lvItem.iSubItem = 0;
						ListView_GetItem(hWatchList, &lvItem);
						watcher = (struct str_Watcher*) lvItem.lParam;
						if (!watcher)	return TRUE;
						if (watcher->size & WATCHER_SUBDATA)	return TRUE; //can't edit subdata watcher


						/* SOR : FFFF20 = nb Special attack + life */
						GetWindowText(hEdit, buf, sizeof(buf));
						adrHexa = strtoul( buf, NULL, 16 );
						adrHexa = min(adrHexa, 0x00FFFFFF);
						adrHexa = max(adrHexa, 0x00000000);
						watcher->adr = adrHexa;
						wsprintf(buf, "%0.8X", adrHexa);

						lvItem.mask = LVIF_PARAM | LVIF_TEXT;
						lvItem.iItem = i;
						lvItem.iSubItem = 0;
						lvItem.pszText = buf;
						lvItem.lParam = (LPARAM) (watcher); //really needed ?
						ListView_SetItem(hWatchList, &lvItem);

						//UpdateWatchers_KMod( );
					}
                    break;
            }
            break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_WATCHERS );
			break;

		case WM_SIZE:
			ResizeWatcher_KMod( hwnd, LOWORD(lParam), HIWORD(lParam) );
			break;

		case WM_DESTROY:
			for(i=0; i< ListView_GetItemCount(hWatchList); i++)
			{
				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = i;
				lvItem.iSubItem = 0;
				ListView_GetItem(hWatchList, &lvItem);
				watcher = (struct str_Watcher *) lvItem.lParam;
				HeapFree(GetProcessHeap(), 0, watcher);
			}

			ListView_DeleteAllItems( hWatchList );

			DestroyWindow( hWatchers );
			PostQuitMessage(0);
			break;
		
		default:
            return FALSE;
    }
    return TRUE;
}

/**************** Layers *************************/
void LayersInit_KMod( )
{
	CheckDlgButton( hLayers,IDC_LAYER_A, BST_CHECKED);
	CheckDlgButton( hLayers,IDC_LAYER_B, BST_CHECKED);
	CheckDlgButton( hLayers,IDC_LAYER_SPRITE, BST_CHECKED);
	CheckDlgButton( hLayers,IDC_LAYER_WINDOW, BST_CHECKED);
	CheckDlgButton( hLayers,IDC_LAYER_32X, BST_CHECKED);
	ActiveLayer = 0x1F;
}


BOOL CALLBACK LayersDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
    {
		case WM_INITDIALOG:
			LayersInit_KMod( );
			break;
		
		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case IDC_LAYER_A:
					if ( IsDlgButtonChecked(hwnd,IDC_LAYER_A) == BST_CHECKED )
						ActiveLayer |= 0x08;
					else
						ActiveLayer &= 0x17;
					break;
				case IDC_LAYER_B:
					if ( IsDlgButtonChecked(hwnd,IDC_LAYER_B) == BST_CHECKED )
						ActiveLayer |= 0x04;
					else
						ActiveLayer &= 0x1B;
					break;
				case IDC_LAYER_SPRITE:
					if ( IsDlgButtonChecked(hwnd,IDC_LAYER_SPRITE) == BST_CHECKED )
						ActiveLayer |= 0x02;
					else
						ActiveLayer &= 0x1D;
					break;
				case IDC_LAYER_WINDOW:
					if ( IsDlgButtonChecked(hwnd,IDC_LAYER_WINDOW) == BST_CHECKED )
						ActiveLayer |= 0x01;
					else
						ActiveLayer &= 0x1E;
					break;
				case IDC_LAYER_32X:
					if ( IsDlgButtonChecked(hwnd,IDC_LAYER_32X) == BST_CHECKED )
						ActiveLayer |= 0x10;
					else
						ActiveLayer &= 0x0F;
					break;
			}
			break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_LAYERS );
			break;

		case WM_DESTROY:
			DestroyWindow( hLayers );
			PostQuitMessage(0);
			break;
		
		default:
            return FALSE;
    }
    return TRUE;
}


/*********** GMV **************/
HANDLE	hGMVFile;
BOOL	bGMVRecord;

void GMVStop_KMod( )
{
	if (hGMVFile == NULL)	return;

	CloseHandle( hGMVFile );
	hGMVFile = NULL;
	Put_Info("GMV Record/Play end", 2000);
}

void GMVPlay_KMod( )
{
	char szFileName[MAX_PATH];

	if (hGMVFile)	GMVStop_KMod( );

	// try to open it	
	strcpy(szFileName, Gens_Path);
	strcat(szFileName, Rom_Name);
	strcat(szFileName, ".gmv");

	hGMVFile = CreateFile (szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
	if (hGMVFile == INVALID_HANDLE_VALUE)
		return;		
		
	/* skip header */
	SetFilePointer(hGMVFile, 0x40, 0, FILE_BEGIN);

	bGMVRecord = FALSE;
	Put_Info("GMV Play start", 2000);
}

void GMVRecord_KMod( )
{
	char szFileName[MAX_PATH];
    DWORD dwBytesWritten;
	char szData[40];
	int tmp;

	strcpy(szFileName, Gens_Path);
	strcat(szFileName, Rom_Name);
	strcat(szFileName, ".gmv");


	if (hGMVFile)	GMVStop_KMod( );

	hGMVFile = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hGMVFile == INVALID_HANDLE_VALUE)
          return;

	strcpy(szData, "Gens Movie KMod1");
	WriteFile(hGMVFile, szData, 16, &dwBytesWritten, NULL) ;

	tmp =0;
	WriteFile(hGMVFile, &tmp, sizeof(tmp), &dwBytesWritten, NULL) ;
	WriteFile(hGMVFile, &tmp, sizeof(tmp), &dwBytesWritten, NULL) ;

	//ZeroMemory(szData, 40);
	strncpy(szData, Rom_Name, 40);
	WriteFile(hGMVFile, szData, 40, &dwBytesWritten, NULL) ;

	bGMVRecord = TRUE;
	Put_Info("GMV Record start", 2000);
}


void GMVUpdatePlay_KMod( )
{
    DWORD	dwBytesRead;
	BYTE	state;

	if (hGMVFile == NULL)	return;

	if (bGMVRecord)	return; //recording

	ReadFile(hGMVFile, &state, 1, &dwBytesRead, NULL);
	if (dwBytesRead == 0)
	{
		//EOF
		GMVStop_KMod( );
		return;
	}

	Controller_1_Up = state&0x01;
	state>>=1;
	Controller_1_Down = state&0x01;
	state>>=1;
	Controller_1_Left = state&0x01;
	state>>=1;
	Controller_1_Right = state&0x01;
	state>>=1;
	Controller_1_A = state&0x01;
	state>>=1;
	Controller_1_B = state&0x01;
	state>>=1;
	Controller_1_C = state&0x01;
	state>>=1;
	Controller_1_Start = state&0x01;
	
	ReadFile(hGMVFile, &state, 1, &dwBytesRead, NULL);
	/* ............ */
	
	ReadFile(hGMVFile, &state, 1, &dwBytesRead, NULL);
	/* ............ */
}

void GMVUpdateRecord_KMod( )
{
    DWORD	dwBytesWritten;
	BYTE	state;

	if (hGMVFile == NULL)	return;

	if (bGMVRecord == FALSE)	return; // playing

	state = Controller_1_Up;
	state +=(Controller_1_Down<<1);
	state +=(Controller_1_Left<<2);
	state +=(Controller_1_Right<<3);
	state +=(Controller_1_A<<4);
	state +=(Controller_1_B<<5);
	state +=(Controller_1_C<<6);
	state +=(Controller_1_Start<<7);
	WriteFile(hGMVFile, &state, 1, &dwBytesWritten, NULL) ;

	state = Controller_2_Up;
	state +=(Controller_2_Down<<1);
	state +=(Controller_2_Left<<2);
	state +=(Controller_2_Right<<3);
	state +=(Controller_2_A<<4);
	state +=(Controller_2_B<<5);
	state +=(Controller_2_C<<6);
	state +=(Controller_2_Start<<7);
	WriteFile(hGMVFile, &state, 1, &dwBytesWritten, NULL) ;

	state = Controller_1_X;
	state +=(Controller_1_Y<<1);
	state +=(Controller_1_Z<<2);
	state +=(Controller_1_Mode<<3);
	state +=(Controller_2_X<<4);
	state +=(Controller_2_Y<<5);
	state +=(Controller_2_Z<<6);
	state +=(Controller_2_Mode<<7);
	WriteFile(hGMVFile, &state, 1, &dwBytesWritten, NULL) ;
}

/*********** GMV Tools **************/
HANDLE	hGMVFile_In;
HANDLE	hGMVFile_Out;

void GMVT_BrowseIn_Init( HWND hwndParent, char *szFileName )
{
	if (hGMVFile_In)	CloseHandle(hGMVFile_In);
    hGMVFile_In = CreateFile (szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hGMVFile_In == INVALID_HANDLE_VALUE)  return;
	SetDlgItemText(hwndParent, IDC_GMVT_IN, szFileName);
	
	if (hGMVFile_Out)	CloseHandle(hGMVFile_Out);
	szFileName[ strlen(szFileName) -1] = 'c';
	hGMVFile_Out = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hGMVFile_Out == INVALID_HANDLE_VALUE)	return;
	SetDlgItemText(hwndParent, IDC_GMVT_OUT, szFileName);
}

void GMVT_BrowseIn( HWND hwndParent )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */


    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwndParent;
    szFile.lpstrFilter = "GMV File (*.gmv)\0*.gmv\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = Gens_Path;//(LPSTR)NULL;
    szFile.lpstrTitle = "Open GMV file";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "gmv";

	SetCurrentDirectory(Gens_Path);

    if (GetOpenFileName(&szFile)!=TRUE)   return;

	GMVT_BrowseIn_Init( hwndParent, szFileName);
}

void GMVT_BrowseOut( HWND hwndParent )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwndParent;
    szFile.lpstrFilter = "GMV compressed file (*.gmc)\0*.gmc\0Binary\0*.*\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = Gens_Path;//(LPSTR)NULL;
    szFile.lpstrTitle = "Save GMV compressed";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "gmc";

	SetCurrentDirectory(Gens_Path);

    if (GetSaveFileName(&szFile)!=TRUE)   return;

	if (hGMVFile_Out)	CloseHandle(hGMVFile_Out);
    hGMVFile_Out = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hGMVFile_Out == INVALID_HANDLE_VALUE)	return;
	SetDlgItemText(hwndParent, IDC_GMVT_OUT, szFileName);
}

BOOL GMVTools_Convert(UINT comp, UINT p1, UINT p2, UINT b6)
{
	DWORD	dwBytesRead, dwBytesWritten;
	BYTE	oldstate[3], cnt, state[3];

	if (!hGMVFile_In)	return FALSE;
	if (!hGMVFile_Out)	return FALSE;

	cnt = 0;
	if (p1)		cnt|=0x01;
	if (p2)		cnt|=0x02;
	if (b6)		cnt|=0x04;
	if (comp)	cnt|=0x80;	
	WriteFile(hGMVFile_Out, &cnt, 1, &dwBytesWritten, NULL);
	SetFilePointer(hGMVFile_Out, 0x10, 0, FILE_BEGIN); //keep some space

	oldstate[0] = 0xFF;
	oldstate[1] = 0xFF;
	oldstate[2] = 0xFF;	
	cnt = 0;
	

	/* skip header */
	SetFilePointer(hGMVFile_In, 0x40, 0, FILE_BEGIN);	

	do
	{
		ReadFile(hGMVFile_In, state, 3, &dwBytesRead, NULL);
		if (!p2)
		{
			state[2] &= 0x0F; //skip p2 data
		}
		else if (!p1)
		{
			state[2] &= 0xF0; //skip p1 data
			state[2] >>= 4;
		}

		if (dwBytesRead == 3)
		{
			if (comp)
			{
				if ( (oldstate[0] == state[0]) && (oldstate[1] == state[1]) && (oldstate[2] == state[2]) && (cnt < 0xFF))
					cnt++;
				else
				{
					if (cnt) // to skip if first keyinput
					{
						WriteFile(hGMVFile_Out, &cnt, 1, &dwBytesWritten, NULL);
						if (p1)	WriteFile(hGMVFile_Out, &oldstate[0], 1, &dwBytesWritten, NULL);
						if (p2)	WriteFile(hGMVFile_Out, &oldstate[1], 1, &dwBytesWritten, NULL);
						if (b6)	WriteFile(hGMVFile_Out, &oldstate[2], 1, &dwBytesWritten, NULL);
					}
					cnt = 1;
					oldstate[0] = state[0];
					oldstate[1] = state[1];
					oldstate[2] = state[2];
				}

			}
			else
			{
				if (p1)	WriteFile(hGMVFile_Out, &state[0], 1, &dwBytesWritten, NULL);
				if (p2)	WriteFile(hGMVFile_Out, &state[1], 1, &dwBytesWritten, NULL);
				if (b6)	WriteFile(hGMVFile_Out, &state[2], 1, &dwBytesWritten, NULL);
			}
		}
	}
	while( dwBytesRead == 3);

	if (comp && cnt)
	{
		WriteFile(hGMVFile_Out, &cnt, 1, &dwBytesWritten, NULL);
		if (p1)	WriteFile(hGMVFile_Out, &oldstate[0], 1, &dwBytesWritten, NULL);
		if (p2)	WriteFile(hGMVFile_Out, &oldstate[1], 1, &dwBytesWritten, NULL);
		if (b6)	WriteFile(hGMVFile_Out, &oldstate[2], 1, &dwBytesWritten, NULL);
	}		

	Spy_KMod( "Ok");

	return TRUE;
}

BOOL CALLBACK GMVToolsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char szFileName[MAX_PATH];

	switch(Message)
    {
		case WM_INITDIALOG:	
			//GMV default
			CheckDlgButton(hwnd, IDC_GMVT_P1, BST_CHECKED);
			CheckDlgButton(hwnd, IDC_GMVT_P2, BST_CHECKED);
			CheckDlgButton(hwnd, IDC_GMVT_6BUTTONS, BST_CHECKED);

			if (Game)
			{
				strcpy(szFileName, Gens_Path);
				strcat(szFileName, Rom_Name);
				strcat(szFileName, ".gmv");

				GMVT_BrowseIn_Init( hwnd, szFileName );
				
			}
			break;


		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_GMVT_BROWSE_IN:
					GMVT_BrowseIn( hwnd );
					break;

				case IDC_GMVT_BROWSE_OUT:
					GMVT_BrowseOut( hwnd );
					break;

				case IDOK:
					if ( GMVTools_Convert(IsDlgButtonChecked(hwnd, IDC_GMVT_COMP),IsDlgButtonChecked(hwnd, IDC_GMVT_P1),
									IsDlgButtonChecked(hwnd, IDC_GMVT_P2), IsDlgButtonChecked(hwnd, IDC_GMVT_6BUTTONS))	)
						EndDialog(hwnd, IDOK);
					break;

				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
					break;
			}
			break;

		default:
            return FALSE;
    }
    return TRUE;
}



void GMVTools_KMod( )
{
	DialogBox(ghInstance, MAKEINTRESOURCE(IDD_GMVTOOLS), HWnd, GMVToolsDlgProc);

	if (hGMVFile_In)	CloseHandle(hGMVFile_In);
	if (hGMVFile_Out)	CloseHandle(hGMVFile_Out);
}

/*********** M68K **************/
int Current_PC_M68K;
			
unsigned short Next_Word_M68K(void)
{
	unsigned short val;
	
	val = M68K_RW(Current_PC_M68K);

	Current_PC_M68K += 2;

	return(val);
}


unsigned int Next_Long_M68K(void)
{
	unsigned int val;
	
	val = M68K_RW(Current_PC_M68K);
	val <<= 16;
	val |= M68K_RW(Current_PC_M68K + 2);

	Current_PC_M68K += 4;

	return(val);
}


unsigned char M68_ViewMode;
unsigned int  M68k_StartLineDisasm, M68k_StartLineRAM, M68k_StartLineROM;



void SwitchM68kViewMode_KMod( )
{
	SCROLLINFO si;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
	si.nMin   = 0; 
	si.nPage  = 13;

	if (M68_ViewMode == 0)
	{
		// DISASM VIEW
		si.nPos   = M68k_StartLineDisasm; 
		si.nMax   = Rom_Size -1;
	}
	else if (M68_ViewMode == 1)
	{
		// ROM VIEW
		si.nPos   = M68k_StartLineROM; 
		si.nMax   = (Rom_Size/8) -1;
	}
	else
	{
		// RAM VIEW
		si.nPos   = M68k_StartLineRAM; 
		si.nMax   = (0x10000/8) -1;
	}
	SetScrollInfo(GetDlgItem(hM68K, IDC_68K_SCROLL), SB_CTL, &si, TRUE); 
}

void JumpM68KRam_KMod( DWORD adr )
{
	M68_ViewMode = 2; //RAM
	M68k_StartLineRAM = adr/8;
	SwitchM68kViewMode_KMod( );
}

void UpdateM68k_KMod( )
{
	unsigned int i, PC;	
	unsigned char tmp_string[256];

	if ( OpenedWindow_KMod[ 0 ] == FALSE )	return;

	
	SendDlgItemMessage(hM68K , IDC_68K_DISAM, LB_RESETCONTENT, (WPARAM) 0 , (LPARAM) 0);
	if (M68_ViewMode == 0)
	{
		M68k_StartLineDisasm =	GetScrollPos(GetDlgItem(hM68K, IDC_68K_SCROLL) ,SB_CTL);
		Current_PC_M68K = M68k_StartLineDisasm; //main68k_context.pc;
		for(i = 0; i < 13; i++)
		{
			PC = Current_PC_M68K;
			wsprintf(debug_string, "%.5X    %-33s", PC, M68KDisasm(Next_Word_M68K, Next_Long_M68K));
			SendDlgItemMessage(hM68K , IDC_68K_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
		}
	}
	else if (M68_ViewMode == 1)
	{
		Byte_Swap(Rom_Data, Rom_Size);
		M68k_StartLineROM = GetScrollPos(GetDlgItem(hM68K, IDC_68K_SCROLL) ,SB_CTL);
		for(i = 0; i < 13; i++)
		{
			wsprintf(tmp_string, "%.5X ", M68k_StartLineROM*8 + i*8);
			tmp_string[5] = 0x20;
			Hexview( (unsigned char *) (Rom_Data + M68k_StartLineROM*8 + i*8), tmp_string + 6);
			tmp_string[23] = 0x20;
			Ansiview( (unsigned char *) (Rom_Data + M68k_StartLineROM*8 + i*8), tmp_string + 24);
			wsprintf(debug_string, "%s", tmp_string);
			SendDlgItemMessage(hM68K , IDC_68K_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
		}
		Byte_Swap(Rom_Data, Rom_Size);
	}
	else if (M68_ViewMode == 2)
	{
		Byte_Swap(Ram_68k, Rom_Size);
		M68k_StartLineRAM = GetScrollPos(GetDlgItem(hM68K, IDC_68K_SCROLL) ,SB_CTL);
		for(i = 0; i < 13; i++)
		{
			wsprintf(tmp_string, "%.4X ", M68k_StartLineRAM*8 + i*8);
			tmp_string[4] = 0x20;
			Hexview( (unsigned char *) (Ram_68k + M68k_StartLineRAM*8 + i*8), tmp_string + 5);
			tmp_string[22] = 0x20;
			Ansiview( (unsigned char *) (Ram_68k + M68k_StartLineRAM*8 + i*8), tmp_string + 23);
			wsprintf(debug_string, "%s", tmp_string);
			SendDlgItemMessage(hM68K , IDC_68K_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
		}
		Byte_Swap(Ram_68k, Rom_Size);
	}
	wsprintf(debug_string, "X=%d N=%d Z=%d V=%d C=%d  SR=%.4X Cycles=%.10d",(main68k_context.sr & 0x10)?1:0, (main68k_context.sr & 0x8)?1:0, (main68k_context.sr & 0x4)?1:0, (main68k_context.sr & 0x2)?1:0, (main68k_context.sr & 0x1)?1:0, main68k_context.sr, main68k_context.odometer);
	SendDlgItemMessage(hM68K , IDC_68K_STATUS_SR, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "A0=%.8X A1=%.8X A2=%.8X A3=%.8X\nA4=%.8X A5=%.8X A6=%.8X A7=%.8X\n", main68k_context.areg[0], main68k_context.areg[1], main68k_context.areg[2], main68k_context.areg[3], main68k_context.areg[4], main68k_context.areg[5], main68k_context.areg[6], main68k_context.areg[7]);
	SendDlgItemMessage(hM68K , IDC_68K_STATUS_ADR, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "D0=%.8X D1=%.8X D2=%.8X D3=%.8X\nD4=%.8X D5=%.8X D6=%.8X D7=%.8X\n", main68k_context.dreg[0], main68k_context.dreg[1], main68k_context.dreg[2], main68k_context.dreg[3], main68k_context.dreg[4], main68k_context.dreg[5], main68k_context.dreg[6], main68k_context.dreg[7]);
	SendDlgItemMessage(hM68K , IDC_68K_STATUS_DATA, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "PC=%.8X", main68k_context.pc);
	SendDlgItemMessage(hM68K, IDC_68K_PC, WM_SETTEXT, (WPARAM)0, (LPARAM) debug_string);

/*
	sprintf(GString, "Bank for Z80 = %.8X\n", Bank_Z80);
 */
}


void Dump68KRom_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_68K");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "ROM dump (*.bin)\0*.bin\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump Genesis ROM";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "bin";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = Rom_Size;
	Byte_Swap(Rom_Data, Rom_Size);
    WriteFile(hFr, Rom_Data, dwBytesToWrite, &dwBytesWritten, NULL) ;
	Byte_Swap(Rom_Data, Rom_Size);

    CloseHandle(hFr);

	Put_Info("Genesis ROM dumped", 1500);
}


void Dump68K_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_68K");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump 68K memory";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "ram";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 64 * 1024;
	Byte_Swap(Ram_68k, Rom_Size);
    WriteFile(hFr, Ram_68k, dwBytesToWrite, &dwBytesWritten, NULL) ;
	Byte_Swap(Ram_68k, Rom_Size);

    CloseHandle(hFr);

	Put_Info("68K RAM dumped", 1500);
}

BOOL CALLBACK M68KDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;
	SCROLLINFO si;

	switch(Message)
    {
		case WM_INITDIALOG:			
			hFont = (HFONT) GetStockObject(OEM_FIXED_FONT)	;
			SendDlgItemMessage(hwnd, IDC_68K_DISAM, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_68K_STATUS_SR, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_68K_STATUS_ADR, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_68K_STATUS_DATA, WM_SETFONT, (WPARAM)hFont, TRUE);
			M68_ViewMode = 0; //disasm
			M68k_StartLineDisasm= M68k_StartLineRAM= M68k_StartLineROM = 0;
			SwitchM68kViewMode_KMod(  ); // init with wrong values (since no game loaded by default)
			break;

		case WM_SHOWWINDOW:
			SwitchM68kViewMode_KMod(  );
			break;


		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_68K_DUMP_ROM:
					Dump68KRom_KMod( hwnd );
					break;
				case IDC_68K_DUMP_RAM:
					Dump68K_KMod( hwnd );
					break;
				case IDC_68K_VIEW_ROM:
					M68_ViewMode = !M68_ViewMode;
					if (M68_ViewMode == 0)
					{					
						SendDlgItemMessage(hwnd, IDC_68K_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View ROM");
					}
					else
					{
						SendDlgItemMessage(hwnd, IDC_68K_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
					}
					SwitchM68kViewMode_KMod( );
					UpdateWindow (hwnd); 
					UpdateM68k_KMod( );
					break;
				case IDC_68K_VIEW_RAM:
					M68_ViewMode = 2; //RAM
					SendDlgItemMessage(hwnd, IDC_68K_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
					SwitchM68kViewMode_KMod( );
					UpdateWindow (hwnd); 
					UpdateM68k_KMod( );
					break;

				case IDC_68K_PC:
					if (M68_ViewMode == 0)
					{
						M68k_StartLineDisasm = main68k_context.pc;
						SwitchM68kViewMode_KMod( );
						UpdateWindow (hwnd); 
						UpdateM68k_KMod( );
					}
					break;

			}
			break;

		case WM_VSCROLL:
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_ALL; //SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS; 
			GetScrollInfo((HWND) lParam, SB_CTL, &si);
			switch(LOWORD (wParam)) 
			{ 
				case SB_PAGEUP: 
					 si.nPos -= si.nPage; 
					 break; 
				case SB_PAGEDOWN: 
					 si.nPos += si.nPage;
					 break; 
				case SB_LINEUP: 
					 si.nPos--; 
					 break;
				case SB_LINEDOWN: 
					 si.nPos++; 
					 break; 
//				case SB_THUMBPOSITION:
				case SB_THUMBTRACK: 
					// don't change!! else you'll loose the 32bits value for a 16bits value
//					 si.nPos = HIWORD(wParam); 
					 si.nPos = si.nTrackPos;
					 break; 
			} 
 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_POS; 
			SetScrollInfo((HWND) lParam, SB_CTL, &si, TRUE); 
			UpdateWindow (hwnd); 
			UpdateM68k_KMod( );
			return 0; 

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_68K );
			break;

		case WM_DESTROY:
			DeleteObject( (HGDIOBJ) hFont );
			DestroyWindow(hM68K);
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;

}

/************ Z80 **************/
BOOL Z80_ViewMode;
unsigned int  Z80_StartLineDisasm, Z80_StartLineMem;

void SwitchZ80ViewMode_KMod( )
{
	SCROLLINFO si; 

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
	si.nMin   = 0; 
	si.nPage  = 13;

	if (Z80_ViewMode)
	{
		// MEM VIEW
		si.nPos   = Z80_StartLineMem; 
		si.nMax   = (1024) -1;
	}
	else
	{
		// DISASM VIEW
		si.nPos   = Z80_StartLineDisasm; 
		si.nMax   = (1024*8) -1;
	}
	SetScrollInfo(GetDlgItem(hZ80, IDC_Z80_SCROLL), SB_CTL, &si, TRUE); 
}

void UpdateZ80_KMod( )
{
	unsigned int i, PC;
	unsigned char tmp_string[256];

	if ( OpenedWindow_KMod[ 1 ] == FALSE )	return;

	
	SendDlgItemMessage(hZ80 , IDC_Z80_DISAM, LB_RESETCONTENT, (WPARAM) 0 , (LPARAM) 0);
	if (Z80_ViewMode == 0)
	{
		Z80_StartLineDisasm =	GetScrollPos(GetDlgItem(hZ80, IDC_Z80_SCROLL) ,SB_CTL);
		PC = Z80_StartLineDisasm; //z80_Get_PC(&M_Z80);
		for(i = 0; i < 13; i++)
		{
			z80dis((unsigned char *)Ram_Z80, (int *)&PC, tmp_string);
			// to skip the \n z80dis add
			lstrcpyn(debug_string, tmp_string, lstrlen(tmp_string));
			SendDlgItemMessage(hZ80 , IDC_Z80_DISAM, LB_INSERTSTRING, i, (LPARAM)debug_string);
		}
	}
	else
	{
		Z80_StartLineMem =	GetScrollPos(GetDlgItem(hZ80, IDC_Z80_SCROLL) ,SB_CTL);
		for(i = 0; i < 13; i++)
		{
			wsprintf(tmp_string, "%.4X ", Z80_StartLineMem*8 + i*8);
			tmp_string[4] = 0x20;
			Hexview( (unsigned char *) (Ram_Z80 + Z80_StartLineMem*8 + i*8), tmp_string + 5);
			tmp_string[22] = 0x20;
			Ansiview( (unsigned char *) (Ram_Z80 + Z80_StartLineMem*8 + i*8), tmp_string + 23);
			wsprintf(debug_string, "%s", tmp_string);
			SendDlgItemMessage(hZ80 , IDC_Z80_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
		}
	}


	wsprintf(debug_string, "AF =%.4X BC =%.4X DE =%.4X HL =%.4X\nAF2=%.4X BC2=%.4X DE2=%.4X HL2=%.4X", z80_Get_AF(&M_Z80), M_Z80.BC.w.BC, M_Z80.DE.w.DE, M_Z80.HL.w.HL, z80_Get_AF2(&M_Z80), M_Z80.BC2.w.BC2, M_Z80.DE2.w.DE2, M_Z80.HL2.w.HL2);
	SendDlgItemMessage(hZ80 , IDC_Z80_STATUS_RS1, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "IX =%.4X IY =%.4X SP =%.4X ", M_Z80.IX.w.IX, M_Z80.IY.w.IY, M_Z80.SP.w.SP);
	SendDlgItemMessage(hZ80 , IDC_Z80_STATUS_RS3, WM_SETTEXT, 0 , (LPARAM)debug_string);
	
	wsprintf(debug_string, "IFF1=%d IFF2=%d I=%.2X R=%.2X IM=%.2X", M_Z80.IFF.b.IFF1, M_Z80.IFF.b.IFF2, M_Z80.I, M_Z80.R.b.R1, M_Z80.IM);
	SendDlgItemMessage(hZ80 , IDC_Z80_STATUS_MISC, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "S=%d Z=%d Y=%d H=%d X=%d P=%d N=%d C=%d", (z80_Get_AF(&M_Z80) & 0x80) >> 7, (z80_Get_AF(&M_Z80) & 0x40) >> 6, (z80_Get_AF(&M_Z80) & 0x20) >> 5, (z80_Get_AF(&M_Z80) & 0x10) >> 4, (z80_Get_AF(&M_Z80) & 0x08) >> 3, (z80_Get_AF(&M_Z80) & 0x04) >> 2, (z80_Get_AF(&M_Z80) & 0x02) >> 1, (z80_Get_AF(&M_Z80) & 0x01) >> 0);
	SendDlgItemMessage(hZ80 , IDC_Z80_STATUS_FLAG, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "Bank = %0.8X State=%.2X", Bank_Z80, Z80_State);
	SendDlgItemMessage(hZ80 , IDC_Z80_STATUS_BANK, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "PC=%.4X", z80_Get_PC(&M_Z80));
	SendDlgItemMessage(hZ80 , IDC_Z80_PC, WM_SETTEXT, 0 , (LPARAM)debug_string);

/*
	sprintf(GString, "Status=%.2X ILine=%.2X IVect=%.2X\n", M_Z80.Status & 0xFF, M_Z80.IntLine, M_Z80.IntVect);
	sprintf(GString, "Bank68K=%.8X State=%.2X\n", M_Z80.Status & 0xFF, Bank_M68K, Z80_State);
*/

}

void DumpZ80_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_Z80");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump Z80 memory";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "ram";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 8 * 1024;
    WriteFile(hFr, Ram_Z80, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	Put_Info("Z80 Ram dumped", 1500);
}

BOOL CALLBACK Z80DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;
	SCROLLINFO si; 

	switch(Message)
    {
		case WM_INITDIALOG:			
			hFont = (HFONT) GetStockObject(OEM_FIXED_FONT);
			SendDlgItemMessage(hwnd, IDC_Z80_DISAM, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_Z80_STATUS_RS1, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_Z80_STATUS_RS2, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_Z80_STATUS_RS3, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_Z80_STATUS_MISC, WM_SETFONT, (WPARAM)hFont, TRUE);	
			SendDlgItemMessage(hwnd, IDC_Z80_STATUS_FLAG, WM_SETFONT, (WPARAM)hFont, TRUE);	
			SendDlgItemMessage(hwnd, IDC_Z80_STATUS_BANK, WM_SETFONT, (WPARAM)hFont, TRUE);	
			
			Z80_ViewMode = 0; //disasm
			Z80_StartLineDisasm = Z80_StartLineMem = 0;
			SwitchZ80ViewMode_KMod(  ); // init with wrong values (since no game loaded by default)
			break;

		case WM_SHOWWINDOW:
			SwitchZ80ViewMode_KMod(  );
			break;

		case WM_COMMAND:
			switch( LOWORD(wParam))
			{
				case IDC_Z80_DUMP_MEM:
					DumpZ80_KMod( hwnd );
					break;

				case IDC_Z80_VIEW_MEM :
					Z80_ViewMode = !Z80_ViewMode;
					if (Z80_ViewMode == 0)
					{					
						SendDlgItemMessage(hwnd, IDC_Z80_VIEW_MEM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Memory");
					}
					else
					{
						SendDlgItemMessage(hwnd, IDC_Z80_VIEW_MEM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
					}
 					SwitchZ80ViewMode_KMod( );
					UpdateWindow (hwnd); 
					UpdateZ80_KMod( );
					break;

				case IDC_Z80_PC:
					if (Z80_ViewMode == 0)
					{
						Z80_StartLineDisasm = z80_Get_PC(&M_Z80);
						SwitchZ80ViewMode_KMod( );
						UpdateWindow (hwnd); 
						UpdateZ80_KMod( );
					}
					break;
			}
					
			break;

		case WM_VSCROLL:
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_ALL; 
			GetScrollInfo((HWND) lParam, SB_CTL, &si);
			switch(LOWORD (wParam)) 
			{ 
				case SB_PAGEUP: 
					 si.nPos -= si.nPage; 
					 break; 
				case SB_PAGEDOWN: 
					 si.nPos += si.nPage;
					 break; 
				case SB_LINEUP: 
					 si.nPos--; 
					 break;
				case SB_LINEDOWN: 
					 si.nPos++; 
					 break; 
				case SB_THUMBTRACK: 
					 //si.nPos = HIWORD(wParam); 
					 si.nPos = si.nTrackPos;
					 break; 
			} 
 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_POS; 
			SetScrollInfo((HWND) lParam, SB_CTL, &si, TRUE); 
			UpdateWindow (hwnd); 
			UpdateZ80_KMod( );
			return 0; 


		case WM_CLOSE:
			CloseWindow_KMod( DMODE_Z80 );
			break;

		case WM_DESTROY:
			DeleteObject( (HGDIOBJ) hFont );
			DestroyWindow(hZ80);
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;
}


/************ VDP **************/
long palH, palV;
UCHAR pal_KMod, zoomTile_KMod, TileCurDraw; //, tileBank_KMod;
USHORT tile_KMod;

HWND hWndRedArrow;
HWND hWndPal;
RECT rcArrow, rcPal, rcTiles;

COLORREF GetPal_KMod( unsigned char numPal, unsigned numColor )
{
	/* can't use MD_Palette since it's DirectX mode (555 or 565) */
	/* !! CRAM is (binary:)GGG0RRR00000BBB0   while COLORREF is (hexa:)0x00BBGGRR */
	unsigned short int md_color;
	COLORREF newColor;

	/* B */
	newColor = (COLORREF) CRam[ 2*16*numPal + 2*numColor + 1];
	newColor <<= 20; 

	/* G */
	md_color = (unsigned short int) ( CRam[ 2*16*numPal + 2*numColor]&0xF0 );
	md_color <<= 8;
	newColor |= md_color; 

	/* R */
	md_color = (unsigned short int) ( CRam[ 2*16*numPal + 2*numColor]&0x0F );
	md_color <<= 4;
	newColor |= md_color; 

	if (newColor == 0x00E0E0E0)
		newColor = 0x00FFFFFF;


	// handle our false pal ;)
	if (numPal==4)
		newColor = PALETTEINDEX(numColor);

	return newColor;

}

void DrawTile_KMod( HDC hDCMain, unsigned short int numTile, WORD x, WORD y, UCHAR pal, UCHAR zoom)
{
	unsigned char j;
	unsigned char TileData;
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC( hDCMain );
	hBitmap = CreateCompatibleBitmap( hDCMain, 8, 8);
	hOldBitmap = SelectObject(hDC, hBitmap);


	for(j = 0; j < 8; j++)
	{
		TileData = VRam[numTile*32 + j*4 + 1]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 0, j, GetPal_KMod(pal, TileData) );

		TileData = VRam[numTile*32 + j*4 + 1]&0x0F;
		SetPixelV(hDC, 1, j, GetPal_KMod(pal, TileData) );

		TileData = VRam[numTile*32 + j*4 ]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 2, j, GetPal_KMod(pal, TileData) );

		TileData = VRam[numTile*32 + j*4 ]&0x0F;
		SetPixelV(hDC, 3, j, GetPal_KMod(pal, TileData) );

		TileData = VRam[numTile*32 + j*4 + 3]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 4, j, GetPal_KMod(pal, TileData) );

		TileData = VRam[numTile*32 + j*4 + 3]&0x0F;
		SetPixelV(hDC, 5, j, GetPal_KMod(pal, TileData) );

		TileData = VRam[numTile*32 + j*4 + 2]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 6, j, GetPal_KMod(pal, TileData) );

		TileData = VRam[numTile*32 + j*4 + 2]&0x0F;
		SetPixelV(hDC, 7, j, GetPal_KMod(pal, TileData) );
	}	

	StretchBlt(
		hDCMain, // handle to destination device context
		x,  // x-coordinate of destination rectangle's upper-left 
               // corner
		y,  // y-coordinate of destination rectangle's upper-left 
               // corner
		8*zoom,  // width of destination rectangle
		8*zoom, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left 
			// corner
		0,   // y-coordinate of source rectangle's upper-left 
               // corner
		8,
		8,
		SRCCOPY  // raster operation code
	);


	SelectObject(hDC, hOldBitmap);
    DeleteObject( hBitmap );

	DeleteDC( hDC );
}

void UpdateVDP_KMod( )
{
	if ( OpenedWindow_KMod[ 2 ] == FALSE )	return;

    RedrawWindow( hWndPal, NULL, NULL, RDW_INVALIDATE);
    RedrawWindow( GetDlgItem(hVDP, IDC_VDP_TILES), NULL, NULL, RDW_INVALIDATE);
    RedrawWindow( GetDlgItem(hVDP, IDC_VDP_PREVIEW), NULL, NULL, RDW_INVALIDATE);

	TileCurDraw++;
	if (TileCurDraw > 3)	TileCurDraw = 0;
}

void DrawVDP_Pal_KMod( LPDRAWITEMSTRUCT hlDIS  )
{
	unsigned char i, j;
	HBRUSH newBrush = NULL;
	RECT rc;

	for(j = 0; j < 5; j++)
	{
		rc.top = j* palV;
		rc.bottom = rc.top + palV;

		for(i = 0; i < 16; i++)
		{
			if (newBrush)	DeleteObject( (HGDIOBJ) newBrush );
			rc.left = i*palH;
			rc.right = rc.left + palH;

			newBrush =  CreateSolidBrush( GetPal_KMod(j, i) );
			
			FillRect(hlDIS->hDC, &rc, newBrush);
		}
	}

	if (newBrush)	DeleteObject( (HGDIOBJ) newBrush );

	MoveWindow(hWndRedArrow, rcPal.left - (rcArrow.right - rcArrow.left), rcPal.top + pal_KMod*palV + ( palV - (rcArrow.bottom - rcArrow.top))/2, (rcArrow.right - rcArrow.left), (rcArrow.bottom - rcArrow.top), TRUE);
}


void DrawVDP_Tiles_KMod( LPDRAWITEMSTRUCT hlDIS  )
{

	WORD posX, posY, maxY, maxX;
	unsigned short int numTile = 0;
	SCROLLINFO si;
   
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC( hlDIS->hDC );
	hBitmap = CreateCompatibleBitmap( hlDIS->hDC,  hlDIS->rcItem.right - hlDIS->rcItem.left, hlDIS->rcItem.bottom - hlDIS->rcItem.top);
	hOldBitmap = SelectObject(hDC, hBitmap);
	
	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_POS; 
	GetScrollInfo((HWND) GetDlgItem(hVDP, IDC_VDP_SCROLL), SB_CTL, &si);

	numTile = si.nPos*16; //tileBank_KMod*16;
	if (Paused)
	{
		maxY = 32*8;
	}
	else
	{
		/* let's speed up emulation by redrawing 1/4 each refresh */
		maxY = 8*8;
		numTile += TileCurDraw*16*8;
	}

	maxX = 16*8;

	for(posY = 0; posY < maxY; posY+=8)
	{
		for(posX = 0; posX < maxX; posX+=8)
		{
			DrawTile_KMod( hDC, numTile++, posX, posY, pal_KMod, 1);
		}
	}


	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left 
               // corner
		(Paused?0:TileCurDraw*maxY),  // y-coordinate of destination rectangle's upper-left 
               // corner
		maxX,  // width of destination rectangle
		maxY, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left 
               // corner
		0,   // y-coordinate of source rectangle's upper-left 
               // corner
		SRCCOPY  // raster operation code
	);

	SelectObject(hDC, hOldBitmap);
    DeleteObject( hBitmap );

	DeleteDC( hDC );
}

void DrawVDPZoom_Tiles_KMod( LPDRAWITEMSTRUCT hlDIS  )
{
	SCROLLINFO si;
	
	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_POS; 
	GetScrollInfo((HWND) GetDlgItem(hVDP, IDC_VDP_SCROLL), SB_CTL, &si);

	DrawTile_KMod( hlDIS->hDC, (si.nPos*16) + tile_KMod, 0, 0, pal_KMod, zoomTile_KMod);
}

void DumpTiles_KMod( HWND hwnd )
{

	BITMAPFILEHEADER	bmfh;
	BITMAPINFOHEADER	bmiHeader;
	RGBQUAD				bmiColors[16];
	LPBYTE				pBits ;

	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
	HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

	WORD posX, posY;

	COLORREF		tmpColor;
	DWORD			j;
	
	unsigned short int numTile;
	unsigned char		TileData;


    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_VRAM");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "16Colors Bitmap file (*.bmp)\0*.bmp\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Convert VRAM to bitmap";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "bmp";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

	bmfh.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
	bmfh.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) +  sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) + 64 * 2000/2); // 4bits per pixel
	bmfh.bfOffBits = (DWORD) (sizeof(BITMAPFILEHEADER) +  sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) );


	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = 16*8;
    bmiHeader.biHeight = 125*8;
    bmiHeader.biPlanes = 0;
    bmiHeader.biBitCount = 4;
    bmiHeader.biClrUsed = 16;
    bmiHeader.biCompression = BI_RGB;
    bmiHeader.biSizeImage = 64 * 2000/2;
    bmiHeader.biClrImportant = 0;

	for(j = 0; j < 16; j++)
	{
		tmpColor = GetPal_KMod(pal_KMod, j);
		bmiColors[j].rgbRed = (BYTE) tmpColor & 0xFF;
		tmpColor>>=8;
		bmiColors[j].rgbGreen = (BYTE) tmpColor & 0xFF;
		tmpColor>>=8;
		bmiColors[j].rgbBlue = (BYTE) tmpColor & 0xFF;
	}

	pBits = (LPBYTE) LocalAlloc(LPTR, bmiHeader.biSizeImage);
	if (pBits == NULL)
	{
		return;
	}


	numTile = 0;
	for(posY = 0; posY < 125; posY++)
	{
		for(posX = 0; posX < 16; posX++)
		{
			for(j = 0; j < 8; j++)
			{
				TileData = VRam[numTile*32 + j*4 + 1];
				pBits[ posX*4 + (124-posY)*8*16*4 + (7-j)*16*4] = TileData;

				TileData = VRam[numTile*32 + j*4 ];
				pBits[ posX*4 + (124-posY)*8*16*4 + (7-j)*16*4 + 1] = TileData;

				TileData = VRam[numTile*32 + j*4 + 3];
				pBits[ posX*4 + (124-posY)*8*16*4 + (7-j)*16*4 + 2] = TileData;

				TileData = VRam[numTile*32 + j*4 + 2];
				pBits[ posX*4 + (124-posY)*8*16*4 + (7-j)*16*4 + 3] = TileData;			
			}
			numTile++;
		}
	}

	hFr = CreateFile (szFileName, GENERIC_WRITE, (DWORD) 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
    if (hFr == INVALID_HANDLE_VALUE)
	{
		LocalFree((HLOCAL) pBits);
        return;
	}

	dwBytesToWrite = sizeof(BITMAPFILEHEADER);
    WriteFile(hFr, &bmfh, dwBytesToWrite, &dwBytesWritten, NULL) ;

	dwBytesToWrite = sizeof(BITMAPINFOHEADER);
    WriteFile(hFr, &bmiHeader, dwBytesToWrite, &dwBytesWritten, NULL) ;

	dwBytesToWrite = 16 * sizeof(RGBQUAD);
    WriteFile(hFr, bmiColors, dwBytesToWrite, &dwBytesWritten, NULL) ;

	dwBytesToWrite = bmiHeader.biSizeImage;
    WriteFile(hFr, pBits, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	LocalFree((HLOCAL) pBits);

	Put_Info("Tiles dumped", 1500);
}

void DumpVRAM_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_VRAM");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump VRAM";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "ram";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 64 * 1024;
    WriteFile(hFr, VRam, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);
	
	Put_Info("VRAM dumped", 1500);
}


void DumpCRAM_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_CRAM");


    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump CRAM";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "ram";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 64 * 8;
    WriteFile(hFr, CRam, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	Put_Info("CRAM dumped", 1500);
}

void VDPPal_Choose( )
{
	POINT pt;

	GetCursorPos(&pt);
	ScreenToClient(hVDP, &pt);
    if ( !PtInRect(&rcPal, pt) )        return;

	pt.y -= rcPal.top;
	pal_KMod = (unsigned char) (pt.y/palV);

	if (Paused)	UpdateVDP_KMod( );
}

void VDPTile_Choose( )
{
	POINT pt;

	GetCursorPos(&pt);
	ScreenToClient(hVDP, &pt);

    if ( !PtInRect(&rcTiles, pt) )        return;

	pt.y -= rcTiles.top;
	tile_KMod = (unsigned char) (pt.y/8);
	tile_KMod *= 16;

	pt.x -= rcTiles.left;
	tile_KMod += (unsigned char) (pt.x/8);

	if (Paused)	UpdateVDP_KMod( );
}


void VDPInit_KMod( HWND hwnd )
{
	RECT TZoomRect;
	SCROLLINFO si;

	GetClientRect( GetDlgItem(hwnd, IDC_VDP_PREVIEW), &TZoomRect);

	zoomTile_KMod = (TZoomRect.right - TZoomRect.left) / 8;
	zoomTile_KMod = min(zoomTile_KMod, (TZoomRect.bottom - TZoomRect.top) / 8);

	pal_KMod = 0;
//	tileBank_KMod = 0;

    hWndPal = GetDlgItem(hwnd, IDC_VDP_PAL);
    GetClientRect(hWndPal, &rcPal);
	palV = (rcPal.bottom - rcPal.top)/5;//4;
	palH = (rcPal.right - rcPal.left)/16;

    hWndRedArrow = GetDlgItem(hwnd, IDC_VDP_PAL_ARROW);
	GetClientRect(hWndRedArrow, &rcArrow);
			
	MapWindowPoints( hWndPal, hwnd, (LPPOINT) &rcPal, 2 );

	GetClientRect(GetDlgItem(hwnd, IDC_VDP_TILES), &rcTiles);			
	MapWindowPoints( GetDlgItem(hwnd, IDC_VDP_TILES), hwnd, (LPPOINT) &rcTiles, 2 );
	
	rcTiles.bottom = rcTiles.top+32*8;
	rcTiles.right = rcTiles.left+16*8;
	MoveWindow(GetDlgItem(hwnd, IDC_VDP_TILES), rcTiles.left, rcTiles.top, rcTiles.right-rcTiles.left, rcTiles.bottom-rcTiles.top, TRUE);

	MoveWindow(GetDlgItem(hwnd, IDC_VDP_SCROLL), rcTiles.right, rcTiles.top, GetSystemMetrics(SM_CXVSCROLL), rcTiles.bottom-rcTiles.top, TRUE);

	CheckDlgButton( hwnd, IDC_VDP_PAL_1, BST_CHECKED);
	CheckDlgButton( hwnd, IDC_VDP_PAL_2, BST_CHECKED);
	CheckDlgButton( hwnd, IDC_VDP_PAL_3, BST_CHECKED);
	CheckDlgButton( hwnd, IDC_VDP_PAL_4, BST_CHECKED);
	ActivePal = 0x0F;

	TileCurDraw = 0;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
	si.nMin   = 0; 
	si.nPage  = 1;
	si.nPos   = 0; 
	si.nMax   = 125-32;

	SetScrollInfo(GetDlgItem(hwnd, IDC_VDP_SCROLL), SB_CTL, &si, TRUE);
}


BOOL CALLBACK VDPDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO si;

	switch(Message)
    {
		case WM_INITDIALOG:			
			VDPInit_KMod( hwnd );
			break;

		case WM_DRAWITEM:
			if ( (UINT) wParam == IDC_VDP_PAL )
				DrawVDP_Pal_KMod( (LPDRAWITEMSTRUCT) lParam);
			else if ( (UINT) wParam == IDC_VDP_TILES )
				DrawVDP_Tiles_KMod( (LPDRAWITEMSTRUCT) lParam);
			else if ( (UINT) wParam == IDC_VDP_PREVIEW )
				DrawVDPZoom_Tiles_KMod( (LPDRAWITEMSTRUCT) lParam);
            break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_VDP_PAL:
					VDPPal_Choose( );
					break;
				case IDC_VDP_TILES:
					VDPTile_Choose( );
					break;
				case IDC_VDP_VRAM:
					DumpVRAM_KMod( hwnd );
					break;
				case IDC_VDP_CRAM:
					DumpCRAM_KMod( hwnd );
					break;
/*
				case IDC_VDP_TILES_UP:
					if (tileBank_KMod)	tileBank_KMod--;
					wsprintf(debug_string, "0x%.4X", tileBank_KMod*16);
					SendDlgItemMessage(hVDP , IDC_VDP_TILES_ADR, WM_SETTEXT, 0 , (LPARAM)debug_string);
				
					if (Paused)	UpdateVDP_KMod( );
					break;
				case IDC_VDP_TILES_DWN:
					if (tileBank_KMod < 96) tileBank_KMod++;
					wsprintf(debug_string, "0x%.4X", tileBank_KMod*16);
					SendDlgItemMessage(hVDP , IDC_VDP_TILES_ADR, WM_SETTEXT, 0 , (LPARAM)debug_string);

					if (Paused)	UpdateVDP_KMod( );
					break;
*/
				case IDC_VDP_BMP:
					DumpTiles_KMod( hwnd );
					break;
				case IDC_VDP_PAL_1:
					if ( IsDlgButtonChecked(hwnd,IDC_VDP_PAL_1) == BST_CHECKED )
						ActivePal |= 0x01;
					else
						ActivePal &= 0x0E;
					CRam_Flag = 1;
					break;
				case IDC_VDP_PAL_2:
					if ( IsDlgButtonChecked(hwnd,IDC_VDP_PAL_2) == BST_CHECKED )
						ActivePal |= 0x02;
					else
						ActivePal &= 0x0D;
					CRam_Flag = 1;
					break;
				case IDC_VDP_PAL_3:
					if ( IsDlgButtonChecked(hwnd,IDC_VDP_PAL_3) == BST_CHECKED )
						ActivePal |= 0x04;
					else
						ActivePal &= 0x0B;
					CRam_Flag = 1;
					break;
				case IDC_VDP_PAL_4:
					if ( IsDlgButtonChecked(hwnd,IDC_VDP_PAL_4) == BST_CHECKED )
						ActivePal |= 0x08;
					else
						ActivePal &= 0x07;
					CRam_Flag = 1;
					break;
			}
			break;

		case WM_VSCROLL:
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_ALL; 
			GetScrollInfo((HWND) lParam, SB_CTL, &si);
			switch(LOWORD (wParam)) 
			{ 
				case SB_PAGEUP: 
					 si.nPos -= si.nPage; 
					 break; 
				case SB_PAGEDOWN: 
					 si.nPos += si.nPage;
					 break; 
				case SB_LINEUP: 
					 si.nPos--; 
					 break;
				case SB_LINEDOWN: 
					 si.nPos++; 
					 break; 
				case SB_THUMBTRACK: 
//					 si.nPos = HIWORD(wParam); 
					 si.nPos = si.nTrackPos;
					 break; 
			} 
 
			si.cbSize = sizeof(SCROLLINFO); 
			si.fMask  = SIF_POS; 
			SetScrollInfo((HWND) lParam, SB_CTL, &si, TRUE); 
			
			wsprintf(debug_string, "0x%.4X", si.nPos*16);
			SendDlgItemMessage(hVDP , IDC_VDP_TILES_ADR, WM_SETTEXT, 0 , (LPARAM)debug_string);
			
			UpdateVDP_KMod( );
			UpdateWindow (hwnd);
			return 0; 

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_VDP );
			break;

		case WM_DESTROY:
			DestroyWindow(hVDP);
			PostQuitMessage(0);
			break;
		default:
            return FALSE;
    }
    return TRUE;
}

/************ CD 68K **************/

int Current_PC_S68K;
			
unsigned short Next_Word_S68K(void)
{
	unsigned short val;
	
	val = S68K_RW(Current_PC_S68K);

	Current_PC_S68K += 2;

	return(val);
}


unsigned int Next_Long_S68K(void)
{
	unsigned int val;
	
	val = S68K_RW(Current_PC_S68K);
	val <<= 16;
	val |= S68K_RW(Current_PC_S68K + 2);

	Current_PC_S68K += 4;

	return(val);
}


unsigned char S68k_ViewMode;
unsigned int  S68k_StartLineDisasm, S68k_StartLineWRAM, S68k_StartLinePRAM;



void SwitchS68kViewMode_KMod( )
{
	SCROLLINFO si; 

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
	si.nMin   = 0; 
	si.nPage  = 13;


	if (S68k_ViewMode&2)
	{
		if (S68k_ViewMode&1)
		{
			// DISASM VIEW
			si.nPos   = S68k_StartLineDisasm; 
			si.nMax   = Rom_Size -1;
			SendDlgItemMessage(hCD_68K, IDC_S68K_VIEW_PRAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View PRAM");
		}
		else
		{
			// Program RAM VIEW
			si.nPos   = S68k_StartLinePRAM; 
			si.nMax   = ((512 * 1024)/8) -1;
			SendDlgItemMessage(hCD_68K, IDC_S68K_VIEW_PRAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}
	}
	else if (S68k_ViewMode&8)
	{
		// Word RAM VIEW
		si.nPos   = S68k_StartLineWRAM; 
		si.nMax   = ((256 * 1024)/8) -1;
	}
	SetScrollInfo(GetDlgItem(hCD_68K, IDC_S68K_SCROLL), SB_CTL, &si, TRUE); 
}


void UpdateCD_68K_KMod( )
{
	unsigned int i, PC;
	unsigned char tmp_string[256];

	if ( OpenedWindow_KMod[ 3 ] == FALSE )	return;


	SendDlgItemMessage(hCD_68K , IDC_S68K_DISAM, LB_RESETCONTENT, (WPARAM) 0 , (LPARAM) 0);
	if (S68k_ViewMode&2)
	{
		if (S68k_ViewMode&1)
		{
			S68k_StartLineDisasm =	GetScrollPos(GetDlgItem(hCD_68K, IDC_S68K_SCROLL) ,SB_CTL);
			Current_PC_S68K = S68k_StartLineDisasm; //sub68k_context.pc;
			for(i = 0; i < 13; i++)
			{
				PC = Current_PC_S68K;
				wsprintf(debug_string, "%.5X    %-33s", PC, M68KDisasm(Next_Word_S68K, Next_Long_S68K));
				SendDlgItemMessage(hCD_68K , IDC_S68K_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
			}
		}
		else
		{
			Byte_Swap(Ram_Prg, 512 * 1024);
			S68k_StartLinePRAM = GetScrollPos(GetDlgItem(hCD_68K, IDC_S68K_SCROLL) ,SB_CTL);
			for(i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.5X ", S68k_StartLinePRAM*8 + i*8);
				tmp_string[5] = 0x20;
				Hexview( (unsigned char *) (Ram_Prg + S68k_StartLinePRAM*8 + i*8), tmp_string + 6);
				tmp_string[23] = 0x20;
				Ansiview( (unsigned char *) (Ram_Prg + S68k_StartLinePRAM*8 + i*8), tmp_string + 24);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hCD_68K , IDC_S68K_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
			}
			Byte_Swap(Ram_Prg, 512 * 1024);
		}
	}
	else if (S68k_ViewMode&8)
	{
		Byte_Swap(Ram_Word_1M, 256 * 1024);
		S68k_StartLineWRAM = GetScrollPos(GetDlgItem(hCD_68K, IDC_S68K_SCROLL) ,SB_CTL);
		for(i = 0; i < 13; i++)
		{
			wsprintf(tmp_string, "%.5X ", S68k_StartLineWRAM*8 + i*8);
			tmp_string[5] = 0x20;
			Hexview( (unsigned char *) (Ram_Word_1M + S68k_StartLineWRAM*8 + i*8), tmp_string + 6);
			tmp_string[23] = 0x20;
			Ansiview( (unsigned char *) (Ram_Word_1M + S68k_StartLineWRAM*8 + i*8), tmp_string + 24);
			wsprintf(debug_string, "%s", tmp_string);
			SendDlgItemMessage(hCD_68K , IDC_S68K_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
		}
		Byte_Swap(Ram_Word_1M, 256 * 1024);
	}


	wsprintf(debug_string, "X=%d N=%d Z=%d V=%d C=%d  SR=%.4X Cycles=%.10d",(sub68k_context.sr & 0x10)?1:0, (sub68k_context.sr & 0x8)?1:0, (sub68k_context.sr & 0x4)?1:0, (sub68k_context.sr & 0x2)?1:0, (sub68k_context.sr & 0x1)?1:0, sub68k_context.sr, sub68k_context.odometer);
	SendDlgItemMessage(hCD_68K , IDC_S68K_STATUS_SR, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "A0=%.8X A1=%.8X A2=%.8X A3=%.8X A4=%.8X A5=%.8X A6=%.8X A7=%.8X\n", sub68k_context.areg[0], sub68k_context.areg[1], sub68k_context.areg[2], sub68k_context.areg[3], sub68k_context.areg[4], sub68k_context.areg[5], sub68k_context.areg[6], sub68k_context.areg[7]);
	SendDlgItemMessage(hCD_68K , IDC_S68K_STATUS_ADR, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "D0=%.8X D1=%.8X D2=%.8X D3=%.8X D4=%.8X D5=%.8X D6=%.8X D7=%.8X\n", sub68k_context.dreg[0], sub68k_context.dreg[1], sub68k_context.dreg[2], sub68k_context.dreg[3], sub68k_context.dreg[4], sub68k_context.dreg[5], sub68k_context.dreg[6], sub68k_context.dreg[7]);
	SendDlgItemMessage(hCD_68K , IDC_S68K_STATUS_DATA, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "PC=%.8X", sub68k_context.pc);
	SendDlgItemMessage(hCD_68K, IDC_S68K_PC, WM_SETTEXT, (WPARAM)0, (LPARAM) debug_string);

}



void DumpS68KPRam_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_S68K");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "ROM dump (*.bin)\0*.bin\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump CD ROM";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "bin";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	
	dwBytesToWrite = 512 * 1024;
	Byte_Swap(Ram_Prg, dwBytesToWrite);
    WriteFile(hFr, Ram_Prg, dwBytesToWrite, &dwBytesWritten, NULL) ;
	Byte_Swap(Ram_Prg, dwBytesToWrite);

    CloseHandle(hFr);

	Put_Info("CD ROM dumped", 1500);
}

void DumpS68KWRam_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_S68K");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump CD 68K memory";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "ram";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 256 * 1024;
	Byte_Swap(Ram_Word_1M, dwBytesToWrite);
    WriteFile(hFr, Ram_Word_1M, dwBytesToWrite, &dwBytesWritten, NULL) ;
	Byte_Swap(Ram_Word_1M, dwBytesToWrite);

    CloseHandle(hFr);

	Put_Info("CD 68K RAM dumped", 1500);
}


BOOL CALLBACK CD_68KDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	HFONT hFont = NULL;
	SCROLLINFO si; 

	switch(Message)
    {
		case WM_INITDIALOG:			
			hFont = (HFONT) GetStockObject(OEM_FIXED_FONT)	;
			SendDlgItemMessage(hwnd, IDC_S68K_DISAM, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_S68K_STATUS_SR, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_S68K_STATUS_ADR, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_S68K_STATUS_DATA, WM_SETFONT, (WPARAM)hFont, TRUE);
			S68k_ViewMode = 2; //disasm
			S68k_StartLineDisasm= S68k_StartLineWRAM= S68k_StartLinePRAM = 0;
			SwitchS68kViewMode_KMod(  ); // init with wrong values (since no game loaded by default)
			break;

		case WM_SHOWWINDOW:
			SwitchS68kViewMode_KMod(  );
			break;


		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_S68K_DUMP_PRAM:
					DumpS68KPRam_KMod( hwnd );
					break;

				case IDC_S68K_DUMP_WRAM:
					DumpS68KWRam_KMod( hwnd );
					break;

				case IDC_S68K_VIEW_PRAM:
					S68k_ViewMode &= 0x15;
					S68k_ViewMode ^= 0x1;
					S68k_ViewMode |= 0x2;
					SwitchS68kViewMode_KMod( );
					UpdateWindow (hwnd); 
					UpdateCD_68K_KMod( );
					break;

				case IDC_S68K_VIEW_WRAM:
					S68k_ViewMode &= 0x15;
					S68k_ViewMode ^= 0x4;
					S68k_ViewMode |= 0x8;
					SwitchS68kViewMode_KMod( );
					UpdateWindow (hwnd); 
					UpdateCD_68K_KMod( );
					break;

				case IDC_S68K_PC:
					if (S68k_ViewMode&2)
					{
						S68k_ViewMode &= 0x15;
						S68k_ViewMode |= 0x2;

						S68k_StartLineDisasm = sub68k_context.pc;
						S68k_StartLinePRAM = S68k_StartLineDisasm/8;
						SwitchS68kViewMode_KMod( );
						UpdateWindow (hwnd); 
						UpdateCD_68K_KMod( );
					}
					break;

			}
			break;

		case WM_VSCROLL:
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_ALL; 
			GetScrollInfo((HWND) lParam, SB_CTL, &si);
			switch(LOWORD (wParam)) 
			{ 
				case SB_PAGEUP: 
					 si.nPos -= si.nPage; 
					 break; 
				case SB_PAGEDOWN: 
					 si.nPos += si.nPage;
					 break; 
				case SB_LINEUP: 
					 si.nPos--; 
					 break;
				case SB_LINEDOWN: 
					 si.nPos++; 
					 break; 
				case SB_THUMBTRACK: 
//					 si.nPos = HIWORD(wParam); 
					 si.nPos = si.nTrackPos;
					 break; 
			} 
 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_POS; 
			SetScrollInfo((HWND) lParam, SB_CTL, &si, TRUE); 
			UpdateWindow (hwnd); 
			UpdateCD_68K_KMod( );
			return 0; 

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_CD_68K );
			break;

		case WM_DESTROY:			
			DeleteObject( (HGDIOBJ) hFont );
			DestroyWindow(hCD_68K);
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;
}

/************ CD CDC **************/
HWND hCDCDCList;

void CD_CDCInit_KMod( HWND hwnd)
{
	LV_COLUMN   lvColumn;
	LVITEM		lvItem;
//	int         i;
	char		buf[64];
	RECT		rSize;
	TCHAR       szString[2][20] = {"Name", "Value"};

	hCDCDCList = GetDlgItem(hwnd, IDC_CDC_LIST);
	ListView_DeleteAllItems(hCDCDCList);

	GetWindowRect( hCDCDCList, &rSize);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 80;
	lvColumn.pszText = szString[0];
	ListView_InsertColumn(hCDCDCList, 0, &lvColumn);
	
	lvColumn.cx = 80;
	lvColumn.pszText = szString[1];
	ListView_InsertColumn(hCDCDCList, 1, &lvColumn);


	ListView_SetExtendedListViewStyle(hCDCDCList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	lvItem.mask = LVIF_TEXT;

	lvItem.iItem = 0;
	lvItem.iSubItem = 0;
	lvItem.pszText = "COMIN";
	ListView_InsertItem(hCDCDCList, &lvItem);
//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X",  CDC.COMIN);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 1;
	lvItem.iSubItem = 0;
	lvItem.pszText = "IFSTAT";
	ListView_InsertItem(hCDCDCList, &lvItem);
//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X",  CDC.IFSTAT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 2;
	lvItem.iSubItem = 0;
	lvItem.pszText = "DBC";
	ListView_InsertItem(hCDCDCList, &lvItem);
//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X",  CDC.DBC);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 3;
	lvItem.iSubItem = 0;
	lvItem.pszText = "HEAD";
	ListView_InsertItem(hCDCDCList, &lvItem);
//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X",  CDC.HEAD);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 4;
	lvItem.iSubItem = 0;
	lvItem.pszText = "PT";
	ListView_InsertItem(hCDCDCList, &lvItem);
//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X",  CDC.PT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 5;
	lvItem.iSubItem = 0;
	lvItem.pszText = "WA";
	ListView_InsertItem(hCDCDCList, &lvItem);
//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X",  CDC.WA);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 6;
	lvItem.iSubItem = 0;
	lvItem.pszText = "STAT";
	ListView_InsertItem(hCDCDCList, &lvItem);
//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X",  CDC.STAT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 7;
	lvItem.iSubItem = 0;
	lvItem.pszText = "CTRL";
	ListView_InsertItem(hCDCDCList, &lvItem);
//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X",  CDC.CTRL);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);
	
//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 8;
	lvItem.iSubItem = 0;
	lvItem.pszText = "DAC";
	ListView_InsertItem(hCDCDCList, &lvItem);
//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X",  CDC.DAC);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 9;
	lvItem.iSubItem = 0;
	lvItem.pszText = "IFCTRL";
	ListView_InsertItem(hCDCDCList, &lvItem);
//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X",  CDC.IFCTRL);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);
}

void UpdateCD_CDC_KMod( )
{
//	int         i;
	char		buf[64];
	LVITEM		lvItem;

	if ( OpenedWindow_KMod[ 4 ] == FALSE )	return;

	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;


//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 0;
//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X",  CDC.COMIN);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 1;
//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X",  CDC.IFSTAT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 2;
//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X",  CDC.DBC);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 3;
//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X",  CDC.HEAD);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 4;
//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X",  CDC.PT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 5;
//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X",  CDC.WA);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 6;
//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X",  CDC.STAT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 7;
//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X",  CDC.CTRL);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);
	
//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 8;
//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X",  CDC.DAC);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 9;
//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X",  CDC.IFCTRL);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);
}


BOOL CALLBACK CD_CDCDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch(Message)
    {
		case WM_INITDIALOG:
			CD_CDCInit_KMod( hwnd );
			break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_CD_CDC );
			break;

		case WM_DESTROY:			
			DestroyWindow(hCD_CDC);
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;
}

/************ CD GFX **************/
//// size WRAM = 256*1024....4096tiles (64b) ==> (32*8)*(64*16)


UCHAR CDpal_KMod, CDtileBank_KMod, CDzoomTile_KMod, CDTileCurDraw;

void DumpTilesCD_KMod( HWND hwnd )
{

	BITMAPFILEHEADER	bmfh;
	BITMAPINFOHEADER	bmiHeader;
	RGBQUAD				bmiColors[16];
	LPBYTE				pBits ;

	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
	HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

	WORD posX, posY;
	WORD maxX, maxY;

	COLORREF		tmpColor;
	DWORD			j;
	
	unsigned short int numTile=0;
	unsigned char		TileData;


    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_VRAM");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "16Colors Bitmap file (*.bmp)\0*.bmp\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Convert Word RAM to bitmap";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "bmp";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

	bmfh.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
	bmfh.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) +  sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) + 64 * 2000/2); // 4bits per pixel
	bmfh.bfOffBits = (DWORD) (sizeof(BITMAPFILEHEADER) +  sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) );


	maxY = 64*16;
	maxX = 32*8;

	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = maxX;
    bmiHeader.biHeight = maxY;
    bmiHeader.biPlanes = 0;
    bmiHeader.biBitCount = 4;
    bmiHeader.biClrUsed = 16;
    bmiHeader.biCompression = BI_RGB;
    bmiHeader.biSizeImage = (bmiHeader.biWidth*bmiHeader.biHeight)/2;
    bmiHeader.biClrImportant = 0;

	for(j = 0; j < 16; j++)
	{
		tmpColor = GetPal_KMod(pal_KMod, j);
		bmiColors[j].rgbRed = (BYTE) tmpColor & 0xFF;
		tmpColor>>=8;
		bmiColors[j].rgbGreen = (BYTE) tmpColor & 0xFF;
		tmpColor>>=8;
		bmiColors[j].rgbBlue = (BYTE) tmpColor & 0xFF;
	}

	pBits = (LPBYTE) LocalAlloc(LPTR, bmiHeader.biSizeImage);
	if (pBits == NULL)
	{
		return;
	}

	for(posY = 0; posY < maxY; posY+=16)
	{
		for(posX = 0; posX < maxX/2; posX+=4)
		{
			for(j = 0; j < 8; j++)
			{
				TileData = Ram_Word_2M[numTile*64 + j*4 + 1];
				pBits[(bmiHeader.biHeight - (posY+j))*bmiHeader.biWidth/2 + posX +0 ] = TileData;

				TileData = Ram_Word_2M[numTile*64 + j*4 ];
				pBits[(bmiHeader.biHeight - (posY+j))*bmiHeader.biWidth/2 + posX +1 ] = TileData;

				TileData = Ram_Word_2M[numTile*64 + j*4 + 3];
				pBits[(bmiHeader.biHeight - (posY+j))*bmiHeader.biWidth/2 + posX +2 ] = TileData;

				TileData = Ram_Word_2M[numTile*64 + j*4 + 2];
				pBits[(bmiHeader.biHeight - (posY+j))*bmiHeader.biWidth/2 + posX +3 ] = TileData;
			}	

			for(j = 0; j < 8; j++)
			{
				TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 1];
				pBits[(bmiHeader.biHeight - (posY+j+8))*bmiHeader.biWidth/2 + posX +0 ] = TileData;

				TileData = Ram_Word_2M[numTile*64 + 32 + j*4 ];
				pBits[(bmiHeader.biHeight - (posY+j+8))*bmiHeader.biWidth/2 + posX +1 ] = TileData;

				TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 3];
				pBits[(bmiHeader.biHeight - (posY+j+8))*bmiHeader.biWidth/2 + posX +2 ] = TileData;

				TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 2];
				pBits[(bmiHeader.biHeight - (posY+j+8))*bmiHeader.biWidth/2 + posX +3 ] = TileData;
			}
			numTile++;
		}
	}

	hFr = CreateFile (szFileName, GENERIC_WRITE, (DWORD) 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
    if (hFr == INVALID_HANDLE_VALUE)
	{
		LocalFree((HLOCAL) pBits);
        return;
	}

	dwBytesToWrite = sizeof(BITMAPFILEHEADER);
    WriteFile(hFr, &bmfh, dwBytesToWrite, &dwBytesWritten, NULL) ;

	dwBytesToWrite = sizeof(BITMAPINFOHEADER);
    WriteFile(hFr, &bmiHeader, dwBytesToWrite, &dwBytesWritten, NULL) ;

	dwBytesToWrite = 16 * sizeof(RGBQUAD);
    WriteFile(hFr, bmiColors, dwBytesToWrite, &dwBytesWritten, NULL) ;

	dwBytesToWrite = bmiHeader.biSizeImage;
    WriteFile(hFr, pBits, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	LocalFree((HLOCAL) pBits);

	Put_Info("WRAM Tiles dumped", 1500);
}

	
void DrawTileCD_KMod( HDC hDCMain, unsigned short int numTile, WORD x, WORD y, UCHAR pal, UCHAR zoom)
{
	unsigned char j;
	unsigned char TileData;
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC( hDCMain );
	hBitmap = CreateCompatibleBitmap( hDCMain, 8, 16);
	hOldBitmap = SelectObject(hDC, hBitmap);


	for(j = 0; j < 8; j++)
	{
		TileData = Ram_Word_2M[numTile*64 + j*4 + 1]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 0, j, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + j*4 + 1]&0x0F;
		SetPixelV(hDC, 1, j, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + j*4 ]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 2, j, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + j*4 ]&0x0F;
		SetPixelV(hDC, 3, j, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + j*4 + 3]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 4, j, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + j*4 + 3]&0x0F;
		SetPixelV(hDC, 5, j, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + j*4 + 2]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 6, j, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + j*4 + 2]&0x0F;
		SetPixelV(hDC, 7, j, GetPal_KMod(pal, TileData) );
	}	

	for(j = 0; j < 8; j++)
	{
		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 1]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 0, j+8, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 1]&0x0F;
		SetPixelV(hDC, 1, j+8, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 ]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 2, j+8, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 ]&0x0F;
		SetPixelV(hDC, 3, j+8, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 3]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 4, j+8, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 3]&0x0F;
		SetPixelV(hDC, 5, j+8, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 2]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 6, j+8, GetPal_KMod(pal, TileData) );

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 2]&0x0F;
		SetPixelV(hDC, 7, j+8, GetPal_KMod(pal, TileData) );
	}

	StretchBlt(
		hDCMain, // handle to destination device context
		x,  // x-coordinate of destination rectangle's upper-left 
               // corner
		y,  // y-coordinate of destination rectangle's upper-left 
               // corner
		8*zoom,  // width of destination rectangle
		16*zoom, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left 
			// corner
		0,   // y-coordinate of source rectangle's upper-left 
               // corner
		8,
		16,
		SRCCOPY  // raster operation code
	);


	SelectObject(hDC, hOldBitmap);
    DeleteObject( hBitmap );

	DeleteDC( hDC );
}


void DrawCD_GFX_KMod( LPDRAWITEMSTRUCT hlDIS )
{
	WORD posX, posY, maxY, maxX;
	unsigned short int numTile = 0;
	SCROLLINFO si;

	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC( hlDIS->hDC );
	hBitmap = CreateCompatibleBitmap( hlDIS->hDC,  hlDIS->rcItem.right - hlDIS->rcItem.left, hlDIS->rcItem.bottom - hlDIS->rcItem.top);
	hOldBitmap = SelectObject(hDC, hBitmap);

//	numTile = CDtileBank_KMod*32;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_POS; 
	GetScrollInfo((HWND) GetDlgItem(hCD_GFX, IDC_CD_GFX_SCROLL), SB_CTL, &si);

	numTile = si.nPos*32;

	if (Paused)
	{
		maxY = 16*16;
	}
	else
	{
		// speed up emulation by redrawing 1/4 each refresh 
		maxY = 4*16;
		numTile += CDTileCurDraw*4*32;
	}
	

	maxX = 32*8;
	for(posY = 0; posY < maxY; posY+=16)
	{
		for(posX = 0; posX < maxX; posX+=8)
		{
			DrawTileCD_KMod( hDC, numTile++, posX, posY, pal_KMod, 1);
		}
	}


	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left 
               // corner
		(Paused?0:CDTileCurDraw*maxY),  // y-coordinate of destination rectangle's upper-left 
               // corner
		maxX,  // width of destination rectangle
		maxY, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left 
               // corner
		0,   // y-coordinate of source rectangle's upper-left 
               // corner
		SRCCOPY  // raster operation code
	);

	SelectObject(hDC, hOldBitmap);
    DeleteObject( hBitmap );

	DeleteDC( hDC );

}


void UpdateCD_GFX_KMod( )
{
	if ( OpenedWindow_KMod[ 5 ] == FALSE )	return;

	RedrawWindow( GetDlgItem(hCD_GFX, IDC_CD_GFX), NULL, NULL, RDW_INVALIDATE);

	CDTileCurDraw++;
	if (CDTileCurDraw > 3)	CDTileCurDraw = 0;
}


void CD_GFXInit_KMod( HWND hwnd )
{
	SCROLLINFO si;
	RECT rc;

	CDtileBank_KMod = 0;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
	si.nMin   = 0; 
	si.nPage  = 1;
	si.nPos   = 0; 
	si.nMax   = 64-16;

	SetScrollInfo(GetDlgItem(hwnd, IDC_CD_GFX_SCROLL), SB_CTL, &si, TRUE);


	GetClientRect(GetDlgItem(hwnd, IDC_CD_GFX), &rc);			
	MapWindowPoints( GetDlgItem(hwnd, IDC_CD_GFX), hwnd, (LPPOINT) &rc, 2 );

	rc.bottom = rc.top+16*16;
	rc.right = rc.left+32*8;
	MoveWindow(GetDlgItem(hwnd, IDC_CD_GFX), rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);


	MoveWindow(GetDlgItem(hwnd, IDC_CD_GFX_SCROLL), rc.right, rc.top, GetSystemMetrics(SM_CXVSCROLL), rc.bottom-rc.top, TRUE);

}


BOOL CALLBACK CD_GFXDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO si;

	switch(Message)
    {
		case WM_INITDIALOG:			
			CD_GFXInit_KMod(hwnd );
			break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_CD_GFX_DUMP:
					break;
				
				case IDC_CD_GFX_BMP:
					DumpTilesCD_KMod( hCD_GFX );
					break;
			}
			break;

		case WM_VSCROLL:
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_ALL; 
			GetScrollInfo((HWND) lParam, SB_CTL, &si);
			switch(LOWORD (wParam)) 
			{ 
				case SB_PAGEUP: 
					 si.nPos -= si.nPage; 
					 break; 
				case SB_PAGEDOWN: 
					 si.nPos += si.nPage;
					 break; 
				case SB_LINEUP: 
					 si.nPos--; 
					 break;
				case SB_LINEDOWN: 
					 si.nPos++; 
					 break; 
				case SB_THUMBTRACK: 
//					 si.nPos = HIWORD(wParam); 
					 si.nPos = si.nTrackPos;
					 break; 
			} 
 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_POS; 
			SetScrollInfo((HWND) lParam, SB_CTL, &si, TRUE); 
			
			wsprintf(debug_string, "0x%.5X", (32*si.nPos)*0x500);
			SendDlgItemMessage(hCD_GFX , IDC_CD_GFX_ADR, WM_SETTEXT, 0 , (LPARAM)debug_string);

			UpdateCD_GFX_KMod( );
			UpdateWindow (hwnd);
			return 0; 

		case WM_DRAWITEM:
			if ( (UINT) wParam == IDC_CD_GFX )
				DrawCD_GFX_KMod( (LPDRAWITEMSTRUCT) lParam);
			break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_CD_GFX );
			break;

		case WM_DESTROY:		
			DestroyWindow(hCD_GFX);
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;
}

/************ CD REG **************/
HWND hCDRegList;

void CDRegInit_KMod( HWND hwnd)
{
	LV_COLUMN   lvColumn;
	LVITEM		lvItem;
	int         i;
	char		buf[64];
	RECT		rSize;
	TCHAR       szString[3][20] = {"Address (hex)", "MainCPU (0xA12000)", "SubCPU (0xFF8000)"};

	hCDRegList = GetDlgItem(hwnd, IDC_CD_REG_LIST);
	ListView_DeleteAllItems(hCDRegList);

	GetWindowRect( hCDRegList, &rSize);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 80; // ((rSize.right - rSize.left -5)/3);
	lvColumn.pszText = szString[0];
	ListView_InsertColumn(hCDRegList, 0, &lvColumn);
	
	lvColumn.cx = 0x75;
	lvColumn.pszText = szString[1];
	ListView_InsertColumn(hCDRegList, 1, &lvColumn);

	lvColumn.cx = 0x75;
	lvColumn.pszText = szString[2];
	ListView_InsertColumn(hCDRegList, 2, &lvColumn);
	
	ListView_SetExtendedListViewStyle(hCDRegList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	for (i=0; i<= 0x4A; i+=2)
	{
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 0;
		wsprintf(buf, "%0.2X", i);
		lvItem.pszText = buf;
		lvItem.iItem = i;
		 ListView_InsertItem(hCDRegList, &lvItem);

		lvItem.iSubItem = 1;
		if (i > 0x2E)
			wsprintf(buf, "");
		else
			wsprintf(buf, "0x%0.4X",  M68K_RW(0xA12000 | i));
		lvItem.pszText = buf;
		ListView_SetItem(hCDRegList, &lvItem);
		
		lvItem.iSubItem = 2;
		wsprintf(buf, "0x%0.4X", S68K_RW(0xFF8000 | i));
		lvItem.pszText = buf;
		ListView_SetItem(hCDRegList, &lvItem);	
	}
}

void UpdateCD_Reg_KMod( )
{
	int         i;
	char		buf[64];
	LVITEM		lvItem;

	if ( OpenedWindow_KMod[ DMODE_CD_REG-1 ] == FALSE )	return;

	for (i=0; i<= 0x4A; i+=2)
	{
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = i/2;

		lvItem.iSubItem = 1;
		if (i > 0x2E)
			wsprintf(buf, "");
		else
			wsprintf(buf, "0x%0.4X",  M68K_RW(0xA12000 | i));
		lvItem.pszText = buf;
		ListView_SetItem(hCDRegList, &lvItem);
		
		lvItem.iSubItem = 2;
		wsprintf(buf, "0x%0.4X", S68K_RW(0xFF8000 | i));
		lvItem.pszText = buf;
		ListView_SetItem(hCDRegList, &lvItem);
	}
}


BOOL CALLBACK CD_RegDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch(Message)
    {
		case WM_INITDIALOG:
			CDRegInit_KMod( hwnd );
			break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_CD_REG );
			break;

		case WM_DESTROY:			
			DestroyWindow(hCD_Reg);
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;
}

/************ 32X MSH2 **************/
unsigned char MSH2_ViewMode;
unsigned int  MSH2_StartLineROMDisasm, MSH2_StartLineRAMDisasm, MSH2_StartLineRAM, MSH2_StartLineROM,MSH2_StartLineCache;

void UpdateMSH2_KMod( )
{
	unsigned int i, PC;	
	unsigned char tmp_string[256];

	if ( OpenedWindow_KMod[ 6 ] == FALSE )	return;

	SendDlgItemMessage(hMSH2 , IDC_MSH2_DISAM, LB_RESETCONTENT, (WPARAM) 0 , (LPARAM) 0);

	if (MSH2_ViewMode&2)
	{
		if (MSH2_ViewMode&1)
		{
			// ROM disasm
			//PC = (sh->PC - sh->Base_PC) - 4;
			MSH2_StartLineROMDisasm =	GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL) ,SB_CTL);
			PC = 0x02000000 + MSH2_StartLineROMDisasm*2;
			for(i = 0; i < 13; PC+=2, i++)
			{
				SH2Disasm(tmp_string, PC, SH2_Read_Word(&M_SH2, PC), 0);
				SendDlgItemMessage(hMSH2 , IDC_MSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)tmp_string);
			}
		}
		else
		{
		// ROM view
			MSH2_StartLineROM = GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL) ,SB_CTL);
			for(i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.8X ", 0x02000000 + MSH2_StartLineROM*8 + i*8);
				tmp_string[8] = 0x20;
				Hexview( (unsigned char *) (_32X_Rom + MSH2_StartLineROM*8 + i*8), tmp_string + 9);
				tmp_string[26] = 0x20;
				Ansiview( (unsigned char *) (_32X_Rom + MSH2_StartLineROM*8 + i*8), tmp_string + 27);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hMSH2 , IDC_MSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
			}
		}
	}
	else if (MSH2_ViewMode&8)
	{
		if (MSH2_ViewMode&4)
		{
			// RAM disasm
			//PC = (sh->PC - sh->Base_PC) - 4;
			MSH2_StartLineRAMDisasm =	GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL) ,SB_CTL);
			PC = 0x06000000 + MSH2_StartLineRAMDisasm*2;
			for(i = 0; i < 13; PC+=2, i++)
			{
				SH2Disasm(tmp_string, PC, SH2_Read_Word(&M_SH2, PC), 0);
				SendDlgItemMessage(hMSH2 , IDC_MSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)tmp_string);
			}
		}
		else
		{
			// RAM view
			MSH2_StartLineRAM = GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL) ,SB_CTL);
			for(i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.8X ", 0x06000000 + MSH2_StartLineRAM*8 + i*8);
				tmp_string[8] = 0x20;
				Hexview( (unsigned char *) (_32X_Ram + MSH2_StartLineRAM*8 + i*8), tmp_string + 9);
				tmp_string[26] = 0x20;
				Ansiview( (unsigned char *) (_32X_Ram + MSH2_StartLineRAM*8 + i*8), tmp_string + 27);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hMSH2 , IDC_MSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
			}
		}
	}
	else if (MSH2_ViewMode&0x20)
	{
/*
		if (MSH2_ViewMode&0x10)
		{
			// Cache disasm
			MSH2_StartLineRAMDisasm =	GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL) ,SB_CTL);
			PC = 0x06000000 + MSH2_StartLineRAMDisasm*2;
			for(i = 0; i < 13; PC+=2, i++)
			{
				SH2Disasm(tmp_string, PC, SH2_Read_Word(&M_SH2, PC), 0);
				SendDlgItemMessage(hMSH2 , IDC_MSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)tmp_string);
			}
		}
		else
		{*/
			// Cache view
			MSH2_StartLineCache = GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL) ,SB_CTL);
			for(i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.8X ", 0xC0000000 +MSH2_StartLineCache*8 + i*8);
				tmp_string[8] = 0x20;
				Hexview( (unsigned char *) (M_SH2.Cache + MSH2_StartLineCache*8 + i*8), tmp_string + 9);
				tmp_string[26] = 0x20;
				Ansiview( (unsigned char *) (M_SH2.Cache + MSH2_StartLineCache*8 + i*8), tmp_string + 27);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hMSH2 , IDC_MSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
			}
	//	}
	}
	wsprintf(debug_string, "T=%d S=%d Q=%d M=%d I=%.1X SR=%.4X Status=%.4X",SH2_Get_SR(&M_SH2) & 1, (SH2_Get_SR(&M_SH2) >> 1) & 1, (SH2_Get_SR(&M_SH2) >> 8) & 1, (SH2_Get_SR(&M_SH2) >> 9) & 1, (SH2_Get_SR(&M_SH2) >> 4) & 0xF,SH2_Get_SR(&M_SH2), M_SH2.Status & 0xFFFF);
	SendDlgItemMessage(hMSH2 , IDC_MSH2_STATUS_SR, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "R0=%.8X R1=%.8X R2=%.8X R3=%.8X\nR4=%.8X R5=%.8X R6=%.8X R7=%.8X\nR8=%.8X R9=%.8X RA=%.8X RB=%.8X\nRC=%.8X RD=%.8X RE=%.8X RF=%.8X", SH2_Get_R(&M_SH2, 0), SH2_Get_R(&M_SH2, 1), SH2_Get_R(&M_SH2, 2), SH2_Get_R(&M_SH2, 3), SH2_Get_R(&M_SH2, 4), SH2_Get_R(&M_SH2, 5), SH2_Get_R(&M_SH2, 6), SH2_Get_R(&M_SH2, 7), SH2_Get_R(&M_SH2, 8), SH2_Get_R(&M_SH2, 9), SH2_Get_R(&M_SH2, 0xA), SH2_Get_R(&M_SH2, 0xB), SH2_Get_R(&M_SH2, 0xC), SH2_Get_R(&M_SH2, 0xD), SH2_Get_R(&M_SH2, 0xE), SH2_Get_R(&M_SH2, 0xF));
	SendDlgItemMessage(hMSH2 , IDC_MSH2_STATUS_ADR, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "GBR=%.8X VBR=%.8X PR=%.8X\nMACH=%.8X MACL=%.8X\nIL=%.2X IV=%.2X", SH2_Get_GBR(&M_SH2), SH2_Get_VBR(&M_SH2), SH2_Get_PR(&M_SH2), SH2_Get_MACH(&M_SH2), SH2_Get_MACL(&M_SH2), M_SH2.INT.Prio,M_SH2.INT.Vect);
	SendDlgItemMessage(hMSH2 , IDC_MSH2_STATUS_DATA, WM_SETTEXT, 0 , (LPARAM)debug_string);
	
	wsprintf(debug_string, "PC=%.8X", SH2_Get_PC(&M_SH2));//(M_SH2.PC - M_SH2.Base_PC) - 4);
	SendDlgItemMessage(hMSH2, IDC_MSH2_PC, WM_SETTEXT, (WPARAM)0, (LPARAM) debug_string);

}

void Dump32XRom_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_ROM");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "ROM dump (*.bin)\0*.bin\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump 32X ROM";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "bin";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 4 * 1024 * 1024;
    WriteFile(hFr, _32X_Rom, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	Put_Info("32X ROM dumped", 1500);
}

void Dump32XRam_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_MSH2");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump 32X memory";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "ram";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 256 * 1024;
    WriteFile(hFr, _32X_Ram, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	Put_Info("32X RAM dumped", 1500);
}


void DumpMSH2Cache_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_MSH2");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "Cache dump (*.dat)\0*.dat\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump MSH2 cache";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "dat";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 0x1000;
    WriteFile(hFr, M_SH2.Cache, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	Put_Info("MSH2 Cache dumped", 1500);
}

void SwitchMSH2ViewMode_KMod(  )
{
	SCROLLINFO si;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
	si.nMin   = 0; 
	si.nPage  = 13;

	if (MSH2_ViewMode&2)
	{
		if (MSH2_ViewMode&1)
		{
			// ROM DISASM VIEW
			si.nPos   = MSH2_StartLineROMDisasm; 
			si.nMax   = ((4 * 1024 * 1024)/2) -1;
			SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View ROM");
		}
		else 
		{
			// ROM VIEW
			si.nPos   = MSH2_StartLineROM; 
			si.nMax   = ((4 * 1024 * 1024)/8) -1;
			SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}
	}
	else if (MSH2_ViewMode&8)
	{
		if (MSH2_ViewMode&4)
		{
			// RAM DISASM VIEW
			si.nPos   = MSH2_StartLineRAMDisasm; 
			si.nMax   = ((256 * 1024)/2) -1;
			SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View RAM");
		}
		else
		{
			// RAM VIEW
			si.nPos   = MSH2_StartLineRAM; 
			si.nMax   = ((256 * 1024)/8) -1;
			SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}
	}
	else if (MSH2_ViewMode&0x20)
	{
		/*
		if (MSH2_ViewMode&0x10)
		{
			// CACHE DISASM VIEW
			si.nPos   = MSH2_StartLineRAMDisasm; 
			si.nMax   = ((256 * 1024)/2) -1;
			SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View RAM");
		}
		else
		{*/
			// CACHE VIEW
			si.nPos   = MSH2_StartLineCache; 
			si.nMax   = (0x1000/8) -1;
/*
			SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}*/
	}
	SetScrollInfo(GetDlgItem(hMSH2, IDC_MSH2_SCROLL), SB_CTL, &si, TRUE); 
}

BOOL CALLBACK MSH2DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;
	SCROLLINFO si;
	unsigned int curPC;
	switch(Message)
    {
		case WM_INITDIALOG:
			hFont = (HFONT) GetStockObject(OEM_FIXED_FONT)	;
			SendDlgItemMessage(hwnd, IDC_MSH2_DISAM, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_MSH2_STATUS_SR, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_MSH2_STATUS_ADR, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_MSH2_STATUS_DATA, WM_SETFONT, (WPARAM)hFont, TRUE);
			MSH2_ViewMode = 0x7; //disasm ROM
			MSH2_StartLineROMDisasm= MSH2_StartLineROM= MSH2_StartLineRAMDisasm = MSH2_StartLineRAM= MSH2_StartLineCache=0;
			SwitchMSH2ViewMode_KMod( ); // init with wrong values (since no game loaded by default)
			break;

		case WM_SHOWWINDOW:
			SwitchMSH2ViewMode_KMod( );
			break;	

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_MSH2_DUMP_ROM:
					Dump32XRom_KMod( hwnd );
					break;
				case IDC_MSH2_DUMP_RAM:
					Dump32XRam_KMod( hwnd );
					break;
				case IDC_MSH2_DUMP_CACHE:
					DumpMSH2Cache_KMod( hwnd );
					break;
				case IDC_MSH2_VIEW_ROM:
					MSH2_ViewMode &= 0x15;
					MSH2_ViewMode ^= 0x1;
					MSH2_ViewMode |= 0x2;
					SwitchMSH2ViewMode_KMod( );
					UpdateWindow (hwnd); 
					UpdateMSH2_KMod( );
					break;
				case IDC_MSH2_VIEW_RAM:
					MSH2_ViewMode &= 0x15;
					MSH2_ViewMode ^= 0x4;
					MSH2_ViewMode |= 0x8;

					SwitchMSH2ViewMode_KMod(  );
					UpdateWindow (hwnd); 
					UpdateMSH2_KMod( );
					break;
				case IDC_MSH2_VIEW_CACHE:
					MSH2_ViewMode &= 0x15;
//					MSH2_ViewMode ^= 0x10;
					MSH2_ViewMode |= 0x20;

					SwitchMSH2ViewMode_KMod(  );
					UpdateWindow (hwnd); 
					UpdateMSH2_KMod( );
					break;

				case IDC_MSH2_PC:
					curPC = SH2_Get_PC(&M_SH2); //(M_SH2.PC - M_SH2.Base_PC) - 4;
					if (curPC < 0x2400000)
					{
						MSH2_ViewMode &= 0x15;
						MSH2_ViewMode |= 0x2;
						MSH2_StartLineROM = (curPC-0x02000000)/8;
						MSH2_StartLineROMDisasm = (curPC-0x02000000)/2;
						SwitchMSH2ViewMode_KMod( );
					}
					else if (curPC < 0x6040000)
					{
						MSH2_ViewMode &= 0x15;
						MSH2_ViewMode |= 0x8;
						MSH2_StartLineRAM = (curPC-0x06000000)/8;
						MSH2_StartLineRAMDisasm = (curPC-0x06000000)/2;
						SwitchMSH2ViewMode_KMod( );
					}
					
					UpdateWindow (hwnd); 
					UpdateMSH2_KMod( );
					break;

			}
			break;

		case WM_VSCROLL:
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_ALL; //SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS; 
			GetScrollInfo((HWND) lParam, SB_CTL, &si);
			switch(LOWORD (wParam)) 
			{ 
				case SB_PAGEUP: 
					 si.nPos -= si.nPage; 
					 break; 
				case SB_PAGEDOWN: 
					 si.nPos += si.nPage;
					 break; 
				case SB_LINEUP: 
					 si.nPos--; 
					 break;
				case SB_LINEDOWN: 
					 si.nPos++; 
					 break; 
//				case SB_THUMBPOSITION:
				case SB_THUMBTRACK: 
					// don't change!! else you'll loose the 32bits value for a 16bits value
//					 si.nPos = HIWORD(wParam); 
					 si.nPos = si.nTrackPos;
					 break; 
			} 
 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_POS; 
			SetScrollInfo((HWND) lParam, SB_CTL, &si, TRUE); 
			UpdateWindow (hwnd); 
			UpdateMSH2_KMod( );
			return 0; 

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_32_MSH2 );
			break;

		case WM_DESTROY:
			DeleteObject( (HGDIOBJ) hFont );			
			DestroyWindow(hMSH2);
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;
}
/************ 32X SSH2 **************/
unsigned char SSH2_ViewMode;
unsigned int  SSH2_StartLineROMDisasm, SSH2_StartLineRAMDisasm, SSH2_StartLineRAM, SSH2_StartLineROM,SSH2_StartLineCache;

void UpdateSSH2_KMod( )
{
	unsigned int i, PC;	
	unsigned char tmp_string[256];

	if ( OpenedWindow_KMod[ 7 ] == FALSE )	return;

	SendDlgItemMessage(hSSH2 , IDC_SSH2_DISAM, LB_RESETCONTENT, (WPARAM) 0 , (LPARAM) 0);

	if (SSH2_ViewMode&2)
	{
		if (SSH2_ViewMode&1)
		{
			// ROM disasm
			//PC = (sh->PC - sh->Base_PC) - 4;
			SSH2_StartLineROMDisasm =	GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL) ,SB_CTL);
			PC = 0x02000000 + SSH2_StartLineROMDisasm*2;
			for(i = 0; i < 13; PC+=2, i++)
			{
				SH2Disasm(tmp_string, PC, SH2_Read_Word(&S_SH2, PC), 0);
				SendDlgItemMessage(hSSH2 , IDC_SSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)tmp_string);
			}
		}
		else
		{
		// ROM view
			SSH2_StartLineROM = GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL) ,SB_CTL);
			for(i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.8X ",0x02000000 +  SSH2_StartLineROM*8 + i*8);
				tmp_string[8] = 0x20;
				Hexview( (unsigned char *) (_32X_Rom + SSH2_StartLineROM*8 + i*8), tmp_string + 9);
				tmp_string[26] = 0x20;
				Ansiview( (unsigned char *) (_32X_Rom + SSH2_StartLineROM*8 + i*8), tmp_string + 27);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hSSH2 , IDC_SSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
			}
		}
	}
	else if (SSH2_ViewMode&8)
	{
		if (SSH2_ViewMode&4)
		{
			// RAM disasm
			//PC = (sh->PC - sh->Base_PC) - 4;
			SSH2_StartLineRAMDisasm =	GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL) ,SB_CTL);
			PC = 0x06000000 + SSH2_StartLineRAMDisasm*2;
			for(i = 0; i < 13; PC+=2, i++)
			{
				SH2Disasm(tmp_string, PC, SH2_Read_Word(&S_SH2, PC), 0);
				SendDlgItemMessage(hSSH2 , IDC_SSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)tmp_string);
			}
		}
		else
		{
			// RAM view
			SSH2_StartLineRAM = GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL) ,SB_CTL);
			for(i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.8X ", 0x06000000 + SSH2_StartLineRAM*8 + i*8);
				tmp_string[8] = 0x20;
				Hexview( (unsigned char *) (_32X_Ram + SSH2_StartLineRAM*8 + i*8), tmp_string + 9);
				tmp_string[26] = 0x20;
				Ansiview( (unsigned char *) (_32X_Ram + SSH2_StartLineRAM*8 + i*8), tmp_string + 27);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hSSH2 , IDC_SSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
			}
		}
	}
	else if (SSH2_ViewMode&0x20)
	{
/*
		if (SSH2_ViewMode&0x10)
		{
			// Cache disasm
			SSH2_StartLineRAMDisasm =	GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL) ,SB_CTL);
			PC = 0x06000000 + SSH2_StartLineRAMDisasm*2;
			for(i = 0; i < 13; PC+=2, i++)
			{
				SH2Disasm(tmp_string, PC, SH2_Read_Word(&S_SH2, PC), 0);
				SendDlgItemMessage(hSSH2 , IDC_SSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)tmp_string);
			}
		}
		else
		{*/
			// Cache view
			SSH2_StartLineCache = GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL) ,SB_CTL);
			for(i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.8X ", 0xC0000000 +SSH2_StartLineCache*8 + i*8);
				tmp_string[8] = 0x20;
				Hexview( (unsigned char *) (S_SH2.Cache + SSH2_StartLineCache*8 + i*8), tmp_string + 9);
				tmp_string[26] = 0x20;
				Ansiview( (unsigned char *) (S_SH2.Cache + SSH2_StartLineCache*8 + i*8), tmp_string + 27);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hSSH2 , IDC_SSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)debug_string);
			}
	//	}
	}
	wsprintf(debug_string, "T=%d S=%d Q=%d M=%d I=%.1X SR=%.4X Status=%.4X",SH2_Get_SR(&S_SH2) & 1, (SH2_Get_SR(&S_SH2) >> 1) & 1, (SH2_Get_SR(&S_SH2) >> 8) & 1, (SH2_Get_SR(&S_SH2) >> 9) & 1, (SH2_Get_SR(&S_SH2) >> 4) & 0xF,SH2_Get_SR(&S_SH2), S_SH2.Status & 0xFFFF);
	SendDlgItemMessage(hSSH2 , IDC_SSH2_STATUS_SR, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "R0=%.8X R1=%.8X R2=%.8X R3=%.8X\nR4=%.8X R5=%.8X R6=%.8X R7=%.8X\nR8=%.8X R9=%.8X RA=%.8X RB=%.8X\nRC=%.8X RD=%.8X RE=%.8X RF=%.8X", SH2_Get_R(&S_SH2, 0), SH2_Get_R(&S_SH2, 1), SH2_Get_R(&S_SH2, 2), SH2_Get_R(&S_SH2, 3), SH2_Get_R(&S_SH2, 4), SH2_Get_R(&S_SH2, 5), SH2_Get_R(&S_SH2, 6), SH2_Get_R(&S_SH2, 7), SH2_Get_R(&S_SH2, 8), SH2_Get_R(&S_SH2, 9), SH2_Get_R(&S_SH2, 0xA), SH2_Get_R(&S_SH2, 0xB), SH2_Get_R(&S_SH2, 0xC), SH2_Get_R(&S_SH2, 0xD), SH2_Get_R(&S_SH2, 0xE), SH2_Get_R(&S_SH2, 0xF));
	SendDlgItemMessage(hSSH2 , IDC_SSH2_STATUS_ADR, WM_SETTEXT, 0 , (LPARAM)debug_string);

	wsprintf(debug_string, "GBR=%.8X VBR=%.8X PR=%.8X\nMACH=%.8X MACL=%.8X\nIL=%.2X IV=%.2X", SH2_Get_GBR(&S_SH2), SH2_Get_VBR(&S_SH2), SH2_Get_PR(&S_SH2), SH2_Get_MACH(&S_SH2), SH2_Get_MACL(&S_SH2), S_SH2.INT.Prio,S_SH2.INT.Vect);
	SendDlgItemMessage(hSSH2 , IDC_SSH2_STATUS_DATA, WM_SETTEXT, 0 , (LPARAM)debug_string);
	
	wsprintf(debug_string, "PC=%.8X", SH2_Get_PC(&S_SH2));//(S_SH2.PC - S_SH2.Base_PC) - 4);
	SendDlgItemMessage(hSSH2, IDC_SSH2_PC, WM_SETTEXT, (WPARAM)0, (LPARAM) debug_string);

}

void DumpSSH2Cache_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_SSH2");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "Cache dump (*.dat)\0*.dat\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump SSH2 cache";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "dat";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 0x1000;
    WriteFile(hFr, S_SH2.Cache, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	Put_Info("SSH2 Cache dumped", 1500);
}

void SwitchSSH2ViewMode_KMod(  )
{
	SCROLLINFO si;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS; 
	si.nMin   = 0; 
	si.nPage  = 13;

	if (SSH2_ViewMode&2)
	{
		if (SSH2_ViewMode&1)
		{
			// ROM DISASM VIEW
			si.nPos   = SSH2_StartLineROMDisasm; 
			si.nMax   = ((1 * 1024)/2) -1;
			SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View ROM");
		}
		else 
		{
			// ROM VIEW
			si.nPos   = SSH2_StartLineROM; 
			si.nMax   = ((1 * 1024)/8) -1;
			SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}
	}
	else if (SSH2_ViewMode&8)
	{
		if (SSH2_ViewMode&4)
		{
			// RAM DISASM VIEW
			si.nPos   = SSH2_StartLineRAMDisasm; 
			si.nMax   = ((256 * 1024)/2) -1;
			SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View RAM");
		}
		else
		{
			// RAM VIEW
			si.nPos   = SSH2_StartLineRAM; 
			si.nMax   = ((256 * 1024)/8) -1;
			SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}
	}
	else if (SSH2_ViewMode&0x20)
	{
		/*
		if (SSH2_ViewMode&0x10)
		{
			// CACHE DISASM VIEW
			si.nPos   = SSH2_StartLineRAMDisasm; 
			si.nMax   = ((256 * 1024)/2) -1;
			SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View RAM");
		}
		else
		{*/
			// CACHE VIEW
			si.nPos   = SSH2_StartLineCache; 
			si.nMax   = (0x1000/8) -1;
/*
			SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}*/
	}
	SetScrollInfo(GetDlgItem(hSSH2, IDC_SSH2_SCROLL), SB_CTL, &si, TRUE); 
}

BOOL CALLBACK SSH2DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;
	SCROLLINFO si;
	unsigned int curPC;
	switch(Message)
    {
		case WM_INITDIALOG:
			hFont = (HFONT) GetStockObject(OEM_FIXED_FONT)	;
			SendDlgItemMessage(hwnd, IDC_SSH2_DISAM, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_SSH2_STATUS_SR, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_SSH2_STATUS_ADR, WM_SETFONT, (WPARAM)hFont, TRUE);
			SendDlgItemMessage(hwnd, IDC_SSH2_STATUS_DATA, WM_SETFONT, (WPARAM)hFont, TRUE);
			SSH2_ViewMode = 0x7; //disasm ROM
			SSH2_StartLineROMDisasm= SSH2_StartLineROM= SSH2_StartLineRAMDisasm = SSH2_StartLineRAM= SSH2_StartLineCache=0;
			SwitchSSH2ViewMode_KMod( ); // init with wrong values (since no game loaded by default)
			break;

		case WM_SHOWWINDOW:
			SwitchSSH2ViewMode_KMod( );
			break;	

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_SSH2_DUMP_ROM:
					Dump32XRom_KMod( hwnd );
					break;
				case IDC_SSH2_DUMP_RAM:
					Dump32XRam_KMod( hwnd );
					break;
				case IDC_SSH2_DUMP_CACHE:
					DumpSSH2Cache_KMod( hwnd );
					break;
				case IDC_SSH2_VIEW_ROM:
					SSH2_ViewMode &= 0x15;
					SSH2_ViewMode ^= 0x1;
					SSH2_ViewMode |= 0x2;
					SwitchSSH2ViewMode_KMod( );
					UpdateWindow (hwnd); 
					UpdateSSH2_KMod( );
					break;
				case IDC_SSH2_VIEW_RAM:
					SSH2_ViewMode &= 0x15;
					SSH2_ViewMode ^= 0x4;
					SSH2_ViewMode |= 0x8;

					SwitchSSH2ViewMode_KMod(  );
					UpdateWindow (hwnd); 
					UpdateSSH2_KMod( );
					break;
				case IDC_SSH2_VIEW_CACHE:
					SSH2_ViewMode &= 0x15;
//					SSH2_ViewMode ^= 0x10;
					SSH2_ViewMode |= 0x20;

					SwitchSSH2ViewMode_KMod(  );
					UpdateWindow (hwnd); 
					UpdateSSH2_KMod( );
					break;

				case IDC_SSH2_PC:
					curPC = SH2_Get_PC(&S_SH2); //(S_SH2.PC - S_SH2.Base_PC) - 4;
					if (curPC < 0x2400000)
					{
						SSH2_ViewMode &= 0x15;
						SSH2_ViewMode |= 0x2;
						SSH2_StartLineROM = (curPC-0x02000000)/8;
						SSH2_StartLineROMDisasm = (curPC-0x02000000)/2;
						SwitchSSH2ViewMode_KMod( );
					}
					else if (curPC < 0x6040000)
					{
						SSH2_ViewMode &= 0x15;
						SSH2_ViewMode |= 0x8;
						SSH2_StartLineRAM = (curPC-0x6000000)/8;
						SSH2_StartLineRAMDisasm = (curPC-0x6000000)/2;
						SwitchSSH2ViewMode_KMod( );
					}
					
					UpdateWindow (hwnd); 
					UpdateSSH2_KMod( );
					break;

			}
			break;

		case WM_VSCROLL:
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_ALL; //SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS; 
			GetScrollInfo((HWND) lParam, SB_CTL, &si);
			switch(LOWORD (wParam)) 
			{ 
				case SB_PAGEUP: 
					 si.nPos -= si.nPage; 
					 break; 
				case SB_PAGEDOWN: 
					 si.nPos += si.nPage;
					 break; 
				case SB_LINEUP: 
					 si.nPos--; 
					 break;
				case SB_LINEDOWN: 
					 si.nPos++; 
					 break; 
//				case SB_THUMBPOSITION:
				case SB_THUMBTRACK: 
					// don't change!! else you'll loose the 32bits value for a 16bits value
//					 si.nPos = HIWORD(wParam); 
					 si.nPos = si.nTrackPos;
					 break; 
			} 
 
			si.cbSize = sizeof(si); 
			si.fMask  = SIF_POS; 
			SetScrollInfo((HWND) lParam, SB_CTL, &si, TRUE); 
			UpdateWindow (hwnd); 
			UpdateSSH2_KMod( );
			return 0; 

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_32_SSH2 );
			break;

		case WM_DESTROY:
			DeleteObject( (HGDIOBJ) hFont );			
			DestroyWindow(hSSH2);
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;
}

/************ 32X VDP **************/

void Update32X_VDP_KMod( )
{
	if ( OpenedWindow_KMod[ 8 ] == FALSE )	return;

   RedrawWindow( GetDlgItem(h32X_VDP, IDC_32XVDP_TILES), NULL, NULL, RDW_INVALIDATE);
   RedrawWindow( GetDlgItem(h32X_VDP, IDC_32XVDP_TILES2), NULL, NULL, RDW_INVALIDATE);
   RedrawWindow( GetDlgItem(h32X_VDP, IDC_32XVDP_PAL), NULL, NULL, RDW_INVALIDATE);

}

void Draw32XPal_KMod( LPDRAWITEMSTRUCT hlDIS  )
{

	unsigned char i, j, tone;
	HBRUSH newBrush = NULL;
	HPEN hPen, hPenOld;
	RECT rc;
	WORD pix, h;
	COLORREF col;

	palV = (hlDIS->rcItem.bottom - hlDIS->rcItem.top)/64;
	palH = (hlDIS->rcItem.right - hlDIS->rcItem.left)/4;

	for(j = 0; j < 64; j++)
	{
		rc.top = j* palV;
		rc.bottom = rc.top + palV;

		for(i = 0; i < 4; i++)
		{
			if (newBrush)	DeleteObject( (HGDIOBJ) newBrush );
			rc.left = i*palH;
			rc.right = rc.left + palH;

			// COLORREF = 0x00bbggrr 
			// pix = bgr (3*5bit)
			pix = _32X_VDP_CRam[ i + j*4];
			col = 0x000000;
			tone = (pix>>10)&0x1F;
			col |= (tone<<3);
			col<<=8;
			tone = (pix>>5)&0x1F;
			col |= (tone<<3);
			col<<=8;
			tone = pix&0x1F;
			col |= (tone<<3);

			newBrush =  CreateSolidBrush( col );
			FillRect(hlDIS->hDC, &rc, newBrush);

			if (pix&0x80)
			{				
				//FillRect(hlDIS->hDC, &rc, (HBRUSH) (COLOR_WINDOWFRAME+1));

				hPen = CreatePen(  PS_DOT, 1, col);
				hPenOld = SelectObject(hlDIS->hDC, hPen); 

				for (h=rc.top; h < rc.bottom; h+=4)
				{
					MoveToEx(hlDIS->hDC, rc.left, h, NULL);
					LineTo(hlDIS->hDC, rc.right, h);
					MoveToEx(hlDIS->hDC, rc.left+2, h+2, NULL);
					LineTo(hlDIS->hDC, rc.right, h+2);
				}

				SelectObject(hlDIS->hDC, hPenOld); 
                DeleteObject(hPen); 

			}

		}
	}

	if (newBrush)	DeleteObject( (HGDIOBJ) newBrush );



}

void Draw32XVDP_KMod( LPDRAWITEMSTRUCT hlDIS  )
{
	unsigned char toDraw, tone;
	WORD pix, lineToDraw;
	WORD *VRAM, *lineTable;
	COLORREF col;
	WORD posX, posY, maxY, maxX;
 
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC( hlDIS->hDC );
	hBitmap = CreateCompatibleBitmap( hlDIS->hDC,  hlDIS->rcItem.right - hlDIS->rcItem.left, hlDIS->rcItem.bottom - hlDIS->rcItem.top);
	hOldBitmap = SelectObject(hDC, hBitmap);
	

	toDraw = (unsigned char) _32X_VDP.State&1;
	if ( IsDlgButtonChecked(h32X_VDP,IDC_32XVDP_FB2)==BST_CHECKED )
	{
		toDraw = 0;
	}
	else if ( IsDlgButtonChecked(h32X_VDP,IDC_32XVDP_FB2)==BST_CHECKED )
	{
		toDraw = 1;
	}
	
	lineTable = _32X_VDP_Ram + (toDraw*0x20000);

	maxX = 320;
	maxY = 240;

	for(posY = 0; posY < maxY; posY++)
	{

		lineToDraw = lineTable[posY];
		VRAM = lineTable + lineToDraw;
		for(posX = 0; posX < maxX; posX++)
		{
			// COLORREF = 0x00bbggrr 
			// pix = bgr (3*5bit)
			pix = VRAM[ posX ];
			col = 0x000000;
			tone = (pix>>10)&0x1F;
			col |= (tone<<3);
			col<<=8;
			tone = (pix>>5)&0x1F;
			col |= (tone<<3);
			col<<=8;
			tone = pix&0x1F;
			col |= (tone<<3);


			SetPixelV(hDC, posX, posY, col );
		}
	}


	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left 
               // corner
		0,  // y-coordinate of destination rectangle's upper-left 
               // corner
		maxX,  // width of destination rectangle
		maxY, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left 
               // corner
		0,   // y-coordinate of source rectangle's upper-left 
               // corner
		SRCCOPY  // raster operation code
	);

	SelectObject(hDC, hOldBitmap);
    DeleteObject( hBitmap );

	DeleteDC( hDC );
}

void Draw32XVDPRaw_KMod( LPDRAWITEMSTRUCT hlDIS  )
{
	unsigned char toDraw, tone;
	WORD pix;
	WORD *VRAM;
	COLORREF col;
	WORD posX, posY, maxY, maxX;
 
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC( hlDIS->hDC );
	hBitmap = CreateCompatibleBitmap( hlDIS->hDC,  hlDIS->rcItem.right - hlDIS->rcItem.left, hlDIS->rcItem.bottom - hlDIS->rcItem.top);
	hOldBitmap = SelectObject(hDC, hBitmap);
	

	toDraw = (unsigned char) _32X_VDP.State&1;
	if ( IsDlgButtonChecked(h32X_VDP,IDC_32XVDP_FB2)==BST_CHECKED )
	{
		toDraw = 0;
	}
	else if ( IsDlgButtonChecked(h32X_VDP,IDC_32XVDP_FB2)==BST_CHECKED )
	{
		toDraw = 1;
	}
	
	VRAM = _32X_VDP_Ram + (toDraw*0x20000) + 256*2; //skip 256 words of line table

	
	maxX = 320;
	maxY = 240;

	for(posY = 0; posY < maxY; posY++)
	{
		for(posX = 0; posX < maxX; posX++)
		{
		// COLORREF = 0x00bbggrr 
		// pix = bgr (3*5bit)
		pix = VRAM[ posX + (posY*maxX) ];
		col = 0x000000;
		tone = (pix>>10)&0x1F;
		col |= (tone<<3);
		col<<=8;
		tone = (pix>>5)&0x1F;
		col |= (tone<<3);
		col<<=8;
		tone = pix&0x1F;
		col |= (tone<<3);


		SetPixelV(hDC, posX, posY, col );

		}
	}


	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left 
               // corner
		0,  // y-coordinate of destination rectangle's upper-left 
               // corner
		maxX,  // width of destination rectangle
		maxY, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left 
               // corner
		0,   // y-coordinate of source rectangle's upper-left 
               // corner
		SRCCOPY  // raster operation code
	);

	SelectObject(hDC, hOldBitmap);
    DeleteObject( hBitmap );

	DeleteDC( hDC );
}


void Dump32XCRAM_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_CRAM");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "32X CRAM dump (*.bin)\0*.bin\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump 32X CRAM";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "bin";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 0x100;
    WriteFile(hFr, _32X_VDP_CRam_Ajusted, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	Put_Info("32X CRAM dumped", 1500);
}

void Dump32XVDP_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_VDP");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "Frame Buffer dump (*.bin)\0*.bin\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump 32X Frame buffer";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "bin";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;

	dwBytesToWrite = 0x100 * 1024;
    WriteFile(hFr, _32X_VDP_Ram, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	Put_Info("32X Frame buffer dumped", 1500);
}


BOOL CALLBACK _32X_VDPDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch(Message)
    {
		case WM_INITDIALOG:
			CheckRadioButton( hwnd, IDC_32XVDP_FB0, IDC_32XVDP_FB2, IDC_32XVDP_FB0 );
			break;

		case WM_DRAWITEM:
			if ( (UINT) wParam == IDC_32XVDP_TILES )
				Draw32XVDPRaw_KMod( (LPDRAWITEMSTRUCT) lParam);
			else if ( (UINT) wParam == IDC_32XVDP_TILES2 )
				Draw32XVDP_KMod( (LPDRAWITEMSTRUCT) lParam);
			else if ( (UINT) wParam == IDC_32XVDP_PAL )
				Draw32XPal_KMod( (LPDRAWITEMSTRUCT) lParam);
			break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_32XVDP_DUMP:
					Dump32XVDP_KMod( h32X_VDP );
					break;
				case IDC_32XVDP_CRAM:
					Dump32XCRAM_KMod( h32X_VDP );
					break;
				case IDC_32XVDP_FB0:
				case IDC_32XVDP_FB1:
				case IDC_32XVDP_FB2:
					Update32X_VDP_KMod( );
					break;
			}
			break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_32_VDP );
			break;

		case WM_DESTROY:
			DestroyWindow( h32X_VDP );
			PostQuitMessage(0);
			break;
		default:
            return FALSE;
    }
    return TRUE;
}


/************ 32X REG **************/
HWND h32XRegList;

void _32X_RegInit_KMod( HWND hwnd)
{
	LV_COLUMN   lvColumn;
	LVITEM		lvItem;
	int         i, adr;
	char		buf[64];
	TCHAR       szString[6][20] = {"Description", "MD Address", "Value", "Value", "32X Address", "Description"};

	h32XRegList = GetDlgItem(hwnd, IDC_32XREG_LIST);
	ListView_DeleteAllItems(h32XRegList);

//	GetWindowRect( h32XRegList, &rSize);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 140;
	lvColumn.pszText = szString[0];
	ListView_InsertColumn(h32XRegList, 0, &lvColumn);

	lvColumn.cx = 80;
	lvColumn.pszText = szString[1];
	ListView_InsertColumn(h32XRegList, 1, &lvColumn);
	
	lvColumn.cx = 60;
	lvColumn.pszText = szString[2];
	ListView_InsertColumn(h32XRegList, 2, &lvColumn);
	
	lvColumn.cx = 60;
	lvColumn.pszText = szString[3];
	ListView_InsertColumn(h32XRegList, 3, &lvColumn);

	lvColumn.cx = 80;
	lvColumn.pszText = szString[4];
	ListView_InsertColumn(h32XRegList, 4, &lvColumn);

	lvColumn.cx = 140;
	lvColumn.pszText = szString[5];
	ListView_InsertColumn(h32XRegList, 5, &lvColumn);
	
	ListView_SetExtendedListViewStyle(h32XRegList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );


	i = 0;
	while( _32X_register[i].side != 0xff )
	{

		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		lvItem.pszText =  _32X_register[i].descriptionMD;
		ListView_InsertItem(h32XRegList, &lvItem);


		if (_32X_register[i].side&1)
		{
			adr = 0xA15100 ;
			adr |=  _32X_register[i].adr;

			lvItem.iSubItem = 1;
			wsprintf(buf, "0x%0.6X", adr);
			lvItem.pszText = buf;
			ListView_SetItem(h32XRegList, &lvItem);
		
		}

		if (_32X_register[i].side&2)
		{
			adr = 0x4000 ;
			adr |=  _32X_register[i].adr;

			lvItem.iSubItem = 4;
			wsprintf(buf, "0x%0.6X", adr);
			lvItem.pszText = buf;
			ListView_SetItem(h32XRegList, &lvItem);
		
		}


		lvItem.iSubItem = 5;
		lvItem.pszText =  _32X_register[i].description32X;
		ListView_SetItem(h32XRegList, &lvItem);
		
		i++;
	
	}
	ListView_Scroll(h32XRegList, 100, 0);
}

void Update32X_Reg_KMod( )
{
	int         i;
	char		buf[64];
	LVITEM		lvItem;

	if ( OpenedWindow_KMod[ DMODE_32_REG -1] == FALSE )	return;

	lvItem.mask = LVIF_TEXT;


	i = 0;
	while( _32X_register[i].side != 0xff )
	{
		lvItem.iItem = i;
		if (_32X_register[i].side&1)
		{
			wsprintf(buf, "0x%0.4X",  M68K_RW( (0xA15100 | _32X_register[i].adr) ));
			lvItem.iSubItem = 2;
			lvItem.pszText = buf;
			ListView_SetItem(h32XRegList, &lvItem);
		}

		if (_32X_register[i].side&2)
		{
			wsprintf(buf, "0x%0.4X",  SH2_Read_Word( &M_SH2, (0x4000 | _32X_register[i].adr) ));
			lvItem.iSubItem = 3;
			lvItem.pszText = buf;
			ListView_SetItem(h32XRegList, &lvItem);
		}
		
		i++;
		
	}
}


BOOL CALLBACK _32X_RegDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch(Message)
    {
		case WM_INITDIALOG:
			_32X_RegInit_KMod( hwnd );
			break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_32_REG );
			break;

		case WM_DESTROY:			
			DestroyWindow(h32X_Reg);
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;
}




/************ VDP REG **************/
HWND hVDPReg_List;

void VDPRegInit_KMod( HWND hwnd)
{
	LV_COLUMN   lvColumn;
	LVITEM		lvItem;
	int         i;
	char		buf[64];

	hVDPReg_List = GetDlgItem(hwnd, IDC_VDP_REG_LIST);
	ListView_DeleteAllItems(hVDPReg_List);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 40;
	lvColumn.pszText = "Reg";
	ListView_InsertColumn(hVDPReg_List, 0, &lvColumn);
	
	lvColumn.cx = 40;
	lvColumn.pszText = "Value";
	ListView_InsertColumn(hVDPReg_List, 1, &lvColumn);

	lvColumn.cx = 280;
	lvColumn.pszText = "Desc";
	ListView_InsertColumn(hVDPReg_List, 2, &lvColumn);

	ListView_SetExtendedListViewStyle(hVDPReg_List, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	for (i=0; i<= 23; i++)
	{
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 0;
		wsprintf(buf, "#%0.2d", i);
		lvItem.pszText = buf;
		lvItem.iItem = i;
		ListView_InsertItem(hVDPReg_List, &lvItem);
/*
		lvItem.iSubItem = 1;
		lvItem.pszText = "";
		ListView_SetItem(hVDPReg_List, &lvItem);
		
		lvItem.iSubItem = 2;
		lvItem.pszText = "";
		ListView_SetItem(hVDPReg_List, &lvItem);	
		*/
	}
}

void UpdateVDPReg_KMod( )
{
	BYTE		valReg;
	LVITEM		lvItem;

	if ( OpenedWindow_KMod[ 9 ] == FALSE )	return;

	lvItem.mask = LVIF_TEXT;
	
	// Reg 0
	valReg = VDP_Reg.Set1;
	lvItem.iItem = 0;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "");
	if (valReg&0x10)	lstrcat(debug_string, "HIntEnable ");
	if (valReg&0x02)	lstrcat(debug_string, "HV counter stop ");

	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 1
	valReg = VDP_Reg.Set2;
	lvItem.iItem = 1;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "");
	if (valReg&0x40)	lstrcat(debug_string, "DisplayEnable ");
	if (valReg&0x20)	lstrcat(debug_string, "VintEnable ");
	if (valReg&0x10)	lstrcat(debug_string, "DMAEnable ");

	if (valReg&0x08)
		lstrcat(debug_string, "V30");
	else
		lstrcat(debug_string, "V28");

	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);



	// Reg 2
	valReg = VDP_Reg.Pat_ScrA_Adr;
	lvItem.iItem = 2;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "FieldA: 0x%.2X00", valReg << 2);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 3
	valReg = VDP_Reg.Pat_Win_Adr;
	lvItem.iItem = 3;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "Window: 0x%.2X00", valReg << 2);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 4
	valReg = VDP_Reg.Pat_ScrB_Adr;
	lvItem.iItem = 4;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "FieldB: 0x%.2X00", valReg << 5);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);
	
	// Reg 5
	valReg = VDP_Reg.Spr_Att_Adr;
	lvItem.iItem = 5;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "Sprite: 0x%.2X00", valReg << 1);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 6
	valReg = VDP_Reg.Reg6;
	lvItem.iItem = 6;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 7
	valReg = VDP_Reg.BG_Color;
	lvItem.iItem = 7;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "BG Color (Pal %d Idx %d)", valReg/16, valReg%16);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 8
	valReg = VDP_Reg.Reg8;
	lvItem.iItem = 8;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 9
	valReg = VDP_Reg.Reg9;
	lvItem.iItem = 9;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 10
	valReg = VDP_Reg.H_Int;
	lvItem.iItem = 10;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "HInt value : %d", valReg);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);
	
	// Reg 11
	valReg = VDP_Reg.Set3;
	lvItem.iItem = 11;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, " ");
	if (valReg&0x08)	lstrcat(debug_string, "EIntEnable ");

	if (valReg&0x04)
		lstrcat(debug_string, "Half VScroll ");
	else
		lstrcat(debug_string, "Full VScroll ");

	switch (valReg&0x03)
	{
		case 0:
			lstrcat(debug_string, "Full HScroll");
			break;
		case 1:
			break;
		case 2:
			lstrcat(debug_string, "Cell HScroll");
			break;
		case 3:
			lstrcat(debug_string, "Line HScroll");
			break;
	}
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 12
	valReg = VDP_Reg.Set4;
	lvItem.iItem = 12;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, " ");
	if (valReg&0x08) lstrcat(debug_string, "Shadow/Highlight ");

	switch (valReg&0x06)
	{
		case 0:
			break;
		case 2:
			lstrcat(debug_string, "Interlace ");
			break;
		case 4:
			break;
		case 6:
			lstrcat(debug_string, "Double Interlace ");
			break;
	}
	if (valReg&0x01)
		lstrcat(debug_string, "H40");
	else
		lstrcat(debug_string, "H32");

	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 13
	valReg = VDP_Reg.H_Scr_Adr;
	lvItem.iItem = 13;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "HScroll: 0x%.2X00", valReg << 2);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);
	
	// Reg 14
	valReg = VDP_Reg.Reg14;
	lvItem.iItem = 14;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 15
	valReg = VDP_Reg.Auto_Inc;
	lvItem.iItem = 15;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "Auto Increment of %d", valReg);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 16
	valReg = VDP_Reg.Scr_Size;
	lvItem.iItem = 16;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, " ");
	switch (valReg&0x30)
	{
		case 0x00:
			lstrcat(debug_string, "V32 Cell ");
			break;
		case 0x10:
			lstrcat(debug_string, "V64 Cell ");
			break;
		case 0x20:
			break;
		case 0x30:
			lstrcat(debug_string, " V128 Cell");
			break;
	}
	switch (valReg&0x03)
	{
		case 0x00:
			lstrcat(debug_string, "H32 Cell");
			break;
		case 0x01:
			lstrcat(debug_string, "H64 Cell");
			break;
		case 0x02:
			break;
		case 0x03:
			lstrcat(debug_string, "H128 Cell");
			break;
	}
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);
	
	// Reg 17
	valReg = VDP_Reg.Win_H_Pos;
	lvItem.iItem = 17;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "Window is %d cells ", valReg&0x1F);
	if (valReg&0x80)
		lstrcat(debug_string, "from right");
	else
		lstrcat(debug_string, "from left");
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 18
	valReg = VDP_Reg.Win_V_Pos;
	lvItem.iItem = 18;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "Window is %d cells ", valReg&0x1F);
	if (valReg&0x80)
		lstrcat(debug_string, "from top");
	else
		lstrcat(debug_string, "from bottom");
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 19
	valReg = VDP_Reg.DMA_Lenght_L;
	lvItem.iItem = 19;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "see below";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 20
	valReg = VDP_Reg.DMA_Lenght_H;
	lvItem.iItem = 20;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "DMA Lenght 0x%0.2X%0.2X", valReg, VDP_Reg.DMA_Lenght_L);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 21
	valReg = VDP_Reg.DMA_Src_Adr_L;
	lvItem.iItem = 21;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "see below";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 22
	valReg = VDP_Reg.DMA_Src_Adr_M;
	lvItem.iItem = 22;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "see below";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 23
	valReg = VDP_Reg.DMA_Src_Adr_H;
	lvItem.iItem = 23;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "DMA Source 0x00%0.2X%0.2X%0.2X", valReg&0x3F, VDP_Reg.DMA_Src_Adr_M, VDP_Reg.DMA_Src_Adr_L);
	valReg >>= 6;
	lstrcat(debug_string, ", ");
	switch (valReg)
	{
		case 0x00:
		case 0x01:
			lstrcat(debug_string, "Memory copy");
			break;
		case 0x02:
			lstrcat(debug_string, "VRAM Fill");
			break;
		case 0x03:
			lstrcat(debug_string, "VRAM Copy");
			break;
	}
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);
}



BOOL CALLBACK VDPRegDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
    {
		case WM_INITDIALOG:
			VDPRegInit_KMod(hwnd);
			break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_MISC );
			break;
		case WM_DESTROY:
			DestroyWindow( hMisc );
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;
}


/******************** SPRITES ***************/
unsigned char TrueSize_KMod( unsigned short int data )
{
	switch( data)
	{
		case 0:
			return 8;
		case 1:
			return 16;
		case 2:
			return 24;
		case 3:
			return 32;
	}

	return 0;
}

void UpdateSprites_KMod( )
{
	unsigned int i, topIdx, selIdx;
	unsigned short int *sprData, tmp;
	unsigned char tmp_string[32];

	if ( OpenedWindow_KMod[ 10 ] == FALSE )	return;

	SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, WM_SETREDRAW   , (WPARAM) FALSE , (LPARAM) 0);

	topIdx = SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, LB_GETTOPINDEX  , (WPARAM) 0 , (LPARAM) 0);
	selIdx = SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, LB_GETCURSEL , (WPARAM) 0 , (LPARAM) 0);


	sprData = VRam + ( VDP_Reg.Spr_Att_Adr << 9 );

	SendDlgItemMessage(hSprites, IDC_SPRITES_LIST, LB_RESETCONTENT, (WPARAM) 0 , (LPARAM) 0);
	for(i = 0; i < 80; i++)
	{
		wsprintf(debug_string, "%.2d",i);
		wsprintf(tmp_string, "  %4d", sprData[0]&0x03FF);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string, "  %4d", sprData[3]&0x03FF);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string,"  %.2dx", TrueSize_KMod( ((sprData[1]&0x0C00)>>10) ) );
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string,"%.2d", TrueSize_KMod( ((sprData[1]&0x0300)>>8) ) );
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string,"  %2d", sprData[1]&0x007F);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string," %2d", (sprData[2]&0x6000)>>13 );
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string,"  %4d", (sprData[2]&0x07FF) );
		lstrcat(debug_string, tmp_string);
		tmp = 0;
		if ( sprData[2]&0x8000)	tmp +=100;
		if ( sprData[2]&0x1000)	tmp +=010;
		if ( sprData[2]&0x0800)	tmp +=001;
		wsprintf(tmp_string,"  %.3d", tmp );
		lstrcat(debug_string, tmp_string);
		SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, LB_INSERTSTRING, (WPARAM) i, (LPARAM) debug_string);

		sprData += 4;
	}

	SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, LB_SETCURSEL , (WPARAM) selIdx , (LPARAM) 0);
	SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, LB_SETTOPINDEX   , (WPARAM) topIdx , (LPARAM) 0);

	SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, WM_SETREDRAW   , (WPARAM) TRUE , (LPARAM) 0);

//    RedrawWindow(hSprites, NULL, NULL, RDW_INVALIDATE);
    RedrawWindow(GetDlgItem(hSprites, IDC_SPRITES_PREVIEW), NULL, NULL, RDW_INVALIDATE);
    RedrawWindow(GetDlgItem(hSprites, IDC_SPRITES_PREVIEW2), NULL, NULL, RDW_INVALIDATE);

}


void DrawSprite_KMod( LPDRAWITEMSTRUCT hlDIS  )
{
	unsigned int selIdx;
	unsigned short int *sprData, pal;
	WORD posX, posY;
	unsigned char sizeH, sizeV;
	unsigned short int numTile = 0;
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	selIdx = SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, LB_GETCURSEL , (WPARAM) 0 , (LPARAM) 0);
	if (selIdx == LB_ERR)
		return;

	sprData = VRam + ( VDP_Reg.Spr_Att_Adr << 9 );
	sprData += selIdx*4; /* each sprite is 4 short int data */

	numTile = sprData[2]&0x07FF;
	sizeH = TrueSize_KMod( ((sprData[1]&0x0C00)>>10) );
	sizeV = TrueSize_KMod( ((sprData[1]&0x0300)>>8 ) );

	hDC = CreateCompatibleDC( hlDIS->hDC );
	hBitmap = CreateCompatibleBitmap( hlDIS->hDC,  32, 32);
	hOldBitmap = SelectObject(hDC, hBitmap);
	FillRect(hDC, &(hlDIS->rcItem), (HBRUSH) (COLOR_3DFACE+1) );

	pal = (sprData[2]&0x6000)>>13;
	for(posX = 0; posX < sizeH; posX+=8)
	{
		for(posY = 0; posY < sizeV; posY+=8)
		{
			DrawTile_KMod( hDC, numTile++, posX, posY, (UCHAR) pal, 1 );
		}
	}

	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left 
               // corner
		0,  // y-coordinate of destination rectangle's upper-left 
               // corner
		32,  // width of destination rectangle
		32, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left 
               // corner
		0,   // y-coordinate of source rectangle's upper-left 
               // corner
		SRCCOPY  // raster operation code
	);

	SelectObject(hDC, hOldBitmap);
    DeleteObject( hBitmap );

	DeleteDC( hDC );

}


void DrawSpriteZoom_KMod( LPDRAWITEMSTRUCT hlDIS  )
{
	unsigned int selIdx;
	unsigned short int *sprData, pal;
	WORD posX, posY;
	unsigned char sizeH, sizeV, zoom;
	unsigned short int numTile = 0;
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	selIdx = SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, LB_GETCURSEL , (WPARAM) 0 , (LPARAM) 0);
	if (selIdx == LB_ERR)
		return;

	sprData = VRam + ( VDP_Reg.Spr_Att_Adr << 9 );
	sprData += selIdx*4; /* each sprite is 4 short int data */

	numTile = sprData[2]&0x07FF;
	sizeH = TrueSize_KMod( ((sprData[1]&0x0C00)>>10) );
	sizeV = TrueSize_KMod( ((sprData[1]&0x0300)>>8 ) );

	zoom = (hlDIS->rcItem.right - hlDIS->rcItem.left) / 32;
	zoom = min(zoom, (hlDIS->rcItem.bottom - hlDIS->rcItem.top) / 32);

	hDC = CreateCompatibleDC( hlDIS->hDC );
	hBitmap = CreateCompatibleBitmap( hlDIS->hDC,  32*zoom, 32*zoom);
	hOldBitmap = SelectObject(hDC, hBitmap);
	FillRect(hDC, &(hlDIS->rcItem), (HBRUSH) (COLOR_3DFACE+1) );

	pal = (sprData[2]&0x6000)>>13;
	for(posX = 0; posX < sizeH; posX+=8)
	{
		for(posY = 0; posY < sizeV; posY+=8)
		{
			DrawTile_KMod( hDC, numTile++, posX*zoom, posY*zoom, (UCHAR) pal, zoom );
		}
	}


	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left 
               // corner
		0,  // y-coordinate of destination rectangle's upper-left 
               // corner
		32*zoom,  // width of destination rectangle
		32*zoom, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left 
               // corner
		0,   // y-coordinate of source rectangle's upper-left 
               // corner
		SRCCOPY  // raster operation code
	);

	SelectObject(hDC, hOldBitmap);
    DeleteObject( hBitmap );

	DeleteDC( hDC );


}

void DumpSprite_KMod( HWND hwnd )
{
	BITMAPFILEHEADER	bmfh;
	BITMAPINFOHEADER	bmiHeader;
	RGBQUAD				bmiColors[16];
	LPBYTE				pBits ;

	OPENFILENAME		szFile;
    CHAR				szFileName[MAX_PATH];
	HANDLE				hFr;
    DWORD				dwBytesToWrite, dwBytesWritten ;

	COLORREF			tmpColor;
	unsigned short int	numTile, selIdx;
	unsigned char		TileData, sizeH, sizeV, posX, posY, j, pal, tmp;
	unsigned short int	*sprData;

	selIdx = SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, LB_GETCURSEL , (WPARAM) 0 , (LPARAM) 0);
	if (selIdx == LB_ERR)
		return;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_Spr");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "16Colors Bitmap file (*.bmp)\0*.bmp\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump sprite";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "bmp";

    if (GetSaveFileName(&szFile)!=TRUE)   return;


	sprData = VRam + ( VDP_Reg.Spr_Att_Adr << 9 );
	sprData += selIdx*4; /* each sprite is 4 short int data */

	numTile = sprData[2]&0x07FF;
	sizeH = TrueSize_KMod( ((sprData[1]&0x0C00)>>10) );
	sizeV = TrueSize_KMod( ((sprData[1]&0x0300)>>8 ) );
	pal = (sprData[2]&0x6000)>>13;

	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = sizeH;
    bmiHeader.biHeight = sizeV;
    bmiHeader.biPlanes = 0;
    bmiHeader.biBitCount = 4;
    bmiHeader.biClrUsed = 16;
    bmiHeader.biCompression = BI_RGB;
    bmiHeader.biSizeImage = sizeH*sizeV/2;
    bmiHeader.biClrImportant = 0;


	bmfh.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
	bmfh.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) +  sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) + bmiHeader.biSizeImage);
	bmfh.bfOffBits = (DWORD) (sizeof(BITMAPFILEHEADER) +  sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) );
	
	
	for(j = 0; j < 16; j++)
	{
		tmpColor = GetPal_KMod(pal, j);
		bmiColors[j].rgbRed = (BYTE) tmpColor & 0xFF;
		tmpColor>>=8;
		bmiColors[j].rgbGreen = (BYTE) tmpColor & 0xFF;
		tmpColor>>=8;
		bmiColors[j].rgbBlue = (BYTE) tmpColor & 0xFF;
	}

	pBits = (LPBYTE) LocalAlloc(LPTR, bmiHeader.biSizeImage);
	if (pBits == NULL)
	{
		return;
	}

	for(posX = 0; posX < sizeH; posX+=8)
	{
		for(posY = 0; posY < sizeV; posY+=8)
		{
			for(j = 0; j < 8; j++)
			{
				TileData = VRam[numTile*32 + j*4 + 1];
				tmp = posX/2 + ((sizeV-1)-(posY+j))*sizeH/2;
				pBits[ tmp ] = TileData;

				TileData = VRam[numTile*32 + j*4 ];
				tmp = posX/2 + ((sizeV-1)-(posY+j))*sizeH/2 + 1;
				pBits[ tmp ] = TileData;

				TileData = VRam[numTile*32 + j*4 + 3];
				tmp = posX/2 + ((sizeV-1)-(posY+j))*sizeH/2 + 2;
				pBits[ tmp ] = TileData;

				TileData = VRam[numTile*32 + j*4 + 2];
				tmp = posX/2 + ((sizeV-1)-(posY+j))*sizeH/2 + 3;
				pBits[ tmp ] = TileData;			
			}
			numTile++;
		}
	}

	hFr = CreateFile (szFileName, GENERIC_WRITE, (DWORD) 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
    if (hFr == INVALID_HANDLE_VALUE)
	{
		LocalFree((HLOCAL) pBits);
        return;
	}

	dwBytesToWrite = sizeof(BITMAPFILEHEADER);
    WriteFile(hFr, &bmfh, dwBytesToWrite, &dwBytesWritten, NULL) ;

	dwBytesToWrite = sizeof(BITMAPINFOHEADER);
    WriteFile(hFr, &bmiHeader, dwBytesToWrite, &dwBytesWritten, NULL) ;

	dwBytesToWrite = 16 * sizeof(RGBQUAD);
    WriteFile(hFr, bmiColors, dwBytesToWrite, &dwBytesWritten, NULL) ;

	dwBytesToWrite = bmiHeader.biSizeImage;
    WriteFile(hFr, pBits, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);

	LocalFree((HLOCAL) pBits);
	
	Put_Info("Sprite dumped", 1500);

}

BOOL CALLBACK SpritesDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;
	DWORD i;

	switch(Message)
    {
		case WM_INITDIALOG:			
			hFont = (HFONT) GetStockObject(ANSI_FIXED_FONT);
			SendDlgItemMessage(hwnd, IDC_SPRITES_LIST, WM_SETFONT, (WPARAM)hFont, TRUE);
			for(i = 0; i < 80; i++)
			{
				wsprintf(debug_string, "%.2d",i);
				SendDlgItemMessage(hSprites , IDC_SPRITES_LIST, LB_INSERTSTRING, (WPARAM) i , (LPARAM) debug_string);
			}

			break;
		case WM_DRAWITEM:
			if ( (UINT) wParam == IDC_SPRITES_PREVIEW )
				DrawSprite_KMod( (LPDRAWITEMSTRUCT) lParam);
			else if ( (UINT) wParam == IDC_SPRITES_PREVIEW2 )
				DrawSpriteZoom_KMod( (LPDRAWITEMSTRUCT) lParam);
			break;

        case WM_COMMAND: 
			switch (LOWORD(wParam))
			{ 
 
                case IDC_SPRITES_LIST:  
					switch (HIWORD(wParam))
					{ 
                        case LBN_SELCHANGE:
						    RedrawWindow( GetDlgItem(hSprites, IDC_SPRITES_PREVIEW), NULL, NULL, RDW_INVALIDATE);
						    RedrawWindow( GetDlgItem(hSprites, IDC_SPRITES_PREVIEW2), NULL, NULL, RDW_INVALIDATE);
							break;
					}
					break;
				case IDC_SPRITES_DUMP:
					DumpSprite_KMod( hSprites );
					break;
			}
			break;
		case WM_CLOSE:
			CloseWindow_KMod( DMODE_SPRITES );
			break;

		case WM_DESTROY:
			DeleteObject( (HGDIOBJ) hFont );
			DestroyWindow( hSprites );
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
    return TRUE;
}


/************ YM2612 **************/
HWND hTabYM2612;
UINT  notes[6][4];
UCHAR curAlgo;
/*RECT rcAlgo;*/
DWORD KDAC_ticks;
int KDAC_freq;

void SpyYM2612DAC( )
{
	DWORD curTicks;
	curTicks = z80_Read_Odo(&M_Z80);
	if (curTicks != KDAC_ticks)
	{
		if (CPU_Mode)
			KDAC_freq = Round_Double( ((double) CLOCK_PAL / 15.0)/(curTicks - KDAC_ticks) );
		else
			KDAC_freq = Round_Double( ((double) CLOCK_NTSC / 15.0)/(curTicks - KDAC_ticks) );
	}
	KDAC_ticks = curTicks;

}

//called every frame
void UpdateYM2612_KMod( )
{
	int timer;
	UCHAR curSel, op, chan, part, algo;
	UCHAR i, j;
	CHAR tmp_string[30];
	HBITMAP hBitmap;

	switch( YM2612.REG[0][0x28]&0x07 )
	{
		case 0:
			chan = 0;
			break;
		case 1:
			chan = 1;
			break;
		case 2:
			chan = 2;
			break;
		case 4:
			chan = 3;
			break;
		case 5:
			chan = 4;
			break;
		case 6:
			chan = 5;
			break;
		default:
			chan = 0;
			
	}
	
	op = YM2612.REG[0][0x28]&0xF0;
	op >>=4;

	if (op&0x01)
		notes[chan][0] = BST_CHECKED;
	else
		notes[chan][0] = BST_UNCHECKED;

	if (op&0x02)
		notes[chan][1] = BST_CHECKED;
	else
		notes[chan][1] = BST_UNCHECKED;

	if (op&0x04)
		notes[chan][2] = BST_CHECKED;
	else
		notes[chan][2] = BST_UNCHECKED;

	if (op&0x08)
		notes[chan][3] = BST_CHECKED;
	else
		notes[chan][3] = BST_UNCHECKED;
		
	
	if ( OpenedWindow_KMod[ 11 ] == FALSE )	return;


	curSel = TabCtrl_GetCurSel(hTabYM2612);


	part = 0;
	chan = curSel % 3;
	if (curSel >= 3)	part = 1;



	/* Operators */
	for (op=0; op<4; op++)
	{
		wsprintf(debug_string,"0x%0.2X\n", YM2612.REG[part][0x30 + chan + 4*op]>>4);
		wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0x30 + chan + 4*op]&0x0F);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0x40 + chan + 4*op]&0x7F);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0x50 + chan + 4*op]>>6);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0x50 + chan + 4*op]&0x1F);
		lstrcat(debug_string, tmp_string);
		if (YM2612.REG[0][0x60]&0x80)
			lstrcat(debug_string, "Yes\n");
		else
			lstrcat(debug_string, "No\n");
		wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0x60 + chan + 4*op]&0x1F);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0x70 + chan + 4*op]&0x1F);
		lstrcat(debug_string, tmp_string);

		wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0x80 + chan + 4*op]&0xF0 >>4);
		lstrcat(debug_string, tmp_string);

		wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0x80 + chan + 4*op]&0x0F);
		lstrcat(debug_string, tmp_string);

		wsprintf(tmp_string,"0x%0.2X", YM2612.REG[part][0x90 + chan + 4*op]&0xFF);
		lstrcat(debug_string, tmp_string);

		SetDlgItemText(hYM2612, IDC_YM2612_OP1+op, debug_string);
	}


	/* Channel */
	wsprintf(debug_string,"0x%0.2X%0.2X\n", YM2612.REG[part][0xA4 + chan]&0x07, YM2612.REG[part][0xA0 + chan]);
	wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0xA4 + chan]&0x38>>2);
	lstrcat(debug_string, tmp_string);
	SetDlgItemText(hYM2612, IDC_YM2612_FREQ, debug_string);
	
	wsprintf(debug_string,"0x%0.2X\n", (YM2612.REG[part][0xB4 + chan]&0x30)>>4);
	wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0xB4 + chan]&0x08 );
	lstrcat(debug_string, tmp_string);
	SetDlgItemText(hYM2612, IDC_YM2612_MOD, debug_string);
	
	if ( YM2612.REG[part][0xB4 + chan]&0x80)
		CheckDlgButton(hYM2612, IDC_YM2612_LEFT,BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_LEFT,BST_UNCHECKED);

	if ( YM2612.REG[part][0xB4 + chan]&0x40)
		CheckDlgButton(hYM2612, IDC_YM2612_RIGHT,BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_RIGHT,BST_UNCHECKED);

	wsprintf(debug_string,"0x%0.2X", (YM2612.REG[part][0xB0 + chan]&0x38)>>3);
	SetDlgItemText(hYM2612, IDC_YM2612_FBACK, debug_string);

	algo = YM2612.REG[part][0xB0 + chan]&0x07;
	/* optimization to avoid flickers */
	if (algo != curAlgo)
	{
		hBitmap = LoadBitmap( ghInstance, MAKEINTRESOURCE( IDB_ALGO0 + algo) );
		SendDlgItemMessage( hYM2612, IDC_YM2612_ALGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBitmap );
		curAlgo = algo;
	}


	/* global */
	if ( YM2612.REG[0][0x22]&0x08)
	{
		CheckDlgButton(hYM2612, IDC_YM2612_LFO,BST_CHECKED);
		wsprintf(debug_string,"LFO 0x%0.2X",  YM2612.REG[0][0x22]&0x07 );
	}
	else
	{
		CheckDlgButton(hYM2612, IDC_YM2612_LFO,BST_UNCHECKED);
		lstrcpy(debug_string,"LFO");
	}

	SetDlgItemText(hYM2612, IDC_YM2612_LFO, debug_string);

	timer = YM2612.REG[0][0x24];
	timer <<= 2;
	timer |= YM2612.REG[0][0x25];
	wsprintf(debug_string,"0x%0.4X\n0x%0.2X", timer, YM2612.REG[0][0x26] );
	SetDlgItemText(hYM2612, IDC_YM2612_TIMER, debug_string);

	if ( YM2612.REG[0][0x27]&0x01)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A1,BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A1,BST_UNCHECKED);

	if ( YM2612.REG[0][0x27]&0x02)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B1,BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B1,BST_UNCHECKED);

	if ( YM2612.REG[0][0x27]&0x04)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A2,BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A2,BST_UNCHECKED);

	if ( YM2612.REG[0][0x27]&0x08)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B2,BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B2,BST_UNCHECKED);

	if ( YM2612.REG[0][0x27]&0x10)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A3,BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A3,BST_UNCHECKED);

	if ( YM2612.REG[0][0x27]&0x20)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B3,BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B3,BST_UNCHECKED);


	switch( YM2612.REG[0][0x27]&0xC0 )
	{
		case 0:
			lstrcpy(debug_string,"Normal");
			break;
		case 0x40:
			lstrcpy(debug_string,"Special");
			break;
		case 0x80:
		case 0xC0:
		default:
			lstrcpy(debug_string,"Illegal");
			
	}
	SetDlgItemText(hYM2612, IDC_YM2612_C3MODE, debug_string);


	for (i=0; i<6; i++)
	{
		for(j=0; j<4; j++)
		{
			CheckDlgButton(hYM2612, IDC_YM2612_KEY_1_1 + j + i*4,notes[i][j]);
		}
	}


	if ( YM2612.DAC)
	{
		CheckDlgButton(hYM2612, IDC_YM2612_DAC,BST_CHECKED);
		wsprintf(debug_string,"DAC 0x%0.2X at %0.5dHz",  ((YM2612.DACdata)>>7)&0xFF, KDAC_freq );
	}
	else
	{
		CheckDlgButton(hYM2612, IDC_YM2612_DAC,BST_UNCHECKED);
		lstrcpy(debug_string,"DAC");
	}

	SetDlgItemText(hYM2612, IDC_YM2612_DAC, debug_string);
}


void YM2612_ChangeChannel( HWND hwnd )
{
	int curSel = TabCtrl_GetCurSel(hTabYM2612);

	/* enabled */
	if ( EnabledChannels[curSel] == TRUE)
		CheckDlgButton(hYM2612, IDC_YM2612_MUTE,BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_MUTE,BST_UNCHECKED);

	UpdateYM2612_KMod( );
}

void YM2612_ToggleMute_KMod( HWND hwnd )
{
	int tabIndex;
	TCITEM tab;

	tabIndex = TabCtrl_GetCurSel(hTabYM2612);

	tab.mask = TCIF_IMAGE;
	if ( IsDlgButtonChecked(hYM2612, IDC_YM2612_MUTE) == BST_CHECKED )
	{
		EnabledChannels[tabIndex] = TRUE;
		tab.iImage = 0;
	}
	else
	{
		EnabledChannels[tabIndex] = FALSE;
		tab.iImage = 1;		
	}

	TabCtrl_SetItem(hTabYM2612, tabIndex, &tab);
}

void DumpYM2612_KMod( HWND hwnd )
{
	OPENFILENAME		szFile;
    CHAR				szFileName[MAX_PATH];
	HANDLE				hFr;
    DWORD				dwBytesToWrite, dwBytesWritten ;
	UCHAR				curSel, op, chan, part, tmp;

    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_Chan");
 
    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "YM2612 voice dump (*.y12)\0*.y12\0TFM'ucker instrument (*.tfi)\0*.tfi\0VED voice dump (*.ff)\0*.ff\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Dump YM2612 voice";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "y12";

    if (GetSaveFileName(&szFile)!=TRUE)   return;


		
	curSel = TabCtrl_GetCurSel(hTabYM2612);
	part = 0;
	chan = curSel % 3;
	if (curSel >= 3)	part = 1;

	hFr = CreateFile (szFileName, GENERIC_WRITE, (DWORD) 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
    if (hFr == INVALID_HANDLE_VALUE)
	{
        return;
	}


    ZeroMemory(&KM2612, sizeof(szFile));

	/* operators data */
	for (op=0; op<4; op++)
	{
		KM2612.ym.op[op].op_data[0] = (UCHAR) (YM2612.REG[part][0x30 + chan + 4*op])&0xFF;
		KM2612.ym.op[op].op_data[1] = (UCHAR) (YM2612.REG[part][0x40 + chan + 4*op])&0xFF;
		KM2612.ym.op[op].op_data[2] = (UCHAR) (YM2612.REG[part][0x50 + chan + 4*op])&0xFF;
		KM2612.ym.op[op].op_data[3] = (UCHAR) (YM2612.REG[part][0x60 + chan + 4*op])&0xFF;
		KM2612.ym.op[op].op_data[4] = (UCHAR) (YM2612.REG[part][0x70 + chan + 4*op])&0xFF;
		KM2612.ym.op[op].op_data[5] = (UCHAR) (YM2612.REG[part][0x80 + chan + 4*op])&0xFF;
		KM2612.ym.op[op].op_data[6] = (UCHAR) (YM2612.REG[part][0x90 + chan + 4*op])&0xFF;
	}

	/* channel data */
	KM2612.ym.algo = (UCHAR) (YM2612.REG[part][0xB0 + chan])&0x07;
	KM2612.ym.fback = (UCHAR) (YM2612.REG[part][0xB0 + chan])&0x38;
	KM2612.ym.fback >>=3;

	if (szFile.nFilterIndex == 1)
	{
		/* y12 */		
		strncpy(KM2612.name, Rom_Name,16);
		strncpy(KM2612.dumper, Rom_Name,16);
		strncpy(KM2612.game, Rom_Name,16);

		dwBytesToWrite = sizeof(KM2612);
		WriteFile(hFr, &KM2612, dwBytesToWrite, &dwBytesWritten, NULL) ;
	}
	else if (szFile.nFilterIndex == 2)
	{
		/* tfi */
		dwBytesToWrite = 1;
		WriteFile(hFr, &KM2612.ym.algo, dwBytesToWrite, &dwBytesWritten, NULL) ;
		WriteFile(hFr, &KM2612.ym.fback, dwBytesToWrite, &dwBytesWritten, NULL) ;


		for (op=0; op<4; op++)
		{
			dwBytesToWrite = 1;
			//MUL
			tmp = (UCHAR) (KM2612.ym.op[op].op_data[0])&0xF;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;
			//DT1
			tmp = (UCHAR) (KM2612.ym.op[op].op_data[0])&0x70;
			tmp >>= 4;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;

			//TL
			tmp = (UCHAR) (KM2612.ym.op[op].op_data[1])&0x7F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;

			//RS
			tmp = (UCHAR) (KM2612.ym.op[op].op_data[2])&0xC0;
			tmp >>=6;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;
			//AR
			tmp = (UCHAR) (KM2612.ym.op[op].op_data[2])&0x1F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;

			//D1R
			tmp = (UCHAR) (KM2612.ym.op[op].op_data[3])&0x1F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;

			//D2R
			tmp = (UCHAR) (KM2612.ym.op[op].op_data[4])&0x1F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;

			//RR
			tmp = (UCHAR) (KM2612.ym.op[op].op_data[5])&0x0F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;
			//D1L
			tmp = (UCHAR) (KM2612.ym.op[op].op_data[5])&0xF0;
			tmp>>=4;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;

			//SSG-EG
			tmp = (UCHAR) (KM2612.ym.op[op].op_data[6])&0x0F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;
		
		}
	}
	else if (szFile.nFilterIndex == 3)
	{
		/* ff */
		for (tmp=0; tmp<6; tmp++)
		{
			for (op=0; op<4; op++)
			{
				dwBytesToWrite = 1;
				WriteFile(hFr, &KM2612.ym.op[op].op_data[tmp], dwBytesToWrite, &dwBytesWritten, NULL) ;
			}
		}

		tmp = (UCHAR) (YM2612.REG[part][0xB0 + chan])&0xFF;
		dwBytesToWrite = 1;
		WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL) ;
		
		strncpy(KM2612.name, Rom_Name,7);
		dwBytesToWrite = 7;
		WriteFile(hFr, KM2612.name, dwBytesToWrite, &dwBytesWritten, NULL);	
	}

    CloseHandle(hFr);

	Put_Info("YM2616 voice dumped", 1500);
}


BOOL CALLBACK YM2612DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
    {
		case WM_INITDIALOG:
			{
			HIMAGELIST himl = NULL;
			TCITEM tie; 
			int i;

			curAlgo = 0;
			KDAC_ticks = 0;
			KDAC_freq = 0;

			ZeroMemory(&tie, sizeof(TCITEM));
			hTabYM2612 = GetDlgItem(hwnd, IDC_YM2612_TAB);

			TabCtrl_DeleteAllItems(hTabYM2612);

			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage  = 0; 
			tie.pszText = "Channel 1";
			i = TabCtrl_InsertItem(hTabYM2612, 0, &tie);

			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage  = 0;
			tie.pszText = "Channel 2";
			i = TabCtrl_InsertItem(hTabYM2612, 1, &tie);

			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage  = 0;
			tie.pszText = "Channel 3";
			i = TabCtrl_InsertItem(hTabYM2612, 2, &tie);

			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage  = 0;
			tie.pszText = "Channel 4";
			i = TabCtrl_InsertItem(hTabYM2612, 3, &tie);

			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage  = 0;
			tie.pszText = "Channel 5";
			i = TabCtrl_InsertItem(hTabYM2612, 4, &tie);

			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage  = 0;
			tie.pszText = "Channel 6";
			i = TabCtrl_InsertItem(hTabYM2612, 5, &tie);


/*			GetWindowRect( GetDlgItem(hwnd, IDC_YM2612_ALGO), &rcAlgo );*/
			himl = ImageList_LoadImage(ghInstance, MAKEINTRESOURCE(IDB_YM2612_IMAGELIST), 16, 0, IMAGE_BITMAP, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_LOADTRANSPARENT);
			if (himl == NULL)
				return -1;

			TabCtrl_SetImageList(hTabYM2612, himl);

			//enabled by default
			CheckDlgButton(hwnd, IDC_YM2612_MUTE,BST_CHECKED);
			}
			break;

		case WM_COMMAND:
			if ( LOWORD(wParam) == IDC_YM2612_DUMP )
				DumpYM2612_KMod( hwnd );
			else if ( LOWORD(wParam) == IDC_YM2612_MUTE )
				YM2612_ToggleMute_KMod( hwnd );
			break;

		case WM_NOTIFY: 
            switch (((NMHDR *)lParam)->code)
			{ 
				case TCN_SELCHANGE:		
					YM2612_ChangeChannel( hwnd );
                    break; 

			}
			break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_YM2612 );
			break;

		case WM_DESTROY:
			DestroyWindow( hYM2612 );
			PostQuitMessage(0);
			break;

		default:
            return FALSE;
    }
	return TRUE;
}


/************ PSG **************/

HWND hVol1_PSG, hVol2_PSG,hVol3_PSG,hVol4_PSG;

void UpdatePSG_KMod( )
{
	int VolPer;
	if ( OpenedWindow_KMod[ 12 ] == FALSE )	return;

	wsprintf(debug_string,"%d Hz", PSG.Register[0]==0 ? 0 : (int)(3579545/(PSG.Register[0]*32)));
	SetDlgItemText(hPSG, IDC_PSG_FREQ_1, debug_string);
	wsprintf(debug_string,"%d", PSG.Register[0] );
	SetDlgItemText(hPSG, IDC_PSG_DATA_1, debug_string);
	wsprintf(debug_string,"%d Hz", PSG.Register[2]==0 ? 0 : (int)(3579545/(PSG.Register[2]*32)));
	SetDlgItemText(hPSG, IDC_PSG_FREQ_2, debug_string);
	wsprintf(debug_string,"%d", PSG.Register[2] );
	SetDlgItemText(hPSG, IDC_PSG_DATA_2, debug_string);
	wsprintf(debug_string,"%d Hz", PSG.Register[4]==0 ? 0 : (int)(3579545/(PSG.Register[4]*32)));
	SetDlgItemText(hPSG, IDC_PSG_FREQ_3, debug_string);
	wsprintf(debug_string,"%d", PSG.Register[4] );
	SetDlgItemText(hPSG, IDC_PSG_DATA_3, debug_string);
	wsprintf(debug_string,"%s", (PSG.Register[6]>>2)==1 ? "Periodic" : "White");
	SetDlgItemText(hPSG, IDC_PSG_FEEDBACK, debug_string);
	if((PSG.Register[6]&0x03)==0)
	{
		wsprintf(debug_string,"%s", "Clock/2");
	} else if((PSG.Register[6]&0x03)==0) {
		wsprintf(debug_string,"%s", "Clock/2");
	} else if((PSG.Register[6]&0x03)==1) {
		wsprintf(debug_string,"%s", "Clock/4");
	} else if((PSG.Register[6]&0x03)==2) {
		wsprintf(debug_string,"%s", "Clock/8");
	} else if((PSG.Register[6]&0x03)==3) {
		wsprintf(debug_string,"%s", "Tone 3");
	}
	SetDlgItemText(hPSG, IDC_PSG_CLOCK, debug_string);

	VolPer = (100* PSG.Volume[0])/PSG_MaxVolume;
	wsprintf(debug_string,"%d%%", VolPer );
	SetDlgItemText(hPSG, IDC_PSG_VOLUME_1, debug_string);
	SendMessage(hVol1_PSG, PBM_SETPOS, (WPARAM) VolPer, (LPARAM) 0);

	VolPer = (100* PSG.Volume[1])/PSG_MaxVolume;
	wsprintf(debug_string,"%d%%", VolPer );
	SetDlgItemText(hPSG, IDC_PSG_VOLUME_2, debug_string);
	SendMessage(hVol2_PSG, PBM_SETPOS, (WPARAM) VolPer, (LPARAM) 0);

	VolPer = (100* PSG.Volume[2])/PSG_MaxVolume;
	wsprintf(debug_string,"%d%%", VolPer );
	SetDlgItemText(hPSG, IDC_PSG_VOLUME_3, debug_string);
	SendMessage(hVol3_PSG, PBM_SETPOS, (WPARAM) VolPer, (LPARAM) 0);

	VolPer = (100* PSG.Volume[3])/PSG_MaxVolume;
	wsprintf(debug_string,"%d%%", VolPer );
	SetDlgItemText(hPSG, IDC_PSG_VOLUME_4, debug_string);
	SendMessage(hVol4_PSG, PBM_SETPOS, (WPARAM) VolPer, (LPARAM) 0);
}



BOOL CALLBACK PSGDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch(Message)
    {
		case WM_INITDIALOG:
			hVol1_PSG = GetDlgItem(hwnd, IDC_PSG_SLIDER_1);
			hVol2_PSG = GetDlgItem(hwnd, IDC_PSG_SLIDER_2);
			hVol3_PSG = GetDlgItem(hwnd, IDC_PSG_SLIDER_3);
			hVol4_PSG = GetDlgItem(hwnd, IDC_PSG_SLIDER_4);

			SendMessage(hVol1_PSG, PBM_SETPOS, (WPARAM) 0, (LPARAM) 0);
			SendMessage(hVol2_PSG, PBM_SETPOS, (WPARAM) 0, (LPARAM) 0);
			SendMessage(hVol3_PSG, PBM_SETPOS, (WPARAM) 0, (LPARAM) 0);
			SendMessage(hVol4_PSG, PBM_SETPOS, (WPARAM) 0, (LPARAM) 0);
			break;

		case WM_CLOSE:
			CloseWindow_KMod( DMODE_PSG );
			break;
		case WM_DESTROY:
			DestroyWindow( hPSG );
			PostQuitMessage(0);
			break;
		default:
            return FALSE;
    }
    return TRUE;
}




/************ Config **************/
void SaveConfig_KMod( )
{
	char Conf_File[MAX_PATH];

	SetCurrentDirectory(Gens_Path);
	strcpy(Conf_File, Gens_Path);
	strcat(Conf_File, "GensKMod.cfg");

	wsprintf(debug_string, "%d", KConf.bAutoWatch);
	WritePrivateProfileString("Debug", "AutoLoadW", debug_string, Conf_File);

	wsprintf(debug_string, "%d", KConf.Spy);
	WritePrivateProfileString("Debug", "Spy", debug_string, Conf_File);

	wsprintf(debug_string, "%d", KConf.SpyReg);
	WritePrivateProfileString("Debug", "Spy1", debug_string, Conf_File);

	wsprintf(debug_string, "%d", KConf.SpySpr);
	WritePrivateProfileString("Debug", "Spy2", debug_string, Conf_File);

	wsprintf(debug_string, "%d", KConf.SpyRW);
	WritePrivateProfileString("Debug", "Spy3", debug_string, Conf_File);
	
	wsprintf(debug_string, "%d", KConf.SpyDMA);
	WritePrivateProfileString("Debug", "Spy4", debug_string, Conf_File);

	wsprintf(debug_string, "%d", KConf.CDBios);
	WritePrivateProfileString("Debug", "SpyCD", debug_string, Conf_File);

	wsprintf(debug_string, "%d", KConf.noCDBSTAT);
	WritePrivateProfileString("Debug", "SpyCD1", debug_string, Conf_File);

	wsprintf(debug_string, "%d", KConf.noCDCSTAT);
	WritePrivateProfileString("Debug", "SpyCD2", debug_string, Conf_File);

	wsprintf(debug_string, "%d", KConf.bBluePause);
	WritePrivateProfileString("Debug", "BluePause", debug_string, Conf_File);	

	wsprintf(debug_string, "%d", KConf.Special);
	WritePrivateProfileString("Debug", "Special", debug_string, Conf_File);	
}

void LoadConfig_KMod( )
{
	unsigned char Conf_File[MAX_PATH];

	SetCurrentDirectory(Gens_Path);
	strcpy(Conf_File, Gens_Path);
	strcat(Conf_File, "GensKMod.cfg");
	
	KConf.bAutoWatch = (BOOL) GetPrivateProfileInt("Debug", "AutoLoadW", FALSE, Conf_File);
	KConf.Spy = (BOOL) GetPrivateProfileInt("Debug", "Spy", FALSE, Conf_File);
	KConf.SpyReg = (BOOL) GetPrivateProfileInt("Debug", "Spy1", FALSE, Conf_File);
	KConf.SpySpr = (BOOL) GetPrivateProfileInt("Debug", "Spy2", FALSE, Conf_File);
	KConf.SpyRW = (BOOL) GetPrivateProfileInt("Debug", "Spy3", FALSE, Conf_File);
	KConf.SpyDMA = (BOOL) GetPrivateProfileInt("Debug", "Spy4", FALSE, Conf_File);
	KConf.CDBios = (BOOL) GetPrivateProfileInt("Debug", "SpyCD", FALSE, Conf_File);
	KConf.noCDBSTAT = (BOOL) GetPrivateProfileInt("Debug", "SpyCD1", FALSE, Conf_File);
	KConf.noCDCSTAT = (BOOL) GetPrivateProfileInt("Debug", "SpyCD2", FALSE, Conf_File);
	KConf.bBluePause = (BOOL) GetPrivateProfileInt("Debug", "BluePause", TRUE, Conf_File);
	KConf.Special = (BOOL) GetPrivateProfileInt("Debug", "Special", FALSE, Conf_File);
}

void RefreshSpyButtons( HWND hwnd )
{
	long i;
	BOOL bEnable = FALSE;

	bEnable = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY);	
	for( i = IDC_DCONFIG_SPY1; i<= IDC_DCONFIG_SPY4; i++)
		EnableWindow( GetDlgItem( hwnd, i) , bEnable );

	bEnable = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPYCD);	
	for( i = IDC_DCONFIG_SPYCD1; i<= IDC_DCONFIG_SPYCD2; i++)
		EnableWindow( GetDlgItem( hwnd, i) , bEnable );
}


void Get_DebugZip_KMod( HWND hwnd )
{
	OPENFILENAME szFile;
    char szFileName[MAX_PATH];
    HANDLE hFr;
    DWORD dwBytesToWrite, dwBytesWritten ;

	HRSRC hr;
	HGLOBAL hg;
	LPVOID lpData;

	hr = FindResource (ghInstance, MAKEINTRESOURCE(IDR_DEBUGZIP), RT_RCDATA);
	if (hr == NULL)	return;

	dwBytesToWrite = SizeofResource(ghInstance, hr);
	if ( dwBytesToWrite == 0)	return;

	hg = LoadResource (ghInstance, hr);
	if (hg == NULL)	return;

	lpData = LockResource (hg);
	if (lpData == NULL)	return;


    ZeroMemory(&szFile, sizeof(szFile));
    szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, "debug_S");

    szFile.lStructSize = sizeof(szFile);
    szFile.hwndOwner = hwnd;
    szFile.lpstrFilter = "Zip file dump (*.zip)\0*.zip\0\0";
    szFile.lpstrFile= szFileName;
    szFile.nMaxFile = sizeof(szFileName);
    szFile.lpstrFileTitle = (LPSTR)NULL;
    szFile.lpstrInitialDir = (LPSTR)NULL;
    szFile.lpstrTitle = "Get Debug_S.zip";
    szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST  | OFN_HIDEREADONLY;
    szFile.lpstrDefExt = "zip";

    if (GetSaveFileName(&szFile)!=TRUE)   return;

    hFr = CreateFile (szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) ;
    if (hFr == INVALID_HANDLE_VALUE)
          return;


    WriteFile(hFr, lpData, dwBytesToWrite, &dwBytesWritten, NULL) ;

    CloseHandle(hFr);
}


BOOL CALLBACK ConfigKDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
    {
		case WM_INITDIALOG:
			if ( KConf.bAutoWatch )
				CheckDlgButton(hwnd, IDC_DCONFIG_AUTOW, BST_CHECKED);

			if ( KConf.Spy )
				CheckDlgButton(hwnd, IDC_DCONFIG_SPY, BST_CHECKED);

			if ( KConf.SpyReg )
				CheckDlgButton(hwnd, IDC_DCONFIG_SPY1, BST_CHECKED);

			if ( KConf.SpySpr )
				CheckDlgButton(hwnd, IDC_DCONFIG_SPY2, BST_CHECKED);

			if ( KConf.SpyRW )
				CheckDlgButton(hwnd, IDC_DCONFIG_SPY3, BST_CHECKED);

			if ( KConf.SpyDMA )
				CheckDlgButton(hwnd, IDC_DCONFIG_SPY4, BST_CHECKED);

			if ( KConf.CDBios )
				CheckDlgButton(hwnd, IDC_DCONFIG_SPYCD, BST_CHECKED);

			if ( KConf.noCDBSTAT )
				CheckDlgButton(hwnd, IDC_DCONFIG_SPYCD1, BST_CHECKED);

			if ( KConf.noCDCSTAT )
				CheckDlgButton(hwnd, IDC_DCONFIG_SPYCD2, BST_CHECKED);

			if ( KConf.bBluePause )
				CheckDlgButton(hwnd, IDC_DCONFIG_BSCREEN, BST_CHECKED);

			if ( KConf.Special )
				CheckDlgButton(hwnd, IDC_DCONFIG_SPECIAL, BST_CHECKED);

			RefreshSpyButtons( hwnd );

			break;

		case WM_COMMAND:
            switch(LOWORD(wParam))
            {
				case IDC_DCONFIG_AUTOW:
					break;
				case IDC_DCONFIG_SPY:
				case IDC_DCONFIG_SPYCD:
					RefreshSpyButtons( hwnd );				
					break;
				case IDC_DCONFIG_ZIP:
					Get_DebugZip_KMod( hwnd );
					break;
                case IDOK:
					KConf.bAutoWatch = IsDlgButtonChecked(hwnd, IDC_DCONFIG_AUTOW);
					KConf.Spy = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY);
					KConf.SpyReg = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY1);
					KConf.SpySpr = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY2);
					KConf.SpyRW  = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY3);
					KConf.SpyDMA  = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY4);
					KConf.CDBios = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPYCD);
					KConf.noCDBSTAT = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPYCD1);
					KConf.noCDCSTAT = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPYCD2);
					KConf.bBluePause  = IsDlgButtonChecked(hwnd, IDC_DCONFIG_BSCREEN);
					KConf.Special  = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPECIAL);
					SaveConfig_KMod( );
                    EndDialog(hwnd, IDOK);                    
                    break;
                case IDCANCEL:
					LoadConfig_KMod( );
                    EndDialog(hwnd, IDCANCEL);
                    break;
            }
            break;
		default:
            return FALSE;
    }
    return TRUE;
}



/********************************** PUBLIC ****************************************/
void Init_KMod( )
{
	// creation du repertoire Watchers
	strcpy(szWatchDir,Gens_Path);
	strcat(szWatchDir,"watchers\\");
	CreateDirectory( szWatchDir, NULL);
	
	// fichier log par defaut
	strcpy(szKModLog,Gens_Path);
	strcat(szKModLog,"gens_KMod.log");

	LoadConfig_KMod( );


	hM68K = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUG68K), HWnd, M68KDlgProc);
	hZ80 = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGZ80), HWnd, Z80DlgProc);
	hVDP = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGVDP), HWnd, VDPDlgProc);
	hCD_68K = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGCD_68K), HWnd, CD_68KDlgProc);
	hCD_CDC = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGCD_CDC), HWnd, CD_CDCDlgProc);
	hCD_GFX = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGCD_GFX), HWnd, CD_GFXDlgProc);
	hMSH2 = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUG32X_MSH2), HWnd, MSH2DlgProc);
	hSSH2 = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUG32X_SSH2), HWnd, SSH2DlgProc);
	h32X_VDP = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUG32X_VDP), HWnd, _32X_VDPDlgProc);
	hMisc = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGVDP_REG), HWnd, VDPRegDlgProc);
	hSprites = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGSPRITES), HWnd, SpritesDlgProc);
	hYM2612 = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGYM2612 ), HWnd, YM2612DlgProc);
	hPSG = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGPSG ), HWnd, PSGDlgProc);
	hWatchers = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_WATCHERS ), HWnd, WatcherDlgProc);
	hLayers = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_LAYERS), HWnd, LayersDlgProc);
	hDMsg = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGMSG), HWnd, MsgDlgProc);
	hCD_Reg = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGCD_REG), HWnd, CD_RegDlgProc);
	h32X_Reg = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUG32X_REG), HWnd, _32X_RegDlgProc);

	HandleWindow_KMod[0] = hM68K;
	HandleWindow_KMod[1] = hZ80;
	HandleWindow_KMod[2] = hVDP;
	HandleWindow_KMod[3] = hCD_68K;
	HandleWindow_KMod[4] = hCD_CDC;
	HandleWindow_KMod[5] = hCD_GFX;
	HandleWindow_KMod[6] = hMSH2;
	HandleWindow_KMod[7] = hSSH2;
	HandleWindow_KMod[8] = h32X_VDP;
	HandleWindow_KMod[9] = hMisc;
	HandleWindow_KMod[10] = hSprites;
	HandleWindow_KMod[11] = hYM2612;
	HandleWindow_KMod[12] = hPSG;
	HandleWindow_KMod[13] = hWatchers;
	HandleWindow_KMod[14] = hLayers;
	HandleWindow_KMod[15] = hDMsg;
	HandleWindow_KMod[16] = hCD_Reg;
	HandleWindow_KMod[17] = h32X_Reg;

	ResetDebug_KMod( );

}

void Update_KMod( )
{
	UpdateM68k_KMod( );
	UpdateZ80_KMod( );
	UpdateVDP_KMod( );
	UpdateVDPReg_KMod( );
	UpdateSprites_KMod( );
	UpdateYM2612_KMod( );
	UpdatePSG_KMod( );
	UpdateWatchers_KMod( );
//	UpdateLayers_KMod( ); no update needed
	UpdateSpy_KMod( );

	GMVUpdateRecord_KMod( );

	if (AutoShot_KMod)
		Take_Shot( );


	if (AutoPause_KMod)
	{
		Paused = 1;
		AutoPause_KMod = 0;
	}
	
	if (SegaCD_Started)
	{
		UpdateCD_68K_KMod( );
		UpdateCD_CDC_KMod( );
		UpdateCD_GFX_KMod( );
		UpdateCD_Reg_KMod( );		
	}

	if (_32X_Started)
	{
		UpdateMSH2_KMod( );
		UpdateSSH2_KMod( );
		Update32X_VDP_KMod( );
		Update32X_Reg_KMod( );
	}
}

void ResetDebug_KMod(  )
{
	UCHAR mode;
	UCHAR i,j;



	for (mode = 0; mode < WIN_NUMBER; mode++)
	{
		if ( OpenedWindow_KMod[ mode ] && mode != (DMODE_MSG-1) )
		{
			CloseWindow_KMod( (UCHAR) (mode+1) );
		}
	}

	
	//reset ym2612 channel
	for(i=0; i<5; i++)
	{
		EnabledChannels[i] = TRUE;
	}
	
	//start_tiles = 0;
	ListView_DeleteAllItems(GetDlgItem(hWatchers, IDC_WATCHER_LIST));
	
	LayersInit_KMod( );
	VDPInit_KMod( hVDP );
	CD_GFXInit_KMod( hCD_GFX );
	MsgInit_KMod( hDMsg );
	GMVStop_KMod( );

	AutoPause_KMod = 0;
	AutoShot_KMod = 0;

	for(i=0;i<6; i++)
	{
		for(j=0; j<4; j++)
		{
			notes[i][j] = BST_UNCHECKED;
		}
	}

		
	Put_Info("Debug reset", 1500);
}


void ToggleWindow_KMod( HWND hWnd, int Debug_Mode)
{
	if (Debug_Mode)
	{
		if ( OpenedWindow_KMod[ Debug_Mode-1 ] )
		{
			CloseWindow_KMod( (UCHAR) Debug_Mode );
		}
		else
		{
			if ((!Game) && (Debug_Mode != DMODE_MSG))	return; //only message windows can be opened without game

			OpenWindow_KMod( (UCHAR) Debug_Mode );
		}
	}
}

void ToggleAutoShot_KMod( )
{
	if (!Game)	return;

	if( AutoShot_KMod )
	{
		AutoShot_KMod = 0;
	}
	else
	{
		AutoShot_KMod = 1;
		if (Show_Message)
			MessageBox(HWnd, "Show Message active\nYou'll have unwanted text on screenshot","Warning", MB_ICONWARNING);


	}
}

BOOL IsDialogMsg_KMod( LPMSG lpMsg )
{
	UCHAR mode;

	for (mode = 0; mode < WIN_NUMBER; mode++)
	{
		if ( IsDialogMessage( HandleWindow_KMod[mode], lpMsg) )
		{
			TranslateAccelerator(HandleWindow_KMod[mode], hAccelTable, lpMsg);
			return TRUE;
		}
	}

	return FALSE;
}

void Config_KMod( )
{
	//if (Paused)
	DialogBox(ghInstance, MAKEINTRESOURCE(IDD_DCONFIG), HWnd, ConfigKDlgProc);
}

void AutoWatcher_KMod( )
{
	if (KConf.bAutoWatch)
		AutoLoadWatchers_KMod( );

}

void FrameStep_KMod( )
{
	Paused = 0;
	AutoPause_KMod = 1;

}