

#ifndef __APP_DEFINES_H__
#define __APP_DEFINES_H__
#include "app_includes.h"

#define FLASH_SAVE_ADDR 		0X0800FC00		//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)

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


