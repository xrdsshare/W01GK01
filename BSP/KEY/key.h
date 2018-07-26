

#ifndef __KEY_H
#define __KEY_H
#include "app_includes.h"


/*******
*按键按下标置
KEY_ON 0
KEY_OFF 1
********/
#define KEY_ON					0
#define KEY_OFF 				1

void Key_Init(void);
u8 Key_Scan(GPIO_TypeDef * GPIOx, u16 GPIO_Pin);
void Key0_Test(void);


#endif /* __KEY_H */

