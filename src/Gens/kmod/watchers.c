#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../G_main.h"
#include "../G_gfx.h" //used for Put_Info

#include "../Mem_M68k.h"

#include "../resource.h"
#include "../parser.h" //used for struct watchers

//TODO remove to use right header(s)
#include "../kmod.h"

#include "common.h"
#include "watchers.h"


#define WATCHER_SUBDATA		0x8000
#define WATCHER_STRUCT_MASK	0x7F00 // 126 struct max
#define WATCHER_SIZE_MASK	0x00FF

struct str_Watcher {
	DWORD	adr;
	char	name[50];
	WORD	size;
};

struct str_StructElement {
	char	name[20];
	UCHAR	size;
};

struct str_Structure {
	char	name[20];
	UCHAR	nbElements;
	struct str_StructElement	elements[10];
} StructureKMod[126];


HWND hWatchers;
HWND hWatchList;

char szWatchDir[MAX_PATH];

/// MSDN : You cannot set the state or lParam members for subitems because subitems do not have these attributes

char *GetZoneWatch_KMod(unsigned long adr)
{
	if (adr < 0x00400000)
		return "ROM";
	else if (adr < 0x00800000)
		return "Sega 1";
	else if (adr < 0x00A00000)
		return "Sega 2";
	else if (adr < 0x00A10000)
		return "Z80";
	else if (adr < 0x00A11000)
		return "IO";
	else if (adr < 0x00A12000)
		return "Control";
	else if (adr < 0x00B00000)
		return "Sega 3";
	else if (adr < 0x00C00000)
		return "Sega 4";
	else if (adr < 0x00E00000)
		return "VDP";
	else if (adr < 0x00FF0000)
		return "!! WARNING !!";
	else
		return "RAM";

}


void AddWatcher_KMod(struct str_Watcher newWatcher, WORD pos)//unsigned long adrHexa, char *wchname )
{
	LVITEM		lvItem;
	char		buf[64];
	struct str_Watcher* watcher;

	watcher = (struct str_Watcher *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct str_Watcher));
	if (!watcher)	return;

	watcher->adr = newWatcher.adr;
	watcher->size = newWatcher.size;
	strcpy(watcher->name, newWatcher.name);


	/* add a watcher to 0 (user must mod it later) */
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.iItem = pos;
	lvItem.iSubItem = 0;
	wsprintf(buf, "%0.8X", watcher->adr);
	lvItem.pszText = buf;
	lvItem.lParam = (LPARAM)(watcher);
	ListView_InsertItem(hWatchList, &lvItem);
}



void DeleteWatcher_KMod(int  i)
{
	LVITEM		lvItem;
	struct str_Watcher* watcher;

	/* delete the first selected watch */
	if (i == -1)	i = ListView_GetNextItem(hWatchList, -1, LVNI_ALL | LVNI_SELECTED);

	if (i == -1)	return;

	do
	{
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		if (!ListView_GetItem(hWatchList, &lvItem))		return; //happens if i<0 for example...but it's impossible unless bug

		watcher = (struct str_Watcher*) lvItem.lParam;
		i--;
	} while (watcher->size & WATCHER_SUBDATA); //if subdata, move back to main watcher

	i++;
	HeapFree(GetProcessHeap(), 0, watcher);
	ListView_DeleteItem(hWatchList, i);


	//if ( !(watcher->size & WATCHER_STRUCT_MASK) )	return; //no sub element to delete

	do
	{
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		if (!ListView_GetItem(hWatchList, &lvItem))		return;

		watcher = (struct str_Watcher*) lvItem.lParam;
		if (watcher->size & WATCHER_SUBDATA)	ListView_DeleteItem(hWatchList, i);

	} while (watcher->size & WATCHER_SUBDATA); //delete every subdata
}

