#ifndef VDP_32X_H
#define VDP_32X_H

#ifdef __cplusplus
extern "C" {
#endif


struct VDP_32X_Type
{
	unsigned int Mode; //Bitmap Mode (0x20004100) + ScreenShift (0x20004102)
	unsigned int State; // frame buffer control (0x2000410A)
	unsigned int AF_Data; // Auto fill data (0x20004108)
	unsigned int AF_St; // Auto fill start adress (0x20004106)
	unsigned int AF_Len; // Auto fill len (0x20004104)
	unsigned int AF_Line;
};

extern unsigned short _32X_Palette_16B[0x10000];
extern unsigned char _32X_VDP_Ram[0x100 * 1024];
extern unsigned short _32X_VDP_CRam[0x100];
extern unsigned short _32X_VDP_CRam_Ajusted[0x100];

extern struct VDP_32X_Type _32X_VDP;

void _32X_VDP_Reset();
void _32X_VDP_Draw(int FB_Num);

	
#ifdef __cplusplus
};
#endif

#endif