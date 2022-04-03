#include "http_json.h"
#include "SmartNest_server.h"



 /**
 * @brief  �������ı�����Ȳ���������1���ջ����С
 * @param	uint16_t input ����Ҫ���Ƶ���ֵ
 * @retval ���ط�Χ�ڵ���ֵ
 * @author ZCD1300
 * @Time 2021��11��7��
*/
uint16_t MAXBuff_Limt(uint16_t input )
{
	if(input>BuffMAX)
	{return BuffMAX;}
	else
	{return input;}
}

/**
 * @brief  utoken Iput Test 	//�ֶ��涨һ��token����������ʹ�ã�
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��10��
*/
void utoken_input_manually(char * utoken_temp)
{
		for(uint8_t i=0;i<32;i++)
		{
			BodyHead_pro.token[i]=utoken_temp[i];
		}

}

/**
 * @brief  Body head make		//׼��
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��9��
*/

char Head_Send_Buff[Head_Len_MAX]={0};//make��ͷ�Ļ�����
uint16_t Head_Send_BuffPoint=0;//ͷ������ָ��ƫ��
uint8_t Body_head_Make(char *CMD,uint16_t CMD_Len ,uint8_t Content_Type,uint16_t Content_Length)
{
	Head_Send_BuffPoint=CMD_Len-1;
	strcpy(Head_Send_Buff,CMD);
	
	strcpy(Head_Send_Buff+Head_Send_BuffPoint,"User-Agent: SmartNest-Host/1.4.9\r\n");
	Head_Send_BuffPoint=Head_Send_BuffPoint+34;
		
	if(Content_Type==1)
	{
		strcpy(Head_Send_Buff+Head_Send_BuffPoint,"Content-Type: application/json\r\n");
		Head_Send_BuffPoint=Head_Send_BuffPoint+32;
	}
	strcpy(Head_Send_Buff+Head_Send_BuffPoint,"Host: smartnestxdu.top\r\n");
	Head_Send_BuffPoint=Head_Send_BuffPoint+24;
	
	char Len_temp[25];
	if(Content_Length>99)//3λ�� 100-999
	{	
		strcpy(Len_temp,"Content-Length: 999\r\n");
		//Len_temp="Content-Length: 999\r\n";		
		Len_temp[16]=(Content_Length/100)+48;
		Len_temp[17]=((Content_Length%100)/10)+48;
		Len_temp[18]=(Content_Length%10)+48;		
		strcpy(Head_Send_Buff+Head_Send_BuffPoint,Len_temp);
		Head_Send_BuffPoint=Head_Send_BuffPoint+21;
	}
	else if(Content_Length>9)//��λ�� 10-99
	{
		strcpy(Len_temp,"Content-Length: 99\r\n");
		//Len_temp="Content-Length: 99\r\n";		
		Len_temp[16]=(Content_Length/10)+48;
		Len_temp[17]=(Content_Length%10)+48;	
		strcpy(Head_Send_Buff+Head_Send_BuffPoint,Len_temp);
		Head_Send_BuffPoint=Head_Send_BuffPoint+20;		
	}
	else if((Content_Length>=0)&&(Content_Length<=9))//��λ�� 0-9
	{
		strcpy(Len_temp,"Content-Length: 9\r\n");
		//Len_temp="Content-Length: 9\r\n";
		Len_temp[16]=Content_Length+48;		
		strcpy(Head_Send_Buff+Head_Send_BuffPoint,Len_temp);
		Head_Send_BuffPoint=Head_Send_BuffPoint+19;
	}
	
	char temp[6]={0};
	if(StringMatch( BodyHead_pro.token,6,temp,6)==0)//token����
	{
		strcpy(Head_Send_Buff+Head_Send_BuffPoint,"Cookie: utoken=");
		Head_Send_BuffPoint=Head_Send_BuffPoint+15;
		strcpy(Head_Send_Buff+Head_Send_BuffPoint,BodyHead_pro.token);
		Head_Send_Buff[Head_Send_BuffPoint+32]='\r';
		Head_Send_Buff[Head_Send_BuffPoint+33]='\n';
		Head_Send_Buff[Head_Send_BuffPoint+34]='\r';
		Head_Send_Buff[Head_Send_BuffPoint+35]='\n';		
		Head_Send_BuffPoint=Head_Send_BuffPoint+4+32;
	}
	else
	{
		Head_Send_Buff[Head_Send_BuffPoint+1]='\r';
		Head_Send_Buff[Head_Send_BuffPoint+2]='\n';
		Head_Send_Buff[Head_Send_BuffPoint+3]='\r';
		Head_Send_Buff[Head_Send_BuffPoint+4]='\n';			
		Head_Send_BuffPoint=Head_Send_BuffPoint+4;
	}
	

	return 0;
}


