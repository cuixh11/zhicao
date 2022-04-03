#include "host_app.h"

FirmwareINFO_t FirmwareINFO_Local={ .FirmwareType="HOST",\
																		.FirmwareVersion="Y149",\
																		.FirmwareSize=1024,\
																		.Firmware_MD5="abcd567890abcde" };

/**
 * @brief ���������������Ӵ�����������������
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��14��
*/
int8_t Host_SystemStartUp_SmartNest(void)
{
	LCD_Startup_ProgressBar(0);
	json_Space_init();//����������ǰ�����json_t�������ٿռ�
	maintain_Offline_data_BUFF();
	Key_read();
	//-------LCD------
	HMI_Rest();
	Delay(500);
	HMI_Init();
	
	{//----LED Ctrl-----
	LED_ctrl(0,1);
	LED_ctrl(1,1);
	LED_ctrl(2,1);
	Delay(1000);
	LED_ctrl(0,0);
	LED_ctrl(1,0);
	LED_ctrl(2,0);
	}
	//-----------------
	DebugPrint_uart("\r\n||-----------System startup-----------||\r\n\r\n",0x200);
	DebugPrint_uart("   Firmware version 1.4.9\r\n\r\n",0x200);
	
	//-----------------wifiģ������
	DebugPrint_uart("||----Start initializing WiFi module....\r\n\r\n",0x200);
	Wifi_Init();
	LCD_Startup_ProgressBar(40);
	LED_ctrl(0,1);//LED0 on
	
	//-----------------NBIOTģ������
	DebugPrint_uart("||----Start initializing NBIOT module...\r\n\r\n",0x200);	
	NBIOT_Init();
	LCD_Startup_ProgressBar(60);
	LED_ctrl(1,1);//LED1 on
	
	//-----------------�ӻ���������
	DebugPrint_uart("||----Start initializing ZigBee network....\r\n\r\n",0x200);	
	//ZigBeeNET_Init();
	LCD_Startup_ProgressBar(80);
	LED_ctrl(2,1);//LED2 on
	
	Host_test_SmartNest();//������������
	LCD_Startup_ProgressBar(95);	
	
	DebugPrint_uart("\r\n||-----------Boot complete------------||\r\n\r\n",200);	

	LED_ctrl(0,0);
	LED_ctrl(1,0);
	LED_ctrl(2,0);
//	Wifi_ExitTransp_Send();
//	Wifi_CLOSE_Connection(0);

}
/**
 * @brief utoken�Ĺ��ڼ�飬�����Զ����µ�¼����token;�˺���Ӧ������ѭ�����߶�ʱ��ѭ����
 * @param	void
 * @retval int8_t return //0��ʾtoken��Ч�������»�ȡ�ɹ���1��ʾ���µ�¼ʧ�ܣ�Ӧ��ת����¼������
 * @author ZCD1300
 * @Time 2021��11��14��
*/
int8_t Host_utoken_Check_SmartNest(void)
{
	uint8_t cmp_flag=0;
	{//��Ч���ж�
		if(year_token>year)
		{
			cmp_flag=1;
		}
		else if(month_token > month)
		{
			cmp_flag=1;	
		}	
		else if(day_token>day)
		{
			cmp_flag=1;
		}
		else if(hour_token>hour)
		{
			cmp_flag=1;
		}
		else if(min_token>min)
		{
			cmp_flag=1;
		}
		else if(sec_token>sec)
		{
			cmp_flag=1;
		}
		else
		{cmp_flag=0;}
	}
	if(cmp_flag==1)//utoken��Ч
	{
		BodyHead_pro.utoken_Live=1;
		return 0;
	}
	else//utoken����
	{
		//�������
		uint8_t Rebuild_Connection_CNT_T=0;
		uint8_t Retry_EnterTranspSend_CNT_t=0;
		while(1)
		{
			CheckTCP_Return=Check_TCP_Connection();//�������ͬʱ�����tcp���Ӳ��˳�͸��
			if(TCPLink_Alive!=1)
			{//������ӶϿ������½�������
				Wifi_ExitTransp_Send();				
				Wifi_CLOSE_Connection(0);
				Connect_Server_Smatnest();//�������ӵ�������
				if(TCPLink_Alive==1)
				{//�����������ɹ�
					if(Start_Transp_Send()==0)
					{//ʹ�ܲ�����͸��
						break;
					}
					else
					{	
						Retry_EnterTranspSend_CNT_t++;
						if(Retry_EnterTranspSend_CNT_t>10)
						{//ʮ�γ��Խ���͸��ʧ��
							Retry_EnterTranspSend_CNT_t=0;
							return 4;//WiFi����͸��ģʽʧ��
						}
					}
				}
				else
				{//����ʧ��
					Rebuild_Connection_CNT_T++;
					if(Rebuild_Connection_CNT_T>10)//ʮ���������ӵ�������ʧ��
					{
						Rebuild_Connection_CNT_T=0;
						return 3;//���ӵ�������ʧ��
					}
				}
				
			}
			else
			{//����δ�Ͽ�,���½���͸��
				Retry_EnterTranspSend_CNT_t=0;
				while(Start_Transp_Send())
				{
					Retry_EnterTranspSend_CNT_t++;
					if(Retry_EnterTranspSend_CNT_t>10)
					{//ʮ�γ��Խ���͸��ʧ��
						Retry_EnterTranspSend_CNT_t=0;
						return 4;//WiFi����͸��ģʽʧ��
					}					
				}
			}
		}
		
		BodyHead_pro.utoken_Live=0;
		uint8_t retrying_cnt=0;
		DebugPrint_uart("** Utoken invalid. Attempting to log in again.\r\n",0x200);
		while(Login_Uesr(userName.Str,Password.Str)!=0)
		{
			Delay(2000);
			DebugPrint_uart("** Utoken invalid. Attempting to log in again.\r\n",0x200);
			retrying_cnt++;
			if(retrying_cnt>=10)//���µ�¼ʧ��
			{
				retrying_cnt=0;
				DebugPrint_uart("** The attempt to log in again failed.\r\n",0x200);
				
				Registed_User=0;
				Flash_Failure_Mark();//���ﻹҪ���flash��������Ч
				Reg_User_Check();
				return 1;//utoken���ڣ������µ�¼ʧ��
			}
		}
		
		//���µ�¼������utoken�ɹ�
		BodyHead_pro.utoken_Live=1;
		DebugPrint_uart_LineFeed("** The token has been refreshed.",200);
		return 0;		
	}
	//�����������в��ᵽ��
	return 2;//δ֪����	
}
/**
 * @brief �������������������ӵĺ�����Ӧ��������������֮��
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��14��
*/
uint8_t get_timetest_cnt=0;
int8_t Host_test_SmartNest(void)
{
	retry_time_test:
	if(get_timetest_cnt>10)
	{
		MCU_System_RST();
	}
	Get_NETTime();	//��ȡʱ��,�������
	Body_Process();
	if(BodyHead_pro.State_code!=200)
	{
		get_timetest_cnt++;
		goto retry_time_test;	
	}
	Unpack_return=JsonUnpack("test/time");
  
	InputUser_retry:
	Reg_User_Check();
		
	Print_Time(year,month,day,hour,min,sec,weekday);	
	if(Host_utoken_Check_SmartNest()==0)
	{
		DebugPrint_uart("** token alive\r\n",0x200);
		//Print_Time(year_token,month_token,day_token,hour_token,min_token,sec_token,weekday);
	}
	else
	{
		DebugPrint_uart("** Get token invalid\r\n",0x200);
		Registed_User=0;
		Flash_Failure_Mark();//���ﻹҪ���flash��������Ч		
		goto InputUser_retry;
	}
	
}
/**
 * @brief Smart nest Debug APP 
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021��11��18��
*/
uint8_t Debug_enable_flag=0;
uint8_t press_test_Flag=0;
void SmartNest_Debug_APP(void)
{
//����Debugָ��������ã��ƥ��20���ַ���
	if(Debug_enable_flag!=0)
	{
		if(Uart_Debug_CMD_ADD("regHost",Rx_Buff2,0,100))//ע������
		{  Reg_Host_App();	}
		if(Uart_Debug_CMD_ADD("regDevice_test",Rx_Buff2,0,100))//ע��ӻ��������õ�SN
		{	 Register_Device();	}			
		if(Uart_Debug_CMD_ADD("regDevice",Rx_Buff2,0,100))//ע��ӻ���ȫ���Ѿ�����Zigbee�����
		{	 Reg_Device_ALL();	}	
		if(Uart_Debug_CMD_ADD("login",Rx_Buff2,0,100))//�ֶ���¼ 
		{	 Login_Uesr(userName.Str,Password.Str);		}		
		if(Uart_Debug_CMD_ADD("flashR",Rx_Buff2,0,100))//��ȡFlash(ָ������)
		{	 	
			char Read_temp[64];
			Flash_Read(UserInfo_FlashSaveAddr,16,(uint32_t *)Read_temp);	
			DebugPrint_uart("\r\n** >>",200);	
			DebugPrint_uart(Read_temp,200);		
			DebugPrint_uart("\r\n** Flash read over.",200);								
		}	
		if(Uart_Debug_CMD_ADD("flashE",Rx_Buff2,0,100))//����������û���Ϣ
		{	 	
			Flash_Failure_Mark();
			Registed_User=0;	
			DebugPrint_uart_LineFeed("** User information has been erased.",200);	
		}					
		if(Uart_Debug_CMD_ADD("unpackBuff",Rx_Buff2,0,100))//ʹ��json Unpack buff��ʾ����
		{	 Display_unpackBuff_Flag=1;
			 DebugPrint_uart_LineFeed("** Enable display JSON unpack buff.",400);
		}	
		if(Uart_Debug_CMD_ADD("packBuff",Rx_Buff2,0,100))//ʹ��json Pack buff��ʾ����
		{	 Display_packBuff_Flag=1;
			 DebugPrint_uart_LineFeed("** Enable display JSON pack buff.",400);
		}					
		if(Uart_Debug_CMD_ADD("wifiSwitch",Rx_Buff2,0,100))//�л�wifi ap����
		{	
			DebugPrint_uart_LineFeed("** Manually switch AP:",200);
			if(Wifi_ExitTransp_Send())//�˳�͸��
			{
				Switch_Wifi_AP();
				WifiRST_OK_Flag=1;//������λWifiģ��
				Wifi_Connect_AP_Flag=0;//���WiFi���ӱ�־
				GotIP_Flag=0;//���Got IP��־
				Connected_to_Server_Flag=0;
				TCPLink_Alive=0;
				Wifi_Init();
			}
			else
			{
				DebugPrint_uart_LineFeed("** Exit transparent send fail.",200);
			}
		}	
		if(Uart_Debug_CMD_ADD("appWifiSet",Rx_Buff2,0,100))//�л�wifi ap������APP������
		{	
			DebugPrint_uart_LineFeed("** Manually switch AP:",200);
			if(Wifi_ExitTransp_Send())//�˳�͸��
			{
				Distribution_via_app();
			}
			else
			{
				DebugPrint_uart_LineFeed("** Exit transparent send fail.",200);
			}
		}			
		if(Uart_Debug_CMD_ADD("pressTest",Rx_Buff2,0,100))//�ϴ�����ѹ������
		{	 press_test_Flag=1;		}		
		if(Uart_Debug_CMD_ADD("checkAP",Rx_Buff2,0,100))//�ϴ�����ѹ������
		{	 Wifi_Check_AP_Name();	}				
		if(Uart_Debug_CMD_ADD("TCP?",Rx_Buff2,0,100))//��ʾ��ǰTCP����״̬
		{	Check_TCP_Connection();
			if(TCPLink_Alive==1)
			{DebugPrint_uart_LineFeed("** TCP connection alive.",200);}
			else if(TCPLink_Alive==0)
			{DebugPrint_uart_LineFeed("** TCP connection not alive.",200);}
			else
			{DebugPrint_uart_LineFeed("** TCP state error.",200);}
		}				
		if(Uart_Debug_CMD_ADD("closeConnect",Rx_Buff2,0,100))//�ر�ȫ������
		{	 
			Wifi_ExitTransp_Send();
			Wifi_CLOSE_Connection(0);
			DebugPrint_uart_LineFeed("** OK",200);
		}	
		if(Uart_Debug_CMD_ADD("buildConnect",Rx_Buff2,0,100))//�ֶ���������,�Զ�����͸��
		{	 
			Connect_Server_Smatnest();
			DebugPrint_uart_LineFeed("** OK",200);
			Start_Transp_Send();
		}	
		if(Uart_Debug_CMD_ADD("SYSRST",Rx_Buff2,0,100))//RST MCU
		{	 
			 DebugPrint_uart_LineFeed("** System restart.",400);
			 MCU_System_RST();
		}
		else if(Uart_Debug_CMD_ADD("sysrst",Rx_Buff2,0,100))//RST MCU
		{
			 DebugPrint_uart_LineFeed("** System restart.",400);
			 MCU_System_RST();		
		}
		if(Uart_Debug_CMD_ADD("wifiBuff",Rx_Buff2,0,100))//��ӡwifi buff 100
		{	 
			 UART_Send(&huart2,Rx_Buff1,100,400);
		}	
		if(Uart_Debug_CMD_ADD("zgSensor",Rx_Buff2,0,100))//��zigbee����Ĵ���������
		{	 
			 DebugPrint_uart("** Start\r\n",200);
			 ZigbeeRead_ALL_Sensor();
			DebugPrint_uart_LineFeed("** OK",200);
		}		
		if(Uart_Debug_CMD_ADD("zgMAC",Rx_Buff2,0,100))//��zigbee����MAC
		{	 
			DebugPrint_uart("** Start\r\n",200);
			zigbee_Refesh_OnlineDeviceNUM();
			DebugPrint_uart_LineFeed("** OK",200);
		}			
		if(Uart_Debug_CMD_ADD("ServerPUSH",Rx_Buff2,0,100))//ServerPUSH,���������͹��ܲ���
		{	
			PUSH_Serve();
			DebugPrint_uart("** PUSH test.\r\n",200);
		}			
		if(Uart_Debug_CMD_ADD("PUSH",Rx_Buff2,0,100))//�����ݴ����PUSH
		{	 
			Sensor_PUSH_APP(2);
			DebugPrint_uart_LineFeed("** OK",200);
		}	
		if(Uart_Debug_CMD_ADD("POP",Rx_Buff2,0,100))//�����ݴ����POP
		{	 
			Sensor_POP_APP(HistoryDataPOP_BUFF);
			DebugPrint_uart_LineFeed("** OK",200);
		}		
		if(Uart_Debug_CMD_ADD("DestroyStack",Rx_Buff2,0,100))//��������ջ����
		{	 
			StackHeader_NUM=0;
			DebugPrint_uart_LineFeed("** OK",200);
		}	

		if(Uart_Debug_CMD_ADD("WtFirmINFO",Rx_Buff2,0,100))//��д�̼���Ϣ������ָ��
		{	 
			Wirte_firmwareINFO(FirmwareINFO_Local);
			DebugPrint_uart_LineFeed("** OK",200);
		}			
		if(Uart_Debug_CMD_ADD("FirmINFO",Rx_Buff2,0,100))//��ȡ�̼���Ϣ
		{	 
			Update_FirmwareINFO();
			DebugPrint_uart_LineFeed("** OK",200);
		}	

		if(Uart_Debug_CMD_ADD("UploadState",Rx_Buff2,0,100))//UploadState
		{	
			char temp_t[]="** Upload error code: 0\r\n";
			temp_t[22]=UploadDataAPP_Return+48;
			DebugPrint_uart(temp_t,200);
			DebugPrint_uart_LineFeed("** OK",200);
		}	
		if(Uart_Debug_CMD_ADD("stopHMI",Rx_Buff2,0,100))//�رմ�������ʾˢ�£��⴮�������ܻ���Ϊ��Ļ���������������������Խ����ڵ���ʱ�ر�ˢ�£�
		{	 HMI_Display_RefeshDisableFlag=1;
			 DebugPrint_uart_LineFeed("** Disable HMI display refresh.",400);
		}	
		if(Uart_Debug_CMD_ADD("startHMI",Rx_Buff2,0,100))//������������ʾˢ�£��⴮�������ܻ���Ϊ��Ļ���������������������Խ����ڵ���ʱ�ر�ˢ�£�
		{	 HMI_Display_RefeshDisableFlag=0;
			 HMI_Rest();
			 DebugPrint_uart_LineFeed("** Enable HMI display refresh.",400);
		}	
	}		
	//----------------------------------------------------------------//
	if((Uart_Debug_CMD_ADD("Debug_zcd123",Rx_Buff2,0,100))||(KeyState[0]==0))//����Debug
	{	 
		Debug_enable_flag=1;	
		Display_unpackBuff_Flag=0;//���½���Debugģʽ���� ȡ����ʾʹ��
		Display_packBuff_Flag=0;
		press_test_Flag=0;
		DebugPrint_uart("\r\n** Uart Debug Enabled.\r\n",200);
		//DebugPrint_uart_LineFeed("** Uart Debug Enabled.",200);	
	}			
	if(Uart_Debug_CMD_ADD("Debug quit",Rx_Buff2,0,100))//�˳�Debug
	{	 
		Debug_enable_flag=0;	
		DebugPrint_uart_LineFeed("** Uart Debug Disabled.",200);	
	}	
	//----------------------------------------------------------------//			
	{//Debug_APPĬ����������
		if(Upload_pressTest_CNT>50000)
		{
			press_test_Flag=0;
		}
		if(press_test_Flag==1)
		{
			Upload_DeviceData();		
			
		}
	
	}


}
/**
 * @brief ���ڴ�ӡ�з�����ֵ��ʮ������
 * @param	int
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021��11��18��
*/
void UART_Print_Number(int NumtoPrint)
{
	char display_num_t[12]="\r\n** 000000";	
	if(NumtoPrint>0)
	{		
		if(NumtoPrint<10)
		{display_num_t[10]=48+NumtoPrint;	}
		else if(NumtoPrint<100)
		{  
			display_num_t[10]=48+(NumtoPrint%10);	
			display_num_t[9]=48+NumtoPrint/10;	
		}
		else if(NumtoPrint<1000)
		{  
			display_num_t[10]=48+(NumtoPrint%10);	
			display_num_t[9]=48+((NumtoPrint%100)/10);	
			display_num_t[8]=48+(NumtoPrint/100);						
		}
		else if(NumtoPrint<10000)
		{  
			display_num_t[10]=48+(NumtoPrint%10);	
			display_num_t[9]=48+((NumtoPrint%100)/10);
			display_num_t[8]=48+((NumtoPrint%1000)/100);	
			display_num_t[7]=48+(NumtoPrint/1000);						
		}
		else if(NumtoPrint<100000)
		{  
			display_num_t[10]=48+(NumtoPrint%10);	
			display_num_t[9]=48+((NumtoPrint%100)/10);
			display_num_t[8]=48+((NumtoPrint%1000)/100);	
			display_num_t[7]=48+((NumtoPrint%10000)/1000);	
			display_num_t[6]=48+(NumtoPrint/10000);						
		}
	}
	else
	{
		NumtoPrint=NumtoPrint*(-1);
		display_num_t[5]='-';
		if(NumtoPrint<10)
		{display_num_t[10]=48+NumtoPrint;	}
		else if(NumtoPrint<100)
		{  
			display_num_t[10]=48+(NumtoPrint%10);	
			display_num_t[9]=48+NumtoPrint/10;	
		}
		else if(NumtoPrint<1000)
		{  
			display_num_t[10]=48+(NumtoPrint%10);	
			display_num_t[9]=48+((NumtoPrint%100)/10);	
			display_num_t[8]=48+(NumtoPrint/100);						
		}
		else if(NumtoPrint<10000)
		{  
			display_num_t[10]=48+(NumtoPrint%10);	
			display_num_t[9]=48+((NumtoPrint%100)/10);
			display_num_t[8]=48+((NumtoPrint%1000)/100);	
			display_num_t[7]=48+(NumtoPrint/1000);						
		}
		else if(NumtoPrint<100000)
		{  
			display_num_t[10]=48+(NumtoPrint%10);	
			display_num_t[9]=48+((NumtoPrint%100)/10);
			display_num_t[8]=48+((NumtoPrint%1000)/100);	
			display_num_t[7]=48+((NumtoPrint%10000)/1000);	
			display_num_t[6]=48+(NumtoPrint/10000);						
		}	
	
	}
	DebugPrint_uart(display_num_t,400);		
}

