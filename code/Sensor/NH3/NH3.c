#include "NH3.h"
#include "stm32f1xx_hal.h"
#include "usart.h"
#include "stdio.h"



uint8_t  NH3TxBuffer[9]={0xFF,0x01,0x07,0x00,0x00,0x00,0x00,0x00,0x07};//��ѯŨ��ָ��
uint8_t  NH3Buffer[10]; //���ģ�鷵�ص����� 16����
uint8_t  RxLine=0; //���յ������ݳ���
uint8_t  RecieveBuffer[1]={0};//��ʱ�������
uint8_t  gaozijie;  //NH3Ũ�ȸ��ֽ�
uint8_t  dizijie;   //NH3Ũ�ȵ��ֽ�
uint16_t  NH3concentration;
double  NH3Dec;   //ʮ���Ƶ�NH3Ũ��
uint8_t errorBuffer[]="\r\nerror\r\n";
extern uint8_t flag;

/**
 * @brief ����һ��ָ�����һ��NH3Ũ��
 * @param	void
 * @retval void
 * @author LDX2000
 * @Time 2021��11��15��
*/
void NH3_Tx()
{
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)NH3TxBuffer, sizeof(NH3TxBuffer));
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
	RxLine++;  //���յ������ݼ�һ
	NH3Buffer[RxLine-1]=RecieveBuffer[0];  //�����յ�����ȷ����
//�ж�֡ͷ�Ƿ���ȷ
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
		if(RecieveBuffer[0]==0x02)  //�ֱ��ʹ̶�Ϊ0x02����0.01
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
		gaozijie=RecieveBuffer[0];  //����NH3Ũ�ȸ��ֽ�
	}
	else if(RxLine==6)
	{
		dizijie=RecieveBuffer[0];   //����NH3Ũ�ȵ��ֽ�
		//16����ת��10����,�ֱ���0.01   ����ʮ����NH3Ũ��
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




