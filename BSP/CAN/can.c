

/*****************************************************************************
 版本:	1.0.0
 作者:	XRDS
 时间:	2018/6/18
 描述:	
	1、使用引脚
		RX PA11
		TX PA12
	2、本程序允许接收大于8字节的数据包
		数据帧格式必须为： 标识符（0x50 0x55） + 总数据长度 +数据
		标识符 0x50 为小于等于8字节的数据帧，0x55 为大于8字节的数据帧
		如：
			1、50 07 01 02 03 04 05
			2、55 0E 01 02 03 04 05 06 07 08 09 10 11 12
 主要外部接口函数如下：
	1、CAN_Config();		CAN功能配置
	2、CAN_Send(0x1234, "12345678");		CAN通信报文内容发送
	3、Can_Text();	Can测试函数，将接收到的数据直接通过串口打印出来
 使用方法:	
	1、app_includes.h 开启或添加 #include "can.h"
	2、stm32f10x_conf.h 中开启 #include "stm32f10x_can.h"
	3、添加 stm32f10x_can.c 文件到项目中
	4、添加 can.h 头文件编译路径
	5、编写终端服务函数 USB_LP_CAN1_RX0_IRQHandler()
	6、主函数添加下列声明
		//用volatile定义的变量会在程序外被改变,每次都必须从内存中读取，而不能重复使用
	放在cache或寄存器中的备份。
		volatile u8 	CanReceiveState = 0; //串口1接收不定长度字符串结束标识符
		volatile u8 	CanReceiveCounter = 0; //串口1接收到的字符串个数
		volatile u8 	CanBuffer[20] = {0x00};
	7、主函数调用 CAN_Config();
	8、在主函数while中调用Can_Text();测试发送接收数据是否一致
*****************************************************************************/
#include "can.h"

CanTxMsg		TxMessage;
CanRxMsg		RxMessage;

// CAN 数据变量定义
extern u8		CanReceiveState; //CAN接收不定长度字符串结束标识符
extern u8		CanReceiveCounter; //CAN接收到的字符串个数
extern u8		CanBuffer[20];

extern u16		MyID;
extern u8		SFlag;
extern u8		CMD; //电压通道
extern u8		VMD; //电流通道

extern long double VolRate;
extern long double VolCha;

extern long double ADC_Rate;
extern long double Rev_Rate;
extern long double Vol_Rate;
extern long double Rev0_0;
extern long double Rev0_1;



/************************************************* 
 函数: CAN_NVIC_Config(void)
 描述: CAN的NVIC 配置,第1优先级组，0，0优先级
 输入: 
	1、
	2、
 返回: 
 调用方法: 
	1、内部调用
*************************************************/
static void CAN_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/*中断设置*/
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn; //CAN1 RX0中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //子优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/************************************************* 
 函数: CAN_GPIO_Config(void)
 描述: CAN的GPIO 配置,	PB8上拉输入RX，PB9推挽输出TX,
 输入: 
	1、
	2、
 返回: 
 调用方法: 
	1、内部调用
*************************************************/
static void CAN_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*外设时钟设置*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

	/* Configure CAN pin: RX PA11*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	// 上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure CAN pin: TX PA12 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

}


/************************************************* 
 函数: CAN_Mode_Config(void)
 描述: CAN的模式 配置
 输入: 
	1、
	2、
 返回: 
 调用方法: 
	1、内部调用
*************************************************/
static void CAN_Mode_Config(void)
{
	CAN_InitTypeDef CAN_InitStructure;

	/************************CAN通信参数设置**********************************/
	/*CAN寄存器初始化*/
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);

	/*CAN单元初始化*/
	CAN_InitStructure.CAN_TTCM = DISABLE;			//MCR-TTCM	关闭时间触发通信模式使能
	CAN_InitStructure.CAN_ABOM = ENABLE;			//MCR-ABOM	自动离线管理
	CAN_InitStructure.CAN_AWUM = ENABLE;			//MCR-AWUM	使用自动唤醒模式
	CAN_InitStructure.CAN_NART = DISABLE;			//MCR-NART	禁止报文自动重传	  DISABLE-自动重传
	CAN_InitStructure.CAN_RFLM = DISABLE;			//MCR-RFLM	接收FIFO 锁定模式  DISABLE-溢出时新报文会覆盖原有报文
	CAN_InitStructure.CAN_TXFP = DISABLE;			//MCR-TXFP	发送FIFO优先级 DISABLE-优先级取决于报文标示符
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	//正常工作模式
	CAN_InitStructure.CAN_SJW = CAN_SJW_2tq;		//BTR-SJW 重新同步跳跃宽度 2个时间单元
	CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;		//BTR-TS1 时间段1 占用了6个时间单元
	CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;		//BTR-TS1 时间段2 占用了3个时间单元
	CAN_InitStructure.CAN_Prescaler = 4;			////BTR-BRP 波特率分频器	定义了时间单元的时间长度 36/(1+6+3)/4=0.9Mbps
	CAN_Init(CAN1, &CAN_InitStructure);
}


