#include "NH3.h"
#include "stm32f1xx_hal.h"
#include "usart.h"
#include "stdio.h"



uint8_t  NH3TxBuffer[9]={0xFF,0x01,0x07,0x00,0x00,0x00,0x00,0x00,0x07};//问询浓度指令
uint8_t  NH3Buffer[10]; //存放模块返回的数据 16进制
uint8_t  RxLine=0; //接收到的数据长度
uint8_t  RecieveBuffer[1]={0};//临时存放数据
uint8_t  gaozijie;  //NH3浓度高字节
uint8_t  dizijie;   //NH3浓度低字节
uint16_t  NH3concentration;
double  NH3Dec;   //十进制的NH3浓度
uint8_t errorBuffer[]="\r\nerror\r\n";
extern uint8_t flag;

/**
 * @brief 发送一次指令接收一次NH3浓度
 * @param	void
 * @retval void
 * @author LDX2000
 * @Time 2021年11月15日
*/
void NH3_Tx()
{
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)NH3TxBuffer, sizeof(NH3TxBuffer));
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
	RxLine++;  //接收到的数据加一
	NH3Buffer[RxLine-1]=RecieveBuffer[0];  //保存收到的正确数据
//判断帧头是否正确
	if (RxLine==1)
	{
		if(RecieveBuffer[0]==0xFF)
		{
			RxLine=1;
		}
		else
		{
			RxLine=0;
		}	
	}
	else if(RxLine==2)
	{
		if(RecieveBuffer[0]==0x01)
		{
			RxLine=2;
		}
		else
		{
			RxLine=0;
		}	
	}
	else if(RxLine==3)
	{
		if(RecieveBuffer[0]==0x07)
		{
			RxLine=3;
		}
		else
		{
			RxLine=0;
		}	
	}
	else if(RxLine==4)
	{
		if(RecieveBuffer[0]==0x02)  //分辨率固定为0x02，即0.01
		{
			RxLine=4;
		}
		else
		{
			RxLine=0;
		}	
	}
	else if(RxLine==5)
	{
		gaozijie=RecieveBuffer[0];  //保存NH3浓度高字节
	}
	else if(RxLine==6)
	{
		dizijie=RecieveBuffer[0];   //保存NH3浓度低字节
		//16进制转换10进制,分辨率0.01   计算十进制NH3浓度
		NH3Dec=gaozijie*256+dizijie*1;
		NH3Dec=NH3Dec*0.01;     
	}
	else if(RxLine==7)
	{
		if(RecieveBuffer[0]==0x00) 
		{
			RxLine=7;
		}
		else
		{
			RxLine=0;
		}	
	}
	else if(RxLine==8)
	{
		if(RecieveBuffer[0]==0x00)  
		{
			RxLine=8;
		}
		else
		{
			RxLine=0;
		}	
	}
	else if(RxLine==9)
	{
		if(RecieveBuffer[0]==0x00)  
		{
			RxLine=9;
		}
		else
		{
			RxLine=0;
		}	
	}
	else if(RxLine==10&flag==0)
	{
		printf("%.2f ppm",NH3Dec);  
		flag=1;
		RxLine=0;	
	}
	else 
	{
		flag=1;
		RxLine=0;	
	}
	
	RecieveBuffer[0]=0;
	HAL_UART_Receive_IT(&huart1, (uint8_t *)RecieveBuffer, 1);
}




