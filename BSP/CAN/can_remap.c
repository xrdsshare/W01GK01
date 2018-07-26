

/*****************************************************************************
 �汾:	1.0.0
 ����:	XRDS
 ʱ��:	2018/6/18
 ����:	
	1��ʹ������
		RX PB8
		TX PB9
	2��������������մ���8�ֽڵ����ݰ�
		����֡��ʽ����Ϊ�� ��ʶ����0x50 0x55�� + �����ݳ��� +����
		��ʶ�� 0x50 ΪС�ڵ���8�ֽڵ�����֡��0x55 Ϊ����8�ֽڵ�����֡
		�磺
			1��50 07 01 02 03 04 05
			2��55 0E 01 02 03 04 05 06 07 08 09 10 11 12
 ��Ҫ�ⲿ�ӿں������£�
	1��CAN_Config();		CAN��������
	2��CAN_Send(0x1234, "12345678");		CANͨ�ű������ݷ���
	3��Can_Text();	Can���Ժ����������յ�������ֱ��ͨ�����ڴ�ӡ����
 ʹ�÷���:	
	1��app_includes.h ��������� #include "can_remap.h"
	2��stm32f10x_conf.h �п��� #include "stm32f10x_can.h"
	3����� stm32f10x_can.c �ļ�����Ŀ��
	4����� can_remap.h ͷ�ļ�����·��
	5����д�ն˷����� USB_LP_CAN1_RX0_IRQHandler()
	6�������������������
		//��volatile����ı������ڳ����ⱻ�ı�,ÿ�ζ�������ڴ��ж�ȡ���������ظ�ʹ��
	����cache��Ĵ����еı��ݡ�
		volatile u8 	Usart1ReceiveState = 0; //����1���ղ��������ַ���������ʶ��
		volatile u8 	Usart1ReceiveCounter = 0; //����1���յ����ַ�������
		volatile u8 	Usart1Buffer[20] = {0x00};
	7������������ CAN_Config();
	8����������while�е���Can_Text();���Է��ͽ��������Ƿ�һ��
*****************************************************************************/
#include "can_remap.h"

CanTxMsg		TxMessage;
CanRxMsg		RxMessage;

// CAN ���ݱ�������
extern u8		CanReceiveState; //CAN���ղ��������ַ���������ʶ��
extern u8		CanReceiveCounter; //CAN���յ����ַ�������
extern u8		CanBuffer[20];


/************************************************* 
 ����: CAN_NVIC_Config(void)
 ����: CAN��NVIC ����,��1���ȼ��飬0��0���ȼ�
 ����: 
	1��
	2��
 ����: 
 ���÷���: 
	1���ڲ�����
*************************************************/
static void CAN_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/*�ж�����*/
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn; //CAN1 RX0�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //�����ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/************************************************* 
 ����: CAN_GPIO_Config(void)
 ����: CAN��GPIO ����,	PB8��������RX��PB9�������TX,
 ����: 
	1��
	2��
 ����: 
 ���÷���: 
	1���ڲ�����
