
#include "app_includes.h"

//用volatile定义的变量会在程序外被改变,每次都必须从内存中读取，而不能重复使用放在cache或寄存器中的备份。
volatile u8 	Usart1ReceiveState = 0; //串口1接收不定长度字符串结束标识符
volatile u8 	Usart1ReceiveCounter = 0; //串口1接收到的字符串个数
volatile u8 	Usart1Buffer[20] =
{
	0x00
};


// CAN 数据变量定义
volatile u8 	CanReceiveState = 0; //CAN接收不定长度字符串结束标识符
volatile u8 	CanReceiveCounter = 0; //CAN接收到的字符串个数
volatile u8 	CanBuffer[20] =
{
	0x00
};
	
volatile u16		MyID = 0x8000; //本机发送ID
short			Electric_data; //电流数据
long double 	VolRate = 0.6238491745551583961; //0.59604644775390625
long			VolAGND = 0.0;


int main(void)
{
	LED_Init(); 									//LED 端口初始化 
//	Key_Init(); 									//按键初始化
	USART1_Config(115200);

	/* 配置CAN模块 */
	CAN_Config();
	Init_ADS1256_GPIO();							//初始化ADS1256 GPIO管脚 
	Delay_ms(100);
	GPIO_SetBits(GPIOB, GPIO_Pin_11);
//	ADS1256_Init();
	G6A_Init();
//	Vol_Calibrate_ByADR4525();						//	电压自校准

	printf("==============测试程序开始！==============");

	CAN_Send(0x1234, "12345678");

	while (1)
	{
		LED0_Test();
		//		Key0_Test();

		//串口测试函数
		USART1_Test();
//		Can_Text();
	};
}


