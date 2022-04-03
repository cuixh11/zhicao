 /**
 * @brief  nbiot
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年11月23日
*/
#include "nbiot.h"
#include "ESP8266.h"

uint8_t SmartNest_Domain_NBIOT[]="1,0,\"TCP\",\"smartnestxdu.top\",80,0,1";
uint8_t SmartNest_Domain_NBIOTLEN=sizeof(SmartNest_Domain_NBIOT);
 /**
 * @brief  BC20建立TCP/UDP连接
 * @param	 Type，连接类型 0 UDP    1 TCP
 * @retval	void
 * @author ZCD
 * @Time 2021年11月24日  
*/
void nbiot_build_connect(uint8_t *TargetIP,uint16_t len_t)
{	
	NBIOT_CMD_SEND(NBIOT_OPEN_Socket,TargetIP,sizeof(NBIOT_OPEN_Socket),len_t-1);
}
 /**
 * @brief  BC20断开TCP/UDP连接
 * @param	 uint8_t Connection_Num //连接编号
 * @retval	uint8_t return 0正常返回
 * @author ZCD
 * @Time 2021年11月24日  
*/
uint8_t nbiot_close_connect(uint8_t Connection_Num)
{
	char NUM_temp_t[1]={0};
	uint8_t retey_t=0;
	NUM_temp_t[0]=Connection_Num+48;
	NBIOT_CMD_SEND(NBIOT_CLOSE_Socket,NUM_temp_t,sizeof(NBIOT_CLOSE_Socket),1);
	while(!StringSearch_Assign("CLOSE OK",Rx_Buff4,8,150))
	{
		retey_t++;
		Delay(100);
		if(retey_t>50)
		{
			return 1;//关闭失败
		}
	}
	return 0;
}
 /**
* @brief  BC20发送数据
 * @param	 uint8_t ConnNum //连接编号|uint16_t SendLen//数据长度|char* Send_Buff//数据内容
 * @retval	uint8_t return 0正常返回
 * @author ZCD
 * @Time 2021年11月24日  
*/
uint8_t NBIOT_SEND_PUSH(uint8_t ConnNUM,uint16_t SendLen,char* Send_Buff)
{
	char NBIOT_SEND_BUFF_HEAD_t[10]={0};

	NBIOT_SEND_BUFF_HEAD_t[0]=ConnNUM+48;
	NBIOT_SEND_BUFF_HEAD_t[1]=',';
	if(SendLen<10)
	{
		NBIOT_SEND_BUFF_HEAD_t[2]=SendLen+48;
	}
	else if(SendLen<100)
	{//两位数
		NBIOT_SEND_BUFF_HEAD_t[2]=(SendLen/10)+48;
		NBIOT_SEND_BUFF_HEAD_t[3]=(SendLen%10)+48;

	}
	else if(SendLen<1000)
	{//三位数
		NBIOT_SEND_BUFF_HEAD_t[2]=(SendLen/100)+48;
		NBIOT_SEND_BUFF_HEAD_t[3]=((SendLen%100)/10)+48;
		NBIOT_SEND_BUFF_HEAD_t[4]=(SendLen%10)+48;		
	}		
	else
	{
		return 1;//ERROR
	}
	//strcpy(NBIOT_SEND_BUFF_t+ContentBuff_Str,Send_Buff);
	NBIOT_CMD_SEND(NBIOT_SEND,NBIOT_SEND_BUFF_HEAD_t,sizeof(NBIOT_SEND),strlen(NBIOT_SEND_BUFF_HEAD_t));
	
	UART4_Refresh_Flag=0;//检测串口触发前要置位		
	Wait_Until_FlagTrue(&UART4_Refresh_Flag,5000);//5s超时时间
	NBIOT_DATA_SEND(NBIOT_SEND,SendLen);
	return 0;
}