void SaveWatchers_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;
	LVITEM		lvItem;
	WORD	i, nbItems;
	struct str_Watcher* tmpWatcher;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, szWatchDir);
	strcat(szFileName, Rom_Name);
	strcat(szFileName, ".wch");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "Watchers (*.wch)\0*.wch\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = szWatchDir;//(LPSTR)NULL;
	szFile.lpstrTitle = "Save Watchers";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "wch";


	SetCurrentDirectory(Gens_Path);

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;


	nbItems = ListView_GetItemCount(hWatchList);
	if (nbItems < 1)
	{
		MessageBox(NULL, "No watcher to save", NULL, MB_OK);
		CloseHandle(hFr);
		return;
	}

	dwBytesToWrite = sizeof(nbItems);
	WriteFile(hFr, &nbItems, dwBytesToWrite, &dwBytesWritten, NULL);


	dwBytesToWrite = sizeof(struct str_Watcher);
	for (i = 0; i<nbItems; i++)
	{
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		ListView_GetItem(hWatchList, &lvItem);
		tmpWatcher = (struct str_Watcher*) lvItem.lParam;


		//only save main watcher
		if (!(tmpWatcher->size&WATCHER_SUBDATA))
		{
			//WATCHER_STRUCT_MASK is saved with watcher, assuming user won't change the structures !!
			WriteFile(hFr, tmpWatcher, dwBytesToWrite, &dwBytesWritten, NULL);
		}
	}

	CloseHandle(hFr);

	Put_Info("Watchers saved", 1500);
}


BOOL ImportWatchers_KMod(HANDLE source)
{
	DWORD dwBytesToRead, dwBytesRead;
	WORD	i, nbItems;
	struct str_Watcher tmpWatcher;

	ListView_DeleteAllItems(hWatchList);

	dwBytesToRead = sizeof(nbItems);
	ReadFile(source, &nbItems, dwBytesToRead, &dwBytesRead, NULL);
	if (nbItems < 1)	return (FALSE);

	dwBytesToRead = sizeof(tmpWatcher);
	for (i = 0; i<nbItems; i++)
	{
		ReadFile(source, &tmpWatcher, dwBytesToRead, &dwBytesRead, NULL);
		AddWatcher_KMod(tmpWatcher, i);
	}

	return (TRUE);

}

void LoadWatchers_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;



	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "Watchers (*.wch)\0*.wch\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = szWatchDir;//(LPSTR)NULL;
	szFile.lpstrTitle = "Load Watchers";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "wch";


	SetCurrentDirectory(Gens_Path);

	if (GetOpenFileName(&szFile) != TRUE)   return;


	hFr = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)	return;

	ImportWatchers_KMod(hFr);

	CloseHandle(hFr);

	Put_Info("Watchers loaded", 1500);
}


void ImportStructures_KMod(HANDLE hFr)
{
	DWORD i;

	CHAR type[6], name[20];
	BOOL	inStruct = FALSE;
	CHAR	curStruct = 0, curElement = 0;


	ZeroMemory(StructureKMod, sizeof(StructureKMod));

	if (!Parser_Load(hFr))	return;

	for (i = 0; i< Parser_NumLine(); i++)
	{
		sscanf((char *)trim(Parser_Line(i), " \t"), "%s %s", type, name);

		if (lstrcmpi(type, "STRUCT") == 0)
		{
			//new struct

			//Close previous one
			if (inStruct)	curStruct++;

			lstrcpyn(StructureKMod[curStruct].name, name, 20);
			StructureKMod[curStruct].nbElements = 0;

			inStruct = TRUE;
			curElement = 0;
		}
		else if (lstrcmpi(type, "END") == 0)
		{
			//end struct
			if (inStruct)	curStruct++;
			inStruct = FALSE;
		}
		else if (lstrcmpi(type, "CHAR") == 0)
		{
			//char
			if (inStruct)
			{
				lstrcpyn(StructureKMod[curStruct].elements[curElement].name, name, 20);
				StructureKMod[curStruct].elements[curElement].size = 1;

				curElement++;
				StructureKMod[curStruct].nbElements++;
			}
		}
		else if (lstrcmpi(type, "SHORT") == 0)
		{
			//short
			if (inStruct)
			{
				lstrcpyn(StructureKMod[curStruct].elements[curElement].name, name, 20);
				StructureKMod[curStruct].elements[curElement].size = 2;

				curElement++;
				StructureKMod[curStruct].nbElements++;
			}
		}
		else if (lstrcmpi(type, "LONG") == 0)
		{
			//long
			if (inStruct)
			{
				lstrcpyn(StructureKMod[curStruct].elements[curElement].name, name, 20);
				StructureKMod[curStruct].elements[curElement].size = 4;

				curElement++;
				StructureKMod[curStruct].nbElements++;
			}
		}

	}

	Parser_Unload();
}

