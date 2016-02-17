#ifndef KMOD_SPY_H
#define KMOD_SPY_H


#ifdef __cplusplus
extern "C" {
#endif

void Spy_KMod(char *log);

void SpyReg( unsigned char a, unsigned char b);
void SpySpr( );
void SpyZ80NotPaused(unsigned int address);
void SpyZ80NotPaused_DMA(unsigned int address);
void SpyBadByteRead(unsigned int address);
void SpyBadWordRead(unsigned int address);
void SpyBadByteWrite(unsigned int address);
void SpyBadWordWrite(unsigned int address);

void SpyCDBiosCall();

#ifdef __cplusplus
};
#endif

#endif //KMOD_SPY_H