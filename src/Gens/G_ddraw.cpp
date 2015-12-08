#include <stdio.h>
#include <math.h>
#include "G_ddraw.h"
#include "G_gdi.h"
#include "G_gfx.h"
#include "G_dsound.h"
#include "G_Input.h"
#include "G_main.h"
#include "resource.h"
#include "gens.h"
#include "mem_M68K.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "misc.h"
#include "blit.h"
#include "scrshot.h"
#include "net.h"

#include "cdda_mp3.h"

LPDIRECTDRAW lpDD_Init;
LPDIRECTDRAW4 lpDD;
LPDIRECTDRAWSURFACE4 lpDDS_Primary;
LPDIRECTDRAWSURFACE4 lpDDS_Flip;
LPDIRECTDRAWSURFACE4 lpDDS_Back;
LPDIRECTDRAWSURFACE4 lpDDS_Blit;
LPDIRECTDRAWCLIPPER lpDDC_Clipper;

clock_t Last_Time = 0, New_Time = 0;
clock_t Used_Time = 0;

int Flag_Clr_Scr = 0;
int Sleep_Time;
int Stretch;
int Blit_Soft;
int Effect_Color = 7;
int FPS_Style = EMU_MODE | BLANC;
int Message_Style = EMU_MODE | BLANC | SIZE_X2;
int Kaillera_Error = 0;

int (*Update_Frame)();
int (*Update_Frame_Fast)();


int Init_Fail(HWND hwnd, char *err)
{
	End_DDraw();
	MessageBox(hwnd, err, "Oups ...", MB_OK);
	DestroyWindow(hwnd);

	return 0;
}


int Init_DDraw(HWND hWnd)
{
	int Rend;
	HRESULT rval;
	DDSURFACEDESC2 ddsd;

	End_DDraw();
	
	if (Full_Screen) Rend = Render_FS;
	else Rend = Render_W;

	if (DirectDrawCreate(NULL, &lpDD_Init, NULL) != DD_OK)
		return Init_Fail(hWnd, "Error with DirectDrawCreate !");

	if (lpDD_Init->QueryInterface(IID_IDirectDraw4, (LPVOID *) &lpDD) != DD_OK)
		return Init_Fail(hWnd, "Error with QueryInterface !\nUpgrade your DirectX version.");

	lpDD_Init->Release();
	lpDD_Init = NULL;

	if (!(Mode_555 & 2))
	{
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		lpDD->GetDisplayMode(&ddsd);

		if (ddsd.ddpfPixelFormat.dwGBitMask == 0x03E0) Mode_555 = 1;
		else Mode_555 = 0;

		Recalculate_Palettes();
	}

	if (Full_Screen)
		rval = lpDD->SetCooperativeLevel(hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	else
		rval = lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);

	if (rval != DD_OK)
		return Init_Fail(hWnd, "Error with lpDD->SetCooperativeLevel !");

	if (Full_Screen)
	{
		if (lpDD->SetDisplayMode(320 * ((Rend > 0)?2:1), 240 * ((Rend > 0)?2:1), 16, 0, 0) != DD_OK)
			return Init_Fail(hWnd, "Error with lpDD->SetDisplayMode !");
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	if ((Full_Screen) && (FS_VSync))
	{
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 2;
	}
	else
	{
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}

	if (lpDD->CreateSurface(&ddsd, &lpDDS_Primary, NULL ) != DD_OK)
		return Init_Fail(hWnd, "Error with lpDD->CreateSurface !");

	if (Full_Screen)
	{
	    if (FS_VSync)
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

			if (lpDDS_Primary->GetAttachedSurface(&ddsd.ddsCaps, &lpDDS_Flip) != DD_OK)
				return Init_Fail(hWnd, "Error with lpDDPrimary->GetAttachedSurface !");

			lpDDS_Blit = lpDDS_Flip;
		}
		else lpDDS_Blit = lpDDS_Primary;
	}
	else
	{
		if (lpDD->CreateClipper(0, &lpDDC_Clipper, NULL ) != DD_OK)
			return Init_Fail(hWnd, "Error with lpDD->CreateClipper !");

		if (lpDDC_Clipper->SetHWnd(0, hWnd) != DD_OK)
			return Init_Fail(hWnd, "Error with lpDDC_Clipper->SetHWnd !");

		if (lpDDS_Primary->SetClipper(lpDDC_Clipper) != DD_OK)
			return Init_Fail(hWnd, "Error with lpDDS_Primary->SetClipper !");
	}

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;

	if (Rend < 2)
	{
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		ddsd.dwWidth = 336;
		ddsd.dwHeight = 240;
	}
	else
	{
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		ddsd.dwWidth = 640;
		ddsd.dwHeight = 480;
	}

	if (lpDD->CreateSurface(&ddsd, &lpDDS_Back, NULL) != DD_OK)
		return Init_Fail(hWnd, "Error with lpDD->CreateSurface !");

	if (!Full_Screen) lpDDS_Blit = lpDDS_Back;

	if (Rend < 2)
	{
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		if (lpDDS_Back->GetSurfaceDesc(&ddsd) != DD_OK)
			return Init_Fail(hWnd, "Error with lpDD_Back->GetSurfaceDesc !");

		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT;
		ddsd.dwWidth = 336;
		ddsd.dwHeight = 240;
		ddsd.lPitch = 336 * 2;
		ddsd.lpSurface = &MD_Screen[0];

		if (lpDDS_Back->SetSurfaceDesc(&ddsd, 0) != DD_OK)
			return Init_Fail(hWnd, "Error with lpDD_Back->SetSurfaceDesc !");
	}

	return 1;
}

