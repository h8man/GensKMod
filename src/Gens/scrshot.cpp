#include <windows.h>
#include "G_Main.h"
#include "G_gfx.h"
#include "rom.h"
#include "scrshot.h"

#include "kmod/utils.h"


char ScrShot_Dir[1024] = "\\";

void Create_Shot(unsigned char* Dest, int X, int Y, unsigned char* Screen, int Pitch, int mode)
{
	int offs, i, j, len, tmp;
	unsigned char* Src = NULL;

	len = (X * Y * 3) + 54;

	Dest[0] = 'B';
	Dest[1] = 'M';

	Dest[2] = (unsigned char)((len >> 0) & 0xFF);
	Dest[3] = (unsigned char)((len >> 8) & 0xFF);
	Dest[4] = (unsigned char)((len >> 16) & 0xFF);
	Dest[5] = (unsigned char)((len >> 24) & 0xFF);

	Dest[6] = Dest[7] = Dest[8] = Dest[9] = 0;

	Dest[10] = 54;
	Dest[11] = Dest[12] = Dest[13] = 0;

	Dest[14] = 40;
	Dest[15] = Dest[16] = Dest[17] = 0;

	Dest[18] = (unsigned char)((X >> 0) & 0xFF);
	Dest[19] = (unsigned char)((X >> 8) & 0xFF);
	Dest[20] = (unsigned char)((X >> 16) & 0xFF);
	Dest[21] = (unsigned char)((X >> 24) & 0xFF);

	Dest[22] = (unsigned char)((Y >> 0) & 0xFF);
	Dest[23] = (unsigned char)((Y >> 8) & 0xFF);
	Dest[24] = (unsigned char)((Y >> 16) & 0xFF);
	Dest[25] = (unsigned char)((Y >> 24) & 0xFF);

	Dest[26] = 1;
	Dest[27] = 0;

	Dest[28] = 24;
	Dest[29] = 0;

	Dest[30] = Dest[31] = Dest[32] = Dest[33] = 0;

	len -= 54;

	Dest[34] = (unsigned char)((len >> 0) & 0xFF);
	Dest[35] = (unsigned char)((len >> 8) & 0xFF);
	Dest[36] = (unsigned char)((len >> 16) & 0xFF);
	Dest[37] = (unsigned char)((len >> 24) & 0xFF);

	Dest[38] = Dest[42] = 0xC4;
	Dest[39] = Dest[43] = 0x0E;
	Dest[40] = Dest[44] = Dest[41] = Dest[45] = 0;

	Dest[46] = Dest[47] = Dest[48] = Dest[49] = 0;
	Dest[50] = Dest[51] = Dest[52] = Dest[53] = 0;

	Src = (unsigned char*)(Screen);
	Src += Pitch * (Y - 1);

	if (mode)
	{
		for (offs = 54, j = 0; j < Y; j++, Src -= Pitch, offs += (3 * X))
		{
			for (i = 0; i < X; i++)
			{
				tmp = (unsigned int)(Src[2 * i + 0] + (Src[2 * i + 1] << 8));
				Dest[offs + (3 * i) + 2] = ((tmp >> 7) & 0xF8);
				Dest[offs + (3 * i) + 1] = ((tmp >> 2) & 0xF8);
				Dest[offs + (3 * i) + 0] = ((tmp << 3) & 0xF8);
			}
		}
	}
	else
	{
		for (offs = 54, j = 0; j < Y; j++, Src -= Pitch, offs += (3 * X))
		{
			for (i = 0; i < X; i++)
			{
				tmp = (unsigned int)(Src[2 * i + 0] + (Src[2 * i + 1] << 8));
				Dest[offs + (3 * i) + 2] = ((tmp >> 8) & 0xF8);
				Dest[offs + (3 * i) + 1] = ((tmp >> 3) & 0xFC);
				Dest[offs + (3 * i) + 0] = ((tmp << 3) & 0xF8);
			}
		}
	}
}

int Save_Shot(unsigned char *Screen, int mode, int X, int Y, int Pitch)
{
	HANDLE ScrShot_File;
	unsigned char *Dest = NULL;
	int i, j, len, num = -1;
	unsigned long BW;
	char Name[1024], ext[16];

	SetCurrentDirectory(Gens_Path);

	len = (X * Y * 3) + 54;
	
	if (!Game) return(0);
	if ((Dest = (unsigned char *) malloc(len)) == NULL) return(0);

	memset(Dest, 0, len);
	
	do
	{
		if (num++ > 99999)
		{
			free(Dest);
			return(0);
		}

		ext[0] = '_';
		i = 1;

		j = num / 10000;
		if (j) ext[i++] = '0' + j;
		j = (num / 1000) % 10;
		if (j) ext[i++] = '0' + j;
		j = (num / 100) % 10;
		ext[i++] = '0' + j;
		j = (num / 10) % 10;
		ext[i++] = '0' + j;
		j = num % 10;
		ext[i++] = '0' + j;
		ext[i++] = '.';
		ext[i++] = 'b';
		ext[i++] = 'm';
		ext[i++] = 'p';
		ext[i] = 0;

		strcpy(Name, ScrShot_Dir);
		strcat(Name, Rom_Name);
		strcat(Name, ext);

		ScrShot_File = CreateFile(Name, GENERIC_WRITE, NULL,
		NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	} while(ScrShot_File == INVALID_HANDLE_VALUE);

	Create_Shot(Dest, X, Y, Screen, Pitch, mode);

	WriteFile(ScrShot_File, Dest, len, &BW, NULL);

	CloseHandle(ScrShot_File);
	if (Dest)
	{
		free(Dest);
		Dest = NULL;
	}

	wsprintf(Name, "Screen shot %d saved", num);
	Put_Info(Name, 1500);

	return(1);
}

int Copy_Shot(unsigned char* Screen, int mode, int X, int Y, int Pitch)
{
	unsigned char* Dest = NULL;
	int i, j, len, offset = 14;

	len = (X * Y * 3) + 54;

	if (!Game) return(0);
	if ((Dest = (unsigned char*)malloc(len)) == NULL) return(0);

	memset(Dest, 0, len);

	Create_Shot(Dest, X, Y, Screen, Pitch, mode);

	CopyToClipboard(CF_DIB, Dest + offset, len - offset, 1);

	if (Dest)
	{
		free(Dest);
		Dest = NULL;
	}

	Put_Info("Screen shot saved to clipboard", 1500);

	return(1);
}