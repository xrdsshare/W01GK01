/*****************************************************************************
 �汾:	1.0.0
 ����:	XRDS
 ʱ��:	2018/6/18
 ����:	
 	1�����������ļ�
 ��Ҫ�ⲿ�ӿں������£�
 	1��Key_Init();	������ʼ������
	2��Key_Scan();
	3��Key0_Test();
 ʹ�÷���:	
 	1��app_includes.h ��������� #include "key.h"
 	2����� usart1..c �ļ�����Ŀ��
	3����� usart1.h ͷ�ļ�����·��
 	4������������ʼʱ������ Key_Init(); 
 	5������������ while �����е��� Key0_Test(); ��ע�⣺�ų�����LED0���Ƶĸ��ţ�
*****************************************************************************/
#include "key.h"


/************************************************* 
 ����: Key_Init(void)
 ����: ������ʼ������
 ����: 
    1��
    2��
 ����: 
 ���÷���: 
    1������������ʼʱ������ Key_Init(); 
*************************************************/
void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;			/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/

	RCC_APB2PeriphClockCmd(KEY0_RCC, ENABLE); // ʹ��KEY�˿�ʱ��
	GPIO_InitStructure.GPIO_Pin = KEY0_Pin; 		//ѡ���Ӧ������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	/*��������ģʽΪͨ���������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; /*������������Ϊ50MHz */
	GPIO_Init(KEY0_GPIOx, &GPIO_InitStructure); 	//��ʼ��KEY�˿�
}

/************************************************* 
 ����: Key_Scan(GPIO_TypeDef * GPIOx, u16 GPIO_Pin)
 ����: ����ɨ�輰��������
 ����: 
    1��GPIOx Ҫɨ�谴���Ŀ���GPIO�飬��ѡ GPIOA��GPIOB��GPIOC �ȣ�
    2��GPIO_Pin Ҫɨ�谴�������ţ���ѡ GPIO_Pin_0��GPIO_Pin_1��GPIO_Pin_3������
 ����: 
 ���÷���: 
    1��
*************************************************/
u8 Key_Scan(GPIO_TypeDef * GPIOx, u16 GPIO_Pin)
{
	/*����Ƿ��а������� */
	if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
	{
		/*��ʱ����*/
		Delay_ms(10);

		if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == KEY_ON)
		{
			/*�ȴ������ͷ� */
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
 ����: Key0_Test(void)
 ����: �������Ժ���
 		�������º�LED0��ת
 ����: 
    1��
    2��
 ����: 
 ���÷���: 
    1������������ while �����е��� Key0_Test(); ��ע�⣺�ų�����LED0���Ƶĸ��ţ�
*************************************************/
void Key0_Test(void)
{
	if (Key_Scan(KEY0_GPIOx, KEY0_Pin) == KEY_ON) //�ж�KEY1�Ƿ���
	{
		GPIO_WriteBit(LED0_GPIOx, LED0_Pin, (BitAction) ((1 - GPIO_ReadOutputDataBit(LED0_GPIOx, LED0_Pin)))); //LED0��ת
	}
}




