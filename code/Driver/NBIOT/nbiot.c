 /**
 * @brief  nbiot
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��11��23��
*/
#include "nbiot.h"
#include "ESP8266.h"

uint8_t SmartNest_Domain_NBIOT[]="1,0,\"TCP\",\"smartnestxdu.top\",80,0,1";
uint8_t SmartNest_Domain_NBIOTLEN=sizeof(SmartNest_Domain_NBIOT);
 /**
 * @brief  BC20����TCP/UDP����
 * @param	 Type���������� 0 UDP    1 TCP
 * @retval	void
 * @author ZCD
 * @Time 2021��11��24��  
*/
void nbiot_build_connect(uint8_t *TargetIP,uint16_t len_t)
{	
	NBIOT_CMD_SEND(NBIOT_OPEN_Socket,TargetIP,sizeof(NBIOT_OPEN_Socket),len_t-1);
}
 /**
 * @brief  BC20�Ͽ�TCP/UDP����
 * @param	 uint8_t Connection_Num //���ӱ��
 * @retval	uint8_t return 0��������
 * @author ZCD
 * @Time 2021��11��24��  
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
			return 1;//�ر�ʧ��
		}
	}
	return 0;
}
 /**
* @brief  BC20��������
 * @param	 uint8_t ConnNum //���ӱ��|uint16_t SendLen//���ݳ���|char* Send_Buff//��������
 * @retval	uint8_t return 0��������
 * @author ZCD
 * @Time 2021��11��24��  
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
	{//��λ��
		NBIOT_SEND_BUFF_HEAD_t[2]=(SendLen/10)+48;
		NBIOT_SEND_BUFF_HEAD_t[3]=(SendLen%10)+48;

	}
	else if(SendLen<1000)
	{//��λ��
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
	
	UART4_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ		
	Wait_Until_FlagTrue(&UART4_Refresh_Flag,5000);//5s��ʱʱ��
	NBIOT_DATA_SEND(NBIOT_SEND,SendLen);
	return 0;
}

/**
 * @brief  NBIOT Init//
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��24��
*/
uint8_t nbiotRST_OK_Flag=0;
uint8_t NBIOT_Connected_to_Server_Flag=0;
void NBIOT_Init(void)
{
	{//����RST�����Ӽ��,Debugר��,ȥ��ע�͵ĺ���������NBIOT��ʼ������
		//Skip_NBIOTRST_Check();
		if(GPIO_Read(GPIOF,GPIO_PIN_1)==RESET)
		{//key2 ���£�ֱ������NBIOT_Init
			nbiotRST_OK_Flag=1;		
			DebugPrint_uart("||----NBIOT module initialization completed----||\r\n\r\n",0x200);//ģ���������	
			return ;//����RST���
		}
	}
	uint8_t NBIOT_RST_wait_cnt=0;
	NBIOT_RST_Retry:
	if(nbiotRST_OK_Flag==0)//�Ƿ���ҪRST
	{
		
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);	
		Delay(1000);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);	//�������ģ��RSTĬ�ϸ���Ч			
		Delay(1000);
		NBIOT_AT_Test();
		while(!(StringSearch_Assign("+IP:",Rx_Buff4,4,100)||StringSearch_Assign("READY",Rx_Buff4,5,100)))
		{	
			{//�������ѭ��
				if(GPIO_Read(GPIOF,GPIO_PIN_1)==RESET)
				{//key2 ���£�ֱ������NBIOT_Init
					nbiotRST_OK_Flag=1;		
					DebugPrint_uart("||----NBIOT module initialization completed----||\r\n\r\n",0x200);//ģ���������	
					return ;//����RST���
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
	//NBIOT_CMD_SEND(NBIOT_ATBACK0,0,sizeof(NBIOT_ATBACK0),0);//�رջ���
	{//RST���,GOTIP,��������
		uint8_t NBIOT_Connected_To_Server_retry_Cnt=0;
		uint8_t NBIOT_Connected_To_ServerFailRetry_Cnt=0;
		retry_build_NBIOT_connevtion:
		Delay(500);		
		nbiot_build_connect(SmartNest_Domain_NBIOT,SmartNest_Domain_NBIOTLEN);
		Delay(500);
		uint8_t NBIOT_Connected_To_Server_Print_Cnt=0;
		while(!StringSearch_Assign("+QIOPEN: 0,0",Rx_Buff4,12,150))//�ȴ��������������������
		{//δ��������
			{//�������ѭ��
				if(GPIO_Read(GPIOF,GPIO_PIN_1)==RESET)
				{//key2 ���£�ֱ������NBIOT_Init
					nbiotRST_OK_Flag=1;		
					DebugPrint_uart("||----NBIOT module initialization completed----||\r\n\r\n",0x200);//ģ���������	
					return ;//����RST���
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
			{//����ʧ��
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
	DebugPrint_uart("||----NBIOT module initialization completed----||\r\n\r\n",0x200);//ģ���������	
		
}