void End_DDraw()
{
	if (lpDDC_Clipper)
	{
		lpDDC_Clipper->Release();
		lpDDC_Clipper = NULL;
	}

	if (lpDDS_Back)
	{
		lpDDS_Back->Release();
		lpDDS_Back = NULL;
	}

	if (lpDDS_Flip)
	{
		lpDDS_Flip->Release();
		lpDDS_Flip = NULL;
	}

	if (lpDDS_Primary)
	{
		lpDDS_Primary->Release();
		lpDDS_Primary = NULL;
	}

	if (lpDD)
	{
		lpDD->SetCooperativeLevel(HWnd, DDSCL_NORMAL);
		lpDD->Release();
		lpDD = NULL;
	}

	lpDDS_Blit = NULL;
}


HRESULT RestoreGraphics()
{
	HRESULT rval;

	rval = lpDDS_Primary->Restore();
	rval = lpDDS_Back->Restore();

	return rval;
}


int Clear_Primary_Screen_DDraw(HWND hWnd)
{
	DDSURFACEDESC2 ddsd;
	DDBLTFX ddbltfx;
	RECT RD;
	POINT p;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = 0;

	if (Full_Screen)
	{
		if (FS_VSync)
		{
			lpDDS_Flip->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);

			lpDDS_Flip->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);

			lpDDS_Flip->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
		}
		else lpDDS_Primary->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	}
	else
	{
		p.x = p.y = 0;
		GetClientRect(hWnd, &RD);
		ClientToScreen(hWnd, &p);

		RD.left = p.x;
		RD.top = p.y;
		RD.right += p.x;
		RD.bottom += p.y;

		if (RD.top < RD.bottom)
			lpDDS_Primary->Blt(&RD, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	}

	return 1;
}


int Clear_Back_Screen_DDraw(HWND hWnd)
{
	DDSURFACEDESC2 ddsd;
	DDBLTFX ddbltfx;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = 0;

	lpDDS_Back->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);

	return 1;
}


void Restore_Primary(void)
{
	if (lpDD && Full_Screen && FS_VSync)
	{
		while (lpDDS_Primary->GetFlipStatus(DDGFS_ISFLIPDONE) != DD_OK);
		lpDD->FlipToGDISurface();
	}
}


