#include "system_clock.h"
#include "stm32f10x_rcc.h"

/**
 * @brief 系统时钟初始化
 * @details 配置步骤：
 * 1. 启用HSE（外部8MHz晶振）
 * 2. 等待HSE稳定
 * 3. 配置PLL：HSE作为输入，9倍频输出72MHz
 * 4. 配置AHB、APB1、APB2分频
 * 5. 启用PLL并切换为系统时钟源
 */
void SystemClock_Config(void) {
    /* 1. 复位RCC配置（可选） */
    RCC_DeInit();

    /* 2. 启用HSE（外部高速晶振，通常8MHz） */
    RCC_HSEConfig(RCC_HSE_ON); // 开启HSE
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET); // 等待HSE就绪

    /* 3. 配置PLL：HSE作为输入，9倍频输出72MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // HSE不分频，9倍频（8MHz * 9 = 72MHz）

    /* 4. 配置时钟分频 */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);   // AHB时钟 = 72MHz
    RCC_PCLK1Config(RCC_HCLK_Div2);    // APB1时钟 = 36MHz（最大36MHz）
    RCC_PCLK2Config(RCC_HCLK_Div1);    // APB2时钟 = 72MHz

    /* 5. 启用PLL并等待就绪 */
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    /* 6. 切换系统时钟源到PLL */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08); // 检查PLL是否成功切换为系统时钟

    /* 7. 启用预取指和Flash延迟（72MHz需2个等待周期） */
    FLASH_SetLatency(FLASH_Latency_2);
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
}
