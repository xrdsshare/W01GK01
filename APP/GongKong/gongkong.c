

/*****************************************************************************
Version:  1.0.0  
Author:   MXD	   
Date:  2018/4/11	   
Description:   ���ذ���Ҫ���ƽӿں���	
History:	   
*****************************************************************************/
#include "gongkong.h"

extern u16		MyID;
extern long 	VolAGND;
extern long double VolRate;



int 			i = 0;

/**
* funtion: ��ȡflash�е�ID
* 2018/4/3
**/
uint16_t Flash_Read_ID(void)
{
	uint16_t		FMyID = 0xFFFF;
	uint8_t 		Flash_Data[2];

	STMFLASH_Read(FLASH_SAVE_ADDR, (uint16_t *) Flash_Data, 1);
	FMyID				= Flash_Data[0] << 8 | Flash_Data[1];
	return FMyID;
}


/**
* funtion: д��Flash��ID����
* 2018/4/3
**/
void Flash_Write_ID(uint16_t FMyID)
{
	uint8_t 		Flash_Data[2];

	Flash_Data[1]		= FMyID & 0x00FF;
	Flash_Data[0]		= FMyID >> 8 & 0x00FF;
	STMFLASH_Write(FLASH_SAVE_ADDR, (uint16_t *) Flash_Data, 1);
	MyID				= FMyID;
	CAN_Config();
}


/************************************************* 
	Function:	Git_Vol_ByAIN
	Description:	��ȡָ��ͨ����ѹֵ�����ض�Ӧ��Long��������
	Input:	1.AIN_n ��ȡ��ͨ��
	Return:	 
	Others:	 AIN_n ��ȡ��ADS1256_MUXP_AIN0 �� ADS1256_MUXP_AIN1 �� ADS1256_MUXP_AIN2 �� ADS1256_MUXP_AIN3 ��
						 ADS1256_MUXP_AIN4 �� ADS1256_MUXP_AIN5 �� ADS1256_MUXP_AIN6 �� ADS1256_MUXP_AIN7
	
*************************************************/
long Git_Vol_ByAIN(char AIN_n)
{
	long			ulResult0, ulResult1, ulResult2;

	//			ulResult = ADS_sum( (i << 4) | ADS1256_MUXN_AINCOM);	//��ͨ������
	ulResult0			= ADS_sum(AIN_n | ADS1256_MUXN_AINCOM); //��ͨ������
	ulResult0			= ADS_sum(AIN_n | ADS1256_MUXN_AINCOM); //��ͨ������

	if (ulResult0 & 0x800000)
	{
		ulResult0			= ~(unsigned long)
		ulResult0;
		ulResult0			&= 0x7fffff;
		ulResult0			+= 1;
		ulResult0			= -ulResult0;
	}

	ulResult1			= ADS_sum(AIN_n | ADS1256_MUXN_AINCOM); //��ͨ������

	if (ulResult1 & 0x800000)
	{
		ulResult1			= ~(unsigned long)
		ulResult1;
		ulResult1			&= 0x7fffff;
		ulResult1			+= 1;
		ulResult1			= -ulResult1;
	}

	ulResult2			= ADS_sum(AIN_n | ADS1256_MUXN_AINCOM); //��ͨ������

	if (ulResult2 & 0x800000)
	{
		ulResult2			= ~(unsigned long)
		ulResult2;
		ulResult2			&= 0x7fffff;
		ulResult2			+= 1;
		ulResult2			= -ulResult2;
	}

	return (ulResult0 + ulResult1 + ulResult2) / 3;
}


//can���͵�ѹ����
void Send_Vol_Long(void)
{
	long			ulResult = Git_Vol_ByAIN(VOL_VIN0);

	//	char *q = (char*)&ldVolutage;
	//	CAN_Send(MyID, q, 8);
}


/**
* funtion: ����2��ȡ���� ָ�AA 55 02 FE 01 00
* 2018/4/4
*
void Get_Elc_ByUSART(void)
{
	USART2_Send_Char(0xAA);
	USART2_Send_Char(0x55);
	USART2_Send_Char(0x22);
	USART2_Send_Char(0xFE);
	USART2_Send_Char(0x01);
	USART2_Send_Char(0x00);
}
*/

