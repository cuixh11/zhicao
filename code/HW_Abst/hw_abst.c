#include "hw_abst.h"
#include "gpio.h"
#include "lcd.h"

#if defined Bootloader
	
#endif
#if defined HostAPP
	#include "http_json.h"
	#include "host_app.h"
	#include "tim.h"
#endif

 /**
 * @brief  System reset //����ϵͳ��λ����������MCU
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��22��
*/
void MCU_System_RST(void)
{
	HAL_NVIC_SystemReset();
}
 /**
 * @brief  ����Ӳ�������ӿڣ����䲻ͬоƬֻ��Ҫ�޸Ĵ˽ӿ� 
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��10��19��
*/
void UART_Send(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	HAL_UART_Transmit(huart,pData,Size,Timeout);
}

 /**
 * @brief  ����2Ӳ�������ӿڣ����䲻ͬоƬֻ��Ҫ�޸Ĵ˽ӿ�,ר����Debug �̶�����2		pa2-TX pa3-RX
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��10��
*/
void DebugPrint_uart(char *pData,  uint32_t Timeout)
{
	uint16_t Size=strlen(pData);
	HAL_UART_Transmit(&huart2,pData,Size,Timeout);
	usb_hid_Tx(pData);
	LCD_StringBUFF_Manage(pData);
//	Debug_Uart_Display(pData,1);
}
void DebugPrint_uart_LineFeed(char *pData,  uint32_t Timeout)
{
	//malloc()
	uint16_t Size_t=strlen(pData);
	char Send_temp[500]={0};
	
	strcpy(Send_temp,"\r\n");
	strcpy(Send_temp+2,pData);
	strcpy(Send_temp+2+Size_t,"\r\n");
	
	//DebugPrint_uart("\r\n",Timeout);
	DebugPrint_uart(Send_temp,Timeout);	
	//DebugPrint_uart("\r\n",Timeout);
}
 /**
 * @brief  GPIOӲ�������ӿڣ����䲻ͬоƬֻ��Ҫ�޸Ĵ˽ӿ� 
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��12��9��
*/
GPIO_PinState GPIO_Read(GPIO_TypeDef* GPIOx_t, uint16_t GPIO_Pin_t)
{
	GPIO_PinState GPIO_bitstatus;
	GPIO_bitstatus=HAL_GPIO_ReadPin(GPIOx_t,GPIO_Pin_t);
	return GPIO_bitstatus;
}
 /**
 * @brief  GPIO KEY //��öKEY�ĵ�ƽ��ȡ������ѭ����һֱ���
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��12��9��
*/
uint8_t KeyLastState[3]={0};
uint8_t KeyState[3]={0};
void Key_read(void)
{
	
	if(GPIO_Read(GPIOF,GPIO_PIN_0)==GPIO_PIN_RESET)
	{
		Delay(10);
		if(GPIO_Read(GPIOF,GPIO_PIN_0)==GPIO_PIN_RESET)
		{
			KeyState[0]=0;
		}
		else
		{	KeyState[0]=1;}
	}
	else
	{KeyState[0]=1;}
	
	if(GPIO_Read(GPIOF,GPIO_PIN_1)==GPIO_PIN_RESET)
	{
		Delay(10);
		if(GPIO_Read(GPIOF,GPIO_PIN_1)==GPIO_PIN_RESET)
		{
			KeyState[1]=0;
		}	
		else
		{	KeyState[1]=1;}		
	}
	else
	{KeyState[1]=1;}	
	
	if(GPIO_Read(GPIOF,GPIO_PIN_2)==GPIO_PIN_RESET)
	{
		Delay(10);
		if(GPIO_Read(GPIOF,GPIO_PIN_2)==GPIO_PIN_RESET)
		{
			KeyState[2]=0;
		}
		else
		{	KeyState[2]=1;}		
	}	
	else
	{KeyState[2]=1;}	
	
	KeyLastState[0]=KeyState[0];
	KeyLastState[1]=KeyState[1];
	KeyLastState[2]=KeyState[2];
	
	
//	if(KeyState[0]==0)
//	{
//		LED_ctrl(0,1);
//	}
//	else
//	{
//		LED_ctrl(0,0);
//	}
//	if(KeyState[1]==0)
//	{
//		  LED_ctrl(1,1);
//	}
//	else
//	{
//		LED_ctrl(1,0);
//	}	
//	if(KeyState[2]==0)
//	{
//		  LED_ctrl(2,1);
//	}	
//	else
//	{
//		LED_ctrl(2,0);
//	}	
}
 /**
 * @brief  LED Control //��öLED�Ŀ���
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��12��21��
*/
void LED_ctrl(uint8_t LEDID,uint8_t LED_state)
{
	switch(LEDID)
	{
		case 0:
		{
			if(LED_state!=0)
			{
				HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10,GPIO_PIN_RESET);
			}
			else
			{
				HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10,GPIO_PIN_SET);
			}
			break;
		}
		case 1:
		{
			if(LED_state!=0)
			{
				HAL_GPIO_WritePin(GPIOG, GPIO_PIN_11,GPIO_PIN_RESET);
			}
			else
			{
				HAL_GPIO_WritePin(GPIOG, GPIO_PIN_11,GPIO_PIN_SET);
			}			
			
			break;
		}
		case 2:
		{
			if(LED_state!=0)
			{
				HAL_GPIO_WritePin(GPIOG, GPIO_PIN_12,GPIO_PIN_RESET);
			}
			else
			{
				HAL_GPIO_WritePin(GPIOG, GPIO_PIN_12,GPIO_PIN_SET);
			}			
			
			break;
		}	
		default:
		{break;}
	}

}


