

/*****************************************************************************
 版本:	1.0.0
 作者:	XRDS
 时间:	2018/5/1
 描述:	串口1配置，及提供外部接口函数
	USART1 Tx PA.09
	USART1 Rx PA.10
 主要外部接口函数如下：
	1、USART1_Config(115200); 串口1配置函数
	2、USART1_Char(); 串口1输出一个字节数据函数
	3、USART1_Printf(); 格式化输出，类似于C库中的printf，但这里没有用到C库
	4、USART1_Test();	串口1测试函数，将接收到的数据直接答应出来
 使用方法:	
	1、app_includes.h 开启或添加 usart1.h
	2、stm32f10x_conf.h 中开启 #include "stm32f10x_usart.h"
	3、添加 stm32f10x_usart.c 文件到项目中
	4、添加 usart1.h 头文件编译路径
	5、编写终端服务函数 USART1_IRQHandler()
	6、主函数添加下列声明
		//用volatile定义的变量会在程序外被改变,每次都必须从内存中读取，而不能重复使用
	放在cache或寄存器中的备份。
		volatile u8 	Usart1Buffer[20] = {0x00};
		volatile u8 	Usart1ReceiveState = 0; //串口1接收不定长度字符串结束标识符
		volatile u8 	Usart1ReceiveCounter = 0; //串口1接收到的字符串个数
	7、主函数调用 USART1_Config(115200);
	8、在主函数while中调用USART1_Test();测试发送接收数据是否一致
*****************************************************************************/
#include "usart1.h"

extern u8		Usart1Buffer[20];
extern u8		Usart1ReceiveState; //串口1接收不定长度字符串结束标识符
extern u8		Usart1ReceiveCounter; //串口1接收到的字符串个数

extern u16		MyID;
extern u8		SFlag;
extern long double VolRate;
extern u8		CMD; //电压通道
extern u8		VMD; //电流通道




/************************************************* 
 函数: USART1_Config
 描述: 串口1配置函数
 输入: 
 返回: 
 调用方法: 
	1、
*************************************************/
void USART1_Config(u32 USART1_BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* USART1 时钟配置 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* USART1 GPIO config */
	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 配置USART1工作模式 */
	USART_InitStructure.USART_BaudRate = USART1_BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);	//使能接收IDLE中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	//使能接收中断
	USART_Cmd(USART1, ENABLE);

	/* 配置USART1接收中断 */
	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* Enable the USARTy Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/************************************************* 
 函数: fputc
 描述: 重定向c库函数printf到USART1,
 输入: 
	1、ch 要输出的字节数据
	2、f 文件指针，此函数不需要使用
 返回: 
 调用方法: 
	1、直接使用c库函数printf，由printf调用
*************************************************/
int fputc(int ch, FILE * f)
{
	/* 将Printf内容发往串口 */
	USART_SendData(USART1, (unsigned char) ch);

	while (! (USART1->SR & USART_FLAG_TXE))
		;

	return (ch);
}


/************************************************* 
 函数: itoa
 描述: 将整形数据转换成字符串
 输入: 
	1、radix =10 表示10进制，其他结果为0
	2、value 要转换的整形数
	3、buf 转换后的字符串
	4、radix = 10
 返回: 
 调用方法: 
	1、直接被USART1_Printf()调用
*************************************************/
static char * itoa(int value, char * string, int radix)
{
	int 			i, d;
	int 			flag = 0;
	char *			ptr = string;

	/* This implementation only works for decimal numbers. */
	if (radix != 10)
	{
		*ptr				= 0;
		return string;
	}

	if (!value)
	{
		*ptr++				= 0x30;
		*ptr				= 0;
		return string;
	}

	/* if this is a negative value insert the minus sign. */
	if (value < 0)
	{
		*ptr++				= '-';

		/* Make the value positive. */
		value				*= -1;
	}

	for (i = 10000; i > 0; i /= 10)
	{
		d					= value / i;

		if (d || flag)
		{
			*ptr++				= (char) (d + 0x30);
			value				-= (d * i);
			flag				= 1;
		}
	}

	/* Null terminate the string. */
	*ptr				= 0;

	return string;

} /* NCL_Itoa */

