#ifndef _DS3231_H
#define _DS3231_H

void DS3231_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t DS3231_ReadReg(uint8_t RegAddress);
void DS3231_Init(void);
void DS3231_SetTime(uint8_t year, uint8_t mon, uint8_t da, uint8_t week, uint8_t hour, uint8_t min, uint8_t sec);
void DS3231_GetTime(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *week, uint8_t *hour, uint8_t *min, uint8_t *sec);
void LCD_DisplayTime(void);

#endif
