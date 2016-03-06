#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"

#include "../G_main.h"

#include "common.h"
#include "message.h"
#include "config.h"

static HWND hDMsg;

static char szKModLog[MAX_PATH];
static HANDLE	KMsgLog;
static UCHAR	msgIdx_KMod, msg_KMod[255];
static char	*errorText_KMod = "** Too many messages **";
static UINT logMaxSize, logSize;
static CHAR *logMessages;
static CHAR *logToAdd;

static HFONT hFont = NULL;



static void MsgBrowse_KMod(HWND hwnd)
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
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Message Log file";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "log";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	CloseHandle(hFr);

	strcpy(KConf.logfile, szFileName);

	WritePrivateProfileString("Debug", "file", KConf.logfile, Conf_File);

	if (KMsgLog)	CloseHandle(KMsgLog);
	//FILE_SHARE_READ pour pouvoir l'ouvrir dans notepad
	KMsgLog = CreateFile(KConf.logfile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (KMsgLog != INVALID_HANDLE_VALUE)	SetFilePointer(KMsgLog, 0, 0, FILE_END);

	SetDlgItemText(hwnd, IDC_MSG_FILE, KConf.logfile);
}

static void MsgOpen_KMod(HWND hwnd)
{
	ShellExecute(hwnd, "open", "notepad", KConf.logfile, NULL, SW_SHOW);
}

static void MsgClear_KMod(HWND hwnd)
{
	if (logToAdd)		LocalFree((HLOCAL)logToAdd);

	logToAdd = NULL;

	ZeroMemory(logMessages, logMaxSize);
	logSize = 0;

	msgIdx_KMod = 0;
	ZeroMemory(msg_KMod, 255);
}


static void MsgReset_KMod(HWND hwnd)
{
	if (KMsgLog)	CloseHandle(KMsgLog);
	KMsgLog = NULL;

	MsgClear_KMod(hwnd);

	if (logMessages)	LocalFree((HLOCAL)logMessages);
	logMessages = NULL;
}


static void MsgInit_KMod(HWND hwnd)
{
	unsigned char Conf_File[MAX_PATH];

	SetCurrentDirectory(Gens_Path);
	strcpy(Conf_File, Gens_Path);
	strcat(Conf_File, "GensKMod.cfg");

	// default file log
	strcpy(szKModLog, Gens_Path);
	strcat(szKModLog, "gens_KMod.log");

	GetPrivateProfileString("Log", "file", szKModLog, KConf.logfile, MAX_PATH, Conf_File);
	WritePrivateProfileString("Debug", "file", KConf.logfile, Conf_File);

	//FILE_SHARE_READ pour pouvoir l'ouvrir dans notepad
	KMsgLog = CreateFile(szKModLog, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (KMsgLog != INVALID_HANDLE_VALUE)	SetFilePointer(KMsgLog, 0, 0, FILE_END);

	SetDlgItemText(hwnd, IDC_MSG_FILE, szKModLog);

	logMaxSize = (UINT)SendDlgItemMessage(hDMsg, IDC_MSG_EDIT, EM_GETLIMITTEXT, (WPARAM)0, (LPARAM)0);
	logMessages = (CHAR *)LocalAlloc(LPTR, logMaxSize);
	ZeroMemory(logMessages, logMaxSize);
	logSize = 0;

	if (!Game)	return;

	wsprintf(logMessages, "*******************%s\r\n", Rom_Name);
	logSize = strlen(logMessages);

	if (KMsgLog)
	{
		DWORD dwBytesWritten;
		WriteFile(KMsgLog, logMessages, (DWORD)logSize, &dwBytesWritten, NULL);
	}

	msgIdx_KMod = 0;
	ZeroMemory(msg_KMod, 255);
}


void UpdateMsg_KMod()
{
	
}




BOOL Msg_KMod(char *msg)
{
	if (logToAdd == NULL)
	{
		if (logMaxSize == 0)		logMaxSize = (UINT)SendDlgItemMessage(hDMsg, IDC_MSG_EDIT, EM_GETLIMITTEXT, (WPARAM)0, (LPARAM)0);

		logToAdd = (CHAR *)LocalAlloc(LPTR, logMaxSize / 2);
		if (logToAdd == NULL)	return FALSE;

		ZeroMemory(logToAdd, logMaxSize / 2);
	}

	if ((strlen(logToAdd) + strlen(msg)) > (logMaxSize / 2)) 	return TRUE;

	strcat(logToAdd, msg);

	if (strlen(logToAdd) > (logMaxSize / 2))
	{
		strcat(logToAdd, "*** too much message per frame ***");
		strcat(logToAdd, "***.. skipping some messages ..***");
	}

	return TRUE;
}


BOOL CALLBACK MsgDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	
	switch (Message)
	{
	case WM_INITDIALOG:
		hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
		//	SendDlgItemMessage(hwnd, IDC_MSG_EDIT, WM_SETFONT, (WPARAM)hFont, TRUE);

		MsgInit_KMod(hwnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_MSG_CLEAR:
			MsgClear_KMod(hwnd);
			break;

		case IDC_MSG_OPEN:
			MsgOpen_KMod(hwnd);
			break;

		case IDC_MSG_BROWSE:
			MsgBrowse_KMod(hwnd);
			break;
		}
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_MSG);
		break;

	case WM_DESTROY:
		message_destroy();
		
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}