/************************************************* 
 函数: USART1_Printf
 描述: 格式化输出，类似于C库中的printf，但这里没有用到C库
 输入: 
	1、Data	 要发送到串口的内容的指针
	2、... 	 其他参数
 返回: 
 调用方法: 典型应用
	1、USART1_Printf( USART1, "\r\n this is a demo \r\n" );
	2、USART1_Printf( USART1, "\r\n %d \r\n", i );
	3、USART1_Printf( USART1, "\r\n %s \r\n", j ); 
*************************************************/


void USART1_Printf(uint8_t * Data, ...)
{
	const char *	s;
	int 			d;
	char			buf[16];

	va_list 		ap;

	va_start(ap, Data);

	while (*Data != 0) // 判断是否到达字符串结束符
	{
		if (*Data == 0x5c) //'\'
		{
			switch (* ++Data)
			{
				case 'r': //回车符
					USART_SendData(USART1, 0x0d);
					Data++;
					break;

				case 'n': //换行符
					USART_SendData(USART1, 0x0a);
					Data++;
					break;

				default:
					Data++;
					break;
			}
		}
		else if (*Data == '%')
		{
			//
			switch (* ++Data)
			{
				case 's': //字符串
					s = va_arg(ap, const char *);

					for (; *s; s++)
					{
						USART_SendData(USART1, *s);

						while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
							;
					}

					Data++;
					break;

				case 'd': //十进制
					d = va_arg(ap, int);
					itoa(d, buf, 10);

					for (s = buf; *s; s++)
					{
						USART_SendData(USART1, *s);

						while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
							;
					}

					Data++;
					break;

				default:
					Data++;
					break;
			}
		} /* end of else if */
		else 
			USART_SendData(USART1, *Data++);

		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
			;
	}
}


/************************************************* 
 函数: USART1_Char
 描述: 串口1输出一个字节数据函数
 输入: 
	1、ch 要输出的字节数据
	2、
 返回: 
 调用方法: 
	1、USART1_Char(0x00);
*************************************************/
void USART1_Char(u8 ch)
{
	USART_SendData(USART1, (u8) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		;
}


/************************************************* 
 函数: USART1_IRQHandler()
 描述: 串口1中断服务函数，可接受不定长度字符串
 输入: 
	1、
	2、
 返回: 
 调用方法: 
	1、
*************************************************/
void USART1_IRQHandler(void)
{
	u8				usart1Clear = usart1Clear;		//定义该变量用于去除编辑器，未使用变量警告
	u8				i;

	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		if (Usart1ReceiveCounter == 0)
		{
			for (i = 0; i < 20; i++)
			{
				Usart1Buffer[i] 	= 0x00;
			}
		}

		Usart1Buffer[Usart1ReceiveCounter++] = USART_ReceiveData(USART1);

		//		USART1_Char(USART_ReceiveData(USART1));
	}
	else if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		usart1Clear 		= USART1->SR;			//读取SR寄存器
		usart1Clear 		= USART1->DR;			//读取DR寄存器（想读取SR，在读取DR，是为了清除IDLE中断）

		Usart1ReceiveState	= 1;


		USART1_Work();

		//		Can_Work();
	}

}


