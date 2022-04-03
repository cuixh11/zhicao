#include "zg6907.h"

ZG_Module ZG_Module_State;

 /**
 * @brief  zigbee模块切换到配置模式
 * @param	void
 * @retval 返回0表示切换成功，返回1表示失败
 * @author ZCD1300
 * @Time 2021年10月18日
*/
uint8_t Switch_to_SetMode(void)
{
	uint8_t Switch_SetMode[]=Set_mode;
	UART_Send(&huart1,Switch_SetMode,3,0x10);
	HAL_Delay(100);
	uint8_t temp=0;
	if(Rx_Buff1[0]==0x7a)
	{
		temp=1;			
	}
	if(Rx_Buff1[1]==0x7d)
	{
		temp++;
	}
	if(Rx_Buff1[2]==0x7e)
	{
		temp++;
	}
	if(temp==3)
	{	
		ZG_Module_State.Module_Mode=1;
		return 0;}
	else
	{return 1;}

} 

 /**
 * @brief  zigbee模块切换到传输模式
 * @param	void
 * @retval 返回0表示切换成功，返回1表示失败
 * @author ZCD1300
 * @Time 2021年10月18日
*/
uint8_t Switch_to_TransMode(void)
{
	uint8_t Switch_TranMode[]=Trans_mode;
	UART_Send(&huart1,Switch_TranMode,3,0x10);
	HAL_Delay(100);
	uint8_t temp=0;
	if(Rx_Buff1[0]==0x7f)
	{
		temp=1;			
	}
	if(Rx_Buff1[1]==0x7c)
	{
		temp++;
	}
	if(Rx_Buff1[2]==0x7b)
	{
		temp++;
	}
	if(temp==3)
	{	
		ZG_Module_State.Module_Mode=0;
		return 0;}
	else
	{return 1;}
}
 /**
 * @brief 模块软重启（修改节点类型后要重启）
 * @param	void
 * @retval 返回0表示成功，返回1失败
 * @author ZCD1300
 * @Time 2021年10月19日
*/
uint8_t ReBoot_Module(void)
{
	if(Switch_to_SetMode()==0)
	{
		ZG_Module_State.Module_Mode=1;
		uint8_t ReBoot[]=ReBoot_Arr;
		UART_Send(&huart1,ReBoot,4,0x10);
		HAL_Delay(100);
		uint8_t temp=0;
		if(Rx_Buff1[0]==0xfa)
		{
			temp=1;			
		}
		if(Rx_Buff1[1]==0x12)
		{
			temp++;
		}
		if(temp==2)
		{	
			ZG_Module_State.Module_Mode=0;
			Read_AllNet_Value();
			return 0;}
		else
		{return 1;}		
	}
	else{return 1;}
}
 /**
 * @brief 模块恢复出厂设置
 * @param	void
 * @retval 返回0表示成功，返回1失败
 * @author ZCD1300
 * @Time 2021年10月20日
*/ 
uint8_t Recovery_Module(void)
{
	if(Switch_to_SetMode()==0)
	{
		ZG_Module_State.Module_Mode=1;
		uint8_t Recovery[]=Recovery_Arr;
		UART_Send(&huart1,Recovery,4,0x10);
		HAL_Delay(100);
		uint8_t temp=0;
		if(Rx_Buff1[0]==0xfa)
		{
			temp=1;			
		}
		if(Rx_Buff1[1]==0x13)
		{
			temp++;
		}
		if(temp==2)
		{	
			Read_AllNet_Value();
			return 0;}
		else
		{return 1;}	
	}
	else{return 1;}	
}

