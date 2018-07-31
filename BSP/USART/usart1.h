#ifndef __USART1_H
#define	__USART1_H

#include "app_includes.h"

void USART1_Config(u32 USART1_BaudRate);
void NVIC_Configuration(void);

int fputc(int ch, FILE *f);
void USART1_Printf(uint8_t *Data,...);
void USART1_Char(char c);
void USART1_IRQHandler(void);
void USART1_Work(void);
void USART1_Test(void);


#endif /* __USART1_H */

