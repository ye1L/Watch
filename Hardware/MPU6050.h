#ifndef __MPU6050_H
#define __MPU6050_H

#define			GPIO_Pin_SCL			GPIO_Pin_12
#define			GPIO_Pin_SDA			GPIO_Pin_11

void MPU6050_W_SCL(uint8_t BitValue);
void MPU6050_W_SDA(uint8_t BitValue);
uint8_t MPU6050_R_SDA(void);
void MPU6050_I2C_Init();
void MPU6050_Start(void);
void MPU6050_Stop(void);
void MPU6050_SendByte(uint8_t Byte);
uint8_t MPU6050_ReceiveByte(void);
void MPU6050_SendAck(uint8_t AckBit);
uint8_t MPU6050_ReceiveAck(void);
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t RegAddress);
void MPU6050_Init(void);
uint8_t MPU6050_GetID(void);
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ);
#endif
