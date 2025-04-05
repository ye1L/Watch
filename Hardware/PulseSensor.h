#ifndef __PulseSensor_H__
#define __PulseSensor_H__

#include "stm32f10x.h"                  // Device header
 
// ADC 编号选择
// 可以是 ADC1/2/3
#define    ADCx                          ADC1
#define    ADC_CLK                       RCC_APB2Periph_ADC1
 
#define		PulseSensor_GPIO_CLK							RCC_APB2Periph_GPIOA
#define 	PulseSensor_GPIO_PORT							GPIOC
#define 	PulseSensor_GPIO_PIN							GPIO_Pin_4
 
#define HEART_PERIOD 10  // Example: 10ms between samples

void PulseSensor_ADC_Init(void);
u16  Get_Adc(u8 ch); 
u16  Get_Adc_Average(u8 ch,u8 times); 
void scaleData(void);
static void calculatePulse(uint8_t *pulse, uint16_t *maxValue);
uint16_t getArrayMax(uint16_t arr[], int size);
uint16_t getArrayMin(uint16_t arr[], int size);
 
#endif
