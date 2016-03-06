#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "gens.h"
#include "resource.h"
#include "G_main.h"
#include "keycode.h"
#include "G_gfx.h" // for Put_Info
#include "Misc.h" // for byte swap

//#include "M68KD.h"
#include "Cpu_68k.h"
#include "mem_M68K.h"
//#include "mem_S68K.h"

#include "z80.h" //for M_Z80 & z80_Read_Odo

#include "vdp_io.h"

//#include "ym2612.h"

//#include "psg.h"


//#include "Mem_SH2.h"
//#include "SH2.h"
//#include "SH2D.h"

#include "io.h"

#include "kmod\common.h"
#include "kmod\utils.h"
#include "kmod\message.h"
#include "kmod\watchers.h"
#include "kmod\layers.h"
#include "kmod\planes.h"
#include "kmod\gmv.h"
#include "kmod\m68k.h"
#include "kmod\z80.h"
#include "kmod\vdp.h"
#include "kmod\vdp_reg.h"
#include "kmod\sprites.h"
#include "kmod\ym2612.h"
#include "kmod\psg.h"

#include "kmod\s68k.h"
#include "kmod\cdc.h"
#include "kmod\cdgfx.h"
#include "kmod\cd_reg.h"

#include "kmod\mSH2.h"
#include "kmod\sSH2.h"
#include "kmod\vdp_32x.h"
#include "kmod\s32x_reg.h"

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
 ****** 0.7.2 - Graz release
 ** - GDB
 ** - remove 16bit flick
 ** - drag & drop support
 ** - always on top
 ** - optimize tile, sprite and register window
 ** - plane explorer
 ** - bug : z80 issue
 ****** 0.7.3
 ** - bug : crash on screenshot
 ** - bug : unreadable plane info
 ** - bug : crash on joypad key mapping
 ** - bug : Gens hangs when Message reach limit
 ** - bug : YM2612 wrong AM value (AlyJ)
 ** - bug : YM2612 wrong FMS value (AlyJ)
 ** - bug : PSG wrong noise type (AlyJ)
 ** - bug : YM2612 Chan6 enable status is wrong
 ** - bug : VDP tile ID wrong
 ** - bug : fake pal no longer works
 ** - bug : savestate pal wrong (Dr MefistO)
 ** - rewrite message logs
 ** - better 68k debug view, with current address and not relative
 ** - faster plane explorer (Dr MefistO)
 ** - VS2013 compile
 ** - WinXP support
 ** - rethink Sprites list
 ** - toggle GDB
 ** - single instance mode
 ** - pause at start
 ** - fast reload with ctrl+alt+l
 ****** 0.7.4
 ** - bug : fast reload need to be called twice on SRAM based game
 ** - bug : clear log doesn't really clear it on memory
 ** - keyboard now working on VDP Sprite
 *********************************************/

/*********************************************
 **
 ** Mod to do
 **
 ** - bug : Timer not working
 ** - jump to rom/ram address on Genesis - 68k view
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

#define TIMER_CYCLES		66480	/* cycles used by timer call */



struct ConfigKMod_struct KConf;

CHAR debug_string[1024];

int AutoPause_KMod;
int AutoShot_KMod;

//TODO could be handle by typedef struct channel__ .enabled ?
BOOL	EnabledChannels[6];


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
			message_addChar(b);
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
					m68kdebug_dump( );
					break;

				case 0x51: // dump z80 ram
					z80debug_dump( );
					break;

				case 0x52: // dump s68k program ram
					s68kdebug_dump();
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



