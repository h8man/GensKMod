#ifndef G_GDI_H
#define G_GDI_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

int Init_GDI(HWND hWnd);
int Flip_GDI(HWND hWnd);
void End_GDI();
int Clear_Back_Screen_GDI(HWND hWnd);
int Clear_Primary_Screen_GDI(HWND hWnd);

#ifdef __cplusplus
}
#endif

#endif /* G_GDI_H */

