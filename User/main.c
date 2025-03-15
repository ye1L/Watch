#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "usart.h"
#include "OLED.h"
#include "Key.h"


TaskHandle_t mytaskHandler1;
TaskHandle_t mytaskHandler2;
TaskHandle_t mytaskHandler3;
TaskHandle_t StartTaskHandler;
//StackType_t mytaskStack[128];
//StaticTask_t mystaskTCB;

void mytask1(void *arg)
{
	while(1)
	{
		taskENTER_CRITICAL();
		printf("mytask1 is running!\r\n");
		taskEXIT_CRITICAL();
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);
		vTaskDelay(100);
		GPIO_SetBits(GPIOB, GPIO_Pin_13);
		vTaskDelay(100);
	}
}

void mytask2(void *arg)
{
	while(1)
	{
		taskENTER_CRITICAL();
		printf("mytask2 is running!\r\n");
		taskEXIT_CRITICAL();
		OLED_ShowString(1, 1, "STM32F103ZET6");
		OLED_ShowString(2, 1, "FreeRTOS");
		vTaskDelay(100);
	}
}

void mytask3(void *arg)
{
	while(1)
	{
		taskENTER_CRITICAL();
		printf("mytask3 is running!\r\n");
		taskEXIT_CRITICAL();
		if (Key_GetNum() == 1)		//PB1
		{
			taskENTER_CRITICAL();
			printf("Suspending mytask1...\r\n");
			taskEXIT_CRITICAL();
			vTaskSuspend(mytaskHandler1);	//挂起mytask1
		}
		if (Key_GetNum() == 2)	//PB11
		{
			taskENTER_CRITICAL();
			printf("Resuming mytask1...\r\n");
			taskEXIT_CRITICAL();
			OLED_ShowString(3, 1, "OK");
			vTaskResume(mytaskHandler1);	//恢复mytask1
		}
		vTaskDelay(100);
	}
}

void StartTask(void *arg)
{
	taskENTER_CRITICAL();
	printf("StartTask is running! \r\n");
	taskEXIT_CRITICAL();
	xTaskCreate(mytask1, "LED", 128, NULL, 2, &mytaskHandler1);
	xTaskCreate(mytask2, "OLED", 512, NULL, 2, &mytaskHandler2);
	xTaskCreate(mytask3, "Key_Control", 128, NULL, 2, &mytaskHandler3);
	//xTaskCreateStatic(mytask, "mytask", 128, NULL, 2, mytaskStack, &mystaskTCB);
	vTaskDelete(NULL);
}

int main(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOA的时钟
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*外设初始化*/
	USART_Config();
	OLED_Init();
	Key_Init();
	/*创建开始任务、开启调度*/
	xTaskCreate(StartTask, "StartTask", 128, NULL, 2, &StartTaskHandler);
	vTaskStartScheduler();	//启动 FreeRTOS 的任务调度器

	while(1);
}




/*
/// @brief 静态内存分配需要用到的函数
StaticTask_t IdleTaskTCB;
StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, 
									StackType_t **ppxIdleTaskStackBuffer, 
									uint32_t *pulIdleTaskStackSize )
{
	*ppxIdleTaskTCBBuffer = &IdleTaskTCB;
	*ppxIdleTaskStackBuffer = IdleTaskStack;
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

// 定义定时器任务的控制块和堆栈
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, 
									StackType_t **ppxTimerTaskStackBuffer, 
									uint32_t *pulTimerTaskStackSize )
{
	// 返回定时器任务的内存
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
*/