void message_create(HINSTANCE hInstance, HWND hWndParent)
{
	hDMsg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGMSG), hWndParent, MsgDlgProc);
}



void message_show(BOOL visibility)
{
	ShowWindow(hDMsg, visibility?SW_SHOW:SW_HIDE);
}

void message_reset()
{
	MsgInit_KMod(hDMsg);
}
void message_update()
{
	CHAR *editCutText;
	UINT nSizeToAdd, nSize;

	if (logMessages == NULL)	return;
	if (logToAdd == NULL)	return;

	if (KMsgLog)
	{
		DWORD dwBytesToWrite, dwBytesWritten;

		dwBytesToWrite = strlen(logToAdd);
		if (dwBytesToWrite)		WriteFile(KMsgLog, logToAdd, dwBytesToWrite, &dwBytesWritten, NULL);
	}

	nSizeToAdd = strlen(logToAdd) + 1;
	nSize = logSize; // (UINT)SendDlgItemMessage(hDMsg, IDC_MSG_EDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);
	nSize += nSizeToAdd;

	if (nSize >= logMaxSize)
	{
		editCutText = logMessages;
		editCutText += nSizeToAdd;
		do
		{
			editCutText = strstr(editCutText, "\r\n");
			if (editCutText == NULL)
			{
				editCutText = logMessages;
				nSize = nSizeToAdd;
			}
			else
			{
				editCutText += 2;
				nSize = strlen(editCutText) + nSizeToAdd;
			}
		} while (nSize > logMaxSize);
		memmove(logMessages, editCutText, strlen(editCutText) + 1);
	}
	strcat(logMessages, logToAdd);
	logSize = strlen(logMessages);

	SetDlgItemText(hDMsg, IDC_MSG_EDIT, logMessages);
	SendDlgItemMessage(hDMsg, IDC_MSG_EDIT, EM_LINESCROLL, (WPARAM)0, (LPARAM)SendDlgItemMessage(hDMsg, IDC_MSG_EDIT, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0));

	LocalFree((HLOCAL)logToAdd);
	logToAdd = NULL;
}

void message_destroy()
{
	if (KMsgLog)
		CloseHandle(KMsgLog);

	if (hFont != NULL)
		DeleteObject((HGDIOBJ)hFont);

	DestroyWindow(hDMsg);
}


void message_addChar(unsigned char b)
{
	if (b == 0)
	{
		if (msgIdx_KMod > 0)
		{
			CHAR message[300];
			wsprintf(message, "Message : %s\r\n", msg_KMod);
			Msg_KMod(message);

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
			message_addChar(0); /* flush to msgbox */
	}
}