#include "stm32f10x_tim.h"

void TIM3_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    // 1. 开启TIM3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // 2. 配置TIM3基础参数
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;    // 自动重装载值（决定触发频率）例如，TIM_Period = 1000-1 表示每1000个时钟周期触发一次ADC
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;   // 预分频值（72MHz/72 = 1MHz）若系统时钟为72MHz，分频后TIM3时钟为 72MHz / 72 = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // 3. 配置TIM3触发输出（TRGO）
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update); // 使用更新事件作为TRGO源

    // 4. 启动TIM3
    TIM_Cmd(TIM3, ENABLE);
}