/**
 * @brief Time local refesh 
 * @param	uint8_t TimeTick	//ѭ����������ڣ���λ�루s��������������60������ʱ�䲻׼
 * @retval void
 * @author ZCD1300 
 * @Time 2021��11��18��
*/
void Local_time_refresh(uint8_t TimeTick)
{
	for(uint8_t i=0;i<TimeTick;i++)
	{
		if(sec<59)
		{
			sec++;
		}		
		else
		{
			sec=sec-59;
			min++;
		}		
		if(min<=59)
		{
			//min++;
		}
		else
		{
			min=0;
			hour++;
		}
		if(hour<=23)
		{
			//hour++;
		}
		else
		{
			hour=0;
			day++;
		}
		switch (month)
		{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:			
			{//31��
				if(day<=31)
				{
				}
				else
				{
					month++;
				}
				break;
			}
			case 4:
			case 6:
			case 9:
			case 11:
			{//30��
				if(day<=30)
				{
				}
				else
				{
					month++;
				}			
				break;
			}	
			default:
			{//2��
				if(day<=29)//Ĭ�������꣬�������ȴ�����У׼
				{
				}
				else
				{
					month++;
				}				
				break;
			}
		}
		if(month<=12)
		{}
		else
		{year++;}		
	
	}	
}
/**
 * @brief Reg Host and Input host name//ע������ʱ�ȴ�����host name
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��15��
*/
int8_t Reg_Host_App(void)
{
	char hostname_temp[21]={0};
	DebugPrint_uart("\r\n** Name this Host >>",0x200);
	Clear_Buff(Rx_Buff2,100,BuffMAX2);
	UART2_Refresh_Flag=0;
	if(Wait_Until_FlagTrue(&UART2_Refresh_Flag,30000))//���ȴ�30s
	{//ʱ���ڽ��յ�����
		strcpy(hostname_temp,Rx_Buff2);
		DebugPrint_uart(hostname_temp,200);
		if(!Load_host_name(hostname_temp))
		{//SN��host_name���������
			Register_Host();
			return 0;
		}
		DebugPrint_uart_LineFeed("** host name is too long.",200);
		return 2;//����host name����//����״���²��ᵽ����
	}
	else
	{//�ȴ���ʱ
		DebugPrint_uart_LineFeed("** It took too long to exit automatically.",0x200);
		return 1;//���볬ʱ���Զ��˳�
	}
	
}
/**
 * @brief Reg_User_Check//����Ƿ��Ѿ���ע���User�˻���û�еĻ�Ӧ���û�ע�ᡣ��������ʱ������Ƕ��ʽ�ն�ע�ᣬ��ʽ�İ汾Ӧ���ɿͻ���ע�ᣩ
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��16��
*/
uint8_t Registed_User=0;
uint8_t Changed_UserInfo=0;
int8_t Reg_User_Check(void)
{
	Flash_Read_UserInfo();
	if(Registed_User==0)
	{
		if(Changed_UserInfo==0)
		{//û��ע���User account���״ε�¼		
			DebugPrint_uart("\r\n** It looks like you're logging in for the first time.We need your smart nest account to continue.\r\n** Your account >>",400);
		}
		else
		{//�޸Ĺ����룬�Զ����µ�¼ʧ��
			DebugPrint_uart("\r\n** Your password or account has expired. Please log in again.\r\n** Your account >>",400);			
		}
		Clear_Buff(Rx_Buff2,100,BuffMAX2);
		UART2_Refresh_Flag=0;
		if(Wait_Until_FlagTrue(&UART2_Refresh_Flag,0))//�����ó�ʱʱ��
		{//�յ�����
			strcpy(userName.Str,Rx_Buff2);
		}
		DebugPrint_uart(userName.Str,200);
		DebugPrint_uart("\r\n** Your password >>",400);
		Clear_Buff(Rx_Buff2,100,BuffMAX2);
		UART2_Refresh_Flag=0;
		if(Wait_Until_FlagTrue(&UART2_Refresh_Flag,0))//�����ó�ʱʱ��
		{//�յ�����
			strcpy(Password.Str,Rx_Buff2);
		}		
		Login_Uesr(userName.Str,Password.Str);
		return 0;
		
	}
	else
	{
		return Login_Uesr(userName.Str,Password.Str);	
	
	}
}

