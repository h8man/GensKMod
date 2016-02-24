#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../G_gfx.h" // for Put_Info
#include "../G_main.h"
#include "../io.h"
#include "../resource.h"
#include "gmv.h"

HANDLE	hGMVFile_In;
HANDLE	hGMVFile_Out;

void GMVT_BrowseIn_Init(HWND hwndParent, char *szFileName)
{
	if (hGMVFile_In)	CloseHandle(hGMVFile_In);
	hGMVFile_In = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hGMVFile_In == INVALID_HANDLE_VALUE)  return;
	SetDlgItemText(hwndParent, IDC_GMVT_IN, szFileName);

	if (hGMVFile_Out)	CloseHandle(hGMVFile_Out);
	szFileName[strlen(szFileName) - 1] = 'c';
	hGMVFile_Out = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hGMVFile_Out == INVALID_HANDLE_VALUE)	return;
	SetDlgItemText(hwndParent, IDC_GMVT_OUT, szFileName);
}

void GMVT_BrowseIn(HWND hwndParent)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */


	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwndParent;
	szFile.lpstrFilter = "GMV File (*.gmv)\0*.gmv\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = Gens_Path;//(LPSTR)NULL;
	szFile.lpstrTitle = "Open GMV file";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "gmv";

	SetCurrentDirectory(Gens_Path);

	if (GetOpenFileName(&szFile) != TRUE)   return;

	GMVT_BrowseIn_Init(hwndParent, szFileName);
}

void GMVT_BrowseOut(HWND hwndParent)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwndParent;
	szFile.lpstrFilter = "GMV compressed file (*.gmc)\0*.gmc\0Binary\0*.*\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = Gens_Path;//(LPSTR)NULL;
	szFile.lpstrTitle = "Save GMV compressed";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "gmc";

	SetCurrentDirectory(Gens_Path);

	if (GetSaveFileName(&szFile) != TRUE)   return;

	if (hGMVFile_Out)	CloseHandle(hGMVFile_Out);
	hGMVFile_Out = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
	if (p1)		cnt |= 0x01;
	if (p2)		cnt |= 0x02;
	if (b6)		cnt |= 0x04;
	if (comp)	cnt |= 0x80;
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
				if ((oldstate[0] == state[0]) && (oldstate[1] == state[1]) && (oldstate[2] == state[2]) && (cnt < 0xFF))
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
	} while (dwBytesRead == 3);

	if (comp && cnt)
	{
		WriteFile(hGMVFile_Out, &cnt, 1, &dwBytesWritten, NULL);
		if (p1)	WriteFile(hGMVFile_Out, &oldstate[0], 1, &dwBytesWritten, NULL);
		if (p2)	WriteFile(hGMVFile_Out, &oldstate[1], 1, &dwBytesWritten, NULL);
		if (b6)	WriteFile(hGMVFile_Out, &oldstate[2], 1, &dwBytesWritten, NULL);
	}

	return TRUE;
}

BOOL CALLBACK GMVToolsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char szFileName[MAX_PATH];

	switch (Message)
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

			GMVT_BrowseIn_Init(hwnd, szFileName);

		}
		break;


	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_GMVT_BROWSE_IN:
			GMVT_BrowseIn(hwnd);
			break;

		case IDC_GMVT_BROWSE_OUT:
			GMVT_BrowseOut(hwnd);
			break;

		case IDOK:
			if (GMVTools_Convert(IsDlgButtonChecked(hwnd, IDC_GMVT_COMP), IsDlgButtonChecked(hwnd, IDC_GMVT_P1),
				IsDlgButtonChecked(hwnd, IDC_GMVT_P2), IsDlgButtonChecked(hwnd, IDC_GMVT_6BUTTONS)))
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



void GMVTools_KMod()
{
	DialogBox(ghInstance, MAKEINTRESOURCE(IDD_GMVTOOLS), HWnd, GMVToolsDlgProc);

	if (hGMVFile_In)	CloseHandle(hGMVFile_In);
	if (hGMVFile_Out)	CloseHandle(hGMVFile_Out);
}




/*********** GMV **************/
HANDLE	hGMVFile;
BOOL	bGMVRecord;

void GMVStop_KMod()
{
	if (hGMVFile == NULL)	return;

	CloseHandle(hGMVFile);
	hGMVFile = NULL;
	Put_Info("GMV Record/Play end", 2000);
}

