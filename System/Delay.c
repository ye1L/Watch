#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

/**
  * @brief  微秒级延时（基于循环）
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t xus)
{
    uint32_t i;
    for (i = 0; i < xus; i++) {
        __asm__ volatile ("nop");  // 空操作，延时一个时钟周期
    }
}

/**
  * @brief  毫秒级延时（基于 FreeRTOS）
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
    vTaskDelay(pdMS_TO_TICKS(xms));  // 使用 FreeRTOS 的延时函数
}

/**
  * @brief  秒级延时（基于 FreeRTOS）
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
    while (xs--) {
        Delay_ms(1000);  // 调用毫秒级延时函数
    }
}
