 /**
 * @brief  ESP8266驱动
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年1月24日
*/

#include "ESP8266.h"

#include "wifi.h"
#include "http_json.h"

uint8_t Wifi_module_State=0;
 /**
 * @brief  ESP8266发送函数
 * @param	指令1，指令2，指令1长度，指令2长度（实际发送为指令1+指令2；指令2为0则只发送指令1）
 * @retval	void
 * @author ZCD
 * @Time 2021年1月25日
*/
void Wifi_CMD_SEND(char *cmd,char* cmd2,uint8_t LenTemp,uint8_t LenTemp2)
{
	if(cmd2!=0)
	{
		HAL_UART_Transmit(&huart1,cmd,LenTemp-1,0x100);//sizeof计算的长度包括“\0”占位符要减掉
		HAL_UART_Transmit(&huart1,cmd2,LenTemp2,0x100);	
		HAL_UART_Transmit(&huart1,"\r\n",2,0x100);	
	}
	else
	{
		HAL_UART_Transmit(&huart1,cmd,LenTemp-1,0x100);//sizeof计算的长度包括“\0”占位符要减掉
		HAL_UART_Transmit(&huart1,"\r\n",2,0x100);		
	}
}
 /**
 * @brief  ESP8266复位函数
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年1月25日
*/
void Wifi_RST(void)//初始化
{
	Wifi_CMD_SEND(WifiRST,0,sizeof(WifiRST),2);
}
 /**
 * @brief  ESP8266设置模块工作模式函数
 * @param	 mode = 1-sta  2-ap  3-sta+ap
 * @retval	void
 * @author ZCD
 * @Time 2021年1月25日
*/
 /**
 * @brief  ESP8266 设置工作模式
 * @param	 uint8_t mode 1 sta/2 ap/3 sta+ap   
 * @retval	uint8_t return  0表示切换成功
 * @author ZCD
 * @Time 2021年11月19日  
*/
uint8_t Wifi_ModeSet(uint8_t mode)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//未退出透传
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
	else//输入参数错误
	{	return 2; }
	UART1_Refresh_Flag=0;//检测串口触发前要置位
	if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//最大超时时间5s
	{
		if(StringSearch("\r\nOK",4,100))
		{//模式切换完成	
			Wifi_Read_WorkState();//刷新状态
			return 0;
		}
		else
		{//似乎没有响应或者返回数据被刷新掉
			return 3;
		}
	}
	else
	{
		return 6;//模块超时未响应
	}
}
 /**
 * @brief  ESP8266连接路由器函数,可手动选择
 * @param	 SSID，password
 * @retval	void
 * @author ZCD
 * @Time 2021年11月11日  
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
 * @brief  ESP8266连接路由器函数，固定连接到指定AP,开发测试用
 * @param	 SSID，密码
 * @retval	void
 * @author ZCD
 * @Time 2021年3月28日  
*/
void Wifi_Connect_AP2(void)
{
	uint8_t temp_Connect[]="\"404 NOT FOUND\",\"xd@404@12138\"";
	//uint8_t temp_Connect[]="\"zcd123456\",\"121380000\"";
		
	Wifi_CMD_SEND(WifiJoinAP,temp_Connect,sizeof(WifiJoinAP),sizeof(temp_Connect)-1);
}
 /**
 * @brief  ESP8266多连接模式切换
 * @param	 
 * @retval	void
 * @author ZCD1300
 * @Time 2021年12月24日  
*/
uint8_t Connection_List[9]={0};//建立的连接表,分别对应连接0~9，内容：0-无连接；1-UDP连接；2-TCP连接
uint8_t CIPMUX_Status=0;
uint8_t Wifi_CIPMUX_Ctrl(uint8_t MUXMode_t)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//未退出透传
		return 1;
	}	
	{//读取当前MUX使能状态
		Wifi_CMD_SEND(WifiMUXCheck,0,sizeof(WifiMUXCheck),2);
		Clear_Buff(Rx_Buff1,150,BuffMAX);//清空Rx_buff1一块区域
		UART1_Refresh_Flag=0;//检测串口触发前要置位			
		if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//超时时间5s
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
				return 3;//解析错误
			}
		}
		else
		{
			return 2;//模块返回接收超时
		}
		
		if(CIPMUX_Status==MUXMode_t)
		{//当前多连接状态同步
			return 4;
		}	
	}
	
	if(MUXMode_t!=0)
	{//使能多连接
		Wifi_CMD_SEND(WifiMUX,"1",sizeof(WifiMUX),1);
		Clear_Buff(Rx_Buff1,150,BuffMAX);//清空Rx_buff1一块区域
		UART1_Refresh_Flag=0;//检测串口触发前要置位	
		Wait_Until_FlagTrue(&UART1_Refresh_Flag,3000);
		if(	StringSearch("\r\nOK",4,150))
		{
			return 0;//正常返回
		}
		
	}
	else
	{//关闭多连接
		Wifi_CMD_SEND(WifiMUX,"0",sizeof(WifiMUX),1);
		Clear_Buff(Rx_Buff1,150,BuffMAX);//清空Rx_buff1一块区域
		UART1_Refresh_Flag=0;//检测串口触发前要置位			
		Wait_Until_FlagTrue(&UART1_Refresh_Flag,3000);
		if(	StringSearch("\r\nOK",4,150))
		{
			return 0;//正常返回
		}
	}
	return 1;//切换失败

}

 /**
 * @brief  ESP8266 TCP Server Ctrl
 * @param	 
 * @retval	void
 * @author ZCD1300
 * @Time 2021年12月24日  
*/
uint8_t Wifi_TCPServer_Ctrl(uint8_t ServerState_t)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//未退出透传
		return 1;
	}		
	if(ServerState_t!=0)
	{//开服务器
		Wifi_CMD_SEND(WifiTCPServer,"1,80",sizeof(WifiTCPServer),4);
		Clear_Buff(Rx_Buff1,150,BuffMAX);//清空Rx_buff1一块区域
		UART1_Refresh_Flag=0;//检测串口触发前要置位			
		if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//超时时间5s
		{
			if(StringSearch("\r\nOK",4,150))
			{//切换完成
				return 0;
			}
		}			
	}
	else
	{//关服务器
		Wifi_CMD_SEND(WifiTCPServer,"0",sizeof(WifiTCPServer),1);
		Clear_Buff(Rx_Buff1,150,BuffMAX);//清空Rx_buff1一块区域
		UART1_Refresh_Flag=0;//检测串口触发前要置位			
		if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//超时时间5s
		{
			if(StringSearch("\r\nOK",4,150))
			{//切换完成
				return 0;
			}			
		}			
	}
}

