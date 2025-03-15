#include "stm32f10x.h"                  // Device header
#include "DS3231.h"
#include "DS3231_Reg.h"
#include "MyI2C.h"

#define DS3231_ADDRESS 	    0xD0//DS3213的写地址（这里是写地址，方便后面代码的编写，把0位也计算进去了）
								//DS3231的IIC地址：0x68

/**
  * 函    数：DS3231写寄存器
  * 参    数：RegAddress 寄存器地址
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void DS3231_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(DS3231_ADDRESS);	    //发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(Data);				//发送要写入寄存器的数据
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_Stop();						//I2C终止
}

/**
  * 函    数：DS3231读寄存器
  * 参    数：RegAddress 寄存器地址
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
uint8_t DS3231_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(DS3231_ADDRESS);	    //发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	
	MyI2C_Start();						    //I2C重复起始
	MyI2C_SendByte(DS3231_ADDRESS | 0x01);	//发送从机地址，读写位为1，表示即将读取
	MyI2C_ReceiveAck();					    //接收应答
	Data = MyI2C_ReceiveByte();			    //接收指定寄存器的数据
	MyI2C_SendAck(1);					    //发送应答，给从机非应答，终止从机的数据输出
	MyI2C_Stop();						    //I2C终止
	
	return Data;
}

void DS3231_Init(void)
{
	MyI2C_Init();									//先初始化底层的I2C
	DS3231_WriteReg(DS3231_ADDRESS_CONTROL, 0x1C);      //初始化控制寄存器

}

/**
  * 函    数：BCD(8421)转DEC.//二进制转十进制
  * 参    数：val：BCD码.
  * 参    数：i：DEC码.
  * 返 回 值：无
  */
uint8_t BCD_DEC(u8 val)
{
	u8 i;
	i = val & 0x0f;   //取原val低4位
	val >>= 4;        //原val右移4位
	val &= 0x0f;      //相当于取原val的高4位
	val *= 10;        
	i += val;    
	return i;
}

/**
  * 函    数：DEC转BCD(8421).//十进制转二进制
  * 参    数：val：DEC码.
  * 参    数：k：BCD码.
  * 返 回 值：无
  */
uint8_t DEC_BCD(u8 val)
{
    u8 i,j,k;
    i = val / 10;
    j = val % 10;
    k = j + (i << 4);
    return k;
}


/**
  * 函    数：时间设置
  *	参    数：分别输入 年 月 日 星期 时 分 秒
  * 返 回 值：无
  */
void DS3231_SetTime(u8 year, u8 mon, u8 da, u8 week, u8 hour, u8 min, u8 sec)
{
    //u8  year = 0x23;    //23年 0010 0011 
    //u8	mon = 0x10;     //10月 0001 0000
    //u8	data = 0x13;    //13日 0001 0011
    //u8	week = 0x06;    //周6 0000 0110
    //u8	hour = 0x08;    //8时 0000 1000
    //u8	min = 0x08;     //8分
    //u8	sec = 0x08;     //8秒 
    DS3231_WriteReg(0x06,DEC_BCD(year));
 
    DS3231_WriteReg(0x05,DEC_BCD(mon));
 
    DS3231_WriteReg(0x04,DEC_BCD(da));
    
    DS3231_WriteReg(0x03,DEC_BCD(week));
 
    DS3231_WriteReg(0x02,DEC_BCD(hour));
 
    DS3231_WriteReg(0x01,DEC_BCD(min));
 
    DS3231_WriteReg(0x00,DEC_BCD(sec));
 
}	


/**
  * 函    数：DS3231获取数据
  * 参    数：year--年,month--月,date--日,week--周几,hour--时,min--分,sec--秒,
             使用输出参数的形式返回，范围：0-255
  * 返 回 值：无
  */
void DS3231_GetTime(u8 *year, u8 *month, u8 *date, u8 *week, u8 *hour, u8 *min, u8 *sec)
{
	*year=DS3231_ReadReg(0x06);  
	*year=BCD_DEC(*year);
 
	*month=DS3231_ReadReg(0x05); 
	*month=BCD_DEC(*month);
 
	*date=DS3231_ReadReg(0x04);  
	*date=BCD_DEC(*date);
 
	*week=DS3231_ReadReg(0x03);  
	*week=BCD_DEC(*week);
 
	*hour=DS3231_ReadReg(0x02); 
	*hour&=0x3f;  //设定为12小时制的时候起作用，去除小时制设定位的读取                 
	*hour=BCD_DEC(*hour);
 
	*min=DS3231_ReadReg(0x01);
	*min=BCD_DEC(*min);
 
	*sec=DS3231_ReadReg(0x00);
	*sec=BCD_DEC(*sec);
}
