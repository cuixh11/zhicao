 /**
 * @brief  cc2530��صĿ��ƣ��ǹٷ��̼���
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��11��26��
*/
//cc2530 ������Ԥ����UART4
#include "cc2530.h"
#include "ESP8266.h"//ֻ�������е�StringSearch_Assign()���������������ַ�����

//#include "http_json.h"
//#include "host_app.h"


uint8_t DeviceOnline_Falg[DeviceNUM_MAX]={0};
 /**
 * @brief  ZigBee Device num refesh
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��12��3��
*/
uint8_t zigbee_Refesh_OnlineDeviceNUM(void)
{
	for(uint8_t i=1;i<=DeviceNUM_MAX;i++)
	{
		if (zigbee_Read_MAC(i,DeviceSNList))
		{//��ȡʧ�ܣ���Ϊ�ն�����
			DeviceOnline_Falg[i-1]=0;
		}
		else
		{
			//if((Device_BUFF_T[1]==0xFE)&&(Device_BUFF_T[5]==i))
			if(!((Device_BUFF_T[1]==0xFE)||(Device_BUFF_T[1]==0xff)))
			{//�ն��Ѿ�ע��
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
 * @Time 2021��12��3��
*/
uint8_t ZGRST_OK_Flag=0;
uint8_t ZigBeeNET_Init(void)
{
	uint8_t ZGRST_wait_cnt=0;
	if(GPIO_Read(GPIOF,GPIO_PIN_2)==RESET)
	{//key3 ���£�ֱ������ZigBeeNET_Init
		ZGRST_OK_Flag=1;
		zigbee_Refesh_OnlineDeviceNUM();
		DebugPrint_uart("||----ZigBee net initialization completed----||\r\n\r\n",0x200);//���	
		return 1;//����RST���
	}
	
	ZGRST_Retry:
	{//Э������λ
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET);	
		Delay(100);
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET);		
	}
	//�ȴ���λ���
	while(!StringSearch_Assign("say Hello",Rx_Buff3,9,100))
	{
		{//�������ZigBeeNET_Init
			if(GPIO_Read(GPIOF,GPIO_PIN_2)==RESET)
			{//key3 ���£�ֱ������ZigBeeNET_Init
				ZGRST_OK_Flag=1;
				zigbee_Refesh_OnlineDeviceNUM();
				DebugPrint_uart("||----ZigBee net initialization completed----||\r\n\r\n",0x200);//���	
				ZGRST_wait_cnt=0;
				return 1;//����RST���
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
	
	{//�ռ�Zigbee������Ϣ
	  //device ��Ч���������ID�Ƿ����ߣ�ͬʱ��ȡMAC��Ӧ��ID
	  zigbee_Refesh_OnlineDeviceNUM();
	

	}
	DebugPrint_uart("||----ZigBee net initialization completed----||\r\n\r\n",0x200);//���	
	return 0;
}

uint8_t Recv_BUFF_cc2530[RecvBUFF_cc2530_MAX]={0};
 /**
 * @brief  ���ڷ���,����Ӳ������������
 * @param	char Send_temp,uint16_t Send_len_t;�������ݺ����ݳ���
 * @retval	void
 * @author ZCD
 * @Time 2021��11��26��
*/
int8_t Uart_Send_cc2530(char* Send_temp,uint16_t Send_len_t)
{
	
	UART_Send(&huart3,Send_temp,Send_len_t,0x400);
	return 0;
}
 /**
 * @brief  ��������ת��
 * @param	viod
 * @retval	void
 * @author ZCD
 * @Time 2021��11��26��
*/
void UartBUFF_TranSave_cc2530(void)
{
	char head_t[2]={CMD_Head1_cc2530,CMD_Head2_cc2530};
	StringSearch_Assign(head_t,Rx_Buff3,2,100);//Ѱ������ͷ
	for(uint8_t j=0;j<RecvBUFF_cc2530_MAX;j++)
	{
		Recv_BUFF_cc2530[j]=Rx_Buff3[j+StringSecanPointer_Assign_Buff];
	}	
	Rx_Buff3[StringSecanPointer_Assign_Buff]=0;//����ͷ��־
	Rx_Buff3[StringSecanPointer_Assign_Buff+1]=0;
}
/**
 * @brief  ͷβУ��
 * @param	viod
 * @retval	uint8_t return //0��ʾ����ȷ��1��ʾУ��ɹ�
 * @author ZCD
 * @Time 2021��11��26��
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
			break;//���ջ������ռ䲻��
		}
	}
	if(Check_cnt_t==4)
	{//����ͷβ����
		return 1;
	}
	else
	{return 0;}
}
 /**
 * @brief  ��ȡ���ݣ���ת�浽Sensor_BUFF_T
 * @param	char Send_temp,uint16_t Send_len_t;�������ݺ����ݳ���
 * @retval	void
 * @author ZCD
 * @Time 2021��11��26��
*/
uint8_t Device_BUFF_T[RecvBUFF_cc2530_MAX]={0};
int8_t zigbee_ReadData(uint8_t Device_num,uint8_t Sensor_type)
{
	char Send_Buff_t[6]={CMD_Head1_cc2530,CMD_Head2_cc2530,Device_num,Sensor_type,CMD_End1_cc2530,CMD_End2_cc2530};
	Uart_Send_cc2530(Send_Buff_t,6);
	
	UART3_Refresh_Flag=0;//�ȴ����ڷ���
	if(Wait_Until_FlagTrue(&UART3_Refresh_Flag,5000))
	{//5s���յ���������
		//����ת��
		UartBUFF_TranSave_cc2530();//��buff��ת�浽Recv_BUFF_cc2530
		if(zigbee_Head_End_Check())
		{//���ݸ�ʽУ��ͨ��
			for(uint8_t i=0;i< Val_MAX_Limit_U(RetData_Len,RecvBUFF_cc2530_MAX);i++)
			{
				Device_BUFF_T[i]=Recv_BUFF_cc2530[i+2];//��Ч����ת��
				Recv_BUFF_cc2530[i+2]=0;//�������
				Recv_BUFF_cc2530[0]=0;
				Recv_BUFF_cc2530[1]=0;
			}

			return 0;//�����Ǹ�cc2530��û���յ���ķ���ֵ������ֻ��ֱ�ӷ���	
		}	
		else
		{return 2;}//���ݸ�ʽ����
	}
	else
	{
		return 1;//���ݽ��ճ�ʱ
	}
}
 /**
 * @brief  ��ȡ����������
 * @param	char Send_temp,uint16_t Send_len_t;�������ݺ����ݳ���
 * @retval	void
 * @author ZCD
 * @Time 2021��11��26��
*/
float Temperture_D=0.0;
uint32_t RH_D=0;
uint16_t CO2_D=0;
uint16_t NH3_D=0;
uint16_t Lux_D=0;

void zigbee_Read_Sensor(uint8_t Device_num,uint8_t Sensor_type,Device_t *DeviceSave_t)
{
	if(DeviceOnline_Falg[Device_num-1]!=1)
	{//ID��Ӧ��Deviceδע��
		return ;
	}
	else
	{
		if(Sensor_type==0x05)//�¶� 3�ֽ�
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
		else if(Sensor_type==0x04)//ʪ�� 3�ֽ�
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
		else if(Sensor_type==0x03)//���� 2�ֽ�
		{
			uint32_t lux_t=0;
			if(zigbee_ReadData(Device_num,Sensor_type))
			{//����������
				lux_t=0;
			}
			else
			{
				lux_t=(uint32_t)Device_BUFF_T[4]<<8|(uint32_t)Device_BUFF_T[5];			
			}
			Lux_D=(lux_t*10)/12;
		}
		else if(Sensor_type==0x01)//CO2 2�ֽ�
		{	
			uint32_t co2_t=0;
			if(zigbee_ReadData(Device_num,Sensor_type))
			{//����������
				co2_t=0;
			}
			else
			{
				co2_t=(uint32_t)Device_BUFF_T[4]<<8|(uint32_t)Device_BUFF_T[5];			
			}
			CO2_D=co2_t;		
		}
		else if(Sensor_type==0x02)//NH3 2�ֽ�
		{
			uint32_t nh3_t=0;
			if(zigbee_ReadData(Device_num,Sensor_type))
			{//����������
				nh3_t=0;
			}
			else
			{
				nh3_t=(uint32_t)Device_BUFF_T[4]<<8|(uint32_t)Device_BUFF_T[5];			
			}
			NH3_D=nh3_t/100;		
		}	
		{//������ת�浽ָ���Ľṹ��һ��
			DeviceSave_t[Device_num-1].Temperture=Temperture_D;
			DeviceSave_t[Device_num-1].RH=RH_D;
			DeviceSave_t[Device_num-1].NH3=NH3_D;
			DeviceSave_t[Device_num-1].Lux=Lux_D;
			DeviceSave_t[Device_num-1].CO2=CO2_D;
			//ת����ɺ��������
			for(uint8_t i=0;i<RecvBUFF_cc2530_MAX;i++)
			{
				Device_BUFF_T[i]=0;
			}
		}	
		
	}

}

/**
 * @brief  ��ȡdevice MAC ����Ӧ��deviceID
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��11��30��
*/
Device_t DeviceSNList[6]={0};

uint8_t zigbee_Read_MAC(uint8_t device_ID,Device_t *DeviceList_t)
{
	if( zigbee_ReadData(device_ID,0x00))
	{//��ȡMAC��ʱ���߸�ʽ����
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
	//UART_Send(&huart2,DeviceList_t[device_ID-1].DeviceSN,16,200);//debugʹ��
	return 0;

}

 /**
 * @brief  ��ȡȫ���Ĵ�������Ϣ
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��12��4��
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
* @brief  ���ͺ���սṹ������
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��12��4��
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

