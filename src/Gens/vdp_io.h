#ifdef __cplusplus
extern "C" {
#endif

struct Reg_VDP_Type {
	unsigned int Set1;
	unsigned int Set2;
	unsigned int Pat_ScrA_Adr;
	unsigned int Pat_Win_Adr;
	unsigned int Pat_ScrB_Adr;
	unsigned int Spr_Att_Adr;
	unsigned int Reg6;
	unsigned int BG_Color;
	unsigned int Reg8;
	unsigned int Reg9;
	unsigned int H_Int;
	unsigned int Set3;
	unsigned int Set4;
	unsigned int H_Scr_Adr;
	unsigned int Reg14;
	unsigned int Auto_Inc;
	unsigned int Scr_Size;
	unsigned int Win_H_Pos;
	unsigned int Win_V_Pos;
	unsigned int DMA_Lenght_L;
	unsigned int DMA_Lenght_H;
	unsigned int DMA_Src_Adr_L;
	unsigned int DMA_Src_Adr_M;
	unsigned int DMA_Src_Adr_H;
	unsigned int DMA_Lenght;
	unsigned int DMA_Address;
};

extern unsigned char VRam[64000];
extern unsigned char CRam[256]; /* CRAM is in fact 64 dword, but for save/load, it's easier by char */
extern unsigned char VSRam[256];
extern unsigned char H_Counter_Table[512 * 2];
extern unsigned int Spr_Link[256];
extern int Genesis_Started;
extern int SegaCD_Started;
extern int _32X_Started;

extern struct Reg_VDP_Type VDP_Reg;
extern int VDP_Current_Line;
extern int VDP_Num_Lines;
extern int VDP_Num_Vis_Lines;
extern int CRam_Flag;
extern int VRam_Flag;
extern int VDP_Int;
extern int VDP_Status;
extern int DMAT_Lenght;
extern struct {
	int Flag;	//0 : Word 1, 1: Word 2
	int Data;
	int Write;
	int Access;
	int Address;
	int DMA_Mode;
	int DMA;
} Ctrl;


extern int H_Cell;


void Reset_VDP(void);
unsigned int Update_DMA(void);
unsigned short Read_VDP_Data(void);
unsigned short Read_VDP_Status(void);
unsigned char Read_VDP_H_Counter(void);
unsigned char Read_VDP_V_Counter(void);
int Write_Low_Byte_VDP_Data(unsigned char Data);
int Write_High_Byte_VDP_Data(unsigned char Data);
int Write_Word_VDP_Data(unsigned short Data);
int Write_VDP_Ctrl(unsigned short Data);
int Set_VDP_Reg(int Num_Reg, int Val);
void Update_IRQ_Line(void);

#ifdef __cplusplus
};
#endif
