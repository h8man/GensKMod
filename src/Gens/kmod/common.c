#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "..\gens.h"

#include "common.h"
#include "message.h"
#include "watchers.h"
#include "layers.h"
#include "planes.h"

#include "m68k.h"
#include "z80.h"
#include "vdp.h"
#include "vdp_reg.h"

#include "s68k.h"
#include "cdc.h"
#include "cdgfx.h"
#include "cd_reg.h"

#include "mSH2.h"
#include "sSH2.h"
#include "vdp_32x.h"


ULONG	timer_KMod;

// timer related : TO REVAMP

void IncTimer_KMod(unsigned odom)
{
	if (timer_KMod)	timer_KMod += odom;
}

/* Dirty way to know which window is open, using original Gens Debug values */
UCHAR OpenedWindow_KMod[WIN_NUMBER]; /* 0: Window Closed, 1: Window Opened */
//TODO remove
HWND  HandleWindow_KMod[WIN_NUMBER];

void CloseWindow_KMod(UCHAR mode)
{
	if (mode < 1)	return;
	if (mode > WIN_NUMBER)	return;

	OpenedWindow_KMod[mode - 1] = 0;

	switch (mode)
	{
	case DMODE_MSG:
		message_show(FALSE);
		break;
	case DMODE_WATCHERS:
		watchers_show(FALSE);
		break;
	case DMODE_LAYERS:
		layers_show(FALSE);
		break;
	case DMODE_PLANEEXPLORER:
		planes_show(FALSE);
		break; 
	case DMODE_68K:
		m68kdebug_show(FALSE);
		break;
	case DMODE_Z80:
		z80debug_show(FALSE);
		break;
	case DMODE_VDP:
		vdpdebug_show(FALSE);
		break;
	case DMODE_VDP_REG:
		vdpreg_show(FALSE);
		break;
	case DMODE_CD_68K:
		s68kdebug_show(FALSE);
		break;
	case DMODE_CD_CDC:
		cdcdebug_show(FALSE);
		break;
	case DMODE_CD_GFX:
		cdgfx_show(FALSE);
		break;
	case DMODE_CD_REG:
		cdreg_show(FALSE);
		break;
	case DMODE_32_MSH2:
		mSH2_show(FALSE);
		break;
	case DMODE_32_SSH2:
		sSH2_show(FALSE);
		break;
	case DMODE_32_VDP:
		vdp32x_show(FALSE);
		break;
	default:
		ShowWindow(HandleWindow_KMod[mode - 1], SW_HIDE);
	}
}

void OpenWindow_KMod(UCHAR mode)
{
	if (mode < 1)	return;
	if (mode > WIN_NUMBER)	return;

	OpenedWindow_KMod[mode - 1] = 1;

	switch (mode)
	{
	case DMODE_MSG:
		message_show(TRUE);
		break;
	case DMODE_WATCHERS:
		watchers_show(TRUE);
		break;
	case DMODE_LAYERS:
		layers_show(TRUE);
		break;
	case DMODE_PLANEEXPLORER:
		planes_show(TRUE);
		break;
	case DMODE_68K:
		m68kdebug_show(TRUE);
		break;
	case DMODE_Z80:
		z80debug_show(TRUE);
		break;
	case DMODE_VDP:
		vdpdebug_show(TRUE);
		break;
	case DMODE_VDP_REG:
		vdpreg_show(TRUE);
		break;
	case DMODE_CD_68K:
		s68kdebug_show(TRUE);
		break;
	case DMODE_CD_CDC:
		cdcdebug_show(TRUE);
		break;
	case DMODE_CD_GFX:
		cdgfx_show(TRUE);
		break;
	case DMODE_CD_REG:
		cdreg_show(TRUE);
		break;
	case DMODE_32_MSH2:
		mSH2_show(TRUE);
		break;
	case DMODE_32_SSH2:
		sSH2_show(TRUE);
		break;
	case DMODE_32_VDP:
		vdp32x_show(TRUE);
		break;
	default:
		ShowWindow(HandleWindow_KMod[mode - 1], SW_SHOW);
	}

	Update_KMod();
}