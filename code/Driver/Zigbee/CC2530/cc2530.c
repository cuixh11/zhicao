 /**
 * @brief  cc2530相关的控制（非官方固件）
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年11月26日
*/
//cc2530 这里用预留的UART4
#include "cc2530.h"
#include "ESP8266.h"//只依赖其中的StringSearch_Assign()函数，用来检索字符串；

//#include "http_json.h"
//#include "host_app.h"


uint8_t DeviceOnline_Falg[DeviceNUM_MAX]={0};
 /**
 * @brief  ZigBee Device num refesh
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年12月3日
*/
uint8_t zigbee_Refesh_OnlineDeviceNUM(void)
{
	for(uint8_t i=1;i<=DeviceNUM_MAX;i++)
	{
		if (zigbee_Read_MAC(i,DeviceSNList))
		{//读取失败，认为终端离线
			DeviceOnline_Falg[i-1]=0;
		}
		else
		{
			//if((Device_BUFF_T[1]==0xFE)&&(Device_BUFF_T[5]==i))
			if(!((Device_BUFF_T[1]==0xFE)||(Device_BUFF_T[1]==0xff)))
			{//终端已经注册
				DeviceOnline_Falg[i-1]=1;
				Device_BUFF_T[0]=0;
			}
			else
			{
				DeviceOnline_Falg[i-1]=0;
				Device_BUFF_T[0]=0;
			}		
		}
	}
	

}
 /**
 * @brief  ZigBee net initial
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年12月3日
*/
uint8_t ZGRST_OK_Flag=0;
uint8_t ZigBeeNET_Init(void)
{
	uint8_t ZGRST_wait_cnt=0;
	if(GPIO_Read(GPIOF,GPIO_PIN_2)==RESET)
	{//key3 按下，直接跳过ZigBeeNET_Init
		ZGRST_OK_Flag=1;
		zigbee_Refesh_OnlineDeviceNUM();
		DebugPrint_uart("||----ZigBee net initialization completed----||\r\n\r\n",0x200);//完成	
		return 1;//跳过RST检查
	}
	
	ZGRST_Retry:
	{//协调器复位
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET);	
		Delay(100);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET);		
	}
	//等待复位完成
	while(!StringSearch_Assign("say Hello",Rx_Buff3,9,100))
	{
		{//跳过检查ZigBeeNET_Init
			if(GPIO_Read(GPIOF,GPIO_PIN_2)==RESET)
			{//key3 按下，直接跳过ZigBeeNET_Init
				ZGRST_OK_Flag=1;
				zigbee_Refesh_OnlineDeviceNUM();
				DebugPrint_uart("||----ZigBee net initialization completed----||\r\n\r\n",0x200);//完成	
				ZGRST_wait_cnt=0;
				return 1;//跳过RST检查
			}			
		}
		ZGRST_wait_cnt++;
		Delay(2000);
		if((ZGRST_wait_cnt%2)==1)
		{
			DebugPrint_uart("** Wait for ZigBee module RST.\r\n",0x200);		
		}
		if(ZGRST_wait_cnt>=10)
		{
			ZGRST_wait_cnt=0;
			goto ZGRST_Retry;
		}
	}	
	ZGRST_OK_Flag=1;
	DebugPrint_uart("** ZigBee net RST OK.\r\n",0x200);
	
	{//收集Zigbee网络信息
	  //device 有效数量，标记ID是否在线，同时读取MAC对应到ID
	  zigbee_Refesh_OnlineDeviceNUM();
	

	}
	DebugPrint_uart("||----ZigBee net initialization completed----||\r\n\r\n",0x200);//完成	
	return 0;
}

