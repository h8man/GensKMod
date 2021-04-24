#ifndef KMOD_UTILS_H
#define KMOD_UTILS_H


#ifdef __cplusplus
extern "C" {
#endif

void Hexview(unsigned char *addr, unsigned char *dest);
void Ansiview(unsigned char *addr, unsigned char *dest);

void drawTile(HDC hDCMain, unsigned short int numTile, WORD x, WORD y, UCHAR pal, UCHAR zoom);

int CopyToClipboard(int Type, unsigned char* Buffer, size_t buflen, BOOL clear); // feos added this

#ifdef __cplusplus
};
#endif

#endif //KMOD_UTILS_H