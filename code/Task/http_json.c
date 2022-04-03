#include "http_json.h"
#include "SmartNest_server.h"



 /**
 * @brief  限制最大的遍历深度不超过串口1接收缓存大小
 * @param	uint16_t input 输入要限制的数值
 * @retval 返回范围内的数值
 * @author ZCD1300
 * @Time 2021年11月7日
*/
uint16_t MAXBuff_Limt(uint16_t input )
{
	if(input>BuffMAX)
	{return BuffMAX;}
	else
	{return input;}
}

/**
 * @brief  utoken Iput Test 	//手动规定一个token，仅供测试使用；
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月10日
*/
void utoken_input_manually(char * utoken_temp)
{
		for(uint8_t i=0;i<32;i++)
		{
			BodyHead_pro.token[i]=utoken_temp[i];
		}

}

/**
 * @brief  Body head make		//准备
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月9日
*/

char Head_Send_Buff[Head_Len_MAX]={0};//make的头的缓冲区
uint16_t Head_Send_BuffPoint=0;//头缓冲区指针偏移
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
	if(Content_Length>99)//3位数 100-999
	{	
		strcpy(Len_temp,"Content-Length: 999\r\n");
		//Len_temp="Content-Length: 999\r\n";		
		Len_temp[16]=(Content_Length/100)+48;
		Len_temp[17]=((Content_Length%100)/10)+48;
		Len_temp[18]=(Content_Length%10)+48;		
		strcpy(Head_Send_Buff+Head_Send_BuffPoint,Len_temp);
		Head_Send_BuffPoint=Head_Send_BuffPoint+21;
	}
	else if(Content_Length>9)//两位数 10-99
	{
		strcpy(Len_temp,"Content-Length: 99\r\n");
		//Len_temp="Content-Length: 99\r\n";		
		Len_temp[16]=(Content_Length/10)+48;
		Len_temp[17]=(Content_Length%10)+48;	
		strcpy(Head_Send_Buff+Head_Send_BuffPoint,Len_temp);
		Head_Send_BuffPoint=Head_Send_BuffPoint+20;		
	}
	else if((Content_Length>=0)&&(Content_Length<=9))//个位数 0-9
	{
		strcpy(Len_temp,"Content-Length: 9\r\n");
		//Len_temp="Content-Length: 9\r\n";
		Len_temp[16]=Content_Length+48;		
		strcpy(Head_Send_Buff+Head_Send_BuffPoint,Len_temp);
		Head_Send_BuffPoint=Head_Send_BuffPoint+19;
	}
	
	char temp[6]={0};
	if(StringMatch( BodyHead_pro.token,6,temp,6)==0)//token存在
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
 * @Time 2021年11月7日