/**
 * @brief Read username and password from Flash//��ȡflash�е�����
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��16��
*/
	uint8_t Flash_ReadBuff_t[45]={0};
const	uint8_t ExistenceCheck_FlashLen=1;//ǰ4���ֽ���������Ƿ�д���
const	uint8_t UserName_FlashLen=5;//usernameռ�õ�����
const	uint8_t Password_FlashLen=5;//passwordռ�õ�����
int8_t Flash_Read_UserInfo(void)
{

	uint8_t ExistenceCheck_t[4]={0};
	Flash_Read(UserInfo_FlashSaveAddr,1,(uint32_t *)ExistenceCheck_t);
	Delay(100);			
	Flash_Read(UserInfo_FlashSaveAddr,11,(uint32_t *)Flash_ReadBuff_t);	
		


	uint8_t Cnt_t=0;
	if(ExistenceCheck_t[0]==ExistenceCheck1)
	{		Cnt_t++;	}
	if(ExistenceCheck_t[1]==ExistenceCheck2)
	{		Cnt_t++;	}	
	if(ExistenceCheck_t[2]==ExistenceCheck3)
	{		Cnt_t++;	}
	if(ExistenceCheck_t[3]==ExistenceCheck4)
	{		Cnt_t++;	}	
	if(Cnt_t>=3)
	{//Flash��д�����Ϣ
		Registed_User=1;		
		for(uint8_t i=0;i<UserName_LenMAX;i++)
		{
			userName.Str[i]=Flash_ReadBuff_t [4+i];
		}	
		for(uint8_t j=0;j<Password_LenMAX;j++)
		{
			Password.Str[j]=Flash_ReadBuff_t [24+j];
		}		
		return 1;
	}
	else
	{
		Registed_User=0;
		return 0;		
	}	
}
/**
 * @brief Flash_Failure_mark//��Flash�е����ݱ��Ϊ��Ч
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��17��
*/
int8_t Flash_Failure_Mark(void)
{
	//�������д���⼸�����ݻᱣ֤Flash ��ÿ���洢λĥ������
	uint8_t FlashWrite_temp[4]={ExistenceCheck2,ExistenceCheck1,\
															 ExistenceCheck4,ExistenceCheck3};
	Flash_Write(UserInfo_FlashSaveAddr,ExistenceCheck_FlashLen,(uint32_t*)FlashWrite_temp); 
	return 0;
}
/**
 * @brief Write username and password from Flash//flashд���û���Ϣ���ƻ��ô�ռ����ĥ�����
 * @param	Write_Flag
 * @retval void
 * @author ZCD1300 
 * @Time 2021��11��17��
*/
/*
	 ����STM32F407��
	�����Flash_Sector_11 0x080E0000��128k����ʼд�룬��Ϊ�˴���flash�������ᱻռ�õ�������
	֮ǰ��֮��Ԥ����λ�ö��㹻��	
	��ʼ��ַ��0x080E0000		�ռ��С��128K�ֽ�
*/

