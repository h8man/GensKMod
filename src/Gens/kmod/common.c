#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "..\gens.h"

#include "common.h"
#include "message.h"
#include "watchers.h"
#include "layers.h"
#include "planes.h"

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
	default:
		ShowWindow(HandleWindow_KMod[mode - 1], SW_SHOW);
	}

	Update_KMod();
}