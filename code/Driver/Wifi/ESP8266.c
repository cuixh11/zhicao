 /**
 * @brief  ESP8266����
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��1��24��
*/

#include "ESP8266.h"

#include "wifi.h"
#include "http_json.h"

uint8_t Wifi_module_State=0;
 /**
 * @brief  ESP8266���ͺ���
 * @param	ָ��1��ָ��2��ָ��1���ȣ�ָ��2���ȣ�ʵ�ʷ���Ϊָ��1+ָ��2��ָ��2Ϊ0��ֻ����ָ��1��
 * @retval	void
 * @author ZCD
 * @Time 2021��1��25��
*/
void Wifi_CMD_SEND(char *cmd,char* cmd2,uint8_t LenTemp,uint8_t LenTemp2)
{
	if(cmd2!=0)
	{
		HAL_UART_Transmit(&huart1,cmd,LenTemp-1,0x100);//sizeof����ĳ��Ȱ�����\0��ռλ��Ҫ����
		HAL_UART_Transmit(&huart1,cmd2,LenTemp2,0x100);	
		HAL_UART_Transmit(&huart1,"\r\n",2,0x100);	
	}
	else
	{
		HAL_UART_Transmit(&huart1,cmd,LenTemp-1,0x100);//sizeof����ĳ��Ȱ�����\0��ռλ��Ҫ����
		HAL_UART_Transmit(&huart1,"\r\n",2,0x100);		
	}
}
 /**
 * @brief  ESP8266��λ����
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��1��25��
*/
void Wifi_RST(void)//��ʼ��
{
	Wifi_CMD_SEND(WifiRST,0,sizeof(WifiRST),2);
}
 /**
 * @brief  ESP8266����ģ�鹤��ģʽ����
 * @param	 mode = 1-sta  2-ap  3-sta+ap
 * @retval	void
 * @author ZCD
 * @Time 2021��1��25��
*/
 /**
 * @brief  ESP8266 ���ù���ģʽ
 * @param	 uint8_t mode 1 sta/2 ap/3 sta+ap   
 * @retval	uint8_t return  0��ʾ�л��ɹ�
 * @author ZCD
 * @Time 2021��11��19��  
*/
uint8_t Wifi_ModeSet(uint8_t mode)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//δ�˳�͸��
		return 1;
	}
	if(mode==1)
	{
		Wifi_CMD_SEND(WifiMODE,"1",sizeof(WifiMODE),1);	
	}
	else if(mode==2)
	{
		Wifi_CMD_SEND(WifiMODE,"2",sizeof(WifiMODE),1);		
	}
	else if(mode==3)
	{
		Wifi_CMD_SEND(WifiMODE,"3",sizeof(WifiMODE),1);		
	}
	else//�����������
	{	return 2; }
	UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
	if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//���ʱʱ��5s
	{
		if(StringSearch("\r\nOK",4,100))
		{//ģʽ�л����	
			Wifi_Read_WorkState();//ˢ��״̬
			return 0;
		}
		else
		{//�ƺ�û����Ӧ���߷������ݱ�ˢ�µ�
			return 3;
		}
	}
	else
	{
		return 6;//ģ�鳬ʱδ��Ӧ
	}
}
 /**
 * @brief  ESP8266����·��������,���ֶ�ѡ��
 * @param	 SSID��password
 * @retval	void
 * @author ZCD
 * @Time 2021��11��11��  
*/
void Wifi_Connect_AP(char* SSID,char* PassWord)
{
	uint8_t SSID_LEN=0;
	uint8_t Password_LEN=0;
	SSID_LEN=strlen(SSID)-1;
	Password_LEN=strlen(PassWord)-1;
	char Send_temp[100]={0};
	Send_temp[0]='\"';
	for(uint8_t i=0;i<SSID_LEN;i++)
	{
		Send_temp[i+1]=SSID[i];
	}
	Send_temp[SSID_LEN+1]='\"';
	Send_temp[SSID_LEN+2]=',';
	Send_temp[SSID_LEN+3]='\"';

	for(uint8_t i=0;i<Password_LEN;i++)
	{
		Send_temp[i+4+SSID_LEN]=PassWord[i];
	}	
	Send_temp[SSID_LEN+Password_LEN+4]='\"';

	Wifi_CMD_SEND(WifiJoinAP,Send_temp,sizeof(WifiJoinAP),strlen(Send_temp));
}
 /**
 * @brief  ESP8266����·�����������̶����ӵ�ָ��AP,����������
 * @param	 SSID������
 * @retval	void
 * @author ZCD
 * @Time 2021��3��28��  
*/
void Wifi_Connect_AP2(void)
{
	uint8_t temp_Connect[]="\"404 NOT FOUND\",\"xd@404@12138\"";
	//uint8_t temp_Connect[]="\"zcd123456\",\"121380000\"";
		
	Wifi_CMD_SEND(WifiJoinAP,temp_Connect,sizeof(WifiJoinAP),sizeof(temp_Connect)-1);
}
 /**
 * @brief  ESP8266������ģʽ�л�
 * @param	 
 * @retval	void
 * @author ZCD1300
 * @Time 2021��12��24��  
*/
uint8_t Connection_List[9]={0};//���������ӱ�,�ֱ��Ӧ����0~9�����ݣ�0-�����ӣ�1-UDP���ӣ�2-TCP����
uint8_t CIPMUX_Status=0;
uint8_t Wifi_CIPMUX_Ctrl(uint8_t MUXMode_t)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//δ�˳�͸��
		return 1;
	}	
	{//��ȡ��ǰMUXʹ��״̬
		Wifi_CMD_SEND(WifiMUXCheck,0,sizeof(WifiMUXCheck),2);
		Clear_Buff(Rx_Buff1,150,BuffMAX);//���Rx_buff1һ������
		UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ			
		if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//��ʱʱ��5s
		{
			if(StringSearch("+CIPMUX:",8,150))
			{		
				if(Rx_Buff1[StringSecanPointer+8]>=48)
				{
					CIPMUX_Status = Rx_Buff1[StringSecanPointer+8]-48;
				}
			}
			else
			{
				return 3;//��������
			}
		}
		else
		{
			return 2;//ģ�鷵�ؽ��ճ�ʱ
		}
		
		if(CIPMUX_Status==MUXMode_t)
		{//��ǰ������״̬ͬ��
			return 4;
		}	
	}
	
	if(MUXMode_t!=0)
	{//ʹ�ܶ�����
		Wifi_CMD_SEND(WifiMUX,"1",sizeof(WifiMUX),1);
		Clear_Buff(Rx_Buff1,150,BuffMAX);//���Rx_buff1һ������
		UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ	
		Wait_Until_FlagTrue(&UART1_Refresh_Flag,3000);
		if(	StringSearch("\r\nOK",4,150))
		{
			return 0;//��������
		}
		
	}
	else
	{//�رն�����
		Wifi_CMD_SEND(WifiMUX,"0",sizeof(WifiMUX),1);
		Clear_Buff(Rx_Buff1,150,BuffMAX);//���Rx_buff1һ������
		UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ			
		Wait_Until_FlagTrue(&UART1_Refresh_Flag,3000);
		if(	StringSearch("\r\nOK",4,150))
		{
			return 0;//��������
		}
	}
	return 1;//�л�ʧ��

}

 /**
 * @brief  ESP8266 TCP Server Ctrl
 * @param	 
 * @retval	void
 * @author ZCD1300
 * @Time 2021��12��24��  
*/
uint8_t Wifi_TCPServer_Ctrl(uint8_t ServerState_t)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//δ�˳�͸��
		return 1;
	}		
	if(ServerState_t!=0)
	{//��������
		Wifi_CMD_SEND(WifiTCPServer,"1,80",sizeof(WifiTCPServer),4);
		Clear_Buff(Rx_Buff1,150,BuffMAX);//���Rx_buff1һ������
		UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ			
		if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//��ʱʱ��5s
		{
			if(StringSearch("\r\nOK",4,150))
			{//�л����
				return 0;
			}
		}			
	}
	else
	{//�ط�����
		Wifi_CMD_SEND(WifiTCPServer,"0",sizeof(WifiTCPServer),1);
		Clear_Buff(Rx_Buff1,150,BuffMAX);//���Rx_buff1һ������
		UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ			
		if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//��ʱʱ��5s
		{
			if(StringSearch("\r\nOK",4,150))
			{//�л����
				return 0;
			}			
		}			
	}
}

