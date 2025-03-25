#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdio.h"
#include "stm32f10x.h"
#include "DS3231.h"
#include "lcd_spi_154.h"
#include "MPU6050.h"
#include "stdbool.h"
#include "OLED.h"
#include "usart.h"
#include "Key.h"
#include "PulseSensor.h"
#include "semphr.h"

TaskHandle_t xWakeUpTaskHandle;
TaskHandle_t xTimeUpdateTaskHandle;
TaskHandle_t xHeartRateTaskHandle;
TaskHandle_t xAlarmTaskHandle;
TaskHandle_t xButtonsTaskHandle;
TaskHandle_t xScreenTimeoutTaskHandle;
TaskHandle_t xInterfaceTaskHandle;

// 定义消息类型
typedef enum {
    MSG_WAKE_UP,      // 抬腕唤醒
    MSG_SCREEN_SLEEP,
    MSG_TIME_UPDATE,  // 时间更新
    MSG_HEART_RATE,   // 心率检测
    MSG_ALARM,        // 闹钟提醒
    MSG_SWITCH_SCREEN // 切换界面
} MessageType;

// 定义界面类型
typedef enum {
    SCREEN_TIME,      // 时间显示界面
    SCREEN_ALARM,     // 闹钟设置界面
    SCREEN_HEART_RATE // 心率检测界面
} ScreenType;

// 全局变量
QueueHandle_t xMessageQueue; // 消息队列
ScreenType currentScreen = SCREEN_TIME; // 当前界面
static bool isWristUp = false;
static TickType_t lastTrigger = 0;
SemaphoreHandle_t xMutex = NULL;  // 互斥量句柄

// 硬件初始化函数
void Hardware_Init() {
    // 初始化系统时钟
    //SystemClock_Config();

    // 初始化传感器和外设
    MPU6050_Init();  // MPU6050 初始化
    DS3231_Init();   // DS3231 初始化
    PulseSensor_ADC_Init(); // 心率传感器初始化
    SPI_LCD_Init();      // LCD 初始化
    //Buzzer_Init();   // 蜂鸣器初始化
    //Motor_Init();    // 震动马达初始化
    Key_Init();  // 按键初始化
    OLED_Init();
    USART_Config();
}

