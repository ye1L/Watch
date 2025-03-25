#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Key.h"

/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  */
void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOB的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_K1 | GPIO_Pin_K2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx, &GPIO_InitStructure);						//将PB1和PB11引脚初始化为上拉输入
}

/**
  * 函    数：按键获取键码
  * 参    数：无
  * 返 回 值：按下按键的键码值，范围：0~2，返回0代表没有按键按下
  * 注意事项：此函数是阻塞式操作，当按键按住不放时，函数会卡住，直到按键松手
  */

uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;		//定义变量，默认键码值为0
	
	if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin_K1) == 0)			//读PB1输入寄存器的状态，如果为0，则代表按键1按下
	{
		Delay_ms(20);											//延时消抖
		while (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin_K1) == 0);	//等待按键松手
		Delay_ms(20);											//延时消抖
		KeyNum = 1;												//置键码为1
	}
	
	if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin_K2) == 0)			//读PB11输入寄存器的状态，如果为0，则代表按键2按下
	{
		Delay_ms(20);											//延时消抖
		while (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin_K2) == 0);	//等待按键松手
		Delay_ms(20);											//延时消抖
		KeyNum = 2;												//置键码为2
	}
	
	return KeyNum;			//返回键码值，如果没有按键按下，所有if都不成立，则键码为默认值0
}


/// @brief 
/// @param  
/// @return 
/*非阻塞式，使其立即返回按键状态，而不是等待按键松开。*/
/*
uint8_t Key_GetNum(void)
{
    static uint8_t last_state_PB1 = 1;  // 上一次 PB1 的状态
    static uint8_t last_state_PB11 = 1; // 上一次 PB11 的状态
    uint8_t current_state_PB1 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);  // 读取 PB1 状态
    uint8_t current_state_PB11 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11); // 读取 PB11 状态
    uint8_t KeyNum = 0; // 默认键码值为0

    // 检测 PB1 按键
    if (last_state_PB1 == 1 && current_state_PB1 == 0) // 检测到下降沿（按键按下）
    {
        Delay_ms(20); // 简单延时去抖动
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0) // 再次确认按键状态
        {
            KeyNum = 1; // 置键码为1
        }
    }
    last_state_PB1 = current_state_PB1;

    // 检测 PB11 按键
    if (last_state_PB11 == 1 && current_state_PB11 == 0) // 检测到下降沿（按键按下）
    {
        Delay_ms(20); // 简单延时去抖动
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0) // 再次确认按键状态
        {
            KeyNum = 2; // 置键码为2
        }
    }
    last_state_PB11 = current_state_PB11;

    return KeyNum; // 返回键码值
}*/
