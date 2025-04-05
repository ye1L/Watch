#ifndef __PulseSensor_H__
#define __PulseSensor_H__

#include "stm32f10x.h"                  // Device header

/* 硬件配置宏定义 */
#define ADCx                   ADC1
#define ADC_CLK                RCC_APB2Periph_ADC1
#define PulseSensor_GPIO_PORT  GPIOC
#define PulseSensor_GPIO_PIN   GPIO_Pin_4
#define PulseSensor_GPIO_CLK   RCC_APB2Periph_GPIOC
#define SAMPLE_RATE           100     // 采样频率 (Hz)
#define BUFFER_SIZE           128     // 采样缓冲区大小
#define HEART_PERIOD          (1000 / SAMPLE_RATE)  // 采样间隔(ms)

void PulseSensor_ADC_Init(void);
void getPulse(uint8_t *pulse, uint16_t *maxValue);
static void scaleData(void);
static void calculatePulse(uint8_t *pulse, uint16_t *maxValue);
static uint16_t getArrayMax(uint16_t arr[], int size);
static uint16_t getArrayMin(uint16_t arr[], int size);
uint16_t Get_HeartRate(void);
uint16_t Get_IBI(void);

#endif
