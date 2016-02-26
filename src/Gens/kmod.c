#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "gens.h"
#include "resource.h"
#include "G_main.h"
#include "keycode.h"
#include "G_gfx.h" // for Put_Info
#include "Misc.h" // for byte swap

#include "M68KD.h"
#include "Cpu_68k.h"
#include "mem_M68K.h"
#include "mem_S68K.h"

#include "z80.h" //for M_Z80 & z80_Read_Odo

#include "vdp_io.h"

#include "ym2612.h"

#include "psg.h"

#include "LC89510.h"

#include "Mem_SH2.h"
#include "SH2.h"
#include "SH2D.h"
#include "vdp_32X.h"

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





struct _32X_register_struct
{
	BYTE side; //bit 0:MD, bit 1:32X
	WORD adr;
	char *descriptionMD;
	char *description32X;
};



struct _32X_register_struct _32X_register[] =
{
	{3, 0x00, "Adapter control register", "Interrupt Mask"},
	{3, 0x02, "Interrupt control register", "Stand By change"},
	{3, 0x04, "Bank set register", "H Count"},
	{3, 0x06, "DREQ control register", "DREQ control register"},
	{3, 0x08, "DREQ source MSB", "DREQ source MSB"},
	{3, 0x0A, "DREQ source LSB", "DREQ source LSB"},
	{3, 0x0C, "DREQ destination MSB", "DREQ destination MSB"},
	{3, 0x0E, "DREQ destination LSB", "DREQ destination LSB"},
	{3, 0x10, "DREQ length", "DREQ length"},
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
	{2, 0x104, "", "Auto fill length"},
	{2, 0x106, "", "Auto fill start"},
	{2, 0x108, "", "Auto fill data"},
	{2, 0x10A, "", "Frame Buffer control"},
	{0xff,-1,""}
};





HWND hMisc, hSprites, hYM2612, hPSG;
HWND hCD_68K, hCD_CDC, hCD_GFX, hCD_Reg;
HWND hMSH2, hSSH2, h32X_VDP, h32X_Reg;

struct ConfigKMod_struct KConf;

CHAR debug_string[1024];

int AutoPause_KMod;
int AutoShot_KMod;

//TODO could be handle by typedef struct channel__ .enabled ?
BOOL	EnabledChannels[6];

void Dump68K_KMod(HWND hwnd);
void DumpS68KPRam_KMod(HWND hwnd);
void DumpZ80_KMod(HWND hwnd);

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
	si.nPage  = 10;


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

#include "kmod/vdp.h"
//TODO what the exact use of pal_Kmod here ? why not CDpal_KMod ?

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
		tmpColor = vdpdebug_getColor(pal_KMod, j);
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
		SetPixelV(hDC, 0, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + j*4 + 1]&0x0F;
		SetPixelV(hDC, 1, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + j*4 ]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 2, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + j*4 ]&0x0F;
		SetPixelV(hDC, 3, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + j*4 + 3]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 4, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + j*4 + 3]&0x0F;
		SetPixelV(hDC, 5, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + j*4 + 2]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 6, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + j*4 + 2]&0x0F;
		SetPixelV(hDC, 7, j, vdpdebug_getColor(pal, TileData));
	}

	for(j = 0; j < 8; j++)
	{
		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 1]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 0, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 1]&0x0F;
		SetPixelV(hDC, 1, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 ]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 2, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 ]&0x0F;
		SetPixelV(hDC, 3, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 3]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 4, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 3]&0x0F;
		SetPixelV(hDC, 5, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 2]&0xF0;
		TileData >>=4;
		SetPixelV(hDC, 6, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile*64 + 32 + j*4 + 2]&0x0F;
		SetPixelV(hDC, 7, j + 8, vdpdebug_getColor(pal, TileData));
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
	si.nPage  = 10;

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
	si.nPage  = 10;

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
long palH, palV;

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
	LONG pix, h;
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

	lineTable = (WORD *)(_32X_VDP_Ram + ((unsigned int)toDraw * 0x20000));

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

	VRAM = (WORD *)(_32X_VDP_Ram + ((unsigned int)toDraw * 0x20000) + 256 * 2); //skip 256 words of line table


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
    static struct Reg_VDP_Type old_regs = { 0, };

    if (OpenedWindow_KMod[9] == FALSE)	return;

    if (!memcmp(&old_regs, &VDP_Reg, sizeof(old_regs)))
        return;

    memcpy(&old_regs, &VDP_Reg, sizeof(old_regs));

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

    debug_string[0] = 0;
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

    debug_string[0] = 0;
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

    debug_string[0] = 0;
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

	wsprintf(debug_string, "DMA Length 0x%0.2X%0.2X", valReg, VDP_Reg.DMA_Lenght_L);
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
HWND hSpriteList;

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