int8_t Flash_ValidMark_Equilibrium(uint8_t Write_Flag)
{
	uint8_t FlashWrite_temp[UserName_LenMAX+Password_LenMAX+4]= \
														 {ExistenceCheck1,ExistenceCheck2,\
															ExistenceCheck3,ExistenceCheck4,0}; //11��\44�ֽ�

	if(Write_Flag==1)
	{
		FlashWrite_temp[0]=ExistenceCheck1;
		FlashWrite_temp[1]=ExistenceCheck2;
		FlashWrite_temp[2]=ExistenceCheck3;	
		FlashWrite_temp[3]=ExistenceCheck4;			
		for(uint8_t i=0;i<UserName_LenMAX;i++)
		{
			FlashWrite_temp[4+i]=userName.Str[i];
			FlashWrite_temp[UserName_LenMAX+4+i]=Password.Str[i];
		}

		Flash_Write(UserInfo_FlashSaveAddr,11,(uint32_t*)FlashWrite_temp); 
		Delay(100);


		return 1;			
	}
	else
	{
		return 0;	
	}

}
/**
 * @brief Distribution via mobile app //wifi�ֻ�����
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021��11��22��
*/
	uint8_t SSID_Sta_t=0;
	uint8_t Password_Sta_t=0;
	uint8_t Password_End_t=0;