*************************************************/
static void CAN_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*����ʱ������*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

	/*IO��������*/
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);

	/* Configure CAN pin: RX PB8*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	// ��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure CAN pin: TX PB9 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // �����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}


/************************************************* 
 ����: CAN_Mode_Config(void)
 ����: CAN��ģʽ ����
 ����: 
	1��
	2��
 ����: 
 ���÷���: 
	1���ڲ�����
*************************************************/
static void CAN_Mode_Config(void)
{
	CAN_InitTypeDef CAN_InitStructure;

	/************************CANͨ�Ų�������**********************************/
	/*CAN�Ĵ�����ʼ��*/
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);

	/*CAN��Ԫ��ʼ��*/
	CAN_InitStructure.CAN_TTCM = DISABLE;			//MCR-TTCM	�ر�ʱ�䴥��ͨ��ģʽʹ��
	CAN_InitStructure.CAN_ABOM = ENABLE;			//MCR-ABOM	�Զ����߹���
	CAN_InitStructure.CAN_AWUM = ENABLE;			//MCR-AWUM	ʹ���Զ�����ģʽ
	CAN_InitStructure.CAN_NART = DISABLE;			//MCR-NART	��ֹ�����Զ��ش�	  DISABLE-�Զ��ش�
	CAN_InitStructure.CAN_RFLM = DISABLE;			//MCR-RFLM	����FIFO ����ģʽ  DISABLE-���ʱ�±��ĻḲ��ԭ�б���
	CAN_InitStructure.CAN_TXFP = DISABLE;			//MCR-TXFP	����FIFO���ȼ� DISABLE-���ȼ�ȡ���ڱ��ı�ʾ��
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	//��������ģʽ
	CAN_InitStructure.CAN_SJW = CAN_SJW_2tq;		//BTR-SJW ����ͬ����Ծ��� 2��ʱ�䵥Ԫ
	CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;		//BTR-TS1 ʱ���1 ռ����6��ʱ�䵥Ԫ
	CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;		//BTR-TS1 ʱ���2 ռ����3��ʱ�䵥Ԫ
	CAN_InitStructure.CAN_Prescaler = 4;			////BTR-BRP �����ʷ�Ƶ��	������ʱ�䵥Ԫ��ʱ�䳤�� 36/(1+6+3)/4=0.9Mbps
	CAN_Init(CAN1, &CAN_InitStructure);
}


