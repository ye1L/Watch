#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdio.h"
#include "stm32f10x.h"

TaskHandle_t xWakeUpTaskHandle;
TaskHandle_t xTimeUpdateTaskHandle;
TaskHandle_t xHeartRateTaskHandle;
TaskHandle_t xAlarmTaskHandle;
TaskHandle_t xButtonsTaskHandle;
TaskHandle_t xInterfaceTaskHandle;

// 定义消息类型
typedef enum {
    MSG_WAKE_UP,      // 抬腕唤醒
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

// 硬件初始化函数
void Hardware_Init() {
    // 初始化系统时钟
    SystemClock_Config();

    // 初始化外设
    MX_I2C1_Init(); // I2C 初始化（用于 MPU6050 和 DS3231）
    MX_SPI1_Init(); // SPI 初始化（用于 LCD）
    MX_GPIO_Init(); // GPIO 初始化（用于按键、蜂鸣器、震动马达等）

    // 初始化传感器和外设
    MPU6050_Init();  // MPU6050 初始化
    DS3231_Init();   // DS3231 初始化
    HeartRate_Init(); // 心率传感器初始化
    LCD_Init();      // LCD 初始化
    Buzzer_Init();   // 蜂鸣器初始化
    Motor_Init();    // 震动马达初始化
    Buttons_Init();  // 按键初始化
}

// 抬腕唤醒任务
void WakeUp_Task(void* pvParameters) {
    while (1) {
        if (MPU6050_IsWristRaised()) { // 检测是否抬腕
            MessageType msg = MSG_WAKE_UP;
            xQueueSend(xMessageQueue, &msg, portMAX_DELAY); // 发送唤醒消息
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 每 100ms 检测一次
    }
}

// 时间更新任务
void TimeUpdate_Task(void* pvParameters) {
    while (1) {
        DS3231_Time time = DS3231_GetTime(); // 获取当前时间
        char timeString[20];
        snprintf(timeString, sizeof(timeString), "Time: %02d:%02d:%02d", time.hour, time.minute, time.second);

        // 发送时间更新消息
        MessageType msg = MSG_TIME_UPDATE;
        xQueueSend(xMessageQueue, &msg, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒更新一次
    }
}

// 心率检测任务
void HeartRate_Task(void* pvParameters) {
    while (1) {
        if (currentScreen == SCREEN_HEART_RATE) { // 仅在心率检测界面检测心率
            uint8_t heartRate = HeartRate_GetValue(); // 获取心率值
            char heartRateString[20];
            snprintf(heartRateString, sizeof(heartRateString), "HR: %d BPM", heartRate);

            // 发送心率检测消息
            MessageType msg = MSG_HEART_RATE;
            xQueueSend(xMessageQueue, &msg, portMAX_DELAY);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒检测一次
    }
}

// 闹钟提醒任务
void Alarm_Task(void* pvParameters) {
    while (1) {
        if (Alarm_IsTriggered()) { // 检测闹钟是否触发
            MessageType msg = MSG_ALARM;
            xQueueSend(xMessageQueue, &msg, portMAX_DELAY); // 发送闹钟提醒消息
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 每 100ms 检测一次
    }
}

// 按键检测任务
void Buttons_Task(void* pvParameters) {
    while (1) {
        if (Button1_Pressed()) { // 按键1：切换到上一个界面
            currentScreen = (currentScreen - 1 + 3) % 3; // 循环切换界面
            MessageType msg = MSG_SWITCH_SCREEN;
            xQueueSend(xMessageQueue, &msg, portMAX_DELAY);
        } else if (Button2_Pressed()) { // 按键2：切换到下一个界面
            currentScreen = (currentScreen + 1) % 3; // 循环切换界面
            MessageType msg = MSG_SWITCH_SCREEN;
            xQueueSend(xMessageQueue, &msg, portMAX_DELAY);
        }
        vTaskDelay(pdMS_TO_TICKS(200)); // 按键消抖，每 200ms 检测一次
    }
}

// 界面任务
void Interface_Task(void* pvParameters) {
    MessageType msg;
    while (1) {
        if (xQueueReceive(xMessageQueue, &msg, portMAX_DELAY) == pdPASS) {
            switch (msg) {
                case MSG_WAKE_UP:
                    LCD_WakeUp(); // 唤醒 LCD
                    break;
                case MSG_TIME_UPDATE:
                    if (currentScreen == SCREEN_TIME) {
                        LCD_DisplayTime(); // 显示时间
                    }
                    break;
                case MSG_HEART_RATE:
                    if (currentScreen == SCREEN_HEART_RATE) {
                        LCD_DisplayHeartRate(); // 显示心率
                    }
                    break;
                case MSG_ALARM:
                    Buzzer_Beep(); // 蜂鸣器提醒
                    Motor_Vibrate(); // 震动马达提醒
                    break;
                case MSG_SWITCH_SCREEN:
                    LCD_Clear(); // 清屏
                    switch (currentScreen) {
                        case SCREEN_TIME:
                            LCD_DisplayTime(); // 显示时间界面
                            break;
                        case SCREEN_ALARM:
                            LCD_DisplayAlarm(); // 显示闹钟设置界面
                            break;
                        case SCREEN_HEART_RATE:
                            LCD_DisplayHeartRate(); // 显示心率检测界面
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

int main() {
    // 硬件初始化
    Hardware_Init();

    // 创建消息队列
    xMessageQueue = xQueueCreate(10, sizeof(MessageType));

    // 创建任务
    xTaskCreate(WakeUp_Task, "WakeUp", configMINIMAL_STACK_SIZE, NULL, 1, &xWakeUpTaskHandle);
    xTaskCreate(TimeUpdate_Task, "TimeUpdate", configMINIMAL_STACK_SIZE, NULL, 1, &xTimeUpdateTaskHandle);
    xTaskCreate(HeartRate_Task, "HeartRate", configMINIMAL_STACK_SIZE, NULL, 1, &xHeartRateTaskHandle);
    xTaskCreate(Alarm_Task, "Alarm", configMINIMAL_STACK_SIZE, NULL, 1, &xAlarmTaskHandle);
    xTaskCreate(Buttons_Task, "Buttons", configMINIMAL_STACK_SIZE, NULL, 1, &xButtonsTaskHandle);
    xTaskCreate(Interface_Task, "Interface", configMINIMAL_STACK_SIZE, NULL, 1, &xInterfaceTaskHandle);

    // 启动调度器
    vTaskStartScheduler();

    // 如果调度器启动失败，进入死循环
    while (1);
}