uint8_t HTMLBuff[]="<!DOCTYPE html><html><style>.b {margin-inline:16px;width:100%;height:48px;font-size:36px;}</style><body><form name='myForm'><div><p class='b'>SSID:</p><input class='b'name='ss_id'></div><div><p class='b'>Password:</p><input class='b'name='pa_ss'></div></form><p class='b'></p><button class='b'onclick='submit()'>SUBMIT</button></body><script>function submit() {let v1=myForm.ss_id.value;let v2=myForm.pa_ss.value;if(v1&&v2) {let r =new XMLHttpRequest();r.open('POST','/connect',true);let obj={'_ssid':v1,'_pass':v2};r.send(JSON.stringify(obj))} else {alert('error input')}}</script></html>";
int8_t Distribution_via_app(void)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)//�˳�͸��
	{//δ�˳�͸��
		if(Wifi_ExitTransp_Send())
		{
			//�˳�͸��
		}
		else
		{return 2;}//�˳�͸��ʧ��
	}
	Wifi_CLOSE_Connection(0);//�ر�����	
	
	Wifi_CheckSwitch_Workstate(3);//�л�����APģʽ
	Delay(2000);//��΢��һ�ᣬ��AP��
	Wifi_Read_WorkState();
	
	if(Wifi_module_State==1)
	{//ģʽ�������л�ʧ��
		return 3;
	}
	//�����Ѿ����˳�͸�����ر���ȫ�����ӡ�����AP
	Wifi_Read_IP_MAC();//��ȡAP����
	Wifi_CIPMUX_Ctrl(1);//ʹ�ܶ�����
	
	char GETWAY_broadcast_t[16]={0};
	char UDP_Target_t1[]="0,\"UDP\",\"";
	char UDP_Target_t2[]="\",1112,8080,0";
	char UDP_Target_t[42+2]={0};
	uint8_t CMD_len_t=0;
	uint8_t GATEWAY_LEN_t=0;	

	{//������������
		strcpy(GETWAY_broadcast_t,wifi_AP_GETWAY);
		uint8_t point_t=0;
		for(uint8_t i=15;i>=4;i--)
		{//����Ѱ�ҵ�һ��'.'
			if(GETWAY_broadcast_t[i]=='.')
			{
				point_t=i;
				break;
			}
		}
		GETWAY_broadcast_t[point_t+1]='2';
		GETWAY_broadcast_t[point_t+2]='5';
		GETWAY_broadcast_t[point_t+3]='5';		
		strcpy(UDP_Target_t,UDP_Target_t1);
		strcpy(UDP_Target_t+7+2,GETWAY_broadcast_t);
		GATEWAY_LEN_t=7+2+strlen(GETWAY_broadcast_t);
		strcpy(UDP_Target_t+GATEWAY_LEN_t,UDP_Target_t2);
		for(uint8_t l=0;l<42+2;l++)
		{
			if(UDP_Target_t[l]!=0)
			{
				CMD_len_t++;
			}
			else
			{
				break;
			}
		}
	}
	wifi_build_connect(0,UDP_Target_t,CMD_len_t);//����UDP�㲥����
	Connection_List[0]=1;//���UDP���ӹ̶�Ϊ����0,���ΪUDP����
	
	Delay(2000);
	Wifi_TCPServer_Ctrl(1);//��Server
	
	DebugPrint_uart_LineFeed("** Waiting for app distribution network.",200);	

	char SSID_input_t[50]={0};	//�ݴ�
	char Password_t[50]={0};		
	//�����Ѿ������UDP��TCP Server�ĳ�ʼ��
	
	AP_SubmitError_Lable:
	Clear_Buff(Rx_Buff1,150,BuffMAX);//���Rx_buff1һ������
	UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ	
	Delay(1000);
	while(1)
	{
 		Clear_Buff(Rx_Buff1,150,BuffMAX);//���Rx_buff1һ������		
		Delay(10);
		{//TCP���Ӵ���
			if(StringSearch(",CONNECT",8,150))
			{//�½�TCP����
				if(Connection_List[Rx_Buff1[StringSecanPointer-1]-48]==0)
				{
					Connection_List[Rx_Buff1[StringSecanPointer-1]-48]=2;
					Rx_Buff1[StringSecanPointer]=' ';
				}
			}
			else if(StringSearch(",CLOSED",7,150))
			{//�ر�TCP����
				Connection_List[Rx_Buff1[StringSecanPointer-1]-48]=0;
 				Rx_Buff1[StringSecanPointer]=' ';
			}
		}				
		{//HTTP Server
			if(StringSearch("GET / HTTP/1.1\r\n",16,150))
			{//GET ����
				Rx_Buff1[StringSecanPointer]=' ';
				uint16_t Html_Len=sizeof(HTMLBuff);
				char Send_temp[]="1,417";
				Send_temp[2]=Html_Len/100+48;
				Send_temp[3]=(Html_Len%100)/10 +48;
				Send_temp[4]=Html_Len%10+48;
				Wifi_CMD_SEND(WifiSENDLen,Send_temp,sizeof(WifiSENDLen),5);
				UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ	
				Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);
				UART_Send(&huart1,HTMLBuff,Html_Len,1000);			
				
			}
			else if(StringSearch(":POST /connect",13,150))
			{//Web Submit
				uint8_t SubmintTemp_t[500]={0};		
				for(uint16_t i=400;i<400+500;i++)
				{
					SubmintTemp_t[i-400]=Rx_Buff1[i];
				}
				
				StringSearch_Assign("\r\n\r\n{\"_ssid\":\"",SubmintTemp_t,14,150);
				SSID_Sta_t=StringSecanPointer_Assign_Buff+14;
				
				StringSearch_Assign("\"_pass\":\"",SubmintTemp_t,9,150);
				Password_Sta_t=StringSecanPointer_Assign_Buff+9;	
				
				StringSearch_Assign("\"}",SubmintTemp_t,2,150);
				Password_End_t=StringSecanPointer_Assign_Buff;		
				
				for(uint16_t j=0;j<1+((Password_Sta_t-9-2)-SSID_Sta_t);j++)//�ݴ�ssid
				{
					SSID_input_t[j]=SubmintTemp_t[SSID_Sta_t+j];
					if(j==((Password_Sta_t-9-2)-SSID_Sta_t))
					{
						SSID_input_t[j]='#';
					}
				}			
				for(uint16_t k=0;k<1+(Password_End_t-Password_Sta_t);k++)//�ݴ�password
				{
					Password_t[k]=SubmintTemp_t[Password_Sta_t+k];				
					if(k==(Password_End_t-Password_Sta_t))
					{
						Password_t[k]='#';
					}
				}		
				break;					
			}
		}		
		{//UDP Data
			if(StringSearch("+IPD,0",6,150))
			{
				Rx_Buff1[StringSecanPointer]=' ';
				if(StringSearch("SSID:",5,150))
				{
					Rx_Buff1[StringSecanPointer]=' ';
					SSID_Sta_t=StringSecanPointer;
					StringSearch("#Password:",10,150);
					Rx_Buff1[StringSecanPointer]=' ';
					Password_Sta_t=StringSecanPointer;	
					StringSearch("#END",4,150);
					Rx_Buff1[StringSecanPointer]=' ';
					Password_End_t=StringSecanPointer;				
					for(uint8_t j=0;j<1+(Password_Sta_t-(SSID_Sta_t+5));j++)//�ݴ�ssid
					{
						SSID_input_t[j]=Rx_Buff1[SSID_Sta_t+5+j];
					}			
					for(uint8_t k=0;k<1+(Password_End_t-(Password_Sta_t+10));k++)//�ݴ�password
					{
						Password_t[k]=Rx_Buff1[Password_Sta_t+10+k];
					}		
					break;					
				}
				else
				{//�������ݵĸ�ʽ����,��������
					DebugPrint_uart_LineFeed("** Data in wrong format.",200);	
				
				}
			}
		}
		
		
	}
	if(SSID_input_t[0]==0||Password_t[0]==0)
	{//�����ָ��
		goto AP_SubmitError_Lable;
	}
	Wifi_Connect_AP(SSID_input_t,Password_t);
	Switched_AP=1;
	DebugPrint_uart("** SSID:",200);
	DebugPrint_uart(SSID_input_t,200);		
	DebugPrint_uart("\r\n",200);
	DebugPrint_uart_LineFeed("** Distribution network completed.",200);	
	DebugPrint_uart_LineFeed("** Is automatically restarting your system.",200);	
	Delay(3000);
	MCU_System_RST();
	return 0;			//��������

}
/**
 * @brief Offline data staging area //����ʱ�����ݴ���
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021��11��25��
*/
void Calcu_StackRemainingSpace_BYTE(void)//�����������������滮�Ĵ洢��ʣ��ռ�
{
		StackRemainingSpace_BYTE=OfflineDATAStack_MAX-(StackHeader_NUM*StackUnitSize);//����ʣ��ռ�
}
uint16_t StackHeader_NUM _no_init;//�洢ջ�е�����
uint16_t StackRemainingSpace_BYTE=0;//ʣ����ÿռ䣨�ֽڣ�
uint16_t OfflineDATA_PowerON_Judge _no_init;//���������ж���ϵͳ��λ���ǵ��������ϵ磻�����ϵ��Ӧд��һ�����ݣ�
uint8_t PowerON_Flag=0;
uint8_t Offline_DATA_Stack[OfflineDATAStack_MAX] _no_init;//�������ϵͳ��λ���ᱻˢ�µ�������Ӳ���ϵ���Ȼ��

