#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../G_main.h"
#include "../resource.h"
#include "config.h"

struct ConfigKMod_struct KConf;
static CHAR debug_string[1024];

void SaveConfig_KMod()
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

void RefreshSpyButtons(HWND hwnd)
{
	long i;
	BOOL bEnable = FALSE;

	bEnable = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY);
	for (i = IDC_DCONFIG_SPY1; i <= IDC_DCONFIG_SPY4; i++)
		EnableWindow(GetDlgItem(hwnd, i), bEnable);

	bEnable = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPYCD);
	for (i = IDC_DCONFIG_SPYCD1; i <= IDC_DCONFIG_SPYCD2; i++)
		EnableWindow(GetDlgItem(hwnd, i), bEnable);
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

void Get_DebugZip_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	HRSRC hr;
	HGLOBAL hg;
	LPVOID lpData;

	hr = FindResource(ghInstance, MAKEINTRESOURCE(IDR_DEBUGZIP), RT_RCDATA);
	if (hr == NULL)	return;

	dwBytesToWrite = SizeofResource(ghInstance, hr);
	if (dwBytesToWrite == 0)	return;

	hg = LoadResource(ghInstance, hr);
	if (hg == NULL)	return;

	lpData = LockResource(hg);
	if (lpData == NULL)	return;


	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, "debug_S");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "Zip file dump (*.zip)\0*.zip\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Get Debug_S.zip";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "zip";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;


	WriteFile(hFr, lpData, dwBytesToWrite, &dwBytesWritten, NULL);

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
	switch (Message)
	{
	case WM_INITDIALOG:
		if (KConf.bAutoWatch)
			CheckDlgButton(hwnd, IDC_DCONFIG_AUTOW, BST_CHECKED);

		if (KConf.Spy)
			CheckDlgButton(hwnd, IDC_DCONFIG_SPY, BST_CHECKED);

		if (KConf.SpyReg)
			CheckDlgButton(hwnd, IDC_DCONFIG_SPY1, BST_CHECKED);

		if (KConf.SpySpr)
			CheckDlgButton(hwnd, IDC_DCONFIG_SPY2, BST_CHECKED);

		if (KConf.SpyRW)
			CheckDlgButton(hwnd, IDC_DCONFIG_SPY3, BST_CHECKED);

		if (KConf.SpyDMA)
			CheckDlgButton(hwnd, IDC_DCONFIG_SPY4, BST_CHECKED);

		if (KConf.CDBios)
			CheckDlgButton(hwnd, IDC_DCONFIG_SPYCD, BST_CHECKED);

		if (KConf.noCDBSTAT)
			CheckDlgButton(hwnd, IDC_DCONFIG_SPYCD1, BST_CHECKED);

		if (KConf.noCDCSTAT)
			CheckDlgButton(hwnd, IDC_DCONFIG_SPYCD2, BST_CHECKED);

		if (KConf.bBluePause)
			CheckDlgButton(hwnd, IDC_DCONFIG_BSCREEN, BST_CHECKED);

		if (KConf.pausedAtStart)
			CheckDlgButton(hwnd, IDC_DCONFIG_PAUSE_START, BST_CHECKED);

		if (KConf.singleInstance)
			CheckDlgButton(hwnd, IDC_DCONFIG_SINGLE, BST_CHECKED);

		if (KConf.Special)
			CheckDlgButton(hwnd, IDC_DCONFIG_SPECIAL, BST_CHECKED);

		if (KConf.useGDB)
			CheckDlgButton(hwnd, IDC_DCONFIG_GDB, BST_CHECKED);

		SetUIntToEdit(hwnd, IDC_DCONFIG_GDBPORTM68K, KConf.gdb_m68kport);
		SetUIntToEdit(hwnd, IDC_DCONFIG_GDBPORTS68K, KConf.gdb_s68kport);
		SetUIntToEdit(hwnd, IDC_DCONFIG_GDBPORTMSH2, KConf.gdb_msh2port);
		SetUIntToEdit(hwnd, IDC_DCONFIG_GDBPORTSSH2, KConf.gdb_ssh2port);

		RefreshSpyButtons(hwnd);
		RefreshGDBControls(hwnd);

		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_DCONFIG_AUTOW:
			break;
		case IDC_DCONFIG_SPY:
		case IDC_DCONFIG_SPYCD:
			RefreshSpyButtons(hwnd);
			break;
		case IDC_DCONFIG_GDB:
			RefreshGDBControls(hwnd);
			break;
		case IDC_DCONFIG_ZIP:
			Get_DebugZip_KMod(hwnd);
			break;
		case IDOK:
			KConf.bAutoWatch = IsDlgButtonChecked(hwnd, IDC_DCONFIG_AUTOW);
			KConf.Spy = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY);
			KConf.SpyReg = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY1);
			KConf.SpySpr = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY2);
			KConf.SpyRW = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY3);
			KConf.SpyDMA = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPY4);
			KConf.CDBios = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPYCD);
			KConf.noCDBSTAT = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPYCD1);
			KConf.noCDCSTAT = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPYCD2);
			KConf.bBluePause = IsDlgButtonChecked(hwnd, IDC_DCONFIG_BSCREEN);
			KConf.pausedAtStart = IsDlgButtonChecked(hwnd, IDC_DCONFIG_PAUSE_START);
			KConf.singleInstance = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SINGLE);
			KConf.Special = IsDlgButtonChecked(hwnd, IDC_DCONFIG_SPECIAL);

			KConf.useGDB = IsDlgButtonChecked(hwnd, IDC_DCONFIG_GDB);
			KConf.gdb_m68kport = GetUIntFromEdit(hwnd, IDC_DCONFIG_GDBPORTM68K);
			KConf.gdb_s68kport = GetUIntFromEdit(hwnd, IDC_DCONFIG_GDBPORTS68K);
			KConf.gdb_msh2port = GetUIntFromEdit(hwnd, IDC_DCONFIG_GDBPORTMSH2);
			KConf.gdb_ssh2port = GetUIntFromEdit(hwnd, IDC_DCONFIG_GDBPORTSSH2);
			SaveConfig_KMod();
			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			LoadConfig_KMod();
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}




