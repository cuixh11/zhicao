#include "SmartNest_server.h"


//-----------------------GET--------------------------//
char Reg_new[]="GET /api/reg/user?name=tester1&password=123456789 HTTP/1.1\r\n";
char Login[]="GET /api/login/user?name=tester1&password=123456789 HTTP/1.1\r\n";
char Query_Host[]="GET /api/query/host?num=-1 HTTP/1.1\r\n";//����������ѯ�ӿ����������ò���


//----------------------POST--------------------------//

char Post_test[]="POST /api/upload/test HTTP/1.1\r\n";
char Post_RegHost[]="POST /api/reg/host HTTP/1.1\r\n";
char Post_RegDevice[]="POST /api/reg/devices HTTP/1.1\r\n";
char Post_UploadData_SN[]="POST /api/upload/devices/data/sn HTTP/1.1\r\n";
char Post_PUSH[]="POST /api/msg/push HTTP/1.1\r\n";

//----------------------------------------------------//
/**
* @brief  Send_to_Server		//send,���ͺ���;wifiģ��ʹ�ô���1
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��10��
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
* @brief  Post_to_Server		//post send,���ͺ���ִ�к�����body��head�Ļ�����
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��10��
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
	
	UART_Send(&huart1,Server_SendBuff,strlen(Server_SendBuff),0x400);//�˴��ľ�����char��uint8_t,ʵ��������
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
	//UART_Send(&huart4,Server_SendBuff,strlen(Server_SendBuff),0x400);//�˴��ľ�����char��uint8_t,ʵ��������
	Clear_Buff(Server_SendBuff,strlen(Server_SendBuff),Server_SEND_BuffLen_MAX);
	Clear_Buff(Head_Send_Buff,Head_Send_BuffPoint,Head_Len_MAX);
	Clear_Buff(Json_packed_BUFF,strlen(Json_packed_BUFF)+1,Json_packed_BUFF_MAX);

	return 0;
}
/**
 * @brief  Register_Host 	//ע�������ӿ�
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��11��
*/
int8_t Register_Host(void)
{
	
	Pack_return=JsonPack("reg_host");		
	Body_head_Make(Post_RegHost,sizeof(Post_RegHost),1,strlen(Json_packed_BUFF));

	Post_to_Server();
	
	UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//���ʱʱ��5s
	Body_Process();
	if(BodyHead_pro.State_code==200)//���������Ƿ�Ϊ��������
	{//��
		Unpack_return=JsonUnpack("reg_host");
		if(Unpack_return==0)//ע��ɹ�
		{
			json_unpack_copy(Host_sn.Str,Host_sn.Str_p);	
			json_unpack_copy(Host_type.Str,Host_type.Str_p);		
			DebugPrint_uart("\r\n** Host registration succeeded.",0x200);
		}
		else if(Unpack_return == -4)//host_sn �Ѿ���ע��
		{
			DebugPrint_uart("\r\n** This host is already registered or SN error.",0x200);		
			Json_Delete_Enable_Flag=1;
			json_unpack_copy(0,0);
		}
		else if(Unpack_return == -8)//������json���ղ�ȫ���߽���ʧ��
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
 * @brief  Login User  	//Login�ӿ�,��¼���Զ������µ�utoken
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��14��
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
	Send_to_Server(Get_Send_Buff);//����get����	
	
	UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//���ʱʱ��5s//��ʱû���������ʱδ����
	
	Unpack_return=JsonUnpack("Login");
	NetTime_Refesh();
	if(BodyHead_pro.State_code==200)//���������Ƿ�Ϊ��������
	{//��
		if(Unpack_return==0)//��¼�ɹ�
		{
			json_unpack_copy(userName.Str,userName.Str_p);	
			//json_unpack_copy(Host_type.Str,Host_type.Str_p);		
			BodyHead_pro.utoken_Live=1;
			
			Flash_ValidMark_Equilibrium(BodyHead_pro.utoken_Live);
			return 0;		
		}	
		else if(Unpack_return==-4)//��¼ʧ��,����head��body��token��һ�£����ش���
		{		
			return 2;
		}
		else if(Unpack_return==-3)//��¼ʧ�ܣ��û����������
		{
			
			json_unpack_copy(userName.Str,userName.Str_p);	
			json_unpack_copy(Password.Str,Password.Str_p);	//���صĴ����û���������
			if(Registed_User==0)
			{//�״ε�¼
				return 1;
			}
			else
			{//Flash д����˻������룬����Ŀǰ�����޸ĵȵ��²���ȷ
				Changed_UserInfo=1;
				
				return 5;
			}
				
			
		}
		else	
		{
			DebugPrint_uart("\r\n** Unknown state error.",0x200);
			return 3;//δ֪�Ĵ���
		}	
	}
	else//���������ӡ����ش���
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
 * @Time 2021��11��18��
*/
int8_t Register_Device(void)
{
	Pack_return=JsonPack("reg_device");		
	Body_head_Make(Post_RegDevice,sizeof(Post_RegDevice),1,strlen(Json_packed_BUFF));

	Post_to_Server();
	
	UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//���ʱʱ��5s
	Body_Process();
	if(BodyHead_pro.State_code==200)//���������Ƿ�Ϊ��������
	{//��
		Unpack_return=JsonUnpack("reg_device");
		if(Unpack_return==0)//ע��ɹ�
		{
			json_unpack_copy(Device_Suc_SN.Str,Device_Suc_SN.Str_p);	
			json_unpack_copy(Device_Err_SN.Str,Device_Err_SN.Str_p);	
			
			if(deviceReg_ErrCNT==0)
			{//ȫ��ע��ɹ�
				DebugPrint_uart("\r\n** All devices are registered successfully.",0x200);				
			}
			else if(deviceReg_SucCNT==0)
			{//ȫ��ע��ʧ��
				DebugPrint_uart("\r\n** Failed to register all devices.",0x200);			
			}
			else	
			{//����ע��ɹ�
				DebugPrint_uart("\r\n** Device partially registered successfully.",0x200);
			}

		}
		else if(Unpack_return == -4)//hostδע��
		{
			DebugPrint_uart("\r\n** This host was not registered.",0x200);		
		}
		else if(Unpack_return == -8)//������json���ղ�ȫ���߽���ʧ��
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
 * @Time 2021��11��18��
*/
int Upload_pressTest_CNT=0;
char UploadData_buff[Json_packed_BUFF_MAX-100]={0};
char Upload_DataTemp[83]="Time=2021/12/31/00:00:00, TEMP= 00.0 ,RH= 000% ,Eav= 00000 ,CO2= 0000 ,NH3= 0000,\n";
char Upload_DataSend[73]="Time=2021/12/31/00:00:00,TEMP=00.0,RH=000%,Eav=00000,CO2=0000,NH3=0000,\n";//�����������Ϊǰ�˴���ո�Ӱ�����ܣ�����ʵ�ʷ���������������飻���������������һ��ӹ�
//char Upload_DataTemp[]="abc1\nabc1\nabc1\nabc1\nabc1\nabc1\nabc1\nabc1\nabc1\nabc1\n";
void Upload_Data_tempProcess(void)//��������������������ϴ��ַ���
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
	{//ʹ��ջ�ڱ����ʱ���
		StackTimeStamp_Flag=0;
	}
	else
	{
		Load_TimeStamp(Upload_DataTemp);//��ʱ���	
	}
	Upload_Data_tempProcess();
	strcpy(UploadData_buff,Upload_DataSend);
	
	Pack_return=JsonPack("upload_data");	
	Body_head_Make(Post_UploadData_SN,sizeof(Post_UploadData_SN),1,strlen(Json_packed_BUFF));
	Post_to_Server();
	
	UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//���ʱʱ��5s
	Body_Process();
	if(BodyHead_pro.State_code==200)//���������Ƿ�Ϊ��������
	{//��
		Unpack_return=JsonUnpack("upload_data");
		if(Unpack_return==0)//�����ϴ��ɹ�
		{
			Upload_pressTest_CNT++;
			UART_Print_Number(Upload_pressTest_CNT);
			return 0;//�����ϴ��ɹ�
		}
		else if(Unpack_return == -4)//host����deviceδע��
		{
			DebugPrint_uart("\r\n** This host or device was not registered.",0x200);	
			return 3;//�������ߴӻ�δע��
		}
		else if(Unpack_return == -8)//������json���ղ�ȫ���߽���ʧ��
		{
			DebugPrint_uart("\r\n** Server JSON parsing failed.",0x200);
			return 4;//�����ύ���󷵻�			
		}
		else if(Unpack_return==-9)
		{
			DebugPrint_uart("\r\n** Slave is registered, data submission failed.",0x200);
			return 5;//�����ύ���󷵻�			
		}
		else
		{
			DebugPrint_uart("\r\n** Unknown state error.",0x200);
			return 1;//�����ύ���󷵻�
		}
	}
	else
	{
		DebugPrint_uart_LineFeed("** Connection error with server.",200);
		return 2;//������ͨ�Ŵ��󣨶������߷��ʴ���
	}
}
/**
 * @brief Upload_DeviceData by NBIOT 
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021��12��4��
*/

int8_t Upload_DeviceData_NBIOT(void)
{
	Load_TimeStamp(Upload_DataTemp);//��ʱ���
	Upload_Data_tempProcess();
	strcpy(UploadData_buff,Upload_DataSend);
	
	Pack_return=JsonPack("upload_data");	
	Body_head_Make(Post_UploadData_SN,sizeof(Post_UploadData_SN),1,strlen(Json_packed_BUFF));
	Post_to_Server_NBIOT();
	
	UART4_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
	Wait_Until_FlagTrue(&UART4_Refresh_Flag,5000);//���ʱʱ��5s
	Delay(1000);
	if(NBIOT_Received_Flag==1)
	{	NBIOT_Received_Flag=0;	}
	else
	{
		return 2;
	}
	Body_Process();
	if(BodyHead_pro.State_code==200)//���������Ƿ�Ϊ��������
	{//��
		Unpack_return=JsonUnpack("upload_data");
		if(Unpack_return==0)//�����ϴ��ɹ�
		{
			Upload_pressTest_CNT++;
			UART_Print_Number(Upload_pressTest_CNT);
			return 0;//�����ϴ��ɹ�
		}
		else if(Unpack_return == -4)//host����deviceδע��
		{
			DebugPrint_uart("\r\n** This host or device was not registered.",0x200);	
			return 3;//�������ߴӻ�δע��
		}
		else if(Unpack_return == -8)//������json���ղ�ȫ���߽���ʧ��
		{
			DebugPrint_uart("\r\n** Server JSON parsing failed.",0x200);
			return 4;//�����ύ���󷵻�			
		}
		else if(Unpack_return==-9)
		{
			DebugPrint_uart("\r\n** Slave is registered, data submission failed.",0x200);
			return 5;//�����ύ���󷵻�			
		}
		else
		{
			DebugPrint_uart("\r\n** Unknown state error.",0x200);
			return 1;//�����ύ���󷵻�
		}
	}
	else
	{
		DebugPrint_uart_LineFeed("** Connection error with server.",200);
		return 2;//������ͨ�Ŵ��󣨶������߷��ʴ���
	}	
	
}
/**
 * @brief  Time update by http header  	//
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��14��
*/
int8_t NetTime_Refesh(void)
{
	//�ӷ��ص�head�л�ȡʱ��
	uint8_t str_point_t=0;
	if(StringSearch("\r\nDate: ",8,100))
	{
		str_point_t=StringSecanPointer+8;
		
		{//Weekday
			if(Rx_Buff1[str_point_t]=='S')//�ܼ��� S��ͷ
			{
				if(Rx_Buff1[str_point_t+1]=='u')//�м���ĸ
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
			if(Rx_Buff1[str_point_t]=='J')//�·�J��ͷ  1 6 7
			{
				if(Rx_Buff1[str_point_t+1]=='a')//�м���ĸ
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
				if(Rx_Buff1[str_point_t+1]=='a')//�м���ĸ
				{//Mar
					month=3;
				}				
				else if(Rx_Buff1[str_point_t+2]=='y')//�����ĸ
				{//May
					month=5;
				}
			}
			else if(Rx_Buff1[str_point_t]=='A')//4 8
			{
				if(Rx_Buff1[str_point_t+1]=='p')//�м���ĸ
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
 * @brief Load SN//��оƬID���ص�SN
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��15��
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
 * @brief Load host name//��ע������ʱ����host name
 * @param	void
 * @retval 1 ��ʾ��0��ʾ����
 * @author ZCD1300
 * @Time 2021��11��15��
*/
int8_t Load_host_name(char *hostName_t)
{
	uint8_t HostName_Len_t =strlen(hostName_t);
	if(HostName_Len_t>20)
	{
		return 1;//��������
	}
	for(uint8_t i=0;i<HostName_Len_t;i++)
	{
		Host_Name[i]=hostName_t[i];
	}
	return 0;
}
/**
 * @brief Load time stamp//���ַ�������ʱ���
 * @param	char *OperationBuff
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��19��
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
 * @brief Connect to server// ���ӵ�������
 * @param	void 
 * @retval int8_t 
 * @author ZCD1300
 * @Time 2021��11��19��
*/
uint8_t ConnetServer_Overtime_Flag=0;
int8_t Connect_Server_Smatnest(void)
{
	if(Close_Connection_Return==1)
	{//δ�˳�͸��ģʽ
		retryClose_ConnectServerSmatnest:
		Wifi_ExitTransp_Send();//�˳�͸��
		if(Wifi_CLOSE_Connection(0)!=0)//�ر�ȫ������
		{//δ�ر�����
			goto retryClose_ConnectServerSmatnest;
		}	
	}
	{//���ӵ�������
		wifi_build_connect(1,SmartNest_Domain,SmartNest_DomainLEN);//���ӵ�������
		uint8_t Connected_To_Server_Print_Cnt=0;
		uint8_t ConnectServer_retryCNT=0;

		while(!StringSearch("NECT\r\n\r\nOK",10,150))//�ȴ��������������������
		{//δ��������
			Delay(100);
			Connected_To_Server_Print_Cnt++;
			if((Connected_To_Server_Print_Cnt/3)==1)
			{
				Connected_To_Server_Print_Cnt=0;
				ConnectServer_retryCNT++;
				DebugPrint_uart("** Connecting to server.\r\n",0x200);
			}
			if(ConnectServer_retryCNT>10)//��������������ӷ���������
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
 * @brief Load sensor data //���ַ������ش���������
 * @param	char *OperationBuff
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��19��
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
 * @brief Load device SN//��deviceоƬID���ص�SN
 * @param	uint8_t DeviceID_t ָ����device ID
 * @retval void
 * @author ZCD1300
 * @Time 2021��12��1��
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
 * @brief Regest all device SN//ע������device SN
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��12��4��
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
			Load_DeviceSN(i+1);	//���+1����Ϊdevice ID��1��ʼ
			Register_Device();//ֻע��һ��device��
		}
	}
	DebugPrint_uart(temp_t1,200);//��ӡ�Ѿ�ע��ı��
	DebugPrint_uart(temp_t2,200);
}
/**
* @brief PUSH Warning//���������͹���
 * @param	void
 * @retval uint8_t return 
 * @author ZCD1300 
 * @Time 2021��12��11��
*/
int8_t PUSH_Serve(void)
{
	Pack_return=JsonPack("warningPUSH");		
	Body_head_Make(Post_PUSH,sizeof(Post_PUSH),1,strlen(Json_packed_BUFF));
	Post_to_Server();
	
	UART1_Refresh_Flag=0;//��⴮�ڴ���ǰҪ��λ
	Wait_Until_FlagTrue(&UART1_Refresh_Flag,5000);//���ʱʱ��5s
	Body_Process();
	if(BodyHead_pro.State_code==200)//���������Ƿ�Ϊ��������
	{//��
		Unpack_return=JsonUnpack("warningPUSH");
		if(Unpack_return==0)//�������ͳɹ�
		{
			DebugPrint_uart("\r\n** Warning PUSH Successed.\r\n",0x200);	
			return 0;
		}
		else if(Unpack_return == -2)//�û�δ��¼
		{
			DebugPrint_uart("\r\n** No user login.",0x200);	
			return 3;//�������ߴӻ�δע��
		}		
		else if(Unpack_return == -8)//������json���ղ�ȫ���߽���ʧ��
		{
			DebugPrint_uart("\r\n** Server JSON parsing failed.",0x200);
			return 4;//�����ύ���󷵻�			
		}
		else if(Unpack_return==-9)
		{
			DebugPrint_uart("\r\n** Slave is registered, data submission failed.",0x200);
			return 5;//�����ύ���󷵻�			
		}
		else
		{
			DebugPrint_uart("\r\n** Unknown state error.",0x200);
			return 1;//�����ύ���󷵻�
		}
	}
	else
	{
		DebugPrint_uart_LineFeed("** Connection error with server.",200);
		return 2;//������ͨ�Ŵ��󣨶������߷��ʴ���
	}

}
