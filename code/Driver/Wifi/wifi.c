#include "wifi.h"
#include "http_json.h"
#include "SmartNest_server.h"
#include "Flash_Operation.h"

uint8_t UDP_Target[]="\"UDP\",\"192.168.31.255\",8080,1112,0";
uint8_t TCP_Target[]="\"TCP\",\"192.168.31.194\",8080,0";
uint8_t SmartNest_Domain[]="\"TCP\",\"smartnestxdu.top\",80";
uint8_t UDP_TargetLen=sizeof(UDP_Target);
uint8_t TCP_TargetLen=sizeof(TCP_Target);
uint8_t SmartNest_DomainLEN=sizeof(SmartNest_Domain);


 /**
 * @brief  ESP8266建立TCP UDP连接
 * @param	 Type，连接类型 0 UDP    1 TCP
 * @retval	void
 * @author ZCD
 * @Time 2021年3月28日  
*/
void wifi_build_connect(uint8_t Type,uint8_t *TargetIP,uint8_t len)
{	
	if(Type==0)
	{
		Wifi_CMD_SEND(WifiBuildConnect,TargetIP,sizeof(WifiBuildConnect),len-1);
	}
	else if(Type==1)
	{
		Wifi_CMD_SEND(WifiBuildConnect,TargetIP,sizeof(WifiBuildConnect),len-1);		
	}
	else
	{}
}
/**
 * @brief  Switch Wifi AP	
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月13日
*/
uint8_t Switched_AP=0;
uint8_t Switch_Wifi_AP(void)
{

	char SSID_input[50]={0};	
	char Password[50]={0};	
	
	DebugPrint_uart("** Connection AP timeout.\r\n** Please input available WiFi hotspot SSID and password.\r\n",0x200);
	DebugPrint_uart("** >>SSID:",0x200);

	while(!(StringSearch_Assign("|",Rx_Buff2,1,100)&&(StringSecanPointer_Assign_Buff>1)))//检查是否输入 “|”
	{	Delay(20);}

	strcpy(SSID_input,Rx_Buff2);//暂存SSID
	Clear_Buff(Rx_Buff2,StringSecanPointer_Assign_Buff+1,BuffMAX2);
	DebugPrint_uart(SSID_input,0x200);		
		

	DebugPrint_uart("\r\n** >>Password:\r\n",0x200);
	while(!(StringSearch_Assign("|",Rx_Buff2,1,100)&&(StringSecanPointer_Assign_Buff>7)))//检查是否输入 “|”
	{	Delay(20);}

	strcpy(Password,Rx_Buff2);//暂存Password
	Clear_Buff(Rx_Buff2,StringSecanPointer_Assign_Buff+1,BuffMAX2);	

	Wifi_Connect_AP(SSID_input,Password);
	Switched_AP=1;
	return 0;

}

/**
 * @brief  Skip_WifiRST_Check Init	//跳过wifi模块初始化和连接检查,仅用在测试过程中,正式release版本不能存在
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月11日
*/
uint8_t WifiInitOKflag=0;
uint8_t WifiRST_OK_Flag=0;
uint8_t Wifi_Connect_AP_Flag=0;
uint8_t GotIP_Flag=0;	
uint8_t Connected_to_Server_Flag=0;
uint8_t Wifi_Transparent_Send_Flag=0;
uint8_t Wifi_Transparent_SendEntered_Flag=0;
void Skip_WifiRST_Check(void)
{
	WifiInitOKflag=1;
	WifiRST_OK_Flag=1;
	Wifi_Connect_AP_Flag=1;
	GotIP_Flag=1;
	Connected_to_Server_Flag=1;
	TCPLink_Alive=1;
	
	Switched_AP=0;
}