#if defined HostAPP
/**
 * @brief ��ʱ���жϻص�����	
 * @param	TIM_HandleTypeDef *htim	//��ʱ���ṹ��
 * @retval void	
 * @author ZCD1300
 * @Time 2021��11��15��
*/
uint16_t tim3_cnt=0;
uint16_t tim3CNT_Zigbee=0;
uint16_t tim3CNT_TCP=0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==&htim3)
	{//��ʱ��3�жϣ�10s����һ���ж�
		//ע�⣬������ʱ���ж�ʱ�����һ��

		Local_time_refresh(10);//����ʱ��ˢ�¡��ж�����10s
		if(Debug_enable_flag!=1)
		{
			tim3CNT_TCP++;
			if(tim3CNT_TCP>1)
			{
				tim3CNT_TCP=0;

				//Check_TCP_Connection();//�������Ӧ����Debug���ܿ�����ʱ����ʱ����					
			}		
		}
		{//��Ļ�Զ�Ϣ��
			//����Ļ�ڲ�ʵ��
//			DisplaySleep_TimeCNT++;
//			if(DisplaySleep_TimeCNT>=2)
//			{
//				//DisplaySleep_TimeCNT=0;
//				LCD_Status(Sleep_Status);
//				Delay(10);
//				LCD_Status(Sleep_Status);
//			}				
		}
		tim3_cnt++;
		tim3CNT_Zigbee++;
		if(tim3_cnt>6)
		{//1���Ӽ��һ��
			tim3_cnt=0;			
			if(!Host_utoken_Check_SmartNest())
			{//token��Ч����ˢ�³ɹ�
			}
			else
			{//token���»�ȡ��ʱ(10s)���������ӵ�������ʧ��

				//Reg_User_Check();//�����û���¼���棬�����¼ʧ�ܲ����Զ����ص�¼ҳ�棬��һֱ���ԣ���Ҫ�ֶ��л�����¼ҳ��
			}
		}
		if(tim3CNT_Zigbee>60)
		{//ʮ���Ӽ��һ��
			tim3CNT_Zigbee=0;
			if(Debug_enable_flag!=1)//debugģʽ���Զ����
			{
				zigbee_Refesh_OnlineDeviceNUM();//��ʱ���zigbee�����ն�����״̬
				page1();
			}
		}
	}

	
	
}
#endif
/**
 * @brief  Clear_Buff 	//���buff
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��10��
*/
void Clear_Buff(char *Buff,uint16_t Len,uint16_t Buff_Max)
{
	for(uint16_t i=0;i<=Val_MAX_Limit_U(Len+2,Buff_Max);i++)
	{
		Buff[i]=0;
	}
	
}
/**
 * @brief  �ַ���ƥ��  ���������������Ӧ����Ӳ������㣬���ǳ��ã����Ƿŵ�����㣩
 * @param	 char* input ,uint8_t input_Len,char* target,uint8_t target_Len  (�������д���ȾͶ���0���ú����Լ�����)
 * @retval uint8_t 1 ��ʾƥ��ɹ� 0���ɹ�	��ע�������� �ɹ�Ϊ1������if����жϣ�
 * @author ZCD1300
 * @Time 2021��11��7��
*/
uint8_t StringMatch(char* input ,uint8_t input_Len,char *target,uint8_t target_Len)
{
	uint8_t temp_Cnt=0;
	if((input_Len==0)&&(target_Len==0))
	{
		input_Len= strlen(input);
		target_Len=strlen(target);
	
	}
	if(input_Len > target_Len)
	{
		return 0;
	}
		
	for(uint8_t i=0;i<input_Len;i++)
	{
		if(input[i]==target[i])
		{
			temp_Cnt++;
		}
	}
	if(temp_Cnt==input_Len)
	{
		return 1;
	}
	else
	{return 0;}
	
	return 0;
}

 /**
 * @brief  ���������ֵ����(�޷���)  ���������������Ӧ����Ӳ������㣬���ǳ��ã����Ƿŵ�����㣩
 * @param	 uint16_t inputNUM uint16_t MAX_NUM  
 * @retval uint16_t ֱ�ӷ��ط�Χ�ڵ���ֵ
 * @Time 2021��11��8��
*/
uint16_t Val_MAX_Limit_U(uint16_t inputNUM,uint16_t MAX_NUM)
{
	if(inputNUM<=MAX_NUM)
	{
		return inputNUM;
	}
	else{return MAX_NUM;}
}
 /**
 * @brief  ��ʱ����  
 * @param	 uint32_t delay_Time
 * @retval void
 * @Time 2021��11��11��
*/
void Delay(uint32_t delay_Time)
{
	HAL_Delay(delay_Time);
}
 /**
 * @brief  Wait_Until_FlagTrue  //�ȴ�ֱ��FlagΪTure;ע����뺯�� ���� ��Flag�Զ����㣬Ҫ�յ���λ
 * @param	 uint8_t *Flag uint32_t Timeout ��λms,������ͷ�ļ��޸�,�������0�������Ƴ�ʱʱ�䣻
 * @retval int8_t return //0��ʱ  1�����ɹ�
 * @Time 2021��11��14��
*/
uint8_t Inv_optim_t=0;

