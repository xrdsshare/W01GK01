#ifndef __USART2_H
#define	__USART2_H

#include "app_includes.h"

void USART2_Config(u32 USART2_BaudRate);
void NVIC_Configuration(void);

int fputc(int ch, FILE *f);
void USART2_Printf(uint8_t *Data,...);
void USART2_Char(char c);
void USART2_IRQHandler(void);
void USART2_Test(void);


#endif /* __USART2_H */

