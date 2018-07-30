
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


volatile u16	MyID = 0x8000; //本机发送ID
short			Electric_data; //电流数据
volatile long double 	VolRate = 0.6238491745551583961; //0.59604644775390625
volatile long			VolAGND = 0.0;

u8				Flash_Data[2];



int main(void)
{
	long double 	vol_ADR4515;

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
	printf("==============测试程序开始！==============\n");

	STMFLASH_Read(FLASH_SAVE_ADDR, (u16 *) Flash_Data, 1);
	MyID				= Flash_Data[0] << 8 | Flash_Data[1];
	printf("My ID is %X\n", MyID);

	CAN_Send(MyID, (char *) &MyID);	
	
	Git_Vol_ByAIN(VOL_IN);
	vol_ADR4515 = (Git_Vol_ByAIN(VOL_IN) -VolAGND) *VolRate;
	printf("ADR4515 = %LfuV, %LfmV, %LfV\r\n", vol_ADR4515, vol_ADR4515 / 1000, vol_ADR4515 / 1000000);

	//	FLASH_Write_Test(0X0800FC00, 0x1234);
	while (1)
	{
		//		LED0_Test();
		//		Key0_Test();
		//串口测试函数
//		USART1_Test();

		//		Can_Text();
	};
}


