#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../gens.h"
#include "../G_main.h"
#include "common.h"
#include "resource.h"
#include "lua_script.h"

#define LUA_RECENT_COUNT 10
#define WM_KMOD_LUA_OUTPUT (WM_APP + 0x415)

static HWND hLuaScript;
static char recentScripts[LUA_RECENT_COUNT][MAX_PATH];
static char currentScript[MAX_PATH];

static void LuaScriptRefreshRecent(HWND hwnd);

static void LuaScriptLoadRecent(void)
{
	char configPath[MAX_PATH];
	char key[32];
	char path[MAX_PATH];
	int count = 0;
	int i;
	int j;

	strcpy(configPath, Gens_Path);
	strcat(configPath, "GensKMod.cfg");
	ZeroMemory(recentScripts, sizeof(recentScripts));

	for (i = 0; i < LUA_RECENT_COUNT; i++)
	{
		wsprintf(key, "Script %d", i + 1);
		GetPrivateProfileString("Lua", key, "", path, MAX_PATH, configPath);
		if (path[0] == '\0')
			continue;

		for (j = 0; j < count; j++)
		{
			if (_stricmp(recentScripts[j], path) == 0)
				break;
		}
		if (j == count)
			strcpy(recentScripts[count++], path);
	}
}

static void LuaScriptSaveRecent(void)
{
	char configPath[MAX_PATH];
	char key[32];
	int i;

	strcpy(configPath, Gens_Path);
	strcat(configPath, "GensKMod.cfg");
	for (i = 0; i < LUA_RECENT_COUNT; i++)
	{
		wsprintf(key, "Script %d", i + 1);
		WritePrivateProfileString("Lua", key, recentScripts[i], configPath);
	}
}

static void LuaScriptAddRecent(const char *path)
{
	char pathCopy[MAX_PATH];
	int existingIndex = LUA_RECENT_COUNT - 1;
	int i;

	lstrcpyn(pathCopy, path, MAX_PATH);
	for (i = 0; i < LUA_RECENT_COUNT; i++)
	{
		if (_stricmp(recentScripts[i], pathCopy) == 0)
		{
			existingIndex = i;
			break;
		}
	}
	if (existingIndex == 0)
		return;

	for (i = existingIndex; i > 0; i--)
		strcpy(recentScripts[i], recentScripts[i - 1]);
	strcpy(recentScripts[0], pathCopy);
	LuaScriptSaveRecent();
}

static void LuaScriptSetOutput(HWND hwnd, const char *message)
{
	char formatted[MAX_PATH + 80];

	wsprintf(formatted, "LDLG: %s\r\n", message);
	SetDlgItemText(hwnd, IDC_LUA_OUTPUT, formatted);
}

static void LuaScriptAppendOutput(HWND hwnd, const char *message)
{
	HWND output = GetDlgItem(hwnd, IDC_LUA_OUTPUT);
	int currentLength = GetWindowTextLength(output);
	size_t messageLength = strlen(message);
	char *contents = (char *)malloc((size_t)currentLength + messageLength + 1);

	if (contents == NULL)
		return;
	GetDlgItemText(hwnd, IDC_LUA_OUTPUT, contents, currentLength + 1);
	memcpy(contents + currentLength, message, messageLength + 1);

	SetDlgItemText(hwnd, IDC_LUA_OUTPUT, contents);
	free(contents);
	currentLength = GetWindowTextLength(output);
	SendMessage(output, EM_SETSEL, currentLength, currentLength);
	SendMessage(output, EM_SCROLLCARET, 0, 0);
}

void kmod_lua_output(const char *owner, const char *message)
{
	char *formatted;
	const char *ownerName;
	size_t ownerLength;
	size_t messageLength;
	size_t position = 0;

	if (hLuaScript == NULL || message == NULL)
		return;
	ownerName = owner != NULL && owner[0] != '\0' ? owner : "NONE";
	ownerLength = strlen(ownerName);
	messageLength = strlen(message);
	formatted = (char *)malloc(ownerLength + messageLength + 5);
	if (formatted == NULL)
		return;
	memcpy(formatted, ownerName, ownerLength);
	position += ownerLength;
	memcpy(formatted + position, ": ", 2);
	position += 2;
	memcpy(formatted + position, message, messageLength);
	position += messageLength;
	memcpy(formatted + position, "\r\n", 2);
	position += 2;
	formatted[position] = '\0';
	if (!PostMessage(hLuaScript, WM_KMOD_LUA_OUTPUT, 0, (LPARAM)formatted))
		free(formatted);
}

