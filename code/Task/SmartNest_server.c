#include "SmartNest_server.h"


//-----------------------GET--------------------------//
char Reg_new[]="GET /api/reg/user?name=tester1&password=123456789 HTTP/1.1\r\n";
char Login[]="GET /api/login/user?name=tester1&password=123456789 HTTP/1.1\r\n";
char Query_Host[]="GET /api/query/host?num=-1 HTTP/1.1\r\n";//算了主机查询接口在主机上用不到


//----------------------POST--------------------------//

char Post_test[]="POST /api/upload/test HTTP/1.1\r\n";
char Post_RegHost[]="POST /api/reg/host HTTP/1.1\r\n";
char Post_RegDevice[]="POST /api/reg/devices HTTP/1.1\r\n";
char Post_UploadData_SN[]="POST /api/upload/devices/data/sn HTTP/1.1\r\n";
char Post_PUSH[]="POST /api/msg/push HTTP/1.1\r\n";

//----------------------------------------------------//
/**
* @brief  Send_to_Server		//send,发送函数;wifi模块使用串口1
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月10日
*/
int8_t Send_to_Server(uint8_t *Send_temp)
{
	uint16_t Send_len_t=strlen(Send_temp);
	UART_Send(&huart1,Send_temp,Send_len_t,0x400);
	return 0;
}
int8_t Send_to_Server_NBIOT(uint8_t *Send_temp)
{
	uint16_t Send_len_t=strlen(Send_temp);
	UART_Send(&huart4,Send_temp,Send_len_t,0x400);
	return 0;
}
/**
* @brief  Post_to_Server		//post send,发送函数执行后会清空body和head的缓存区
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月10日
*/
//uint16_t strlen_len_test=0;
char Server_SendBuff[Server_SEND_BuffLen_MAX]={0};
int8_t Post_to_Server(void)
{
//	JsonPack("post_test");		
//	Body_head_Make(Post_test,sizeof(Post_test),1,strlen(Json_packed_BUFF));
	strcpy(Server_SendBuff,Head_Send_Buff);
	strcpy(Server_SendBuff+strlen(Head_Send_Buff),Json_packed_BUFF);
	if(Display_packBuff_Flag==1)
	{
		DebugPrint_uart("\r\nHeader and JSON pack buff:",0x200);
		DebugPrint_uart_LineFeed(Server_SendBuff,0x400);
	}
	
	UART_Send(&huart1,Server_SendBuff,strlen(Server_SendBuff),0x400);//此处的警告是char与uint8_t,实际无区别
	Clear_Buff(Server_SendBuff,strlen(Server_SendBuff),Server_SEND_BuffLen_MAX);
	Clear_Buff(Head_Send_Buff,Head_Send_BuffPoint,Head_Len_MAX);
	Clear_Buff(Json_packed_BUFF,strlen(Json_packed_BUFF)+1,Json_packed_BUFF_MAX);

	return 0;
}
int8_t Post_to_Server_NBIOT(void)
{
	strcpy(Server_SendBuff,Head_Send_Buff);
	strcpy(Server_SendBuff+strlen(Head_Send_Buff),Json_packed_BUFF);
	if(Display_packBuff_Flag==1)
	{
		DebugPrint_uart("\r\nHeader and JSON pack buff(NBIOT):",0x200);
		DebugPrint_uart_LineFeed(Server_SendBuff,0x400);
	}
	NBIOT_SEND_PUSH(0,strlen(Server_SendBuff),Server_SendBuff);
	//UART_Send(&huart4,Server_SendBuff,strlen(Server_SendBuff),0x400);//此处的警告是char与uint8_t,实际无区别
	Clear_Buff(Server_SendBuff,strlen(Server_SendBuff),Server_SEND_BuffLen_MAX);
	Clear_Buff(Head_Send_Buff,Head_Send_BuffPoint,Head_Len_MAX);
	Clear_Buff(Json_packed_BUFF,strlen(Json_packed_BUFF)+1,Json_packed_BUFF_MAX);

	return 0;
}
/**
 * @brief  Register_Host 	//注册主机接口
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月11日
*/
int8_t Register_Host(void)
{
	
	Pack_return=JsonPack("reg_host");		
	Body_head_Make(Post_RegHost,sizeof(Post_RegHost),1,strlen(Json_packed_BUFF));

	Post_to_Server();
	
	UART1_Refresh_Flag=0;//检测串口触发前要置位
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//最大超时时间5s
	Body_Process();
	if(BodyHead_pro.State_code==200)//检查服务器是否为正常访问
	{//是
		Unpack_return=JsonUnpack("reg_host");
		if(Unpack_return==0)//注册成功
		{
			json_unpack_copy(Host_sn.Str,Host_sn.Str_p);	
			json_unpack_copy(Host_type.Str,Host_type.Str_p);		
			DebugPrint_uart("\r\n** Host registration succeeded.",0x200);
		}
		else if(Unpack_return == -4)//host_sn 已经被注册
		{
			DebugPrint_uart("\r\n** This host is already registered or SN error.",0x200);		
			Json_Delete_Enable_Flag=1;
			json_unpack_copy(0,0);
		}
		else if(Unpack_return == -8)//服务器json接收不全或者解析失败
		{
			DebugPrint_uart("\r\n** Server JSON parsing failed.",0x200);
			Json_Delete_Enable_Flag=1;
			json_unpack_copy(0,0);
		}
		else
		{
			DebugPrint_uart("\r\n** Unknown state error.",0x200);
		}		
	}
	else
	{
		DebugPrint_uart_LineFeed("** Connection error with server.",200);
	}
	
	return 0;

}
/**
 * @brief  Login User  	//Login接口,登录会自动保存新的utoken
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月14日
*/
int8_t Login_Uesr(char *UserName_t,char *Password_t)
{
	uint8_t Username_Len=0;
	uint8_t Password_Len=0;
	char Http_End_t[]=" HTTP/1.1\r\nHost: smartnestxdu.top\r\n\r\n\r\n";
	Username_Len=strlen(UserName_t);
	Password_Len=strlen(Password_t);
	char Get_Send_Buff[100]={0};
	strcpy(Get_Send_Buff,Login);
	strcpy(Get_Send_Buff+25,UserName_t);
	strcpy(Get_Send_Buff+25+Username_Len,"&password=");
	strcpy(Get_Send_Buff+35+Username_Len,Password_t);	
	strcpy(Get_Send_Buff+35+Username_Len+Password_Len,Http_End_t);
	
	//DebugPrint_uart_LineFeed(Get_Send_Buff,0x200);
	Send_to_Server(Get_Send_Buff);//发送get请求	
	
	UART1_Refresh_Flag=0;//检测串口触发前要置位
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//最大超时时间5s//临时没处理如果超时未触发
	
	Unpack_return=JsonUnpack("Login");
	NetTime_Refesh();
	if(BodyHead_pro.State_code==200)//检查服务器是否为正常访问
	{//是
		if(Unpack_return==0)//登录成功
		{
			json_unpack_copy(userName.Str,userName.Str_p);	
			//json_unpack_copy(Host_type.Str,Host_type.Str_p);		
			BodyHead_pro.utoken_Live=1;
			
			Flash_ValidMark_Equilibrium(BodyHead_pro.utoken_Live);
			return 0;		
		}	
		else if(Unpack_return==-4)//登录失败,返回head与body的token不一致（严重错误）
		{		
			return 2;
		}
		else if(Unpack_return==-3)//登录失败，用户或密码错误
		{
			
			json_unpack_copy(userName.Str,userName.Str_p);	
			json_unpack_copy(Password.Str,Password.Str_p);	//返回的错误用户名、密码
			if(Registed_User==0)
			{//首次登录
				return 1;
			}
			else
			{//Flash 写入过账户、密码，但是目前密码修改等导致不正确
				Changed_UserInfo=1;
				
				return 5;
			}
				
			
		}
		else	
		{
			DebugPrint_uart("\r\n** Unknown state error.",0x200);
			return 3;//未知的错误
		}	
	}
	else//服务器连接、返回错误
	{
		DebugPrint_uart_LineFeed("** Connection error with server.",200);
		return 4;
	}

}
/**
 * @brief Reg_Devices 
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021年11月18日
*/
int8_t Register_Device(void)
{
	Pack_return=JsonPack("reg_device");		
	Body_head_Make(Post_RegDevice,sizeof(Post_RegDevice),1,strlen(Json_packed_BUFF));

	Post_to_Server();
	
	UART1_Refresh_Flag=0;//检测串口触发前要置位
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//最大超时时间5s
	Body_Process();
	if(BodyHead_pro.State_code==200)//检查服务器是否为正常访问
	{//是
		Unpack_return=JsonUnpack("reg_device");
		if(Unpack_return==0)//注册成功
		{
			json_unpack_copy(Device_Suc_SN.Str,Device_Suc_SN.Str_p);	
			json_unpack_copy(Device_Err_SN.Str,Device_Err_SN.Str_p);	
			
			if(deviceReg_ErrCNT==0)
			{//全部注册成功
				DebugPrint_uart("\r\n** All devices are registered successfully.",0x200);				
			}
			else if(deviceReg_SucCNT==0)
			{//全部注册失败
				DebugPrint_uart("\r\n** Failed to register all devices.",0x200);			
			}
			else	
			{//部分注册成功
				DebugPrint_uart("\r\n** Device partially registered successfully.",0x200);
			}

		}
		else if(Unpack_return == -4)//host未注册
		{
			DebugPrint_uart("\r\n** This host was not registered.",0x200);		
		}
		else if(Unpack_return == -8)//服务器json接收不全或者解析失败
		{
			DebugPrint_uart("\r\n** Server JSON parsing failed.",0x200);
		}
		else
		{
			DebugPrint_uart("\r\n** Unknown state error.",0x200);
		}
	}
	else
	{
		DebugPrint_uart_LineFeed("** Connection error with server.",200);
	}
	
	return 0;
	
	
	
}
/**
 * @brief Upload_DeviceData 
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021年11月18日
*/
int Upload_pressTest_CNT=0;
char UploadData_buff[Json_packed_BUFF_MAX-100]={0};
char Upload_DataTemp[83]="Time=2021/12/31/00:00:00, TEMP= 00.0 ,RH= 000% ,Eav= 00000 ,CO2= 0000 ,NH3= 0000,\n";
char Upload_DataSend[73]="Time=2021/12/31/00:00:00,TEMP=00.0,RH=000%,Eav=00000,CO2=0000,NH3=0000,\n";//这个函数是因为前端处理空格影响性能，所以实际发送数据是这个数组；本质是上面数组的一层加工
//char Upload_DataTemp[]="abc1\nabc1\nabc1\nabc1\nabc1\nabc1\nabc1\nabc1\nabc1\nabc1\n";
void Upload_Data_tempProcess(void)//这个函数用来处理两个上传字符串
{
	for(uint8_t i=0;i<25;i++)//time
	{
		Upload_DataSend[i]=Upload_DataTemp[i];
	}
	for(uint8_t i=0;i<25;i++)//temperture
	{
		Upload_DataSend[i]=Upload_DataTemp[i];
	}
	Upload_DataSend[30]=Upload_DataTemp[32];//temper
	Upload_DataSend[30+1]=Upload_DataTemp[32+1];
	Upload_DataSend[30+3]=Upload_DataTemp[32+3];
	
	Upload_DataSend[38]=Upload_DataTemp[42];//RH
	Upload_DataSend[38+1]=Upload_DataTemp[42+1];
	Upload_DataSend[38+2]=Upload_DataTemp[42+2];
	
	Upload_DataSend[47]=Upload_DataTemp[53];//Eav
	Upload_DataSend[47+1]=Upload_DataTemp[53+1];
	Upload_DataSend[47+2]=Upload_DataTemp[53+2];
	Upload_DataSend[47+3]=Upload_DataTemp[53+3];
	Upload_DataSend[47+4]=Upload_DataTemp[53+4];
	
	Upload_DataSend[57]=Upload_DataTemp[65];//CO2
	Upload_DataSend[57+1]=Upload_DataTemp[65+1];
	Upload_DataSend[57+2]=Upload_DataTemp[65+2];
	Upload_DataSend[57+3]=Upload_DataTemp[65+3];

	Upload_DataSend[66]=Upload_DataTemp[76];//NH3
	Upload_DataSend[66+1]=Upload_DataTemp[76+1];
	Upload_DataSend[66+2]=Upload_DataTemp[76+2];
	Upload_DataSend[66+3]=Upload_DataTemp[76+3];	
	Upload_DataSend[72]=0;
}
int8_t Upload_DeviceData(void)
{
	if(StackTimeStamp_Flag==1)
	{//使用栈内保存的时间戳
		StackTimeStamp_Flag=0;
	}
	else
	{
		Load_TimeStamp(Upload_DataTemp);//打时间戳	
	}
	Upload_Data_tempProcess();
	strcpy(UploadData_buff,Upload_DataSend);
	
	Pack_return=JsonPack("upload_data");	
	Body_head_Make(Post_UploadData_SN,sizeof(Post_UploadData_SN),1,strlen(Json_packed_BUFF));
	Post_to_Server();
	
	UART1_Refresh_Flag=0;//检测串口触发前要置位
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//最大超时时间5s
	Body_Process();
	if(BodyHead_pro.State_code==200)//检查服务器是否为正常访问
	{//是
		Unpack_return=JsonUnpack("upload_data");
		if(Unpack_return==0)//数据上传成功
		{
			Upload_pressTest_CNT++;
			UART_Print_Number(Upload_pressTest_CNT);
			return 0;//数据上传成功
		}
		else if(Unpack_return == -4)//host或者device未注册
		{
			DebugPrint_uart("\r\n** This host or device was not registered.",0x200);	
			return 3;//主机或者从机未注册
		}
		else if(Unpack_return == -8)//服务器json接收不全或者解析失败
		{
			DebugPrint_uart("\r\n** Server JSON parsing failed.",0x200);
			return 4;//数据提交错误返回			
		}
		else if(Unpack_return==-9)
		{
			DebugPrint_uart("\r\n** Slave is registered, data submission failed.",0x200);
			return 5;//数据提交错误返回			
		}
		else
		{
			DebugPrint_uart("\r\n** Unknown state error.",0x200);
			return 1;//数据提交错误返回
		}
	}
	else
	{
		DebugPrint_uart_LineFeed("** Connection error with server.",200);
		return 2;//服务器通信错误（丢包或者访问错误）
	}
}
/**
 * @brief Upload_DeviceData by NBIOT 
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021年12月4日
*/