/**
 * @brief  Body head process	
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��7��
*/
uint16_t year_token=0,month_token=0,day_token=0,hour_token=0,min_token=0,sec_token=0;
Body_head BodyHead_pro={0};
uint8_t Body_Process(void)
{
	StringSearch("HTTP/1.1 ",9,60);//����״̬ 200 404 502 ��
	BodyHead_pro.State_code=(Rx_Buff1[StringSecanPointer+9]-48)*100+(Rx_Buff1[StringSecanPointer+10]-48)*10+(Rx_Buff1[StringSecanPointer+11]-48);
	Rx_Buff1[StringSecanPointer+9]=0;
	Rx_Buff1[StringSecanPointer+10]=0;
	Rx_Buff1[StringSecanPointer+11]=0;
	StringSearch("Date:",5,MAXBuff_Limt(StringSecanPointer+100));//����
	for(uint16_t i=0;i<29;i++)
	{
		BodyHead_pro.Date[i]=Rx_Buff1[StringSecanPointer+i+6];
	}
	
	StringSearch("nt-Length:",10,MAXBuff_Limt(StringSecanPointer+150));//json����
	if(Rx_Buff1[StringSecanPointer+11+2]>=48)
	{
		BodyHead_pro.Content_Lenth=(Rx_Buff1[StringSecanPointer+11]-48)*100+(Rx_Buff1[StringSecanPointer+12]-48)*10+(Rx_Buff1[StringSecanPointer+13]-48);
	}
	else
	{BodyHead_pro.Content_Lenth=(Rx_Buff1[StringSecanPointer+11]-48)*10+(Rx_Buff1[StringSecanPointer+12]-48);}
	
	if( StringSearch("keep-alive",10,MAXBuff_Limt(StringSecanPointer+50)))//tcp���ӷ�ʽ
	{
		BodyHead_pro.Connection=1;
	}else{BodyHead_pro.Connection=0;}
	
	
	if(StringSearch("et-Cookie:",10,MAXBuff_Limt(StringSecanPointer+20)))
	{
		StringSecanPointer=StringSecanPointer+18;
		for(uint8_t i=0;i<32;i++)
		{
			BodyHead_pro.token[i]=Rx_Buff1[StringSecanPointer+i];
		}
		{//����ʱ��
			if(StringSearch("; Expires=",10,MAXBuff_Limt(StringSecanPointer+64)))
			{
				uint16_t Expires_pointer_t=StringSecanPointer+15;
				{//day
					if((Rx_Buff1[Expires_pointer_t+1]>=48)&&(Rx_Buff1[Expires_pointer_t+1]<58))
					{
						day_token=((Rx_Buff1[Expires_pointer_t]-48)*10)+(Rx_Buff1[Expires_pointer_t+1]-48);
						Expires_pointer_t+=3;
					}		
					else
					{
						day_token=(Rx_Buff1[Expires_pointer_t]-48);
						Expires_pointer_t+=2;
					}
					
				}
				{//month
					if(Rx_Buff1[Expires_pointer_t]=='J')//�·�J��ͷ  1 6 7
					{
						if(Rx_Buff1[Expires_pointer_t+1]=='a')//�м���ĸ
						{//Jan
							month_token=1;
						}
						else if(Rx_Buff1[Expires_pointer_t+1]=='u')
						{//Jun
							month_token=6;
						}
						else if(Rx_Buff1[Expires_pointer_t+2]=='l')
						{//Jul
							month_token=7;
						}				
					}
					else if(Rx_Buff1[Expires_pointer_t]=='M')//3 5
					{
						if(Rx_Buff1[Expires_pointer_t+1]=='a')//�м���ĸ
						{//Mar
							month_token=3;
						}				
						else if(Rx_Buff1[Expires_pointer_t+2]=='y')//�����ĸ
						{//May
							month_token=5;
						}
					}
					else if(Rx_Buff1[Expires_pointer_t]=='A')//4 8
					{
						if(Rx_Buff1[Expires_pointer_t+1]=='p')//�м���ĸ
						{//Apr
							month_token=4;
						}				
						else if(Rx_Buff1[Expires_pointer_t+1]=='u')
						{//Aug
							month_token=8;
						}
					}
					else if(Rx_Buff1[Expires_pointer_t]=='F')//2 F
					{//Feb
						month_token=2;
					}	
					else if(Rx_Buff1[Expires_pointer_t]=='S')//9 S
					{//Sep
						month_token=9;
					}			
					else if(Rx_Buff1[Expires_pointer_t]=='O')//10 O
					{//Oct
						month_token=10;
					}			
					else if(Rx_Buff1[Expires_pointer_t]=='N')//11 N
					{//Feb
						month_token=11;
					}			
					else if(Rx_Buff1[Expires_pointer_t]=='D')//12 D
					{//Dec
						month_token=12;
					}
					Expires_pointer_t+=4;
				}
				{//year
					year_token=((Rx_Buff1[Expires_pointer_t]-48)*1000)+((Rx_Buff1[Expires_pointer_t+1]-48)*100)\
					+((Rx_Buff1[Expires_pointer_t+2]-48)*10)+(Rx_Buff1[Expires_pointer_t+3]-48);
					Expires_pointer_t+=5;
				}
				{//hour
					hour_token=((Rx_Buff1[Expires_pointer_t]-48)*10)+(Rx_Buff1[Expires_pointer_t+1]-48);
					Expires_pointer_t+=3;
				}
				{//min
					min_token=((Rx_Buff1[Expires_pointer_t]-48)*10)+(Rx_Buff1[Expires_pointer_t+1]-48);
					Expires_pointer_t+=3;		
				}
				{//sec
					sec_token=((Rx_Buff1[Expires_pointer_t]-48)*10)+(Rx_Buff1[Expires_pointer_t+1]-48);
					Expires_pointer_t+=3;
				}				
						
			}	
		}
		
		
	}
	
	StringSearch("\r\n\r\n",4,400);//json��ʼλ��
	BodyHead_pro.json_Start = StringSecanPointer+4;
	//StringSecanPointer=0;
}

 /**
 * @brief  Jansson Str Type_init	��ʼ��һ���ṹ�岢����ָ����С�ռ䣻����json�Ľ��������ݴ洢
 * @param	uint16_t Len��Json_str_t *Str_struct	//Lenָ����Ҫ����İ�ȫ�洢�ռ䣻*Str_struct�ṹ��ָ��
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��7��
*/
uint8_t Json_Str_typeInit(uint16_t Len,Json_str_t *Str_struct)
{
	Str_struct->Str=(char*)malloc(Len);
	Str_struct->Str_Len=Len;
	Str_struct->Str_p=(char*)malloc(4);
	return 0;
}

 /**
 * @brief  Jansson Space_init		//���齫������õ����ַ�������ṹ�壬ȫ�������������ʼ������������ǰ���ã�
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��9��
*/

