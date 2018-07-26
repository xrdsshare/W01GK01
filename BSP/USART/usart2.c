

/*****************************************************************************
 版本:	1.0.0
 作者:	XRDS
 时间:	2018/5/1
 描述:	串口2配置，及提供外部接口函数
	USART2 Tx PA.02
	USART2 Rx PA.03
 主要外部接口函数如下：
	1、USART2_Config(115200); 串口1配置函数
	2、USART2_Char(); 串口2输出一个字节数据函数
	3、USART2_Printf(); 格式化输出，类似于C库中的printf，但这里没有用到C库
	4、USART2_Test();	串口2测试函数，将接收到的数据直接答应出来
 使用方法:	
	1、app_includes.h 开启或添加 usart2.h
	2、stm32f10x_conf.h 中开启 #include "stm32f10x_usart.h"
	3、添加 stm32f10x_usart.c 文件到项目中
	4、添加 usart2.h 头文件编译路径
	5、编写终端服务函数 USART2_IRQHandler()
	6、主函数添加下列声明
		//用volatile定义的变量会在程序外被改变,每次都必须从内存中读取，而不能重复使用放在cache或寄存器中的备份。
		volatile u8 	Usart2Buffer[20] = {0x00};
		volatile u8 	Usart2ReceiveState = 0; //串口2接收不定长度字符串结束标识符
		volatile u8 	Usart2ReceiveCounter = 0; //串口2接收到的字符串个数
	7、主函数调用 USART2_Config(115200);
	8、在主函数while中调用USART2_Test();测试发送接收数据是否一致
*****************************************************************************/
#include "usart2.h"

extern u8		Usart2Buffer[20];
extern u8		Usart2ReceiveState; //串口1接收不定长度字符串结束标识符
extern u8		Usart2ReceiveCounter; //串口1接收到的字符串个数



/************************************************* 
 函数: USART2_Config
 描述: 串口2配置函数
 输入: 
 返回: 
 调用方法: 
	1、
*************************************************/
void USART2_Config(u32 USART2_BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* config USART2 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 

	/* USART2 GPIO config */
	/* Configure USART2 Tx (PA.02) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Configure USART2 Rx (PA.03) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* USART2 mode config */
	USART_InitStructure.USART_BaudRate = USART2_BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);	//使能接收IDLE中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);                    //使能接收中断
	USART_Cmd(USART2, ENABLE);

	/* 配置USART2接收中断 */
	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* Enable the USARTy Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/************************************************* 
 函数: fputc
 描述: 重定向c库函数printf到USART2,
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
	USART_SendData(USART2, (unsigned char) ch);

	while (! (USART2->SR & USART_FLAG_TXE))
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
	1、直接被USART2_Printf()调用
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
 函数: USART2_Printf
 描述: 格式化输出，类似于C库中的printf，但这里没有用到C库
 输入: 
	1、Data	 要发送到串口的内容的指针
	2、... 	 其他参数
 返回: 
 调用方法: 典型应用
	1、USART2_Printf( USART2, "\r\n this is a demo \r\n" );
	2、USART2_Printf( USART2, "\r\n %d \r\n", i );
	3、USART2_Printf( USART2, "\r\n %s \r\n", j ); 
*************************************************/


void USART2_Printf(uint8_t * Data, ...)
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
					USART_SendData(USART2, 0x0d);
					Data++;
					break;

				case 'n': //换行符
					USART_SendData(USART2, 0x0a);
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
						USART_SendData(USART2, *s);

						while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
							;
					}

					Data++;
					break;

				case 'd': //十进制
					d = va_arg(ap, int);
					itoa(d, buf, 10);

					for (s = buf; *s; s++)
					{
						USART_SendData(USART2, *s);

						while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
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
			USART_SendData(USART2, *Data++);

		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
			;
	}
}


/************************************************* 
 函数: USART2_Char
 描述: 串口1输出一个字节数据函数
 输入: 
	1、ch 要输出的字节数据
	2、
 返回: 
 调用方法: 
	1、USART2_Char(0x00);
*************************************************/
void USART2_Char(char ch)
{
	USART_SendData(USART2, (unsigned char) ch);

	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
		;
}


/************************************************* 
 函数: USART2_IRQHandler()
 描述: 串口1中断服务函数，可接受不定长度字符串
 输入: 
	1、
	2、
 返回: 
 调用方法: 
	1、
*************************************************/
void USART2_IRQHandler(void)
{
	u8				usart2Clear = usart2Clear;		//定义该变量用于去除编辑器，未使用变量警告

	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		Usart2Buffer[Usart2ReceiveCounter++] = USART_ReceiveData(USART2);
	}
	else if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		usart2Clear 		= USART2->SR;			//读取SR寄存器
		usart2Clear 		= USART2->DR;			//读取DR寄存器（想读取SR，在读取DR，是为了清除IDLE中断）
		Usart2ReceiveState	= 1;
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////


/************************************************* 
 函数: USART2_Test()
 描述: 串口测试函数，将接收到的数据发送回去
 输入: 
	1、
	2、
 返回: 
 调用方法: 
	1、USART2_Test();
*************************************************/
void USART2_Test(void)
{
	u8				i	= 0;

	if (Usart2ReceiveState == 1) //如果接收到1帧数据
	{
		Usart2ReceiveState	= 0;

		while (Usart2ReceiveCounter--) // 把接收到数据发送回串口
		{
			USART2_Char(Usart2Buffer[i++]);
		}

		Usart2ReceiveCounter = 0;
	}
}


