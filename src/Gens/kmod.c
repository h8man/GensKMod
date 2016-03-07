#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "gens.h"
#include "resource.h"
#include "G_main.h"
#include "G_gfx.h" // for Put_Info
#include "vdp_io.h" // to know which system is active
#include "Star_68k.h"


#include "kmod\common.h"
#include "kmod\utils.h"
#include "kmod\config.h"
#include "kmod\message.h"
#include "kmod\watchers.h"
#include "kmod\gmv.h"

#include "kmod\layers.h"
#include "kmod\planes.h"
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

#define TIMER_CYCLES		66480	/* cycles used by timer call */

CHAR debug_string[1024];


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

	for (mode = 0; mode < WIN_NUMBER; mode++)
	{
		if (OpenedWindow_KMod[mode] && mode != (DMODE_MSG - 1))
		{
			CloseWindow_KMod((UCHAR)(mode + 1));
		}
	}

	//start_tiles = 0;

	watchers_reset();
	message_reset(); 
	ym2612_reset();


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



void FrameStep_KMod( )
{
	Paused = 0;
	AutoPause_KMod = 1;
}