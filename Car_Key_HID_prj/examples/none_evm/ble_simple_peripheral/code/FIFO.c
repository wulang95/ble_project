
#include <stdint.h>
#include "FIFO.h"

volatile STRUCT_FIFO  RxFIFO[FIFO_MAX_NUM] = {0};	          //���ݽ��ջ��λ���

/***************************************************************************************************
** ����    : ���ջ��λ����ʼ��
** ����    : uint8_t Index  FIFO����
             uint8_t *pBuf   BUF
             uint16_t Len    BUF����
** ���    : ��
** ����    : 2017-3-30 16:03:02
** �޸ļ�¼:
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
** ����    : FIFO����У�飬�����������Ƿ����жϵ��´�λ
** ���    : ��ȡ���ĳ���
** ����    : 2021-7-6 22:52:44
** �޸ļ�¼:
***************************************************************************************************/
void FIFO_Size_Check(uint8_t Index)
{
    uint16_t n;

    n = RxFIFO[Index].FFOutOffset + RxFIFO[Index].FFValidSize;
    if(n >= RxFIFO[Index].FFDepth)
    {
        n -= RxFIFO[Index].FFDepth;
    }

    if(n != RxFIFO[Index].FFInOffset)                 //�������������󳤶�����,У������
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
** ����    : �ӻ�����Ԥ��ȡ���ݣ��������������Ч���ݣ����ڼ�����ݰ��Ƿ�������
** ����    : uint8_t Index   FIFO����
             uint8_t *pBuf   ����BUF
			 uint16_t Len    ��ȡ�����ݳ���
** ���    : RT_OK
             RT_FAIL
** ����    : 2021-7-6 22:38:19
** �޸ļ�¼:
***************************************************************************************************/
uint8_t FIFO_Data_Prefetch(uint8_t Index, uint8_t *pBuf, uint16_t Len)
{
    uint16_t OutOffset;

    FIFO_Size_Check(Index);   							 //������ݻ����Ƿ���ȷ

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
** ����    : ���ȫ����������
** ����    : uint8_t Index   FIFO����
** ���    : ��
** ����    : 2017-3-4 15:26:10
** �޸ļ�¼:
*/
void FIFO_Clean_Buf(uint8_t Index)
{
    RxFIFO[Index].FFValidSize = 0;
    RxFIFO[Index].FFInOffset = 0;
    RxFIFO[Index].FFOutOffset = 0;
}

/***************************************************************************************************
** ����    : ���������ָ�����ȵ����ݣ����FIFO_Data_Prefetch����ʹ�ã���֤������Ч���ٴӻ��������ָ���������ݣ�
** ����    : uint8_t Index   FIFO����
			 uint16_t Len    ����ĳ���
** ���    : ��
** ����    : 2021-7-6 22:38:19
** �޸ļ�¼:
***************************************************************************************************/
void FIFO_Clean_SpecifyLen(uint8_t Index, uint16_t Len)
{
	if(Len == 0)
		return;
             
    if(RxFIFO[Index].FFValidSize > Len)	                        //��Ч���ݳ��ȴ���ָ�����������
    {
        //RxFIFO[Index].FFOutOffset = RxFIFO[Index].FFOutOffset + RxFIFO[Index].FFValidSize;
		RxFIFO[Index].FFOutOffset = RxFIFO[Index].FFOutOffset + Len;          //2021-12-29 �޸�BUG 
        if(RxFIFO[Index].FFOutOffset >= RxFIFO[Index].FFDepth)
            RxFIFO[Index].FFOutOffset -= RxFIFO[Index].FFDepth;
           
        RxFIFO[Index].FFValidSize -= Len;
    }
    else                                                       //���ȫ������
    {
        FIFO_Clean_Buf(Index);
    }
}

/***************************************************************************************************
** ����    : �ӻ����ж�ȡ����
** ����    : uint8_t Index   FIFO����
             uint8_t *pBuf   ����BUF
             uint16_t Len    ���յ����ݳ���
** ���    : RT_OK
             RT_FAIL
** ����    : 2017-3-30 16:03:02
** �޸ļ�¼:
***************************************************************************************************/
uint8_t FIFO_Rece_Buf(uint8_t Index, uint8_t *pBuf, uint16_t Len)
{
    FIFO_Size_Check(Index);   							 //������ݻ����Ƿ���ȷ

    if(RxFIFO[Index].FFValidSize >= Len)
    {
        RxFIFO[Index].FFValidSize -= Len;
        while(Len--)
        {
            *pBuf++ = RxFIFO[Index].pFFData[RxFIFO[Index].FFOutOffset++];
            if(RxFIFO[Index].FFOutOffset == RxFIFO[Index].FFDepth)
                RxFIFO[Index].FFOutOffset = 0;
            //RxFIFO[Prot].FFValidSize--;	// ��Ч���� size - 1
        }
        return 0;
    }
    return -1;
}

/***************************************************************************************************
** ����    : �ӻ����ж�ȡ����(�����ȡ����ȡ���д��FIFO������)
** ����    : uint8_t Index   FIFO����
             uint8_t *pBuf   ����BUF
             uint16_t Len    ���յ����ݳ���
** ���    : RT_OK
             RT_FAIL
** ����    : 2023-6-6 21:25:42
** �޸ļ�¼:
***************************************************************************************************/
uint8_t FIFO_Rece_Buf_Reverse(uint8_t Index, uint8_t *pBuf, uint16_t Len)
{
    FIFO_Size_Check(Index);   							 //������ݻ����Ƿ���ȷ

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
** ����    : �����ջ����е����ݳ���
** ����    : uint8_t Index   FIFO����
** ���    : ��Ч���ݳ���
** ����    : 2021-3-26 17:30:07
** �޸ļ�¼:
***************************************************************************************************/
uint16_t FIFO_Valid_Size(uint8_t Index)
{
    return RxFIFO[Index].FFValidSize;
}

/***************************************************************************************************
** ����    : ��ȡFIFO�ܳ���
** ����    : uint8_t Index   FIFO����
** ���    : �ܳ���
** ����    : 2021-3-26 17:30:07
** �޸ļ�¼:
***************************************************************************************************/
uint16_t FIFO_FFDepth(uint8_t Index)
{
    return RxFIFO[Index].FFDepth;
}

/***************************************************************************************************
** ����    : ��FIFOд��һ�ֽ�
** ����    : RT_OK          д��ɹ�
						 RT_FAIL        д��ʧ�� ��������
** ����    : 2021-3-15 21:05:50
** �޸ļ�¼:
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
** ����    : ��FIFOд��N�ֽ�
** ����    : RT_OK          д��ɹ�
						 RT_FAIL        д��ʧ�� ��������
** ����    : 2021-5-21 16:04:32
** �޸ļ�¼:
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
** ����    : ����FIFO�ռ��
** ���    : FALSE     FIFO �ǿ�
						 TRUE      FIFO Ϊ��
** ����    : 2021-3-26 17:36:02
** �޸ļ�¼:
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

