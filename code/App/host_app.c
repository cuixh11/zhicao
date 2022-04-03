#include "host_app.h"

FirmwareINFO_t FirmwareINFO_Local={ .FirmwareType="HOST",\
																		.FirmwareVersion="Y149",\
																		.FirmwareSize=1024,\
																		.Firmware_MD5="abcd567890abcde" };

/**
 * @brief 主机启动函数，从此主机进入启动流程
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月14日
*/
int8_t Host_SystemStartUp_SmartNest(void)
{
	LCD_Startup_ProgressBar(0);
	json_Space_init();//进入主函数前必须给json_t变量开辟空间
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
	
	//-----------------wifi模块启动
	DebugPrint_uart("||----Start initializing WiFi module....\r\n\r\n",0x200);
	Wifi_Init();
	LCD_Startup_ProgressBar(40);
	LED_ctrl(0,1);//LED0 on
	
	//-----------------NBIOT模块启动
	DebugPrint_uart("||----Start initializing NBIOT module...\r\n\r\n",0x200);	
	NBIOT_Init();
	LCD_Startup_ProgressBar(60);
	LED_ctrl(1,1);//LED1 on
	
	//-----------------从机网络启动
	DebugPrint_uart("||----Start initializing ZigBee network....\r\n\r\n",0x200);	
	//ZigBeeNET_Init();
	LCD_Startup_ProgressBar(80);
	LED_ctrl(2,1);//LED2 on
	
	Host_test_SmartNest();//测试网络连接
	LCD_Startup_ProgressBar(95);	
	
	DebugPrint_uart("\r\n||-----------Boot complete------------||\r\n\r\n",200);	

	LED_ctrl(0,0);
	LED_ctrl(1,0);
	LED_ctrl(2,0);
//	Wifi_ExitTransp_Send();
//	Wifi_CLOSE_Connection(0);

}
/**
 * @brief utoken的过期检查，过期自动重新登录更新token;此函数应放在主循环或者定时器循环中
 * @param	void
 * @retval int8_t return //0表示token有效或者重新获取成功，1表示重新登录失败（应跳转到登录函数）
 * @author ZCD1300
 * @Time 2021年11月14日
*/
int8_t Host_utoken_Check_SmartNest(void)
{
	uint8_t cmp_flag=0;
	{//有效性判断
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
	if(cmp_flag==1)//utoken有效
	{
		BodyHead_pro.utoken_Live=1;
		return 0;
	}
	else//utoken过期
	{
		//检查连接
		uint8_t Rebuild_Connection_CNT_T=0;
		uint8_t Retry_EnterTranspSend_CNT_t=0;
		while(1)
		{
			CheckTCP_Return=Check_TCP_Connection();//这个函数同时检查了tcp连接并退出透传
			if(TCPLink_Alive!=1)
			{//如果连接断开；重新建立连接
				Wifi_ExitTransp_Send();				
				Wifi_CLOSE_Connection(0);
				Connect_Server_Smatnest();//尝试连接到服务器
				if(TCPLink_Alive==1)
				{//重连服务器成功
					if(Start_Transp_Send()==0)
					{//使能并开启透传
						break;
					}
					else
					{	
						Retry_EnterTranspSend_CNT_t++;
						if(Retry_EnterTranspSend_CNT_t>10)
						{//十次尝试进入透传失败
							Retry_EnterTranspSend_CNT_t=0;
							return 4;//WiFi进入透传模式失败
						}
					}
				}
				else
				{//重连失败
					Rebuild_Connection_CNT_T++;
					if(Rebuild_Connection_CNT_T>10)//十次重试连接到服务器失败
					{
						Rebuild_Connection_CNT_T=0;
						return 3;//连接到服务器失败
					}
				}
				
			}
			else
			{//连接未断开,重新进入透传
				Retry_EnterTranspSend_CNT_t=0;
				while(Start_Transp_Send())
				{
					Retry_EnterTranspSend_CNT_t++;
					if(Retry_EnterTranspSend_CNT_t>10)
					{//十次尝试进入透传失败
						Retry_EnterTranspSend_CNT_t=0;
						return 4;//WiFi进入透传模式失败
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
			if(retrying_cnt>=10)//重新登录失败
			{
				retrying_cnt=0;
				DebugPrint_uart("** The attempt to log in again failed.\r\n",0x200);
				
				Registed_User=0;
				Flash_Failure_Mark();//这里还要标记flash的内容无效
				Reg_User_Check();
				return 1;//utoken过期，且重新登录失败
			}
		}
		
		//重新登录并重置utoken成功
		BodyHead_pro.utoken_Live=1;
		DebugPrint_uart_LineFeed("** The token has been refreshed.",200);
		return 0;		
	}
	//这里正常运行不会到达
	return 2;//未知错误	
}
/**
 * @brief 主机用来测试网络连接的函数，应在主机启动函数之后
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月14日
*/
uint8_t get_timetest_cnt=0;
int8_t Host_test_SmartNest(void)
{
	retry_time_test:
	if(get_timetest_cnt>10)
	{
		MCU_System_RST();
	}
	Get_NETTime();	//获取时间,检查连接
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
		Flash_Failure_Mark();//这里还要标记flash的内容无效		
		goto InputUser_retry;
	}
	
}
/**
 * @brief Smart nest Debug APP 
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021年11月18日
*/
uint8_t Debug_enable_flag=0;
uint8_t press_test_Flag=0;
void SmartNest_Debug_APP(void)
{
//串口Debug指令，仅测试用（最长匹配20个字符）
	if(Debug_enable_flag!=0)
	{
		if(Uart_Debug_CMD_ADD("regHost",Rx_Buff2,0,100))//注册主机
		{  Reg_Host_App();	}
		if(Uart_Debug_CMD_ADD("regDevice_test",Rx_Buff2,0,100))//注册从机，测试用的SN
		{	 Register_Device();	}			
		if(Uart_Debug_CMD_ADD("regDevice",Rx_Buff2,0,100))//注册从机，全部已经加入Zigbee网络的
		{	 Reg_Device_ALL();	}	
		if(Uart_Debug_CMD_ADD("login",Rx_Buff2,0,100))//手动登录 
		{	 Login_Uesr(userName.Str,Password.Str);		}		
		if(Uart_Debug_CMD_ADD("flashR",Rx_Buff2,0,100))//读取Flash(指定区域)
		{	 	
			char Read_temp[64];
			Flash_Read(UserInfo_FlashSaveAddr,16,(uint32_t *)Read_temp);	
			DebugPrint_uart("\r\n** >>",200);	
			DebugPrint_uart(Read_temp,200);		
			DebugPrint_uart("\r\n** Flash read over.",200);								
		}	
		if(Uart_Debug_CMD_ADD("flashE",Rx_Buff2,0,100))//擦除保存的用户信息
		{	 	
			Flash_Failure_Mark();
			Registed_User=0;	
			DebugPrint_uart_LineFeed("** User information has been erased.",200);	
		}					
		if(Uart_Debug_CMD_ADD("unpackBuff",Rx_Buff2,0,100))//使能json Unpack buff显示函数
		{	 Display_unpackBuff_Flag=1;
			 DebugPrint_uart_LineFeed("** Enable display JSON unpack buff.",400);
		}	
		if(Uart_Debug_CMD_ADD("packBuff",Rx_Buff2,0,100))//使能json Pack buff显示函数
		{	 Display_packBuff_Flag=1;
			 DebugPrint_uart_LineFeed("** Enable display JSON pack buff.",400);
		}					
		if(Uart_Debug_CMD_ADD("wifiSwitch",Rx_Buff2,0,100))//切换wifi ap函数
		{	
			DebugPrint_uart_LineFeed("** Manually switch AP:",200);
			if(Wifi_ExitTransp_Send())//退出透传
			{
				Switch_Wifi_AP();
				WifiRST_OK_Flag=1;//跳过复位Wifi模块
				Wifi_Connect_AP_Flag=0;//清空WiFi连接标志
				GotIP_Flag=0;//清空Got IP标志
				Connected_to_Server_Flag=0;
				TCPLink_Alive=0;
				Wifi_Init();
			}
			else
			{
				DebugPrint_uart_LineFeed("** Exit transparent send fail.",200);
			}
		}	
		if(Uart_Debug_CMD_ADD("appWifiSet",Rx_Buff2,0,100))//切换wifi ap函数（APP配网）
		{	
			DebugPrint_uart_LineFeed("** Manually switch AP:",200);
			if(Wifi_ExitTransp_Send())//退出透传
			{
				Distribution_via_app();
			}
			else
			{
				DebugPrint_uart_LineFeed("** Exit transparent send fail.",200);
			}
		}			
		if(Uart_Debug_CMD_ADD("pressTest",Rx_Buff2,0,100))//上传数据压力测试
		{	 press_test_Flag=1;		}		
		if(Uart_Debug_CMD_ADD("checkAP",Rx_Buff2,0,100))//上传数据压力测试
		{	 Wifi_Check_AP_Name();	}				
		if(Uart_Debug_CMD_ADD("TCP?",Rx_Buff2,0,100))//显示当前TCP连接状态
		{	Check_TCP_Connection();
			if(TCPLink_Alive==1)
			{DebugPrint_uart_LineFeed("** TCP connection alive.",200);}
			else if(TCPLink_Alive==0)
			{DebugPrint_uart_LineFeed("** TCP connection not alive.",200);}
			else
			{DebugPrint_uart_LineFeed("** TCP state error.",200);}
		}				
		if(Uart_Debug_CMD_ADD("closeConnect",Rx_Buff2,0,100))//关闭全部连接
		{	 
			Wifi_ExitTransp_Send();
			Wifi_CLOSE_Connection(0);
			DebugPrint_uart_LineFeed("** OK",200);
		}	
		if(Uart_Debug_CMD_ADD("buildConnect",Rx_Buff2,0,100))//手动建立连接,自动进入透传
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
		if(Uart_Debug_CMD_ADD("wifiBuff",Rx_Buff2,0,100))//打印wifi buff 100
		{	 
			 UART_Send(&huart2,Rx_Buff1,100,400);
		}	
		if(Uart_Debug_CMD_ADD("zgSensor",Rx_Buff2,0,100))//读zigbee网络的传感器数据
		{	 
			 DebugPrint_uart("** Start\r\n",200);
			 ZigbeeRead_ALL_Sensor();
			DebugPrint_uart_LineFeed("** OK",200);
		}		
		if(Uart_Debug_CMD_ADD("zgMAC",Rx_Buff2,0,100))//读zigbee网络MAC
		{	 
			DebugPrint_uart("** Start\r\n",200);
			zigbee_Refesh_OnlineDeviceNUM();
			DebugPrint_uart_LineFeed("** OK",200);
		}			
		if(Uart_Debug_CMD_ADD("ServerPUSH",Rx_Buff2,0,100))//ServerPUSH,服务器推送功能测试
		{	
			PUSH_Serve();
			DebugPrint_uart("** PUSH test.\r\n",200);
		}			
		if(Uart_Debug_CMD_ADD("PUSH",Rx_Buff2,0,100))//数据暂存测试PUSH
		{	 
			Sensor_PUSH_APP(2);
			DebugPrint_uart_LineFeed("** OK",200);
		}	
		if(Uart_Debug_CMD_ADD("POP",Rx_Buff2,0,100))//数据暂存测试POP
		{	 
			Sensor_POP_APP(HistoryDataPOP_BUFF);
			DebugPrint_uart_LineFeed("** OK",200);
		}		
		if(Uart_Debug_CMD_ADD("DestroyStack",Rx_Buff2,0,100))//销毁整个栈数据
		{	 
			StackHeader_NUM=0;
			DebugPrint_uart_LineFeed("** OK",200);
		}	

		if(Uart_Debug_CMD_ADD("WtFirmINFO",Rx_Buff2,0,100))//复写固件信息，测试指令
		{	 
			Wirte_firmwareINFO(FirmwareINFO_Local);
			DebugPrint_uart_LineFeed("** OK",200);
		}			
		if(Uart_Debug_CMD_ADD("FirmINFO",Rx_Buff2,0,100))//读取固件信息
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
		if(Uart_Debug_CMD_ADD("stopHMI",Rx_Buff2,0,100))//关闭串口屏显示刷新（这串口屏可能会因为屏幕死机导致整机卡死，所以建议在调试时关闭刷新）
		{	 HMI_Display_RefeshDisableFlag=1;
			 DebugPrint_uart_LineFeed("** Disable HMI display refresh.",400);
		}	
		if(Uart_Debug_CMD_ADD("startHMI",Rx_Buff2,0,100))//开启串口屏显示刷新（这串口屏可能会因为屏幕死机导致整机卡死，所以建议在调试时关闭刷新）
		{	 HMI_Display_RefeshDisableFlag=0;
			 HMI_Rest();
			 DebugPrint_uart_LineFeed("** Enable HMI display refresh.",400);
		}	
	}		
	//----------------------------------------------------------------//
	if((Uart_Debug_CMD_ADD("Debug_zcd123",Rx_Buff2,0,100))||(KeyState[0]==0))//进入Debug
	{	 
		Debug_enable_flag=1;	
		Display_unpackBuff_Flag=0;//重新进入Debug模式可以 取消显示使能
		Display_packBuff_Flag=0;
		press_test_Flag=0;
		DebugPrint_uart("\r\n** Uart Debug Enabled.\r\n",200);
		//DebugPrint_uart_LineFeed("** Uart Debug Enabled.",200);	
	}			
	if(Uart_Debug_CMD_ADD("Debug quit",Rx_Buff2,0,100))//退出Debug
	{	 
		Debug_enable_flag=0;	
		DebugPrint_uart_LineFeed("** Uart Debug Disabled.",200);	
	}	
	//----------------------------------------------------------------//			
	{//Debug_APP默认运行内容
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
 * @brief 串口打印有符号数值，十万以内
 * @param	int
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021年11月18日
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
 * @param	uint8_t TimeTick	//循环进入的周期，单位秒（s），必须能整除60，否则时间不准
 * @retval void
 * @author ZCD1300 
 * @Time 2021年11月18日
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
			{//31天
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
			{//30天
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
			{//2月
				if(day<=29)//默认是闰年，如果错误等带网络校准
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
 * @brief Reg Host and Input host name//注册主机时等待输入host name
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月15日
*/
int8_t Reg_Host_App(void)
{
	char hostname_temp[21]={0};
	DebugPrint_uart("\r\n** Name this Host >>",0x200);
	Clear_Buff(Rx_Buff2,100,BuffMAX2);
	UART2_Refresh_Flag=0;
	if(Wait_Until_FlagTrue(&UART2_Refresh_Flag,30000))//最大等待30s
	{//时间内接收到输入
		strcpy(hostname_temp,Rx_Buff2);
		DebugPrint_uart(hostname_temp,200);
		if(!Load_host_name(hostname_temp))
		{//SN和host_name均加载完成
			Register_Host();
			return 0;
		}
		DebugPrint_uart_LineFeed("** host name is too long.",200);
		return 2;//输入host name错误//正常状况下不会到这里
	}
	else
	{//等待超时
		DebugPrint_uart_LineFeed("** It took too long to exit automatically.",0x200);
		return 1;//输入超时，自动退出
	}
	
}
/**
 * @brief Reg_User_Check//检测是否已经有注册过User账户，没有的话应该用户注册。（仅开发时可以用嵌入式终端注册，正式的版本应该由客户端注册）
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月16日
*/
uint8_t Registed_User=0;
uint8_t Changed_UserInfo=0;
int8_t Reg_User_Check(void)
{
	Flash_Read_UserInfo();
	if(Registed_User==0)
	{
		if(Changed_UserInfo==0)
		{//没有注册过User account，首次登录		
			DebugPrint_uart("\r\n** It looks like you're logging in for the first time.We need your smart nest account to continue.\r\n** Your account >>",400);
		}
		else
		{//修改过密码，自动重新登录失败
			DebugPrint_uart("\r\n** Your password or account has expired. Please log in again.\r\n** Your account >>",400);			
		}
		Clear_Buff(Rx_Buff2,100,BuffMAX2);
		UART2_Refresh_Flag=0;
		if(Wait_Until_FlagTrue(&UART2_Refresh_Flag,0))//不设置超时时间
		{//收到输入
			strcpy(userName.Str,Rx_Buff2);
		}
		DebugPrint_uart(userName.Str,200);
		DebugPrint_uart("\r\n** Your password >>",400);
		Clear_Buff(Rx_Buff2,100,BuffMAX2);
		UART2_Refresh_Flag=0;
		if(Wait_Until_FlagTrue(&UART2_Refresh_Flag,0))//不设置超时时间
		{//收到输入
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
 * @brief Read username and password from Flash//读取flash中的内容
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月16日
*/
	uint8_t Flash_ReadBuff_t[45]={0};
const	uint8_t ExistenceCheck_FlashLen=1;//前4个字节用来检查是否被写入过
const	uint8_t UserName_FlashLen=5;//username占用的字数
const	uint8_t Password_FlashLen=5;//password占用的字数
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
	{//Flash被写入过信息
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
 * @brief Flash_Failure_mark//将Flash中的内容标记为无效
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月17日
*/
int8_t Flash_Failure_Mark(void)
{
	//这个变量写入这几个内容会保证Flash 的每个存储位磨损相似
	uint8_t FlashWrite_temp[4]={ExistenceCheck2,ExistenceCheck1,\
															 ExistenceCheck4,ExistenceCheck3};
	Flash_Write(UserInfo_FlashSaveAddr,ExistenceCheck_FlashLen,(uint32_t*)FlashWrite_temp); 
	return 0;
}
/**
 * @brief Write username and password from Flash//flash写入用户信息；计划用大空间进行磨损均衡
 * @param	Write_Flag
 * @retval void
 * @author ZCD1300 
 * @Time 2021年11月17日
*/
/*
	 仅限STM32F407：
	建议从Flash_Sector_11 0x080E0000（128k）开始写入，因为此处的flash几乎不会被占用到，而且
	之前和之后预留的位置都足够大。	
	开始地址：0x080E0000		空间大小：128K字节
*/

int8_t Flash_ValidMark_Equilibrium(uint8_t Write_Flag)
{
	uint8_t FlashWrite_temp[UserName_LenMAX+Password_LenMAX+4]= \
														 {ExistenceCheck1,ExistenceCheck2,\
															ExistenceCheck3,ExistenceCheck4,0}; //11字\44字节

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
 * @brief Distribution via mobile app //wifi手机配网
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021年11月22日
*/
	uint8_t SSID_Sta_t=0;
	uint8_t Password_Sta_t=0;
	uint8_t Password_End_t=0;
uint8_t HTMLBuff[]="<!DOCTYPE html><html><style>.b {margin-inline:16px;width:100%;height:48px;font-size:36px;}</style><body><form name='myForm'><div><p class='b'>SSID:</p><input class='b'name='ss_id'></div><div><p class='b'>Password:</p><input class='b'name='pa_ss'></div></form><p class='b'></p><button class='b'onclick='submit()'>SUBMIT</button></body><script>function submit() {let v1=myForm.ss_id.value;let v2=myForm.pa_ss.value;if(v1&&v2) {let r =new XMLHttpRequest();r.open('POST','/connect',true);let obj={'_ssid':v1,'_pass':v2};r.send(JSON.stringify(obj))} else {alert('error input')}}</script></html>";
int8_t Distribution_via_app(void)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)//退出透传
	{//未退出透传
		if(Wifi_ExitTransp_Send())
		{
			//退出透传
		}
		else
		{return 2;}//退出透传失败
	}
	Wifi_CLOSE_Connection(0);//关闭连接	
	
	Wifi_CheckSwitch_Workstate(3);//切换到带AP模式
	Delay(2000);//稍微等一会，等AP打开
	Wifi_Read_WorkState();
	
	if(Wifi_module_State==1)
	{//模式错误且切换失败
		return 3;
	}
	//到此已经，退出透传、关闭了全部连接、打开了AP
	Wifi_Read_IP_MAC();//读取AP网关
	Wifi_CIPMUX_Ctrl(1);//使能多连接
	
	char GETWAY_broadcast_t[16]={0};
	char UDP_Target_t1[]="0,\"UDP\",\"";
	char UDP_Target_t2[]="\",1112,8080,0";
	char UDP_Target_t[42+2]={0};
	uint8_t CMD_len_t=0;
	uint8_t GATEWAY_LEN_t=0;	

	{//构造连接命令
		strcpy(GETWAY_broadcast_t,wifi_AP_GETWAY);
		uint8_t point_t=0;
		for(uint8_t i=15;i>=4;i--)
		{//倒序寻找第一个'.'
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
	wifi_build_connect(0,UDP_Target_t,CMD_len_t);//建立UDP广播连接
	Connection_List[0]=1;//这个UDP连接固定为连接0,标记为UDP连接
	
	Delay(2000);
	Wifi_TCPServer_Ctrl(1);//开Server
	
	DebugPrint_uart_LineFeed("** Waiting for app distribution network.",200);	

	char SSID_input_t[50]={0};	//暂存
	char Password_t[50]={0};		
	//到此已经完成了UDP和TCP Server的初始化
	
	AP_SubmitError_Lable:
	Clear_Buff(Rx_Buff1,150,BuffMAX);//清空Rx_buff1一块区域
	UART1_Refresh_Flag=0;//检测串口触发前要置位	
	Delay(1000);
	while(1)
	{
 		Clear_Buff(Rx_Buff1,150,BuffMAX);//清空Rx_buff1一块区域		
		Delay(10);
		{//TCP连接处理
			if(StringSearch(",CONNECT",8,150))
			{//新建TCP连接
				if(Connection_List[Rx_Buff1[StringSecanPointer-1]-48]==0)
				{
					Connection_List[Rx_Buff1[StringSecanPointer-1]-48]=2;
					Rx_Buff1[StringSecanPointer]=' ';
				}
			}
			else if(StringSearch(",CLOSED",7,150))
			{//关闭TCP连接
				Connection_List[Rx_Buff1[StringSecanPointer-1]-48]=0;
 				Rx_Buff1[StringSecanPointer]=' ';
			}
		}				
		{//HTTP Server
			if(StringSearch("GET / HTTP/1.1\r\n",16,150))
			{//GET 请求
				Rx_Buff1[StringSecanPointer]=' ';
				uint16_t Html_Len=sizeof(HTMLBuff);
				char Send_temp[]="1,417";
				Send_temp[2]=Html_Len/100+48;
				Send_temp[3]=(Html_Len%100)/10 +48;
				Send_temp[4]=Html_Len%10+48;
				Wifi_CMD_SEND(WifiSENDLen,Send_temp,sizeof(WifiSENDLen),5);
				UART1_Refresh_Flag=0;//检测串口触发前要置位	
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
				
				for(uint16_t j=0;j<1+((Password_Sta_t-9-2)-SSID_Sta_t);j++)//暂存ssid
				{
					SSID_input_t[j]=SubmintTemp_t[SSID_Sta_t+j];
					if(j==((Password_Sta_t-9-2)-SSID_Sta_t))
					{
						SSID_input_t[j]='#';
					}
				}			
				for(uint16_t k=0;k<1+(Password_End_t-Password_Sta_t);k++)//暂存password
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
					for(uint8_t j=0;j<1+(Password_Sta_t-(SSID_Sta_t+5));j++)//暂存ssid
					{
						SSID_input_t[j]=Rx_Buff1[SSID_Sta_t+5+j];
					}			
					for(uint8_t k=0;k<1+(Password_End_t-(Password_Sta_t+10));k++)//暂存password
					{
						Password_t[k]=Rx_Buff1[Password_Sta_t+10+k];
					}		
					break;					
				}
				else
				{//接收数据的格式不对,返回重试
					DebugPrint_uart_LineFeed("** Data in wrong format.",200);	
				
				}
			}
		}
		
		
	}
	if(SSID_input_t[0]==0||Password_t[0]==0)
	{//避免空指针
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
	return 0;			//正常返回

}
/**
 * @brief Offline data staging area //离线时数据暂存区
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021年11月25日
*/
void Calcu_StackRemainingSpace_BYTE(void)//这个函数仅用来计算规划的存储区剩余空间
{
		StackRemainingSpace_BYTE=OfflineDATAStack_MAX-(StackHeader_NUM*StackUnitSize);//计算剩余空间
}
uint16_t StackHeader_NUM _no_init;//存储栈中的数量
uint16_t StackRemainingSpace_BYTE=0;//剩余可用空间（字节）
uint16_t OfflineDATA_PowerON_Judge _no_init;//这是用来判断是系统复位还是掉电重新上电；正常上电后应写入一个数据；
uint8_t PowerON_Flag=0;
uint8_t Offline_DATA_Stack[OfflineDATAStack_MAX] _no_init;//这块区域系统复位不会被刷新掉，但是硬件上电仍然会

int8_t maintain_Offline_data_BUFF(void)//init maintain
{
	if(OfflineDATA_PowerON_Judge!=12345)
	{//上电的随机值几乎不可能是12345；如果是这个数值，认为是系统带电复位
	 //这里说明是上电
		
		OfflineDATA_PowerON_Judge=12345;
		Clear_Buff(Offline_DATA_Stack,OfflineDATAStack_MAX,OfflineDATAStack_MAX);//如果是上电初始化成0
		StackHeader_NUM=0;	//释放全部数据
		Calcu_StackRemainingSpace_BYTE();//计算剩余空间
		PowerON_Flag=1;
		return 1;//重新上电
	}
	//下面是系统带电软重启\或者未重启的流程
	Calcu_StackRemainingSpace_BYTE();
	PowerON_Flag=0;
	
	return 0;
}
/**
 * @brief Offline data staging area push//离线时数据暂存区 入栈
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021年11月25日
*/
int8_t Offline_Data_PUSH(uint8_t *DataStackInput_t)
{
	if(StackRemainingSpace_BYTE<StackUnitSize)
	{//空间不足，入栈失败
		return 1;
	}
	OfflineDATA_PowerON_Judge=12345;
	uint16_t DATABuff_point_BASE=0;	
	DATABuff_point_BASE=StackHeader_NUM*StackUnitSize;
	for(uint8_t i=0;i<StackUnitSize;i++)//单个单位的数据保存
	{
		Offline_DATA_Stack[DATABuff_point_BASE+i]=DataStackInput_t[i];
	}
	StackHeader_NUM++;//内容计数增加
	return 0;
}
/**
 * @brief Offline data staging area pop//离线时数据暂存区 出栈
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021年11月25日
*/
int8_t Offline_Data_POP(uint8_t *DataStackOutput_t)
{
	if(StackHeader_NUM==0)
	{//栈内为空
		return 1;
	}
	OfflineDATA_PowerON_Judge=12345;
	uint16_t DATABuff_point_BASE=0;	
	StackHeader_NUM--;//内容计数增加
	DATABuff_point_BASE = StackHeader_NUM*StackUnitSize;
	for(uint8_t i=0;i<StackUnitSize;i++)//单个单位的数据POP
	{
		DataStackOutput_t[i]=Offline_DATA_Stack[DATABuff_point_BASE+i];//数据导出
		Offline_DATA_Stack[DATABuff_point_BASE+i]=0;//出栈后数据清空
	}	
	return 0;
}
/**
 * @brief Sensor data push to stack
 * @param	uint8_t DeviceID_t//1~6
 * @retval void
 * @author ZCD1300 
 * @Time 2021年12月1日
*/
int8_t Sensor_PUSH_APP(uint8_t DeviceID_t)
{
	uint8_t PUSHSensor_t[StackUnitSize]={0};
	if(DeviceID_t>=1)
	{
		Load_Sensor_Data(Upload_DataTemp,DeviceID_t);
		{//修改数据
			PUSHSensor_t[0]='*';
			PUSHSensor_t[34]='#';
			PUSHSensor_t[35]=0;//头尾
			PUSHSensor_t[1]=DeviceID_t;//id
			
			PUSHSensor_t[2]=Upload_DataTemp[31];//温度4
			PUSHSensor_t[3]=Upload_DataTemp[32];
			PUSHSensor_t[4]=Upload_DataTemp[33];
			PUSHSensor_t[5]=Upload_DataTemp[34];
			
			PUSHSensor_t[6]=Upload_DataTemp[42];//湿度3
			PUSHSensor_t[7]=Upload_DataTemp[43];
			PUSHSensor_t[8]=Upload_DataTemp[44];
			
			PUSHSensor_t[9]=Upload_DataTemp[53];//光照5
			PUSHSensor_t[10]=Upload_DataTemp[54];
			PUSHSensor_t[11]=Upload_DataTemp[55];
			PUSHSensor_t[12]=Upload_DataTemp[56];
			PUSHSensor_t[13]=Upload_DataTemp[57];
			
			PUSHSensor_t[14]=Upload_DataTemp[65];//二氧化碳 4
			PUSHSensor_t[15]=Upload_DataTemp[66];
			PUSHSensor_t[16]=Upload_DataTemp[67];
			PUSHSensor_t[17]=Upload_DataTemp[68];
			
			PUSHSensor_t[18]=Upload_DataTemp[76];//氨气 4
			PUSHSensor_t[19]=Upload_DataTemp[77];
			PUSHSensor_t[20]=Upload_DataTemp[78];
			PUSHSensor_t[21]=Upload_DataTemp[79];
			
			PUSHSensor_t[22]=Upload_DataTemp[7];//年月日
			PUSHSensor_t[23]=Upload_DataTemp[8];
			PUSHSensor_t[24]=Upload_DataTemp[10];
			PUSHSensor_t[25]=Upload_DataTemp[11];
			PUSHSensor_t[26]=Upload_DataTemp[13];
			PUSHSensor_t[27]=Upload_DataTemp[14];
			
			PUSHSensor_t[28]=Upload_DataTemp[16];//时分秒
			PUSHSensor_t[29]=Upload_DataTemp[17];
			PUSHSensor_t[30]=Upload_DataTemp[19];
			PUSHSensor_t[31]=Upload_DataTemp[20];
			PUSHSensor_t[32]=Upload_DataTemp[22];
			PUSHSensor_t[33]=Upload_DataTemp[23];
		}
		//数据入栈
		if(Offline_Data_PUSH(PUSHSensor_t)==0)
		{
			return 0;
		}
		else
		{return 2;}//入栈失败，空间不足
	}
	else
	{return 1;}//输入ID错误
	
}
/**
 * @brief Sensor stack data pop to buff 
 * @param	char * OutputBUFF_t //OutputBUFF_t空间不能小于存储单元大小
 * @retval void
 * @author ZCD1300 
 * @Time 2021年12月1日
*/
uint8_t DeviceID_stack=0;
uint8_t StackTimeStamp_Flag=0;
int8_t Sensor_POP_APP(char * OutputBUFF_t)
{
	if(Offline_Data_POP(OutputBUFF_t)==0)
	{//正常POP
		if((OutputBUFF_t[0]=='*')&&(OutputBUFF_t[34]=='#'))
		{//数据正常
			DeviceID_stack=OutputBUFF_t[1];
			Upload_DataTemp[31]=OutputBUFF_t[2];//温度4
			Upload_DataTemp[32]=OutputBUFF_t[3];
			Upload_DataTemp[33]=OutputBUFF_t[4];
			Upload_DataTemp[34]=OutputBUFF_t[5];
													
			Upload_DataTemp[42]=OutputBUFF_t[6];//湿度3
			Upload_DataTemp[43]=OutputBUFF_t[7];
			Upload_DataTemp[44]=OutputBUFF_t[8];
													
			Upload_DataTemp[53]=OutputBUFF_t[9];//光照5
			Upload_DataTemp[54]=OutputBUFF_t[10];
			Upload_DataTemp[55]=OutputBUFF_t[11];
			Upload_DataTemp[56]=OutputBUFF_t[12];
			Upload_DataTemp[57]=OutputBUFF_t[13];
													
			Upload_DataTemp[65]=OutputBUFF_t[14];//二氧化碳 4
			Upload_DataTemp[66]=OutputBUFF_t[15];
			Upload_DataTemp[67]=OutputBUFF_t[16];
			Upload_DataTemp[68]=OutputBUFF_t[17];
																			
			Upload_DataTemp[76]=OutputBUFF_t[18];//氨气 4
			Upload_DataTemp[77]=OutputBUFF_t[19];
			Upload_DataTemp[78]=OutputBUFF_t[20];
			Upload_DataTemp[79]=OutputBUFF_t[21];
			
			Upload_DataTemp[5] ='2';
			Upload_DataTemp[6] ='0';
			Upload_DataTemp[7] =OutputBUFF_t[22];//年月日
			Upload_DataTemp[8] =OutputBUFF_t[23];
			Upload_DataTemp[10]=OutputBUFF_t[24];
			Upload_DataTemp[11]=OutputBUFF_t[25];
			Upload_DataTemp[13]=OutputBUFF_t[26];
			Upload_DataTemp[14]=OutputBUFF_t[27];

			Upload_DataTemp[16]=OutputBUFF_t[28];//时分秒
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
			return 2;//数据出错
		}
	}
	else
	{//栈内空
		return 1;
	}
	
}
/**
 * @brief Smart nest main function
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021年11月30日
*/
uint8_t CheckTCP_Return=0;
uint8_t CloseTCP_Return=0;
int8_t SmartNest_Main_Function(void)
{
	DebugPrint_uart_LineFeed("** Auto upload start.",200);
	CheckTCP_Return=Check_TCP_Connection();//这个函数同时检查了tcp连接并退出透传
	if(TCPLink_Alive==1)//连接仍然存在
	{
		Start_Transp_Send();//重新使能并开启透传	
	}

	if(Registed_User==0)
	{//第一次初始化，host应该自己进入注册host流程
		Reg_Host_App();//这里后面看下如果注册超时回怎么样。或者代码设置成永不超时；
		
		{//注册所有Device
			Reg_Device_ALL();
		}
	}
	//到这里应该已经完成host注册和device注册；
	{//数据获取； 读取全部的device数据，
		//ZigbeeRead_ALL_Sensor();
		zigbee_Read_Sensor(2,3,DeviceSNList);
	}

	{//上传数据部分：上传之前应该检查是否建立了连接，并且发送完能主动断开连接。		
		//检查连接
		if(TCPLink_Alive!=1)
		{//连接断开；重新建立连接
			Wifi_ExitTransp_Send();				
			Wifi_CLOSE_Connection(0);
			Connect_Server_Smatnest();//尝试连接到服务器
			if(TCPLink_Alive==1)
			{
				Start_Transp_Send();//使能并开启透传
			}
			else
			{
				return 2;
			}
			
		}
		if(TCPLink_Alive==1)//到此应该已经建立到服务器连接
		{//上传数据
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
 * @brief Data upload //数据上传函数，用于将读取的函数上传；同时管理离线堆栈里的数据存取并上传
 * @param	void
 * @retval int8_t return 
 * @author ZCD1300 
 * @Time 2021年11月30日
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
		{//数据堆栈中存在历史未上传数据，上传历史数据
			Sensor_POP_APP(HistoryDataPOP_BUFF);//同时已经将数据加载到字符串了
			
			Load_DeviceSN(HistoryDataPOP_BUFF[1]);	//对应SN
			
			uint8_t Stack_uploadRetryCNT=0;
	
			while(Upload_DeviceData())	
			{//上传数据，上传成功跳出循环,否则5s重试一次,最大重复10次，超时暂存到栈里
				Delay(2000);		
				if(Stack_uploadRetryCNT>=3)//临时改成3次间隔2秒
				{
					Stack_uploadRetryCNT=0;
					Stack_uploadRetryFlag=1;//栈数据上传失败标志
					break;	
				}
				Stack_uploadRetryCNT++;
			}		
			Stack_uploadRetryCNT=0;

			if(Stack_uploadRetryFlag!=0)
			{//栈内数据上传再次失败
				Offline_Data_PUSH(HistoryDataPOP_BUFF);//重新压入栈中
				break;
			}
			else
			{
				Stack_uploadRetryFlag=0;
			}
		}	
	}

	
	if(1)
	{//离线数据堆栈中没有数据，直接上传数据。
		DebugPrint_uart("\r\n** New data upload.",200);
		uint8_t DataUploadFail_Flag=0;//数据上传失败标志	
		uint8_t WiFiUploadFail_Flag=0;//WiFi上传数据超时
		for(uint8_t i=1;i<=DeviceNUM_MAX;i++)//DeviceNUM_MAX
		{
			if(DeviceOnline_Falg[i-1]!=1)//检测对应的终端是否注册
			{ continue;	}	//终端未注册	
			Load_DeviceSN(i);									//加载device SN
			Load_Sensor_Data(Upload_DataTemp,i);//加载对应device SN的数据到字符串
			uint8_t uploadRetryCNT=0;	
			while(Upload_DeviceData())	
			{//上传数据，上传成功跳出循环,否则重试,最大重复4次，超时暂存到栈里
				Delay(2000);		
				if(uploadRetryCNT>=3)//临时改成4次间隔2秒
				{
					uploadRetryCNT=0;
					WiFiUploadFail_Flag=1;//wifi数据上传失败标志
					
					break;	
				}
				uploadRetryCNT++;
			}
			
			if(WiFiUploadFail_Flag==1)//WiFi上传数据超时后；
			{//这部分是NBIOT模块做备份上传用
							
				nbiot_build_connect(SmartNest_Domain_NBIOT,SmartNest_Domain_NBIOTLEN);
				Delay(500);			
				if(NBIOT_TCPLink_Alive==1&&NBIOT_Connected_to_Server_Flag==1)
				{
					if(Upload_DeviceData_NBIOT()==0)
					{//上传成功
						DataUploadFail_Flag=0;
					}
				}
				else
				{
					DataUploadFail_Flag=1;//两种方式均上传失败
				}				

			}
			if(DataUploadFail_Flag==1)
			{//上传失败
				if(Sensor_PUSH_APP(i)==0)//数据入栈暂存，继续上传进程
				{//数据入栈成功
					
					zigbeeClear_StructData(i);//将现有结构体数据清空			
				}
				else
				{//入栈失败，栈满
					Stack_FULL_Flag=1;
				}
			}		
			else
			{//上传完成
				DataUploadFail_Flag=0;
				zigbeeClear_StructData(i);//将现有结构体数据清空			
			}
		}

		DebugPrint_uart_LineFeed("** Auto upload complete.",200);
	}
	if(Stack_FULL_Flag==1)
	{
		return 2;//离线数据栈满（或者ID错误的入栈失败）
	}
	return 0;
}



/**
 * @brief Connection_ControlAPP
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021年12月5日
*/
//这个函数还要能主动控制到服务器的连接，上传数据完成后维持一段时间连接，超时自动断开
uint16_t TCPLink_AliveTime_CNT=0;
uint8_t Connection_ControlAPP(void)
{//这一部分用来管理与服务器连接的控制
	if(TCPLink_Alive==1)
	{//连接到服务器
		TCPLink_AliveTime_CNT++;
		if(Wifi_Transparent_SendEntered_Flag!=1)
		{
			//Start_Transp_Send();
		}
	}
	else
	{
		
		return 1;}//这里正常运行不会触发
	if(TCPLink_AliveTime_CNT>12000)//2分钟保持时间
	{
		TCPLink_AliveTime_CNT=0;
		if(Debug_enable_flag!=1)
		{//debug模式使能的时候不主动关闭连接
			Wifi_ExitTransp_Send();	
			CloseTCP_Return=Wifi_CLOSE_Connection(0);//关闭全部连接
		}
	}
}	
/**
 * @brief LED Display Ctrl APP
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021年12月21日
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


