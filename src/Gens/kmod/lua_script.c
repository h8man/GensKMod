#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>

#include "../gens.h"
#include "common.h"
#include "resource.h"
#include "lua_script.h"

static HWND hLuaScript;

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
		return TRUE;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_LUASCRIPT);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_LUA_WEBSITE && HIWORD(wParam) == STN_CLICKED)
		{
			ShellExecuteA(hwnd, "open", "https://www.lua.org", NULL, NULL, SW_SHOWNORMAL);
			return TRUE;
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
