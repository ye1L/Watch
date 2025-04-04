#include "Delay.h"
#include "PulseSensor.h"
#include "stm32f10x.h"
#include "Tim.h"
			 
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3																	   
void  PulseSensor_ADC_Init(void)
{ 	
	TIM3_Configuration();  // 调用TIM3初始化
	//时钟配置
	RCC_APB2PeriphClockCmd(PulseSensor_GPIO_CLK|ADC_CLK,ENABLE);//使能GPIO端口和ADC的时钟(APB2总线)
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//配置ADC时钟为PCLK2的6分频，STM32最大ADC时钟为14MHz，通常系统时钟72MHz/6=12MHz
	
	//GPIO配置
    GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin=PulseSensor_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_Init(PulseSensor_GPIO_PORT,& GPIO_InitStructure);
	
	//ADC基本参数配置
	ADC_DeInit(ADCx);	//复位ADC
	
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADCx和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	      //模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	  //转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;//选择TIM3作为外部触发源
	ADC_InitStructure.ADC_DataAlign= ADC_DataAlign_Right;                	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	                              //顺序进行规则转换的ADC通道的数目
	ADC_Init(ADCx, &ADC_InitStructure);	
	
	ADC_ExternalTrigConvCmd(ADCx,ENABLE);//采用外部触发
	//设置指定规则组通道序列和采样时间
	ADC_RegularChannelConfig(ADCx, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);//adc转换时间21us
	ADC_Cmd(ADCx, ENABLE);
	
	ADC_ResetCalibration(ADCx);									//复位校准
	while(ADC_GetResetCalibrationStatus(ADCx));	//等待校准结束，校准结束状态为RESET
	ADC_StartCalibration(ADCx);									//AD校准
	while(ADC_GetCalibrationStatus(ADCx));			//等待校准结束	
 
}				  
//获得ADC值
//ch:通道值 0~3
uint16_t Get_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADCx, ch, 1, ADC_SampleTime_239Cycles5 );	//ADCx,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADCx, ENABLE);		//使能指定的ADCx的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC ));//等待转换结束
 
	return ADC_GetConversionValue(ADCx);	//返回最近一次ADCx规则组的转换结果
}
 
uint16_t Get_Adc_Average(u8 ch, u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t = 0; t < times; t ++)
	{
		temp_val += Get_Adc(ch);
		Delay_ms(5);
	}
	return temp_val / times;
} 	 

/* 全局变量定义 */
uint8_t IBI = 0;            // 心跳间隔（Inter-Beat Interval）
uint16_t Pulse[128] = {0}; // 存储ADC采样数据的数组
uint8_t Pos = 0;          // 数组位置索引


/**
  * @brief  获取脉搏数据并计算心率
  * @param  pulse: 输出心率值（BPM）
  * @param  maxValue: 输出波形最大值
  * @note   需要定期调用（如每10ms调用一次）
  */
void getPulse(uint8_t *pulse, uint16_t *maxValue)
{
    uint16_t Data = 0;
    
    /* 初始化输出参数 */
    *pulse = 0;
    *maxValue = 0;
    
    /* 标准库ADC读取（假设使用ADC1） */
    Data = ADC_GetConversionValue(ADC1);
    
    /* 存储采样数据 */
    Pulse[Pos++] = Data;
    
    /* 当采集满128个点时进行处理 */
    if (Pos >= 128)
    {
        Pos = 0;
        scaleData();            // 数据归一化处理
        calculatePulse(pulse, maxValue); // 计算心率和IBI
    }
}

/**
  * @brief  数据归一化处理
  * @note   将原始ADC数据缩放到合理范围
  */
void scaleData(void)
{
    uint8_t i = 0;
    uint16_t Max = 0, Min = 0, Delter = 0;
    
    /* 获取数组最大值和最小值 */
    Max = getArrayMax(Pulse, 128);
    Min = getArrayMin(Pulse, 128);
    Delter = Max - Min;
    
    /* 如果信号变化太小，视为无效数据 */
    if(Delter < 200)
    {
        for(i = 0; i < 128; i++)
            Pulse[i] = Delter / 2; // 填充中间值
    }
    else 
    {
        /* 归一化处理 */
        for(i = 0; i < 128; i++)
            Pulse[i] = (Pulse[i] - Min) * 1000 / Delter; // 缩放到0-1000范围
    } 
}

/**
  * @brief  计算心率和IBI
  * @param  pulse: 输出心率值（BPM）
  * @param  maxValue: 输出波形最大值
  */
static void calculatePulse(uint8_t *pulse, uint16_t *maxValue)
{
    uint8_t i = 0, firstTime = 0, secondTime = 0;
    uint8_t PrePulse = 0, CurrentPulse = 0;
    uint16_t Max = 0, Min = 0, Mid = 0;
    
    /* 获取波形特征值 */
    Max = getArrayMax(Pulse, 128);
    Min = getArrayMin(Pulse, 128);
    Mid = (Max + Min) / 2;
    *maxValue = Max;
    
    /* 检测上升沿（心跳点） */
    for(i = 0; i < 128; i++)
    {
        PrePulse = CurrentPulse;
        CurrentPulse = (Pulse[i] > Mid) ? 1 : 0; // 二值化处理
        
        /* 检测上升沿 */
        if(PrePulse == 0 && CurrentPulse == 1)
        {
            if(!firstTime) 
                firstTime = i; // 第一个上升沿
            else if(firstTime && firstTime < i)
            {
                secondTime = i; // 第二个上升沿
                break;      
            }
        }
    }
    
    /* 计算心率和IBI */
    if((secondTime - firstTime) > 0)
    {
        IBI = (secondTime - firstTime) * HEART_PERIOD; // 计算心跳间隔
        *pulse = 60000 / (IBI);    // 计算BPM = 60*1000/IBI(ms)
    }
    else
    {
        *pulse = 0; // 无效数据
    }
    
    /* 串口输出调试信息 */
    printf("BPM = %d, IBI = %dms\r\n", *pulse, IBI);
}

/**
  * @brief  获取数组最大值
  * @param  arr: 输入数组
  * @param  size: 数组大小
  * @retval 数组最大值
  */
uint16_t getArrayMax(uint16_t arr[], int size) 
{
    if (size == 0) return 0;
 
    uint16_t max = arr[0];
    for (int i = 1; i < size; i++) 
    {
        if (arr[i] > max) 
        {
            max = arr[i];
        }
    }
    return max;
}

/**
  * @brief  获取数组最小值
  * @param  arr: 输入数组
  * @param  size: 数组大小
  * @retval 数组最小值
  */
uint16_t getArrayMin(uint16_t arr[], int size)
{
    if (size == 0) return 0xFFFF;
    
    uint16_t min = arr[0];
    for (int i = 1; i < size; i++)
    {
        if (arr[i] < min)
        {
            min = arr[i];
        }
    }
    return min;
}