/************************************************* 
 函数: CAN_Filter_Config(u32 filterID, u16 maskIdHigh, u16 maskIdLow)
 描述: CAN的过滤器 配置
 输入: 
	1、filterID 过滤器允许通过的地址
	2、maskIdHigh 选择高16位每位必须过滤匹配情况；对应为，为1时需要匹配
	3、maskIdLow 选择低16位每位必须过滤匹配情况；对应为，为1时需要匹配
 返回: 
 调用方法: 
	1、内部调用
*************************************************/
static void CAN_Filter_Config(u32 filterID, u16 maskIdHigh, u16 maskIdLow)
{
	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	/*CAN过滤器初始化*/
	CAN_FilterInitStructure.CAN_FilterNumber = 0;	//过滤器组0
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask; //工作在标识符屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //过滤器位宽为单个32位。

	// 使能报文标示符过滤器按照标示符的内容进行比对过滤，扩展ID不是如下的就抛弃掉，是的话，会存入FIFO0。
	CAN_FilterInitStructure.CAN_FilterIdHigh = (((u32) filterID << 3) & 0xFFFF0000) >> 16; //要过滤的ID高位
	CAN_FilterInitStructure.CAN_FilterIdLow = (((u32) filterID << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0xFFFF; //要过滤的ID低位
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = maskIdHigh; //选择高16位每位必须过滤匹配情况
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = maskIdLow; //选择低16位每位必须过滤匹配情况
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; //过滤器被关联到FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; //使能过滤器
	CAN_FilterInit(&CAN_FilterInitStructure);

	/*CAN通信中断使能*/
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}


/************************************************* 
 函数: CAN_Config(void)
 描述: CAN功能配置
 输入: 
	1、
	2、
 返回: 
 调用方法: 
	1、外部调用
*************************************************/
void CAN_Config(void)
{
	CAN_GPIO_Config();
	CAN_NVIC_Config();
	CAN_Mode_Config();

	if (MyID == 0x0000)
	{
		CAN_Filter_Config(0x0000, 0x0000, 0x0000);

	}
	else 
	{
		CAN_Filter_Config(0x0000, 0xFFFF, 0xFFFF);

	}

	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);			//使能CAN1数据接收中断
}


/************************************************* 
 函数: CAN_Send(u32 ExtId, char * str)
 描述: CAN通信报文内容发送
 输入: 
	1、ExtId 发送时的ID
	2、str 发送数据的首地址指针
 返回: 
 调用方法: 
	1、CAN_Send(0x1234, "12345678");
*************************************************/
void CAN_Send(u32 ExtId, u8 * str, u8 len)
{
	__IO uint32_t	i	= 0;
	__IO uint8_t	TransmitMailbox = 0;
	u8				n, k, j;

	//TxMessage.StdId=0x00;
	TxMessage.ExtId 	= ExtId;					//使用的扩展ID
	TxMessage.IDE		= CAN_ID_EXT;				//扩展模式
	TxMessage.RTR		= CAN_RTR_DATA; 			//发送的是数据

	if (len < 8)
	{
		TxMessage.DLC		= len;					//设定待传输消息的帧长度

		for (i = 0; i < len; i++)
		{
			TxMessage.Data[i]	= *str++;			// 包含待传输数据
		}

		TransmitMailbox 	= CAN_Transmit(CAN1, &TxMessage); //开始一个消息的传输 

		i					= 0;

		while ((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK) && (i != 0xFF)) //通过检查CANTXOK位来确认发送是否成功 
		{
			i++;
		}
	}
	else 
	{
		TxMessage.DLC		= 8;					//设定待传输消息的帧长度

		for (i = 0; i < 8; i++)
		{
			TxMessage.Data[i]	= *str++;			// 包含待传输数据
		}

		TransmitMailbox 	= CAN_Transmit(CAN1, &TxMessage); //开始一个消息的传输 

		i					= 0;

		while ((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK) && (i != 0xFF)) //通过检查CANTXOK位来确认发送是否成功 
		{
			i++;
		}

		n					= len / 8;

		for (k = 0; k < n; k++)
		{
			if (n - k < 2)
			{
				TxMessage.DLC		= len % 8;

				for (j = 0; j < len % 8; j++)
				{
					TxMessage.Data[j]	= *str++;
				}

				TransmitMailbox 	= CAN_Transmit(CAN1, &TxMessage); //开始一个消息的传输 
				i					= 0;

				while ((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK) && (i != 0xFF)) //通过检查CANTXOK位来确认发送是否成功 
				{
					i++;
				}
			}
			else 
			{
				TxMessage.DLC		= 8;

				for (j = 0; j < 8; j++)
				{
					TxMessage.Data[j]	= *str++;
				}

				TransmitMailbox 	= CAN_Transmit(CAN1, &TxMessage); //开始一个消息的传输 
				i					= 0;

				while ((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK) && (i != 0xFF)) //通过检查CANTXOK位来确认发送是否成功 
				{
					i++;
				}
			}
		}
	}
}


void CAN_Send_VC(u32 ExtId, u8 * str)
{
	__IO uint32_t	i	= 0;
	__IO uint8_t	TransmitMailbox = 0;

	TxMessage.ExtId 	= ExtId;					//使用的扩展ID
	TxMessage.IDE		= CAN_ID_EXT;				//扩展模式
	TxMessage.RTR		= CAN_RTR_DATA; 			//发送的是数据
	TxMessage.DLC		= 8;						//设定待传输消息的帧长度

	for (i = 0; i < 8; i++)
	{
		TxMessage.Data[i]	= *str++;				// 包含待传输数据
	}

	TransmitMailbox 	= CAN_Transmit(CAN1, &TxMessage); //开始一个消息的传输 

	i					= 0;

	while ((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK) && (i != 0xFF)) //通过检查CANTXOK位来确认发送是否成功 
	{
		i++;
	}

}


/************************************************* 
 函数: USB_LP_CAN1_RX0_IRQHandler(void)
 描述: CAN1接收中断服务函数，允许接收大于8字节的数据包
		数据帧格式必须为： 标识符（0x50 0x55） + 总数据长度 +数据
		标识符 0x50 为小于等于8字节的数据帧，0x55 为大于8字节的数据帧
		如：
			1、50 07 01 02 03 04 05
			2、55 0E 01 02 03 04 05 06 07 08 09 10 11 12
 
 调用方法: 
	1、中断调用
*************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	u8				i	= 0;
	u8				id	= 0;
	u16 			sId = 0;
	long double 	ldVol = 0.59604644775390625;
	u8 *			q;

	NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);			//失能CAN1消息接收中断

	//从邮箱中读出报文
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

	if (CanReceiveCounter == 0)
	{
		id					= (u8) (RxMessage.ExtId >> 16);

		if (id == 0x04) //接收电压数据
		{
			/*
				USART1_Char(0xAA);
				USART1_Char(0x0D);
				USART1_Char(id);
				USART1_Char((u8) ((RxMessage.ExtId & 0x0000FF00) >> 8));
				USART1_Char((u8) (RxMessage.ExtId & 0x000000FF));

				for (i = 0; i < 8; i++)
				{
					USART1_Char(RxMessage.Data[i]);
				}
			*/
			sId 				= RxMessage.ExtId & 0x0000FFFF;
			q					= (u8 *) &ldVol;

			for (i = 0; i < 8; i++)
			{
				*q					= RxMessage.Data[i];
				q++;

			}

			//			ldVol = 3300000.0;
			printf("ID=%X Data=%LfuV\n", sId, ldVol);
			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//使能CAN1数据接收中断
		}
		else if (id == 0x05) //接收电流数据
		{
			sId 				= RxMessage.ExtId & 0x0000FFFF;
			q					= (u8 *) &ldVol;

			for (i = 0; i < 8; i++)
			{
				*q					= RxMessage.Data[i];
				q++;

			}

			//			ldVol = 3300000.0;
			printf("ID=%X Data=%LfuA\n", sId, ldVol);
			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//使能CAN1数据接收中断
		}
		else 
		{
			if (RxMessage.Data[0] == 0x50)
			{
				for (i = 0; i < RxMessage.DLC; i++)
					CanBuffer[i] = RxMessage.Data[i];

				CanReceiveState 	= 1;			//接收成功	
				CanReceiveCounter	= 0;

				//			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//使能CAN1数据接收中断
			}
			else if (RxMessage.Data[0] == 0x55)
			{
				CanReceiveState 	= 0;

				for (i = 0; i < RxMessage.DLC; i++)
				{
					CanBuffer[i]		= RxMessage.Data[i];
					CanReceiveCounter++;
				}

				CAN_Filter_Config(RxMessage.ExtId, 0xFFFF, 0xFFFF);
				NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn); //使能CAN1数据接收中断
			}
			else 
			{
				CanReceiveState 	= 0;			//接收失败
				CanReceiveCounter	= 0;
				CAN_Config();
				NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn); //使能CAN1数据接收中断
			}
		}

	}
	else if (CanBuffer[0] == 0x55 && CanReceiveCounter != 0)
	{
		for (i = 0; i < RxMessage.DLC; i++)
		{
			CanBuffer[CanReceiveCounter++] = RxMessage.Data[i];

		}

		if (CanReceiveCounter == CanBuffer[1])
		{
			CanReceiveState 	= 1;				//接收成功	
			CanReceiveCounter	= 0;
			CAN_Config();

			//			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//使能CAN1数据接收中断
		}
		else if (CanReceiveCounter < CanBuffer[1])
		{
			CAN_Filter_Config(RxMessage.ExtId, 0xFFFF, 0xFFFF);
			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//使能CAN1数据接收中断
		}
		else 
		{
			CanReceiveState 	= 0;				//接收失败	
			CanReceiveCounter	= 0;
			CAN_Config();
			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//使能CAN1数据接收中断
		}
	}

	//	USART1_Work();
	Can_Work();
}


