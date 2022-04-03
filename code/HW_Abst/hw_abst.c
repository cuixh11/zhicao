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
 * @brief  System reset //启动系统复位请求以重置MCU
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月22日
*/
void MCU_System_RST(void)
{
	HAL_NVIC_SystemReset();
}
 /**
 * @brief  串口硬件抽象层接口，适配不同芯片只需要修改此接口 
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年10月19日
*/
void UART_Send(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	HAL_UART_Transmit(huart,pData,Size,Timeout);
}

 /**
 * @brief  串口2硬件抽象层接口，适配不同芯片只需要修改此接口,专用与Debug 固定串口2		pa2-TX pa3-RX
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月10日
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
 * @brief  GPIO硬件抽象层接口，适配不同芯片只需要修改此接口 
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月9日
*/
GPIO_PinState GPIO_Read(GPIO_TypeDef* GPIOx_t, uint16_t GPIO_Pin_t)
{
	GPIO_PinState GPIO_bitstatus;
	GPIO_bitstatus=HAL_GPIO_ReadPin(GPIOx_t,GPIO_Pin_t);
	return GPIO_bitstatus;
}
 /**
 * @brief  GPIO KEY //三枚KEY的电平读取，放在循环中一直检测
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月9日
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
 * @brief  LED Control //三枚LED的控制
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月21日
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
 * @brief 定时器中断回调函数	
 * @param	TIM_HandleTypeDef *htim	//定时器结构体
 * @retval void	
 * @author ZCD1300
 * @Time 2021年11月15日
*/
uint16_t tim3_cnt=0;
uint16_t tim3CNT_Zigbee=0;
uint16_t tim3CNT_TCP=0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==&htim3)
	{//定时器3中断，10s进入一次中断
		//注意，开启定时器中断时会进入一次

		Local_time_refresh(10);//本地时间刷新、中断周期10s
		if(Debug_enable_flag!=1)
		{
			tim3CNT_TCP++;
			if(tim3CNT_TCP>1)
			{
				tim3CNT_TCP=0;

				//Check_TCP_Connection();//这个函数应该在Debug功能开启的时候临时跳过					
			}		
		}
		{//屏幕自动息屏
			//由屏幕内部实现
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
		{//1分钟检测一次
			tim3_cnt=0;			
			if(!Host_utoken_Check_SmartNest())
			{//token有效或者刷新成功
			}
			else
			{//token重新获取超时(10s)，或者连接到服务器失败

				//Reg_User_Check();//返回用户登录界面，这里登录失败不再自动返回登录页面，会一直重试，需要手动切换到登录页面
			}
		}
		if(tim3CNT_Zigbee>60)
		{//十分钟检测一次
			tim3CNT_Zigbee=0;
			if(Debug_enable_flag!=1)//debug模式不自动检查
			{
				zigbee_Refesh_OnlineDeviceNUM();//定时检测zigbee网络终端在线状态
				page1();
			}
		}
	}

	
	
}
#endif
/**
 * @brief  Clear_Buff 	//清空buff
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月10日
*/
void Clear_Buff(char *Buff,uint16_t Len,uint16_t Buff_Max)
{
	for(uint16_t i=0;i<=Val_MAX_Limit_U(Len+2,Buff_Max);i++)
	{
		Buff[i]=0;
	}
	
}
/**
 * @brief  字符串匹配  （这个函数本来不应该算硬件抽象层，但是常用，于是放到抽象层）
 * @param	 char* input ,uint8_t input_Len,char* target,uint8_t target_Len  (如果懒得写长度就都填0，让函数自己计算)
 * @retval uint8_t 1 表示匹配成功 0不成功	（注意这里是 成功为1，方便if语句判断）
 * @author ZCD1300
 * @Time 2021年11月7日
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
 * @brief  最大输入数值限制(无符号)  （这个函数本来不应该算硬件抽象层，但是常用，于是放到抽象层）
 * @param	 uint16_t inputNUM uint16_t MAX_NUM  
 * @retval uint16_t 直接返回范围内的数值
 * @Time 2021年11月8日
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
 * @brief  延时函数  
 * @param	 uint32_t delay_Time
 * @retval void
 * @Time 2021年11月11日
*/
void Delay(uint32_t delay_Time)
{
	HAL_Delay(delay_Time);
}
 /**
 * @brief  Wait_Until_FlagTrue  //等待直到Flag为Ture;注意进入函数 不会 将Flag自动清零，要收到置位
 * @param	 uint8_t *Flag uint32_t Timeout 单位ms,最大可在头文件修改,如果输入0，则不限制超时时间；
 * @retval int8_t return //0超时  1触发成功
 * @Time 2021年11月14日
*/
uint8_t Inv_optim_t=0;

int8_t Wait_Until_FlagTrue(uint8_t *Flag,uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Timeout;
	Inv_optim_t=*Flag;
	if(wait==0)
	{//不设置超时时间
		while(*Flag==0)
		{
			Inv_optim_t=*Flag;			
		}
		return 1;//触发成功
	}
	else
	{
		if (wait < WaitUntilFlagTrue_MAX)
		{
			wait += (uint32_t)(uwTickFreq);
		}
		
		while((HAL_GetTick() - tickstart) < wait)//超时时间之内
		{
			if(*Flag==1)
			{//未超时，触发成功
				return 1;
			}

		}
		//等待超时
		return 0;		
	}
}
/**
 * @brief Uart_Debug_CMD_ADD	//串口Debug新增指令
 * @param void	如果懒得输入CmdLen_t 就填0，让函数自己计数，但是可能存在长度不匹配。
 * @retval void	1表示匹配成功，0表示不成功
 * @author ZCD1300
 * @Time 2021年11月15日
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