/**
 * @brief  �����ض��ַ�����������10�������ַ��� (ֻ�ܼ���RX_buff1������)
 * @param	 ���������ַ����ַ�ƥ�䳤�ȣ�<=20�����������
 * @retval	������� 1-��  0-��
 * @author ZCD
 * @Time 2021��1��26��
*/
uint16_t StringSecanPointer=0;
uint8_t StringSearch(char *cha,uint8_t Len/*Len<=10*/,uint16_t Deep)
{
	uint16_t CntMax=Deep;
	uint8_t CheckBuff[20]={0};
	
	for(uint16_t i=0;i<CntMax;i++)
	{
			uint8_t flag=1;
		CheckBuff[0]=Rx_Buff1[i];
		CheckBuff[1]=Rx_Buff1[i+1];
		CheckBuff[2]=Rx_Buff1[i+2];
		CheckBuff[3]=Rx_Buff1[i+3];
		CheckBuff[4]=Rx_Buff1[i+4];
		CheckBuff[5]=Rx_Buff1[i+5];
		CheckBuff[6]=Rx_Buff1[i+6];
		CheckBuff[7]=Rx_Buff1[i+7];
		CheckBuff[8]=Rx_Buff1[i+8];
		CheckBuff[9]=Rx_Buff1[i+9];		

		CheckBuff[10]=Rx_Buff1[i+10];
		CheckBuff[11]=Rx_Buff1[i+11];
		CheckBuff[12]=Rx_Buff1[i+12];
		CheckBuff[13]=Rx_Buff1[i+13];
		CheckBuff[14]=Rx_Buff1[i+14];
		CheckBuff[15]=Rx_Buff1[i+15];
		CheckBuff[16]=Rx_Buff1[i+16];
		CheckBuff[17]=Rx_Buff1[i+17];
		CheckBuff[18]=Rx_Buff1[i+18];
		CheckBuff[19]=Rx_Buff1[i+19];				
		for(uint8_t j=0;j<Len;j++)
		{
			if(CheckBuff[j]!=cha[j])
			{
				flag=0;
			}
		}
		if(flag==1)
		{
			StringSecanPointer=i;
			return 1;
		}
	}

	return 0;
}

 /**
 * @brief  �����ض��ַ�����������10�������ַ�������ָ������������ 
 * @param	 ���������ַ���ָ���ļ�����,�ַ�ƥ�䳤�ȣ�<=20�����������
 * @retval	������� 1-��  0-��
 * @author ZCD
 * @Time 2021��11��13��
*/
uint16_t StringSecanPointer_Assign_Buff=0;
uint8_t StringSearch_Assign(char *cha,uint8_t *Assign_Buff,uint8_t Len/*Len<=10*/,uint16_t Deep)
{
	uint16_t CntMax=Deep;
	uint8_t CheckBuff[20]={0};
	
	for(uint16_t i=0;i<CntMax;i++)
	{
			uint8_t flag=1;
		CheckBuff[0]=Assign_Buff[i];
		CheckBuff[1]=Assign_Buff[i+1];
		CheckBuff[2]=Assign_Buff[i+2];
		CheckBuff[3]=Assign_Buff[i+3];
		CheckBuff[4]=Assign_Buff[i+4];
		CheckBuff[5]=Assign_Buff[i+5];
		CheckBuff[6]=Assign_Buff[i+6];
		CheckBuff[7]=Assign_Buff[i+7];
		CheckBuff[8]=Assign_Buff[i+8];
		CheckBuff[9]=Assign_Buff[i+9];
		
		CheckBuff[10]=Assign_Buff[i+10];
		CheckBuff[11]=Assign_Buff[i+11];
		CheckBuff[12]=Assign_Buff[i+12];
		CheckBuff[13]=Assign_Buff[i+13];
		CheckBuff[14]=Assign_Buff[i+14];
		CheckBuff[15]=Assign_Buff[i+15];
		CheckBuff[16]=Assign_Buff[i+16];
		CheckBuff[17]=Assign_Buff[i+17];
		CheckBuff[18]=Assign_Buff[i+18];
		CheckBuff[19]=Assign_Buff[i+19];			
		for(uint8_t j=0;j<Len;j++)
		{
			if(CheckBuff[j]!=cha[j])
			{
				flag=0;
			}
		}
		if(flag==1)
		{
			StringSecanPointer_Assign_Buff=i;
			return 1;
		}
	}

	return 0;
}