int Flip_DDraw(HWND hWnd)
{
	HRESULT rval;
	DDSURFACEDESC2 ddsd;
	RECT RectDest, RectSrc;
	POINT p;
	float Ratio_X, Ratio_Y;
	int Dep;

	ddsd.dwSize = sizeof(ddsd);

	if (Full_Screen)
	{
		if ((VDP_Reg.Set4 & 0x1) || (Debug))
		{
			if (Flag_Clr_Scr != 40)
			{
				Clear_Primary_Screen(hWnd);
				Clear_Back_Screen(hWnd);
				Flag_Clr_Scr = 40;
			}

			Dep = 0;
			RectSrc.left = 0 + 8;
			RectSrc.right = 320 + 8;
			RectDest.left = 0;
			RectDest.right = 320 << Render_FS;
		}
		else
		{
			if (Flag_Clr_Scr != 32)
			{
				Clear_Primary_Screen(hWnd);
				Clear_Back_Screen(hWnd);
				Flag_Clr_Scr = 32;
			}

			Dep = 64;
			RectSrc.left = 0 + 8;
			RectSrc.right = 256 + 8;

			if (Stretch)
			{
				RectDest.left = 0;
				RectDest.right = 320 << Render_FS;
			}
			else
			{
				RectDest.left = 32 << Render_FS;
				RectDest.right = (256 + 32) << Render_FS;
			}
		}

		if (Render_FS < 2)
		{
			if (Blit_Soft == 1)
			{
				rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

				if (Render_FS == 0)
					Blit_FS((unsigned char *) ddsd.lpSurface + ((ddsd.lPitch * (240 - VDP_Num_Vis_Lines) >> 1) + Dep), ddsd.lPitch, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);
				else
					Blit_FS((unsigned char *) ddsd.lpSurface + ((ddsd.lPitch * ((240 - VDP_Num_Vis_Lines) >> 1) + Dep) << 1), ddsd.lPitch, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);

				lpDDS_Blit->Unlock(NULL);

				if (FS_VSync)
				{
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
			}
			else
			{
				RectSrc.top = 0;
				RectSrc.bottom = VDP_Num_Vis_Lines;

				if ((VDP_Num_Vis_Lines == 224) && (Stretch == 0))
				{
					RectDest.top = 8 << Render_FS;
					RectDest.bottom = (224 + 8) << Render_FS;
				}
				else
				{
					RectDest.top = 0;
					RectDest.bottom = 240 << Render_FS;
				}

				if (FS_VSync)
				{
					lpDDS_Flip->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
				else
				{
					lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
//					lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
				}
			}
		}
		else
		{
			rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

			if (rval != DD_OK) return 1;	// Assuming surface busy ...

			Blit_FS((unsigned char *) ddsd.lpSurface + ((ddsd.lPitch * ((240 - VDP_Num_Vis_Lines) >> 1) + Dep) << 1), ddsd.lPitch, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);

			lpDDS_Blit->Unlock(NULL);

			if (FS_VSync)
			{
				lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
			}
		}
	}
	else // not FullScreen
	{
/* KANEDA_BUG
     if DestScreen outside screen, bad refresh
	 need to create  Adjust.Left, Adjust.Right, Adjust.Top, Adjust.Bottom to add to DestScr 
 */
		p.x = p.y = 0;
		GetClientRect(hWnd, &RectDest);
		ClientToScreen(hWnd, &p);

		Ratio_X = (float) RectDest.right;
		Ratio_Y = (float) RectDest.bottom;
		Ratio_X /= 320.0;
		Ratio_Y /= 240.0;

		RectDest.left = p.x;
		RectDest.top = p.y;
		RectDest.right += p.x;
		RectDest.bottom += p.y;

		if (Render_W < 2)
		{
			RectSrc.top = 0;
			RectSrc.bottom = VDP_Num_Vis_Lines;

			if ((VDP_Num_Vis_Lines == 224) && (Stretch == 0))
			{
				RectDest.top += (int) (8 * Ratio_Y);
				RectDest.bottom -= (int) (8 * Ratio_Y);
			}
		}
		else
		{
			if (VDP_Num_Vis_Lines == 224)
			{
				RectSrc.top =  8 * 2;
				RectSrc.bottom = (224 + 8) * 2;

				if (Stretch == 0)
				{
					RectDest.top += (int) (8 * Ratio_Y);
					RectDest.bottom -= (int) (8 * Ratio_Y);
				}
			}
			else
			{
				RectSrc.top = 0 * 2;
				RectSrc.bottom = 240 * 2;
			}
		}

		if ((VDP_Reg.Set4 & 0x1) || (Debug))
		{
			Dep = 0;

			if (Flag_Clr_Scr != 40)
			{
				Clear_Primary_Screen(hWnd);
				Clear_Back_Screen(hWnd);
				Flag_Clr_Scr = 40;
			}

			if (Render_W < 2)
			{
				RectSrc.left = 8 + 0 ;
				RectSrc.right = 8 + 320;
			}
			else
			{
				RectSrc.left = 0 * 2;
				RectSrc.right = 320 * 2;
			}
		}
		else
		{
			Dep = 64;

			if (Stretch == 0)
			{
				RectDest.left += int(32 * Ratio_X);
				RectDest.right -= int(32 * Ratio_X);
			}

			if (Flag_Clr_Scr != 32)
			{
				Flag_Clr_Scr = 32;
				Clear_Primary_Screen(hWnd);
				Clear_Back_Screen(hWnd);
			}

			if (Render_W < 2)
			{
				RectSrc.left = 8 + 0;
				RectSrc.right = 8 + 256;
			}
			else
			{
				RectSrc.left = 32 * 2;
				RectSrc.right = (256 * 2) + (32 * 2);
			}
		}

		if (Render_W >= 2)
		{
			rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

			if (rval != DD_OK) return 1;	// Assuming surface busy ...

			Blit_W((unsigned char *) ddsd.lpSurface + ((ddsd.lPitch * ((240 - VDP_Num_Vis_Lines) >> 1) + Dep) << 1), ddsd.lPitch, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);

			lpDDS_Blit->Unlock(NULL);
		}
	
		if (RectDest.top < RectDest.bottom)
		{
			if (W_VSync)
			{
				int vb;

				lpDD->GetVerticalBlankStatus(&vb);

				if (!vb) lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
			}

			rval = lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
//			rval = lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
		}
	}

	if (rval == DDERR_SURFACELOST) rval = RestoreGraphics();

	return 1;
}


int Update_Gens_Logo(HWND hWnd)
{
	int i, j, m, n;
	static short tab[64000], Init = 0;
	static float renv = 0, ang = 0, zoom_x = 0, zoom_y = 0, pas;
	unsigned short c;

	if (!Init)
	{
		HBITMAP Logo;

		Logo = LoadBitmap(ghInstance,  MAKEINTRESOURCE(IDB_LOGO_BIG));
		GetBitmapBits(Logo, 64000 * 2, tab);
		pas = 0.05f;
		Init = 1;
	}

	renv += pas;
	zoom_x = sin(renv);
	if (zoom_x == 0.0f) zoom_x = 0.0000001f;
	zoom_x = (1 / zoom_x) * 1;
	zoom_y = 1;

	if (VDP_Reg.Set4 & 0x1)
	{
		for(j = 0; j < 240; j++)
		{
			for(i = 0; i < 320; i++)
			{
				m = (float)(i - 160) * zoom_x;
				n = (float)(j - 120) * zoom_y;

				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5)) MD_Screen[TAB336[j] + i + 8] = c;
				}
			}
		}
	}
	else
	{
		for(j = 0; j < 240; j++)
		{
			for(i = 0; i < 256; i++)
			{
				m = (float)(i - 128) * zoom_x;
				n = (float)(j - 120) * zoom_y;

				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5)) MD_Screen[TAB336[j] + i + 8] = c;
				}
			}
		}
	}

	Half_Blur();
    Flip_GFX(hWnd);

	return 1;
}


