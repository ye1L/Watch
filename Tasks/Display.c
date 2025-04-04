#include "FreeRTOS.h"
#include "task.h"
#include "string.h"

#include "lcd_spi_154.h"
#include "DS3231.h"
#include "string.h"
#include "Menu_Fonts.h"
#include "PulseSensor.h"

/**
 * @brief HomePage显示界面
 * 
 */
void HomePage_Static(void) 
{
    /* 1. 绘制静态界面框架 */
    LCD_SetColor(LCD_WHITE);     // 黑色画笔
    LCD_SetBackColor(LCD_WHITE); // 白色背景
    LCD_FillRect(0, 0, 240, 240);    // 绘制边框
    
    /* 2. 显示固定文字 */
    LCD_SetColor(LCD_BLACK);     // 黑色画笔
    LCD_SetBackColor(LCD_WHITE); // 白色背景
    LCD_DisplayString(10, 10, "Date:");
    LCD_DisplayString(10, 50, "Time:");
    /* 3. 底部状态栏 */
    LCD_DisplayString(5, 220, "MENU");
    LCD_DisplayString(180, 220, "SET");
}

void HomePage_Dynamic(void) 
{
    // 获取时间并显示
    uint8_t year, month, date, week, hour, min, sec;
    char dateString[20];
    char timeString[20];
    char weekString[10];
    DS3231_GetTime(&year, &month, &date, &week, &hour, &min, &sec);
    // 将week数值转换为星期字符串
    switch(week) {
        case Mon: strcpy(weekString, "Mon"); break;
        case Tue: strcpy(weekString, "Tue"); break;
        case Wed: strcpy(weekString, "Wed"); break;
        case Thu: strcpy(weekString, "Thu"); break;
        case Fri: strcpy(weekString, "Fri"); break;
        case Sat: strcpy(weekString, "Sat"); break;
        case Sun: strcpy(weekString, "Sun"); break;
        default:  strcpy(weekString, "Err"); break;  // 错误处理
    }
    // 格式化时间字符串
    snprintf(dateString, sizeof(dateString), "%04d/%02d/%02d", 2000 + year, month, date);
    snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", hour, min, sec);
    snprintf(weekString, sizeof(weekString), "%s", weekString);

    // 设置显示颜色
    LCD_SetColor(LCD_BLACK);  // 设置字体颜色为黑色
    LCD_SetBackColor(LCD_WHITE);  // 设置背景颜色为白色
    // 显示时间字符串
    LCD_DisplayString(70, 10, dateString);  // 在坐标 (10, 10) 处显示日期
    LCD_DisplayString(70, 50, timeString);  // 在坐标 (10, 10) 处显示时间
    LCD_DisplayString(10, 90, weekString); 

    vTaskDelay(20);
}

/**
 * @brief Menu显示界面
 * 
 */
// 静态画面绘制
void Menu_Static(void)
{  
    LCD_WakeUp();
    
    LCD_SetColor(LCD_BLACK);     // 黑色画笔
    LCD_SetBackColor(LCD_WHITE); // 白色背景
    LCD_FillRect(0, 0, 240, 240);    
}

// 菜单图标绘制
void Select(uint8_t place) 
{
    LCD_SetColor(LCD_BLACK);
    LCD_SetBackColor(LCD_WHITE);
    switch (place) 
    {
        case 0: LCD_DrawImage(0, 0, 240, 240, (const uint8_t *)LCD_Alarm); break;
        case 1: LCD_DrawImage(0, 0, 240, 240, (const uint8_t *)LCD_Calendar); break;
        case 2: LCD_DrawImage(0, 0, 240, 240, (const uint8_t *)LCD_HeartRate); break;
        case 3: LCD_DrawImage(0, 0, 240, 240, (const uint8_t *)LCD_Setting); break;
    }
}

/**
 * @brief HeartRate显示界面
 * 
 */
void Interface_HeartRate(void)
{
    printf("--------");
	char HeartRate[20];
	uint16_t val = Get_Adc_Average(0, 50);
    printf("%d\n", val);
	// 格式化时间字符串
	snprintf(HeartRate, sizeof(HeartRate), "HeartRate: %d", val);

	LCD_SetColor(LCD_BLACK);  // 设置字体颜色为黑色
	LCD_SetBackColor(LCD_WHITE);  // 设置背景颜色为白色
	LCD_DisplayString(10, 10, HeartRate); 
}
