
#include <stdint.h>
#include "FIFO.h"

volatile STRUCT_FIFO  RxFIFO[FIFO_MAX_NUM] = {0};	          //数据接收环形缓冲

/***************************************************************************************************
** 功能    : 接收环形缓冲初始化
** 输入    : uint8_t Index  FIFO索引
             uint8_t *pBuf   BUF
             uint16_t Len    BUF长度
** 输出    : 无
** 日期    : 2017-3-30 16:03:02
** 修改记录:
***************************************************************************************************/
void FIFO_Init(uint8_t Index, uint8_t *pBuf, uint16_t Len)
{
    RxFIFO[Index].pFFData = pBuf;
    RxFIFO[Index].FFDepth = Len;
    RxFIFO[Index].FFValidSize = 0;
    RxFIFO[Index].FFInOffset = 0;
    RxFIFO[Index].FFOutOffset = 0;
}

/***************************************************************************************************
** 功能    : FIFO长度校验，检查进出长度是否因中断导致错位
** 输出    : 读取到的长度
** 日期    : 2021-7-6 22:52:44
** 修改记录:
***************************************************************************************************/
void FIFO_Size_Check(uint8_t Index)
{
    uint16_t n;

    n = RxFIFO[Index].FFOutOffset + RxFIFO[Index].FFValidSize;
    if(n >= RxFIFO[Index].FFDepth)
    {
        n -= RxFIFO[Index].FFDepth;
    }

    if(n != RxFIFO[Index].FFInOffset)                 //缓冲器进出错误长度有误,校正长度
    {
        #ifdef DUBUG_FLAG
        printf("\n\n****FFInOffset ERROR**\n");
        printf("n:%d\n", n);
        printf("FFInOffset:%d\n", RxFIFO[Index].FFInOffset);
        #endif
        if(RxFIFO[Index].FFInOffset > RxFIFO[Index].FFValidSize)
            RxFIFO[Index].FFOutOffset = RxFIFO[Index].FFInOffset - RxFIFO[Index].FFValidSize;
        else
            RxFIFO[Index].FFOutOffset = (RxFIFO[Index].FFInOffset + RxFIFO[Index].FFDepth) - RxFIFO[Index].FFValidSize;
    }
}

/***************************************************************************************************
** 功能    : 从缓冲中预读取数据（不清除缓存中有效数据，用于检查数据包是否完整）
** 输入    : uint8_t Index   FIFO索引
             uint8_t *pBuf   接收BUF
			 uint16_t Len    读取的数据长度
** 输出    : RT_OK
             RT_FAIL
** 日期    : 2021-7-6 22:38:19
** 修改记录:
***************************************************************************************************/
uint8_t FIFO_Data_Prefetch(uint8_t Index, uint8_t *pBuf, uint16_t Len)
{
    uint16_t OutOffset;

    FIFO_Size_Check(Index);   							 //检查数据缓冲是否正确

    if(RxFIFO[Index].FFValidSize >= Len)
    {
        OutOffset = RxFIFO[Index].FFOutOffset;
        while(Len--)
        {
            *pBuf++ = RxFIFO[Index].pFFData[OutOffset++];
            if(OutOffset == RxFIFO[Index].FFDepth)
                OutOffset = 0;
        }
        return 0;
    }
    return -1;
}
 

/*
** 功能    : 清除全部缓存数据
** 输入    : uint8_t Index   FIFO索引
** 输出    : 无
** 日期    : 2017-3-4 15:26:10
** 修改记录:
*/
void FIFO_Clean_Buf(uint8_t Index)
{
    RxFIFO[Index].FFValidSize = 0;
    RxFIFO[Index].FFInOffset = 0;
    RxFIFO[Index].FFOutOffset = 0;
}

/***************************************************************************************************
** 功能    : 清除缓存中指定长度的数据（配合FIFO_Data_Prefetch函数使用，验证数据有效后，再从缓存中清除指定长度数据）
** 输入    : uint8_t Index   FIFO索引
			 uint16_t Len    清除的长度
** 输出    : 无
** 日期    : 2021-7-6 22:38:19
** 修改记录:
***************************************************************************************************/
void FIFO_Clean_SpecifyLen(uint8_t Index, uint16_t Len)
{
	if(Len == 0)
		return;
             
    if(RxFIFO[Index].FFValidSize > Len)	                        //有效数据长度大与指定的清除长度
    {
        //RxFIFO[Index].FFOutOffset = RxFIFO[Index].FFOutOffset + RxFIFO[Index].FFValidSize;
		RxFIFO[Index].FFOutOffset = RxFIFO[Index].FFOutOffset + Len;          //2021-12-29 修改BUG 
        if(RxFIFO[Index].FFOutOffset >= RxFIFO[Index].FFDepth)
            RxFIFO[Index].FFOutOffset -= RxFIFO[Index].FFDepth;
           
        RxFIFO[Index].FFValidSize -= Len;
    }
    else                                                       //清除全部数据
    {
        FIFO_Clean_Buf(Index);
    }
}