int Update_Crazy_Effect(HWND hWnd)
{
	int i, j, offset;
	int r = 0, v = 0, b = 0, prev_l, prev_p;
	int RB, G;

 	for(offset = 336 * 240, j = 0; j < 240; j++)
	{
		for(i = 0; i < 336; i++, offset--)
		{
			prev_l = MD_Screen[offset - 336];
			prev_p = MD_Screen[offset - 1];

			if (Mode_555 & 1)
			{
				RB = ((prev_l & 0x7C1F) + (prev_p & 0x7C1F)) >> 1;
				G = ((prev_l & 0x03E0) + (prev_p & 0x03E0)) >> 1;

				if (Effect_Color & 0x4)
				{
					r = RB & 0x7C00;
					if (rand() > 0x2C00) r += 0x0400;
					else r -= 0x0400;
					if (r > 0x7C00) r = 0x7C00;
					else if (r < 0x0400) r = 0;
				}

				if (Effect_Color & 0x2)
				{
					v = G & 0x03E0;
					if (rand() > 0x2C00) v += 0x0020;
					else v -= 0x0020;
					if (v > 0x03E0) v = 0x03E0;
					else if (v < 0x0020) v = 0;
				}

				if (Effect_Color & 0x1)
				{
					b = RB & 0x001F;
					if (rand() > 0x2C00) b++;
					else b--;
					if (b > 0x1F) b = 0x1F;
					else if (b < 0) b = 0;
				}
			}
			else
			{
				RB = ((prev_l & 0xF81F) + (prev_p & 0xF81F)) >> 1;
				G = ((prev_l & 0x07C0) + (prev_p & 0x07C0)) >> 1;

				if (Effect_Color & 0x4)
				{
					r = RB & 0xF800;
					if (rand() > 0x2C00) r += 0x0800;
					else r -= 0x0800;
					if (r > 0xF800) r = 0xF800;
					else if (r < 0x0800) r = 0;
				}

				if (Effect_Color & 0x2)
				{
					v = G & 0x07C0;
					if (rand() > 0x2C00) v += 0x0040;
					else v -= 0x0040;
					if (v > 0x07C0) v = 0x07C0;
					else if (v < 0x0040) v = 0;
				}

				if (Effect_Color & 0x1)
				{
					b = RB & 0x001F;
					if (rand() > 0x2C00) b++;
					else b--;
					if (b > 0x1F) b = 0x1F;
					else if (b < 0) b = 0;
				}
			}

			MD_Screen[offset] = r + v + b;
		}
	}

	Flip_GFX(hWnd);

	return 1;
}