/**
 * @brief Read Module Work State	//��ȡģ�鵱ǰ������ʽ
 * @param	void
 * @retval uint8 return   �ɹ�����0��ʧ�ܷ���1
 * @author ZCD1300
 * @Time 2021��11��15��
*/
uint8_t Wifi_Read_WorkState(void)
{
	Wifi_CMD_SEND(WifiMODECheck,0,sizeof(WifiMODECheck),2);
	UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//���ʱʱ��5s
	if(StringSearch("\r\nOK",4,64))
	{
		if(StringSearch("+CWMODE:",8,64))
		{
			Wifi_module_State=Rx_Buff1[StringSecanPointer+8]-48;
			return 0;
		}
		else
		{return 1;}
	}
	else
	{return 1;}
}
/**
 * @brief Read Module IP&MAC	//��ȡģ��IP��MAC��ַ�����AP����ͬʱ��ȡAP����
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��15��
*/
char wifi_AP_GETWAY[16]={0};
char Wifi_STAIP[16]={0};
char Wifi_MAC[12]={0};
uint8_t Wifi_Read_IP_MAC(void)
{
	if((Wifi_module_State==1)||(Wifi_module_State==3))//���������������ֻ����STAģʽ����ʱ���ܶ�ȡMAC
	{
		Wifi_CMD_SEND(WifiIP,0,sizeof(WifiIP),2);	
		UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
		Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//���ʱʱ��5s	
		if(StringSearch("STAIP,\"",7,100))
		{
			StringSecanPointer+=7;
			uint8_t IP_Len_t =0;	
			for(uint8_t i=0;i<18;i++)
			{
				if(Rx_Buff1[StringSecanPointer+i]=='\"')
				{
					IP_Len_t=i;
				}
			}
			for(uint8_t j=0;j<IP_Len_t;j++)
			{
				Wifi_STAIP[j]=Rx_Buff1[StringSecanPointer+j];
			}
			
		}
		if(StringSearch("STAMAC,\"",8,200))
		{
			StringSecanPointer+=8;
			Wifi_MAC[0]=Rx_Buff1[StringSecanPointer];
			Wifi_MAC[1]=Rx_Buff1[StringSecanPointer+1];
			
			Wifi_MAC[2]=Rx_Buff1[StringSecanPointer+3];
			Wifi_MAC[3]=Rx_Buff1[StringSecanPointer+4];
			
			Wifi_MAC[4]=Rx_Buff1[StringSecanPointer+6];
			Wifi_MAC[5]=Rx_Buff1[StringSecanPointer+7];
			
			Wifi_MAC[6]=Rx_Buff1[StringSecanPointer+9];
			Wifi_MAC[7]=Rx_Buff1[StringSecanPointer+10];			
			
			Wifi_MAC[8]=Rx_Buff1[StringSecanPointer+12];
			Wifi_MAC[9]=Rx_Buff1[StringSecanPointer+13];
			
			Wifi_MAC[10]=Rx_Buff1[StringSecanPointer+15];
			Wifi_MAC[11]=Rx_Buff1[StringSecanPointer+16];			
		}
		if(StringSearch("APIP,\"",6,100))
		{
			StringSecanPointer+=6;
			uint8_t AP_IP_Len_t =0;	
			for(uint8_t i=0;i<18;i++)
			{
				if(Rx_Buff1[StringSecanPointer+i]=='\"')
				{
					AP_IP_Len_t=i;
				}
			}
			for(uint8_t j=0;j<AP_IP_Len_t;j++)
			{
				wifi_AP_GETWAY[j]=Rx_Buff1[StringSecanPointer+j];
			}
			
		}
	}
	//Wifi_CMD_SEND(WifiIP,0,sizeof(WifiIP),0);
}
/**
 * @brief Exit module transparent send//�˳�͸��ģʽ
 * @param	void
 * @retval uint8_t return 1��ʾ�˳�͸���ɹ���0��ʾʧ��
 * @author ZCD1300
 * @Time 2021��11��18��
*/
uint8_t Wifi_ExitTransp_Send(void)
{
	uint8_t Exit_buff[3];

	uint8_t Error_cnt_t=0;
	
	ExitTransp_Send_Lable:
	Exit_buff[0]='+';
	Exit_buff[1]='+';
	Exit_buff[2]='+';	
	UART_Send(&huart1,Exit_buff,3,0x200);
	Delay(20+(10*Error_cnt_t));
	if(!Wifi_Read_WorkState())
	{//�ɹ���ȡ����˵���˳���͸��ģʽ
		Wifi_Transparent_Send_Flag=0;
		Wifi_Transparent_SendEntered_Flag=0;
		WifiInitOKflag=0;
		return 1;
	}
	else
	{//�˳�͸��ʧ��,Ӧ�÷������ԣ����������3�Σ�
		if(Error_cnt_t<3)
		{
			Error_cnt_t++;		
			goto ExitTransp_Send_Lable;
		}
		else
		{//����10��ʧ��,��Ϊģ�����ߣ��˳�͸��
			Error_cnt_t=0;
			Wifi_Transparent_Send_Flag=0;
			Wifi_Transparent_SendEntered_Flag=0;
			return 0;
		}
	}
}
/**
 * @brief Close connection//�ر����ӣ�Ŀǰδ��ɹر�ָ�����ӣ�ֻ��ȫ���رգ�
 * @param	char* Connection_ID ָ���رյ�ID���ַ������������������ֵ0������Ϊȫ���ر�
 * @retval uint8_t return ��ע��
 * @author ZCD1300 
 * @Time 2021��11��18��
*/
uint8_t Close_Connection_Return=0;
uint8_t Wifi_CLOSE_Connection(char* Connect_ID)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//δ�˳�͸��
		Close_Connection_Return=1;
		return 1;
	}
	if(Connect_ID==0)//�ر�ȫ������
	{
		Wifi_CMD_SEND(WifiConnectCloseALL,0,sizeof(WifiConnectCloseALL),2);
		
		UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
		if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//���ʱʱ��5s
		{
			if(StringSearch("CLOSED\r\n\r\nOK",12,200))
			{//�رճɹ�,����0
				Wifi_Transparent_SendEntered_Flag=0;
				Close_Connection_Return=0;
				return 0;
			}
			else if(StringSearch("ERROR",5,200))
			{
				Wifi_Transparent_SendEntered_Flag=0;
				Close_Connection_Return=3;
				return 3;//�Ѿ�û��������
			}
			else
			{//ģ��δ֪����
				Close_Connection_Return=4;
				return 4;
			}
		}
		Wifi_Transparent_SendEntered_Flag=0;
		Close_Connection_Return=0;		
		return 6;//ģ�鳬ʱδ��Ӧ
	}
	else//�ر�ָ�����ӣ���δ���
	{
		Close_Connection_Return=2;
		return 2;//����2��ʾ����δ���
	}
	Close_Connection_Return=5;
	return 5;//�����������
}
/**
 * @brief Check whether a TCP connection is established //����Ƿ�����TCP����
 * @param	void 
 * @retval int8_t return  1 ��ʾ����TCP����   0��ʾ���������� �������ؼ�ע��
 * @author ZCD1300
 * @Time 2021��11��21��
*/
uint8_t TCPLink_Alive=0;
int8_t Check_TCP_Connection(void)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//δ�˳�͸��
		Wifi_ExitTransp_Send();//�˳�͸��
		//return 2;
	}
	{//�������
		Wifi_CMD_SEND(WifiConnectList,0,sizeof(WifiConnectList),2);
		
		UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ		
		if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//���ʱʱ��5s
		{
			if(StringSearch("\"TCP\",\"",7,100))
			{//����TCP����
				for(uint8_t i=0;i<7;i++)
				{
					Rx_Buff1[StringSecanPointer+i]=0;
				}
				
				TCPLink_Alive=1;
				LED_ctrl(1,1);
				return 1;
			}
			else
			{//������TCP����
				TCPLink_Alive=0;
				Connected_to_Server_Flag=0;
				LED_ctrl(1,0);
				return 0;
			}
		}
		TCPLink_Alive=0;
		Connected_to_Server_Flag=0;	
		LED_ctrl(1,0);		
		return 6;//ģ�鳬ʱδ��Ӧ
	}
}
/**
 * @brief Check AP name//��ȡAP����
 * @param	void 
 * @retval int8_t return  0û��AP���� 1��AP����
 * @author ZCD1300
 * @Time 2021��11��21��
*/
char AP_Name[20]={0};
uint8_t Wifi_Check_AP_Name(void)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//δ�˳�͸��
		if(Wifi_ExitTransp_Send())
		{
			//�˳�͸��
		}
		else
		{return 2;}
	}
	uint8_t str_start_t=0;
	uint8_t str_end_t=0;
	Wifi_CMD_SEND(WifiJoinAPSuces,0,sizeof(WifiJoinAPSuces),2);	
	UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
	if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//���ʱʱ��5s
	{
		if(StringSearch("No AP",5,200))
		{//û��AP����
			DebugPrint_uart("** No AP linked.",200);			
			return 0;
		
		}
		else if(StringSearch("+CWJAP:\"",8,200))
		{//��AP����
			str_start_t=StringSecanPointer+8;
			StringSearch("\",\"",3,200);
			str_end_t=StringSecanPointer;
			for(uint8_t i=0;i<(str_end_t-str_start_t);i++)
			{
				AP_Name[i]=Rx_Buff1[str_start_t+i];
			}
			
			DebugPrint_uart("** AP name: ",200);
			DebugPrint_uart(AP_Name,200);
			DebugPrint_uart(" \r\n",200);
			return 1;
		}
		else
		{//δ֪����
			return 3;
		}		
	}
	else//ģ�鳬ʱ
	{return 6;}
}
/**
 * @brief Read sta gateway and Subnet mask
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021��11��22��
*/
char wifi_GATEWAY[16]={0};
char wifi_MASK[16]={0};
uint8_t Wifi_Read_Getway_Mask(void)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//δ�˳�͸��
		if(Wifi_ExitTransp_Send())
		{
			//�˳�͸��
		}
		else
		{return 2;}
	}
	Wifi_CMD_SEND(WifiNETSTA,0,sizeof(WifiNETSTA),2);	
	UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//���ʱʱ��5s	
	if(StringSearch("gateway:\"",9,100))
	{
			StringSecanPointer+=9;
			uint8_t Gateway_Len_t =0;	
			for(uint8_t i=0;i<18;i++)
			{
				if(Rx_Buff1[StringSecanPointer+i]=='\"')
				{
					Gateway_Len_t=i;
				}
			}
			for(uint8_t j=0;j<Gateway_Len_t;j++)
			{
				wifi_GATEWAY[j]=Rx_Buff1[StringSecanPointer+j];
			}		
	}		
	if(StringSearch("netmask:\"",9,100))
	{
			StringSecanPointer+=9;
			uint8_t Mask_Len_t =0;	
			for(uint8_t i=0;i<18;i++)
			{
				if(Rx_Buff1[StringSecanPointer+i]=='\"')
				{
					Mask_Len_t=i;
				}
			}
			for(uint8_t j=0;j<Mask_Len_t;j++)
			{
				wifi_MASK[j]=Rx_Buff1[StringSecanPointer+j];
			}		
	}	

}	

/*
//Fr����ʵ��
void Wifi_Control(void const *argument)
{

	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

//-------------------------------------------------------//	
	
	HAL_Delay(1000);
	Wifi_Init();
	HAL_Delay(2000);
	uint8_t ConnectFlag=0;
	uint8_t TCPConFlag=0;
	for(;;)
	{	
		if(ConnectFlag==0)
		{
			ConnectFlag = StringSearch("GOT IP",4);
		}
		if(ConnectFlag==1)
		{
			if(TCPConFlag==0)
			{
				Get_NETTime();
				TCPConFlag=1;		
			}
		}
		
		
//-------------------------------------------------------//

//--------------------------------------------------------------------------------------------------------------------//		
		osDelayUntil(&xLastWakeTime,10/portTICK_RATE_MS);		
  }
}
//���̾��
osThreadId Wifi_ControlHandle;
//���̴�������
void Wifi_ControlThreadCreate(osPriority taskPriority)
{
	osThreadDef(Wifi_ControlThread,Wifi_Control,taskPriority,0,1024);
	Wifi_ControlHandle = osThreadCreate(osThread(Wifi_ControlThread),NULL);
}
*/


