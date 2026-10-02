#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../M68KD.h"
#include "../gens.h"
#include "../resource.h"
#include "../Star_68k.h"
#include "../Mem_M68k.h"
#include "../Misc.h" //for Byte_Swap
#include "../G_gfx.h" //used for Put_Info

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "m68k.h"
#include "hexbox.h"
#include "dasmbox.h"

static HWND hM68K;
static int Current_PC_M68K;
static CHAR debug_string[1024];
static CHAR register_text[2048];

static unsigned short Next_Word_M68K(void)
{
	unsigned short val;

	val = M68K_RW(Current_PC_M68K);

	Current_PC_M68K += 2;

	return(val);
}


static unsigned int Next_Long_M68K(void)
{
	unsigned int val;

	val = M68K_RW(Current_PC_M68K);
	val <<= 16;
	val |= M68K_RW(Current_PC_M68K + 2);

	Current_PC_M68K += 4;

	return(val);
}


static unsigned char M68_ViewMode;
static unsigned int  M68k_StartLineDisasm, M68k_StartLineRAM, M68k_StartLineROM;

static BOOL CALLBACK GetM68kDasmInstruction(void* context, DWORD offset, DWORD* nextOffset, LPSTR text, UINT textCapacity)
{
	char* instruction;
	(void)context;

	Current_PC_M68K = (int)offset;
	instruction = M68KDisasm(Next_Word_M68K, Next_Long_M68K);

	*nextOffset = Current_PC_M68K;
	lstrcpynA(text, instruction ? instruction : "", textCapacity);
	return TRUE;

}



