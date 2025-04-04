#ifndef __SYSTEM_CLOCK_H
#define __SYSTEM_CLOCK_H

#include "stm32f10x.h"

/**
 * @brief 系统时钟初始化函数
 * @param 无
 * @return 无
 * @note 配置HSE（外部高速晶振）作为时钟源，PLL倍频至72MHz
 */
void SystemClock_Config(void);

#endif /* __SYSTEM_CLOCK_H */