static BOOL LuaScriptOpen(HWND hwnd, const char *path)
{
	FILE *file;
	char *contents;
	char message[MAX_PATH + 64];
	long fileSize;
	size_t bytesRead;

	file = fopen(path, "rb");
	if (file == NULL)
	{
		wsprintf(message, "Unable to open script:\r\n%s", path);
		LuaScriptSetOutput(hwnd, message);
		return FALSE;
	}
	if (fseek(file, 0, SEEK_END) != 0 || (fileSize = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0)
	{
		fclose(file);
		LuaScriptSetOutput(hwnd, "Unable to read script file.");
		return FALSE;
	}

	contents = (char *)malloc((size_t)fileSize + 1);
	if (contents == NULL)
	{
		fclose(file);
		LuaScriptSetOutput(hwnd, "Not enough memory to open script.");
		return FALSE;
	}
	bytesRead = fread(contents, 1, (size_t)fileSize, file);
	if (bytesRead != (size_t)fileSize || ferror(file))
	{
		free(contents);
		fclose(file);
		LuaScriptSetOutput(hwnd, "Unable to read script file.");
		return FALSE;
	}
	fclose(file);
	contents[fileSize] = '\0';

	if (!SetDlgItemText(hwnd, IDC_LUA_EDITOR, contents))
	{
		free(contents);
		LuaScriptSetOutput(hwnd, "Unable to display script contents.");
		return FALSE;
	}
	free(contents);
	strcpy(currentScript, path);
	LuaScriptAddRecent(path);
	LuaScriptRefreshRecent(hwnd);
	SetDlgItemText(hwnd, IDC_LUA_OUTPUT, "");
	return TRUE;
}

static BOOL LuaScriptSave(HWND hwnd)
{
	FILE *file;
	char *contents;
	char message[MAX_PATH + 64];
	int textLength;
	int textWritten;
	int closeResult;

	if (currentScript[0] == '\0')
	{
		LuaScriptSetOutput(hwnd, "Open a script before saving.");
		return FALSE;
	}

	textLength = GetWindowTextLength(GetDlgItem(hwnd, IDC_LUA_EDITOR));
	contents = (char *)malloc((size_t)textLength + 1);
	if (contents == NULL)
	{
		LuaScriptSetOutput(hwnd, "Not enough memory to save script.");
		return FALSE;
	}
	GetDlgItemText(hwnd, IDC_LUA_EDITOR, contents, textLength + 1);

	file = fopen(currentScript, "wb");
	if (file == NULL)
	{
		wsprintf(message, "Unable to save script:\r\n%s", currentScript);
		free(contents);
		LuaScriptSetOutput(hwnd, message);
		return FALSE;
	}
	textWritten = (int)fwrite(contents, 1, (size_t)textLength, file);
	closeResult = fclose(file);
	free(contents);
	if (textWritten != textLength || closeResult != 0)
	{
		LuaScriptSetOutput(hwnd, "Unable to write script file.");
		return FALSE;
	}
	SetDlgItemText(hwnd, IDC_LUA_OUTPUT, "");
	return TRUE;
}

static const char *LuaScriptFileName(const char *path)
{
	const char *fileName = strrchr(path, '\\');
	const char *forwardSlash = strrchr(path, '/');
	if (forwardSlash != NULL && (fileName == NULL || forwardSlash > fileName))
		fileName = forwardSlash;
	return fileName != NULL ? fileName + 1 : path;
}

static void LuaScriptRefreshRecent(HWND hwnd)
{
	HWND list = GetDlgItem(hwnd, IDC_LUA_RECENT);
	LRESULT item;
	int i;

	SendMessage(list, LB_RESETCONTENT, 0, 0);
	for (i = 0; i < LUA_RECENT_COUNT; i++)
	{
		if (recentScripts[i][0] == '\0')
			continue;
		item = SendMessage(list, LB_ADDSTRING, 0, (LPARAM)LuaScriptFileName(recentScripts[i]));
		if (item != LB_ERR && item != LB_ERRSPACE)
			SendMessage(list, LB_SETITEMDATA, (WPARAM)item, (LPARAM)i);
	}
}

static void LuaScriptOpenSelected(HWND hwnd)
{
	LRESULT selection;
	LRESULT recentIndex;

	selection = SendDlgItemMessage(hwnd, IDC_LUA_RECENT, LB_GETCURSEL, 0, 0);
	if (selection == LB_ERR)
	{
		LuaScriptSetOutput(hwnd, "Select a recent script first.");
		return;
	}
	recentIndex = SendDlgItemMessage(hwnd, IDC_LUA_RECENT, LB_GETITEMDATA, (WPARAM)selection, 0);
	if (recentIndex < 0 || recentIndex >= LUA_RECENT_COUNT)
	{
		LuaScriptSetOutput(hwnd, "The selected recent script is invalid.");
		return;
	}
	LuaScriptOpen(hwnd, recentScripts[recentIndex]);
}

static void LuaScriptBrowse(HWND hwnd)
{
	OPENFILENAME dialog;
	char path[MAX_PATH];

	ZeroMemory(&dialog, sizeof(dialog));
	path[0] = '\0';
	if (currentScript[0] != '\0')
		lstrcpyn(path, currentScript, MAX_PATH);
	dialog.lStructSize = sizeof(dialog);
	dialog.hwndOwner = hwnd;
	dialog.lpstrFilter = "Lua scripts (*.lua)\0*.lua\0All files (*.*)\0*.*\0\0";
	dialog.lpstrFile = path;
	dialog.nMaxFile = MAX_PATH;
	dialog.lpstrInitialDir = Gens_Path;
	dialog.lpstrTitle = "Open Lua script";
	dialog.lpstrDefExt = "lua";
	dialog.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileName(&dialog))
		LuaScriptOpen(hwnd, path);
	else if (CommDlgExtendedError() != 0)
		LuaScriptSetOutput(hwnd, "Unable to open the file picker.");
}