*/
uint16_t year_token=0,month_token=0,day_token=0,hour_token=0,min_token=0,sec_token=0;
Body_head BodyHead_pro={0};
uint8_t Body_Process(void)
{
	StringSearch("HTTP/1.1 ",9,60);//返回状态 200 404 502 等
	BodyHead_pro.State_code=(Rx_Buff1[StringSecanPointer+9]-48)*100+(Rx_Buff1[StringSecanPointer+10]-48)*10+(Rx_Buff1[StringSecanPointer+11]-48);
	Rx_Buff1[StringSecanPointer+9]=0;
	Rx_Buff1[StringSecanPointer+10]=0;
	Rx_Buff1[StringSecanPointer+11]=0;
	StringSearch("Date:",5,MAXBuff_Limt(StringSecanPointer+100));//日期
	for(uint16_t i=0;i<29;i++)
	{
		BodyHead_pro.Date[i]=Rx_Buff1[StringSecanPointer+i+6];
	}
	
	StringSearch("nt-Length:",10,MAXBuff_Limt(StringSecanPointer+150));//json长度
	if(Rx_Buff1[StringSecanPointer+11+2]>=48)
	{
		BodyHead_pro.Content_Lenth=(Rx_Buff1[StringSecanPointer+11]-48)*100+(Rx_Buff1[StringSecanPointer+12]-48)*10+(Rx_Buff1[StringSecanPointer+13]-48);
	}
	else
	{BodyHead_pro.Content_Lenth=(Rx_Buff1[StringSecanPointer+11]-48)*10+(Rx_Buff1[StringSecanPointer+12]-48);}
	
	if( StringSearch("keep-alive",10,MAXBuff_Limt(StringSecanPointer+50)))//tcp连接方式
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
		{//过期时间
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
					if(Rx_Buff1[Expires_pointer_t]=='J')//月份J开头  1 6 7
					{
						if(Rx_Buff1[Expires_pointer_t+1]=='a')//中间字母
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
						if(Rx_Buff1[Expires_pointer_t+1]=='a')//中间字母
						{//Mar
							month_token=3;
						}				
						else if(Rx_Buff1[Expires_pointer_t+2]=='y')//最后字母
						{//May
							month_token=5;
						}
					}
					else if(Rx_Buff1[Expires_pointer_t]=='A')//4 8
					{
						if(Rx_Buff1[Expires_pointer_t+1]=='p')//中间字母
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
	
	StringSearch("\r\n\r\n",4,400);//json开始位置
	BodyHead_pro.json_Start = StringSecanPointer+4;
	//StringSecanPointer=0;
}

 /**
 * @brief  Jansson Str Type_init	初始化一个结构体并分配指定大小空间；用于json的解析和数据存储
 * @param	uint16_t Len，Json_str_t *Str_struct	//Len指的是要分配的安全存储空间；*Str_struct结构体指针
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月7日
*/
uint8_t Json_Str_typeInit(uint16_t Len,Json_str_t *Str_struct)
{
	Str_struct->Str=(char*)malloc(Len);
	Str_struct->Str_Len=Len;
	Str_struct->Str_p=(char*)malloc(4);
	return 0;
}

 /**
 * @brief  Jansson Space_init		//建议将解包所用到的字符串处理结构体，全放在这个函数初始化，在主函数前调用；
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月9日
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
uint16_t Number;//一个json中Device包的数量
uint16_t UID=0;
uint16_t HID=0;
uint8_t deviceReg_SucCNT=0;
uint8_t deviceReg_ErrCNT=0;
uint8_t Upload_file_CNT=0;
char WarningPUSH_Contant[50]={0};//push最大字符长度不能超过50

void json_Space_init(void)
{
	Json_Str_typeInit(UserName_LenMAX,&userName);
	Json_Str_typeInit(32,&Login_token);
	Json_Str_typeInit(Password_LenMAX,&Password);
	Json_Str_typeInit(Host_sn_LenMAX,&Host_sn);
	Json_Str_typeInit(Host_type_LenMAX,&Host_type);
	
	Json_Str_typeInit(20,&Device_Suc_SN);
	Json_Str_typeInit(20,&Device_Err_SN);	
/*/	Json_Str_typeInit(20,&Device_sn1);			//正式release版本不会有这个空间开辟 
//	Json_Str_typeInit(20,&type1);
//	Json_Str_typeInit(20,&Device_sn2);
//	Json_Str_typeInit(20,&type2);*/
}

 /**
 * @brief  Json unpack result copy	//将json解包在堆中的数据（字符串）拷贝到指定区域
 * @param	char *output,char* temp	//output 安全存储区域的指针，temp指向堆中的指针
 * @retval uint8_t return	error code
 * @author ZCD1300
 * @Time 2021年11月8日
*/

json_t *root;
json_error_t *errot;
uint8_t Json_Delete_Enable_Flag=0;
//这个函数在Json_Delete_Enable_Flag==0时不能进行json_delete
uint8_t json_unpack_copy(char *output,char* temp)
{
	uint16_t Copy_len=strlen(temp);
	
	for(uint16_t i=0;i< Copy_len;i++)//这里似乎在unpack函数返回3状态下，导致硬件死机
	{
		output[i]=temp[i];
	
	}

	if(Json_Delete_Enable_Flag>0)
	{
		Json_Delete_Enable_Flag--;	
		if(Json_Delete_Enable_Flag==0)
		{
			json_delete(root);//(已经修复)这里不应该直接回收，应等所有copy函数执行后才能释放	
		}		
		
	}


	return 0;
}

 /**
 * @brief  Jansson unpack
 * @param	char* json_unpack_type//解包类型
 * @retval uint8_t return 0正常  1json格式错误	2state未知(服务器问题)	3不安全的释放内存 
 * @author ZCD1300
 * @Time 2021年11月3日
*/
//const char *test="{\"host_sn\":\"DFHJ-DAHE-FJSN-DMHD\",\"number\":2,\"devices\":[{\"device_sn\":\"RTDS-SADF-GZSA-WQRD\",\"type\":\"gasDetector/0.1\"},{\"device_sn\":\"SDAF-WERC-FAZX-TVSF\",\"type\":\"gasDetector/0.2\"}]}";
int state_p=0;//同下
char *info_p;//不安全的处理方式		就这么用吧，info不用全部保存，只在需要时临时使用一下
int16_t Unpack_return=0;
uint8_t Display_unpackBuff_Flag=0;
int16_t JsonUnpack(char* json_unpack_type)
{
	Body_Process();//返回报文头处理	
	char json_unpack_buff[json_unpack_max]={0};//解析json的临时数组

	for(uint16_t i=0;i<Val_MAX_Limit_U( BodyHead_pro.Content_Lenth,json_unpack_max);i++)//将内容从串口缓冲搬运到解析数组
	{
		//可能导致死机，修bug
		//限制了意外状态下的Len过大,避免死机
		json_unpack_buff[i]=Rx_Buff1[BodyHead_pro.json_Start+i];
		
	}
	if(Display_unpackBuff_Flag==1)
	{
		DebugPrint_uart("\r\nJson Unpack Buff:",0x200);
		DebugPrint_uart_LineFeed(json_unpack_buff,0x400);
	
	}
	
	
	if(json_unpack_buff[9]=='-')//判断一下json内返回的state  //0~ -99
	{	
		if((json_unpack_buff[11]>=48)&&(json_unpack_buff[11]<58))//state为两位数
		{
			state_p=0-((json_unpack_buff[10]-48)*10)-(json_unpack_buff[11]-48);
		
		}
		else if((json_unpack_buff[10]>=48)&&(json_unpack_buff[10]<58))//state为一位数
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
					case 0://注册成功
					{
						json_unpack(root,"{s:i,s:s,s:{s:i,s:s}}","state",&state_p,"info",&info_p,\
						"data","uid",&UID,"name",&(userName.Str_p));				
					break;
					}
					case -1://重复注册
					{
						json_unpack(root,"{s:i,s:s,s:{s:s}}","state",&state_p,"info",&info_p,\
						"data","name",&(userName.Str_p));							
					break;
					}
					case -5://无效的用户名和密码
					{
						json_unpack(root,"{s:i,s:s,s:{s:s,s:s}}","state",&state_p,"info",&info_p,\
						"data","name",&(userName.Str_p),"password",&(Password.Str_p));							
					break;
					}			
					default:
					{	
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//未知state错误
						break;}
				}				
			}
			else if(StringMatch( json_unpack_type,0,"Login",0))	//LogIn
			{
				switch (state_p)
				{
					case 0://登录成功
					{	
						json_unpack(root,"{s:i,s:s,s:{s:i,s:s,s:s}}","state",&state_p,"info",&info_p,\
						"data","uid",&UID,"name",&(userName.Str_p),"utoken",&(Login_token.Str_p));							
						Json_Delete_Enable_Flag=1;
						if(StringMatch(Login_token.Str_p,0,BodyHead_pro.token,0)!=1)//head与body的token匹配
						{
							for(uint8_t i=0;i<32;i++)
							{
								BodyHead_pro.token[i]=0;
							}
							json_delete(root);
							Json_Delete_Enable_Flag=0;
							
							return -4;//head与body的token不一致,将BodyHead_pro.token内容清零
						}
						break;
					}
					case -3://登录失败，用户名或密码错误；
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
						return 2;//未知state错误
						break;}					
				}
			}
