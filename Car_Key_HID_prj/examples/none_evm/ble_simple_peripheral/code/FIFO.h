
#ifndef _FIFO_H
#define	_FIFO_H

#include <stdint.h>
 
typedef enum{
	FIFO_INDEX0 = 0,                  
    FIFO_INDEX1,  
	FIFO_INDEX2,
	FIFO_INDEX3,
	FIFO_INDEX4,
	FIFO_INDEX5,
	FIFO_INDEX6,
	FIFO_INDEX7,
	FIFO_INDEX8,
	FIFO_INDEX9,
	FIFO_INDEX10,
	FIFO_INDEX11,
	
	FIFO_MAX_NUM, 
}FIFO_INDEX;

//FIFO环形缓冲结构体
typedef	struct {
	uint8_t *	pFFData;		                           // 接收BUF
	uint16_t	FFDepth;		                           // FIFO 长度
volatile	uint16_t 	FFValidSize;	                 // FIFO中的有效数据大小
volatile	uint16_t 	FFInOffset;		                 // FIFO进 节点
volatile	uint16_t 	FFOutOffset;	                 // FIFO出 节点
}STRUCT_FIFO;
 
extern void FIFO_Init(uint8_t Index, uint8_t *pBuf, uint16_t Len);

extern uint8_t FIFO_Data_Prefetch(uint8_t Index, uint8_t *pBuf, uint16_t Len);
extern void FIFO_Clean_SpecifyLen(uint8_t Index, uint16_t Len);
 
extern uint8_t FIFO_Rece_Buf(uint8_t Index, uint8_t *pBuf, uint16_t Len);	
extern void FIFO_Clean_Buf(uint8_t Index);
extern uint16_t FIFO_Valid_Size(uint8_t Index);
extern uint8_t FIFO_Write_OneByte(uint8_t Index, uint8_t Data);
extern uint8_t FIFO_Write_NByte(uint8_t Index, uint8_t *Data, uint16_t Len);
extern uint8_t FIFO_Empty_Detect(void);
extern uint16_t FIFO_FFDepth(uint8_t Index);

extern uint8_t FIFO_Rece_Buf_Reverse(uint8_t Index, uint8_t *pBuf, uint16_t Len);
#endif	


