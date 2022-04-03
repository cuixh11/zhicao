#include "CO2.h"
#include "stm32f1xx_hal.h"
#include "usart.h"


uint8_t  CO2AskBuffer[9]={0xFF,0x01,0x03,0x02,0x00,0x00,0x00,0x00,0xFB};//设置模块问询模式
uint8_t  RecieveBuffer[1]={0};//暂存接收到的字符
uint8_t  CO2TxBuffer[9]={0xFF,0x01,0x03,0x03,0x01,0x00,0x00,0x00,0xF9};//读取CO2浓度指令
uint8_t  CO2Buffer[12];//接收到的CO2浓度
uint8_t  RxLine=0; //接收到的数据长度
uint8_t errorBuffer[]="\r\nerror\r\n"; //错误时发送

/**
 * @brief 设置模块为问询模式
 * @param	void
 * @retval void
 * @author LDX2000
 * @Time 2021年11月15日
*/
void CO2_Ask()
{
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)CO2AskBuffer, sizeof(CO2AskBuffer));
}

/**
 * @brief 发一次指令，回复一次CO2浓度值
 * @param	void
 * @retval void
 * @author LDX2000
 * @Time 2021年11月15日
*/
void CO2_Tx()
{
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)CO2TxBuffer, sizeof(CO2TxBuffer));
	HAL_UART_Receive_IT(&huart1, (uint8_t *)RecieveBuffer, 1);
}

/**
 * @brief 接收回调：把接收到的数据保存
 * @param	串口号
 * @retval void
 * @author LDX2000
 * @Time 2021年11月15日
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