/************ PSG **************/




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
	wsprintf(debug_string, "%d", KConf.pausedAtStart);
	WritePrivateProfileString("Debug", "pausedAtStart", debug_string, Conf_File);
	wsprintf(debug_string, "%d", KConf.singleInstance);
	WritePrivateProfileString("Debug", "singleInstance", debug_string, Conf_File);

	wsprintf(debug_string, "%d", KConf.Special);
	WritePrivateProfileString("Debug", "Special", debug_string, Conf_File);


	wsprintf(debug_string, "%d", KConf.useGDB);
	WritePrivateProfileString("Debug", "gdb", debug_string, Conf_File);

    wsprintf(debug_string, "%d", KConf.gdb_m68kport);
    WritePrivateProfileString("Debug", "gdbm68k", debug_string, Conf_File);

    wsprintf(debug_string, "%d", KConf.gdb_s68kport);
    WritePrivateProfileString("Debug", "gdbs68k", debug_string, Conf_File);

    wsprintf(debug_string, "%d", KConf.gdb_msh2port);
    WritePrivateProfileString("Debug", "gdbmsh2", debug_string, Conf_File);

    wsprintf(debug_string, "%d", KConf.gdb_ssh2port);
    WritePrivateProfileString("Debug", "gdbssh2", debug_string, Conf_File);
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
	KConf.bBluePause = (BOOL)GetPrivateProfileInt("Debug", "BluePause", TRUE, Conf_File);
	KConf.pausedAtStart = (BOOL)GetPrivateProfileInt("Debug", "pausedAtStart", FALSE, Conf_File);
	KConf.singleInstance = (BOOL)GetPrivateProfileInt("Debug", "singleInstance", FALSE, Conf_File);
	KConf.Special = (BOOL)GetPrivateProfileInt("Debug", "Special", FALSE, Conf_File);
	KConf.useGDB = (BOOL)GetPrivateProfileInt("Debug", "gdb", FALSE, Conf_File);
    KConf.gdb_m68kport = GetPrivateProfileInt("Debug", "gdbm68k", 6868, Conf_File);
    KConf.gdb_s68kport = GetPrivateProfileInt("Debug", "gdbs68k", 6869, Conf_File);
    KConf.gdb_msh2port = GetPrivateProfileInt("Debug", "gdbmsh2", 6870, Conf_File);
    KConf.gdb_ssh2port = GetPrivateProfileInt("Debug", "gdbssh2", 6871, Conf_File);
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

void RefreshGDBControls(HWND hwnd)
{
	BOOL bEnable = FALSE;

	bEnable = IsDlgButtonChecked(hwnd, IDC_DCONFIG_GDB);

	EnableWindow(GetDlgItem(hwnd, IDC_DCONFIG_GDBPORTM68K), bEnable);
	EnableWindow(GetDlgItem(hwnd, IDC_DCONFIG_GDBPORTS68K), bEnable);
	EnableWindow(GetDlgItem(hwnd, IDC_DCONFIG_GDBPORTMSH2), bEnable);
	EnableWindow(GetDlgItem(hwnd, IDC_DCONFIG_GDBPORTSSH2), bEnable);

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

static unsigned int GetUIntFromEdit(HWND hwnd, int item)
{
    char buffer[64];

    GetDlgItemText(hwnd, item, buffer, sizeof(buffer) - 1);

    return atoi(buffer);
}

static void SetUIntToEdit(HWND hwnd, int item, unsigned int value)
{
    char buffer[16];

    wsprintf(buffer, "%u", value);

    SetDlgItemText(hwnd, item, buffer);
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

			if (KConf.pausedAtStart)
				CheckDlgButton(hwnd, IDC_DCONFIG_PAUSE_START, BST_CHECKED);

			if (KConf.singleInstance)
				CheckDlgButton(hwnd, IDC_DCONFIG_SINGLE, BST_CHECKED);

			if ( KConf.Special )
				CheckDlgButton(hwnd, IDC_DCONFIG_SPECIAL, BST_CHECKED);

			if (KConf.useGDB)
				CheckDlgButton(hwnd, IDC_DCONFIG_GDB, BST_CHECKED);

            SetUIntToEdit(hwnd, IDC_DCONFIG_GDBPORTM68K, KConf.gdb_m68kport);
            SetUIntToEdit(hwnd, IDC_DCONFIG_GDBPORTS68K, KConf.gdb_s68kport);
            SetUIntToEdit(hwnd, IDC_DCONFIG_GDBPORTMSH2, KConf.gdb_msh2port);
            SetUIntToEdit(hwnd, IDC_DCONFIG_GDBPORTSSH2, KConf.gdb_ssh2port);

			RefreshSpyButtons( hwnd );
			RefreshGDBControls( hwnd );

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
				case IDC_DCONFIG_GDB:
					RefreshGDBControls(hwnd);
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
					KConf.bBluePause = IsDlgButtonChecked(hwnd, IDC_DCONFIG_BSCREEN);
					KConf.pausedAtStart = IsDlgButtonChecked(hwnd, IDC_DCONFIG_PAUSE_START);
					KConf.singleInstance = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SINGLE);
					KConf.Special  = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPECIAL);

					KConf.useGDB = IsDlgButtonChecked(hwnd, IDC_DCONFIG_GDB);
                    KConf.gdb_m68kport = GetUIntFromEdit(hwnd, IDC_DCONFIG_GDBPORTM68K);
                    KConf.gdb_s68kport = GetUIntFromEdit(hwnd, IDC_DCONFIG_GDBPORTS68K);
                    KConf.gdb_msh2port = GetUIntFromEdit(hwnd, IDC_DCONFIG_GDBPORTMSH2);
                    KConf.gdb_ssh2port = GetUIntFromEdit(hwnd, IDC_DCONFIG_GDBPORTSSH2);
                    SaveConfig_KMod();
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
	LoadConfig_KMod( );

	m68kdebug_create(ghInstance, HWnd);
	z80debug_create(ghInstance, HWnd);
	vdpdebug_create(ghInstance, HWnd);
	vdpreg_create(ghInstance, HWnd);
	sprites_create(ghInstance, HWnd);
	ym2612_create(ghInstance, HWnd);
	psg_create(ghInstance, HWnd);

	s68kdebug_create(ghInstance, HWnd);
	cdcdebug_create(ghInstance, HWnd);
	cdgfx_create(ghInstance, HWnd);
	cdreg_create(ghInstance, HWnd);

	mSH2_create(ghInstance, HWnd);
	sSH2_create(ghInstance, HWnd);
	vdp32x_create(ghInstance, HWnd);
	s32xreg_create(ghInstance, HWnd);

	layers_create(ghInstance, HWnd);
	watchers_create(ghInstance, HWnd);
	message_create(ghInstance, HWnd);
	planes_create(ghInstance, HWnd);

   
	//HandleWindow_KMod[0] = hM68K;
	//HandleWindow_KMod[1] = hZ80;
	//HandleWindow_KMod[2] = hVDP;
	//HandleWindow_KMod[3] = hCD_68K;
	//HandleWindow_KMod[4] = hCD_CDC;
	//HandleWindow_KMod[5] = hCD_GFX;
	//HandleWindow_KMod[6] = hMSH2;
	//HandleWindow_KMod[7] = hSSH2;
	//HandleWindow_KMod[8] = h32X_VDP;
	//HandleWindow_KMod[9] = hMisc;
	//HandleWindow_KMod[10] = hSprites;
	//HandleWindow_KMod[11] = hYM2612;
	//HandleWindow_KMod[12] = hPSG;
	//HandleWindow_KMod[13] = hWatchers;
	//HandleWindow_KMod[14] = hLayers;
	//HandleWindow_KMod[15] = hDMsg;
	//HandleWindow_KMod[16] = hCD_Reg;
	//HandleWindow_KMod[17] = h32X_Reg;
    //HandleWindow_KMod[18] = hPlaneExplorer;
}

