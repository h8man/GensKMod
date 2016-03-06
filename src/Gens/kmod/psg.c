#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../G_main.h"
#include "../resource.h"
#include "../psg.h"

//#include "../G_gfx.h" //used for Put_Info

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "psg.h"

static HWND hPSG;

static HWND hVol1_PSG, hVol2_PSG, hVol3_PSG, hVol4_PSG;
static CHAR debug_string[1024];

void UpdatePSG_KMod()
{
	int VolPer;

	wsprintf(debug_string, "%d Hz", PSG.Register[0] == 0 ? 0 : (int)(3579545 / (PSG.Register[0] * 32)));
	SetDlgItemText(hPSG, IDC_PSG_FREQ_1, debug_string);
	wsprintf(debug_string, "%d", PSG.Register[0]);
	SetDlgItemText(hPSG, IDC_PSG_DATA_1, debug_string);
	wsprintf(debug_string, "%d Hz", PSG.Register[2] == 0 ? 0 : (int)(3579545 / (PSG.Register[2] * 32)));
	SetDlgItemText(hPSG, IDC_PSG_FREQ_2, debug_string);
	wsprintf(debug_string, "%d", PSG.Register[2]);
	SetDlgItemText(hPSG, IDC_PSG_DATA_2, debug_string);
	wsprintf(debug_string, "%d Hz", PSG.Register[4] == 0 ? 0 : (int)(3579545 / (PSG.Register[4] * 32)));
	SetDlgItemText(hPSG, IDC_PSG_FREQ_3, debug_string);
	wsprintf(debug_string, "%d", PSG.Register[4]);
	SetDlgItemText(hPSG, IDC_PSG_DATA_3, debug_string);
	wsprintf(debug_string, "%s", (PSG.Register[6] >> 2) == 1 ? "White" : "Periodic");
	SetDlgItemText(hPSG, IDC_PSG_FEEDBACK, debug_string);
	if ((PSG.Register[6] & 0x03) == 0)
	{
		wsprintf(debug_string, "%s", "Clock/2");
	}
	else if ((PSG.Register[6] & 0x03) == 0) {
		wsprintf(debug_string, "%s", "Clock/2");
	}
	else if ((PSG.Register[6] & 0x03) == 1) {
		wsprintf(debug_string, "%s", "Clock/4");
	}
	else if ((PSG.Register[6] & 0x03) == 2) {
		wsprintf(debug_string, "%s", "Clock/8");
	}
	else if ((PSG.Register[6] & 0x03) == 3) {
		wsprintf(debug_string, "%s", "Tone 3");
	}
	SetDlgItemText(hPSG, IDC_PSG_CLOCK, debug_string);

	VolPer = (100 * PSG.Volume[0]) / PSG_MaxVolume;
	wsprintf(debug_string, "%d%%", VolPer);
	SetDlgItemText(hPSG, IDC_PSG_VOLUME_1, debug_string);
	SendMessage(hVol1_PSG, PBM_SETPOS, (WPARAM)VolPer, (LPARAM)0);

	VolPer = (100 * PSG.Volume[1]) / PSG_MaxVolume;
	wsprintf(debug_string, "%d%%", VolPer);
	SetDlgItemText(hPSG, IDC_PSG_VOLUME_2, debug_string);
	SendMessage(hVol2_PSG, PBM_SETPOS, (WPARAM)VolPer, (LPARAM)0);

	VolPer = (100 * PSG.Volume[2]) / PSG_MaxVolume;
	wsprintf(debug_string, "%d%%", VolPer);
	SetDlgItemText(hPSG, IDC_PSG_VOLUME_3, debug_string);
	SendMessage(hVol3_PSG, PBM_SETPOS, (WPARAM)VolPer, (LPARAM)0);

	VolPer = (100 * PSG.Volume[3]) / PSG_MaxVolume;
	wsprintf(debug_string, "%d%%", VolPer);
	SetDlgItemText(hPSG, IDC_PSG_VOLUME_4, debug_string);
	SendMessage(hVol4_PSG, PBM_SETPOS, (WPARAM)VolPer, (LPARAM)0);
}



BOOL CALLBACK PSGDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch (Message)
	{
	case WM_INITDIALOG:
		hVol1_PSG = GetDlgItem(hwnd, IDC_PSG_SLIDER_1);
		hVol2_PSG = GetDlgItem(hwnd, IDC_PSG_SLIDER_2);
		hVol3_PSG = GetDlgItem(hwnd, IDC_PSG_SLIDER_3);
		hVol4_PSG = GetDlgItem(hwnd, IDC_PSG_SLIDER_4);

		psg_reset();
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_PSG);
		break;
	case WM_DESTROY:
		psg_destroy(hPSG);
		PostQuitMessage(0);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


void psg_create(HINSTANCE hInstance, HWND hWndParent)
{
	hPSG = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGPSG), hWndParent, PSGDlgProc);
}

void psg_show(BOOL visibility)
{
	ShowWindow(hPSG, visibility ? SW_SHOW : SW_HIDE);
}

void psg_update()
{
	if (OpenedWindow_KMod[DMODE_PSG - 1] == FALSE)	return;

	UpdatePSG_KMod();
}

void psg_reset()
{
	SendMessage(hVol1_PSG, PBM_SETPOS, (WPARAM)0, (LPARAM)0);
	SendMessage(hVol2_PSG, PBM_SETPOS, (WPARAM)0, (LPARAM)0);
	SendMessage(hVol3_PSG, PBM_SETPOS, (WPARAM)0, (LPARAM)0);
	SendMessage(hVol4_PSG, PBM_SETPOS, (WPARAM)0, (LPARAM)0);
}
void psg_destroy()
{
	DestroyWindow(hPSG);
}