int8_t Wait_Until_FlagTrue(uint8_t *Flag,uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Timeout;
	Inv_optim_t=*Flag;
	if(wait==0)
	{//�����ó�ʱʱ��
		while(*Flag==0)
		{
			Inv_optim_t=*Flag;			
		}
		return 1;//�����ɹ�
	}
	else
	{
		if (wait < WaitUntilFlagTrue_MAX)
		{
			wait += (uint32_t)(uwTickFreq);
		}
		
		while((HAL_GetTick() - tickstart) < wait)//��ʱʱ��֮��
		{
			if(*Flag==1)
			{//δ��ʱ�������ɹ�
				return 1;
			}

		}
		//�ȴ���ʱ
		return 0;		
	}
}
/**
 * @brief Uart_Debug_CMD_ADD	//����Debug����ָ��
 * @param void	�����������CmdLen_t ����0���ú����Լ����������ǿ��ܴ��ڳ��Ȳ�ƥ�䡣
 * @retval void	1��ʾƥ��ɹ���0��ʾ���ɹ�
 * @author ZCD1300
 * @Time 2021��11��15��
*/
int8_t Uart_Debug_CMD_ADD(char *Cmd_t,uint8_t *UartBuff,uint8_t CmdLen_t,uint16_t MatchDeep_t)
{
	if(CmdLen_t==0)
	{
		CmdLen_t=strlen(Cmd_t);
	}
	if(StringSearch_Assign(Cmd_t,UartBuff,CmdLen_t,MatchDeep_t))
	{
		Clear_Buff(UartBuff,MatchDeep_t,BuffSIZE_MAX2);
		return 1;
	}		
	else
	{return 0;}	
}