/*			else if(StringMatch( json_unpack_type,0,"up_load_test",0))//upload/test		仅开发时使用，正式release不存在
			{
				switch (state_p)
				{		
					case 0://json生成格式正确
					{
						json_unpack(root,"{s:i,s:s,s:{s:s,s:i,s:[{s:s,s:s},{s:s,s:s}]}}","state",&state_p,"info",&info_p,\
						"data","host_sn",&(Host_sn.Str_p),"number",&Number,"devices","device_sn",&(Device_sn1.Str_p),"type"\
						,&(type1.Str_p),"device_sn",&(Device_sn2.Str_p),"type",&(type2.Str_p));
						Json_Delete_Enable_Flag=5;
						
						Json_Delete_Enable_Flag=0;
						json_delete(root);//这里的数据我认为完全不重要，仅测试用，直接将其堆空间释放，并复位Flag
						
						//return 0;
						break;
					}
					case -8://json格式不正确或者不完整
					{
						
						return -8;//json打包测试不正确
						break;
					}
					
					
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//未知state错误
						break;}					
				}		
			}*/
			else if(StringMatch(json_unpack_type,0,"reg_host",0))
			{
				switch (state_p)
				{
					case 0://主机注册成功
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
						return -2;//用户未登录
						break;
					}						
					case -4://主机重复注册
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
						return -8;//服务器json接收不全或解析失败
						break;
					}
					
				
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//未知state错误
						break;}						
				}
			}
			else if(StringMatch(json_unpack_type,0,"reg_device",0))
			{
				switch(state_p)
				{
					case 0:
					{	//目前只能实现对单个注册的解析，后期大版本迭代在修改成多个注册解析
						//实现方式可以是先只用json解析出suc_cnt和Err_cnt，根据数值进行判断
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
						return -2;//用户未登录
						break;
					}						
					case -8:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -8;//json格式错误
						break;
					}
					case -4:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -4;//未授权的主机提交
						break;
					}
					
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//未知state错误
						break;}						
				}
			}
			else if(StringMatch(json_unpack_type,0,"upload_data",0))
			{
				switch(state_p)
				{
					case 0:
					{	//目前只能实现对单个注册的解析，后期大版本迭代在修改成多个注册解析
						//实现方式可以是先只用json解析出suc_cnt和Err_cnt，根据数值进行判断
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
						return -2;//用户未登录
						break;
					}						
					case -8:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -8;//json格式错误
						break;
					}
					case -4:
					{
						json_delete(root);
					 	Json_Delete_Enable_Flag=0;						
						return -4;//未授权的主机提交;或者未授权的Device数据源
						break;
					}
					
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//未知state错误
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
						return -2;//用户未登录
						break;
					}					
					case -8:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return -8;//json格式错误
						break;
					}
					case -4:
					{
						json_delete(root);
					 	Json_Delete_Enable_Flag=0;						
						return -4;//未授权的主机提交;或者未授权的Device数据源
						break;
					}
					
					default:
					{
						json_delete(root);
						Json_Delete_Enable_Flag=0;						
						return 2;//未知state错误
						break;}						
				}
			}					
			return 0;	
		}
		else////json格式错误	
		{
			json_delete(root);
			Json_Delete_Enable_Flag=0;
			return 1;//json格式错误	
		}
			
	}
	else
	{
			return 3;//存在数据丢失可能性,为保障数据安全执行(json_unpack_copy)之后才能进行新json解析和Root占用堆释放,执行后retry
	}

}
 /**
 * @brief  Jansson pack
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年11月9日
*/
char Json_packed_BUFF[300]={0};
char DeviceSN_single[17]="asdf1234qwer1234";//测试SN
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
		else if(StringMatch(json_pack_tpye,0,"reg_host",0))//注册主机
		{
			root=json_pack("{s:s,s:s,s:s}","host_sn",Host_sn.Str,"type","EVANGELION-01","host_name",Host_Name);
			output_str = json_dumps(root, JSON_PRESERVE_ORDER);	
			Json_Delete_Enable_Flag++;			
			json_unpack_copy(Json_packed_BUFF,output_str);
			free(output_str);
			json_decref(root);
			return 0;	//注册主机正常pack
		}
		else if(StringMatch(json_pack_tpye,0,"reg_device",0))//注册Devices 仅测试用(目前只能一个一个注册，后端接口是支持同时多个)
		{
			root=json_pack("{s:s,s:i,s:[{s:s,s:s,s:s}]}","host_sn",Host_sn.Str,"devices_count",1,"devices","device_sn",DeviceSN_single,"type",\
			"testGas","name","Gas_test1");
			output_str = json_dumps(root, JSON_PRESERVE_ORDER);	
			Json_Delete_Enable_Flag++;			
			json_unpack_copy(Json_packed_BUFF,output_str);
			free(output_str);
			json_decref(root);
			return 0;	//注册devices正常pack
		}		
		else if(StringMatch(json_pack_tpye,0,"upload_data",0))//上传数据打包，目前仅测试用，实际实现计划做批量上传
		{
			//只上传1个设备的数据
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
		else if(StringMatch(json_pack_tpye,0,"warningPUSH",0))//请求服务器push
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
		return 3;//root指针被占用，请先释放占用
	}
	
}







