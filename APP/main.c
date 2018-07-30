
#include "app_includes.h"

//��volatile����ı������ڳ����ⱻ�ı�,ÿ�ζ�������ڴ��ж�ȡ���������ظ�ʹ�÷���cache��Ĵ����еı��ݡ�
volatile u8 	Usart1ReceiveState = 0; //����1���ղ��������ַ���������ʶ��
volatile u8 	Usart1ReceiveCounter = 0; //����1���յ����ַ�������
volatile u8 	Usart1Buffer[20] =
{
	0x00
};


// CAN ���ݱ�������
volatile u8 	CanReceiveState = 0; //CAN���ղ��������ַ���������ʶ��
volatile u8 	CanReceiveCounter = 0; //CAN���յ����ַ�������
volatile u8 	CanBuffer[20] =
{
	0x00
};


volatile u16	MyID = 0x8000; //��������ID
short			Electric_data; //��������
volatile long double 	VolRate = 0.6238491745551583961; //0.59604644775390625
volatile long			VolAGND = 0.0;

u8				Flash_Data[2];



int main(void)
{
	long double 	vol_ADR4515;

	LED_Init(); 									//LED �˿ڳ�ʼ�� 

	//	Key_Init(); 									//������ʼ��
	USART1_Config(115200);

	/* ����CANģ�� */
	CAN_Config();
	Init_ADS1256_GPIO();							//��ʼ��ADS1256 GPIO�ܽ� 
	Delay_ms(100);
	GPIO_SetBits(GPIOB, GPIO_Pin_11);

	//	ADS1256_Init();
	G6A_Init();

	//	Vol_Calibrate_ByADR4525();						//	��ѹ��У׼
	printf("==============���Գ���ʼ��==============\n");

	STMFLASH_Read(FLASH_SAVE_ADDR, (u16 *) Flash_Data, 1);
	MyID				= Flash_Data[0] << 8 | Flash_Data[1];
	printf("My ID is %X\n", MyID);

	CAN_Send(MyID, (char *) &MyID);	
	
	Git_Vol_ByAIN(VOL_IN);
	vol_ADR4515 = (Git_Vol_ByAIN(VOL_IN) -VolAGND) *VolRate;
	printf("ADR4515 = %LfuV, %LfmV, %LfV\r\n", vol_ADR4515, vol_ADR4515 / 1000, vol_ADR4515 / 1000000);

	//	FLASH_Write_Test(0X0800FC00, 0x1234);
	while (1)
	{
		//		LED0_Test();
		//		Key0_Test();
		//���ڲ��Ժ���
//		USART1_Test();

		//		Can_Text();
	};
}


