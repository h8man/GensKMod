#ifndef KMOD_GMV_H
#define KMOD_GMV_H


#ifdef __cplusplus
extern "C" {
#endif

void GMVRecord_KMod();
void GMVStop_KMod();
void GMVPlay_KMod();

void GMVUpdatePlay_KMod();
void GMVUpdateRecord_KMod();

void GMVTools_KMod();

#ifdef __cplusplus
};
#endif

#endif //KMOD_GMV_H