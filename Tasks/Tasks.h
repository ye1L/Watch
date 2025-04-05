#include "semphr.h"

#ifndef __TASKS_H
#define __TASKS_H

extern SemaphoreHandle_t Time_SignalHandle;
extern SemaphoreHandle_t Keypad_SignalHandle;
extern SemaphoreHandle_t Gyro_SignalHandle;
extern SemaphoreHandle_t HeartRate_SignalHandle;
extern SemaphoreHandle_t xMutex;  // 互斥量句柄

void Start_Task(void * argument);
void TimeUpdate(void * argument);
void Read_Gyro(void * argument);
void Read_HeartRate(void * argument);
void Display_HomePage(void * argument);
void Display_Menu(void * argument);
void Keypad(void *argument);

#endif