int Update_Emulation(HWND hWnd)
{
	static int Over_Time = 0;
	int current_div;

	if (Frame_Skip != -1)
	{
		if (Sound_Enable)
		{
			WP = (WP + 1) & (Sound_Segs - 1);

			if (WP == Get_Current_Seg())
				WP = (WP + Sound_Segs - 1) & (Sound_Segs - 1);

			Write_Sound_Buffer(NULL);
		}

		Update_Controllers();

		if (Frame_Number++ < Frame_Skip)
		{
			Update_Frame_Fast();
		}
		else
		{
			Frame_Number = 0;
			Update_Frame();
			Flip_GFX(hWnd);
		}
	}
	else
	{
		if (Sound_Enable)
		{
			while (WP == Get_Current_Seg()) Sleep(Sleep_Time);
			
			RP = Get_Current_Seg();

			while (WP != RP)
			{
				Write_Sound_Buffer(NULL);
				WP = (WP + 1) & (Sound_Segs - 1);
				Update_Controllers();

				if (WP != RP)
				{
					Update_Frame_Fast();
				}
				else
				{
					Update_Frame();
                    Flip_GFX(hWnd);
				}
			}
		}
		else
		{
			if (CPU_Mode) current_div = 20;
			else current_div = 16 + (Over_Time ^= 1);

			New_Time = GetTickCount();
			Used_Time += (New_Time - Last_Time);
			Frame_Number = Used_Time / current_div;
			Used_Time %= current_div;
			Last_Time = New_Time;

			if (Frame_Number > 8) Frame_Number = 8;

			for (; Frame_Number > 1; Frame_Number--)
			{
				Update_Controllers();
				Update_Frame_Fast();
			}

			if (Frame_Number)
			{
				Update_Controllers();
				Update_Frame();
                Flip_GFX(hWnd);
			}
			else Sleep(Sleep_Time);
		}
	}

	return 1;
}