uint8_t Recv_BUFF_cc2530[RecvBUFF_cc2530_MAX]={0};
 /**
 * @brief  串口发送,解耦硬件层和软件驱动
 * @param	char Send_temp,uint16_t Send_len_t;发送内容和数据长度
 * @retval	void
 * @author ZCD
 * @Time 2021年11月26日
*/
int8_t Uart_Send_cc2530(char* Send_temp,uint16_t Send_len_t)
{
	
	UART_Send(&huart3,Send_temp,Send_len_t,0x400);
	return 0;
}
 /**
 * @brief  串口数据转存
 * @param	viod
 * @retval	void
 * @author ZCD
 * @Time 2021年11月26日
*/
void UartBUFF_TranSave_cc2530(void)
{
	char head_t[2]={CMD_Head1_cc2530,CMD_Head2_cc2530};
	StringSearch_Assign(head_t,Rx_Buff3,2,100);//寻找数据头
	for(uint8_t j=0;j<RecvBUFF_cc2530_MAX;j++)
	{
		Recv_BUFF_cc2530[j]=Rx_Buff3[j+StringSecanPointer_Assign_Buff];
	}	
	Rx_Buff3[StringSecanPointer_Assign_Buff]=0;//销毁头标志
	Rx_Buff3[StringSecanPointer_Assign_Buff+1]=0;
}
/**
 * @brief  头尾校验
 * @param	viod
 * @retval	uint8_t return //0表示不正确，1表示校验成功
 * @author ZCD
 * @Time 2021年11月26日
*/
uint16_t RetData_Len=0;
uint8_t zigbee_Head_End_Check(void)
{
	uint8_t Check_cnt_t =0;
	if(Recv_BUFF_cc2530[0] == CMD_Head1_cc2530)
	{Check_cnt_t++;}
	if(Recv_BUFF_cc2530[1] == CMD_Head2_cc2530)
	{Check_cnt_t++;}	
	uint16_t End_point_t =0;
	while(1)
	{
		if((Recv_BUFF_cc2530[End_point_t]==CMD_End1_cc2530)&&(Recv_BUFF_cc2530[End_point_t+1]==CMD_End2_cc2530))
		{
			RetData_Len=End_point_t;
			//End_point_t =0;
			Check_cnt_t+=2;
			break;
		}
		End_point_t++;
		if((End_point_t+1)>=RecvBUFF_cc2530_MAX)
		{
			break;//接收缓冲区空间不足
		}
	}
	if(Check_cnt_t==4)
	{//数据头尾正常
		return 1;
	}
	else
	{return 0;}
}
 /**
 * @brief  读取数据，并转存到Sensor_BUFF_T
 * @param	char Send_temp,uint16_t Send_len_t;发送内容和数据长度
 * @retval	void
 * @author ZCD
 * @Time 2021年11月26日
*/
uint8_t Device_BUFF_T[RecvBUFF_cc2530_MAX]={0};
int8_t zigbee_ReadData(uint8_t Device_num,uint8_t Sensor_type)
{
	char Send_Buff_t[6]={CMD_Head1_cc2530,CMD_Head2_cc2530,Device_num,Sensor_type,CMD_End1_cc2530,CMD_End2_cc2530};
	Uart_Send_cc2530(Send_Buff_t,6);
	
	UART3_Refresh_Flag=0;//等待串口返回
	if(Wait_Until_FlagTrue(&UART3_Refresh_Flag,5000))
	{//5s内收到返回数据
		//数据转存
		UartBUFF_TranSave_cc2530();//从buff中转存到Recv_BUFF_cc2530
		if(zigbee_Head_End_Check())
		{//数据格式校验通过
			for(uint8_t i=0;i< Val_MAX_Limit_U(RetData_Len,RecvBUFF_cc2530_MAX);i++)
			{
				Device_BUFF_T[i]=Recv_BUFF_cc2530[i+2];//有效数据转存
				Recv_BUFF_cc2530[i+2]=0;//数据清空
				Recv_BUFF_cc2530[0]=0;
				Recv_BUFF_cc2530[1]=0;
			}

			return 0;//现在那个cc2530还没有收到后的返回值，这里只能直接返回	
		}	
		else
		{return 2;}//数据格式错误
	}
	else
	{
		return 1;//数据接收超时
	}
}
 /**
 * @brief  读取传感器数据
 * @param	char Send_temp,uint16_t Send_len_t;发送内容和数据长度
 * @retval	void
 * @author ZCD
 * @Time 2021年11月26日
*/
float Temperture_D=0.0;
uint32_t RH_D=0;
uint16_t CO2_D=0;
uint16_t NH3_D=0;
uint16_t Lux_D=0;