/************************************************* 
 ����: CAN_Filter_Config(u32 filterID, u16 maskIdHigh, u16 maskIdLow)
 ����: CAN�Ĺ����� ����
 ����: 
	1��filterID ����������ͨ���ĵ�ַ
	2��maskIdHigh ѡ���16λÿλ�������ƥ���������ӦΪ��Ϊ1ʱ��Ҫƥ��
	3��maskIdLow ѡ���16λÿλ�������ƥ���������ӦΪ��Ϊ1ʱ��Ҫƥ��
 ����: 
 ���÷���: 
	1���ڲ�����
*************************************************/
static void CAN_Filter_Config(u32 filterID, u16 maskIdHigh, u16 maskIdLow)
{
	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	/*CAN��������ʼ��*/
	CAN_FilterInitStructure.CAN_FilterNumber = 0;	//��������0
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask; //�����ڱ�ʶ������λģʽ
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //������λ��Ϊ����32λ��

	// ʹ�ܱ��ı�ʾ�����������ձ�ʾ�������ݽ��бȶԹ��ˣ���չID�������µľ����������ǵĻ��������FIFO0��
	CAN_FilterInitStructure.CAN_FilterIdHigh = (((u32) filterID << 3) & 0xFFFF0000) >> 16; //Ҫ���˵�ID��λ
	CAN_FilterInitStructure.CAN_FilterIdLow = (((u32) filterID << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0xFFFF; //Ҫ���˵�ID��λ
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = maskIdHigh; //ѡ���16λÿλ�������ƥ�����
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = maskIdLow; //ѡ���16λÿλ�������ƥ�����
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; //��������������FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; //ʹ�ܹ�����
	CAN_FilterInit(&CAN_FilterInitStructure);

	/*CANͨ���ж�ʹ��*/
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}


/************************************************* 
 ����: CAN_Config(void)
 ����: CAN��������
 ����: 
	1��
	2��
 ����: 
 ���÷���: 
	1���ⲿ����
*************************************************/
void CAN_Config(void)
{
	CAN_GPIO_Config();
	CAN_NVIC_Config();
	CAN_Mode_Config();
	CAN_Filter_Config(0x0000, 0x0000, 0x0000);

	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);			//ʹ��CAN1���ݽ����ж�
}


/************************************************* 
 ����: CAN_Send(u32 ExtId, char * str)
 ����: CANͨ�ű������ݷ���
 ����: 
	1��ExtId ����ʱ��ID
	2��str �������ݵ��׵�ַָ��
 ����: 
 ���÷���: 
	1��CAN_Send(0x1234, "12345678");
*************************************************/
void CAN_Send(u32 ExtId, char * str)
{
	__IO uint32_t	i	= 0;
	__IO uint8_t	TransmitMailbox = 0;

	//TxMessage.StdId=0x00;
	TxMessage.ExtId 	= ExtId;					//ʹ�õ���չID
	TxMessage.IDE		= CAN_ID_EXT;				//��չģʽ
	TxMessage.RTR		= CAN_RTR_DATA; 			//���͵�������
	TxMessage.DLC		= strlen(str);				//�趨��������Ϣ��֡����

	while (*str)
	{
		TxMessage.Data[i++] = *str++;				// ��������������
	}

	TransmitMailbox 	= CAN_Transmit(CAN1, &TxMessage); //��ʼһ����Ϣ�Ĵ��� 

	i					= 0;

	while ((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK) && (i != 0xFF)) //ͨ�����CANTXOKλ��ȷ�Ϸ����Ƿ�ɹ� 
	{
		i++;
	}
}


/************************************************* 
 ����: USB_LP_CAN1_RX0_IRQHandler(void)
 ����: CAN1�����жϷ�������������մ���8�ֽڵ����ݰ�
		����֡��ʽ����Ϊ�� ��ʶ����0x50 0x55�� + �����ݳ��� +����
		��ʶ�� 0x50 ΪС�ڵ���8�ֽڵ�����֡��0x55 Ϊ����8�ֽڵ�����֡
		�磺
			1��50 07 01 02 03 04 05
			2��55 0E 01 02 03 04 05 06 07 08 09 10 11 12
 
 ���÷���: 
	1���жϵ���
*************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	u8				i	= 0;

	NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);			//ʧ��CAN1��Ϣ�����ж�

	//�������ж�������
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);


	if (CanReceiveCounter == 0)
	{
		if (RxMessage.Data[0] == 0x50)
		{
			for (i = 0; i < RxMessage.DLC; i++)
				CanBuffer[i] = RxMessage.Data[i];

			CanReceiveState 	= 1;				//���ճɹ�	
			CanReceiveCounter	= 0;
			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//ʹ��CAN1���ݽ����ж�
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
			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//ʹ��CAN1���ݽ����ж�
		}
		else 
		{
			CanReceiveState 	= 0;				//����ʧ��
			CanReceiveCounter	= 0;
			CAN_Filter_Config(0x0000, 0x0000, 0x0000);
			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//ʹ��CAN1���ݽ����ж�
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
			CanReceiveState 	= 1;				//���ճɹ�	
			CanReceiveCounter	= 0;
			CAN_Filter_Config(0x0000, 0x0000, 0x0000);
			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//ʹ��CAN1���ݽ����ж�
		}
		else if (CanReceiveCounter < CanBuffer[1])
		{
			CAN_Filter_Config(RxMessage.ExtId, 0xFFFF, 0xFFFF);
			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//ʹ��CAN1���ݽ����ж�
		}
		else 
		{
			CanReceiveState 	= 0;				//����ʧ��	
			CanReceiveCounter	= 0;
			CAN_Filter_Config(0x0000, 0x0000, 0x0000);
			NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);	//ʹ��CAN1���ݽ����ж�
		}
	}
}

/************************************************* 
 ����: USB_LP_CAN1_RX0_IRQHandler(void)
 ����: �򵥵�CAN1�����жϷ�������ֱ�ӽ����յ������ݴ�ӡ����
 ����: 
    1��
    2��
 ����: 
 ���÷���: 
    1���жϵ���
*************************************************/
/*void USB_LP_CAN1_RX0_IRQHandler(void)
{
	u8				i	= 0;

	NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);			//ʧ��CAN1��Ϣ�����ж�

	//�������ж�������
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

	//�����յ�������ͨ�����ڴ�ӡ����
	for (i = 0; i < RxMessage.DLC; i++)
		printf("%c", RxMessage.Data[i]);

	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);			//ʹ��CAN1���ݽ����ж�
}*/


////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************* 
 ����: Can_Text(void)
 ����: Can���Ժ����������յ�������ֱ��ͨ�����ڴ�ӡ����
 ����: 
	1��
	2��
 ����: 
 ���÷���: 
	1���������� while �е��� Can_Text();
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