int8_t Upload_DeviceData_NBIOT(void)
{
	Load_TimeStamp(Upload_DataTemp);//打时间戳
	Upload_Data_tempProcess();
	strcpy(UploadData_buff,Upload_DataSend);
	
	Pack_return=JsonPack("upload_data");	
	Body_head_Make(Post_UploadData_SN,sizeof(Post_UploadData_SN),1,strlen(Json_packed_BUFF));
	Post_to_Server_NBIOT();
	
	UART4_Refresh_Flag=0;//检测串口触发前要置位
	Wait_Until_FlagTrue(&UART4_Refresh_Flag,5000);//最大超时时间5s
	Delay(1000);
	if(NBIOT_Received_Flag==1)
	{	NBIOT_Received_Flag=0;	}
	else
	{
		return 2;
	}
	Body_Process();
	if(BodyHead_pro.State_code==200)//检查服务器是否为正常访问
	{//是
		Unpack_return=JsonUnpack("upload_data");
		if(Unpack_return==0)//数据上传成功
		{
			Upload_pressTest_CNT++;
			UART_Print_Number(Upload_pressTest_CNT);
			return 0;//数据上传成功
		}
		else if(Unpack_return == -4)//host或者device未注册
		{
			DebugPrint_uart("\r\n** This host or device was not registered.",0x200);	
			return 3;//主机或者从机未注册
		}
		else if(Unpack_return == -8)//服务器json接收不全或者解析失败
		{
			DebugPrint_uart("\r\n** Server JSON parsing failed.",0x200);
			return 4;//数据提交错误返回			
		}
		else if(Unpack_return==-9)
		{
			DebugPrint_uart("\r\n** Slave is registered, data submission failed.",0x200);
			return 5;//数据提交错误返回			
		}
		else
		{
			DebugPrint_uart("\r\n** Unknown state error.",0x200);
			return 1;//数据提交错误返回
		}
	}
	else
	{
		DebugPrint_uart_LineFeed("** Connection error with server.",200);
		return 2;//服务器通信错误（丢包或者访问错误）
	}	
	
}
/**
 * @brief  Time update by http header  	//
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月14日
*/
int8_t NetTime_Refesh(void)
{
	//从返回的head中获取时间
	uint8_t str_point_t=0;
	if(StringSearch("\r\nDate: ",8,100))
	{
		str_point_t=StringSecanPointer+8;
		
		{//Weekday
			if(Rx_Buff1[str_point_t]=='S')//周几？ S开头
			{
				if(Rx_Buff1[str_point_t+1]=='u')//中间字母
				{//Sun
					weekday=0;
				}
				else if(Rx_Buff1[str_point_t+1]=='a')
				{//Sat
					weekday=6;
				}
			}
			else if(Rx_Buff1[str_point_t]=='M')
			{//Mon
				weekday=1;
			}
			else if(Rx_Buff1[str_point_t]=='T')
			{
				if(Rx_Buff1[str_point_t+1]=='u')
				{	//Tue
					weekday=2;			
				}
				else if(Rx_Buff1[str_point_t+1]=='h')
				{//Thur
					weekday=4;
				}			
			}
			else if(Rx_Buff1[str_point_t]=='W')
			{//Wed
				weekday=3;
			}
	
			else if(Rx_Buff1[str_point_t]=='F')
			{//Fri
				weekday=5;
			}
			str_point_t+=5;
		}
		{//day
			if((Rx_Buff1[str_point_t+1]>=48)&&(Rx_Buff1[str_point_t+1]<58))
			{
				day=((Rx_Buff1[str_point_t]-48)*10)+(Rx_Buff1[str_point_t+1]-48);
				str_point_t+=3;
			}		
			else
			{
				day=(Rx_Buff1[str_point_t]-48);
				str_point_t+=2;
			}
			
		}
		{//month
			if(Rx_Buff1[str_point_t]=='J')//月份J开头  1 6 7
			{
				if(Rx_Buff1[str_point_t+1]=='a')//中间字母
				{//Jan
					month=1;
				}
				else if(Rx_Buff1[str_point_t+1]=='u')
				{//Jun
					month=6;
				}
				else if(Rx_Buff1[str_point_t+2]=='l')
				{//Jul
					month=7;
				}				
			}
			else if(Rx_Buff1[str_point_t]=='M')//3 5
			{
				if(Rx_Buff1[str_point_t+1]=='a')//中间字母
				{//Mar
					month=3;
				}				
				else if(Rx_Buff1[str_point_t+2]=='y')//最后字母
				{//May
					month=5;
				}
			}
			else if(Rx_Buff1[str_point_t]=='A')//4 8
			{
				if(Rx_Buff1[str_point_t+1]=='p')//中间字母
				{//Apr
					month=4;
				}				
				else if(Rx_Buff1[str_point_t+1]=='u')
				{//Aug
					month=8;
				}
			}
			else if(Rx_Buff1[str_point_t]=='F')//2 F
			{//Feb
				month=2;
			}	
			else if(Rx_Buff1[str_point_t]=='S')//9 S
			{//Sep
				month=9;
			}			
			else if(Rx_Buff1[str_point_t]=='O')//10 O
			{//Oct
				month=10;
			}			
			else if(Rx_Buff1[str_point_t]=='N')//11 N
			{//Feb
				month=11;
			}			
			else if(Rx_Buff1[str_point_t]=='D')//12 D
			{//Dec
				month=12;
			}
			str_point_t+=4;
		}
		{//year
			year=((Rx_Buff1[str_point_t]-48)*1000)+((Rx_Buff1[str_point_t+1]-48)*100)\
			+((Rx_Buff1[str_point_t+2]-48)*10)+(Rx_Buff1[str_point_t+3]-48);
			str_point_t+=5;
		}
		{//hour
			hour=((Rx_Buff1[str_point_t]-48)*10)+(Rx_Buff1[str_point_t+1]-48);
			str_point_t+=3;
		}
		{//min
			min=((Rx_Buff1[str_point_t]-48)*10)+(Rx_Buff1[str_point_t+1]-48);
			str_point_t+=3;		
		}
		{//sec
			sec=((Rx_Buff1[str_point_t]-48)*10)+(Rx_Buff1[str_point_t+1]-48);
			str_point_t+=3;
		}
		return 0;
	}
	else
	{
		return 1;
	}
}
/**
 * @brief Load SN//将芯片ID加载到SN
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月15日
*/
int8_t Load_SN(char *ID_t,uint8_t ID_Len)
{
	for(uint8_t j=0;j<16;j++)
	{
		Host_sn.Str[j]='0';
	}
	Host_sn.Str[17]=0;
	for(uint8_t i=0;i<ID_Len;i++)
	{
		Host_sn.Str[i]=ID_t[i];
	}
	return 0;
}
/**
 * @brief Load host name//将注册主机时加载host name
 * @param	void
 * @retval 1 表示否；0表示正常
 * @author ZCD1300
 * @Time 2021年11月15日
*/
int8_t Load_host_name(char *hostName_t)
{
	uint8_t HostName_Len_t =strlen(hostName_t);
	if(HostName_Len_t>20)
	{
		return 1;//超出长度
	}
	for(uint8_t i=0;i<HostName_Len_t;i++)
	{
		Host_Name[i]=hostName_t[i];
	}
	return 0;
}
/**
 * @brief Load time stamp//在字符串加载时间戳
 * @param	char *OperationBuff
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月19日
*/
void Load_TimeStamp(char *OperationBuff)
{
	OperationBuff[5]=year/1000+48;
	OperationBuff[6]=(year%1000)/100+48;
	OperationBuff[7]=(year%100)/10+48;
	OperationBuff[8]=(year%10)+48;
	OperationBuff[10]=month/10+48;
	OperationBuff[11]=month%10+48;	
	OperationBuff[13]=day/10+48;
	OperationBuff[14]=day%10+48;		
	uint8_t hour_offset=hour+8;
	if(hour_offset>=24)
	{
		hour_offset=hour_offset-24;
		OperationBuff[14]++;
	}
	OperationBuff[16]=hour_offset/10+48;
	OperationBuff[17]=hour_offset%10+48;		
	OperationBuff[19]=min/10+48;
	OperationBuff[20]=min%10+48;	
	OperationBuff[22]=sec/10+48;
	OperationBuff[23]=sec%10+48;
}
/**
 * @brief Connect to server// 连接到服务器
 * @param	void 
 * @retval int8_t 
 * @author ZCD1300
 * @Time 2021年11月19日
*/
uint8_t ConnetServer_Overtime_Flag=0;
int8_t Connect_Server_Smatnest(void)
{
	if(Close_Connection_Return==1)
	{//未退出透传模式
		retryClose_ConnectServerSmatnest:
		Wifi_ExitTransp_Send();//退出透传
		if(Wifi_CLOSE_Connection(0)!=0)//关闭全部连接
		{//未关闭连接
			goto retryClose_ConnectServerSmatnest;
		}	
	}
	{//连接到服务器
		wifi_build_connect(1,SmartNest_Domain,SmartNest_DomainLEN);//连接到服务器
		uint8_t Connected_To_Server_Print_Cnt=0;
		uint8_t ConnectServer_retryCNT=0;

		while(!StringSearch("NECT\r\n\r\nOK",10,150))//等待建立到服务器连接完成
		{//未建立连接
			Delay(100);
			Connected_To_Server_Print_Cnt++;
			if((Connected_To_Server_Print_Cnt/3)==1)
			{
				Connected_To_Server_Print_Cnt=0;
				ConnectServer_retryCNT++;
				DebugPrint_uart("** Connecting to server.\r\n",0x200);
			}
			if(ConnectServer_retryCNT>10)//限制最大重试连接服务器次数
			{
				ConnectServer_retryCNT=0;
				ConnetServer_Overtime_Flag=1;
				break;
			}
			else
			{
				ConnetServer_Overtime_Flag=0;
			}
		}
		if(ConnetServer_Overtime_Flag==1)
		{
			DebugPrint_uart("** Connected to server overtime.\r\n",0x200);
			Connected_to_Server_Flag=0;
			return 1;
		}
		DebugPrint_uart("** Connected to server.\r\n",0x200);
		Connected_to_Server_Flag=1;
		TCPLink_Alive=1;
		return 0;
	}	

}
/**
 * @brief Load sensor data //在字符串加载传感器数据
 * @param	char *OperationBuff
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月19日
*/
int8_t Load_Sensor_Data(char* OperationBuff,uint8_t DeviceID_t)
{
	//temperture
	if(DeviceSNList[DeviceID_t-1].Temperture>=0)
	{
		OperationBuff[32]=DeviceSNList[DeviceID_t-1].Temperture/10+48;
		OperationBuff[33]=(int16_t)DeviceSNList[DeviceID_t-1].Temperture%10+48;
		OperationBuff[35]=(int16_t)(DeviceSNList[DeviceID_t-1].Temperture*10)%10+48;	
	}else
	{
		DeviceSNList[DeviceID_t-1].Temperture=(DeviceSNList[DeviceID_t-1].Temperture)*-1;
		OperationBuff[31]='-';
		OperationBuff[32]=DeviceSNList[DeviceID_t-1].Temperture/10+48;
		OperationBuff[33]=(int16_t)DeviceSNList[DeviceID_t-1].Temperture%10+48;
		OperationBuff[35]=(int16_t)(DeviceSNList[DeviceID_t-1].Temperture*10)%10+48;		
	}

	//RH
	if(RH_D>99)
	{
		OperationBuff[42]='1';
		OperationBuff[43]='0';
		OperationBuff[44]='0';
	}
	else
	{
		OperationBuff[42]='0';
		OperationBuff[43]=DeviceSNList[DeviceID_t-1].RH/10+48;
		OperationBuff[44]=DeviceSNList[DeviceID_t-1].RH%10+48;
	}

	//Lux
	OperationBuff[53]=DeviceSNList[DeviceID_t-1].Lux/10000+48;
	OperationBuff[54]=(DeviceSNList[DeviceID_t-1].Lux%10000)/1000+48;
	OperationBuff[55]=(DeviceSNList[DeviceID_t-1].Lux%1000)/100+48;
	OperationBuff[56]=(DeviceSNList[DeviceID_t-1].Lux%100)/10+48;
	OperationBuff[57]=DeviceSNList[DeviceID_t-1].Lux%10+48;
	//CO2
	OperationBuff[65]=DeviceSNList[DeviceID_t-1].CO2/1000+48;
	OperationBuff[66]=(DeviceSNList[DeviceID_t-1].CO2%1000)/100+48;
	OperationBuff[67]=(DeviceSNList[DeviceID_t-1].CO2%100)/10+48;
	OperationBuff[68]=DeviceSNList[DeviceID_t-1].CO2%10+48;
	//NH3
	OperationBuff[76]=DeviceSNList[DeviceID_t-1].NH3/1000+48;
	OperationBuff[77]=(DeviceSNList[DeviceID_t-1].NH3%1000)/100+48;
	OperationBuff[78]=(DeviceSNList[DeviceID_t-1].NH3%100)/10+48;
	OperationBuff[79]=DeviceSNList[DeviceID_t-1].NH3%10+48;	
	
}
/**
 * @brief Load device SN//将device芯片ID加载到SN
 * @param	uint8_t DeviceID_t 指定的device ID
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月1日
*/
int8_t Load_DeviceSN(uint8_t DeviceID_t)
{
	for(uint8_t i=0;i<16;i++)
	{
		DeviceSN_single[i]=DeviceSNList[DeviceID_t-1].DeviceSN[i];
	}	
	DeviceSN_single[16]=0;
	return 0;
}
	
