
#include "reset.h"


void SoftReset(void)
{
	__set_FAULTMASK(1); 							// �ر������ж�
	NVIC_SystemReset(); 							// ��λ
}