Json_str_t userName;
Json_str_t Login_token;
Json_str_t Password;
Json_str_t Host_sn;
Json_str_t Host_type;
/*
Json_str_t Device_sn1;
Json_str_t type1;
Json_str_t Device_sn2;
Json_str_t type2;*/
Json_str_t Device_Suc_SN;
Json_str_t Device_Err_SN;
char Host_Name[20]={0};
uint16_t year ,month,day,weekday,hour,min,sec;
uint16_t Number;//һ��json��Device��������
uint16_t UID=0;
uint16_t HID=0;
uint8_t deviceReg_SucCNT=0;
uint8_t deviceReg_ErrCNT=0;
uint8_t Upload_file_CNT=0;
char WarningPUSH_Contant[50]={0};//push����ַ����Ȳ��ܳ���50

void json_Space_init(void)
{
	Json_Str_typeInit(UserName_LenMAX,&userName);
	Json_Str_typeInit(32,&Login_token);
	Json_Str_typeInit(Password_LenMAX,&Password);
	Json_Str_typeInit(Host_sn_LenMAX,&Host_sn);
	Json_Str_typeInit(Host_type_LenMAX,&Host_type);
	
	Json_Str_typeInit(20,&Device_Suc_SN);
	Json_Str_typeInit(20,&Device_Err_SN);	
/*/	Json_Str_typeInit(20,&Device_sn1);			//��ʽrelease�汾����������ռ俪�� 
//	Json_Str_typeInit(20,&type1);
//	Json_Str_typeInit(20,&Device_sn2);
//	Json_Str_typeInit(20,&type2);*/
}

 /**
 * @brief  Json unpack result copy	//��json����ڶ��е����ݣ��ַ�����������ָ������
 * @param	char *output,char* temp	//output ��ȫ�洢�����ָ�룬tempָ����е�ָ��
 * @retval uint8_t return	error code
 * @author ZCD1300
 * @Time 2021��11��8��
*/