static void SwitchM68kViewMode_KMod()
{
	HWND hexBox = GetDlgItem(hM68K, IDC_68K_HEXBOX);
	LONG_PTR hexBoxStyle = GetWindowLongPtr(hexBox, GWL_STYLE);
	if (M68_ViewMode == 2)
		hexBoxStyle |= HEXBOX_STYLE_EDITABLE;
	else
		hexBoxStyle &= ~((LONG_PTR)HEXBOX_STYLE_EDITABLE);
	SetWindowLongPtr(hexBox, GWL_STYLE, hexBoxStyle);

	ShowWindow(GetDlgItem(hM68K, IDC_68K_DASMBOX), M68_ViewMode == 0 ? SW_SHOW : SW_HIDE);
	ShowWindow(hexBox, M68_ViewMode == 0 ? SW_HIDE : SW_SHOW);
	if (M68_ViewMode == 0)
	{
		SendDlgItemMessage(hM68K, IDC_68K_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View ROM");
	}
	else
	{
		SendDlgItemMessage(hM68K, IDC_68K_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
	}
}


static void JumpM68kTo(DWORD address)
{
	if (M68_ViewMode == 0)
		SendDlgItemMessage(hM68K, IDC_68K_DASMBOX, DASMBOX_GOTO_ADDRESS, (WPARAM)address, 0);
	else
		SendDlgItemMessage(hM68K, IDC_68K_HEXBOX, HEXBOX_GOTO_ADDRESS, (WPARAM)address, 0);
}


static void RestoreM68kAddressLine(void)
{
	if (M68_ViewMode == 0)
		JumpM68kTo(M68k_StartLineDisasm);
	else if (M68_ViewMode == 1)
		JumpM68kTo(M68k_StartLineROM * 8);
	else
		JumpM68kTo(M68k_StartLineRAM * 8);
}



static void UpdateM68k_KMod()
{
	DASMBOX_SOURCE dasmSource;

	if (M68_ViewMode == 0)
	{
		dasmSource.size = Rom_Size;
		dasmSource.addressLength = 6;
		dasmSource.startAddress = 0;
		dasmSource.context = NULL;
		dasmSource.getInstruction = GetM68kDasmInstruction;
		SendDlgItemMessage(hM68K, IDC_68K_DASMBOX, DASMBOX_SET_SOURCE, 0, (LPARAM)&dasmSource);
	}
	else if (M68_ViewMode == 1)
	{
		SendDlgItemMessage(hM68K, IDC_68K_HEXBOX, HEXBOX_SET_DATA, Rom_Size, (LPARAM)Rom_Data);
		SendDlgItemMessage(hM68K, IDC_68K_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(6, 4, HEXBOX_MODE_WORD | HEXBOX_ENDIAN_LITTLE), (LPARAM)0);
	}
	else
	{
		SendDlgItemMessage(hM68K, IDC_68K_HEXBOX, HEXBOX_SET_DATA, sizeof(Ram_68k), (LPARAM)Ram_68k);
		SendDlgItemMessage(hM68K, IDC_68K_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(6, 4, HEXBOX_MODE_WORD | HEXBOX_ENDIAN_LITTLE), (LPARAM)0xff0000);
	}

	wsprintf(debug_string, "X=%d N=%d Z=%d V=%d C=%d  SR=%.4X Cycles=%.10d", (main68k_context.sr & 0x10) ? 1 : 0, (main68k_context.sr & 0x8) ? 1 : 0, (main68k_context.sr & 0x4) ? 1 : 0, (main68k_context.sr & 0x2) ? 1 : 0, (main68k_context.sr & 0x1) ? 1 : 0, main68k_context.sr, main68k_context.odometer);
	lstrcpy(register_text, debug_string);
	lstrcat(register_text, "\r\n");
	lstrcat(register_text, "\r\n");

	wsprintf(debug_string, "A0=%.8X A1=%.8X A2=%.8X A3=%.8X\r\nA4=%.8X A5=%.8X A6=%.8X A7=%.8X", main68k_context.areg[0], main68k_context.areg[1], main68k_context.areg[2], main68k_context.areg[3], main68k_context.areg[4], main68k_context.areg[5], main68k_context.areg[6], main68k_context.areg[7]);
	lstrcat(register_text, debug_string);
	lstrcat(register_text, "\r\n");
	lstrcat(register_text, "\r\n");

	wsprintf(debug_string, "D0=%.8X D1=%.8X D2=%.8X D3=%.8X\r\nD4=%.8X D5=%.8X D6=%.8X D7=%.8X", main68k_context.dreg[0], main68k_context.dreg[1], main68k_context.dreg[2], main68k_context.dreg[3], main68k_context.dreg[4], main68k_context.dreg[5], main68k_context.dreg[6], main68k_context.dreg[7]);
	lstrcat(register_text, debug_string);
	SetDlgItemText(hM68K, IDC_68K_STATUS_SR, register_text);

	wsprintf(debug_string, "PC=%.8X", main68k_context.pc);
	SendDlgItemMessage(hM68K, IDC_68K_PC, WM_SETTEXT, 0, (LPARAM)debug_string);

	/*
	sprintf(GString, "Bank for Z80 = %.8X\n", Bank_Z80);
	*/
}


void Dump68KRom_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_68K");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "ROM dump (*.bin)\0*.bin\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump Genesis ROM";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "bin";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = Rom_Size;
	Byte_Swap(Rom_Data, Rom_Size);
	WriteFile(hFr, Rom_Data, dwBytesToWrite, &dwBytesWritten, NULL);
	Byte_Swap(Rom_Data, Rom_Size);

	CloseHandle(hFr);

	Put_Info("Genesis ROM dumped", 1500);
}


void Dump68K_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_68K");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump 68K memory";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "ram";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 64 * 1024;
	Byte_Swap(Ram_68k, Rom_Size);
	WriteFile(hFr, Ram_68k, dwBytesToWrite, &dwBytesWritten, NULL);
	Byte_Swap(Ram_68k, Rom_Size);

	CloseHandle(hFr);

	Put_Info("68K RAM dumped", 1500);
}

