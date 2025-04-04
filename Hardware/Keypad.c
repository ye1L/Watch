#include "stm32f10x.h"  
#include "Delay.h"
#include "Keypad.h"
#include "Systick.h"

// 按键映射表（可根据需要修改）
const uint8_t KeyMap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// 初始化键盘GPIO
void Keypad_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // 初始化行线为输出（推挽输出）
    GPIO_InitStructure.GPIO_Pin = ROW0_PIN | ROW1_PIN | ROW2_PIN | ROW3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ROW_PORT, &GPIO_InitStructure);

    // 初始化列线为输入（上拉输入）
    GPIO_InitStructure.GPIO_Pin = COL0_PIN | COL1_PIN | COL2_PIN | COL3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(COL_PORT, &GPIO_InitStructure);
}

// 扫描键盘（带消抖）
char Keypad_Scan(void) 
{
    static uint8_t last_key = 0;
    static uint32_t last_time = 0;
    uint8_t current_key = 0;
    char result = 0;

    SysTick_Init();

    // 扫描每一行
    for (uint8_t row = 0; row < 4; row++) {
        // 设置当前行为低电平，其他行为高电平
        if (row != 0) GPIO_SetBits(ROW_PORT, ROW0_PIN); else GPIO_ResetBits(ROW_PORT, ROW0_PIN);
        if (row != 1) GPIO_SetBits(ROW_PORT, ROW1_PIN); else GPIO_ResetBits(ROW_PORT, ROW1_PIN);
        if (row != 2) GPIO_SetBits(ROW_PORT, ROW2_PIN); else GPIO_ResetBits(ROW_PORT, ROW2_PIN);
        if (row != 3) GPIO_SetBits(ROW_PORT, ROW3_PIN); else GPIO_ResetBits(ROW_PORT, ROW3_PIN);
    
        // 检查列线（检测低电平表示按键按下）
        if (GPIO_ReadInputDataBit(COL_PORT, COL0_PIN) == Bit_RESET) current_key = 1 + row * 4;
        if (GPIO_ReadInputDataBit(COL_PORT, COL1_PIN) == Bit_RESET) current_key = 2 + row * 4;
        if (GPIO_ReadInputDataBit(COL_PORT, COL2_PIN) == Bit_RESET) current_key = 3 + row * 4;
        if (GPIO_ReadInputDataBit(COL_PORT, COL3_PIN) == Bit_RESET) current_key = 4 + row * 4;
    
        // 短暂延迟（去抖动）
        Delay_ms(1);
    }
    
    // 所有行恢复高电平
    GPIO_SetBits(ROW_PORT, ROW0_PIN | ROW1_PIN | ROW2_PIN | ROW3_PIN);

    // 消抖处理（20ms间隔检测）
    if (current_key != 0) {
        if (current_key == last_key) {
            if (GetTick() - last_time > 20) {  // 20ms消抖
                result = KeyMap[(current_key - 1) / 4][(current_key - 1) % 4];
                last_time = GetTick();
            }
        } else {
            last_key = current_key;
            last_time = GetTick();
        }
    } else {
        last_key = 0;
    }

    return result;
}

// 使用示例
//int main(void) {
//    Keypad_Init();
//    while (1) {
//        char key = Keypad_Scan();
//        if (key != 0) {
//            printf("Pressed: %c\n", key);  // 替换为您的实际输出方式
//            Delay_ms(100);  // 防止连续触发
//        }
//    }
//}