/************************************************* 
 函数: USB_LP_CAN1_RX0_IRQHandler(void)
 描述: 简单的CAN1接收中断服务函数，直接将接收到的数据打印出来
 输入: 
	1、
	2、
 返回: 
 调用方法: 
	1、中断调用
*************************************************/
/*void USB_LP_CAN1_RX0_IRQHandler(void)
{
	u8				i	= 0;

	NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);			//失能CAN1消息接收中断

	//从邮箱中读出报文
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

	//将接收到的数据通过串口打印出来
	for (i = 0; i < RxMessage.DLC; i++)
		printf("%c", RxMessage.Data[i]);

	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);			//使能CAN1数据接收中断
}*/
void Can_Work(void)
{
	u8				CAN_data_com;
	u16 			ID_1, ID_2;
	long double 	temp1, temp2, ldVolutage;
	u8 *			p;
	u32 			tempId;

	if (CanReceiveState == 1)
	{
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE); //失能接收中断
		NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);		//失能CAN1消息接收中断

		if (CanBuffer[0] == 0x50 || CanBuffer[0] == 0x55)
		{
			CAN_data_com		= CanBuffer[2];
			ID_1				= CanBuffer[3] << 8 | CanBuffer[4];
			ID_2				= CanBuffer[5] << 8 | CanBuffer[6];

			switch (CAN_data_com)
			{
				case 0x00: //主机向从机发送设置ID指令+ 修改从机地址+修改后从机地址
					if (MyID == ID_1 || ID_1 == 0xFFFF)
					{
						Flash_Write_ID(ID_2);
						Can_Seng_ID(0x07, MyID);
					}

					break;

				case 0x01: //主机 向从机请求电压数据指令+从机地址
					if (MyID == ID_1 || ID_1 == 0x8000)
					{
						ldVolutage			= Git_Vol_ByAIN(VMD) *VolRate;
						p					= (u8 *) &ldVolutage;
						Can_Send_Data(0x04, p, 2);

						//						printf("%LfuV, %LfmV, %LfV\r\n", ldVolutage, ldVolutage / 1000, ldVolutage / 1000000);
					}

					break;

				case 0x02: //主机向从机请求电流数据指令+从机地址 
					if (MyID == ID_1 || ID_1 == 0x8000)
					{
						ldVolutage			= Git_Vol_ByAIN(CMD) *VolRate / 10;
						p					= (u8 *) &ldVolutage;
						Can_Send_Data(0x05, p, 2);
					}

					break;

				//				case 0x04: //主机接收从机电压数据
				//					CanBuffer[0] = 0xAA;
				//					for (i = 0; i < CanBuffer[1]; i++)
				//					{
				//						USART1_Char(CanBuffer[i]);
				//					}
				//					//					printf("%LfuV, %LfmV, %LfV\r\n", ldVolutage, ldVolutage / 1000, ldVolutage / 1000000);
				//					break;
				//				case 0x05: //主机接收从机电流数据
				//					CanBuffer[0] = 0xAA;
				//					for (i = 0; i < CanBuffer[1]; i++)
				//					{
				//						USART1_Char(CanBuffer[i]);
				//					}
				//					//					printf("The Curr = %Lf mA\r\n", ldVolutage);
				//					break;
				case 0x07: //主机接收从机发送请求ID指令
					//					USART_Seng_ID(ID_1);
					printf("ID=%X\n", ID_1);
					break;

				case 0x08: //（正向供电设置成功指令）主机发送电脑接收到的下位机ID CAN ID+接收ID

					/*
					USART1_Char(0xAA);
					USART1_Char(0x05);
					USART1_Char(0x08);
					USART1_Char((u8) (ID_1 >> 8));
					USART1_Char((u8) ID_1);
					*/
					printf("ID=%X + \n", ID_1);
					break;

				case 0x09: //（负向供电设置成功指令）主机发送电脑接收到的下位机ID CAN ID+接收ID

					/*
					USART1_Char(0xAA);
					USART1_Char(0x05);
					USART1_Char(0x09);
					USART1_Char((u8) (ID_1 >> 8));
					USART1_Char((u8) ID_1);
					*/
					printf("ID=%X - \n", ID_1);
					break;

				case 0x0A: //（检测设置成功指令）主机发送电脑接收到的下位机ID CAN ID+接收ID

					/*
					USART1_Char(0xAA);
					USART1_Char(0x05);
					USART1_Char(0x0A);
					USART1_Char((u8) (ID_1 >> 8));
					USART1_Char((u8) ID_1);
					*/
					printf("ID=%X = \n", ID_1);
					break;

				case 0x15: //主机要求从机发送自己的ID
					if (MyID == ID_1 || ID_1 == 0x8000)
					{
						Can_Seng_ID(0x07, MyID);
					}

					break;

				case 0x20: //负向供电指令 + ID地址
					if (MyID == ID_1 || ID_1 == 0x8000)
					{
						G6A_Vol(OFF);
						G6A_Cur(OFF);
						PN_PP_EN(ON);
						P_D_EN(ON);
						Can_Seng_ID(0x09, MyID);
						SFlag				= 2;

					}

					break;

				case 0x21: //正向供电功能（电流表）指令 + ID地址
					if (MyID == ID_1 || ID_1 == 0x8000)
					{
						PN_PP_EN(OFF);
						P_D_EN(ON);
						Can_Seng_ID(0x08, MyID);
						SFlag				= 1;
					}

					break;

				case 0x22: //检测（电压表）指令 + ID地址
					if (MyID == ID_1 || ID_1 == 0x8000)
					{
						PN_PP_EN(OFF);
						P_D_EN(OFF);
						Can_Seng_ID(0x0A, MyID);
						SFlag				= 0;
					}

					break;

				case 0x23: //获取正向供电（电流表）数据指令 + ID地址
					if (MyID == ID_1 || ID_1 == 0x8000)
					{
						if (SFlag == 1)
						{
							G6A_Cur(ON);
							Delay_ms(500);

							//							VolRate 			= 2500000.0 / Git_Vol_ByAIN(VOL_ADR); //检测前自校验
							ldVolutage			= Git_Vol_ByAIN(CMD) *VolRate / 5;

							//							Can_Send_Data(0x05, (u8 *) &ldVolutage, 8);
							tempId				= 0x00050000 | MyID;
							CAN_Send_VC(tempId, (u8 *) &ldVolutage);
							printf("%LfuA, %LfmA, %LfA\r\n", ldVolutage, ldVolutage / 1000, ldVolutage / 1000000);

							G6A_Cur(OFF);

						}
						else if (SFlag == 2)
						{
							ldVolutage			= 0.0;
							tempId				= 0x00050000 | MyID;
							CAN_Send_VC(tempId, (u8 *) &ldVolutage);

						}
					}

					break;

				case 0x24: //获取检测（电压表）数据指令 + ID地址
					if (MyID == ID_1 || ID_1 == 0x8000)
					{
						if (SFlag == 0)
						{
							//							G6A_Vol(ON);
							//							Delay_ms(500);
							//							//							VolRate 			= 2500000.0 / Git_Vol_ByAIN(VOL_ADR); //检测前自校验
							//							temp1				= Git_Vol_ByAIN(VMD);
							//							temp2				= Git_Vol_ByAIN(VOL_VIN2);
							//							ldVolutage			= ((temp1 - temp2) *VolRate) -VolCha;
							//							//							Can_Send_Data(0x04, (u8 *) &ldVolutage, 8);
							//							tempId				= 0x00040000 | MyID;
							//							CAN_Send_VC(tempId, (u8 *) &ldVolutage);
							//							printf("%LfuV, %LfmV, %LfV\r\n", ldVolutage, ldVolutage / 1000, ldVolutage / 1000000);
							//							G6A_Vol(OFF);
							ldVolutage			= Vol_Meas();

							printf("v%.9Lfv, m%.6Lfm, u%.3Lfu\r\n", ldVolutage / 1000000, ldVolutage / 1000,
								 ldVolutage);		//double
						}
					}

					break;

				case 0x25: //获取数据（电压表、电流）数据指令 + ID地址
					if (MyID == ID_1 || ID_1 == 0x8000)
					{
						if (SFlag == 0)
						{
							G6A_Vol(ON);
							Delay_ms(500);

							//							VolRate 			= 2500000.0 / Git_Vol_ByAIN(VOL_ADR); //检测前自校验
							temp1				= Git_Vol_ByAIN(VMD);
							temp2				= Git_Vol_ByAIN(VOL_VIN2);
							ldVolutage			= ((temp1 - temp2) *VolRate) -VolCha;

							//							Can_Send_Data(0x04, (u8 *) &ldVolutage, 8);
							tempId				= 0x00040000 | MyID;
							CAN_Send_VC(tempId, (u8 *) &ldVolutage);
							printf("%LfuV, %LfmV, %LfV\r\n", ldVolutage, ldVolutage / 1000, ldVolutage / 1000000);
							G6A_Vol(OFF);

						}
						else if (SFlag == 1)
						{
							G6A_Cur(ON);
							Delay_ms(500);

							//							VolRate 			= 2500000.0 / Git_Vol_ByAIN(VOL_ADR); //检测前自校验
							ldVolutage			= Git_Vol_ByAIN(CMD) *VolRate / 5;

							//							Can_Send_Data(0x05, (u8 *) &ldVolutage, 8);
							tempId				= 0x00050000 | MyID;
							CAN_Send_VC(tempId, (u8 *) &ldVolutage);
							printf("%LfuA, %LfmA, %LfA\r\n", ldVolutage, ldVolutage / 1000, ldVolutage / 1000000);

							G6A_Cur(OFF);

						}
						else if (SFlag == 2)
						{
							ldVolutage			= 0.0;
							tempId				= 0x00050000 | MyID;
							CAN_Send_VC(tempId, (u8 *) &ldVolutage);

						}
					}

					break;

				case 0xFF: //软件复位指令  FF+复位ID
					if (MyID == ID_1 || ID_1 == 0x8000)
					{
						SoftReset();
					}

					break;

				default:
					break;
			}
		}

		CanReceiveState 	= 0;
		NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);		//使能CAN1数据接收中断
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //使能接收中断

	}

}


////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************* 
 函数: Can_Text(void)
 描述: Can测试函数，将接收到的数据直接通过串口打印出来
 输入: 
	1、
	2、
 返回: 
 调用方法: 
	1、在主函数 while 中调用 Can_Text();
*************************************************/
void Can_Text(void)
{
	u8				i	= 0;

	if (CanReceiveState == 1)
	{
		for (i = 0; i < CanBuffer[1]; i++)
			printf("%c", CanBuffer[i]);

		CanReceiveState 	= 0;
	}
}