void AutoLoadWatchers_KMod()
{
	char szFileName[MAX_PATH];
	HANDLE hFr;

	strcpy(szFileName, Rom_Dir);
	strcat(szFileName, Rom_Name);
	strcat(szFileName, ".wch");

	hFr = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
	{
		//try in watcher dir
		strcpy(szFileName, szWatchDir);
		strcat(szFileName, Rom_Name);
		strcat(szFileName, ".wch");

		hFr = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFr == INVALID_HANDLE_VALUE)	return;
	}

	ImportWatchers_KMod(hFr);

	CloseHandle(hFr);

	strcpy(szFileName, Rom_Dir);
	strcat(szFileName, Rom_Name);
	strcat(szFileName, ".str");

	hFr = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
	{
		//try in watcher dir
		strcpy(szFileName, szWatchDir);
		strcat(szFileName, Rom_Name);
		strcat(szFileName, ".str");

		hFr = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	}

	if (hFr != INVALID_HANDLE_VALUE)	ImportStructures_KMod(hFr);

	CloseHandle(hFr);


	Put_Info("Watchers auto loaded", 1500);
}


void ChangeWatcherType_KMod(int item, WORD size)
{
	LVITEM		lvItem;
	struct str_Watcher*	watcher;
	struct str_Watcher newWatcher;

	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = item;
	lvItem.iSubItem = 0;
	ListView_GetItem(hWatchList, &lvItem);
	watcher = (struct str_Watcher*) lvItem.lParam;
	if (!watcher)	return;

	//subelement can't be changed to struct
	if (watcher->size & WATCHER_SUBDATA)	return;

	newWatcher.adr = watcher->adr;
	newWatcher.size = size;
	lstrcpy(newWatcher.name, watcher->name);


	DeleteWatcher_KMod(item); //delete it (with all its possible sub data)

	AddWatcher_KMod(newWatcher, item); //recreate it
}

void ChangeWatcherStruct_KMod(int item, WORD structIdx)
{
	/*
	WATCHER_SUBDATA		0x8000
	WATCHER_STRUCT_MASK	0x7F00
	WATCHER_SIZE_MASK	0x00FF
	*/


	LVITEM		lvItem;
	struct str_Watcher*	watcher;
	struct str_Watcher newWatcher;

	DWORD	adr;
	CHAR	idx;
	CHAR	basename[71]; //50+20+.

	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = item;
	lvItem.iSubItem = 0;
	ListView_GetItem(hWatchList, &lvItem);
	watcher = (struct str_Watcher*) lvItem.lParam;
	if (!watcher)	return;

	//subelement can't be changed to struct
	if (watcher->size & WATCHER_SUBDATA)	return;

	lstrcpy(basename, watcher->name);
	lstrcat(basename, ".");

	adr = watcher->adr;

	newWatcher.adr = adr;
	newWatcher.size = (structIdx + 1) << 8;
	newWatcher.size |= 4; //to be update like a dword
	lstrcpy(newWatcher.name, watcher->name);


	DeleteWatcher_KMod(item); //delete it (with all its possible sub data)

	AddWatcher_KMod(newWatcher, item); //recreate it

	//TODO move this to AddWatcher ?
	for (idx = 0; idx < StructureKMod[structIdx].nbElements; idx++)
	{
		ZeroMemory(&newWatcher, sizeof(newWatcher));

		newWatcher.adr = adr;
		newWatcher.size = StructureKMod[structIdx].elements[idx].size | WATCHER_SUBDATA;
		lstrcpy(newWatcher.name, basename);
		lstrcat(newWatcher.name, StructureKMod[structIdx].elements[idx].name);

		AddWatcher_KMod(newWatcher, item + 1 + idx);

		adr += StructureKMod[structIdx].elements[idx].size;
	}
}