/***************************************************************************************************
** 功能    : 从缓冲中读取数据
** 输入    : uint8_t Index   FIFO索引
             uint8_t *pBuf   接收BUF
             uint16_t Len    接收的数据长度
** 输出    : RT_OK
             RT_FAIL
** 日期    : 2017-3-30 16:03:02
** 修改记录:
***************************************************************************************************/
uint8_t FIFO_Rece_Buf(uint8_t Index, uint8_t *pBuf, uint16_t Len)
{
    FIFO_Size_Check(Index);   							 //检查数据缓冲是否正确

    if(RxFIFO[Index].FFValidSize >= Len)
    {
        RxFIFO[Index].FFValidSize -= Len;
        while(Len--)
        {
            *pBuf++ = RxFIFO[Index].pFFData[RxFIFO[Index].FFOutOffset++];
            if(RxFIFO[Index].FFOutOffset == RxFIFO[Index].FFDepth)
                RxFIFO[Index].FFOutOffset = 0;
            //RxFIFO[Prot].FFValidSize--;	// 有效数据 size - 1
        }
        return 0;
    }
    return -1;
}

/***************************************************************************************************
** 功能    : 从缓冲中读取数据(反向读取，读取最后写入FIFO的数据)
** 输入    : uint8_t Index   FIFO索引
             uint8_t *pBuf   接收BUF
             uint16_t Len    接收的数据长度
** 输出    : RT_OK
             RT_FAIL
** 日期    : 2023-6-6 21:25:42
** 修改记录:
***************************************************************************************************/
uint8_t FIFO_Rece_Buf_Reverse(uint8_t Index, uint8_t *pBuf, uint16_t Len)
{
    FIFO_Size_Check(Index);   							 //检查数据缓冲是否正确

    if(RxFIFO[Index].FFValidSize >= Len)
    {
        RxFIFO[Index].FFValidSize -= Len;
        while(Len--)
        {
			if(RxFIFO[Index].FFInOffset == 0)
				RxFIFO[Index].FFInOffset = RxFIFO[Index].FFDepth-1;
			else
				RxFIFO[Index].FFInOffset--;
			pBuf[Len] = RxFIFO[Index].pFFData[RxFIFO[Index].FFInOffset];
        }
        return 0;
    }
    return -1;
}

/***************************************************************************************************
** 功能    : 检查接收缓存中的数据长度
** 输入    : uint8_t Index   FIFO索引
** 输出    : 有效数据长度
** 日期    : 2021-3-26 17:30:07
** 修改记录:
***************************************************************************************************/
uint16_t FIFO_Valid_Size(uint8_t Index)
{
    return RxFIFO[Index].FFValidSize;
}

/***************************************************************************************************
** 功能    : 获取FIFO总长度
** 输入    : uint8_t Index   FIFO索引
** 输出    : 总长度
** 日期    : 2021-3-26 17:30:07
** 修改记录:
***************************************************************************************************/
uint16_t FIFO_FFDepth(uint8_t Index)
{
    return RxFIFO[Index].FFDepth;
}

/***************************************************************************************************
** 功能    : 向FIFO写入一字节
** 返回    : RT_OK          写入成功
						 RT_FAIL        写入失败 缓存已满
** 日期    : 2021-3-15 21:05:50
** 修改记录:
***************************************************************************************************/
uint8_t FIFO_Write_OneByte(uint8_t Index, uint8_t Data)
{
    if(RxFIFO[Index].FFValidSize < RxFIFO[Index].FFDepth)
    {
        RxFIFO[Index].pFFData[RxFIFO[Index].FFInOffset++] = Data;
        if(RxFIFO[Index].FFInOffset == RxFIFO[Index].FFDepth)
            RxFIFO[Index].FFInOffset = 0;
        RxFIFO[Index].FFValidSize++;
        return 0;
    }
    return -1;
}

/***************************************************************************************************
** 功能    : 向FIFO写入N字节
** 返回    : RT_OK          写入成功
						 RT_FAIL        写入失败 缓存已满
** 日期    : 2021-5-21 16:04:32
** 修改记录:
***************************************************************************************************/
uint8_t FIFO_Write_NByte(uint8_t Index, uint8_t *Data, uint16_t Len)
{
    uint16_t WriteLen, i;

    if(RxFIFO[Index].FFValidSize + Len <= RxFIFO[Index].FFDepth)
        WriteLen = Len;
    else
        WriteLen = RxFIFO[Index].FFDepth - RxFIFO[Index].FFValidSize;

    for(i = 0; i < WriteLen; i++)
    {
        RxFIFO[Index].pFFData[RxFIFO[Index].FFInOffset++] = Data[i];
        if(RxFIFO[Index].FFInOffset == RxFIFO[Index].FFDepth)
            RxFIFO[Index].FFInOffset = 0;
    }
    RxFIFO[Index].FFValidSize += WriteLen;
    return 0;
}

/***************************************************************************************************
** 功能    : 所有FIFO空检测
** 输出    : FALSE     FIFO 非空
						 TRUE      FIFO 为空
** 日期    : 2021-3-26 17:36:02
** 修改记录:
***************************************************************************************************/
uint8_t FIFO_Empty_Detect()
{
    uint8_t Index;

    for(Index = 0; Index < FIFO_MAX_NUM; Index++)
    {
        if(RxFIFO[Index].FFValidSize != 0)
            return -1;
    }
    return 0;
}

