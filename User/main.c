#include "FreeRTOS.h"
#include "task.h"
#include "Tasks.h"
#include "stdio.h"
#include "stm32f10x.h"
#include "stdbool.h"
#include "usart.h"
#include "semphr.h"
#include "MPU6050.h"
#include "lcd_spi_154.h"
#include "Keypad.h"
#include "DS3231.h"

TaskHandle_t Start_TaskHandle;

int main() 
{
    //硬件初始化
    USART_Config();
    MPU6050_Init();
    SPI_LCD_Init();
    Keypad_Init();
    DS3231_Init();

    //初始化互斥量
    xMutex = xQueueCreateMutex(NULL);

    //初始化信号量
    Time_SignalHandle = xSemaphoreCreateBinary();
    Keypad_SignalHandle = xSemaphoreCreateBinary();
    Gyro_SignalHandle = xSemaphoreCreateBinary();
    if (Time_SignalHandle == NULL || Keypad_SignalHandle == NULL || Gyro_SignalHandle == NULL) 
    {
        printf("Failed to create semaphores!\n");
        while(1);
    }
    xSemaphoreGive(Time_SignalHandle); // 关键！初始化为可用状态
    xSemaphoreGive(Keypad_SignalHandle);
    xSemaphoreGive(Gyro_SignalHandle);
    if (xMutex == NULL) {
        printf("Failed to create mutex!\n");
        while(1); // 错误处理
    }

    // 创建任务
    xTaskCreate(Start_Task, "Start_Task", 512, NULL, 1, &Start_TaskHandle);
    printf("Tasks created!\r\n");

    // 启动调度器
    vTaskStartScheduler();

    // 如果调度器启动失败，进入死循环
    while (1);
}
