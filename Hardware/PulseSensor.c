#include "Delay.h"
#include "PulseSensor.h"
#include "stm32f10x.h"
#include "Tim.h"
#include "stdio.h"

/* 全局变量 */
static uint16_t PulseBuffer[BUFFER_SIZE] = {0};
static uint8_t bufferPos = 0;
static uint16_t lastHeartRate = 0;
static uint16_t IBI = 0; 
static uint16_t lastIBI = 0;

/******************** 硬件初始化部分 ********************/
void PulseSensor_ADC_Init(void) {
    /* 初始化TIM3触发源 */
    TIM3_Configuration();
    
    /* 时钟配置 */
    RCC_APB2PeriphClockCmd(PulseSensor_GPIO_CLK | ADC_CLK, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);  // 72MHz/6=12MHz
    
    /* GPIO配置 (模拟输入) */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = PulseSensor_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PulseSensor_GPIO_PORT, &GPIO_InitStruct);
    
    /* ADC配置 */
    ADC_DeInit(ADCx);
    ADC_InitTypeDef ADC_InitStruct;
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStruct.ADC_NbrOfChannel = 14;
    ADC_Init(ADCx, &ADC_InitStruct);
    
    /* 通道配置 */
    ADC_RegularChannelConfig(ADCx, ADC_Channel_14, 1, ADC_SampleTime_239Cycles5);
    ADC_ExternalTrigConvCmd(ADCx, ENABLE);
    ADC_Cmd(ADCx, ENABLE);
    
    /* ADC校准 */
    ADC_ResetCalibration(ADCx);
    while(ADC_GetResetCalibrationStatus(ADCx));
    ADC_StartCalibration(ADCx);
    while(ADC_GetCalibrationStatus(ADCx));
}

/******************** 核心算法部分 ********************/
void getPulse(void) {
    uint16_t adcValue = ADC_GetConversionValue(ADCx);
    PulseBuffer[bufferPos++] = adcValue;
    
    if(bufferPos >= BUFFER_SIZE) {
        bufferPos = 0;
        scaleData();
        
        uint8_t pulse;
        uint16_t maxValue;
        calculatePulse(&pulse, &maxValue);  // 使用局部变量
        
        if(pulse > 0) {
            lastHeartRate = pulse;
            lastIBI = IBI;
        }
    }
}

static void scaleData(void) {
    uint16_t max = getArrayMax(PulseBuffer, BUFFER_SIZE);
    uint16_t min = getArrayMin(PulseBuffer, BUFFER_SIZE);
    uint16_t delta = max - min;
    
    /* 信号有效性检查 */
    if(delta < 200) {  // 信号变化过小
        for(int i = 0; i < BUFFER_SIZE; i ++) 
            PulseBuffer[i] = delta/2;
    } 
    else {  // 归一化到0-1000范围
        for(int i = 0; i < BUFFER_SIZE; i ++) 
            PulseBuffer[i] = (PulseBuffer[i] - min) * 1000 / delta;
    }
}

static void calculatePulse(uint8_t *pulse, uint16_t *maxValue) {
    uint8_t firstPeak = 0, secondPeak = 0;
    uint16_t max = 0, min = 4095, threshold;
    
    /* 计算动态阈值 */
    for(int i = 0; i < BUFFER_SIZE; i ++) {
        if(PulseBuffer[i] > max) max = PulseBuffer[i];
        if(PulseBuffer[i] < min) min = PulseBuffer[i];
    }
    threshold = (max + min) / 2;
    *maxValue = max;
    
    /* 检测心跳上升沿 */
    uint8_t lastState = 0;
    uint8_t peakCount = 0;  // 用于记录检测到的上升沿数量

    for (int i = 0; i < BUFFER_SIZE; i++) {
        uint8_t currentState = (PulseBuffer[i] > threshold);

        if (!lastState && currentState) {  // 上升沿检测
            if (peakCount == 0) {        // 第一次检测到上升沿
                firstPeak = i;
                peakCount ++;
            } 
            else if (peakCount == 1) {   // 第二次检测到上升沿
                secondPeak = i;
                peakCount ++;
                break;  // 找到两个波峰后可以提前退出循环
            }
        }
        lastState = currentState;
        printf("max=%d, min=%d, threshold=%d\n", max, min, threshold);
        printf("firstPeak=%d, secondPeak=%d\n", firstPeak, secondPeak);
    }
    
    /* 计算心率 */
    if(secondPeak > firstPeak) {
        lastIBI = (secondPeak - firstPeak) * HEART_PERIOD;
        *pulse = 60000 / lastIBI;  // BPM = 60000ms / IBI(ms)
    } else {
        *pulse = 0;
    }
    
    printf("BPM: %d, IBI: %dms\n", *pulse, lastIBI);
}

/******************** 工具函数 ********************/
static uint16_t getArrayMax(uint16_t arr[], int size) {
    uint16_t max = arr[0];
    for(int i=1; i<size; i++) 
        if(arr[i] > max) max = arr[i];
    return max;
}

static uint16_t getArrayMin(uint16_t arr[], int size) {
    uint16_t min = arr[0];
    for(int i=1; i<size; i++) 
        if(arr[i] < min) min = arr[i];
    return min;
}

/******************** 对外接口 ********************/
uint16_t Get_HeartRate(void) {
    return lastHeartRate;
}

uint16_t Get_IBI(void) {
    return lastIBI;
}