/**
 * @brief  Wifi Init//初始化wifi模块,并完成模块初始化 Wifi连接 网络连接等检查(esp8266)
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月11日
*/
void Wifi_Init(void)
{
	uint8_t RST_wait_cnt=0;
	RST_Retry:	
	{//跳过RST和连接检查,Debug专用,去掉注释的函数就跳过Wifi初始化过程
		//Skip_WifiRST_Check();
	}	

	if(WifiRST_OK_Flag==0)//每次上电RST wifi模块,此处为等待wifi复位完成
	{	
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);		
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);	
		Delay(400);
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);	
		while(!(StringSearch("\r\ninvalid",9,100)||StringSearch("\r\nready",7,100)))
		{
			RST_wait_cnt++;
			Delay(50);
			if((RST_wait_cnt%20)==1)
			{
				DebugPrint_uart("** Wait for Wifi module RST.\r\n",0x200);		
			}
			if(RST_wait_cnt>=100)
			{
				RST_wait_cnt=0;
				DebugPrint_uart("** WiFi module RST fail\r\n",0x200);
				goto RST_Retry;
			}
		}
		WifiRST_OK_Flag=1;		
		DebugPrint_uart("** Wifi module RST OK.\r\n",0x200);
		Wifi_CheckSwitch_Workstate(3);// 每次重启检查是否更换过WiFi模块(不验证mac)	
		LCD_Startup_ProgressBar(10);
	}
	
	if(Wifi_Connect_AP_Flag==0)//检查是否连接到Wifi AP,是继续,是否进入AP登录流程
	{
		uint8_t Connect_AP_Print_Cnt=0;//串口debug打印计数
		uint8_t Join_AP_Overtime_Cnt=0;//连接AP超时计数，超时时间=delay*Print_Cnt*Overtime_Cnt
		  
		while(!StringSearch("WIFI CONNE",10,100))
		{//没连上AP
			
			Connect_AP_Print_Cnt++;
			if(Join_AP_Overtime_Cnt>=20)//10s未连接上wifi，认为AP无效，进入AP切换模式
			{
				if(GPIO_Read(GPIOF,GPIO_PIN_0)!=RESET)//这里可以检测PF0，如果按下，则使用串口配网
				{
					Distribution_via_app();
				}
				else
				{
					Switch_Wifi_AP();
				}
				
				Join_AP_Overtime_Cnt=0;
			}			
			if((Connect_AP_Print_Cnt/50)==1)
			{
				DebugPrint_uart("** No Wifi AP connected !!!\r\n",0x200);		
				Join_AP_Overtime_Cnt++;
				Connect_AP_Print_Cnt=0;
			}
			Delay(20);
		}
		Wifi_Connect_AP_Flag=1;
		DebugPrint_uart("** Wifi connected.\r\n",0x200);
		LCD_Startup_ProgressBar(20);
	}
	
	if(GotIP_Flag==0)		//连接到AP后,检查是否获取到 IP
	{
		uint8_t GOT_IP_Print_Cnt=0;		
		while(!StringSearch("GOT IP",6,100))
		{//没有获取到有效IP,如果长时间没有IP,建议更换Wifi

			Delay(20);
			if(GOT_IP_Print_Cnt/30==1)
			{
				DebugPrint_uart("** No valid IP.\r\n",0x200);
				GOT_IP_Print_Cnt=0;
			}			
		}
		
		GotIP_Flag=1;
		DebugPrint_uart("** Got IP.\r\n",0x200);
	}
	if(Switched_AP==1)//如果切换过AP
	{//要重新获得IP，且要等返回OK，模块才能继续(分Wifi固件，不同固件可能不需要等返回OK)
		uint8_t SwitchedAP_Retry_WaitOK_cnt=0;//等待超时，不再等待计数/
		while(!StringSearch("\r\nOK",4,64))
		{
			if(SwitchedAP_Retry_WaitOK_cnt<100)//等待10*100ms
			{
				Delay(10);
				SwitchedAP_Retry_WaitOK_cnt++;
			}
			else
			{
				SwitchedAP_Retry_WaitOK_cnt=0;
				break;//不再等待
			}
		}	
		GotIP_Flag=1;
		Switched_AP=0;
	}
		LCD_Startup_ProgressBar(25);
	//如果执行到这里,说明已经完成了WiFi模块的初始化,并连接上AP获取到了IP;局域网链路完成
	if(GotIP_Flag==1)
	{	
	{//读取IP、MAC、AP
		Wifi_Read_WorkState();
		Wifi_Read_IP_MAC();
		Wifi_Check_AP_Name();
		
		DebugPrint_uart("** Native IP:",100);
		DebugPrint_uart(Wifi_STAIP,0x200);
		DebugPrint_uart("\r\n",100);
		
		//DebugPrint_uart_LineFeed(Wifi_MAC,0x200);			
		//加载MAC
		Load_SN(Wifi_MAC,12);
		//DebugPrint_uart_LineFeed(Host_sn.Str,0x200);	
	}
	{//连接到服务器
		//wifi_build_connect(0,UDP_Target,UDP_TargetLen);//UDP连接
		//wifi_build_connect(1,TCP_Target,TCP_TargetLen);//TCP连接
		wifi_build_connect(1,SmartNest_Domain,SmartNest_DomainLEN);//连接到服务器
		uint8_t Connected_To_Server_Print_Cnt=0;
		while(!StringSearch("NECT\r\n\r\nOK",10,150))//等待建立到服务器连接完成
		{//未建立连接
			Delay(100);
			if((Connected_To_Server_Print_Cnt/3)==1)
			{
				Connected_To_Server_Print_Cnt=0;
				DebugPrint_uart("** Connecting to server.\r\n",0x200);
			}
		}
		DebugPrint_uart("** Connected to server.\r\n",0x200);
		Connected_to_Server_Flag=1;
		TCPLink_Alive=1;
		LCD_Startup_ProgressBar(30);
	}		
	{//使能透传
		Wifi_CMD_SEND(WifiSendMODE,"1",sizeof(WifiSendMODE),1);//使能透传
		uint8_t ENable_Transparent_Send_Print_Cnt=0;	
		while(!StringSearch("CIPMODE=1",9,100))//等待使能透传模式完成
		{//未使能透传模式
			Delay(20);
			ENable_Transparent_Send_Print_Cnt++;
			if((ENable_Transparent_Send_Print_Cnt/30)==1)
			{
				ENable_Transparent_Send_Print_Cnt=0;
				DebugPrint_uart("** Enabling transparent send....\r\n",0x200);
			}
		}
		Wifi_Transparent_Send_Flag=1;
		DebugPrint_uart("** Enabled transparent send.\r\n",0x200);
	}		
	LCD_Startup_ProgressBar(35);
	{//等待进入透传模式
		Wifi_CMD_SEND(WifiSEND,0,sizeof(WifiSEND),0);
		uint8_t Enter_Transparent_Send_Print_Cnt=0;
		while(!StringSearch("OK\r\n\r\n>",7,64))//等待进入透传模式
		{//没有进入透传模式
			Delay(20);
			Enter_Transparent_Send_Print_Cnt++;
			if((Enter_Transparent_Send_Print_Cnt/30)==1)
			{
				Enter_Transparent_Send_Print_Cnt=0;
				DebugPrint_uart("** Entering transparent send....\r\n",0x200);
			}			
		}
		Wifi_Transparent_SendEntered_Flag=1;
		WifiInitOKflag=1;
		DebugPrint_uart("** Entered transparent send.\r\n",0x200);
	}
	}
	DebugPrint_uart("||----WiFi module initialization completed----||\r\n\r\n",0x200);//模块启动完成	
	
}	