int8_t maintain_Offline_data_BUFF(void)//init maintain
{
	if(OfflineDATA_PowerON_Judge!=12345)
	{//�ϵ�����ֵ������������12345������������ֵ����Ϊ��ϵͳ���縴λ
	 //����˵�����ϵ�
		
		OfflineDATA_PowerON_Judge=12345;
		Clear_Buff(Offline_DATA_Stack,OfflineDATAStack_MAX,OfflineDATAStack_MAX);//������ϵ��ʼ����0
		StackHeader_NUM=0;	//�ͷ�ȫ������
		Calcu_StackRemainingSpace_BYTE();//����ʣ��ռ�
		PowerON_Flag=1;
		return 1;//�����ϵ�
	}
	//������ϵͳ����������\����δ����������
	Calcu_StackRemainingSpace_BYTE();
	PowerON_Flag=0;
	
	return 0;
}
/**
 * @brief Offline data staging area push//����ʱ�����ݴ��� ��ջ
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021��11��25��
*/
int8_t Offline_Data_PUSH(uint8_t *DataStackInput_t)
{
	if(StackRemainingSpace_BYTE<StackUnitSize)
	{//�ռ䲻�㣬��ջʧ��
		return 1;
	}
	OfflineDATA_PowerON_Judge=12345;
	uint16_t DATABuff_point_BASE=0;	
	DATABuff_point_BASE=StackHeader_NUM*StackUnitSize;
	for(uint8_t i=0;i<StackUnitSize;i++)//������λ�����ݱ���
	{
		Offline_DATA_Stack[DATABuff_point_BASE+i]=DataStackInput_t[i];
	}
	StackHeader_NUM++;//���ݼ�������
	return 0;
}
/**
 * @brief Offline data staging area pop//����ʱ�����ݴ��� ��ջ
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021��11��25��
*/
int8_t Offline_Data_POP(uint8_t *DataStackOutput_t)
{
	if(StackHeader_NUM==0)
	{//ջ��Ϊ��
		return 1;
	}
	OfflineDATA_PowerON_Judge=12345;
	uint16_t DATABuff_point_BASE=0;	
	StackHeader_NUM--;//���ݼ�������
	DATABuff_point_BASE = StackHeader_NUM*StackUnitSize;
	for(uint8_t i=0;i<StackUnitSize;i++)//������λ������POP
	{
		DataStackOutput_t[i]=Offline_DATA_Stack[DATABuff_point_BASE+i];//���ݵ���
		Offline_DATA_Stack[DATABuff_point_BASE+i]=0;//��ջ���������
	}	
	return 0;
}
/**
 * @brief Sensor data push to stack
 * @param	uint8_t DeviceID_t//1~6
 * @retval void
 * @author ZCD1300 
 * @Time 2021��12��1��
*/
int8_t Sensor_PUSH_APP(uint8_t DeviceID_t)
{
	uint8_t PUSHSensor_t[StackUnitSize]={0};
	if(DeviceID_t>=1)
	{
		Load_Sensor_Data(Upload_DataTemp,DeviceID_t);
		{//�޸�����
			PUSHSensor_t[0]='*';
			PUSHSensor_t[34]='#';
			PUSHSensor_t[35]=0;//ͷβ
			PUSHSensor_t[1]=DeviceID_t;//id
			
			PUSHSensor_t[2]=Upload_DataTemp[31];//�¶�4
			PUSHSensor_t[3]=Upload_DataTemp[32];
			PUSHSensor_t[4]=Upload_DataTemp[33];
			PUSHSensor_t[5]=Upload_DataTemp[34];
			
			PUSHSensor_t[6]=Upload_DataTemp[42];//ʪ��3
			PUSHSensor_t[7]=Upload_DataTemp[43];
			PUSHSensor_t[8]=Upload_DataTemp[44];
			
			PUSHSensor_t[9]=Upload_DataTemp[53];//����5
			PUSHSensor_t[10]=Upload_DataTemp[54];
			PUSHSensor_t[11]=Upload_DataTemp[55];
			PUSHSensor_t[12]=Upload_DataTemp[56];
			PUSHSensor_t[13]=Upload_DataTemp[57];
			
			PUSHSensor_t[14]=Upload_DataTemp[65];//������̼ 4
			PUSHSensor_t[15]=Upload_DataTemp[66];
			PUSHSensor_t[16]=Upload_DataTemp[67];
			PUSHSensor_t[17]=Upload_DataTemp[68];
			
			PUSHSensor_t[18]=Upload_DataTemp[76];//���� 4
			PUSHSensor_t[19]=Upload_DataTemp[77];
			PUSHSensor_t[20]=Upload_DataTemp[78];
			PUSHSensor_t[21]=Upload_DataTemp[79];
			
			PUSHSensor_t[22]=Upload_DataTemp[7];//������
			PUSHSensor_t[23]=Upload_DataTemp[8];
			PUSHSensor_t[24]=Upload_DataTemp[10];
			PUSHSensor_t[25]=Upload_DataTemp[11];
			PUSHSensor_t[26]=Upload_DataTemp[13];
			PUSHSensor_t[27]=Upload_DataTemp[14];
			
			PUSHSensor_t[28]=Upload_DataTemp[16];//ʱ����
			PUSHSensor_t[29]=Upload_DataTemp[17];
			PUSHSensor_t[30]=Upload_DataTemp[19];
			PUSHSensor_t[31]=Upload_DataTemp[20];
			PUSHSensor_t[32]=Upload_DataTemp[22];
			PUSHSensor_t[33]=Upload_DataTemp[23];
		}
		//������ջ
		if(Offline_Data_PUSH(PUSHSensor_t)==0)
		{
			return 0;
		}
		else
		{return 2;}//��ջʧ�ܣ��ռ䲻��
	}
	else
	{return 1;}//����ID����
	
}
/**
 * @brief Sensor stack data pop to buff 
 * @param	char * OutputBUFF_t //OutputBUFF_t�ռ䲻��С�ڴ洢��Ԫ��С
 * @retval void
 * @author ZCD1300 
 * @Time 2021��12��1��
*/
uint8_t DeviceID_stack=0;
uint8_t StackTimeStamp_Flag=0;
int8_t Sensor_POP_APP(char * OutputBUFF_t)
{
	if(Offline_Data_POP(OutputBUFF_t)==0)
	{//����POP
		if((OutputBUFF_t[0]=='*')&&(OutputBUFF_t[34]=='#'))
		{//��������
			DeviceID_stack=OutputBUFF_t[1];
			Upload_DataTemp[31]=OutputBUFF_t[2];//�¶�4
			Upload_DataTemp[32]=OutputBUFF_t[3];
			Upload_DataTemp[33]=OutputBUFF_t[4];
			Upload_DataTemp[34]=OutputBUFF_t[5];
													
			Upload_DataTemp[42]=OutputBUFF_t[6];//ʪ��3
			Upload_DataTemp[43]=OutputBUFF_t[7];
			Upload_DataTemp[44]=OutputBUFF_t[8];
													
			Upload_DataTemp[53]=OutputBUFF_t[9];//����5
			Upload_DataTemp[54]=OutputBUFF_t[10];
			Upload_DataTemp[55]=OutputBUFF_t[11];
			Upload_DataTemp[56]=OutputBUFF_t[12];
			Upload_DataTemp[57]=OutputBUFF_t[13];
													
			Upload_DataTemp[65]=OutputBUFF_t[14];//������̼ 4
			Upload_DataTemp[66]=OutputBUFF_t[15];
			Upload_DataTemp[67]=OutputBUFF_t[16];
			Upload_DataTemp[68]=OutputBUFF_t[17];
																			
			Upload_DataTemp[76]=OutputBUFF_t[18];//���� 4
			Upload_DataTemp[77]=OutputBUFF_t[19];
			Upload_DataTemp[78]=OutputBUFF_t[20];
			Upload_DataTemp[79]=OutputBUFF_t[21];
			
			Upload_DataTemp[5] ='2';
			Upload_DataTemp[6] ='0';
			Upload_DataTemp[7] =OutputBUFF_t[22];//������
			Upload_DataTemp[8] =OutputBUFF_t[23];
			Upload_DataTemp[10]=OutputBUFF_t[24];
			Upload_DataTemp[11]=OutputBUFF_t[25];
			Upload_DataTemp[13]=OutputBUFF_t[26];
			Upload_DataTemp[14]=OutputBUFF_t[27];

			Upload_DataTemp[16]=OutputBUFF_t[28];//ʱ����
			Upload_DataTemp[17]=OutputBUFF_t[29];
			Upload_DataTemp[19]=OutputBUFF_t[30];
			Upload_DataTemp[20]=OutputBUFF_t[31];
			Upload_DataTemp[22]=OutputBUFF_t[32];
			Upload_DataTemp[23]=OutputBUFF_t[33];			
			
			StackTimeStamp_Flag=1;
			return 0;
		}
		else
		{
			return 2;//���ݳ���
		}
	}
	else
	{//ջ�ڿ�
		return 1;
	}
	
}
/**
 * @brief Smart nest main function
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021��11��30��
*/
uint8_t CheckTCP_Return=0;
uint8_t CloseTCP_Return=0;
int8_t SmartNest_Main_Function(void)
{
	DebugPrint_uart_LineFeed("** Auto upload start.",200);
	CheckTCP_Return=Check_TCP_Connection();//�������ͬʱ�����tcp���Ӳ��˳�͸��
	if(TCPLink_Alive==1)//������Ȼ����
	{
		Start_Transp_Send();//����ʹ�ܲ�����͸��	
	}

	if(Registed_User==0)
	{//��һ�γ�ʼ����hostӦ���Լ�����ע��host����
		Reg_Host_App();//������濴�����ע�ᳬʱ����ô�������ߴ������ó�������ʱ��
		
		{//ע������Device
			Reg_Device_ALL();
		}
	}
	//������Ӧ���Ѿ����hostע���deviceע�᣻
	{//���ݻ�ȡ�� ��ȡȫ����device���ݣ�
		//ZigbeeRead_ALL_Sensor();
		zigbee_Read_Sensor(2,3,DeviceSNList);
	}

	{//�ϴ����ݲ��֣��ϴ�֮ǰӦ�ü���Ƿ��������ӣ����ҷ������������Ͽ����ӡ�		
		//�������
		if(TCPLink_Alive!=1)
		{//���ӶϿ������½�������
			Wifi_ExitTransp_Send();				
			Wifi_CLOSE_Connection(0);
			Connect_Server_Smatnest();//�������ӵ�������
			if(TCPLink_Alive==1)
			{
				Start_Transp_Send();//ʹ�ܲ�����͸��
			}
			else
			{
				return 2;
			}
			
		}
		if(TCPLink_Alive==1)//����Ӧ���Ѿ�����������������
		{//�ϴ�����
			UploadDataAPP_Return=Upload_Data_APP();
		}	
		else
		{
			DebugPrint_uart("Failed to connect to the server.\r\n",200);
			return 1;
		}
	}
	return 0;
}
/**
 * @brief Data upload //�����ϴ����������ڽ���ȡ�ĺ����ϴ���ͬʱ�������߶�ջ������ݴ�ȡ���ϴ�
 * @param	void
 * @retval int8_t return 
 * @author ZCD1300 
 * @Time 2021��11��30��
*/

