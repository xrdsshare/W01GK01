

#ifndef __CAN_H
#define __CAN_H
#include "app_includes.h"


static void CAN_GPIO_Config(void);
static void CAN_NVIC_Config(void);
static void CAN_Mode_Config(void);
static void CAN_Filter_Config(u32 filterID, u16 maskIdHigh, u16 maskIdLow);
void CAN_Config(void);
void CAN_Send(u32 ExtId, u8 * str, u8 len);
void CAN_Send_VC(u32 ExtId, u8 * str);
void USB_LP_CAN1_RX0_IRQHandler(void);
void Can_Work(void);
void Can_Text(void);



#endif

