#ifndef KMOD_UTILS_H
#define KMOD_UTILS_H


#ifdef __cplusplus
extern "C" {
#endif

void Hexview(unsigned char *addr, unsigned char *dest);
void Ansiview(unsigned char *addr, unsigned char *dest);

#ifdef __cplusplus
};
#endif

#endif //KMOD_UTILS_H