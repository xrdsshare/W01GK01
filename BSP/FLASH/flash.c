

/*****************************************************************************
 版本:	1.0.0
 作者:	XRDS
 时间:	2018/6/17
 描述:	flash操作函数
	主要外部接口函数如下：
	1、STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		//从指定地址开始写入指定
	长度的数据
	2、STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);			//从指定地址开始读出指定
	长度的数据
	3、FLASH_Write_Test(uint32_t WriteAddr,uint16_t WriteData);		//测试写入函数，可在调试模式下查看?
	?入情况
 使用方法:	
	1、app_includes.h 开启或添加 #include "flash.h"
	2、stm32f10x_conf.h 中开启 #include "stm32f10x_flash.h"
	3、添加 stm32f10x_flash.c 文件到项目中
	4、添加 flash.h 路径
	5、修改 FLASH 保存地址 FLASH_SAVE_ADDR
	7、在主函数中调用 FLASH_Write_Test(0X0800FC00, 0x1234); 测试写入情况是否正常
*****************************************************************************/
#include "flash.h"

#define FLASH_SAVE_ADDR 		0X0800FC00		//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)


/************************************************* 
 函数: STMFLASH_ReadHalfWord(uint32_t faddr)
 描述: 读取指定地址的半字(16位数据)
 输入: 
	1、faddr:读地址(此地址必须为2的倍数!!)
	2、
 返回: 
 调用方法: 
	1、该地址对应数据.
*************************************************/
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr)
{
	return * (uint16_t *)
	faddr;
}


#if STM32_FLASH_WREN //如果使能了写 


/************************************************* 
 函数: STMFLASH_Write_NoCheck(uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite)
 描述: 不检查的写入
 输入: 
	1、WriteAddr:起始地址
	2、pBuffer:数据指针
	3、NumToWrite:半字(16位)数
 返回: 
 调用方法: 
	1、
*************************************************/ 
void STMFLASH_Write_NoCheck(uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite)
{
	uint16_t		i;

	for (i = 0; i < NumToWrite; i++)
	{
		FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);
		WriteAddr			+= 2;					//地址增加2.
	}
}



#if STM32_FLASH_SIZE			<256
#define STM_SECTOR_SIZE 		1024 //字节

#else

#define STM_SECTOR_SIZE 		2048
#endif

uint16_t		STMFLASH_BUF[STM_SECTOR_SIZE / 2]; //最多是2K字节


/************************************************* 
 函数: STMFLASH_Write(uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite)
 描述: 
 输入: 
	1、WriteAddr:起始地址(此地址必须为2的倍数!!)
	2、pBuffer:数据指针
	3、NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
 返回: 
 调用方法: 
	1、
*************************************************/ 
void STMFLASH_Write(uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite)
{
	uint32_t		secpos; 						//扇区地址
	uint16_t		secoff; 						//扇区内偏移地址(16位字计算)
	uint16_t		secremain;						//扇区内剩余地址(16位字计算)	   
	uint16_t		i;
	uint32_t		offaddr;						//去掉0X08000000后的地址

	if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
		return; //非法地址

	FLASH_Unlock(); 								//解锁
	offaddr 			= WriteAddr - STM32_FLASH_BASE; //实际偏移地址.
	secpos				= offaddr / STM_SECTOR_SIZE; //扇区地址  0~127 for STM32F103RBT6
	secoff				= (offaddr % STM_SECTOR_SIZE) / 2; //在扇区内的偏移(2个字节为基本单位.)
	secremain			= STM_SECTOR_SIZE / 2 - secoff; //扇区剩余空间大小	 

	if (NumToWrite <= secremain)
		secremain = NumToWrite; //不大于该扇区范围

	while (1)
	{
		STMFLASH_Read(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2); //读出整个扇区的内容

		for (i = 0; i < secremain; i++) //校验数据
		{
			if (STMFLASH_BUF[secoff + i] != 0XFFFF)
				break; //需要擦除	  
		}

		if (i < secremain) //需要擦除
		{
			FLASH_ErasePage(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE); //擦除这个扇区

			for (i = 0; i < secremain; i++) //复制
			{
				STMFLASH_BUF[i + secoff] = pBuffer[i];
			}

			STMFLASH_Write_NoCheck(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2); //写入整个扇区  
		}
		else 
			STMFLASH_Write_NoCheck(WriteAddr, pBuffer, secremain); //写已经擦除了的,直接写入扇区剩余区间.				   

		if (NumToWrite == secremain)
			break; //写入结束了
		else //写入未结束
		{
			secpos++;								//扇区地址增1
			secoff				= 0;				//偏移位置为0	 
			pBuffer 			+= secremain;		//指针偏移
			WriteAddr			+= secremain;		//写地址偏移	   
			NumToWrite			-= secremain;		//字节(16位)数递减

			if (NumToWrite > (STM_SECTOR_SIZE / 2))
				secremain = STM_SECTOR_SIZE / 2; //下一个扇区还是写不完
			else 
				secremain = NumToWrite; //下一个扇区可以写完了
		}
	};

	FLASH_Lock();									//上锁
}


#endif



/************************************************* 
 函数: STMFLASH_Read(uint32_t ReadAddr, uint16_t * pBuffer, uint16_t NumToRead)
 描述: 从指定地址开始读出指定长度的数据
 输入: 
    1、ReadAddr:起始地址
    2、pBuffer:数据指针
    3、NumToWrite:半字(16位)数
 返回: 
 调用方法: 
    1、
*************************************************/ 
void STMFLASH_Read(uint32_t ReadAddr, uint16_t * pBuffer, uint16_t NumToRead)
{
	uint16_t		i;

	for (i = 0; i < NumToRead; i++)
	{
		pBuffer[i]			= STMFLASH_ReadHalfWord(ReadAddr); //读取2个字节.
		ReadAddr			+= 2;					//偏移2个字节.	
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************* 
 函数: FLASH_Write_Test(uint32_t WriteAddr, uint16_t WriteData)
 描述: FLASH写入测试程序
 输入: 
    1、WriteAddr:起始地址
    2、WriteData:要写入的数据
 返回: 
 调用方法: 
    1、
*************************************************/
void FLASH_Write_Test(uint32_t WriteAddr, uint16_t WriteData)
{
	STMFLASH_Write(WriteAddr, &WriteData, 1);		//写入一个字 
}