void Config_KMod()
{
	//if (Paused)
	DialogBox(ghInstance, MAKEINTRESOURCE(IDD_DCONFIG), HWnd, ConfigKDlgProc);
}


void LoadConfig_KMod()
{
	unsigned char Conf_File[MAX_PATH];

	SetCurrentDirectory(Gens_Path);
	strcpy(Conf_File, Gens_Path);
	strcat(Conf_File, "GensKMod.cfg");

	KConf.bAutoWatch = (BOOL)GetPrivateProfileInt("Debug", "AutoLoadW", FALSE, Conf_File);
	KConf.Spy = (BOOL)GetPrivateProfileInt("Debug", "Spy", FALSE, Conf_File);
	KConf.SpyReg = (BOOL)GetPrivateProfileInt("Debug", "Spy1", FALSE, Conf_File);
	KConf.SpySpr = (BOOL)GetPrivateProfileInt("Debug", "Spy2", FALSE, Conf_File);
	KConf.SpyRW = (BOOL)GetPrivateProfileInt("Debug", "Spy3", FALSE, Conf_File);
	KConf.SpyDMA = (BOOL)GetPrivateProfileInt("Debug", "Spy4", FALSE, Conf_File);
	KConf.CDBios = (BOOL)GetPrivateProfileInt("Debug", "SpyCD", FALSE, Conf_File);
	KConf.noCDBSTAT = (BOOL)GetPrivateProfileInt("Debug", "SpyCD1", FALSE, Conf_File);
	KConf.noCDCSTAT = (BOOL)GetPrivateProfileInt("Debug", "SpyCD2", FALSE, Conf_File);
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