void SpritesInit_KMod(HWND hDlg)
{
	LV_COLUMN   lvColumn;
	int         i;
	TCHAR       szString[8][7] = { "Num", "Ypos", "XPos", "Size", "Link", "Pal", "Tile", "Flags*" };
	LVITEM		lvItem;
	char		buf[64];


	hSpriteList = GetDlgItem(hDlg, IDC_SPRITES_LIST);
	ListView_DeleteAllItems(hSpriteList);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 53;
	for (i = 0; i < 8; i++)
	{
		lvColumn.pszText = szString[i];
		ListView_InsertColumn(hSpriteList, i, &lvColumn);
	}

	ListView_SetExtendedListViewStyle(hSpriteList, LVS_EX_FULLROWSELECT |LVS_EX_GRIDLINES);

	for (i = 0; i < 80; i++)
	{
		wsprintf(buf, "%0.2d", i);

		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		lvItem.pszText = buf;
		ListView_InsertItem(hSpriteList, &lvItem);
	}

}

void UpdateSprites_KMod( )
{
	unsigned int i;
	unsigned short int *sprData, tmp;
	unsigned char tmp_string[32];
    static unsigned short data_copy[80 * 4];
	static unsigned int forceRefreshCounter;
	LVITEM		lvItem;


	if ( OpenedWindow_KMod[ 10 ] == FALSE )
        return;

    sprData = (unsigned short *)(VRam + (VDP_Reg.Spr_Att_Adr << 9));


    if (!memcmp(sprData, data_copy, sizeof(data_copy)))
    {
		//sometimes miss the last update... so force refresh every n frame
		if (--forceRefreshCounter > 0 )	return;
    }
	forceRefreshCounter = 0xFFFF;
    memcpy(data_copy, sprData, sizeof(data_copy));


	for(i = 0; i < 80; i++)
	{
		lvItem.iItem = i;
		lvItem.iSubItem = LVIF_TEXT;
		ListView_GetItem(hSpriteList, &lvItem);

		/*
		// not needed
		// index
		wsprintf(tmp_string, "%.2d", i);
		ListView_SetItemText(hSpriteList, i, 0, tmp_string);
		*/

		// yPos
		wsprintf(tmp_string, "%4d", sprData[0] & 0x03FF);
		ListView_SetItemText(hSpriteList, i, 1, tmp_string);

		// xPos
		wsprintf(tmp_string, "%4d", sprData[3] & 0x03FF);
		ListView_SetItemText(hSpriteList, i, 2, tmp_string);

		// size
		wsprintf(tmp_string, "%.2dx%.2d",
			TrueSize_KMod(((sprData[1] & 0x0C00) >> 10)),
			TrueSize_KMod(((sprData[1] & 0x0300) >> 8)));
		ListView_SetItemText(hSpriteList, i, 3, tmp_string);

		// link
		wsprintf(tmp_string, "%.2d", sprData[1] & 0x007F);
		ListView_SetItemText(hSpriteList, i, 4, tmp_string);

		// pal
		wsprintf(tmp_string, "%2d", (sprData[2] & 0x6000) >> 13);
		ListView_SetItemText(hSpriteList, i, 5, tmp_string);

		// tile
		wsprintf(tmp_string, "%4d", (sprData[2] & 0x07FF));
		ListView_SetItemText(hSpriteList, i, 6, tmp_string);

		// flags
		tmp = 0;
		/*
		if ( sprData[2]&0x8000)	tmp +=100;
		if ( sprData[2]&0x1000)	tmp +=010;
		if ( sprData[2]&0x0800)	tmp +=001;
		wsprintf(tmp_string, "%.3d", tmp);
		*/
		if (sprData[2] & 0x8000)	tmp |= 4;
		if (sprData[2] & 0x1000)	tmp |= 2;
		if (sprData[2] & 0x0800)	tmp |= 1;
		wsprintf(tmp_string, "%c%c%c", tmp & 4 ? 'P' : '0', tmp & 2 ? 'V' : '0', tmp & 1 ? 'H' : '0');
		ListView_SetItemText(hSpriteList, i, 7, tmp_string);

		sprData += 4;
	}

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

	selIdx = SendMessage(hSpriteList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED); // return item selected
	if (selIdx == -1)
		return;

	sprData = (unsigned short *)(VRam + ( VDP_Reg.Spr_Att_Adr << 9 ));
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
			vdpdebug_drawTile(hDC, numTile++, posX, posY, (UCHAR)pal, 1);
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

	selIdx = SendMessage(hSpriteList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED); // return item selected
	if (selIdx == -1)
		return;

	sprData = (unsigned short *)(VRam + ( VDP_Reg.Spr_Att_Adr << 9 ));
	sprData += selIdx*4; /* each sprite is 4 short int data */

	numTile = sprData[2]&0x07FF;
	sizeH = TrueSize_KMod( ((sprData[1]&0x0C00)>>10) );
	sizeV = TrueSize_KMod( ((sprData[1]&0x0300)>>8 ) );

	zoom = (unsigned char)((hlDIS->rcItem.right - hlDIS->rcItem.left) / 32);
	zoom = (unsigned char)min(zoom, (hlDIS->rcItem.bottom - hlDIS->rcItem.top) / 32);

	hDC = CreateCompatibleDC( hlDIS->hDC );
	hBitmap = CreateCompatibleBitmap( hlDIS->hDC,  32*zoom, 32*zoom);
	hOldBitmap = SelectObject(hDC, hBitmap);
	FillRect(hDC, &(hlDIS->rcItem), (HBRUSH) (COLOR_3DFACE+1) );

	pal = (sprData[2]&0x6000)>>13;
	for(posX = 0; posX < sizeH; posX+=8)
	{
		for(posY = 0; posY < sizeV; posY+=8)
		{
			vdpdebug_drawTile(hDC, numTile++, posX*zoom, posY*zoom, (UCHAR)pal, zoom);
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
	unsigned short int	numTile;
	int selIdx;
	unsigned char		TileData, sizeH, sizeV, posX, posY, j, pal, tmp;
	unsigned short int	*sprData;

	selIdx = SendMessage(hSpriteList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED); // return item selected
	if (selIdx == -1)
		return;


	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */
	wsprintf(szFileName, "%s_sprite_%.2d", Rom_Name, selIdx);

    ZeroMemory(&szFile, sizeof(szFile));
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


	sprData = (unsigned short *)(VRam + ( VDP_Reg.Spr_Att_Adr << 9 ));
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
		tmpColor = vdpdebug_getColor(pal, j);
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
	switch(Message)
    {
		case WM_INITDIALOG:
			SpritesInit_KMod( hwnd );

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
				case IDC_SPRITES_DUMP:
					DumpSprite_KMod( hSprites );
					break;
			}
			break;

		case WM_NOTIFY:
			switch (LOWORD(wParam))
			{
				case IDC_SPRITES_LIST:
					if ( (((LPNMHDR)lParam)->code == NM_CLICK) || (((LPNMHDR)lParam)->code == NM_SETFOCUS) )
					{
						RedrawWindow(GetDlgItem(hSprites, IDC_SPRITES_PREVIEW), NULL, NULL, RDW_INVALIDATE);
						RedrawWindow(GetDlgItem(hSprites, IDC_SPRITES_PREVIEW2), NULL, NULL, RDW_INVALIDATE);
					}
					break;
			}
			break;
		case WM_CLOSE:
			CloseWindow_KMod( DMODE_SPRITES );
			break;

		case WM_DESTROY:
			ListView_DeleteAllItems(hSpriteList);

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

//TODO rename
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
			lstrcat(debug_string, "ON\n");
		else
			lstrcat(debug_string, "OFF\n");
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

	wsprintf(debug_string,"0x%0.2X\n", (YM2612.REG[part][0xB4 + chan]&0x30)>>4); //AMS
	wsprintf(tmp_string,"0x%0.2X\n", YM2612.REG[part][0xB4 + chan]&0x07 ); //FMS
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
	wsprintf(debug_string,"%s", (PSG.Register[6]>>2)==1 ? "White":"Periodic");
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

	layers_create(ghInstance, HWnd);
	watchers_create(ghInstance, HWnd);
	message_create(ghInstance, HWnd);
	planes_create(ghInstance, HWnd);

	hCD_Reg = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUGCD_REG), HWnd, CD_RegDlgProc);
	h32X_Reg = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_DEBUG32X_REG), HWnd, _32X_RegDlgProc);
   
	//HandleWindow_KMod[0] = hM68K;
	//HandleWindow_KMod[1] = hZ80;
	//HandleWindow_KMod[2] = hVDP;
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
	//HandleWindow_KMod[13] = hWatchers;
	//HandleWindow_KMod[14] = hLayers;
	//HandleWindow_KMod[15] = hDMsg;
	HandleWindow_KMod[16] = hCD_Reg;
	HandleWindow_KMod[17] = h32X_Reg;
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

	UpdateVDPReg_KMod( );
	UpdateSprites_KMod( );
	UpdateYM2612_KMod( );
	UpdatePSG_KMod( );


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

void kmod_close()
{
	UCHAR mode;
	UCHAR i, j;

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

	for (i = 0; i<6; i++)
	{
		for (j = 0; j<4; j++)
		{
			notes[i][j] = BST_UNCHECKED;
		}
	}

}

//TODO rename kmod_resetOnLoad()
void ResetDebug_KMod(  )
{
	CD_GFXInit_KMod(hCD_GFX);

	message_reset();
	watchers_reset();
	layers_reset();
	planes_reset();

	m68kdebug_reset();
	z80debug_reset();

	vdpdebug_reset();

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