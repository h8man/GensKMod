#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"
#include "../vdp_io.h"
#include "../Star_68k.h"

#include "common.h"
#include "spy.h"
#include "message.h"

//TODO
//#include "configuration.h"

static const char *mcd_bios_name[] =
{
	"UNKNOWN 0x00   ",
	"UNKNOWN 0x01   ",
	"MSCSTOP        ",
	"MSCPAUSEON     ",
	"MSCPAUSEOFF    ",
	"MSCSCANFF      ",
	"MSCSCANFR      ",
	"MSCSCANOFF     ",
	"ROMPAUSEON     ",
	"ROMPAUSEOFF    ",
	"DRVOPEN        ",
	"UNKNOWN 0x0b   ",
	"UNKNOWN 0x0c   ",
	"UNKNOWN 0x0d   ",
	"UNKNOWN 0x0e   ",
	"UNKNOWN 0x0f   ",
	"DRVINIT        ",
	"MSCPLAY        ",
	"MSCPLAY1       ",
	"MSCPLAYR       ",
	"MSCPLAYT       ",
	"MSCSEEK        ",
	"MSCSEEKT       ",
	"ROMREAD        ",
	"ROMSEEK        ",
	"MSCSEEK1       ",
	"UNKNOWN 0x1a   ",
	"UNKNOWN 0x1b   ",
	"UNKNOWN 0x1c   ",
	"UNKNOWN 0x1d   ",
	"TESTENTRY      ",
	"TESTENTRYLOOP  ",
	"ROMREADN       ",
	"ROMREADE       ",
	"UNKNOWN 0x22   ",
	"UNKNOWN 0x23   ",
	"UNKNOWN 0x24   ",
	"UNKNOWN 0x25   ",
	"UNKNOWN 0x26   ",
	"UNKNOWN 0x27   ",
	"UNKNOWN 0x28   ",
	"UNKNOWN 0x29   ",
	"UNKNOWN 0x2a   ",
	"UNKNOWN 0x2b   ",
	"UNKNOWN 0x2c   ",
	"UNKNOWN 0x2d   ",
	"UNKNOWN 0x2e   ",
	"UNKNOWN 0x2f   ",
	"UNKNOWN 0x30   ",
	"UNKNOWN 0x31   ",
	"UNKNOWN 0x32   ",
	"UNKNOWN 0x33   ",
	"UNKNOWN 0x34   ",
	"UNKNOWN 0x35   ",
	"UNKNOWN 0x36   ",
	"UNKNOWN 0x37   ",
	"UNKNOWN 0x38   ",
	"UNKNOWN 0x39   ",
	"UNKNOWN 0x3a   ",
	"UNKNOWN 0x3b   ",
	"UNKNOWN 0x3c   ",
	"UNKNOWN 0x3d   ",
	"UNKNOWN 0x3e   ",
	"UNKNOWN 0x3f   ",
	"UNKNOWN 0x40   ",
	"UNKNOWN 0x41   ",
	"UNKNOWN 0x42   ",
	"UNKNOWN 0x43   ",
	"UNKNOWN 0x44   ",
	"UNKNOWN 0x45   ",
	"UNKNOWN 0x46   ",
	"UNKNOWN 0x47   ",
	"UNKNOWN 0x48   ",
	"UNKNOWN 0x49   ",
	"UNKNOWN 0x4a   ",
	"UNKNOWN 0x4b   ",
	"UNKNOWN 0x4c   ",
	"UNKNOWN 0x4d   ",
	"UNKNOWN 0x4e   ",
	"UNKNOWN 0x4f   ",
	"UNKNOWN 0x50   ",
	"UNKNOWN 0x51   ",
	"UNKNOWN 0x52   ",
	"UNKNOWN 0x53   ",
	"UNKNOWN 0x54   ",
	"UNKNOWN 0x55   ",
	"UNKNOWN 0x56   ",
	"UNKNOWN 0x57   ",
	"UNKNOWN 0x58   ",
	"UNKNOWN 0x59   ",
	"UNKNOWN 0x5a   ",
	"UNKNOWN 0x5b   ",
	"UNKNOWN 0x5c   ",
	"UNKNOWN 0x5d   ",
	"UNKNOWN 0x5e   ",
	"UNKNOWN 0x5f   ",
	"UNKNOWN 0x60   ",
	"UNKNOWN 0x61   ",
	"UNKNOWN 0x62   ",
	"UNKNOWN 0x63   ",
	"UNKNOWN 0x64   ",
	"UNKNOWN 0x65   ",
	"UNKNOWN 0x66   ",
	"UNKNOWN 0x67   ",
	"UNKNOWN 0x68   ",
	"UNKNOWN 0x69   ",
	"UNKNOWN 0x6a   ",
	"UNKNOWN 0x6b   ",
	"UNKNOWN 0x6c   ",
	"UNKNOWN 0x6d   ",
	"UNKNOWN 0x6e   ",
	"UNKNOWN 0x6f   ",
	"UNKNOWN 0x70   ",
	"UNKNOWN 0x71   ",
	"UNKNOWN 0x72   ",
	"UNKNOWN 0x73   ",
	"UNKNOWN 0x74   ",
	"UNKNOWN 0x75   ",
	"UNKNOWN 0x76   ",
	"UNKNOWN 0x77   ",
	"UNKNOWN 0x78   ",
	"UNKNOWN 0x79   ",
	"UNKNOWN 0x7a   ",
	"UNKNOWN 0x7b   ",
	"UNKNOWN 0x7c   ",
	"UNKNOWN 0x7d   ",
	"UNKNOWN 0x7e   ",
	"UNKNOWN 0x7f   ",
	"CDBCHK         ",
	"CDBSTAT        ",
	"CDBTOCWRITE    ",
	"CDBTOCREAD     ",
	"CDBPAUSE       ",
	"FDRSET         ",
	"FDRCHG         ",
	"CDCSTART       ",
	"CDCSTARTP      ",
	"CDCSTOP        ",
	"CDCSTAT        ",
	"CDCREAD        ",
	"CDCTRN         ",
	"CDCACK         ",
	"SCDINIT        ",
	"SCDSTART       ",
	"SCDSTOP        ",
	"SCDSTAT        ",
	"SCDREAD        ",
	"SCDPQ          ",
	"SCDPQL         ",
	"LEDSET         ",
	"CDCSETMODE     ",
	"WONDERREQ      ",
	"WONDERCHK      "
};

