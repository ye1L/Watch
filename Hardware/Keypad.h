#include "stm32f10x.h"
#include <stdint.h>

#ifndef __KEYPAD_H
#define __KEYPAD_H

// 定义行和列的GPIO端口及引脚（根据实际硬件修改）
#define ROW_PORT    GPIOD
#define COL_PORT    GPIOD

#define ROW0_PIN    GPIO_Pin_3
#define ROW1_PIN    GPIO_Pin_2
#define ROW2_PIN    GPIO_Pin_1
#define ROW3_PIN    GPIO_Pin_0

#define COL0_PIN    GPIO_Pin_5
#define COL1_PIN    GPIO_Pin_4
#define COL2_PIN    GPIO_Pin_6
#define COL3_PIN    GPIO_Pin_7

extern const uint8_t KeyMap[4][4]; 

/* 初始化键盘GPIO */
void Keypad_Init(void);

/* 扫描键盘（带消抖），返回按下的键值，无按键时返回0 */
char Keypad_Scan(void);

#endif /* __KEYPAD_H */
