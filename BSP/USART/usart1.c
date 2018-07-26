

/*****************************************************************************
 �汾:	1.0.0
 ����:	XRDS
 ʱ��:	2018/5/1
 ����:	����1���ã����ṩ�ⲿ�ӿں���
	USART1 Tx PA.09
	USART1 Rx PA.10
 ��Ҫ�ⲿ�ӿں������£�
	1��USART1_Config(115200); ����1���ú���
	2��USART1_Char(); ����1���һ���ֽ����ݺ���
	3��USART1_Printf(); ��ʽ�������������C���е�printf��������û���õ�C��
	4��USART1_Test();	����1���Ժ����������յ�������ֱ�Ӵ�Ӧ����
 ʹ�÷���:	
	1��app_includes.h ���������� usart1.h
	2��stm32f10x_conf.h �п��� #include "stm32f10x_usart.h"
	3������ stm32f10x_usart.c �ļ�����Ŀ��
	4������ usart1.h ͷ�ļ�����·��
	5����д�ն˷����� USART1_IRQHandler()
	6��������������������
		//��volatile����ı������ڳ����ⱻ�ı�,ÿ�ζ�������ڴ��ж�ȡ���������ظ�ʹ�÷���cache��Ĵ����еı��ݡ�
		volatile u8 	Usart1Buffer[20] = {0x00};
		volatile u8 	Usart1ReceiveState = 0; //����1���ղ��������ַ���������ʶ��
		volatile u8 	Usart1ReceiveCounter = 0; //����1���յ����ַ�������
	7������������ USART1_Config(115200);
	8����������while�е���USART1_Test();���Է��ͽ��������Ƿ�һ��
*****************************************************************************/
#include "usart1.h"

extern u8		Usart1Buffer[20];
extern u8		Usart1ReceiveState; //����1���ղ��������ַ���������ʶ��
extern u8		Usart1ReceiveCounter; //����1���յ����ַ�������



/************************************************* 
 ����: USART1_Config
 ����: ����1���ú���
 ����: 
 ����: 
 ���÷���: 
	1��
*************************************************/
void USART1_Config(u32 USART1_BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* USART1 ʱ������ */
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

	/* ����USART1����ģʽ */
	USART_InitStructure.USART_BaudRate = USART1_BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);	//ʹ�ܽ���IDLE�ж�
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	//ʹ�ܽ����ж�
	USART_Cmd(USART1, ENABLE);

	/* ����USART1�����ж� */
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
 ����: fputc
 ����: �ض���c�⺯��printf��USART1,
 ����: 
	1��ch Ҫ������ֽ�����
	2��f �ļ�ָ�룬�˺�������Ҫʹ��
 ����: 
 ���÷���: 
	1��ֱ��ʹ��c�⺯��printf����printf����
*************************************************/
int fputc(int ch, FILE * f)
{
	/* ��Printf���ݷ������� */
	USART_SendData(USART1, (unsigned char) ch);

	while (! (USART1->SR & USART_FLAG_TXE))
		;

	return (ch);
}


/************************************************* 
 ����: itoa
 ����: ����������ת�����ַ���
 ����: 
	1��radix =10 ��ʾ10���ƣ��������Ϊ0
	2��value Ҫת����������
	3��buf ת������ַ���
	4��radix = 10
 ����: 
 ���÷���: 
	1��ֱ�ӱ�USART1_Printf()����
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
 ����: USART1_Printf
 ����: ��ʽ�������������C���е�printf��������û���õ�C��
 ����: 
	1��Data	 Ҫ���͵����ڵ����ݵ�ָ��
	2��... 	 ��������
 ����: 
 ���÷���: ����Ӧ��
	1��USART1_Printf( USART1, "\r\n this is a demo \r\n" );
	2��USART1_Printf( USART1, "\r\n %d \r\n", i );
	3��USART1_Printf( USART1, "\r\n %s \r\n", j ); 
*************************************************/


void USART1_Printf(uint8_t * Data, ...)
{
	const char *	s;
	int 			d;
	char			buf[16];

	va_list 		ap;

	va_start(ap, Data);

	while (*Data != 0) // �ж��Ƿ񵽴��ַ���������
	{
		if (*Data == 0x5c) //'\'
		{
			switch (* ++Data)
			{
				case 'r': //�س���
					USART_SendData(USART1, 0x0d);
					Data++;
					break;

				case 'n': //���з�
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
				case 's': //�ַ���
					s = va_arg(ap, const char *);

					for (; *s; s++)
					{
						USART_SendData(USART1, *s);

						while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
							;
					}

					Data++;
					break;

				case 'd': //ʮ����
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
 ����: USART1_Char
 ����: ����1���һ���ֽ����ݺ���
 ����: 
	1��ch Ҫ������ֽ�����
	2��
 ����: 
 ���÷���: 
	1��USART1_Char(0x00);
*************************************************/
void USART1_Char(char ch)
{
	USART_SendData(USART1, (unsigned char) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		;
}


/************************************************* 
 ����: USART1_IRQHandler()
 ����: ����1�жϷ��������ɽ��ܲ��������ַ���
 ����: 
	1��
	2��
 ����: 
 ���÷���: 
	1��
*************************************************/
void USART1_IRQHandler(void)
{
	u8				usart1Clear = usart1Clear;		//����ñ�������ȥ���༭����δʹ�ñ�������

	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		Usart1Buffer[Usart1ReceiveCounter++] = USART_ReceiveData(USART1);
	}
	else if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		usart1Clear 		= USART1->SR;			//��ȡSR�Ĵ���
		usart1Clear 		= USART1->DR;			//��ȡDR�Ĵ��������ȡSR���ڶ�ȡDR����Ϊ�����IDLE�жϣ�
		Usart1ReceiveState	= 1;
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************* 
 ����: USART1_Test()
 ����: ���ڲ��Ժ����������յ������ݷ��ͻ�ȥ
 ����: 
	1��
	2��
 ����: 
 ���÷���: 
	1��USART1_Test();
*************************************************/
void USART1_Test(void)
{
	u8				i	= 0;

	if (Usart1ReceiveState == 1) //������յ�1֡����
	{
		Usart1ReceiveState	= 0;

		while (Usart1ReceiveCounter--) // �ѽ��յ����ݷ��ͻش���
		{
			USART1_Char(Usart1Buffer[i++]);
		}

		Usart1ReceiveCounter = 0;
	}
}