/*
 *
 * @brief  获取网络时间
 * @param	 void
 * @retval	void
 * @author ZCD
 * @Time 2021年1月27日
*/
void Get_NETTime(void)
{
//	Wifi_CMD_SEND(WifiBuildConnect,"\"TCP\",\"smartnestxdu.top\",80",sizeof(WifiBuildConnect),27);
//	Delay(500);
//	Wifi_CMD_SEND(WifiSendMODE,"1",sizeof(WifiSendMODE),1);
//	Delay(500);
//	Wifi_CMD_SEND(WifiSEND,0,sizeof(WifiSEND),0);
//	Delay(500);
	
	//uint8_t cha[]="GET http://quan.suning.com/getSysTime.do HTTP/1.1\r\nHost: quan.suning.com\r\n\r\n\r\n";
	uint8_t cha2[]="GET /api/test/time HTTP/1.1\r\nHost: smartnestxdu.top\r\n\r\n\r\n";
	UART_Send(&huart1,cha2,sizeof(cha2)-1,0x200);
	Delay(500);
	//UART_Send(&huart1,"+++",3,0x100);	
}
/**
 * @brief Print_Time	//以时间以指定格式打印
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月14日
*/
uint8_t Print_Time(uint16_t year_t,uint16_t month_t,uint16_t day_t,uint16_t hour_t,\
uint16_t min_t,uint16_t sec_t,uint16_t weekady_t)
{
	char time_str_t[35]="0000/00/00 00:00:00 ***";
	time_str_t[0]=year_t/1000+48;
	time_str_t[1]=(year_t%1000)/100+48;
	time_str_t[2]=(year_t%100)/10+48;
	time_str_t[3]=(year_t%10)+48;
	
	time_str_t[5]=month_t/10+48;
	time_str_t[6]=month_t%10+48;	
	
	time_str_t[8]=day_t/10+48;
	time_str_t[9]=day_t%10+48;	
	
	uint8_t hour_offset=hour_t+8;
	if(hour_offset>=24)
	{
		hour_offset=hour_offset-24;
		time_str_t[9]++;
	}
	
	time_str_t[11]=hour_offset/10+48;
	time_str_t[12]=hour_offset%10+48;		
	
	time_str_t[14]=min_t/10+48;
	time_str_t[15]=min_t%10+48;	

	time_str_t[17]=sec_t/10+48;
	time_str_t[18]=sec_t%10+48;
	
	switch (weekady_t)
	{
		case 0:
		{
			time_str_t[20]='S';
			time_str_t[20+1]='u';
			time_str_t[20+2]='n';				
			break;         
		}                
		case 1:          
		{                
			time_str_t[20]='M';
			time_str_t[20+1]='o';
			time_str_t[20+2]='n';				
			break;         
		}                
		case 2:          
		{                
			time_str_t[20]='T';
			time_str_t[20+1]='u';
			time_str_t[20+2]='e';				
			break;         
		}                
		case 3:          
		{                
			time_str_t[20]='W';
			time_str_t[20+1]='e';
			time_str_t[20+2]='d';				
			break;         
		}                
		case 4:          
		{                
			time_str_t[20]='T';
			time_str_t[20+1]='h';
			time_str_t[20+2]='u';				
			break;         
		}                
		case 5:          
		{                
			time_str_t[20]='F';
			time_str_t[20+1]='r';
			time_str_t[20+2]='i';	
			break;         
		}                
		case 6:          
		{                
			time_str_t[20]='S';
			time_str_t[20+1]='a';
			time_str_t[20+2]='t';		
			break;
		}		
		default:
		{
			return 1;
			break;}
	}
	DebugPrint_uart("\r\n** ",0x200);
	DebugPrint_uart(time_str_t,0x200);
	DebugPrint_uart(" **\r\n",0x200);	
	return 0;
}
/**
 * @brief Start transparent send// 开始透明传输
 * @param	void 
 * @retval int8_t return 0 表示正常进入透传发送//1 表示进入失败
 * @author ZCD1300
 * @Time 2021年11月19日
*/
int8_t Start_Transp_Send(void)
{
	uint8_t ENable_Transparent_Send_RetryCNT=0;
	{//使能透传
		Wifi_CMD_SEND(WifiSendMODE,"1",sizeof(WifiSendMODE),1);//使能透传
		uint8_t ENable_Transparent_Send_Print_Cnt=0;	
		
		while(!StringSearch("CIPMODE=1",9,100))//等待使能透传模式完成
		{//未使能透传模式
			Delay(200);
			ENable_Transparent_Send_Print_Cnt++;
			if((ENable_Transparent_Send_Print_Cnt/3)==1)
			{
				ENable_Transparent_Send_Print_Cnt=0;
				ENable_Transparent_Send_RetryCNT++;
				DebugPrint_uart("** Enabling transparent send....\r\n",0x200);
			}
			if(ENable_Transparent_Send_RetryCNT>10)
			{
				ENable_Transparent_Send_RetryCNT=1;
				break;
			}
		}
		if(ENable_Transparent_Send_RetryCNT==1)
		{
			Wifi_Transparent_Send_Flag=0;
			DebugPrint_uart("** Enable transparent failed.\r\n",0x200);
		}
		else
		{
			Wifi_Transparent_Send_Flag=1;
			DebugPrint_uart("** Enabled transparent send.\r\n",0x200);
		}
		
	}		
	{//等待进入透传模式
		Wifi_CMD_SEND(WifiSEND,0,sizeof(WifiSEND),0);
		uint8_t Enter_Transparent_Send_Print_Cnt=0;
		uint8_t Enter_Transparent_Send_RetryCNT=0;
		while(!StringSearch("OK\r\n\r\n>",7,64))//等待进入透传模式
		{//没有进入透传模式
			Delay(200);
			Enter_Transparent_Send_Print_Cnt++;
			if((Enter_Transparent_Send_Print_Cnt/3)==1)
			{
				Enter_Transparent_Send_Print_Cnt=0;
				DebugPrint_uart("** Entering transparent send....\r\n",0x200);
				Enter_Transparent_Send_RetryCNT++;
			}	
			if(Enter_Transparent_Send_RetryCNT>10)
			{
				Enter_Transparent_Send_RetryCNT=1;
				break;
			}
		}
		Wifi_Transparent_SendEntered_Flag=1;
		WifiInitOKflag=1;
		if((Enter_Transparent_Send_RetryCNT==1)||(ENable_Transparent_Send_RetryCNT==1))
		{
			WifiInitOKflag=0;
			Wifi_Transparent_SendEntered_Flag=0;
			DebugPrint_uart("** Enter transparent failed.\r\n",0x200);
		}
		else
		{
			DebugPrint_uart("** Entered transparent send.\r\n",0x200);
		}
		
	}	
	if((Wifi_Transparent_SendEntered_Flag==1)&&(Wifi_Transparent_Send_Flag==1))
	{
		
		return 0;//成功进入透传
	}
	else
	{
		return 1;//进入透传发送失败
	}
}


/**
 * @brief Check and automatically switch the working mode//检查并自动切换wifi模块工作模式
 * @param	uint8_t DefaultMode //默认切换到的模式
 * @retval int8_t return 0 与指定模式相等,不切换; 1 切换过模式
 * @author ZCD1300
 * @Time 2021年11月19日
*/
//如果切换过到仅AP模式,在切换到有sta模式,sta不会自动重连WiFi,重新复位就好
uint8_t Wifi_CheckSwitch_Workstate(uint8_t DefaultMode_t)
{
	Wifi_Read_WorkState();
	if(Wifi_module_State!=DefaultMode_t)//判断是否为指定模式
	{
		Wifi_ModeSet(DefaultMode_t);
		return 1;
	}
	
	return 0;
}