/**
 * @brief 读取模块单个网络参数
 * @param	CMD：指令数组制作；	Num：指令长度；	*back：返回数据存储指针（最大8字节）
 * @retval 返回0表示读取成功，返回1读取失败
 * @author ZCD1300
 * @Time 2021年10月20日
*/
uint8_t Read_SingleNET_Value(uint8_t * Cmd,uint8_t Num,uint8_t *back)
{
	if(ZG_Module_State.Module_Mode!=1)
	{
		if(Switch_to_SetMode()==0)
		{
			ZG_Module_State.Module_Mode=1;
		}
		else
		{return 1;}	
	}
	UART_Send(&huart1,Cmd,Num,0x20);
	HAL_Delay(100);
	uint8_t temp=0;
	if(Rx_Buff1[0]==0xf7)
	{
		temp=1;			
	}
	if(Rx_Buff1[1]==0xff)
	{
		temp++;
	}
	if(temp==2)
	{	return 1;}
	else
	{	
		if((Rx_Buff1[0]==0xfb)&&(Rx_Buff1[1]==Cmd[2]))
		{
			back[0]=Rx_Buff1[2];
			back[1]=Rx_Buff1[3];
			back[2]=Rx_Buff1[4];
			back[3]=Rx_Buff1[5];
			back[4]=Rx_Buff1[6];
			back[5]=Rx_Buff1[7];
			back[6]=Rx_Buff1[8];
			back[7]=Rx_Buff1[9];
			
			return 0;	
		}
		else
		{return 1;}
	}			
	
}

 /**
 * @brief 读取模块所有网络参数
 * @param	void
 * @retval 返回0表示成功，返回1失败
 * @author ZCD1300
 * @Time 2021年10月19_20日
*/
uint8_t Read_AllNet_Value(void)
{
	if(Switch_to_SetMode()==0)
	{
		ZG_Module_State.Module_Mode=1;
		uint8_t Read_All_netValue[]=ReadAllNetVal;
		UART_Send(&huart1,Read_All_netValue,4,0x10);
		HAL_Delay(100);
		uint8_t temp=0;
		if(Rx_Buff1[0]==0xf7)
		{
			temp=1;			
		}
		if(Rx_Buff1[1]==0xff)
		{
			temp++;
		}
		if(temp==2)
		{	return 1;}
		else
		{
			if((Rx_Buff1[0]==0xfb)&&(Rx_Buff1[1]==0xfe))
			{
				ZG_Module_State.Node_Yype=Rx_Buff1[2];
				ZG_Module_State.NET_State=Rx_Buff1[3];
				ZG_Module_State.PAN_ID=(uint16_t)Rx_Buff1[4]<<8|Rx_Buff1[5];
				ZG_Module_State.Addr=(uint16_t)Rx_Buff1[6]<<8|Rx_Buff1[7];
				ZG_Module_State.MAC=(uint64_t)Rx_Buff1[8]<<56|(uint64_t)Rx_Buff1[9]<<48|(uint64_t)Rx_Buff1[10]<<40|(uint64_t)Rx_Buff1[11]<<32|(uint32_t)Rx_Buff1[12]<<24|(uint32_t)Rx_Buff1[13]<<16|(uint16_t)Rx_Buff1[14]<<8|Rx_Buff1[15];
				ZG_Module_State.Addr_Farther=(uint16_t)Rx_Buff1[16]<<8|Rx_Buff1[17];
				ZG_Module_State.MAC_Father=(uint64_t)Rx_Buff1[18]<<56|(uint64_t)Rx_Buff1[19]<<48|(uint64_t)Rx_Buff1[20]<<40|(uint64_t)Rx_Buff1[21]<<32|(uint32_t)Rx_Buff1[22]<<24|(uint32_t)Rx_Buff1[23]<<16|(uint16_t)Rx_Buff1[24]<<8|Rx_Buff1[25];
				ZG_Module_State.NET_GroupNum=Rx_Buff1[26];
				ZG_Module_State.Channel=Rx_Buff1[27];
				ZG_Module_State.TX_power=Rx_Buff1[28];
				ZG_Module_State.Baud_Rate=Rx_Buff1[29];
				ZG_Module_State.Sleep_Time=Rx_Buff1[30];
				ZG_Module_State.Target_Addr=(uint16_t)Rx_Buff1[31]<<8|Rx_Buff1[32];
				ZG_Module_State.Target_GroupNum=Rx_Buff1[33];
				ZG_Module_State.Target_MAC=(uint64_t)Rx_Buff1[34]<<56|(uint64_t)Rx_Buff1[35]<<48|(uint64_t)Rx_Buff1[36]<<40|(uint64_t)Rx_Buff1[37]<<32|(uint32_t)Rx_Buff1[38]<<24|(uint32_t)Rx_Buff1[39]<<16|(uint16_t)Rx_Buff1[40]<<8|Rx_Buff1[41];
				ZG_Module_State.Send_Mode=Rx_Buff1[42];
				ZG_Module_State.Output_Mode=Rx_Buff1[43];
				ZG_Module_State.NET_Opentime=Rx_Buff1[44];
				ZG_Module_State.Rejoin_Circle=Rx_Buff1[45];
				ZG_Module_State.Rejoin_Count=Rx_Buff1[46];
				ZG_Module_State.Wireless_ID=((uint16_t)Rx_Buff1[47]<<8)|Rx_Buff1[48];
				
				ZG_Module_State.Father_SaveTime=30;
				uint8_t Read_Child_NUM[]={read_head,0x01,0x32,read_end};
				uint8_t Read_AUX_dlayTime[]={read_head,0x01,0x35,read_end};
				uint8_t Read_Hold_Time[]={read_head,0x01,0x36,read_end};
				Read_SingleNET_Value(Read_Child_NUM,4,&ZG_Module_State.Child_Num);
				Read_SingleNET_Value(Read_AUX_dlayTime,4,&ZG_Module_State.AUX_delayTime);
				Read_SingleNET_Value(Read_Hold_Time,4,&ZG_Module_State.Uart_Holdtime);
				
				ZG_Module_State.Module_Mode=1;
				return 0;
			}
			else
			{
				return 1;
			}
		}		
	}
	else
	{return 1;}
}
/**
 * @brief 写入模块单个网络参数
 * @param	CMD：指令数组指针；	Num：指令长度
 * @retval 返回0表示成功，返回1失败
 * @author ZCD1300
 * @Time 2021年10月20日
*/
uint8_t SET_SingleValue(uint8_t * Cmd,uint8_t Num)
{
	if(ZG_Module_State.Module_Mode!=1)
	{
		if(Switch_to_SetMode()==0)
		{
			ZG_Module_State.Module_Mode=1;
		}
		else
		{return 1;}
	}
	else
	{
		UART_Send(&huart1,Cmd,Num,0x10);
		HAL_Delay(100);
		uint8_t temp=0;
		if(Rx_Buff1[0]==0xf7)
		{
			temp=1;			
		}
		if(Rx_Buff1[1]==0xff)
		{
			temp++;
		}
		if(temp==2)
		{	return 1;}	
		else
		{
			if((Rx_Buff1[0]==0xfa)&&(Rx_Buff1[1]==Cmd[2]))
			{
				return 0;
			}
			else
			{return 1;}
		}
	}
	return 1;
}
/**
 * @brief 写入模块全部网络参数的预设值
					(预设参数：PANID：0xfffe；信道11；发射功率20dbm；分组01；协调器、路由器广播；终端单播；透传)
 * @param	void
 * @retval 返回0表示成功，返回1失败
 * @author ZCD1300
 * @Time 2021年10月20日
*/
uint8_t SET_ALLValue(void)
{

	if(ZG_Module_State.Module_Mode!=1)
	{
		if(Switch_to_SetMode()==0)
		{
			ZG_Module_State.Module_Mode=1;
		}
		else
		{return 1;}	
	}
	uint8_t SET_All_netValue[]=SET_AllNETValue_Coordinator;
	UART_Send(&huart1,SET_All_netValue,30,0x20);
	HAL_Delay(100);
	uint8_t temp=0;
	if(Rx_Buff1[0]==0xf7)
	{
		temp=1;			
	}
	if(Rx_Buff1[1]==0xff)
	{
		temp++;
	}
	if(temp==2)
	{	return 1;}	
	else
	{
		if((Rx_Buff1[0]==0xfa)&&(Rx_Buff1[1]==0xfe))
		{
			Read_AllNet_Value();
			return 0;
		}
		else
		{return 1;}
	}	
}
/**
 * @brief 网络相关操作
 * @param	CMD_num:操作数字代号，1打开网络、2离开网络、3新建网络、4开始TouchLink
 * @retval 返回0表示操作成功，返回1操作失败
 * @author ZCD1300
 * @Time 2021年10月20日
*/
uint8_t NWK_Operate(uint8_t CMD_num)
{
	if(ZG_Module_State.Module_Mode==1)
	{
		uint8_t NWK_Arr[]={0xf5,0x01,0x40,CMD_num,0xff};
		UART_Send(&huart1,NWK_Arr,5,0x10);
		HAL_Delay(100);
		uint8_t temp=0;
		if(Rx_Buff1[0]==0xfc)
		{
			temp=1;			
		}
		if(Rx_Buff1[1]==0x40)
		{
			temp++;
		}
		if(Rx_Buff1[2]==0x00)
		{
			temp++;
		}	
		if(temp==3)
		{	
			if(CMD_num==2)
			{ZG_Module_State.NET_State=0;}
			return 0;}	
		else
		{return 1;}	
	}
	else{return 1;}
}
/**
 * @brief 节点类型切换
 * @param	Type_num:操作数字代号，1协调器、2路由器、3非休眠终端、4休眠终端
 * @retval 返回0表示操作成功，返回1操作失败
 * @author ZCD1300
 * @Time 2021年10月20日
*/
uint8_t Nodetype_Switch(uint8_t Type_num)
{
	uint8_t NodeTypSitch[]={0xfd,0x01,0x01,Type_num,0xff};
	if(SET_SingleValue(NodeTypSitch,5)==0)
	{
		if(ReBoot_Module()==0)
		{
			if(ZG_Module_State.Node_Yype==Type_num)
			{	return 0;}
			else
			{return 1;}
		}
		else
		{return 1;}
	}
	else
	{return 1;}
}
/**
 * @brief PAN_ID和信道选择（下面两个函数，为啥见头文件；我也不知道）
 * @param	Target_PANID:16位ID		Target_Channel：信道（8位）
 * @retval 返回0表示操作成功，返回1有一个参数写入失败，2全部失败 3模块重启失败 4ID或Ch与目标值不同
 * @author ZCD1300
 * @Time 2021年10月20日
*/
uint8_t PANID_Channel_SET(uint16_t Target_PANID,uint8_t Target_Channel)
{
	uint8_t temp=0;
	temp=PANID_Channel_SET_Inside(Target_PANID,Target_Channel);
	temp=PANID_Channel_SET_Inside(Target_PANID,Target_Channel);
	return temp;
}
uint8_t PANID_Channel_SET_Inside(uint16_t Target_PANID,uint8_t Target_Channel)
{
	if(ZG_Module_State.Module_Mode!=1)
	{
		if(Switch_to_SetMode()==0)
		{
			ZG_Module_State.Module_Mode=1;
		}
		else
		{return 1;}	
	}
	
	Net_Check:
	if(ZG_Module_State.NET_State!=0)
	{
		if(NWK_Operate(2)!=0)
		{return 1;}
		Read_AllNet_Value();
		goto Net_Check;
	}
	
	uint8_t PANID_Arr[]={0xfd,0x02,0x03,Target_PANID>>8,(uint8_t)Target_PANID,0xff};
	uint8_t Channel_Arr[]={0xfd,0x01,0x0a,Target_Channel,0xff};
	uint8_t temp=2;
	temp=temp-SET_SingleValue(Channel_Arr,5);
	temp=temp-SET_SingleValue(PANID_Arr,6);
	if(temp==2)
	{
		if(ReBoot_Module()==0)
		{
			if((ZG_Module_State.PAN_ID==Target_PANID)&&(ZG_Module_State.Channel==Target_Channel))
			{	
				return 0;
			}
			else
			{
				return 4;
			}
		}
		else
		{return 3;}		
	}
	else 
	{
		Read_AllNet_Value();		
		return 2-temp;
	}
	
}