// 抬腕唤醒任务
void WakeUp_Task(void* pvParameters) {
    int16_t AX, AY, AZ, GX, GY, GZ;
    float AZ_g;
    float threshold = -0.1;
    float calibrationValue = 0.0; // 校准值

    // 校准过程：读取 100 次 Z 轴加速度的平均值
    for (int i = 0; i < 100; i++) {
        MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
        calibrationValue += (float)AZ / 2048.0f;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    calibrationValue /= 100.0f; // 计算平均值
    OLED_ShowFloat(1, 8, calibrationValue, 1, 3);

    while (1) 
    {
        MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
        AZ_g = (float)AZ / 2048.0f - calibrationValue; // 使用 2048，因为量程是 ±16g
        OLED_ShowFloat(1, 1, AZ_g, 1, 3);
        OLED_ShowSignedNum(2, 1, AX, 5);					//OLED显示数据
		OLED_ShowSignedNum(3, 1, AY, 5);
		OLED_ShowSignedNum(4, 1, AZ, 5);
		OLED_ShowSignedNum(2, 8, GX, 5);
		OLED_ShowSignedNum(3, 8, GY, 5);
		OLED_ShowSignedNum(4, 8, GZ, 5);

        if (AZ_g < threshold) 
        {
            TickType_t now = xTaskGetTickCount();
    
            if (!isWristUp && (now - lastTrigger > pdMS_TO_TICKS(1000))) 
            {
                MessageType msg = MSG_WAKE_UP;
                if (xQueueSend(xMessageQueue, &msg, 0) == pdPASS) 
                {
                    printf("Send MSG_WAKE_UP\r\n");
                    lastTrigger = now;
                    xSemaphoreTake(xMutex, portMAX_DELAY);
                    isWristUp = true;  // 标记已唤醒
                    xSemaphoreGive(xMutex);
                }
            }
        } 
        else
        {
            MessageType msg = MSG_SCREEN_SLEEP;
            if (xQueueSend(xMessageQueue, &msg, 100) == pdPASS)
            {
                printf("Send MSG_SCREEN_SLEEP\r\n");
                xSemaphoreTake(xMutex, portMAX_DELAY);
                isWristUp = false;  // 手腕放下后重置状态
                xSemaphoreGive(xMutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// 时间更新任务
void TimeUpdate_Task(void* pvParameters) {
    while (1) {
        //DS3231_SetTime(25, 3, 23, 7, 1, 53, 0);
        // 发送时间更新消息
        MessageType msg = MSG_TIME_UPDATE;
        xQueueSend(xMessageQueue, &msg, portMAX_DELAY);

        // 延时 1 秒
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// 心率检测任务
void HeartRate_Task(void* pvParameters) 
{
    taskENTER_CRITICAL();
    printf("HeartRateTask running...\r\n");
    taskEXIT_CRITICAL();
    while (1) {
        if (currentScreen == SCREEN_HEART_RATE) 
        {
             // 发送心率检测消息
            MessageType msg = MSG_HEART_RATE;
            if (xQueueSend(xMessageQueue, &msg, portMAX_DELAY) != pdPASS)
            {
                printf("Failed to send MSG_HEART_RATE");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒检测一次
    }
}

// 闹钟提醒任务
/*void Alarm_Task(void* pvParameters) {
    while (1) {
        if (Alarm_IsTriggered()) { // 检测闹钟是否触发
            MessageType msg = MSG_ALARM;
            xQueueSend(xMessageQueue, &msg, portMAX_DELAY); // 发送闹钟提醒消息
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 每 100ms 检测一次
    }
}*/

// 按键检测任务
void Buttons_Task(void* pvParameters) {
    while (1) {
        if (Key_GetNum() == 1) {  // 上一个界面
            // 使用明确的枚举切换逻辑
            switch(currentScreen) {  // 使用全局变量currentScreen
                case SCREEN_TIME:
                    currentScreen = SCREEN_HEART_RATE;
                    break;
                case SCREEN_ALARM:
                    currentScreen = SCREEN_TIME;
                    break;
                case SCREEN_HEART_RATE:
                    currentScreen = SCREEN_ALARM;
                    break;
            }
            printf("Switched to previous screen: %d\r\n", (int)currentScreen);
            MessageType msg = MSG_SWITCH_SCREEN;
            xQueueSend(xMessageQueue, &msg, portMAX_DELAY);
        } 
        else if (Key_GetNum() == 2) {  // 下一个界面
            // 使用明确的枚举切换逻辑
            switch(currentScreen) {
                case SCREEN_TIME:
                    currentScreen = SCREEN_ALARM;
                    break;
                case SCREEN_ALARM:
                    currentScreen = SCREEN_HEART_RATE;
                    break;
                case SCREEN_HEART_RATE:
                    currentScreen = SCREEN_TIME;
                    break;
            }
            printf("Switched to next screen: %d\r\n", (int)currentScreen);
            MessageType msg = MSG_SWITCH_SCREEN;
            xQueueSend(xMessageQueue, &msg, portMAX_DELAY);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// 自动息屏任务
void ScreenTimeout_Task(void* pvParameters) 
{
    //const TickType_t xInactivityTimeout = pdMS_TO_TICKS(10000);
    //TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) 
    {
        MessageType msg;
        if (xQueueReceive(xMessageQueue, &msg, pdMS_TO_TICKS(1000)))
        {
            if (msg == MSG_SCREEN_SLEEP) 
            {
                xSemaphoreTake(xMutex, portMAX_DELAY);
                if (isWristUp) 
                {
                    LCD_Sleep();
                    isWristUp = false;
                    printf("Screen OFF by timeout\n");
                }
                xSemaphoreGive(xMutex);
            }
        }
    }
}

// 界面任务
void Interface_Task(void* pvParameters) 
{
    MessageType msg;
    printf("InterfaceTask running...\r\n");
    while (1) 
    {
        if (xQueueReceive(xMessageQueue, &msg, portMAX_DELAY) == pdPASS) 
        {
            switch (msg) 
            {
                case MSG_WAKE_UP:
                    isWristUp = true; // 点亮屏幕
                    LCD_WakeUp(); // 唤醒 LCD
                    break;
                case MSG_TIME_UPDATE:
                    if (currentScreen == SCREEN_TIME && isWristUp) {
                        LCD_DisplayTime(); // 显示时间
                    }
                    break;
                case MSG_HEART_RATE:
                    if (currentScreen == SCREEN_HEART_RATE && isWristUp) {
                        LCD_DisplayHeartRate(); // 显示心率
                    }
                    break;
                /*case MSG_ALARM:
                    Buzzer_Beep(); // 蜂鸣器提醒
                    Motor_Vibrate(); // 震动马达提醒
                    break;*/
                case MSG_SWITCH_SCREEN:
                    if (isWristUp) 
                    {
                        LCD_Clear(); // 清屏
                        switch (currentScreen) 
                        {
                            case SCREEN_TIME:
                                LCD_DisplayTime(); // 显示时间界面
                                break;
                            //case SCREEN_ALARM:
                            //    LCD_DisplayAlarm(); // 显示闹钟设置界面
                            //    break;
                            case SCREEN_HEART_RATE:
                                LCD_DisplayHeartRate(); // 显示心率检测界面
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

int main() 
{
    // 硬件初始化
    Hardware_Init();
    printf("Hardware initialized!\r\n");

    // 创建互斥锁
    xMutex = xQueueCreateMutex(NULL);
    if (xMutex == NULL) {
        printf("Failed to create mutex!\n");
        while(1); // 错误处理
    }

    // 创建消息队列
    xMessageQueue = xQueueCreate(20, sizeof(MessageType));
    printf("Queue created!\r\n");

    // 创建任务
    xTaskCreate(WakeUp_Task, "WakeUp", 256, NULL, 2, &xWakeUpTaskHandle);
    xTaskCreate(TimeUpdate_Task, "TimeUpdate", 256, NULL, 2, &xTimeUpdateTaskHandle);
    xTaskCreate(HeartRate_Task, "HeartRate", 256, NULL, 2, &xHeartRateTaskHandle);
    //xTaskCreate(Alarm_Task, "Alarm", 256, NULL, 1, &xAlarmTaskHandle);
    xTaskCreate(Buttons_Task, "Buttons", 256, NULL, 3, &xButtonsTaskHandle);
    xTaskCreate(ScreenTimeout_Task, "ScreenTimeout", 256, NULL, 1, &xScreenTimeoutTaskHandle);
    xTaskCreate(Interface_Task, "Interface", 256, NULL, 2, &xInterfaceTaskHandle);
    printf("Tasks created!\r\n");

    // 启动调度器
    vTaskStartScheduler();

    // 如果调度器启动失败，进入死循环
    while (1);
}
