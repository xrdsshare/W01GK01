

/*****************************************************************************
 版本:	1.0.0
 作者:	XRDS
 时间:	2018/6/17
 描述:	
	1、简单延时函数
 使用方法:	
	1、在#include "app_includes.h" 中添加或开启 #include "delay.h"
	2、直接在程序中使用 Delay_us(); 或者  Delay_ms()；
*****************************************************************************/
#include "delay.h"

static u8		fac_us = 0; //us延时倍乘数,为系统时钟的1/8			   
static u16		fac_ms = 0; //ms延时倍乘数,每个ms需要的systick时钟数


/************************************************* 
 函数: Delay_us()
 描述: 延时n个us
 输入: 
	1、nus 要延时的us数
	2、
 返回: 
 调用方法: 
	1、Delay_us(100)；延时100us
*************************************************/
void Delay_us(u32 nus)
{
	u32 			temp;

	fac_us				= SystemCoreClock / 8000000;
	fac_ms				= (u16)
	fac_us * 1000;
	SysTick->LOAD		= nus * fac_us; 			//时间加载			 
	SysTick->VAL		= 0x00; 					//清空计数器
	SysTick->CTRL		|= SysTick_CTRL_ENABLE_Msk; //开始倒数	  

	do 
	{
		temp				= SysTick->CTRL;
	}
	while((temp & 0x01) && ! (temp & (1 << 16)));

	//等待时间到达   
	SysTick->CTRL		&= ~SysTick_CTRL_ENABLE_Msk; //关闭计数器
	SysTick->VAL		= 0X00; 					//清空计数器	 
}



/************************************************* 
 函数: Delay_ms()
 描述: 延时n个ms
 			注意nms的范围，SysTick->LOAD为24位寄存器,所以,最大延时为：
 			nms<=0xffffff*8*1000/SYSCLK，SYSCLK单位为Hz,nms单位为ms
 			对72M条件下,nms<=1864 
 输入: 
    1、nms 要延时的ms数
    2、
 返回: 
 调用方法: 
    1、Delay_ms(500);	延时500毫秒
*************************************************/
void Delay_ms(u16 nms)
{
	u32 			temp;

	fac_us				= SystemCoreClock / 8000000;
	fac_ms				= (u16)
	fac_us * 1000;
	SysTick->LOAD		= (u32)
	nms *			fac_ms; 						//时间加载(SysTick->LOAD为24bit)

	SysTick->VAL		= 0x00; 					//清空计数器
	SysTick->CTRL		|= SysTick_CTRL_ENABLE_Msk; //开始倒数	

	do 
	{
		temp				= SysTick->CTRL;
	}
	while((temp & 0x01) && ! (temp & (1 << 16)));

	//等待时间到达   
	SysTick->CTRL		&= ~SysTick_CTRL_ENABLE_Msk; //关闭计数器
	SysTick->VAL		= 0X00; 					//清空计数器			
}