void SeeWatcher_KMod(int item)
{
	LVITEM		lvItem;
	struct str_Watcher*	watcher;

	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = item;
	lvItem.iSubItem = 0;
	ListView_GetItem(hWatchList, &lvItem);
	watcher = (struct str_Watcher*) lvItem.lParam;
	if (!watcher)	return;

	OpenWindow_KMod(DMODE_68K);
	JumpM68KRam_KMod(watcher->adr - 0x00FF0000);
}


void ResizeWatcher_KMod(HWND hDlg, WORD newWidth, WORD newHeight)
{

	HWND hwndButton;

	MoveWindow(hWatchList, 5, 5, newWidth - 10, newHeight - 40, TRUE);

	hwndButton = GetDlgItem(hDlg, IDC_WATCHER_ADD);
	MoveWindow(hwndButton, 5, newHeight - 30, 90, 25, TRUE);
	hwndButton = GetDlgItem(hDlg, IDC_WATCHER_DEL);
	MoveWindow(hwndButton, 100, newHeight - 30, 90, 25, TRUE);

	hwndButton = GetDlgItem(hDlg, IDC_WATCHER_SAVE);
	MoveWindow(hwndButton, newWidth - 190, newHeight - 30, 90, 25, TRUE);
	hwndButton = GetDlgItem(hDlg, IDC_WATCHER_LOAD);
	MoveWindow(hwndButton, newWidth - 95, newHeight - 30, 90, 25, TRUE);

	RedrawWindow(hDlg, NULL, NULL, RDW_INVALIDATE);
}


void WatcherInit_KMod( )
{
	LV_COLUMN   lvColumn;
	int         i;
	TCHAR       szString[4][20] = { "Address (hex)", "Name", "Value", "Type" };

	hWatchList = GetDlgItem(hWatchers, IDC_WATCHER_LIST);
	ListView_DeleteAllItems(hWatchList);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 84;
	for (i = 0; i < 4; i++)
	{
		lvColumn.pszText = szString[i];
		ListView_InsertColumn(hWatchList, i, &lvColumn);
	}

	ListView_SetExtendedListViewStyle(hWatchList, /*LVS_EX_FULLROWSELECT |*/ LVS_EX_GRIDLINES);
}


