#ifndef __PulseSensor_H__
#define __PulseSensor_H__

#include "stm32f10x.h"                  // Device header
 
// ADC 编号选择
// 可以是 ADC1/2/3
#define    ADCx                          ADC1
#define    ADC_CLK                       RCC_APB2Periph_ADC1
 
#define		PulseSensor_GPIO_CLK							RCC_APB2Periph_GPIOA
#define 	PulseSensor_GPIO_PORT							GPIOA
#define 	PulseSensor_GPIO_PIN							GPIO_Pin_4
 
void PulseSensor_ADC_Init(void);
u16  Get_Adc(u8 ch); 
u16  Get_Adc_Average(u8 ch,u8 times); 
 
#endif
