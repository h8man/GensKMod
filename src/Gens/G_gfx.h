#ifndef G_GFX
#define G_GFX

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int FS_VSync;
extern int W_VSync;
extern int Message_Style;
extern int Effect_Color;
extern int Blit_Soft;
extern int FPS_Style;
extern int Flag_Clr_Scr;
extern int Sleep_Time;
extern int Stretch;
extern clock_t Last_Time;
extern clock_t New_Time;
extern clock_t Used_Time;
extern int Kaillera_Error;
extern int Use_GDI;
extern int Render_FS;
extern int Render_W;
extern int Full_Screen;
extern int Always_On_Top;
extern int Show_Message;
extern int Show_FPS;
extern int Fast_Blur;

extern int(*Update_Frame)();
extern int(*Update_Frame_Fast)();

typedef void(*PFN_BLIT)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern PFN_BLIT Blit_FS;
extern PFN_BLIT Blit_W;

int Init_GFX(HWND HWnd);
int Flip_GFX(HWND hWnd);
void End_GFX();

void Put_Info(const char *Message, int Duree);
int Pause_Screen(void);
int Take_Shot();
int Clear_Primary_Screen(HWND hWnd);
int Update_Emulation(HWND hWnd);
int Update_Emulation_One(HWND hWnd);
int Update_Gens_Logo(HWND hWnd);
int Update_Crazy_Effect(HWND hWnd);
int Clear_Back_Screen(HWND hWnd);
void Restore_Primary(void);

#ifdef __cplusplus
enum GFX_BLIT_MODE
#else
typedef enum GFX_BLIT_MODE_t
#endif
{
    GFX_BLIT_1X,                        // 0
    GFX_BLIT_2X,                        // 1
    GFX_BLIT_INTERPOLATED,              // 2
    GFX_BLIT_SCANLINES,                 // 3
    GFX_BLIT_50PCT_SCANLINES,           // 4
    GFX_BLIT_25PCT_SCANLINES,           // 5
    GFX_BLIT_INTERP_SCANLINES,          // 6
    GFX_BLIT_INTERP_50PCT_SCANLINES,    // 7
    GFX_BLIT_INTERP_25PCT_SCANLINES,    // 8
    GFX_BLIT_2XSAI,                     // 9
}
#ifdef __cplusplus
;
#else
GFX_BLIT_MODE;
#endif

void GFX_SetBlit(HWND hWnd, GFX_BLIT_MODE mode);

extern int BlitScale_X;
extern int BlitScale_Y;

int Update_Emulation_Netplay(HWND hWnd, int player, int num_player);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* G_GFX */
