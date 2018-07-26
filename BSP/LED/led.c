/*****************************************************************************
 �汾:	1.0.0
 ����:	XRDS
 ʱ��:	2018/6/18
 ����:	
 	1��LED �����ļ�
 ��Ҫ�ⲿ�ӿں������£�
	1��LED_Init(void); //LED��ʼ������
	2��LED0_Test(void);	//LED���Ժ���
 ʹ�÷���:	
 	1��1��app_includes.h ��������� #include "led.h"
 	2����� led..c �ļ�����Ŀ��
	3����� led.h ͷ�ļ�����·��
 	4������������ʼʱ������ LED_Init(); 
 	5������������ while �����е��� LED0_Test();���в���
*****************************************************************************/
#include "led.h"

/************************************************* 
 ����: LED_Init(void)
 ����: LED IO��ʼ��
 ����: 
    1��
    2��
 ����: 
 ���÷���: 
    1������������ʼʱ������ LED_Init(); 
*************************************************/
void LED_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(LED0_RCC|RCC_APB2Periph_AFIO, ENABLE); //ʹ��LED�˿�ʱ��
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LED0_Pin; 		//LED �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO���ٶ�Ϊ50MHz
	GPIO_Init(LED0_GPIOx, &GPIO_InitStructure); 	//�����趨������ʼ��GPIOB.5
	GPIO_SetBits(LED0_GPIOx, LED0_Pin); 			//LED���� �����
}

////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************* 
 ����: LED0_Test(void)
 ����: LED0 ���Ժ�����LED0ÿ������һ��
 ����: 
    1��
    2��
 ����: 
 ���÷���: 
    1������������ while �����е��� LED0_Test();���в���
*************************************************/
void LED0_Test(void)
{
//	LED0(ON);										// ��
//	Delay_ms(500);
//	LED0(OFF);										// ��
//	Delay_ms(500);

	LED0((BitAction) ((1 - GPIO_ReadOutputDataBit(LED0_GPIOx, LED0_Pin)))); //LED0��ת
	Delay_ms(500);
}


