#ifndef __GONGKONG_H
#define	__GONGKONG_H

#include "app_includes.h"


#define VOL_VIN0 ADS1256_MUXP_AIN3
#define VOL_VIN1 ADS1256_MUXP_AIN4
#define VOL_VIN2 ADS1256_MUXP_AIN5
#define VOL_CIN0 ADS1256_MUXP_AIN0 
#define VOL_CIN1 ADS1256_MUXP_AIN1 
#define VOL_CIN2 ADS1256_MUXP_AIN2 
#define VOL_ADR ADS1256_MUXP_AIN7
#define VOL_AGND ADS1256_MUXP_AIN6

u16 Flash_Read_ID(void);
void Flash_Write_ID(uint16_t MyID);
long Git_Vol_ByAIN(char AIN_n);
void Get_Elc_ByUSART(void);
//long double Git_Vol(void);
void Vol_Git(void);
void GK_Test(void);
void Can_Send_Data(u8 com, u8 * p, u8 len);
void USART_Seng_ID(u16 SID);
void Can_Seng_ID(u8 com, u16 SID);
void Can_Seng_VC(u8 com, u16 SID, u8 * p);



#endif /* __gongkong.h */
