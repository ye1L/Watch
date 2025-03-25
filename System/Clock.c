#include "stm32f10x.h" // 包含标准库头文件

void SystemClock_Config(void) {
    // 1. 使能 HSE（外部高速时钟）
    RCC->CR |= RCC_CR_HSEON; // 开启 HSE
    while (!(RCC->CR & RCC_CR_HSERDY)); // 等待 HSE 稳定

    // 2. 配置 FLASH 预取指和等待状态
    FLASH->ACR |= FLASH_ACR_PRFTBE; // 使能预取指
    FLASH->ACR &= ~FLASH_ACR_LATENCY; // 清除等待状态
    FLASH->ACR |= FLASH_ACR_LATENCY_2; // 2 个等待状态（适用于 48 MHz < SYSCLK <= 72 MHz）

    // 3. 配置 PLL
    RCC->CFGR &= ~RCC_CFGR_PLLSRC; // 清除 PLL 源选择
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE; // 选择 HSE 作为 PLL 输入
    RCC->CFGR &= ~RCC_CFGR_PLLMULL; // 清除 PLL 倍频系数
    RCC->CFGR |= RCC_CFGR_PLLMULL9; // PLL 倍频为 9 倍（8 MHz * 9 = 72 MHz）

    // 4. 使能 PLL
    RCC->CR |= RCC_CR_PLLON; // 开启 PLL
    while (!(RCC->CR & RCC_CR_PLLRDY)); // 等待 PLL 稳定

    // 5. 配置系统时钟源为 PLL
    RCC->CFGR &= ~RCC_CFGR_SW; // 清除系统时钟源
    RCC->CFGR |= RCC_CFGR_SW_PLL; // 选择 PLL 作为系统时钟源
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); // 等待切换完成

    // 6. 配置 AHB、APB1 和 APB2 分频器
    RCC->CFGR &= ~RCC_CFGR_HPRE;  // AHB 不分频
    RCC->CFGR &= ~RCC_CFGR_PPRE1; // APB1 2 分频（36 MHz）
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
    RCC->CFGR &= ~RCC_CFGR_PPRE2; // APB2 不分频（72 MHz）
}