void zigbee_Read_Sensor(uint8_t Device_num,uint8_t Sensor_type,Device_t *DeviceSave_t)
{
	if(DeviceOnline_Falg[Device_num-1]!=1)
	{//ID对应的Device未注册
		return ;
	}
	else
	{
		if(Sensor_type==0x05)//温度 3字节
		{
			zigbee_ReadData(Device_num,Sensor_type);
			uint32_t temper_t=0;
			if(Device_BUFF_T[1]!=0xf4)
			{
				temper_t=(uint32_t)Device_BUFF_T[3]<<16|(uint32_t)Device_BUFF_T[4]<<8|(uint32_t)Device_BUFF_T[5];
				temper_t=temper_t&0xffff;				
			}
			else
			{ temper_t=0;}
			Temperture_D=((temper_t/1048576)*200)-50;
		}
		else if(Sensor_type==0x04)//湿度 3字节
		{
			zigbee_ReadData(Device_num,Sensor_type);
			uint32_t rh_t=0;
			if(Device_BUFF_T[1]!=0xf4)
			{
			rh_t=(uint32_t)Device_BUFF_T[3]<<16|(uint32_t)Device_BUFF_T[4]<<8|(uint32_t)Device_BUFF_T[5];
			rh_t=rh_t>>4;
			}
			else
			{rh_t=0;}
			RH_D=((rh_t/1048576)*200)*100;		
		}
		else if(Sensor_type==0x03)//光照 2字节
		{
			uint32_t lux_t=0;
			if(zigbee_ReadData(Device_num,Sensor_type))
			{//传感器离线
				lux_t=0;
			}
			else
			{
				lux_t=(uint32_t)Device_BUFF_T[4]<<8|(uint32_t)Device_BUFF_T[5];			
			}
			Lux_D=(lux_t*10)/12;
		}
		else if(Sensor_type==0x01)//CO2 2字节
		{	
			uint32_t co2_t=0;
			if(zigbee_ReadData(Device_num,Sensor_type))
			{//传感器离线
				co2_t=0;
			}
			else
			{
				co2_t=(uint32_t)Device_BUFF_T[4]<<8|(uint32_t)Device_BUFF_T[5];			
			}
			CO2_D=co2_t;		
		}
		else if(Sensor_type==0x02)//NH3 2字节
		{
			uint32_t nh3_t=0;
			if(zigbee_ReadData(Device_num,Sensor_type))
			{//传感器离线
				nh3_t=0;
			}
			else
			{
				nh3_t=(uint32_t)Device_BUFF_T[4]<<8|(uint32_t)Device_BUFF_T[5];			
			}
			NH3_D=nh3_t/100;		
		}	
		{//把数据转存到指定的结构体一份
			DeviceSave_t[Device_num-1].Temperture=Temperture_D;
			DeviceSave_t[Device_num-1].RH=RH_D;
			DeviceSave_t[Device_num-1].NH3=NH3_D;
			DeviceSave_t[Device_num-1].Lux=Lux_D;
			DeviceSave_t[Device_num-1].CO2=CO2_D;
			//转存完成后数据清空
			for(uint8_t i=0;i<RecvBUFF_cc2530_MAX;i++)
			{
				Device_BUFF_T[i]=0;
			}
		}	
		
	}

}

/**
 * @brief  读取device MAC 并对应到deviceID
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年11月30日
*/
Device_t DeviceSNList[6]={0};

uint8_t zigbee_Read_MAC(uint8_t device_ID,Device_t *DeviceList_t)
{
	if( zigbee_ReadData(device_ID,0x00))
	{//读取MAC超时或者格式错误
		return 1;
	}
	for(uint8_t j=0;j<16;j++)
	{
		DeviceList_t[device_ID-1].DeviceSN[j]='0';
	}	
	for(uint8_t i=0;i<8;i++)
	{
		DeviceList_t[device_ID-1].DeviceSN[i]=Device_BUFF_T[i];
	}
	DeviceList_t[device_ID-1].DeviceID=device_ID;
	//DebugPrint_uart("** ",200);
	//UART_Send(&huart2,DeviceList_t[device_ID-1].DeviceSN,16,200);//debug使用
	return 0;

}

 /**
 * @brief  读取全部的传感器信息
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年12月4日
*/
uint8_t ZigbeeRead_ALL_Sensor(void)
{
	for(uint8_t i=1;i<=DeviceNUM_MAX;i++)
	{
		for(uint8_t j=1;j<=4;j++)
		{
			zigbee_Read_Sensor(i,j,DeviceSNList);
		}
	}
	return 0;
}
 /**
* @brief  发送后清空结构体数据
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年12月4日
*/
uint8_t zigbeeClear_StructData(uint8_t Device_ID_t)
{
	DeviceSNList[Device_ID_t-1].CO2=0;
	DeviceSNList[Device_ID_t-1].Lux=0;
	DeviceSNList[Device_ID_t-1].NH3=0;
	DeviceSNList[Device_ID_t-1].RH=0;
	DeviceSNList[Device_ID_t-1].Temperture=0;
	return 0;
}

