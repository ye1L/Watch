#include "stm32f10x.h"
#include "Systick.h"
#include "FreeRTOS.h"
#include "task.h"

// FreeRTOS 会自动管理 SysTick，无需手动维护 tick 变量
// 但如果你需要额外的 tick 计数（非 FreeRTOS 自带），可以保留：
//static volatile uint32_t custom_tick = 0;

/*void SysTick_Handler(void) {
    // 1. 调用 FreeRTOS 的 SysTick 处理函数（必须）
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
    
    // 2. 可选：维护自定义的 tick（如果非 FreeRTOS 功能需要）
    custom_tick++;
    
    // 3. 可选：如果使用 HAL 库，需调用 HAL_IncTick()
    // HAL_IncTick();
}*/

uint32_t GetTick(void) {
    // 使用 FreeRTOS 提供的 tick 计数（更可靠）
    return xTaskGetTickCount();
    
    // 或者返回自定义的 custom_tick（如果有特殊需求）
    // return custom_tick;
}

void SysTick_Init(void) {
    // FreeRTOS 会自动配置 SysTick，此处可留空
    // 如果需要自定义频率（非默认 1kHz），可修改 FreeRTOSConfig.h 中的 configTICK_RATE_HZ
}
