#ifndef __G6A_H
#define	__G6A_H

#include "stm32f10x.h"
#include "app_includes.h"

#define G6A_RELAY1(a)	if (a)	\
					GPIO_SetBits(RELAY_GPIOx,RELAY1_Pin);\
					else		\
					GPIO_ResetBits(RELAY_GPIOx,RELAY1_Pin)
#define G6A_RELAY2(a)	if (a)	\
					GPIO_SetBits(RELAY_GPIOx,RELAY2_Pin);\
					else		\
					GPIO_ResetBits(RELAY_GPIOx,RELAY2_Pin)
#define PN_PP_EN(a)	if (a)	\
					GPIO_SetBits(DPN_RCC_GPIOx,PN_PP_EN_Pin);\
					else		\
					GPIO_ResetBits(DPN_RCC_GPIOx,PN_PP_EN_Pin)
#define P_D_EN(a)	if (a)	\
					GPIO_SetBits(DPN_RCC_GPIOx,D_P_EN_Pin);\
					else		\
					GPIO_ResetBits(DPN_RCC_GPIOx,D_P_EN_Pin)

void G6A_Init(void);
void G6A_Test(void);


#endif /* __G6A_H */
