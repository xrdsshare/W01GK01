#include "g6a.h"


void G6A_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;			/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/

	RCC_APB2PeriphClockCmd(RELAY_RCC, ENABLE); // ʹ��PC�˿�ʱ��  
	GPIO_InitStructure.GPIO_Pin = RELAY1_Pin | RELAY2_Pin; //ѡ���Ӧ������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /*��������ģʽΪͨ���������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; /*������������Ϊ50MHz */
	GPIO_Init(RELAY_GPIOx, &GPIO_InitStructure);			//
	GPIO_SetBits(RELAY_GPIOx, RELAY1_Pin | RELAY2_Pin);	// �ر�����LED
	
	RCC_APB2PeriphClockCmd(DPN_RCC, ENABLE); // ʹ��PC�˿�ʱ��  
	GPIO_InitStructure.GPIO_Pin = D_P_EN_Pin | PN_PP_EN_Pin; //ѡ���Ӧ������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /*��������ģʽΪͨ���������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; /*������������Ϊ50MHz */
	GPIO_Init(DPN_RCC_GPIOx, &GPIO_InitStructure);			//
	GPIO_SetBits(DPN_RCC_GPIOx, D_P_EN_Pin | PN_PP_EN_Pin);	// �ر�����LED
}

void G6A_Test(void)
{
	G6A_RELAY1(ON);
	G6A_RELAY2(ON);
	PN_PP_EN(ON);
	P_D_EN(ON);
}