int Update_Emulation_One(HWND hWnd)
{
	Update_Controllers();
	Update_Frame();
    Flip_GFX(hWnd);

	return 1;
}


int Update_Emulation_Netplay(HWND hWnd, int player, int num_player)
{
	static int Over_Time = 0;
	int current_div;

	if (CPU_Mode) current_div = 20;
	else current_div = 16 + (Over_Time ^= 1);

	New_Time = GetTickCount();
	Used_Time += (New_Time - Last_Time);
	Frame_Number = Used_Time / current_div;
	Used_Time %= current_div;
	Last_Time = New_Time;

	if (Frame_Number > 6) Frame_Number = 6;

	for (; Frame_Number > 1; Frame_Number--)
	{
		if (Sound_Enable)
		{
			if (WP == Get_Current_Seg()) WP = (WP - 1) & (Sound_Segs - 1);
			Write_Sound_Buffer(NULL);
			WP = (WP + 1) & (Sound_Segs - 1);
		}

		Scan_Player_Net(player);
		if (Kaillera_Error != -1) Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		//Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		Update_Controllers_Net(num_player);
		Update_Frame_Fast();
	}

	if (Frame_Number)
	{
		if (Sound_Enable)
		{
			if (WP == Get_Current_Seg()) WP = (WP - 1) & (Sound_Segs - 1);
			Write_Sound_Buffer(NULL);
			WP = (WP + 1) & (Sound_Segs - 1);
		}

		Scan_Player_Net(player);
		if (Kaillera_Error != -1) Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		//Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		Update_Controllers_Net(num_player);
		Update_Frame();
        Flip_GFX(hWnd);
	}

	return 1;
}


int Eff_Screen(void)
{
	int i;

	for(i = 0; i < 336 * 240; i++) MD_Screen[i] = 0;

	return 1;
}


int Pause_Screen(void)
{
	int i, j, offset;
	int r, v, b, nr, nv, nb;

#ifdef GENS_KMOD
	if ( KConf.bBluePause == 0)		return 1;
#endif

	r = v = b = nr = nv = nb = 0;

	for(offset = j = 0; j < 240; j++)
	{
		for(i = 0; i < 336; i++, offset++)
		{
			if (Mode_555 & 1)
			{
				r = (MD_Screen[offset] & 0x7C00) >> 10;
				v = (MD_Screen[offset] & 0x03E0) >> 5;
				b = (MD_Screen[offset] & 0x001F);
			}
			else
			{
				r = (MD_Screen[offset] & 0xF800) >> 11;
				v = (MD_Screen[offset] & 0x07C0) >> 6;
				b = (MD_Screen[offset] & 0x001F);
			}

			nr = nv = nb = (r + v + b) / 3;
			
			if ((nb <<= 1) > 31) nb = 31;

			nr &= 0x1E;
			nv &= 0x1E;
			nb &= 0x1E;

			if (Mode_555 & 1)
				MD_Screen[offset] = (nr << 10) + (nv << 5) + nb;
			else
				MD_Screen[offset] = (nr << 11) + (nv << 6) + nb;
		}
	}

	return 1;
}


int Show_Genesis_Screen(HWND hWnd)
{
	Do_VDP_Only();
    Flip_GFX(hWnd);

	return 1;
}