int8_t UploadDataAPP_Return=0;
char HistoryDataPOP_BUFF[StackUnitSize+1]={0};
int8_t Upload_Data_APP(void)
{
	uint8_t Stack_FULL_Flag=0;
	uint8_t Stack_uploadRetryFlag=0;	
	if(StackHeader_NUM>0)
	{
		DebugPrint_uart("** History data upload retry.",200);
		while(StackHeader_NUM)
		{//���ݶ�ջ�д�����ʷδ�ϴ����ݣ��ϴ���ʷ����
			Sensor_POP_APP(HistoryDataPOP_BUFF);//ͬʱ�Ѿ������ݼ��ص��ַ�����
			
			Load_DeviceSN(HistoryDataPOP_BUFF[1]);	//��ӦSN
			
			uint8_t Stack_uploadRetryCNT=0;
	
			while(Upload_DeviceData())	
			{//�ϴ����ݣ��ϴ��ɹ�����ѭ��,����5s����һ��,����ظ�10�Σ���ʱ�ݴ浽ջ��
				Delay(2000);		
				if(Stack_uploadRetryCNT>=3)//��ʱ�ĳ�3�μ��2��
				{
					Stack_uploadRetryCNT=0;
					Stack_uploadRetryFlag=1;//ջ�����ϴ�ʧ�ܱ�־
					break;	
				}
				Stack_uploadRetryCNT++;
			}		
			Stack_uploadRetryCNT=0;

			if(Stack_uploadRetryFlag!=0)
			{//ջ�������ϴ��ٴ�ʧ��
				Offline_Data_PUSH(HistoryDataPOP_BUFF);//����ѹ��ջ��
				break;
			}
			else
			{
				Stack_uploadRetryFlag=0;
			}
		}	
	}

	
	if(1)
	{//�������ݶ�ջ��û�����ݣ�ֱ���ϴ����ݡ�
		DebugPrint_uart("\r\n** New data upload.",200);
		uint8_t DataUploadFail_Flag=0;//�����ϴ�ʧ�ܱ�־	
		uint8_t WiFiUploadFail_Flag=0;//WiFi�ϴ����ݳ�ʱ
		for(uint8_t i=1;i<=DeviceNUM_MAX;i++)//DeviceNUM_MAX
		{
			if(DeviceOnline_Falg[i-1]!=1)//����Ӧ���ն��Ƿ�ע��
			{ continue;	}	//�ն�δע��	
			Load_DeviceSN(i);									//����device SN
			Load_Sensor_Data(Upload_DataTemp,i);//���ض�Ӧdevice SN�����ݵ��ַ���
			uint8_t uploadRetryCNT=0;	
			while(Upload_DeviceData())	
			{//�ϴ����ݣ��ϴ��ɹ�����ѭ��,��������,����ظ�4�Σ���ʱ�ݴ浽ջ��
				Delay(2000);		
				if(uploadRetryCNT>=3)//��ʱ�ĳ�4�μ��2��
				{
					uploadRetryCNT=0;
					WiFiUploadFail_Flag=1;//wifi�����ϴ�ʧ�ܱ�־
					
					break;	
				}
				uploadRetryCNT++;
			}
			
			if(WiFiUploadFail_Flag==1)//WiFi�ϴ����ݳ�ʱ��
			{//�ⲿ����NBIOTģ���������ϴ���
							
				nbiot_build_connect(SmartNest_Domain_NBIOT,SmartNest_Domain_NBIOTLEN);
				Delay(500);			
				if(NBIOT_TCPLink_Alive==1&&NBIOT_Connected_to_Server_Flag==1)
				{
					if(Upload_DeviceData_NBIOT()==0)
					{//�ϴ��ɹ�
						DataUploadFail_Flag=0;
					}
				}
				else
				{
					DataUploadFail_Flag=1;//���ַ�ʽ���ϴ�ʧ��
				}				

			}
			if(DataUploadFail_Flag==1)
			{//�ϴ�ʧ��
				if(Sensor_PUSH_APP(i)==0)//������ջ�ݴ棬�����ϴ�����
				{//������ջ�ɹ�
					
					zigbeeClear_StructData(i);//�����нṹ���������			
				}
				else
				{//��ջʧ�ܣ�ջ��
					Stack_FULL_Flag=1;
				}
			}		
			else
			{//�ϴ����
				DataUploadFail_Flag=0;
				zigbeeClear_StructData(i);//�����нṹ���������			
			}
		}

		DebugPrint_uart_LineFeed("** Auto upload complete.",200);
	}
	if(Stack_FULL_Flag==1)
	{
		return 2;//��������ջ��������ID�������ջʧ�ܣ�
	}
	return 0;
}



