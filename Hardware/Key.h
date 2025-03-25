#ifndef __KEY_H
#define __KEY_H

#define         GPIOx           GPIOA
#define         GPIO_Pin_K1     GPIO_Pin_5
#define         GPIO_Pin_K2     GPIO_Pin_6

void Key_Init(void);
uint8_t Key_GetNum(void);

#endif