int Take_Shot()
{
	HRESULT rval;
	DDSURFACEDESC2 ddsd;
	RECT RD;
	POINT p;
	
/* KANEDA_BUG
	2 screenshots => the 2nd will have "Screen shot x saved" 
	need to clear the screen before
*/
	
	Put_Info(" ", 10);
	//Do_VDP_Only();
    Flip_GFX(HWnd);
	
/***********************/

	if (Full_Screen)
	{
		if (Render_FS == 0)
		{
			if ((Stretch) || (VDP_Reg.Set2 & 0x08))
			{
				RD.top = 0;
				RD.bottom = 240;
			}
			else
			{
				RD.top = 8;
				RD.bottom = 224 + 8;
			}
			if ((Stretch) || (VDP_Reg.Set4 & 0x01))
			{
				RD.left = 0;
				RD.right = 320;
			}
			else
			{
				RD.left = 32;
				RD.right = 256 + 32;
			}
		}
		else if (Render_FS == 1)
		{
			if ((Stretch) || (VDP_Reg.Set2 & 0x08))
			{
				RD.top = 0;
				RD.bottom = 480;
			}
			else
			{
				RD.top = 16;
				RD.bottom = 448 + 16;
			}
			if ((Stretch) || (VDP_Reg.Set4 & 0x01))
			{
				RD.left = 0;
				RD.right = 640;
			}
			else
			{
				RD.left = 64;
				RD.right = 512 + 64;
			}
		}
		else
		{
			if (VDP_Reg.Set2 & 0x08)
			{
				RD.top = 0;
				RD.bottom = 480;
			}
			else
			{
				RD.top = 16;
				RD.bottom = 448 + 16;
			}
			if (VDP_Reg.Set4 & 0x01)
			{
				RD.left = 0;
				RD.right = 640;
			}
			else
			{
				RD.left = 64;
				RD.right = 512 + 64;
			}
		}
	}
	else
	{
		p.x = p.y = 0;
		GetClientRect(HWnd, &RD);
		ClientToScreen(HWnd, &p);

		RD.top = p.y;
		RD.left = p.x;
		RD.bottom += p.y;
		RD.right += p.x;

		if (Render_W == 0)
		{
			if ((!Stretch) && ((VDP_Reg.Set2 & 0x08) == 0))
			{
				RD.top += 8;
				RD.bottom -= 8;
			}
			if ((!Stretch) && ((VDP_Reg.Set4 & 0x01) == 0))
			{
				RD.left += 32;
				RD.right -= 32;
			}

		}
		else
		{
			if ((!Stretch) && ((VDP_Reg.Set2 & 0x08) == 0))
			{
				RD.top += 16;
				RD.bottom -= 16;
			}
			if ((!Stretch) && ((VDP_Reg.Set4 & 0x01) == 0))
			{
				RD.left += 64;
				RD.right -= 64;
			}
		}
	}

	if (Use_GDI)
	{
		SaveGDI(RD);
	}
	else
	{
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		rval = lpDDS_Primary->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

		if (rval == DD_OK)
		{
			Save_Shot((unsigned char *) ddsd.lpSurface + (RD.top * ddsd.lPitch) + (RD.left * 2), Mode_555 & 1, (RD.right - RD.left), (RD.bottom - RD.top), ddsd.lPitch);
			lpDDS_Primary->Unlock(NULL);
			return 1;
		}
		else return 0;
	}
}







/*
void MP3_init_test()
{
	FILE *f;

	f = fopen("\\vc\\gens\\release\\test.mp3", "rb");
	
	if (f == NULL) return;
	
	MP3_Test(f);

	Play_Sound();
}


void MP3_update_test()
{
	int *buf[2];
		
	buf[0] = Seg_L;
	buf[1] = Seg_R;

	while (WP == Get_Current_Seg());
			
	RP = Get_Current_Seg();

	while (WP != RP)
	{
		Write_Sound_Buffer(NULL);
		WP = (WP + 1) & (Sound_Segs - 1);

		memset(Seg_L, 0, (Seg_Lenght << 2));
		memset(Seg_R, 0, (Seg_Lenght << 2));
		MP3_Update(buf, Seg_Lenght);
	}
}
*/