/**
 * @brief  检索特定字符串（最多检索10个特征字符） (只能检索RX_buff1的内容)
 * @param	 检索特征字符，字符匹配长度（<=20），检索深度
 * @retval	检索结果 1-是  0-否
 * @author ZCD
 * @Time 2021年1月26日
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
 * @brief  检索特定字符串（最多检索10个特征字符）（可指定检索的区域） 
 * @param	 检索特征字符，指定的检索池,字符匹配长度（<=20），检索深度
 * @retval	检索结果 1-是  0-否
 * @author ZCD
 * @Time 2021年11月13日
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
 * @brief Read Module Work State	//读取模块当前工作方式
 * @param	void
 * @retval uint8 return   成功返回0，失败返回1
 * @author ZCD1300
 * @Time 2021年11月15日
*/
uint8_t Wifi_Read_WorkState(void)
{
	Wifi_CMD_SEND(WifiMODECheck,0,sizeof(WifiMODECheck),2);
	UART1_Refresh_Flag=0;//检测串口触发前要置位
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//最大超时时间5s
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
 * @brief Read Module IP&MAC	//读取模块IP和MAC地址，如果AP打开则同时读取AP网关
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月15日
*/
char wifi_AP_GETWAY[16]={0};
char Wifi_STAIP[16]={0};
char Wifi_MAC[12]={0};
uint8_t Wifi_Read_IP_MAC(void)
{
	if((Wifi_module_State==1)||(Wifi_module_State==3))//保险起见，先限制只能在STA模式开启时才能读取MAC
	{
		Wifi_CMD_SEND(WifiIP,0,sizeof(WifiIP),2);	
		UART1_Refresh_Flag=0;//检测串口触发前要置位
		Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//最大超时时间5s	
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
 * @brief Exit module transparent send//退出透传模式
 * @param	void
 * @retval uint8_t return 1表示退出透传成功，0表示失败
 * @author ZCD1300
 * @Time 2021年11月18日
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
	{//成功读取到，说明退出了透传模式
		Wifi_Transparent_Send_Flag=0;
		Wifi_Transparent_SendEntered_Flag=0;
		WifiInitOKflag=0;
		return 1;
	}
	else
	{//退出透传失败,应该返回重试，（最大重试3次）
		if(Error_cnt_t<3)
		{
			Error_cnt_t++;		
			goto ExitTransp_Send_Lable;
		}
		else
		{//重试10次失败,认为模块离线，退出透传
			Error_cnt_t=0;
			Wifi_Transparent_Send_Flag=0;
			Wifi_Transparent_SendEntered_Flag=0;
			return 0;
		}
	}
}
/**
 * @brief Close connection//关闭连接（目前未完成关闭指定连接，只能全部关闭）
 * @param	char* Connection_ID 指定关闭的ID，字符串解析，如果输入数值0，则认为全部关闭
 * @retval uint8_t return 看注释
 * @author ZCD1300 
 * @Time 2021年11月18日
*/
uint8_t Close_Connection_Return=0;
uint8_t Wifi_CLOSE_Connection(char* Connect_ID)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//未退出透传
		Close_Connection_Return=1;
		return 1;
	}
	if(Connect_ID==0)//关闭全部连接
	{
		Wifi_CMD_SEND(WifiConnectCloseALL,0,sizeof(WifiConnectCloseALL),2);
		
		UART1_Refresh_Flag=0;//检测串口触发前要置位
		if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//最大超时时间5s
		{
			if(StringSearch("CLOSED\r\n\r\nOK",12,200))
			{//关闭成功,返回0
				Wifi_Transparent_SendEntered_Flag=0;
				Close_Connection_Return=0;
				return 0;
			}
			else if(StringSearch("ERROR",5,200))
			{
				Wifi_Transparent_SendEntered_Flag=0;
				Close_Connection_Return=3;
				return 3;//已经没有连接了
			}
			else
			{//模块未知错误
				Close_Connection_Return=4;
				return 4;
			}
		}
		Wifi_Transparent_SendEntered_Flag=0;
		Close_Connection_Return=0;		
		return 6;//模块超时未响应
	}
	else//关闭指定连接，还未完成
	{
		Close_Connection_Return=2;
		return 2;//返回2表示程序未完成
	}
	Close_Connection_Return=5;
	return 5;//输入参数错误
}
/**
 * @brief Check whether a TCP connection is established //检查是否建立了TCP连接
 * @param	void 
 * @retval int8_t return  1 表示存在TCP连接   0表示不存在连接 其他返回见注释
 * @author ZCD1300
 * @Time 2021年11月21日
*/
uint8_t TCPLink_Alive=0;
int8_t Check_TCP_Connection(void)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//未退出透传
		Wifi_ExitTransp_Send();//退出透传
		//return 2;
	}
	{//检查连接
		Wifi_CMD_SEND(WifiConnectList,0,sizeof(WifiConnectList),2);
		
		UART1_Refresh_Flag=0;//检测串口触发前要置位		
		if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//最大超时时间5s
		{
			if(StringSearch("\"TCP\",\"",7,100))
			{//存在TCP连接
				for(uint8_t i=0;i<7;i++)
				{
					Rx_Buff1[StringSecanPointer+i]=0;
				}
				
				TCPLink_Alive=1;
				LED_ctrl(1,1);
				return 1;
			}
			else
			{//不存在TCP连接
				TCPLink_Alive=0;
				Connected_to_Server_Flag=0;
				LED_ctrl(1,0);
				return 0;
			}
		}
		TCPLink_Alive=0;
		Connected_to_Server_Flag=0;	
		LED_ctrl(1,0);		
		return 6;//模块超时未响应
	}
}
/**
 * @brief Check AP name//读取AP名字
 * @param	void 
 * @retval int8_t return  0没有AP连接 1有AP连接
 * @author ZCD1300
 * @Time 2021年11月21日
*/
char AP_Name[20]={0};
uint8_t Wifi_Check_AP_Name(void)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//未退出透传
		if(Wifi_ExitTransp_Send())
		{
			//退出透传
		}
		else
		{return 2;}
	}
	uint8_t str_start_t=0;
	uint8_t str_end_t=0;
	Wifi_CMD_SEND(WifiJoinAPSuces,0,sizeof(WifiJoinAPSuces),2);	
	UART1_Refresh_Flag=0;//检测串口触发前要置位
	if(Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000))//最大超时时间5s
	{
		if(StringSearch("No AP",5,200))
		{//没有AP连接
			DebugPrint_uart("** No AP linked.",200);			
			return 0;
		
		}
		else if(StringSearch("+CWJAP:\"",8,200))
		{//有AP连接
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
		{//未知错误
			return 3;
		}		
	}
	else//模块超时
	{return 6;}
}
/**
 * @brief Read sta gateway and Subnet mask
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2021年11月22日
*/
char wifi_GATEWAY[16]={0};
char wifi_MASK[16]={0};
uint8_t Wifi_Read_Getway_Mask(void)
{
	if(Wifi_Transparent_SendEntered_Flag!=0)
	{//未退出透传
		if(Wifi_ExitTransp_Send())
		{
			//退出透传
		}
		else
		{return 2;}
	}
	Wifi_CMD_SEND(WifiNETSTA,0,sizeof(WifiNETSTA),2);	
	UART1_Refresh_Flag=0;//检测串口触发前要置位
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//最大超时时间5s	
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
//Fr进程实体
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
//进程句柄
osThreadId Wifi_ControlHandle;
//进程创建声明
void Wifi_ControlThreadCreate(osPriority taskPriority)
{
	osThreadDef(Wifi_ControlThread,Wifi_Control,taskPriority,0,1024);
	Wifi_ControlHandle = osThreadCreate(osThread(Wifi_ControlThread),NULL);
}
*/


