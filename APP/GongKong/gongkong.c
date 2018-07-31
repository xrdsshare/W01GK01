

/*****************************************************************************
Version:  1.0.0  
Author:   MXD	   
Date:  2018/4/11	   
Description:   工控板主要控制接口函数	
History:	   
*****************************************************************************/
#include "gongkong.h"

extern u16		MyID;
extern long 	VolAGND;
extern long double VolRate;



int 			i = 0;

/**
* funtion: 读取flash中的ID
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
* funtion: 写入Flash的ID函数
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
	Description:	读取指定通道电压值，返回对应的Long类型数据
	Input:	1.AIN_n 读取的通道
	Return:	 
	Others:	 AIN_n 可取：ADS1256_MUXP_AIN0 、 ADS1256_MUXP_AIN1 、 ADS1256_MUXP_AIN2 、 ADS1256_MUXP_AIN3 、
						 ADS1256_MUXP_AIN4 、 ADS1256_MUXP_AIN5 、 ADS1256_MUXP_AIN6 、 ADS1256_MUXP_AIN7
	
*************************************************/
long Git_Vol_ByAIN(char AIN_n)
{
	long			ulResult0, ulResult1, ulResult2;

	//			ulResult = ADS_sum( (i << 4) | ADS1256_MUXN_AINCOM);	//多通道测试
	ulResult0			= ADS_sum(AIN_n | ADS1256_MUXN_AINCOM); //单通道测试
	ulResult0			= ADS_sum(AIN_n | ADS1256_MUXN_AINCOM); //单通道测试

	if (ulResult0 & 0x800000)
	{
		ulResult0			= ~(unsigned long)
		ulResult0;
		ulResult0			&= 0x7fffff;
		ulResult0			+= 1;
		ulResult0			= -ulResult0;
	}

	ulResult1			= ADS_sum(AIN_n | ADS1256_MUXN_AINCOM); //单通道测试

	if (ulResult1 & 0x800000)
	{
		ulResult1			= ~(unsigned long)
		ulResult1;
		ulResult1			&= 0x7fffff;
		ulResult1			+= 1;
		ulResult1			= -ulResult1;
	}

	ulResult2			= ADS_sum(AIN_n | ADS1256_MUXN_AINCOM); //单通道测试

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


//can发送电压数据
void Send_Vol_Long(void)
{
	long			ulResult = Git_Vol_ByAIN(VOL_VIN);

	//	char *q = (char*)&ldVolutage;
	//	CAN_Send(MyID, q, 8);
}


/**
* funtion: 串口2获取电流 指令：AA 55 02 FE 01 00
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
	Description:	通过long类型数据获取电压数据
	Input:	
	Return:	 
	Others:	 
*************************************************/
long double Git_Vol(void)
{
	long double 	vol_Data;
	long			vol_long;

	//	Vol_Calibrate_ByADR4525();						//每次测量前进行一次自校准
	VolAGND 			= Git_Vol_ByAIN(VOL_AGND);

	G6A_RELAY1(ON);
	Delay_ms(100);

	Git_Vol_ByAIN(VOL_VIN);
	Git_Vol_ByAIN(VOL_VIN);
	vol_long			= Git_Vol_ByAIN(VOL_VIN);
	vol_Data			= vol_long * VolRate;

	printf("Vol long:%d\n", vol_long);
	G6A_RELAY1(OFF);
	return vol_Data;
}


/************************************************* 
	Function:	Vol_Calibrate_ByADR4525
	Description:	电压表自校准函数
	Input:	1.
	Return:	 
	Others:	 
*************************************************/
void Vol_Calibrate_ByADR4525(void)
{
//	long double 	vol_Data;
//	long			vol_long;

//	Git_Vol_ByAIN(VOL_AGND);						//扔掉第一次不准确的值
//	Git_Vol_ByAIN(VOL_AGND);
//	VolAGND 			= Git_Vol_ByAIN(VOL_AGND);
//	printf("VolAGND long:%X\n", VolAGND);

//	Git_Vol_ByAIN(VOL_VIN);							//扔掉第一次不准确的值
//	Git_Vol_ByAIN(VOL_VIN);
//	vol_long			= Git_Vol_ByAIN(VOL_VIN);
//	printf("vol_long long:%X\n", vol_long);

//	VolRate 			= (2.5 * 1000000) / (vol_long - VolAGND);

//	vol_Data			= VolRate * (vol_long - VolAGND);

	//	printf("\r\n*******************************************************\r\n");
	//	printf("\r\nThe V_2.5 = %Lf V\r\n", vol_Data / 1000000);
	//	printf("The V_AGND = %Lf V\r\n", VolAGND * VolRate / 1000000);
	//	printf("The VolRate = %Lf V\r\n", VolRate);
	//	printf("\r\n*******************************************************\r\n");
}


void Can_Send_Data(u16 sid, u8 com, u8 * p, u8 len)
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


/************************************************* 
	Function:	Git_Curr
	Description:	读取电流数据
	Input:	1.
	Return:	 
	Others:	 
************************************************
long double Git_Curr(void)
{
	long double 	vol_Data;
	long			vol_long;

	//	Vol_Calibrate_ByADR4525();		//每次测量前进行一次自校准
	G6A_RELAY2_CMD(ON);
	G6A_RELAY1_CMD(ON);
	Delay(0x5555);

	Git_Vol_ByAIN(VOL_VIN);
	Git_Vol_ByAIN(VOL_VIN);
	vol_long			= Git_Vol_ByAIN(VOL_VIN);
	vol_Data			= vol_long * VolRate;
	G6A_RELAY1_CMD(OFF);
	G6A_RELAY2_CMD(OFF);

	//	printf("The Curr = %Lf mA\r\n", vol_Data / 10000);
	return vol_Data / 10000;
}
*/

/************************************************* 
	Function:	Delay
	Description:	简单延时函数
	Input:	1.nCount 延时时间常数
	Return:	 
	Others:	 
*************************************************/
//void Delay(__IO uint32_t nCount)
//{
//	for (; nCount != 0; nCount--)
//		;
//}
