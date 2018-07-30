#ifndef __GONGKONG_H
#define	__GONGKONG_H

#include "app_includes.h"


#define VOL_IN ADS1256_MUXP_AIN0
#define VOL_ADR ADS1256_MUXP_AIN2
#define VOL_AGND ADS1256_MUXP_AIN7

u16 Flash_Read_ID(void);
void Flash_Write_ID(uint16_t MyID);
long Git_Vol_ByAIN(char AIN_n);
void Get_Elc_ByUSART(void);
long double Git_Vol(void);
void Vol_Calibrate_ByADR4525(void);
void Delay(__IO uint32_t nCount);
long double Git_Curr(void);


#endif /* __gongkong.h */