json_t *root;
json_error_t *errot;
uint8_t Json_Delete_Enable_Flag=0;
//���������Json_Delete_Enable_Flag==0ʱ���ܽ���json_delete
uint8_t json_unpack_copy(char *output,char* temp)
{
	uint16_t Copy_len=strlen(temp);
	
	for(uint16_t i=0;i< Copy_len;i++)//�����ƺ���unpack��������3״̬�£�����Ӳ������
	{
		output[i]=temp[i];
	
	}

	if(Json_Delete_Enable_Flag>0)
	{
		Json_Delete_Enable_Flag--;	
		if(Json_Delete_Enable_Flag==0)
		{
			json_delete(root);//(�Ѿ��޸�)���ﲻӦ��ֱ�ӻ��գ�Ӧ������copy����ִ�к�����ͷ�	
		}		
		
	}


	return 0;
}

 /**
 * @brief  Jansson unpack
 * @param	char* json_unpack_type//�������
 * @retval uint8_t return 0����  1json��ʽ����	2stateδ֪(����������)	3����ȫ���ͷ��ڴ� 
 * @author ZCD1300
 * @Time 2021��11��3��
*/
//const char *test="{\"host_sn\":\"DFHJ-DAHE-FJSN-DMHD\",\"number\":2,\"devices\":[{\"device_sn\":\"RTDS-SADF-GZSA-WQRD\",\"type\":\"gasDetector/0.1\"},{\"device_sn\":\"SDAF-WERC-FAZX-TVSF\",\"type\":\"gasDetector/0.2\"}]}";
int state_p=0;//ͬ��
char *info_p;//����ȫ�Ĵ���ʽ		����ô�ðɣ�info����ȫ�����棬ֻ����Ҫʱ��ʱʹ��һ��
int16_t Unpack_return=0;
uint8_t Display_unpackBuff_Flag=0;
int16_t JsonUnpack(char* json_unpack_type)
{
	Body_Process();//���ر���ͷ����	
	char json_unpack_buff[json_unpack_max]={0};//����json����ʱ����

	for(uint16_t i=0;i<Val_MAX_Limit_U( BodyHead_pro.Content_Lenth,json_unpack_max);i++)//�����ݴӴ��ڻ�����˵���������
	{
		//���ܵ�����������bug
		//����������״̬�µ�Len����,��������
		json_unpack_buff[i]=Rx_Buff1[BodyHead_pro.json_Start+i];
		
	}
	if(Display_unpackBuff_Flag==1)
	{
		DebugPrint_uart("\r\nJson Unpack Buff:",0x200);
		DebugPrint_uart_LineFeed(json_unpack_buff,0x400);
	
	}
	
	
	if(json_unpack_buff[9]=='-')//�ж�һ��json�ڷ��ص�state  //0~ -99
	{	
		if((json_unpack_buff[11]>=48)&&(json_unpack_buff[11]<58))//stateΪ��λ��
		{
			state_p=0-((json_unpack_buff[10]-48)*10)-(json_unpack_buff[11]-48);
		
		}
		else if((json_unpack_buff[10]>=48)&&(json_unpack_buff[10]<58))//stateΪһλ��
		{
			state_p=0-(json_unpack_buff[10]-48);
		}
	}
	else
	{
		state_p=0;
	}
	if(Json_Delete_Enable_Flag==0)
	{
		//json_delete(root);
		root=json_loads(json_unpack_buff,0,errot);
		Json_Delete_Enable_Flag=1;
		if(json_is_object(root))
		{
			if(StringMatch( json_unpack_type,0,"test/time",0))
			{
				json_unpack(root,"{s:i,s:s,s:{s:i,s:i,s:i,s:i,s:i,s:i,s:i}}","state",&state_p,"info",&info_p,\
				"data","year",&year,"month",&month,"day",&day,"weekday",&weekday,"hour",&hour,"min",&min,"sec",&sec);
				json_delete(root);
				Json_Delete_Enable_Flag=0;
				
			}
			else if(StringMatch(json_unpack_type,0,"reg_new",0))//Log UP
			{
				switch (state_p)
				{
					case 0://ע��ɹ�
					{
						json_unpack(root,"{s:i,s:s,s:{s:i,s:s}}","state",&state_p,"info",&info_p,\
						"data","uid",&UID,"name",&(userName.Str_p));				
					break;
					}
					case -1://�ظ�ע��
					{
						json_unpack(root,"{s:i,s:s,s:{s:s}}","state",&state_p,"info",&info_p,\
						"data","name",&(userName.Str_p));							
					break;
					}
					case -5://��Ч���û���������
					{
						json_unpack(root,"{s:i,s:s,s:{s:s,s:s}}","state",&state_p,"info",&info_p,\
						"data","name",&(userName.Str_p),"password",&(Password.Str_p));							
					break;
					}			
					default:
					{	
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//δ֪state����
						break;}
				}				
			}
			else if(StringMatch( json_unpack_type,0,"Login",0))	//LogIn
			{
				switch (state_p)
				{
					case 0://��¼�ɹ�
					{	
						json_unpack(root,"{s:i,s:s,s:{s:i,s:s,s:s}}","state",&state_p,"info",&info_p,\
						"data","uid",&UID,"name",&(userName.Str_p),"utoken",&(Login_token.Str_p));							
						Json_Delete_Enable_Flag=1;
						if(StringMatch(Login_token.Str_p,0,BodyHead_pro.token,0)!=1)//head��body��tokenƥ��
						{
							for(uint8_t i=0;i<32;i++)
							{
								BodyHead_pro.token[i]=0;
							}
							json_delete(root);
							Json_Delete_Enable_Flag=0;
							
							return -4;//head��body��token��һ��,��BodyHead_pro.token��������
						}
						break;
					}
					case -3://��¼ʧ�ܣ��û������������
					{	
						json_unpack(root,"{s:i,s:s,s:{s:s,s:s}}","state",&state_p,"info",&info_p,\
						"data","name",&(userName.Str_p),"password",&(Password.Str_p));	
						Json_Delete_Enable_Flag=2;
						return -3;
						break;
					}
					
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//δ֪state����
						break;}					
				}
			}
/*			else if(StringMatch( json_unpack_type,0,"up_load_test",0))//upload/test		������ʱʹ�ã���ʽrelease������
			{
				switch (state_p)
				{		
					case 0://json���ɸ�ʽ��ȷ
					{
						json_unpack(root,"{s:i,s:s,s:{s:s,s:i,s:[{s:s,s:s},{s:s,s:s}]}}","state",&state_p,"info",&info_p,\
						"data","host_sn",&(Host_sn.Str_p),"number",&Number,"devices","device_sn",&(Device_sn1.Str_p),"type"\
						,&(type1.Str_p),"device_sn",&(Device_sn2.Str_p),"type",&(type2.Str_p));
						Json_Delete_Enable_Flag=5;
						
						Json_Delete_Enable_Flag=0;
						json_delete(root);//�������������Ϊ��ȫ����Ҫ���������ã�ֱ�ӽ���ѿռ��ͷţ�����λFlag
						
						//return 0;
						break;
					}
					case -8://json��ʽ����ȷ���߲�����
					{
						
						return -8;//json������Բ���ȷ
						break;
					}
					
					
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//δ֪state����
						break;}					
				}		
			}*/
			else if(StringMatch(json_unpack_type,0,"reg_host",0))
			{
				switch (state_p)
				{
					case 0://����ע��ɹ�
					{
						json_unpack(root,"{s:i,s:s,s:{s:i,s:s,s:s}}","state",&state_p,"info",&info_p,\
						"data","hid",&HID,"sn",&(Host_sn.Str_p),"type",&Host_type.Str_p);
						Json_Delete_Enable_Flag=2;	
						//return 0;
						break;
					}
					case -2:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -2;//�û�δ��¼
						break;
					}						
					case -4://�����ظ�ע��
					{
						json_unpack(root,"{s:i,s:s,s:{s:s,s:s}}","state",&state_p,"info",&info_p,\
						"data","sn",&(Host_sn.Str_p),"type",&Host_type.Str_p);
						Json_Delete_Enable_Flag=2;		
						return -4;
						break;					
					}
					case -8:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;
						return -8;//������json���ղ�ȫ�����ʧ��
						break;
					}
					
				
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//δ֪state����
						break;}						
				}
			}
			else if(StringMatch(json_unpack_type,0,"reg_device",0))
			{
				switch(state_p)
				{
					case 0:
					{	//Ŀǰֻ��ʵ�ֶԵ���ע��Ľ��������ڴ�汾�������޸ĳɶ��ע�����
						//ʵ�ַ�ʽ��������ֻ��json������suc_cnt��Err_cnt��������ֵ�����ж�
						json_unpack(root,"{s:i,s:s,s:{s:i,s:i,s:[s*],s:[s*]}}","state",&state_p,"info",&info_p,\
						"data","suc_count",&deviceReg_SucCNT,"err_count",&deviceReg_ErrCNT,\
						"suc_sn",&Device_Suc_SN.Str_p,"err_sn",&Device_Err_SN.Str_p);
						Json_Delete_Enable_Flag=2;	
						//return 0;
						break;
					}	
					case -2:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -2;//�û�δ��¼
						break;
					}						
					case -8:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -8;//json��ʽ����
						break;
					}
					case -4:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -4;//δ��Ȩ�������ύ
						break;
					}
					
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//δ֪state����
						break;}						
				}
			}
			else if(StringMatch(json_unpack_type,0,"upload_data",0))
			{
				switch(state_p)
				{
					case 0:
					{	//Ŀǰֻ��ʵ�ֶԵ���ע��Ľ��������ڴ�汾�������޸ĳɶ��ע�����
						//ʵ�ַ�ʽ��������ֻ��json������suc_cnt��Err_cnt��������ֵ�����ж�
						json_unpack(root,"{s:i,s:s,s:{s:i}}","state",&state_p,"info",&info_p,\
						"data","file_count",&Upload_file_CNT);
						Json_Delete_Enable_Flag=0;	
						json_delete(root);
						//return 0;
						break;
					}	
					case -2:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -2;//�û�δ��¼
						break;
					}						
					case -8:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -8;//json��ʽ����
						break;
					}
					case -4:
					{
						json_delete(root);
					 	Json_Delete_Enable_Flag=0;						
						return -4;//δ��Ȩ�������ύ;����δ��Ȩ��Device����Դ
						break;
					}
					
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//δ֪state����
						break;}						
				}
			}
			else if(StringMatch(json_unpack_type,0,"warningPUSH",0))
			{
				switch(state_p)
				{
					case 0:
					{	
						json_unpack(root,"{s:i,s:s,s:s}","state",&state_p,"info",&info_p,\
						"data",&WarningPUSH_Contant);
						Json_Delete_Enable_Flag=0;	
						json_delete(root);
						//return 0;
						break;
					}
					case -2:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -2;//�û�δ��¼
						break;
					}					
					case -8:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -8;//json��ʽ����
						break;
					}
					case -4:
					{
						json_delete(root);
					 	Json_Delete_Enable_Flag=0;						
						return -4;//δ��Ȩ�������ύ;����δ��Ȩ��Device����Դ
						break;
					}
					
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//δ֪state����
						break;}						
				}
			}					
			return 0;	
		}
		else////json��ʽ����	
		{
			json_delete(root);
			Json_Delete_Enable_Flag=0;
			return 1;//json��ʽ����	
		}
			
	}
	else
	{
			return 3;//�������ݶ�ʧ������,Ϊ�������ݰ�ȫִ��(json_unpack_copy)֮����ܽ�����json������Rootռ�ö��ͷ�,ִ�к�retry
	}

}
 /**
 * @brief  Jansson pack
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021��11��9��
*/
char Json_packed_BUFF[300]={0};
char DeviceSN_single[17]="asdf1234qwer1234";//����SN
int16_t Pack_return=0;
uint8_t PackBuff_Using_flag=0;
uint8_t Display_packBuff_Flag=0;
//char output_str[Json_packed_BUFF_MAX]={0};
int8_t JsonPack(char* json_pack_tpye)
  {
	if(Json_Delete_Enable_Flag==0)
	{
		char* output_str = NULL;	
		if(StringMatch(json_pack_tpye,0,"empty",0))
		{
			root=json_pack("{}");	
			output_str = json_dumps(root, JSON_PRESERVE_ORDER);	
			Json_Delete_Enable_Flag++;			
			json_unpack_copy(Json_packed_BUFF,output_str);			
			free(output_str);
			json_decref(root);			
		}
		else if(StringMatch(json_pack_tpye,0,"post_test",0))
		{
			root=json_pack("{s:s,s:i,s:[{s:s,s:s},{s:s,s:s}]}","host_sn","DFHJ-DAHE-FJSN-","number",1,"devices","device_sn","1234","type","asdfgsdf-dasds","device_sn","123w4","type","asdfgsd-dasds");
			output_str = json_dumps(root, JSON_PRESERVE_ORDER);
			Json_Delete_Enable_Flag++;			
			json_unpack_copy(Json_packed_BUFF,output_str);
			free(output_str);
			json_decref(root);
			return 0;
		}
		else if(StringMatch(json_pack_tpye,0,"reg_host",0))//ע������
		{
			root=json_pack("{s:s,s:s,s:s}","host_sn",Host_sn.Str,"type","EVANGELION-01","host_name",Host_Name);
			output_str = json_dumps(root, JSON_PRESERVE_ORDER);	
			Json_Delete_Enable_Flag++;			
			json_unpack_copy(Json_packed_BUFF,output_str);
			free(output_str);
			json_decref(root);
			return 0;	//ע����������pack
		}
		else if(StringMatch(json_pack_tpye,0,"reg_device",0))//ע��Devices ��������(Ŀǰֻ��һ��һ��ע�ᣬ��˽ӿ���֧��ͬʱ���)
		{
			root=json_pack("{s:s,s:i,s:[{s:s,s:s,s:s}]}","host_sn",Host_sn.Str,"devices_count",1,"devices","device_sn",DeviceSN_single,"type",\
			"testGas","name","Gas_test1");
			output_str = json_dumps(root, JSON_PRESERVE_ORDER);	
			Json_Delete_Enable_Flag++;			
			json_unpack_copy(Json_packed_BUFF,output_str);
			free(output_str);
			json_decref(root);
			return 0;	//ע��devices����pack
		}		
		else if(StringMatch(json_pack_tpye,0,"upload_data",0))//�ϴ����ݴ����Ŀǰ�������ã�ʵ��ʵ�ּƻ��������ϴ�
		{
			//ֻ�ϴ�1���豸������
			DebugPrint_uart_LineFeed(DeviceSN_single,200);
			root=json_pack("{s:s,s:i,s:[{s:s,s:s}]}","host_sn",Host_sn.Str,"device_count",1,"devices","device_sn",DeviceSN_single,"data",\
			UploadData_buff);
			output_str = json_dumps(root, JSON_PRESERVE_ORDER);	
			Json_Delete_Enable_Flag++;			
			json_unpack_copy(Json_packed_BUFF,output_str);
			free(output_str);
			json_decref(root);
			return 0;	
		}	
		else if(StringMatch(json_pack_tpye,0,"warningPUSH",0))//���������push
		{		
			root=json_pack("{s:s,s:s}","topic","warning","message","this is a message for push test from host");
			output_str = json_dumps(root, JSON_PRESERVE_ORDER);	
			Json_Delete_Enable_Flag++;			
			json_unpack_copy(Json_packed_BUFF,output_str);
			free(output_str);
			json_decref(root);
			return 0;			
		
		}
	}
	else
	{
		return 3;//rootָ�뱻ռ�ã������ͷ�ռ��
	}
	
}