void Update_KMod( )
{
	watchers_update();
	message_update();
//	layers_update(); //no update needed
	planes_update();


	m68kdebug_update();
	z80debug_update();
	vdpdebug_update(); 
	vdpreg_update();
	sprites_update();
	ym2612_update();
	psg_update();


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
		s68kdebug_update();
		cdcdebug_update();
		cdgfx_update();
		cdreg_update();
	}

	if (_32X_Started)
	{
		mSH2_update();
		sSH2_update();
		vdp32x_update();
		s32xreg_update();
	}
}

void kmod_close()
{
	UCHAR mode;
	UCHAR i;

	for (mode = 0; mode < WIN_NUMBER; mode++)
	{
		if (OpenedWindow_KMod[mode] && mode != (DMODE_MSG - 1))
		{
			CloseWindow_KMod((UCHAR)(mode + 1));
		}
	}


	//reset ym2612 channel
	for (i = 0; i<6; i++)
	{
		EnabledChannels[i] = TRUE;
	}

	//start_tiles = 0;

	watchers_reset();
	message_reset();


	GMVStop_KMod();

	AutoPause_KMod = 0;
	AutoShot_KMod = 0;
}

//TODO rename kmod_resetOnLoad()
void ResetDebug_KMod(  )
{
	message_reset();
	watchers_reset();
	layers_reset();
	planes_reset();

	m68kdebug_reset();
	z80debug_reset();
	vdpdebug_reset();
	vdpreg_reset();
	sprites_reset();
	ym2612_reset();
	psg_reset();

	s68kdebug_reset();
	cdcdebug_reset();
	cdgfx_reset();
	cdreg_reset();

	mSH2_reset();
	sSH2_reset();
	vdp32x_reset();
	s32xreg_reset();

	if (KConf.pausedAtStart)
	{
		Paused = 1;
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

	if (vdpdebug_isMessage(lpMsg))	return TRUE;

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


void FrameStep_KMod( )
{
	Paused = 0;
	AutoPause_KMod = 1;

}