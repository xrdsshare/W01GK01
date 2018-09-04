
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
volatile u8 	CMD = 0; //电压通道
volatile u8 	VMD = 0; //电流通道

volatile u8 	SFlag = 0; //设备状态标志符， 0x00-电压检测状态、0x01-正向供电状态（电流表状态）、0x02-负向供电状态
short			Electric_data; //电流数据
volatile long double VolRate = 0.59604644775390625; //0.59604644775390625
volatile long double VolCha = 31000; //
volatile long	VolAGND = 0.0;

u8				Flash_Data[2];


int main(void)
{
	//	long double 	VolAGND;
	//	long			temp1, temp2;
	//	long double 	ldVolutage;
	//	u8 *			p;
	//	u8				i;
	//	long double 	vol;
	u8 *			p	= (u8 *) &VolRate;

	//	long double 	ldTemp = 0xFFFFFFFFFFFFFFFF;
	STMFLASH_Read(FLASH_SAVE_ADDR, (u16 *) Flash_Data, 1);

	MyID				= Flash_Data[0] << 8 | Flash_Data[1];

	STMFLASH_Read(FLASH_SAVE_ADDR + 2, (u16 *) Flash_Data, 1); //读取采集通道数据
	SetCVMD(Flash_Data);							//设置采集通道数据

	STMFLASH_Read(FLASH_SAVE_ADDR + 4, (u16 *) p, 8);

	if (*p == 0XFF)
	{
		if (* (p + 1) == 0XFF)
		{
			VolRate 			= 0.59604644775390625;
			STMFLASH_Write(FLASH_SAVE_ADDR+4, (u16 *)p, 8);
		}
	}

	p					= (u8 *) &VolCha;
	STMFLASH_Read(FLASH_SAVE_ADDR + 12, (u16 *) p, 8);

	if (*p == 0XFF)
	{
		if (* (p + 1) == 0XFF)
		{
			VolCha				= 0;
			STMFLASH_Write(FLASH_SAVE_ADDR+12, (u16 *)p, 8);
		}
	}

	LED_Init(); 									//LED 端口初始化 

	//	Key_Init(); 									//按键初始化
	USART1_Config(115200);

	//	USART_Seng_ID(MyID);
	printf("ID=%X\n", MyID);

	/* 配置CAN模块 */
	CAN_Config();
	Init_ADS1256_GPIO();							//初始化ADS1256 GPIO管脚 
	Delay_ms(100);
	GPIO_SetBits(GPIOB, GPIO_Pin_11);

	//	ADS1256_Init();
	G6A_Init();

	//	Vol_Calibrate_ByADR4525();						//	电压自校准
	//	printf("==============测试程序开始！==============\n");
	//	printf("My ID is %X\n", MyID);
	Can_Seng_ID(0x07, MyID);

	//	Vol_Git();
	//	GK_Test();
	//	VolRate = 2500000.0 / Git_Vol_ByAIN(VOL_ADR);
	LED0(ON);


	while (1)
	{
		//		LED0_Test();
		//		Key0_Test();
		//串口测试函数
		//		USART1_Test();
		//		Can_Text();
		//		USART1_Work();
		//		Can_Work();
	};
}