LRESULT ProcessCustomDraw(LPARAM lParam)
{
	HFONT hFontBold, hOldFont, hFont;

	LOGFONT lf = { 0 };


	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

	LVITEM		lvItem;
	struct str_Watcher*	watcher;

	switch (lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT: //Before the paint cycle begins
		//request notifications for individual listview items
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT: //Before an item is drawn
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = (int)lplvcd->nmcd.dwItemSpec;
		lvItem.iSubItem = 0;
		ListView_GetItem(hWatchList, &lvItem);
		watcher = (struct str_Watcher*) lvItem.lParam;

		hFont = (HFONT)SendMessage(hWatchList, WM_GETFONT, 0, 0);
		GetObject(hFont, sizeof(LOGFONT), &lf);
		lf.lfItalic = TRUE;

		hFontBold = CreateFontIndirect(&lf);

		//if (((int)lplvcd->nmcd.dwItemSpec%2)==0)
		if (watcher->size & WATCHER_SUBDATA)
		{
			hOldFont = (HFONT)SelectObject(lplvcd->nmcd.hdc, hFontBold);
			DeleteObject(hFontBold);
			return CDRF_NEWFONT;
		}
		else{
			DeleteObject(hFontBold);
			return CDRF_DODEFAULT;
		}
		break;

		//Before a subitem is drawn
	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		/*
		if ( ListView_GetNextItem(hWatchList, -1, LVNI_ALL | LVNI_SELECTED) == (int)lplvcd->nmcd.dwItemSpec)
		{
		if (0 == lplvcd->iSubItem)
		{
		//customize subitem appearance for column 0
		lplvcd->clrText   = RGB(255,0,0);
		lplvcd->clrTextBk = RGB(255,255,255);

		//To set a custom font:
		//SelectObject(lplvcd->nmcd.hdc,
		//    <your custom HFONT>);

		return CDRF_NEWFONT;
		}
		else if (1 == lplvcd->iSubItem)
		{
		//customize subitem appearance for columns 1..n
		//Note: setting for column i
		//carries over to columnn i+1 unless
		//      it is explicitly reset
		lplvcd->clrTextBk = RGB(255,0,0);
		lplvcd->clrTextBk = RGB(255,255,255);

		return CDRF_NEWFONT;
		}
		}*/
		break;
	}
	return CDRF_DODEFAULT;
}

