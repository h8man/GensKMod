#ifndef KMOD_MESSAGE_H
#define KMOD_MESSAGE_H


#ifdef __cplusplus
extern "C" {
#endif


BOOL Msg_KMod(char *msg);



void message_create(HINSTANCE hInstance, HWND hWndParent);
void message_show(BOOL visibility);
void message_update();
void message_reset();
void message_destroy();

void message_addChar(unsigned char b);

#ifdef __cplusplus
};
#endif

#endif //KMOD_MESSAGE_H