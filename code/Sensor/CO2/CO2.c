#include "CO2.h"
#include "stm32f1xx_hal.h"
#include "usart.h"


uint8_t  CO2AskBuffer[9]={0xFF,0x01,0x03,0x02,0x00,0x00,0x00,0x00,0xFB};//����ģ����ѯģʽ
uint8_t  RecieveBuffer[1]={0};//�ݴ���յ����ַ�
uint8_t  CO2TxBuffer[9]={0xFF,0x01,0x03,0x03,0x01,0x00,0x00,0x00,0xF9};//��ȡCO2Ũ��ָ��
uint8_t  CO2Buffer[12];//���յ���CO2Ũ��
uint8_t  RxLine=0; //���յ������ݳ���
uint8_t errorBuffer[]="\r\nerror\r\n"; //����ʱ����

/**
 * @brief ����ģ��Ϊ��ѯģʽ
 * @param	void
 * @retval void
 * @author LDX2000
 * @Time 2021��11��15��
*/
void CO2_Ask()
{
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)CO2AskBuffer, sizeof(CO2AskBuffer));
}

/**
 * @brief ��һ��ָ��ظ�һ��CO2Ũ��ֵ
 * @param	void
 * @retval void
 * @author LDX2000
 * @Time 2021��11��15��
*/
void CO2_Tx()
{
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)CO2TxBuffer, sizeof(CO2TxBuffer));
	HAL_UART_Receive_IT(&huart1, (uint8_t *)RecieveBuffer, 1);
}

/**
 * @brief ���ջص����ѽ��յ������ݱ���
 * @param	���ں�
 * @retval void
 * @author LDX2000
 * @Time 2021��11��15��
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	UNUSED(huart);
	RxLine++;
	CO2Buffer[RxLine-1]=RecieveBuffer[0];
	if (RxLine>12)
	{
		RxLine=0;
		memset(CO2Buffer,0,sizeof(CO2Buffer)); 
		HAL_UART_Transmit_IT(&huart1, (uint8_t *)errorBuffer,sizeof(errorBuffer));
	}
	if (RecieveBuffer[0]==0x0A)
	{
		HAL_UART_Transmit_IT(&huart1, (uint8_t *)CO2Buffer,RxLine);
		RxLine=0;	
	}

	RecieveBuffer[0]=0;
	HAL_UART_Receive_IT(&huart1, (uint8_t *)RecieveBuffer, 1);
}
