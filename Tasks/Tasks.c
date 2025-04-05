#include "FreeRTOS.h"
#include "DS3231.h"
#include "lcd_spi_154.h"
#include "semphr.h"
#include "task.h"
#include "Display.h"
#include "stdbool.h"
#include "Tasks.h"
#include "usart.h"
#include "stdio.h"
#include "MPU6050.h"
#include "Keypad.h"
#include "PulseSensor.h"

//定义信号量
SemaphoreHandle_t Time_SignalHandle;
SemaphoreHandle_t Gyro_SignalHandle;
SemaphoreHandle_t Keypad_SignalHandle;
SemaphoreHandle_t HeartRate_SignalHandle;
SemaphoreHandle_t xMutex = NULL;

//定义任务句柄
TaskHandle_t Time_TaskHandle;
TaskHandle_t Gyro_TaskHandle;
TaskHandle_t HomePage_TaskHandle;
TaskHandle_t Menu_TaskHandle;
TaskHandle_t Keypad_TaskHandle;
TaskHandle_t HeartRate_TaskHandle;


//定义全局变量
bool LCD_Mode = true;
bool First_Flag;
bool Menu_Flag;
static bool Alarm_Flag = true;
TickType_t lastActivityTime = 0;
const TickType_t timeoutTicks = pdMS_TO_TICKS(30000);

void Start_Task(void * argument) {
    xTaskCreate(TimeUpdate, "Time_Task", 256, NULL, 1, &Time_TaskHandle);
    xTaskCreate(Display_HomePage, "HomePage_Task", 256, NULL, 3, &HomePage_TaskHandle);
    xTaskCreate(Display_Menu, "Menu_Task", 128, NULL, 2, &Menu_TaskHandle);
    xTaskCreate(Read_Gyro, "Gyro_Task", 128, NULL, 1, &Gyro_TaskHandle);
    xTaskCreate(Keypad, "Keypad_Task", 128, NULL, 1, &Keypad_TaskHandle);
    xTaskCreate(Read_HeartRate, "HeartRate_Task", 128, NULL, 1, &HeartRate_TaskHandle);
    vTaskSuspend(Menu_TaskHandle);
    vTaskSuspend(Gyro_TaskHandle);

    vTaskDelete(NULL);
}

void TimeUpdate(void * argument) {
    uint8_t year, month, date, week, hour, min, sec;
    while (1) 
    {
        //获取并打印当前任务（HomePage_Task）的堆栈剩余空间
        //UBaseType_t stack = uxTaskGetStackHighWaterMark(NULL);
        //taskENTER_CRITICAL();
        //printf("Time_Task stack remaining: %d\n", stack);
        //taskEXIT_CRITICAL();

        // 安全获取信号量
        if (xSemaphoreTake(Time_SignalHandle, portMAX_DELAY) == pdTRUE) 
        {
            DS3231_GetTime(&year, &month, &date, &week, &hour, &min, &sec);
            xSemaphoreGive(Time_SignalHandle);  // 无需检查，除非信号量可能被删除
        } 
        vTaskDelay(pdMS_TO_TICKS(100));  // 明确使用 pdMS_TO_TICKS 转换
    }
}

void Keypad(void * argument) {
    while (1) {
        //获取并打印当前任务（HomePage_Task）的堆栈剩余空间
        //UBaseType_t stack = uxTaskGetStackHighWaterMark(NULL);
        //printf("Keypad_Task stack remaining: %d\n", stack);

        // 安全获取信号量
        if (xSemaphoreTake(Keypad_SignalHandle, pdMS_TO_TICKS(1)) == pdTRUE) {
            Keypad_Scan();
            xSemaphoreGive(Keypad_SignalHandle);
        }

        vTaskDelay(pdMS_TO_TICKS(10));  // 明确使用 pdMS_TO_TICKS 转换
    }
}

/**
 * @brief 读取陀螺仪MPU6050数据
 */
