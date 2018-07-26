

#ifndef __APP_DEFINES_H__
#define __APP_DEFINES_H__
#include "app_includes.h"

//LED0 引脚定义
#define LED0_RCC				RCC_APB2Periph_GPIOA
#define LED0_GPIOx				GPIOA
#define LED0_Pin				GPIO_Pin_15

//G6A 引脚定义
#define RELAY_RCC				RCC_APB2Periph_GPIOA
#define RELAY_GPIOx				GPIOA
#define RELAY1_Pin				GPIO_Pin_4
#define RELAY2_Pin				GPIO_Pin_8

#define DPN_RCC				RCC_APB2Periph_GPIOB
#define DPN_RCC_GPIOx				GPIOB
#define D_P_EN_Pin				GPIO_Pin_8
#define PN_PP_EN_Pin				GPIO_Pin_9


#endif


