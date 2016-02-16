#ifndef KMOD_COMMON_H
#define KMOD_COMMON_H

extern ULONG	timer_KMod;

//TODO remove !
extern UCHAR OpenedWindow_KMod[];
extern HWND  HandleWindow_KMod[];

#ifdef __cplusplus
extern "C" {
#endif

//TODO revamp
void IncTimer_KMod(unsigned odom);

//TODO rename
void OpenWindow_KMod(UCHAR mode);
void CloseWindow_KMod(UCHAR mode);


#ifdef __cplusplus
};
#endif

#endif //KMOD_COMMON_H