#include "Delay.h"
#include "PulseSensor.h"
#include "stm32f10x.h"
			 
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3																	   
void  PulseSensor_ADC_Init(void)
{ 	
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
