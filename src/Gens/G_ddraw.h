#ifndef G_DDRAW_H
#define G_DDRAW_H

#define DIRECTDRAW_VERSION         0x0500

#include <ddraw.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

int Init_Fail(HWND hwnd);
int Init_DDraw(HWND hWnd);
int Eff_Screen(void);
int Show_Genesis_Screen(HWND hWnd);
int Clear_Primary_Screen_DDraw(HWND hWnd);
int Clear_Back_Screen_DDraw(HWND hWnd);
int Flip_DDraw(HWND hWnd);
void End_DDraw(void);

//void MP3_update_test();
//void MP3_init_test();

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif