

#ifndef __LED_H
#define __LED_H
#include "app_includes.h"

#define ON						0
#define OFF 					1

#define LED0(a) 				if (a)									\
		GPIO_SetBits(LED0_GPIOx,LED0_Pin);								\
		else															\
		GPIO_ResetBits(LED0_GPIOx,LED0_Pin)

void LED_Init(void); //≥ı ºªØ
void LED0_Test(void);


#endif

