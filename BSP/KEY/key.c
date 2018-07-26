/*****************************************************************************
 版本:	1.0.0
 作者:	XRDS
 时间:	2018/6/18
 描述:	
 	1、按键配置文件
 主要外部接口函数如下：
 	1、Key_Init();	按键初始化函数
	2、Key_Scan();
	3、Key0_Test();
 使用方法:	
 	1、app_includes.h 开启或添加 #include "key.h"
 	2、添加 usart1..c 文件到项目中
	3、添加 usart1.h 头文件编译路径
 	4、在主函数开始时，调用 Key_Init(); 
 	5、在主函数的 while 函数中调用 Key0_Test(); （注意：排除其他LED0控制的干扰）
*****************************************************************************/
#include "key.h"


/************************************************* 
 函数: Key_Init(void)
 描述: 按键初始化函数
 输入: 
    1、
    2、
 返回: 
 调用方法: 
    1、在主函数开始时，调用 Key_Init(); 
*************************************************/
void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;			/*定义一个GPIO_InitTypeDef类型的结构体*/

	RCC_APB2PeriphClockCmd(KEY0_RCC, ENABLE); // 使能KEY端口时钟
	GPIO_InitStructure.GPIO_Pin = KEY0_Pin; 		//选择对应的引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	/*设置引脚模式为通用推挽输出*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; /*设置引脚速率为50MHz */
	GPIO_Init(KEY0_GPIOx, &GPIO_InitStructure); 	//初始化KEY端口
}

/************************************************* 
 函数: Key_Scan(GPIO_TypeDef * GPIOx, u16 GPIO_Pin)
 描述: 按键扫描及消抖函数
 输入: 
    1、GPIOx 要扫描按键的控制GPIO组，可选 GPIOA、GPIOB、GPIOC 等；
    2、GPIO_Pin 要扫描按键的引脚，可选 GPIO_Pin_0、GPIO_Pin_1、GPIO_Pin_3、……
 返回: 
 调用方法: 
    1、
*************************************************/
u8 Key_Scan(GPIO_TypeDef * GPIOx, u16 GPIO_Pin)
{
	/*检测是否有按键按下 */
	if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
	{
		/*延时消抖*/
		Delay_ms(10);

		if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
		{
			/*等待按键释放 */
			while (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
				;

			return KEY_ON;
		}
		else 
			return KEY_OFF;
	}
	else 
		return KEY_OFF;
}

////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************* 
 函数: Key0_Test(void)
 描述: 按键测试函数
 		按键按下后，LED0翻转
 输入: 
    1、
    2、
 返回: 
 调用方法: 
    1、在主函数的 while 函数中调用 Key0_Test(); （注意：排除其他LED0控制的干扰）
*************************************************/
void Key0_Test(void)
{
	if (Key_Scan(KEY0_GPIOx, KEY0_Pin) == KEY_ON) //判断KEY1是否按下
	{
		GPIO_WriteBit(LED0_GPIOx, LED0_Pin, (BitAction) ((1 - GPIO_ReadOutputDataBit(LED0_GPIOx, LED0_Pin)))); //LED0翻转
	}
}