BOOL CALLBACK WatcherDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LVITEM		lvItem;
	NMHDR		*pInfo;
	LPNMITEMACTIVATE lpnmitem;
	LVHITTESTINFO lvhi;
	static	int	curItem;

	HMENU		hPopup, hSubPopup;
	POINT		pt;
	WORD		sizeItem;

	int         i, idStruct;
	char		buf[64];
	static		HWND hEdit;
	unsigned long adrHexa;

	struct str_Watcher*	watcher;
	struct str_Watcher	defWatcher;

	switch (Message)
	{
	case WM_INITDIALOG:
		WatcherInit_KMod(hwnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_WATCHER_ADD:
			defWatcher.adr = 0;
			defWatcher.size = 4;
			strcpy(defWatcher.name, "New watcher");
			AddWatcher_KMod(defWatcher, 0);
			break;

		case IDC_WATCHER_DEL:
			DeleteWatcher_KMod(-1);
			break;

		case IDC_WATCHER_SAVE:
			SaveWatchers_KMod(hwnd);
			break;

		case IDC_WATCHER_LOAD:
			LoadWatchers_KMod(hwnd);
			break;

		case IDC_WATCHER_BYTE:
			ChangeWatcherType_KMod(curItem, 1);
			break;
		case IDC_WATCHER_WORD:
			ChangeWatcherType_KMod(curItem, 2);
			break;
		case IDC_WATCHER_DWORD:
			ChangeWatcherType_KMod(curItem, 4);
			break;
		case IDC_WATCHER_PTR:
			SeeWatcher_KMod(curItem);
			break;

		default:
			if ((LOWORD(wParam) & 0x9900) == 0x9900)
			{
				ChangeWatcherStruct_KMod(curItem, LOWORD(wParam) & 0x7F);
			}
		}
		break;
	case WM_NOTIFY:
		pInfo = (NMHDR *)lParam;
		i = ListView_GetNextItem(hWatchList, -1, LVNI_ALL | LVNI_SELECTED);
		switch (pInfo->code)
		{
		case NM_CUSTOMDRAW:
			SetWindowLong(hwnd, DWL_MSGRESULT, (LONG)ProcessCustomDraw(lParam));
			break;

		case NM_DBLCLK:
			if ((pInfo->hwndFrom == hWatchList) && (i >= 0))
			{
				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = i;
				lvItem.iSubItem = 0;
				ListView_GetItem(hWatchList, &lvItem);
				watcher = (struct str_Watcher*) lvItem.lParam;
				if (watcher->size & WATCHER_SUBDATA)	return TRUE; //can't edit subdata watcher

				ListView_EditLabel(hWatchList, i);
			}
			break;

		case NM_RCLICK:
			if (pInfo->hwndFrom == hWatchList)
			{
				lpnmitem = (LPNMITEMACTIVATE)lParam;

				lvhi.pt = lpnmitem->ptAction;
				ListView_SubItemHitTest(hWatchList, &lvhi);
				if (lvhi.iItem != -1)
				{
					curItem = lvhi.iItem;

					lvItem.mask = LVIF_PARAM;
					lvItem.iItem = curItem;
					lvItem.iSubItem = 0;
					ListView_GetItem(hWatchList, &lvItem);
					watcher = (struct str_Watcher*) lvItem.lParam;


					if (lvhi.iSubItem == 2)
					{
						// popupmenu on size
						if (watcher->size & WATCHER_SUBDATA)	return TRUE; //can't change subdata size

						sizeItem = (watcher->size & WATCHER_SIZE_MASK) - 1;
						if (sizeItem == 3)	sizeItem--;


						idStruct = 0;
						if (StructureKMod[idStruct].nbElements)	hSubPopup = CreatePopupMenu();

						for (idStruct = 0; idStruct<126, StructureKMod[idStruct].nbElements; idStruct++)
						{
							AppendMenu(hSubPopup, MF_STRING, 0x9900 + idStruct, StructureKMod[idStruct].name);
							if (watcher->size & WATCHER_STRUCT_MASK)	CheckMenuRadioItem(hSubPopup, 0, 1, (watcher->size & WATCHER_STRUCT_MASK >> 8) - 1, MF_BYPOSITION);

						}

						hPopup = CreatePopupMenu();
						AppendMenu(hPopup, MF_STRING, IDC_WATCHER_BYTE, "Byte");
						AppendMenu(hPopup, MF_STRING, IDC_WATCHER_WORD, "Word");
						AppendMenu(hPopup, MF_STRING, IDC_WATCHER_DWORD, "Dword");
						if (!(watcher->size & WATCHER_STRUCT_MASK))	CheckMenuRadioItem(hPopup, 0, 2, sizeItem, MF_BYPOSITION);

						if (hSubPopup) AppendMenu(hPopup, MF_POPUP, (UINT)hSubPopup, "Structure"); //for imported from NM2WCH

					}
					else if ((watcher->adr >= 0x00FF0000) && (lvhi.iSubItem == 0))
					{
						// popupmenu on address
						hPopup = CreatePopupMenu();
						AppendMenu(hPopup, MF_STRING, IDC_WATCHER_PTR, "Jump to...");
					}

					if (hPopup)
					{
						pt = lvhi.pt;
						ClientToScreen(pInfo->hwndFrom, &pt);

						TrackPopupMenu(hPopup, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);

						DestroyMenu(hPopup);
					}
				}
			}
			// use GetCursorPos()+ScreenToClient() to get subitem or save it throught LVN_COLUMNCLICK or
			// then ListView_SubItemHitTest(), ListView_GetSubItemRect(), ... to display it at the right position)

			break;

		case LVN_BEGINLABELEDIT:
			if (i >= 0)
			{
				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = i;
				lvItem.iSubItem = 0;
				ListView_GetItem(hWatchList, &lvItem);
				watcher = (struct str_Watcher*) lvItem.lParam;
				if (!watcher)
				{
					SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
					return TRUE;
				}
				if (watcher->size & WATCHER_SUBDATA)
				{
					SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
					return TRUE;
				}
				hEdit = ListView_GetEditControl(hWatchList);
			}
			else
			{
				SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
				return TRUE;
			}
			break;

		case LVN_ENDLABELEDIT:
			if (i >= 0)
			{
				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = i;
				lvItem.iSubItem = 0;
				ListView_GetItem(hWatchList, &lvItem);
				watcher = (struct str_Watcher*) lvItem.lParam;
				if (!watcher)	return TRUE;
				if (watcher->size & WATCHER_SUBDATA)	return TRUE; //can't edit subdata watcher


				/* SOR : FFFF20 = nb Special attack + life */
				GetWindowText(hEdit, buf, sizeof(buf));
				adrHexa = strtoul(buf, NULL, 16);
				adrHexa = min(adrHexa, 0x00FFFFFF);
				adrHexa = max(adrHexa, 0x00000000);
				watcher->adr = adrHexa;
				wsprintf(buf, "%0.8X", adrHexa);

				lvItem.mask = LVIF_PARAM | LVIF_TEXT;
				lvItem.iItem = i;
				lvItem.iSubItem = 0;
				lvItem.pszText = buf;
				lvItem.lParam = (LPARAM)(watcher); //really needed ?
				ListView_SetItem(hWatchList, &lvItem);

				//UpdateWatchers_KMod( );
			}
			break;
		}
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_WATCHERS);
		break;

	case WM_SIZE:
		ResizeWatcher_KMod(hwnd, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_DESTROY:
		for (i = 0; i< ListView_GetItemCount(hWatchList); i++)
		{
			lvItem.mask = LVIF_PARAM;
			lvItem.iItem = i;
			lvItem.iSubItem = 0;
			ListView_GetItem(hWatchList, &lvItem);
			watcher = (struct str_Watcher *) lvItem.lParam;
			HeapFree(GetProcessHeap(), 0, watcher);
		}

		ListView_DeleteAllItems(hWatchList);

		DestroyWindow(hWatchers);
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void watchers_create(HINSTANCE hInstance, HWND hWndParent)
{
	hWatchers = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_WATCHERS), hWndParent, WatcherDlgProc);
}
void watchers_show(BOOL visibility)
{
	ShowWindow(hWatchers, visibility ? SW_SHOW : SW_HIDE);
}
void watchers_update()
{
	int         i, nbItems;
	LVITEM		lvItem;
	char	buf[64];
	DWORD		value;
	struct str_Watcher *watcher;

	if (OpenedWindow_KMod[13] == FALSE)	return;

	nbItems = ListView_GetItemCount(hWatchList);
	if (nbItems < 1)	return;

	for (i = 0; i<nbItems; i++)
	{
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		ListView_GetItem(hWatchList, &lvItem);
		watcher = (struct str_Watcher *) lvItem.lParam;
		if (watcher->size & 1)
			wsprintf(buf, "0x%0.2X", M68K_RB(watcher->adr));
		else if (watcher->size & 2)
			wsprintf(buf, "0x%0.4X", M68K_RW(watcher->adr));
		else
		{
			value = M68K_RW(watcher->adr);
			value <<= 16;
			value |= M68K_RW(watcher->adr + 2);
			wsprintf(buf, "0x%0.8X", value);
		}
		/*
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 2;
		lvItem.pszText = buf;
		ListView_SetItem(hWatchList, &lvItem);
		*/

		ListView_SetItemText(hWatchList, i, 1, watcher->name);
		ListView_SetItemText(hWatchList, i, 2, buf);
		ListView_SetItemText(hWatchList, i, 3, GetZoneWatch_KMod(watcher->adr));
	}
}
void watchers_reset()
{
	// creation du repertoire Watchers
	strcpy(szWatchDir, Gens_Path);
	strcat(szWatchDir, "watchers\\");
	CreateDirectory(szWatchDir, NULL);


	ListView_DeleteAllItems(GetDlgItem(hWatchers, IDC_WATCHER_LIST));

	if (KConf.bAutoWatch)
		AutoLoadWatchers_KMod();

	WatcherInit_KMod( );

}
void watchers_destroy()
{

}