void Read_Gyro(void * argument)
{
    int16_t AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
	while(1)
	{
        //获取并打印当前任务（HomePage_Task）的堆栈剩余空间
        //UBaseType_t stack = uxTaskGetStackHighWaterMark(NULL);
        //printf("Gyro_Task stack remaining: %d\n", stack);

        xSemaphoreTake(Gyro_SignalHandle, pdMS_TO_TICKS(10));
		MPU6050_GetData(&AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ);   //获取陀螺仪数据
        xSemaphoreGive(Gyro_SignalHandle);

		if (!LCD_Mode && AccZ < 1600)
		{
			LCD_WakeUp();   //唤醒屏幕
			LCD_Mode = true;    //屏幕状态：点亮
			vTaskResume(HomePage_TaskHandle);   //恢复Display_HomePage任务
            HomePage_Static();
            xSemaphoreTake(xMutex, portMAX_DELAY);
			lastActivityTime = xTaskGetTickCount(); //更新最后活动时间
            xSemaphoreGive(xMutex);
            vTaskDelay(pdMS_TO_TICKS(500));
            vTaskSuspend(NULL);
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void Read_HeartRate(void * argument) {
    while(1){
        // 安全获取信号量
        if (xSemaphoreTake(HeartRate_SignalHandle, portMAX_DELAY) == pdTRUE) {
            Get_HeartRate();
            xSemaphoreGive(HeartRate_SignalHandle);  // 无需检查，除非信号量可能被删除
        } 
        vTaskDelay(pdMS_TO_TICKS(100));  // 明确使用 pdMS_TO_TICKS 转换
    }
}

/**
 * @brief HomePage界面显示日期、时间、状态栏
 */
void Display_HomePage(void * argument) {
    First_Flag = false;
    while(1){
        if (!First_Flag){
            LCD_WakeUp();
            First_Flag = true;
            HomePage_Static();
            if (Alarm_Flag) {
                LCD_DisplayString(120, 220, "A");
                xSemaphoreTake(xMutex, portMAX_DELAY);
                lastActivityTime = xTaskGetTickCount(); // 更新最后活动时间
                xSemaphoreGive(xMutex);
            }     
        }

        //无操作255后息屏
        if (LCD_Mode && ((xTaskGetTickCount() - lastActivityTime) >= timeoutTicks)) {
            vTaskResume(Gyro_TaskHandle);
            LCD_Sleep();  // 调用 LCD 休眠函数（关闭背光）
            printf("Timeout\r\n");
            LCD_Mode = false;
            vTaskSuspend(HomePage_TaskHandle);
        }

        //按键(Display_HomePage 需要 周期性执行（刷新界面、检测超时），所以不能无限阻塞)
        xSemaphoreTake(Keypad_SignalHandle, pdMS_TO_TICKS(10));
        uint8_t key = Keypad_Scan();
        xSemaphoreGive(Keypad_SignalHandle);
        
        if (key == '4') {
            printf("Enter Menu\n");
			if (eTaskGetState(Menu_TaskHandle) == eSuspended) {
                vTaskResume(Menu_TaskHandle);
                Menu_Flag = false;
                vTaskDelay(pdMS_TO_TICKS(10));
                vTaskSuspend(HomePage_TaskHandle);  // 挂起自身
            } else {
                printf("Error: Menu_Task is already running!\n");
            }
            vTaskDelay(100);
		}

        // 更新主界面动态内容（如刷新时间显示）
        xSemaphoreTake(Time_SignalHandle, portMAX_DELAY);
		HomePage_Dynamic();
        xSemaphoreGive(Time_SignalHandle);
         
		vTaskDelay(100);
    }
}

/**
 * @brief Menu界面显示
 */
void Display_Menu(void *argument) { 
    static uint8_t place = 0;          // 当前选中的位置（0~4）
    const uint8_t max_place = 3;       // 最大选项位置
    Menu_Flag = false;

    while(1) {
        //获取并打印当前任务（Menu_Task）的堆栈剩余空间
        //UBaseType_t stack = uxTaskGetStackHighWaterMark(NULL);
        //taskENTER_CRITICAL();
        //printf("Menu_Task stack remaining: %d\n", stack);
        //taskEXIT_CRITICAL();

        //Menu_Task 是 事件驱动型任务，只有按键触发时才需要运行，因此阻塞等待是合理的
        xSemaphoreTake(Keypad_SignalHandle, portMAX_DELAY);
        uint8_t key = Keypad_Scan();
        xSemaphoreGive(Keypad_SignalHandle);

        if (!Menu_Flag) {
            Select(0);
            Menu_Flag = true;
        }
        
        // 处理按键逻辑
        switch(key) {
            case '2':  // 上移
                printf("Upward\n");
                LCD_Clear();
                place = (place == 0) ? max_place : place - 1;
                Select(place);
                vTaskDelay(100);
                break;
                
            case '8':  // 下移
                printf("Downward\n");
                LCD_Clear();
                place = (place == max_place) ? 0 : place + 1;
                Select(place);
                vTaskDelay(100);
                break;
                
            case '5': {// 确认
                printf("confirm\n");
                bool HeartRate_Flag = true;
                bool firstEnter = true;
                while (HeartRate_Flag) {
                    // 实时更新按键值
                    xSemaphoreTake(Keypad_SignalHandle, pdMS_TO_TICKS(10));
                    key = Keypad_Scan();
                    xSemaphoreGive(Keypad_SignalHandle);
            
                    if (firstEnter) {
                        LCD_Clear();        // 进入后清屏1次
                        firstEnter = false;
                    }
                    
                    switch (place) {
                    case 0:
                        /* code */
                        break;
                    case 1:
                        break;
                    case 2: {
                            xSemaphoreTake(HeartRate_SignalHandle, pdMS_TO_TICKS(10));
                            Interface_HeartRate();
                            xSemaphoreGive(HeartRate_SignalHandle);
                        } break;
                    case 3:
                        break;
                    }
            
                    // 检测退出按键
                    if (key == '3') {
                        printf("Exit\n");
                        LCD_Clear();
                        HeartRate_Flag = false;
                    }

                    if (key == '1') {
                        printf("Enter HomePage\n");
                        LCD_Clear();
                        First_Flag = false; 
                        HeartRate_Flag = false;
                        vTaskResume(HomePage_TaskHandle);
                        vTaskDelay(10);  // 让出 CPU，确保挂起操作完成
                        vTaskSuspend(Menu_TaskHandle);
                    }

                    vTaskDelay(100);  // 防止CPU占用过高
                }
            } break;
                
            case '1':  // 无论在Menu里的哪个界面，按下‘1’后直接返回首页
                printf("Enter HomePage\n");
                LCD_Clear();
                First_Flag = false; 
                vTaskResume(HomePage_TaskHandle);
                vTaskDelay(10);  // 让出 CPU，确保挂起操作完成
                vTaskSuspend(Menu_TaskHandle);
                break;
        }

        //无操作30s后息屏
        if (LCD_Mode && ((xTaskGetTickCount() - lastActivityTime) >= timeoutTicks)) {
            vTaskResume(Gyro_TaskHandle);
            LCD_Sleep();  // 调用 LCD 休眠函数（关闭背光）
            printf("Timeout\r\n");
            LCD_Mode = false;
            LCD_Clear();
            vTaskSuspend(Menu_TaskHandle);
        }
        vTaskDelay(pdMS_TO_TICKS(100));  // 适当延时，防止CPU占用过高
    }
}
