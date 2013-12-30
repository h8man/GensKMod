#include "G_gdi.h"
#include "G_gfx.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "gens.h"

#include <stdint.h>

enum
{
    GFX_GDI_VIRTUAL_WIDTH = 1024,
    GFX_GDI_VIRTUAL_HEIGHT = 600
};

struct gfx_gdi
{
    uint16_t * back_buffer;
};

gfx_gdi gdi_instance = { 0 };

int Init_GDI(HWND hWnd)
{
    gdi_instance.back_buffer = new uint16_t[GFX_GDI_VIRTUAL_WIDTH * GFX_GDI_VIRTUAL_HEIGHT];

    return 0;
}

void End_GDI()
{
    delete[] gdi_instance.back_buffer;
}

int Flip_GDI(HWND hWnd)
{
    struct BMILOCAL
    {
        BITMAPINFOHEADER bmiHeader;
        DWORD            colors[3];
    };

    static const BMILOCAL bmi_back =
    {
        sizeof(BITMAPINFOHEADER),
        GFX_GDI_VIRTUAL_WIDTH,
        -GFX_GDI_VIRTUAL_HEIGHT,
        1,
        16,
        BI_BITFIELDS,
        0,
        250,
        250,
        0,
        0,
        0x0000F800,
        0x000007E0,
        0x0000001F
    };
    static const BMILOCAL bmi_md =
    {
        sizeof(BITMAPINFOHEADER),
        336,
        -224,
        1,
        16,
        BI_BITFIELDS,
        0,
        250,
        250,
        0,
        0,
        0x0000F800,
        0x000007E0,
        0x0000001F
    };

    BMILOCAL bmi = bmi_back;

    RECT rc_src;        // Source for future blit operation
    RECT rc_dest;       // Destination rectangle for future blit operation
    RECT rc_client;     // Client bounds
    RECT rc_vdp;        // VDP screen size
    int offset = 0;
    void * blit_src;
    PFN_BLIT pfnBlit = Full_Screen ? Blit_FS : Blit_W;
    HDC dc = ::GetDC(hWnd);

    RECT rc_left;
    RECT rc_right;
    RECT rc_top;
    RECT rc_bottom;

    ::GetClientRect(hWnd, &rc_client);
    dc = ::GetDC(hWnd);

    rc_vdp.left = 0;
    rc_vdp.top = 0;
    rc_vdp.right = (VDP_Reg.Set4 & 0x1) ? 320 : 256;
    rc_vdp.bottom = VDP_Num_Vis_Lines;
    offset = (VDP_Reg.Set4 & 0x1) ? 32 : 160;

    rc_src.left = 0;
    rc_src.top = 0;
    rc_src.right = rc_vdp.right;
    rc_src.bottom = VDP_Num_Vis_Lines;

    rc_dest.left = 0;
    rc_dest.top = 0;
    rc_dest.right = rc_vdp.right;
    rc_dest.bottom = rc_vdp.bottom;

    if (pfnBlit != 0)
    {
        pfnBlit((unsigned char *)gdi_instance.back_buffer,
            GFX_GDI_VIRTUAL_WIDTH * 2,
            rc_vdp.right,
            rc_vdp.bottom,
            offset);
        blit_src = gdi_instance.back_buffer;
        rc_src.right *= BlitScale_X;
        rc_src.bottom *= BlitScale_Y;
        rc_dest.right *= BlitScale_X;
        rc_dest.bottom *= BlitScale_Y;
        bmi = bmi_back;
        bmi.bmiHeader.biHeight = -rc_src.bottom;
    }
    else
    {
        blit_src = MD_Screen;
        bmi = bmi_md;
    }

    if (Stretch)
    {
        ::StretchDIBits(dc,
            rc_client.left,
            rc_client.top,
            rc_client.right - rc_client.left,
            rc_client.bottom - rc_client.top,
            0, 0,
            rc_src.right - rc_src.left,
            rc_src.bottom - rc_src.top,
            blit_src,
            (const BITMAPINFO *)&bmi,
            DIB_RGB_COLORS,
            SRCCOPY);
    }
    else
    {
        ::OffsetRect(&rc_dest,
            (rc_client.right - rc_dest.right) >> 1,
            (rc_client.bottom - rc_dest.bottom) >> 1);
        ::SetDIBitsToDevice(dc,
            rc_dest.left,
            rc_dest.top,
            rc_dest.right - rc_dest.left,
            rc_dest.bottom - rc_dest.top,
            rc_src.left,
            rc_src.top,
            0,
            rc_dest.bottom - rc_dest.top,
            blit_src,
            (const BITMAPINFO *)&bmi,
            DIB_RGB_COLORS);

        rc_left.left = rc_client.left;
        rc_left.right = rc_dest.left;
        rc_left.top = rc_client.top;
        rc_left.bottom = rc_client.bottom;

        rc_right.left = rc_dest.right;
        rc_right.right = rc_client.right;
        rc_right.top = rc_client.top;
        rc_right.bottom = rc_client.bottom;

        rc_top.left = rc_dest.left;
        rc_top.right = rc_dest.right;
        rc_top.top = rc_client.top;
        rc_top.bottom = rc_dest.top;

        rc_bottom.left = rc_dest.left;
        rc_bottom.right = rc_dest.right;
        rc_bottom.top = rc_dest.bottom;
        rc_bottom.bottom = rc_client.bottom;

        uint32_t color = ((uint16_t*)&CRam[0])[VDP_Reg.BG_Color];
        uint32_t t;

        t  = (color & 0x000F) << 0;
        t += (color & 0x00F0) << 4;
        t += (color & 0x0F00) << 8;
        t += (t << 4);
        t += ((t & 0x00888888) >> 3);

        HBRUSH hbr = ::CreateSolidBrush((COLORREF)t);
        // HBRUSH hbr = (HBRUSH)::GetStockObject(BLACK_BRUSH);
        ::FillRect(dc, &rc_left, hbr);
        ::FillRect(dc, &rc_right, hbr);
        ::FillRect(dc, &rc_top, hbr);
        ::FillRect(dc, &rc_bottom, hbr);
        ::DeleteObject(hbr);
    }

    ::ReleaseDC(hWnd, dc);

    return 0;
}

int Clear_Back_Screen_GDI(HWND hWnd)
{
    if (gdi_instance.back_buffer)
    {
        ::ZeroMemory(gdi_instance.back_buffer,
                     GFX_GDI_VIRTUAL_WIDTH * GFX_GDI_VIRTUAL_HEIGHT * sizeof(uint16_t));
    }

    return 0;
}

int Clear_Primary_Screen_GDI(HWND hWnd)
{
    Clear_Back_Screen_GDI(hWnd);

    return 0;
}