/**
 * @brief  NBIOT Init//
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月24日
*/
uint8_t nbiotRST_OK_Flag=0;
uint8_t NBIOT_Connected_to_Server_Flag=0;
void NBIOT_Init(void)
{
	{//跳过RST和连接检查,Debug专用,去掉注释的函数就跳过NBIOT初始化过程
		//Skip_NBIOTRST_Check();
		if(GPIO_Read(GPIOF,GPIO_PIN_1)==RESET)
		{//key2 按下，直接跳过NBIOT_Init
			nbiotRST_OK_Flag=1;		
			DebugPrint_uart("||----NBIOT module initialization completed----||\r\n\r\n",0x200);//模块启动完成	
			return ;//跳过RST检查
		}
	}
	uint8_t NBIOT_RST_wait_cnt=0;
	NBIOT_RST_Retry:
	if(nbiotRST_OK_Flag==0)//是否需要RST
	{
		
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);	
		Delay(1000);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);	//这个辣鸡模块RST默认高有效			
		Delay(1000);
		NBIOT_AT_Test();
		while(!(StringSearch_Assign("+IP:",Rx_Buff4,4,100)||StringSearch_Assign("READY",Rx_Buff4,5,100)))
		{	
			{//跳出检测循环
				if(GPIO_Read(GPIOF,GPIO_PIN_1)==RESET)
				{//key2 按下，直接跳过NBIOT_Init
					nbiotRST_OK_Flag=1;		
					DebugPrint_uart("||----NBIOT module initialization completed----||\r\n\r\n",0x200);//模块启动完成	
					return ;//跳过RST检查
				}			
			}			
			NBIOT_RST_wait_cnt++;
			Delay(1000);
			if((NBIOT_RST_wait_cnt%2)==1)
			{
				DebugPrint_uart("** Wait for NBIOT module RST.\r\n",0x200);		
			}
			if(NBIOT_RST_wait_cnt>=20)
			{
				NBIOT_RST_wait_cnt=0;
				DebugPrint_uart("** RST retry.\r\n",0x200);
				goto NBIOT_RST_Retry;
			}

		}
		nbiotRST_OK_Flag=1;		
		DebugPrint_uart("** NBIOT module RST OK.\r\n",0x200);			
	}
	//NBIOT_CMD_SEND(NBIOT_ATBACK0,0,sizeof(NBIOT_ATBACK0),0);//关闭回显
	{//RST完成,GOTIP,建立连接
		uint8_t NBIOT_Connected_To_Server_retry_Cnt=0;
		uint8_t NBIOT_Connected_To_ServerFailRetry_Cnt=0;
		retry_build_NBIOT_connevtion:
		Delay(500);		
		nbiot_build_connect(SmartNest_Domain_NBIOT,SmartNest_Domain_NBIOTLEN);
		Delay(500);
		uint8_t NBIOT_Connected_To_Server_Print_Cnt=0;
		while(!StringSearch_Assign("+QIOPEN: 0,0",Rx_Buff4,12,150))//等待建立到服务器连接完成
		{//未建立连接
			{//跳出检测循环
				if(GPIO_Read(GPIOF,GPIO_PIN_1)==RESET)
				{//key2 按下，直接跳过NBIOT_Init
					nbiotRST_OK_Flag=1;		
					DebugPrint_uart("||----NBIOT module initialization completed----||\r\n\r\n",0x200);//模块启动完成	
					return ;//跳过RST检查
				}			
			}	
			Delay(100);
			NBIOT_Connected_To_Server_Print_Cnt++;
			NBIOT_Connected_To_Server_retry_Cnt++;
			if((NBIOT_Connected_To_Server_Print_Cnt/10)==1)
			{
				NBIOT_Connected_To_Server_Print_Cnt=0;
				DebugPrint_uart("** Connecting to server.\r\n",0x200);
			}
			if(NBIOT_Connected_To_Server_retry_Cnt>=21)
			{//连接失败
				NBIOT_Connected_To_Server_retry_Cnt=0;
				nbiot_close_connect(0);
				if(NBIOT_Connected_To_ServerFailRetry_Cnt<5)
				{
					NBIOT_Connected_To_ServerFailRetry_Cnt++;
					goto retry_build_NBIOT_connevtion;
				}
				
				break;
			}
			
		}		
		DebugPrint_uart("** Connected to server.\r\n",0x200);
		NBIOT_Connected_to_Server_Flag=1;
		NBIOT_TCPLink_Alive=1;		
	}
	
	uint8_t NBIOT_retry_connect_CNT=0;
	char cha2[]="GET /api/test/time HTTP/1.1\r\nHost: smartnestxdu.top\r\n\r\n\r\n";
	NBIOT_retry_connect_test:
	NBIOT_SEND_PUSH(0,sizeof(cha2)-1,cha2);
	Delay(1000);
	if(NBIOT_Received_Flag==1)
	{
		DebugPrint_uart_LineFeed("** NBIOT connection test OK",200);
	}
	else
	{	
		if(NBIOT_retry_connect_CNT<5)
		{
			NBIOT_retry_connect_CNT++;
			goto NBIOT_retry_connect_test;
		}
		
	}
	
	NBIOT_Received_Flag=0;
	DebugPrint_uart("||----NBIOT module initialization completed----||\r\n\r\n",0x200);//模块启动完成	
		
}




