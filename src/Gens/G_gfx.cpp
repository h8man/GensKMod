#include "G_gfx.h"

#include "G_ddraw.h"
#include "G_gdi.h"
#include "Misc.h"
#include "blit.h"

#include <stdio.h>

static char Info_String[1024] = "";
static unsigned int Info_Time = 0;
static int Message_Showed = 0;

extern "C"
{

int Use_GDI = 1;
int FS_VSync;
int W_VSync;
int BlitScale_X = 1;
int BlitScale_Y = 1;
PFN_BLIT Blit_FS;
PFN_BLIT Blit_W;

void Put_Info(const char *Message, int Duree)
{
    if (Show_Message)
    {
        strcpy(Info_String, Message);
        Info_Time = GetTickCount() + Duree;
        Message_Showed = 1;
    }
}

int Init_GFX(HWND HWnd)
{
    if (Use_GDI)
    {
        return Init_GDI(HWnd);
    }
    else
    {
        return Init_DDraw(HWnd);
    }
}

int Flip_GFX(HWND hWnd)
{
    static float FPS = 0.0f, frames[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    static unsigned int old_time = 0, view_fps = 0, index_fps = 0, freq_cpu[2] = { 0, 0 };
    unsigned int new_time[2];
    int i;

    if (Fast_Blur) Half_Blur();

    if (Message_Showed)
    {
        if (GetTickCount() > Info_Time)
        {
            Message_Showed = 0;
            strcpy(Info_String, "");
        }
        else Print_Text(Info_String, strlen(Info_String), 10, 210, Message_Style);

    }
    else if (Show_FPS)
    {
        if (freq_cpu[0] > 1)				// accurate timer ok
        {
            if (++view_fps >= 16)
            {
                QueryPerformanceCounter((union _LARGE_INTEGER *) new_time);

                if (new_time[0] != old_time)
                {
                    FPS = (float)(freq_cpu[0]) * 16.0f / (float)(new_time[0] - old_time);
                    sprintf(Info_String, "%.1f", FPS);
                }
                else
                {
                    sprintf(Info_String, "too much...");
                }

                old_time = new_time[0];
                view_fps = 0;
            }
        }
        else if (freq_cpu[0] == 1)			// accurate timer not supported
        {
            if (++view_fps >= 10)
            {
                new_time[0] = GetTickCount();

                if (new_time[0] != old_time) frames[index_fps] = 10000 / (float)(new_time[0] - old_time);
                else frames[index_fps] = 2000;

                index_fps++;
                index_fps &= 7;
                FPS = 0.0f;

                for (i = 0; i < 8; i++) FPS += frames[i];

                FPS /= 8.0f;
                old_time = new_time[0];
                view_fps = 0;
            }

            sprintf(Info_String, "%.1f", FPS);
        }
        else
        {
            QueryPerformanceFrequency((union _LARGE_INTEGER *) freq_cpu);
            if (freq_cpu[0] == 0) freq_cpu[0] = 1;

            sprintf(Info_String, "", FPS);
        }

        Print_Text(Info_String, strlen(Info_String), 10, 210, FPS_Style);
    }

    if (Use_GDI)
    {
        return Flip_GDI(hWnd);
    }
    else
    {
        return Flip_DDraw(hWnd);
    }
}

void End_GFX(void)
{
    if (Use_GDI)
    {
        End_GDI();
    }
    else
    {
        End_DDraw();
    }
}

int Clear_Primary_Screen(HWND hWnd)
{
    if (Use_GDI)
    {
        return Clear_Primary_Screen_GDI(hWnd);
    }
    else
    {
        return Clear_Primary_Screen_DDraw(hWnd);
    }
}


int Clear_Back_Screen(HWND hWnd)
{
    if (Use_GDI)
    {
        return Clear_Back_Screen_GDI(hWnd);
    }
    else
    {
        return Clear_Back_Screen_DDraw(hWnd);
    }
}

void GFX_SetBlit(HWND hWnd, GFX_BLIT_MODE mode)
{
    typedef void (* PFN_BLIT)(unsigned char*, int, int, int, int);

    static const PFN_BLIT blit_funcs[] =
    {
        Blit_X1_MMX,                    // GFX_BLIT_1X,
        Blit_X2_MMX,                    // GFX_BLIT_2X,
        Blit_X2_Int_MMX,                // GFX_BLIT_INTERPOLATED,
        Blit_Scanline_MMX,              // GFX_BLIT_SCANLINES,
        Blit_Scanline_50_MMX,           // GFX_BLIT_50PCT_SCANLINES,
        Blit_Scanline_25_MMX,           // GFX_BLIT_25PCT_SCANLINES,
        Blit_Scanline_Int_MMX,          // GFX_BLIT_INTERP_SCANLINES,
        Blit_Scanline_50_Int_MMX,       // GFX_BLIT_INTERP_50PCT_SCANLINES,
        Blit_Scanline_25_Int_MMX,       // GFX_BLIT_INTERP_25PCT_SCANLINES,
        Blit_2xSAI_MMX,                 // GFX_BLIT_2XSAI,
    };

    static const int post_blit_scale_x[] =
    {
        1, // GFX_BLIT_1X,
        2, // GFX_BLIT_2X,
        2, // GFX_BLIT_INTERPOLATED,
        2, // GFX_BLIT_SCANLINES,
        2, // GFX_BLIT_50PCT_SCANLINES,
        2, // GFX_BLIT_25PCT_SCANLINES,
        2, // GFX_BLIT_INTERP_SCANLINES,
        2, // GFX_BLIT_INTERP_50PCT_SCANLINES,
        2, // GFX_BLIT_INTERP_25PCT_SCANLINES,
        2, // GFX_BLIT_2XSAI,
    };

    static const int post_blit_scale_y[] =
    {
        1, // GFX_BLIT_1X,
        2, // GFX_BLIT_2X,
        2, // GFX_BLIT_INTERPOLATED,
        2, // GFX_BLIT_SCANLINES,
        2, // GFX_BLIT_50PCT_SCANLINES,
        2, // GFX_BLIT_25PCT_SCANLINES,
        2, // GFX_BLIT_INTERP_SCANLINES,
        2, // GFX_BLIT_INTERP_50PCT_SCANLINES,
        2, // GFX_BLIT_INTERP_25PCT_SCANLINES,
        2, // GFX_BLIT_2XSAI,
    };

    BlitScale_X = post_blit_scale_x[mode];
    BlitScale_Y = post_blit_scale_y[mode];

    Clear_Back_Screen(hWnd);
}

}