void GMVPlay_KMod()
{
	char szFileName[MAX_PATH];

	if (hGMVFile)	GMVStop_KMod();

	// try to open it
	strcpy(szFileName, Gens_Path);
	strcat(szFileName, Rom_Name);
	strcat(szFileName, ".gmv");

	hGMVFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hGMVFile == INVALID_HANDLE_VALUE)
		return;

	/* skip header */
	SetFilePointer(hGMVFile, 0x40, 0, FILE_BEGIN);

	bGMVRecord = FALSE;
	Put_Info("GMV Play start", 2000);
}

void GMVRecord_KMod()
{
	char szFileName[MAX_PATH];
	DWORD dwBytesWritten;
	char szData[40];
	int tmp;

	strcpy(szFileName, Gens_Path);
	strcat(szFileName, Rom_Name);
	strcat(szFileName, ".gmv");


	if (hGMVFile)	GMVStop_KMod();

	hGMVFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hGMVFile == INVALID_HANDLE_VALUE)
		return;

	strcpy(szData, "Gens Movie KMod1");
	WriteFile(hGMVFile, szData, 16, &dwBytesWritten, NULL);

	tmp = 0;
	WriteFile(hGMVFile, &tmp, sizeof(tmp), &dwBytesWritten, NULL);
	WriteFile(hGMVFile, &tmp, sizeof(tmp), &dwBytesWritten, NULL);

	//ZeroMemory(szData, 40);
	strncpy(szData, Rom_Name, 40);
	WriteFile(hGMVFile, szData, 40, &dwBytesWritten, NULL);

	bGMVRecord = TRUE;
	Put_Info("GMV Record start", 2000);
}


void GMVUpdatePlay_KMod()
{
	DWORD	dwBytesRead;
	BYTE	state;

	if (hGMVFile == NULL)	return;

	if (bGMVRecord)	return; //recording

	ReadFile(hGMVFile, &state, 1, &dwBytesRead, NULL);
	if (dwBytesRead == 0)
	{
		//EOF
		GMVStop_KMod();
		return;
	}

	Controller_1_Up = state & 0x01;
	state >>= 1;
	Controller_1_Down = state & 0x01;
	state >>= 1;
	Controller_1_Left = state & 0x01;
	state >>= 1;
	Controller_1_Right = state & 0x01;
	state >>= 1;
	Controller_1_A = state & 0x01;
	state >>= 1;
	Controller_1_B = state & 0x01;
	state >>= 1;
	Controller_1_C = state & 0x01;
	state >>= 1;
	Controller_1_Start = state & 0x01;

	ReadFile(hGMVFile, &state, 1, &dwBytesRead, NULL);
	/* ............ */

	ReadFile(hGMVFile, &state, 1, &dwBytesRead, NULL);
	/* ............ */
}

void GMVUpdateRecord_KMod()
{
	DWORD	dwBytesWritten;
	BYTE	state;

	if (hGMVFile == NULL)	return;

	if (bGMVRecord == FALSE)	return; // playing

	state = Controller_1_Up;
	state += (Controller_1_Down << 1);
	state += (Controller_1_Left << 2);
	state += (Controller_1_Right << 3);
	state += (Controller_1_A << 4);
	state += (Controller_1_B << 5);
	state += (Controller_1_C << 6);
	state += (Controller_1_Start << 7);
	WriteFile(hGMVFile, &state, 1, &dwBytesWritten, NULL);

	state = Controller_2_Up;
	state += (Controller_2_Down << 1);
	state += (Controller_2_Left << 2);
	state += (Controller_2_Right << 3);
	state += (Controller_2_A << 4);
	state += (Controller_2_B << 5);
	state += (Controller_2_C << 6);
	state += (Controller_2_Start << 7);
	WriteFile(hGMVFile, &state, 1, &dwBytesWritten, NULL);

	state = Controller_1_X;
	state += (Controller_1_Y << 1);
	state += (Controller_1_Z << 2);
	state += (Controller_1_Mode << 3);
	state += (Controller_2_X << 4);
	state += (Controller_2_Y << 5);
	state += (Controller_2_Z << 6);
	state += (Controller_2_Mode << 7);
	WriteFile(hGMVFile, &state, 1, &dwBytesWritten, NULL);
}