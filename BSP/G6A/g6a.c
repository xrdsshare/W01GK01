#include "g6a.h"


void G6A_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;			/*定义一个GPIO_InitTypeDef类型的结构体*/

	RCC_APB2PeriphClockCmd(RELAY_RCC, ENABLE); // 使能PC端口时钟  
	GPIO_InitStructure.GPIO_Pin = RELAY1_Pin | RELAY2_Pin; //选择对应的引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /*设置引脚模式为通用推挽输出*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; /*设置引脚速率为50MHz */
	GPIO_Init(RELAY_GPIOx, &GPIO_InitStructure);			//
	GPIO_SetBits(RELAY_GPIOx, RELAY1_Pin | RELAY2_Pin);	// 关闭所有LED
	
	RCC_APB2PeriphClockCmd(DPN_RCC, ENABLE); // 使能PC端口时钟  
	GPIO_InitStructure.GPIO_Pin = D_P_EN_Pin | PN_PP_EN_Pin; //选择对应的引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /*设置引脚模式为通用推挽输出*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; /*设置引脚速率为50MHz */
	GPIO_Init(DPN_RCC_GPIOx, &GPIO_InitStructure);			//
	GPIO_SetBits(DPN_RCC_GPIOx, D_P_EN_Pin | PN_PP_EN_Pin);	// 关闭所有LED
}

void G6A_Test(void)
{
	G6A_RELAY1(ON);
	G6A_RELAY2(ON);
	PN_PP_EN(ON);
	P_D_EN(ON);
}

