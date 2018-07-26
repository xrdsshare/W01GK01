

/*****************************************************************************
 �汾:	1.0.0
 ����:	XRDS
 ʱ��:	2018/6/17
 ����:	
	1������ʱ����
 ʹ�÷���:	
	1����#include "app_includes.h" ����ӻ��� #include "delay.h"
	2��ֱ���ڳ�����ʹ�� Delay_us(); ����  Delay_ms()��
*****************************************************************************/
#include "delay.h"

static u8		fac_us = 0; //us��ʱ������,Ϊϵͳʱ�ӵ�1/8			   
static u16		fac_ms = 0; //ms��ʱ������,ÿ��ms��Ҫ��systickʱ����


/************************************************* 
 ����: Delay_us()
 ����: ��ʱn��us
 ����: 
	1��nus Ҫ��ʱ��us��
	2��
 ����: 
 ���÷���: 
	1��Delay_us(100)����ʱ100us
*************************************************/
void Delay_us(u32 nus)
{
	u32 			temp;

	fac_us				= SystemCoreClock / 8000000;
	fac_ms				= (u16)
	fac_us * 1000;
	SysTick->LOAD		= nus * fac_us; 			//ʱ�����			 
	SysTick->VAL		= 0x00; 					//��ռ�����
	SysTick->CTRL		|= SysTick_CTRL_ENABLE_Msk; //��ʼ����	  

	do 
	{
		temp				= SysTick->CTRL;
	}
	while((temp & 0x01) && ! (temp & (1 << 16)));

	//�ȴ�ʱ�䵽��   
	SysTick->CTRL		&= ~SysTick_CTRL_ENABLE_Msk; //�رռ�����
	SysTick->VAL		= 0X00; 					//��ռ�����	 
}



/************************************************* 
 ����: Delay_ms()
 ����: ��ʱn��ms
 			ע��nms�ķ�Χ��SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ��
 			nms<=0xffffff*8*1000/SYSCLK��SYSCLK��λΪHz,nms��λΪms
 			��72M������,nms<=1864 
 ����: 
    1��nms Ҫ��ʱ��ms��
    2��
 ����: 
 ���÷���: 
    1��Delay_ms(500);	��ʱ500����
*************************************************/
void Delay_ms(u16 nms)
{
	u32 			temp;

	fac_us				= SystemCoreClock / 8000000;
	fac_ms				= (u16)
	fac_us * 1000;
	SysTick->LOAD		= (u32)
	nms *			fac_ms; 						//ʱ�����(SysTick->LOADΪ24bit)

	SysTick->VAL		= 0x00; 					//��ռ�����
	SysTick->CTRL		|= SysTick_CTRL_ENABLE_Msk; //��ʼ����	

	do 
	{
		temp				= SysTick->CTRL;
	}
	while((temp & 0x01) && ! (temp & (1 << 16)));

	//�ȴ�ʱ�䵽��   
	SysTick->CTRL		&= ~SysTick_CTRL_ENABLE_Msk; //�رռ�����
	SysTick->VAL		= 0X00; 					//��ռ�����			
}

