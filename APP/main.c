
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
volatile u8 	SFlag = 0; //设备状态标志符， 0x00-电压检测状态、0x01-正向供电状态（电流表状态）、0x02-负向供电状态
short			Electric_data; //电流数据
volatile long double VolRate = 0.59604644775390625; //0.59604644775390625
volatile long	VolAGND = 0.0;

u8				Flash_Data[2];



int main(void)
{
	long double 	vol_ADR4515, VolAGND;
	long			temp1, temp2;
//	long double 	ldVolutage;
//	u8 *			p;
//	u8				i;

	STMFLASH_Read(FLASH_SAVE_ADDR, (u16 *) Flash_Data, 1);
	MyID				= Flash_Data[0] << 8 | Flash_Data[1];

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

	printf("My ID is %X\n", MyID);

	Can_Seng_ID(0x07, MyID);

	temp1				= Git_Vol_ByAIN(VOL_AGND);
	VolAGND 			= temp1 * VolRate;
	printf("LongAGND = %d, VolAGND = %LfuV, %LfmV, %LfV\r\n", temp1, VolAGND, VolAGND / 1000, VolAGND / 1000000);

	temp2				= Git_Vol_ByAIN(VOL_ADR);
	vol_ADR4515 		= temp2 * VolRate;
	printf("LongADR4515 = %d, ADR4515 = %LfuV, %LfmV, %LfV\r\n", temp2, vol_ADR4515, vol_ADR4515 / 1000, 
		vol_ADR4515 / 1000000);

//	Vol_Git();

	vol_ADR4515 		= Git_Vol_ByAIN(VOL_VIN1) * VolRate;
	printf("Vol = %LfuV, %LfmV, %LfV\r\n", vol_ADR4515, vol_ADR4515 / 1000, vol_ADR4515 / 1000000);
	
	while (1)
	{
		//		LED0_Test();
		//		Key0_Test();
		//串口测试函数
		//		USART1_Test();
		//		Can_Text();
		USART1_Work();
		Can_Work();
	};
}


