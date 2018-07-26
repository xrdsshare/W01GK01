/*****************************************************************************
 版本:	1.0.0
 作者:	XRDS
 时间:	2018/6/18
 描述:	
 	1、LED 配置文件
 主要外部接口函数如下：
	1、LED_Init(void); //LED初始化函数
	2、LED0_Test(void);	//LED测试函数
 使用方法:	
 	1、1、app_includes.h 开启或添加 #include "led.h"
 	2、添加 led..c 文件到项目中
	3、添加 led.h 头文件编译路径
 	4、在主函数开始时，调用 LED_Init(); 
 	5、在主函数的 while 函数中调用 LED0_Test();进行测试
*****************************************************************************/
#include "led.h"

/************************************************* 
 函数: LED_Init(void)
 描述: LED IO初始化
 输入: 
    1、
    2、
 返回: 
 调用方法: 
    1、在主函数开始时，调用 LED_Init(); 
*************************************************/
void LED_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(LED0_RCC|RCC_APB2Periph_AFIO, ENABLE); //使能LED端口时钟
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LED0_Pin; 		//LED 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度为50MHz
	GPIO_Init(LED0_GPIOx, &GPIO_InitStructure); 	//根据设定参数初始化GPIOB.5
	GPIO_SetBits(LED0_GPIOx, LED0_Pin); 			//LED引脚 输出高
}

////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************* 
 函数: LED0_Test(void)
 描述: LED0 测试函数，LED0每秒亮灭一次
 输入: 
    1、
    2、
 返回: 
 调用方法: 
    1、在主函数的 while 函数中调用 LED0_Test();进行测试
*************************************************/
void LED0_Test(void)
{
//	LED0(ON);										// 亮
//	Delay_ms(500);
//	LED0(OFF);										// 灭
//	Delay_ms(500);

	LED0((BitAction) ((1 - GPIO_ReadOutputDataBit(LED0_GPIOx, LED0_Pin)))); //LED0翻转
	Delay_ms(500);
}


