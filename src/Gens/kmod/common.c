#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "..\gens.h"

#include "common.h"
#include "message.h"
#include "lua_script.h"
#include "watchers.h"
#include "layers.h"
#include "planes.h"
#include "vdp_composite.h"

#include "m68k.h"
#include "z80.h"
#include "vdp.h"
#include "vdp_reg.h"
#include "sprites.h"
#include "ym2612.h"
#include "psg.h"

#include "s68k.h"
#include "cdc.h"
#include "cdgfx.h"
#include "cd_reg.h"

#include "mSH2.h"
#include "sSH2.h"
#include "vdp_32x.h"
#include "s32x_reg.h"


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

static void SetKModWindowVisibility(UCHAR mode, BOOL visible)
{
	switch (mode)
	{
	case DMODE_MSG:
		message_show(visible);
		break;
	case DMODE_WATCHERS:
		watchers_show(visible);
		break;
	case DMODE_LAYERS:
		layers_show(visible);
		break;
	case DMODE_PLANEEXPLORER:
		planes_show(visible);
		break; 
	case DMODE_PLANECOMPOSITE:
		planes_composite_show(visible);
		break; 
	case DMODE_LUASCRIPT:
		lua_script_show(visible);
		break;
	case DMODE_68K:
		m68kdebug_show(visible);
		break;
	case DMODE_Z80:
		z80debug_show(visible);
		break;
	case DMODE_VDP:
		vdpdebug_show(visible);
		break;
	case DMODE_VDP_REG:
		vdpreg_show(visible);
		break;
	case DMODE_SPRITES:
		sprites_show(visible);
		break;
	case DMODE_YM2612:
		ym2612_show(visible);
		break;
	case DMODE_PSG:
		psg_show(visible);
		break;
	case DMODE_CD_68K:
		s68kdebug_show(visible);
		break;
	case DMODE_CD_CDC:
		cdcdebug_show(visible);
		break;
	case DMODE_CD_GFX:
		cdgfx_show(visible);
		break;
	case DMODE_CD_REG:
		cdreg_show(visible);
		break;
	case DMODE_32_REG:
		s32xreg_show(visible);
		break;
	case DMODE_32_MSH2:
		mSH2_show(visible);
		break;
	case DMODE_32_SSH2:
		sSH2_show(visible);
		break;
	case DMODE_32_VDP:
		vdp32x_show(visible);
		break;
	default:
		ShowWindow(HandleWindow_KMod[mode - 1], visible ? SW_SHOW : SW_HIDE);
	}
}

BOOL SetKModWindowVisible(UCHAR mode, BOOL visible)
{
	if (mode < 1 || mode > WIN_NUMBER)
		return FALSE;
	if (visible && !Game && mode != DMODE_MSG && mode != DMODE_LUASCRIPT)
		return FALSE;

	OpenedWindow_KMod[mode - 1] = visible != FALSE;
	SetKModWindowVisibility(mode, visible);
	return TRUE;
}

void CloseWindow_KMod(UCHAR mode)
{
	SetKModWindowVisible(mode, FALSE);
}

void OpenWindow_KMod(UCHAR mode)
{
	if (mode < 1 || mode > WIN_NUMBER)
		return;
	if (!SetKModWindowVisible(mode, TRUE))
		return;
	Update_KMod();
}