/************************************************* 
	Function:	Git_Vol
	Description:	ͨ��long�������ݻ�ȡ��ѹ����
	Input:	
	Return:	 
	Others:	 
*************************************************/
//long double Git_Vol(void)
//{
//	long double 	vol_Data;
//	long			vol_long;
//	//	Vol_Calibrate_ByADR4525();						//ÿ�β���ǰ����һ����У׼
//	VolAGND 			= Git_Vol_ByAIN(VOL_AGND);
//	G6A_Vol(ON);
//	Delay_ms(100);
//	Git_Vol_ByAIN(VOL_VIN0);
//	Git_Vol_ByAIN(VOL_VIN0);
//	vol_long			= Git_Vol_ByAIN(VOL_VIN0);
//	vol_Data			= vol_long * VolRate;
//	printf("Vol long:%d\n", vol_long);
//	G6A_Vol(OFF);
//	return vol_Data;
//}
void Vol_Git(void)
{
	long double 	ldVolutage;

	G6A_Vol(ON);
	Delay_us(1000);

	ldVolutage			= Git_Vol_ByAIN(VOL_VIN1) *VolRate;
	printf("%LfuV, %LfmV, %LfV\r\n", ldVolutage, ldVolutage / 1000, ldVolutage / 1000000);
	ldVolutage			= Git_Vol_ByAIN(VOL_AGND) *VolRate;
	printf("%LfuV, %LfmV, %LfV\r\n", ldVolutage, ldVolutage / 1000, ldVolutage / 1000000);
}


void Can_Send_Data(u8 com, u8 * p, u8 len)
{
	u8				temp[20];
	u8				i;

	if (len < 6)
	{
		temp[0] 			= 0x50;
	}
	else 
	{
		temp[0] 			= 0x55;
	}

	temp[1] 			= len + 3;
	temp[2] 			= com;

	for (i = 0; i < len; ++i)
	{
		temp[i + 3] 		= * (p + i);
	}

	CAN_Send(MyID, temp, temp[1]);
}


void GK_Test(void)
{
	long			V_M, V_250, V_251, V_AGND;
	long double 	DV_b;

	//	long double 	DV_M, DV_250, DV_251, DV_AGND, DV_b;
	V_AGND				= Git_Vol_ByAIN(VOL_AGND);
	V_250				= Git_Vol_ByAIN(VOL_ADR);

	V_251				= (Git_Vol_ByAIN(VOL_VIN2) / 10);
	//	G6A_Vol(ON);
	//	Delay_ms(500);
	DV_b				=
		 (Git_Vol_ByAIN(VOL_CIN0) / 10.0) + (Git_Vol_ByAIN(VOL_CIN0) / 10.0) + (Git_Vol_ByAIN(VOL_CIN0) / 10.0) + (Git_Vol_ByAIN(VOL_CIN0) / 10.0) + (Git_Vol_ByAIN(VOL_CIN0) / 10.0) + (Git_Vol_ByAIN(VOL_CIN0) / 10.0) + (Git_Vol_ByAIN(VOL_CIN0) / 10.0) + (Git_Vol_ByAIN(VOL_CIN0) / 10.0) + (Git_Vol_ByAIN(VOL_CIN0) / 10.0) + (Git_Vol_ByAIN(VOL_CIN0) / 10.0);

	//	Delay_ms(500);
	//	G6A_Vol(OFF);
	//	DV_b = 2.5 - V_251 * VolRate; 
	printf("%d, %d, %d, %d\r\n", V_AGND, V_250, V_251, V_M);
	printf("%LfV, %LfV, %LfmV, %LfmV\r\n", V_AGND * VolRate / 1000000, V_250 * VolRate / 1000000, 
		DV_b * VolRate / 1000, V_251 * VolRate / 1000);
}


/************************************************* 
 ����: Can_Seng_ID(u8 com, u16 SID)
 ����: ʹ��CAN����ID
 ����: 
	1��com ���͵�����ָ��
	2��SID Ҫ���͵�ID����
 ����: 
 ���÷���: 
	1��ֱ�ӵ���
*************************************************/
void Can_Seng_ID(u8 com, u16 SID)
{
	u16 			Temp;

	Temp				= SID;
	Temp				= Temp << 8 | SID >> 8;
	Can_Send_Data(com, (u8 *) &Temp, 2);
}


void USART_Seng_ID(u16 SID)
{
	USART1_Char(0xAA);
	USART1_Char(0x05);
	USART1_Char(0x07);
	USART1_Char((u8) (SID >> 8));
	USART1_Char((u8) SID);
}


/************************************************* 
	Function:	Git_Curr
	Description:	��ȡ��������
	Input:	1.
	Return:	 
	Others:	 
************************************************
long double Git_Curr(void)
{
	long double 	vol_Data;
	long			vol_long;

	//	Vol_Calibrate_ByADR4525();		//ÿ�β���ǰ����һ����У׼
	G6A_RELAY2_CMD(ON);
	G6A_RELAY1_CMD(ON);
	Delay(0x5555);

	Git_Vol_ByAIN(VOL_VIN0);
	Git_Vol_ByAIN(VOL_VIN0);
	vol_long			= Git_Vol_ByAIN(VOL_VIN0);
	vol_Data			= vol_long * VolRate;
	G6A_RELAY1_CMD(OFF);
	G6A_RELAY2_CMD(OFF);

	//	printf("The Curr = %Lf mA\r\n", vol_Data / 10000);
	return vol_Data / 10000;
}
*/