BOOL CALLBACK M68KDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		SubclassRegisterText(hwnd, IDC_68K_STATUS_SR);
		SendDlgItemMessage(hwnd, IDC_68K_JUMP_TO_INPUT, WM_SETTEXT, (WPARAM)0, (LPARAM)"0x00000000");

		SubclassEditMaxText(hwnd, IDC_68K_JUMP_TO_INPUT);

		m68kdebug_reset();
		break;

	case WM_SHOWWINDOW:
		SwitchM68kViewMode_KMod();
		break;

	case WM_NOTIFY:
		{
			HEXBOX_EDIT_NOTIFICATION* notification = (HEXBOX_EDIT_NOTIFICATION*)lParam;
			if (notification->hdr.idFrom == IDC_68K_HEXBOX &&
				notification->hdr.code == HEXBOXN_EDIT && M68_ViewMode == 2 &&
				notification->offset < sizeof(Ram_68k))
			{
				if (notification->itemSize == 1)
					Ram_68k[notification->offset] = (unsigned char)notification->value;
				else if (notification->itemSize == 2 && sizeof(Ram_68k) - notification->offset >= 2)
				{
					Ram_68k[notification->offset] = (unsigned char)notification->value;
					Ram_68k[notification->offset + 1] = (unsigned char)(notification->value >> 8);
				}
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_68K_DASMBOX:
			if (HIWORD(wParam) == DASMBOXN_SCROLL)
				M68k_StartLineDisasm = (unsigned int)lParam;
			break;
		case IDC_68K_HEXBOX:
			if (HIWORD(wParam) == HEXBOXN_SCROLL)
			{
				if (M68_ViewMode == 1)
					M68k_StartLineROM = (unsigned int)lParam;
				else if (M68_ViewMode == 2)
					M68k_StartLineRAM = (unsigned int)lParam;
			}
			break;
		case IDC_68K_DUMP_ROM:
			Dump68KRom_KMod(hwnd);
			break;
		case IDC_68K_DUMP_RAM:
			Dump68K_KMod(hwnd);
			break;
		case IDC_68K_VIEW_ROM:
			M68_ViewMode = !M68_ViewMode;
			SwitchM68kViewMode_KMod();
			UpdateM68k_KMod();
			RestoreM68kAddressLine();
			break;
		case IDC_68K_VIEW_RAM:
			M68_ViewMode = 2; //RAM
			SwitchM68kViewMode_KMod();
			UpdateM68k_KMod();
			RestoreM68kAddressLine();
			break;

		case IDC_68K_PC:
			if (M68_ViewMode == 0)
			{
				M68k_StartLineDisasm = main68k_context.pc;
				JumpM68kTo(main68k_context.pc);
			}
			break;
		case IDC_68K_JUMP_TO_INPUT:
			if (HIWORD(wParam) == EN_MAXTEXT)
			{
				// Run your Jump handling logic here!
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_68K_JUMP_TO, 0), (LPARAM)GetDlgItem(hwnd, IDC_68K_JUMP_TO_INPUT));
			}
			break;
		case IDC_68K_JUMP_TO:
		{
			DWORD adr;
			char tmp_string[32];
			GetDlgItemText(hwnd, IDC_68K_JUMP_TO_INPUT, tmp_string, 32);
			adr = strtoul(tmp_string, NULL, 16);
			if (M68_ViewMode == 0)
				M68k_StartLineDisasm = adr;
			else if (M68_ViewMode == 1)
				M68k_StartLineROM = adr / 8;
			else if (M68_ViewMode == 2)
				M68k_StartLineRAM = adr / 8;
			JumpM68kTo(adr);
			break;
		}

		}	
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_68K);
		break;

	case WM_DESTROY:
		m68kdebug_destroy();
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;

}


void m68kdebug_create(HINSTANCE hInstance, HWND hWndParent)
{
	hM68K = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUG68K), hWndParent, M68KDlgProc);
}

void m68kdebug_show(BOOL visibility)
{
	ShowWindow(hM68K, visibility ? SW_SHOW : SW_HIDE);
}

void m68kdebug_update()
{
	if (OpenedWindow_KMod[DMODE_68K-1] == FALSE)	return;

	UpdateM68k_KMod();
}

void m68kdebug_reset()
{
	M68_ViewMode = 0; //disasm
	M68k_StartLineDisasm = M68k_StartLineRAM = M68k_StartLineROM = 0;
	SwitchM68kViewMode_KMod(); // init with wrong values (since no game loaded by default)
}
void m68kdebug_destroy()
{
	DestroyWindow(hM68K);
}

void m68kdebug_dump()
{
	Dump68K_KMod(hM68K);
}

void m68kdebug_jumpRAM(DWORD adr)
{
	M68_ViewMode = 2; //RAM
	M68k_StartLineRAM = adr / 8;
	SwitchM68kViewMode_KMod();
	UpdateM68k_KMod();
	JumpM68kTo(adr);
}