static BOOL LuaScriptToolbarInit(HWND hwnd)
{
	HWND toolbar;
	TBBUTTON buttons[3];
	int stringBase;

	toolbar = GetDlgItem(hwnd, IDC_LUA_TOOLBAR);
	if (toolbar == NULL)
		return FALSE;

	SendMessage(toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(toolbar, TB_LOADIMAGES, IDB_STD_SMALL_COLOR, (LPARAM)HINST_COMMCTRL);
	stringBase = (int)SendMessage(toolbar, TB_ADDSTRING, 0, (LPARAM)"Browse\0Edit\0Save\0");
	if (stringBase == -1)
		return FALSE;

	ZeroMemory(buttons, sizeof(buttons));
	buttons[0].iBitmap = STD_FILEOPEN;
	buttons[0].idCommand = IDC_LUA_BROWSE;
	buttons[0].fsState = TBSTATE_ENABLED;
	buttons[0].fsStyle = BTNS_BUTTON;
	buttons[0].iString = stringBase;

	buttons[1].iBitmap = STD_PROPERTIES;
	buttons[1].idCommand = IDC_LUA_EDIT;
	buttons[1].fsState = TBSTATE_ENABLED;
	buttons[1].fsStyle = BTNS_BUTTON;
	buttons[1].iString = stringBase + 1;

	buttons[2].iBitmap = STD_FILESAVE;
	buttons[2].idCommand = IDC_LUA_SAVE;
	buttons[2].fsState = TBSTATE_ENABLED;
	buttons[2].fsStyle = BTNS_BUTTON;
	buttons[2].iString = stringBase + 2;

	if (!SendMessage(toolbar, TB_ADDBUTTONS, 3, (LPARAM)buttons))
		return FALSE;

	SendMessage(toolbar, TB_AUTOSIZE, 0, 0);
	return TRUE;
}

static BOOL CALLBACK LuaScriptDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		LuaScriptToolbarInit(hwnd);
		SendDlgItemMessage(hwnd, IDC_LUA_EDITOR, EM_SETLIMITTEXT, 0x7ffffffe, 0);
		SendDlgItemMessage(hwnd, IDC_LUA_OUTPUT, EM_SETLIMITTEXT, 0x7ffffffe, 0);
		LuaScriptLoadRecent();
		LuaScriptRefreshRecent(hwnd);
		return TRUE;

	case WM_KMOD_LUA_OUTPUT:
	{
		char *output = (char *)lParam;
		if (output != NULL)
		{
			LuaScriptAppendOutput(hwnd, output);
			free(output);
		}
		return TRUE;
	}

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_LUASCRIPT);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LUA_WEBSITE:
			if (HIWORD(wParam) == STN_CLICKED)
			{
				ShellExecute(hwnd, "open", "https://www.lua.org", NULL, NULL, SW_SHOWNORMAL);
				return TRUE;
			}
			break;
		case IDC_LUA_BROWSE:
			LuaScriptBrowse(hwnd);
			return TRUE;
		case IDC_LUA_EDIT:
			LuaScriptOpenSelected(hwnd);
			return TRUE;
		case IDC_LUA_SAVE:
			LuaScriptSave(hwnd);
			return TRUE;
		case IDC_LUA_RECENT:
			if (HIWORD(wParam) == LBN_DBLCLK)
			{
				LuaScriptOpenSelected(hwnd);
				return TRUE;
			}
			break;
		}
		return FALSE;

	case WM_CTLCOLORSTATIC:
		if (GetDlgCtrlID((HWND)lParam) == IDC_LUA_WEBSITE)
		{
			SetTextColor((HDC)wParam, RGB(0, 0, 255));
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (BOOL)(INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
		}
		break;

	default:
		return FALSE;
	}
}

void lua_script_create(HINSTANCE hInstance, HWND hWndParent)
{
	INITCOMMONCONTROLSEX controls;
	HICON smallIcon;
	HICON largeIcon;
	controls.dwSize = sizeof(controls);
	controls.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&controls);

	hLuaScript = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_LUASCRIPT), hWndParent, LuaScriptDlgProc);
	if (hLuaScript != NULL)
	{
		smallIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_LUA_SCRIPT), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
		largeIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_LUA_SCRIPT), IMAGE_ICON,
			GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_SHARED);
		if (smallIcon != NULL)
			SendMessage(hLuaScript, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);
		if (largeIcon != NULL)
			SendMessage(hLuaScript, WM_SETICON, ICON_BIG, (LPARAM)largeIcon);
	}
}

void lua_script_show(BOOL visibility)
{
	if (hLuaScript != NULL)
		ShowWindow(hLuaScript, visibility ? SW_SHOW : SW_HIDE);
}