/**
 * @brief Connection_ControlAPP
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021��12��5��
*/
//���������Ҫ���������Ƶ������������ӣ��ϴ�������ɺ�ά��һ��ʱ�����ӣ���ʱ�Զ��Ͽ�
uint16_t TCPLink_AliveTime_CNT=0;
uint8_t Connection_ControlAPP(void)
{//��һ����������������������ӵĿ���
	if(TCPLink_Alive==1)
	{//���ӵ�������
		TCPLink_AliveTime_CNT++;
		if(Wifi_Transparent_SendEntered_Flag!=1)
		{
			//Start_Transp_Send();
		}
	}
	else
	{
		
		return 1;}//�����������в��ᴥ��
	if(TCPLink_AliveTime_CNT>12000)//2���ӱ���ʱ��
	{
		TCPLink_AliveTime_CNT=0;
		if(Debug_enable_flag!=1)
		{//debugģʽʹ�ܵ�ʱ�������ر�����
			Wifi_ExitTransp_Send();	
			CloseTCP_Return=Wifi_CLOSE_Connection(0);//�ر�ȫ������
		}
	}
}	
/**
 * @brief LED Display Ctrl APP
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021��12��21��
*/
uint8_t LED_State_CtrlAPP(void)
{
	if(TCPLink_Alive==1)
	{
		LED_ctrl(1,1);
	}
	else
	{
		LED_ctrl(1,0);
	}
	
	
	
	return 0;
}


