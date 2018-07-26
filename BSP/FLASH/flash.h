#ifndef __FLASH_H__
#define __FLASH_H__

#include "app_includes.h"


//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE 64 	 		//��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN 1              //ʹ��FLASHд��(0����ʹ��;1��ʹ��)

//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ
 
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr);		  //��������  
void STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����

//����д��
void FLASH_Write_Test(uint32_t WriteAddr,uint16_t WriteData);								   
#endif
