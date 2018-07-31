
#include "reset.h"


void SoftReset(void)
{
	__set_FAULTMASK(1); 							// 关闭所有中端
	NVIC_SystemReset(); 							// 复位
}