void USART1_Work(void)
{
	u8				com;
	u16 			temp;
	long double 	ldVolutage;
	u8				Flash_Data[2];
	u8 *			q;


	if (Usart1ReceiveState == 1) //如果接收到1帧数据
	{
		NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);		//失能CAN1消息接收中断
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE); //使能接收中断

		if (Usart1Buffer[0] == 0xAA)
		{
			com 				= Usart1Buffer[2];

			switch (com)
			{
				case 0x00: //设置ID指令+ 修改从机地址+修改后从机地址
					Can_Send_Data(0x00, Usart1Buffer + 3, 4);
					break;

				case 0x01: //向从机请求电压数据指令+从机地址
					CAN_Send(MyID, Usart1Buffer + 3, 2);
					break;

				case 0x02: //向从机请求电流数据指令+从机地址
					CAN_Send(MyID, Usart1Buffer + 3, 2);
					break;

				case 0x03: //向从机请求电压电流数据指令+从机地址
					CAN_Send(MyID, Usart1Buffer + 3, 2);
					break;

				case 0x04: //读取调试信息		
					GK_Test();
					break;

				case 0x11: //修改本机地址+修改后ID
					MyID = Usart1Buffer[3] << 8 | Usart1Buffer[4];
					Flash_Write_ID(MyID);
					USART_Seng_ID(MyID);
					break;

				case 0x12: //读取本机电压数据指令
					ldVolutage = Git_Vol_ByAIN(VMD) *VolRate;
					printf("%LfuV, %LfmV, %LfV\r\n", ldVolutage, ldVolutage / 1000, ldVolutage / 1000000);
					break;

				case 0x13: //读取本机电流数据指令
					ldVolutage = Git_Vol_ByAIN(CMD) *VolRate;
					printf("%LfuA, %LfmA, %LfA\r\n", ldVolutage, ldVolutage / 5000, ldVolutage / 5000000);
					break;

				case 0x14: //读取本机2.5标准电压指令
					ldVolutage = Git_Vol_ByAIN(VOL_ADR) *VolRate;
					printf("%LfuV, %LfmV, %LfV\r\n", ldVolutage, ldVolutage / 1000, ldVolutage / 1000000);
					break;

				case 0x15: //主机获取从机的ID指令
					Can_Send_Data(0x15, Usart1Buffer + 3, 2);
					break;

				case 0x16: //设置本机采集通道
					temp = ((0xFF - Usart1Buffer[3]) << 8) | (0xFF - Usart1Buffer[4]);
					Flash_Data[1] = temp & 0x00FF;
					Flash_Data[0] = temp >> 8 & 0x00FF;
					SetCVMD(Flash_Data);
					STMFLASH_Write(FLASH_SAVE_ADDR + 2, (u16 *) Flash_Data, 1);
					USART1_Char(0xAA);
					USART1_Char(0x05);
					USART1_Char(0x16);
					USART1_Char(Usart1Buffer[3]);
					USART1_Char(Usart1Buffer[4]);
					break;

				case 0x17: //设置本机AD倍率
					q = (u8 *) &VolRate;

					for (int i = 0; i < 8; i++)
					{
						*q					= Usart1Buffer[3 + i];
					}

					break;

				case 0x18: //外接2.5	v设置本机自校准
					VolRate = 2500000.0 / Git_Vol_ByAIN(VMD);
					printf("VolRate = %2.10Lf, V2.5 = %LfV\r\n", VolRate, Git_Vol_ByAIN(VOL_ADR) *VolRate / 1000000);
					break;

				case 0x20: //负向供电指令 + ID地址
					Can_Send_Data(0x20, Usart1Buffer + 3, 2);
					SFlag = 2;
					break;

				case 0x21: //正向供电功能（电流表）指令 + ID地址
					Can_Send_Data(0x21, Usart1Buffer + 3, 2);
					SFlag = 1;
					break;

				case 0x22: //检测（电压表）指令 + ID地址
					Can_Send_Data(0x22, Usart1Buffer + 3, 2);
					SFlag = 0;
					break;

				case 0x23: //获取正向供电（电流表）数据指令 + ID地址
					Can_Send_Data(0x23, Usart1Buffer + 3, 2);
					break;

				case 0x24: //获取检测（电压表）数据指令 + ID地址
					Can_Send_Data(0x24, Usart1Buffer + 3, 2);
					break;

				case 0x25: //获取数据（电压表、电流）数据指令 + ID地址
					Can_Send_Data(0x25, Usart1Buffer + 3, 2);
					break;

				case 0xEE: //调试使用软件复位指令
					SoftReset();
					break;

				case 0xFF: //软件复位指令
					Can_Send_Data(0xFF, Usart1Buffer + 3, 2);
					break;

				default:
					break;
			}
		}



		Usart1ReceiveState	= 0;
		Usart1ReceiveCounter = 0;
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //使能接收中断
		NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);		//使能CAN1数据接收中断
	}

}


////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************* 
 函数: USART1_Test()
 描述: 串口测试函数，将接收到的数据发送回去
 输入: 
	1、
	2、
 返回: 
 调用方法: 
	1、USART1_Test();
*************************************************/
void USART1_Test(void)
{
	u8				i	= 0;

	if (Usart1ReceiveState == 1) //如果接收到1帧数据
	{
		Usart1ReceiveState	= 0;

		while (Usart1ReceiveCounter--) // 把接收到数据发送回串口
		{
			USART1_Char(Usart1Buffer[i++]);
		}

		Usart1ReceiveCounter = 0;
	}
}


