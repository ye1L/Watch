#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdio.h"
#include "lcd_spi_154.h"

// 定义消息类型
typedef enum {
    MSG_HOME,      // 时间显示界面
    MSG_ALARM,     // 闹钟界面
    MSG_SETTINGS,  // 设置界面
    MSG_HEART_RATE // 心率检测界面
} MessageType;

QueueHandle_t xMessageQueue; // 消息队列

// 模拟 LCD 显示函数
void LCD_Display(const char* message) {
    printf("%s\n", message);
}

// 各个界面的显示函数
void DisplayHomeScreen() {
    LCD_Display("Home Screen: Time");
}

void DisplayAlarmScreen() {
    LCD_Display("Alarm Screen");
}

void DisplaySettingsScreen() {
    LCD_Display("Settings Screen");
}

void DisplayHeartRateScreen() {
    LCD_Display("Heart Rate Screen");
}

// 主任务：接收用户输入并发送消息
void Main_Task(void* pvParameters) {
    MessageType msg;
    while (1) {
        // 模拟用户输入（如按键或触摸事件）
        static int input = 0;
        input = (input + 1) % 4; // 循环切换状态

        msg = (MessageType)input; // 设置消息类型
        xQueueSend(xMessageQueue, &msg, portMAX_DELAY); // 发送消息到队列
        vTaskDelay(pdMS_TO_TICKS(1000)); // 模拟每秒切换一次
    }
}

// 界面任务：根据消息显示界面
void Interface_Task(void* pvParameters) {
    MessageType msg;
    while (1) {
        if (xQueueReceive(xMessageQueue, &msg, portMAX_DELAY) == pdPASS) {
            switch (msg) {
                case MSG_HOME:
                    DisplayHomeScreen();
                    break;
                case MSG_ALARM:
                    DisplayAlarmScreen();
                    break;
                case MSG_SETTINGS:
                    DisplaySettingsScreen();
                    break;
                case MSG_HEART_RATE:
                    DisplayHeartRateScreen();
                    break;
                default:
                    break;
            }
        }
    }
}

void Start_Task(void *pvParameters)
{
	
}

int main() {
	SPI_LCD_Init();
	LCD_Clear();
    // 创建消息队列
    xMessageQueue = xQueueCreate(10, sizeof(MessageType));

    // 创建主任务
    xTaskCreate(Main_Task, "Main_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    // 创建界面任务
    xTaskCreate(Interface_Task, "Interface_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    // 启动调度器
    vTaskStartScheduler();

    // 如果调度器启动失败，进入死循环
    while(1);
}