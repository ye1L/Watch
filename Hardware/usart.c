#include "usart.h"

 /**
  * @brief  ����Ƕ�������жϿ�����NVIC
  * @param  ��
  * @retval ��
  */
static void NVIC_Configuration(void)
{
  	NVIC_InitTypeDef NVIC_InitStructure;
  
  	/* Ƕ�������жϿ�������ѡ�� */
  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  	/* ����USARTΪ�ж�Դ */
  	NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART_IRQ;
  	/* �������ȼ�*/
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  	/* �����ȼ� */
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  	/* ʹ���ж� */
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	/* ��ʼ������NVIC */
 	NVIC_Init(&NVIC_InitStructure);
}

 /**
  * @brief  USART GPIO ����,������������
  * @param  ��
  * @retval ��
  */
void USART_Config(void)
{
	DEBUG_USART_GPIO_APBxClkCmd(DEBUG_USART_GPIO_CLK, ENABLE);	// �򿪴���GPIO��ʱ��
	DEBUG_USART_APBxClkCmd(DEBUG_USART_CLK, ENABLE);			// �򿪴��������ʱ��

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  	// ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);
	
	// ���ô��ڵĹ�������
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = DEBUG_USART_BAUDRATE;			// ���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;			// ���� �������ֳ�
	USART_InitStructure.USART_StopBits = USART_StopBits_1;				// ����ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No ;				// ����У��λ
	USART_InitStructure.USART_HardwareFlowControl = 					// ����Ӳ��������
	USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		// ���ù���ģʽ���շ�һ��
	USART_Init(DEBUG_USARTx, &USART_InitStructure);						// ��ɴ��ڵĳ�ʼ������
	
	// �����ж����ȼ�����
	NVIC_Configuration();
	
	// ʹ�ܴ��ڽ����ж�
	USART_ITConfig(DEBUG_USARTx, USART_IT_RXNE, ENABLE);	
	
	// ʹ�ܴ���
	USART_Cmd(DEBUG_USARTx, ENABLE);	    
}

///�ض���c�⺯��printf�����ڣ��ض�����ʹ��printf����
int fputc(int ch, FILE *f)
{
		/* ����һ���ֽ����ݵ����� */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///�ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f)
{
		/* �ȴ������������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}