static CHAR spy_string[1024];

void Spy_KMod(char *log)
{
	char fullMsg[MAX_PATH];
	wsprintf(fullMsg, "Spy:%s\r\n", log);
	Msg_KMod(fullMsg);
}



void SpyReg(unsigned char a, unsigned char b)
{
	unsigned short VDPS;

	if (!Game)	return;

	if (!KConf.Spy)	return;

	if (!KConf.SpyReg)	return;

	VDPS = Read_VDP_Status();

	switch (a)
	{
	case 0:
		if ((b & 0x04) == 0)
		{
			wsprintf(spy_string, "%.5X Reg. %d : bit 2 must be 1", main68k_context.pc, a);
			Spy_KMod(spy_string);
		}

		if (b & 0xE9)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value doesn't follow the schema 000?01?0", main68k_context.pc, a);
			Spy_KMod(spy_string);
		}
		break;

	case 1:
		if ((b & 0x04) == 0)
		{
			wsprintf(spy_string, "%.5X Reg. %d : bit 2 must be 1", a);
			Spy_KMod(spy_string);
		}

		if (b & 0x83)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 0????100", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}

		if ((b & 0x08) && ((VDPS & 1) == 0))
		{
			wsprintf(spy_string, "%.5X Reg. %d : V30 cell mode (0x08) impossible in NTSC mode", main68k_context.pc, a);
			Spy_KMod(spy_string);
		}
		break;

	case 2:
		if (b & 0xC7)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 00???000", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 3:
		if ((b & 0xC1) && (H_Cell == 32))
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 00?????0", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		else if ((b & 0xC3) && (H_Cell == 40))
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 00????00", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 4:
		if (b & 0xF8)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 00000???", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 5:
		if ((b & 0x80) && (H_Cell == 32))
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 0???????", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		else if ((b & 0x81) && (H_Cell == 40))
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 0??????0", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 6:
		if (b)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X must be 0", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 7:
		if (b & 0xC0)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 00??????", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 8:
		if (b)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X must be 0", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 9:
		if (b)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X must be 0", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 10:
		// nothing to check
		break;

	case 11:
		if (b & 0xF0)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 0000????", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}

		if ((b & 0x03) == 0x01)
		{
			wsprintf(spy_string, "%.5X Reg. %d : wrong HSCroll mode (0x01)", main68k_context.pc, a);
			Spy_KMod(spy_string);
		}
		break;

	case 12:
		if (b & 0x70)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema ?000????", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}

		if (((b & 0x81) == 0x10) || ((b & 0x81) == 0x01))
		{
			wsprintf(spy_string, "%.5X Reg. %d : wrong H cell mode (%.2X)", main68k_context.pc, a, b & 0x81);
			Spy_KMod(spy_string);
		}

		if ((b & 0x06) == 0x40)
		{
			wsprintf(spy_string, "%.5X Reg. %d : wrong interlace mode (2)", main68k_context.pc, a);
			Spy_KMod(spy_string);
		}
		break;

	case 13:
		if (b & 0xC0)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 00??????", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 14:
		if (b)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X must be 0", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 15:
		if (!b)
		{
			wsprintf(spy_string, "%.5X Reg. %d : an auto increment data of 0 is a bad idea!", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 16:
		if (b & 0xCC)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema 00??00??", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}

		if ((b & 0x03) == 0x02)
		{
			wsprintf(spy_string, "%.5X Reg. %d : wrong H scroll mode (2)", main68k_context.pc, a);
			Spy_KMod(spy_string);
		}

		if ((b & 0x30) == 0x20)
		{
			wsprintf(spy_string, "%.5X Reg. %d : wrong V scroll mode (2)", main68k_context.pc, a);
			Spy_KMod(spy_string);
		}
		break;

	case 17:
		if (b & 0x60)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema ?00?????", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 18:
		if (b & 0x60)
		{
			wsprintf(spy_string, "%.5X Reg. %d : value %.2X doesn't follow the schema ?00?????", main68k_context.pc, a, b);
			Spy_KMod(spy_string);
		}
		break;

	case 19:
		// nothing
		break;

	case 20:
		// nothing
		break;

	case 21:
		// nothing
		break;

	case 22:
		// nothing
		break;

	case 23:
		// nothing
		break;

	default:
		wsprintf(spy_string, "%.5X Wrong register (%d) called with value %.2X", main68k_context.pc, a, b);
		Spy_KMod(spy_string);
		break;
	}
}


void SpySpr()
{
	if (!Game)	return;

	if (!KConf.Spy)	return;

	if (!KConf.SpySpr)	return;

	Spy_KMod("More than 80 sprites to draw");
}

void SpyZ80NotPaused(unsigned int address)
{
	if (!Game)	return;

	if (!KConf.Spy)	return;

	if (!KConf.SpyRW)	return;

	wsprintf(spy_string, "%.5X want to access unpaused Z80 at %.8X", main68k_context.pc, address);
	Spy_KMod(spy_string);
}

void SpyZ80NotPaused_DMA(unsigned int address)
{
	if (!Game)	return;

	if (!KConf.Spy)	return;

	if (!KConf.SpyRW)	return;

	wsprintf(spy_string, "%.5X want to DMA while Z80 is running, at %.8X", main68k_context.pc, address);
	Spy_KMod(spy_string);
}


void SpyBadByteRead(unsigned int address)
{
	if (!Game)	return;

	if (!KConf.Spy)	return;

	if (!KConf.SpyRW)	return;

	wsprintf(spy_string, "%.5X want to read a Byte at %.8X", main68k_context.pc, address);
	Spy_KMod(spy_string);
}

void SpyBadWordRead(unsigned int address)
{
	if (!Game)	return;

	if (!KConf.Spy)	return;
	if (!KConf.SpyRW)	return;

	wsprintf(spy_string, "%.5X want to read a Word at %.8X", main68k_context.pc, address);
	Spy_KMod(spy_string);
}

void SpyBadByteWrite(unsigned int address)
{

	if (!Game)	return;

	if (!KConf.Spy)	return;
	if (!KConf.SpyRW)	return;

	wsprintf(spy_string, "%.5X want to write a Byte at %.8X", main68k_context.pc, address);
	Spy_KMod(spy_string);
}

void SpyBadWordWrite(unsigned int address)
{
	if (!Game)	return;

	if (!KConf.Spy)	return;
	if (!KConf.SpyRW)	return;

	wsprintf(spy_string, "%.5X want to write a Word at %.8X", main68k_context.pc, address);
	Spy_KMod(spy_string);
}


void SpyDMA()
{
	if (!Game)	return;

	if (!KConf.Spy)	return;

	if (!KConf.SpyDMA)	return;

	if (!(VDP_Reg.Set2 & 0x10))	return;

	switch (VDP_Reg.DMA_Src_Adr_H >> 6)
	{

	case 0x02:
		wsprintf(spy_string, "VRAM Fill of %d bytes into %0.4X", VDP_Reg.DMA_Lenght, Ctrl.Address & 0xFFFF);
		break;

	case 0x03:
		wsprintf(spy_string, "VRAM copy of %d words from %0.6X to %0.4X", VDP_Reg.DMA_Lenght, VDP_Reg.DMA_Address, Ctrl.Address & 0xFFFF);
		if ((VDP_Reg.DMA_Address / 0x020000) != ((VDP_Reg.DMA_Lenght + VDP_Reg.DMA_Address) / 0x020000))
			lstrcat(spy_string, "\r\nError : DMA must stay between a same 0x020000 block");
		break;

	case 0x00:
	case 0x01:
		// 68k -> VRAM
		wsprintf(spy_string, "@ (%d,%d) 68k (@ 0x%04X) -> VRAM of %d words from %0.6X to %0.4X",
			Read_VDP_H_Counter(),
			Read_VDP_V_Counter(),
			main68k_readPC(),
			VDP_Reg.DMA_Lenght,
			VDP_Reg.DMA_Address * 2,
			Ctrl.Address & 0xFFFF);

		switch (Ctrl.Access & 0x03)
		{
		case 0x00:
			lstrcat(spy_string, " (Wrong access?)");
			break;
		case 0x01:
			lstrcat(spy_string, " (VRAM access)");
			break;
		case 0x02:
			lstrcat(spy_string, " (CRAM access)");
			break;
		case 0x03:
			lstrcat(spy_string, " (VSRAM access)");
			break;
		}

		if ((VDP_Reg.DMA_Address / 0x020000) != ((VDP_Reg.DMA_Lenght + VDP_Reg.DMA_Address) / 0x020000))
			lstrcat(spy_string, "\r\nError : DMA must stay between a same 0x020000 block");
		break;
	default:
		wsprintf(spy_string, "Invalide DMA mode");
	}

	Spy_KMod(spy_string);
}


void SpyCDBiosCall()
{
	unsigned newPC;
	short int function;

	if (!Game)	return;

	if (!KConf.CDBios)	return;

	newPC = sub68k_readPC();
	if (newPC == 0x5f22)
	{
		function = (short int)(sub68k_context.dreg[0] & 0xFFFF);
		if ((function == 0x81) && KConf.noCDBSTAT)	return;

		if ((function == 0x8a) && KConf.noCDCSTAT)	return;

		//wsprintf(spy_string,"jumped at %.X: a0=%.8X a1=%.8X d0=%.4X d1=%.8X", newPC, sub68k_context.areg[0], sub68k_context.areg[1], sub68k_context.dreg[0]&0xFFFF, sub68k_context.dreg[1]);
		wsprintf(spy_string, "Call %s: a0=%.8X a1=%.8X d0=%.4X d1=%.8X", mcd_bios_name[function], sub68k_context.areg[0], sub68k_context.areg[1], sub68k_context.dreg[0] & 0xFFFF, sub68k_context.dreg[1]);
		Spy_KMod(spy_string);
	}
}