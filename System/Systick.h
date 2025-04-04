#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "FreeRTOS.h"
#include "task.h"

void SysTick_Init(void);
uint32_t GetTick(void);

#endif /* __SYSTICK_H */