/**
 * @brief Regest all device SN//注册所有device SN
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月4日
*/	
void Reg_Device_ALL(void)
{
	char temp_t1[]="** Device:\r\n** 123456\r\n";
	char temp_t2[]="**       ";
	for(uint8_t i=0;i<DeviceNUM_MAX;i++)
	{
		if(DeviceOnline_Falg[i]==1)
		{
			temp_t2[i+3]='*';
			Load_DeviceSN(i+1);	//这个+1是因为device ID从1开始
			Register_Device();//只注册一个device的
		}
	}
	DebugPrint_uart(temp_t1,200);//打印已经注册的编号
	DebugPrint_uart(temp_t2,200);
}
/**
* @brief PUSH Warning//服务器推送功能
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021年12月11日
*/
int8_t PUSH_Serve(void)
{
	Pack_return=JsonPack("warningPUSH");		
	Body_head_Make(Post_PUSH,sizeof(Post_PUSH),1,strlen(Json_packed_BUFF));
	Post_to_Server();
	
	UART1_Refresh_Flag=0;//检测串口触发前要置位
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//最大超时时间5s
	Body_Process();
	if(BodyHead_pro.State_code==200)//检查服务器是否为正常访问
	{//是
		Unpack_return=JsonUnpack("warningPUSH");
		if(Unpack_return==0)//数据推送成功
		{
			DebugPrint_uart("\r\n** Warning PUSH Successed.\r\n",0x200);	
			return 0;
		}
		else if(Unpack_return == -2)//用户未登录
		{
			DebugPrint_uart("\r\n** No user login.",0x200);	
			return 3;//主机或者从机未注册
		}		
		else if(Unpack_return == -8)//服务器json接收不全或者解析失败
		{
			DebugPrint_uart("\r\n** Server JSON parsing failed.",0x200);
			return 4;//数据提交错误返回			
		}
		else if(Unpack_return==-9)
		{
			DebugPrint_uart("\r\n** Slave is registered, data submission failed.",0x200);
			return 5;//数据提交错误返回			
		}
		else
		{
			DebugPrint_uart("\r\n** Unknown state error.",0x200);
			return 1;//数据提交错误返回
		}
	}
	else
	{
		DebugPrint_uart_LineFeed("** Connection error with server.",200);
		return 2;//服务器通信错误（丢包或者访问错误）
	}

}
