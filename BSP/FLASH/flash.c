

/*****************************************************************************
 �汾:	1.0.0
 ����:	XRDS
 ʱ��:	2018/6/17
 ����:	flash��������
	��Ҫ�ⲿ�ӿں������£�
	1��STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		//��ָ����ַ��ʼд��ָ��
	���ȵ�����
	2��STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);			//��ָ����ַ��ʼ����ָ��
	���ȵ�����
	3��FLASH_Write_Test(uint32_t WriteAddr,uint16_t WriteData);		//����д�뺯�������ڵ���ģʽ�²鿴?
	?�����
 ʹ�÷���:	
	1��app_includes.h ��������� #include "flash.h"
	2��stm32f10x_conf.h �п��� #include "stm32f10x_flash.h"
	3����� stm32f10x_flash.c �ļ�����Ŀ��
	4����� flash.h ·��
	5���޸� FLASH �����ַ FLASH_SAVE_ADDR
	7�����������е��� FLASH_Write_Test(0X0800FC00, 0x1234); ����д������Ƿ�����
*****************************************************************************/
#include "flash.h"

#define FLASH_SAVE_ADDR 		0X0800FC00		//����FLASH �����ַ(����Ϊż��������ֵҪ���ڱ�������ռ��FLASH�Ĵ�С+0X08000000)


/************************************************* 
 ����: STMFLASH_ReadHalfWord(uint32_t faddr)
 ����: ��ȡָ����ַ�İ���(16λ����)
 ����: 
	1��faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
	2��
 ����: 
 ���÷���: 
	1���õ�ַ��Ӧ����.
*************************************************/
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr)
{
	return * (uint16_t *)
	faddr;
}


#if STM32_FLASH_WREN //���ʹ����д 


/************************************************* 
 ����: STMFLASH_Write_NoCheck(uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite)
 ����: ������д��
 ����: 
	1��WriteAddr:��ʼ��ַ
	2��pBuffer:����ָ��
	3��NumToWrite:����(16λ)��
 ����: 
 ���÷���: 
	1��
*************************************************/ 
void STMFLASH_Write_NoCheck(uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite)
{
	uint16_t		i;

	for (i = 0; i < NumToWrite; i++)
	{
		FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);
		WriteAddr			+= 2;					//��ַ����2.
	}
}



#if STM32_FLASH_SIZE			<256
#define STM_SECTOR_SIZE 		1024 //�ֽ�

#else

#define STM_SECTOR_SIZE 		2048
#endif

uint16_t		STMFLASH_BUF[STM_SECTOR_SIZE / 2]; //�����2K�ֽ�


/************************************************* 
 ����: STMFLASH_Write(uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite)
 ����: 
 ����: 
	1��WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
	2��pBuffer:����ָ��
	3��NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
 ����: 
 ���÷���: 
	1��
*************************************************/ 
void STMFLASH_Write(uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite)
{
	uint32_t		secpos; 						//������ַ
	uint16_t		secoff; 						//������ƫ�Ƶ�ַ(16λ�ּ���)
	uint16_t		secremain;						//������ʣ���ַ(16λ�ּ���)	   
	uint16_t		i;
	uint32_t		offaddr;						//ȥ��0X08000000��ĵ�ַ

	if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
		return; //�Ƿ���ַ

	FLASH_Unlock(); 								//����
	offaddr 			= WriteAddr - STM32_FLASH_BASE; //ʵ��ƫ�Ƶ�ַ.
	secpos				= offaddr / STM_SECTOR_SIZE; //������ַ  0~127 for STM32F103RBT6
	secoff				= (offaddr % STM_SECTOR_SIZE) / 2; //�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain			= STM_SECTOR_SIZE / 2 - secoff; //����ʣ��ռ��С	 

	if (NumToWrite <= secremain)
		secremain = NumToWrite; //�����ڸ�������Χ

	while (1)
	{
		STMFLASH_Read(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2); //������������������

		for (i = 0; i < secremain; i++) //У������
		{
			if (STMFLASH_BUF[secoff + i] != 0XFFFF)
				break; //��Ҫ����	  
		}

		if (i < secremain) //��Ҫ����
		{
			FLASH_ErasePage(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE); //�����������

			for (i = 0; i < secremain; i++) //����
			{
				STMFLASH_BUF[i + secoff] = pBuffer[i];
			}

			STMFLASH_Write_NoCheck(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2); //д����������  
		}
		else 
			STMFLASH_Write_NoCheck(WriteAddr, pBuffer, secremain); //д�Ѿ������˵�,ֱ��д������ʣ������.				   

		if (NumToWrite == secremain)
			break; //д�������
		else //д��δ����
		{
			secpos++;								//������ַ��1
			secoff				= 0;				//ƫ��λ��Ϊ0	 
			pBuffer 			+= secremain;		//ָ��ƫ��
			WriteAddr			+= secremain;		//д��ַƫ��	   
			NumToWrite			-= secremain;		//�ֽ�(16λ)���ݼ�

			if (NumToWrite > (STM_SECTOR_SIZE / 2))
				secremain = STM_SECTOR_SIZE / 2; //��һ����������д����
			else 
				secremain = NumToWrite; //��һ����������д����
		}
	};

	FLASH_Lock();									//����
}


#endif



/************************************************* 
 ����: STMFLASH_Read(uint32_t ReadAddr, uint16_t * pBuffer, uint16_t NumToRead)
 ����: ��ָ����ַ��ʼ����ָ�����ȵ�����
 ����: 
    1��ReadAddr:��ʼ��ַ
    2��pBuffer:����ָ��
    3��NumToWrite:����(16λ)��
 ����: 
 ���÷���: 
    1��
*************************************************/ 
void STMFLASH_Read(uint32_t ReadAddr, uint16_t * pBuffer, uint16_t NumToRead)
{
	uint16_t		i;

	for (i = 0; i < NumToRead; i++)
	{
		pBuffer[i]			= STMFLASH_ReadHalfWord(ReadAddr); //��ȡ2���ֽ�.
		ReadAddr			+= 2;					//ƫ��2���ֽ�.	
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************* 
 ����: FLASH_Write_Test(uint32_t WriteAddr, uint16_t WriteData)
 ����: FLASHд����Գ���
 ����: 
    1��WriteAddr:��ʼ��ַ
    2��WriteData:Ҫд�������
 ����: 
 ���÷���: 
    1��
*************************************************/
void FLASH_Write_Test(uint32_t WriteAddr, uint16_t WriteData)
{
	STMFLASH_Write(WriteAddr, &WriteData, 1);		//д��һ���� 
}


