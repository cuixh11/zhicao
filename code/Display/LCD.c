#include "lcd.h"
#include "usart.h"
#include "stdio.h"
#include "ESP8266.h"


#if defined HostAPP
	#include "cc2530.h"
#endif

uint8_t Page_NOW=0;
uint8_t HMI_Display_RefeshDisableFlag=0;
/**
 * @brief  初始化串口屏
 * @param	 void
 * @retval void
 * @author LDX2000
 * @Time   2021年11月26日
*/
void HMI_Init(void)
	{
	 	Delay(200);
		HMISend_End(0);
		Delay(200);
	}
	/**
 * @brief  串口屏Rest
 * @param	 void
 * @retval void
 * @author ZCD1300
 * @Time   2021年12月12日
*/
void HMI_Rest(void)
{
	char temp[]="rest";
	HMISends(temp,4);
	HMISend_End(0);
}
/**
 * @brief  LCD是否休眠状态
 * @param	 status=0:唤醒状态  status=0:休眠状态
 * @retval void
 * @author LDX2000
 * @Time   2021年11月26日
*/
uint16_t DisplaySleep_TimeCNT=0;
void LCD_Status(uint8_t status)
{
	uint8_t Send_t[]="sleep=0";
	if(status==Wake_Up_Status)
	{
		Send_t[6]='0';
		DisplaySleep_TimeCNT=0;
	}
	else if(status==Sleep_Status)
	{
		Send_t[6]='1';
		DisplaySleep_TimeCNT=0;
	}
	UART_Send(&huart6,Send_t,7,200);
	HMISend_End(0);
}

/**
 * @brief  用来连续发送3个0xff来做结尾标志   //HMISend(0xff);
 * @param	 uint8_t
 * @retval void
 * @author LDX2000
 * @Time   2021年11月26日
*/
void HMISend_End(uint8_t type_t)
{   
	uint8_t HMIEnd[3]={0xff,0xff,0xff};	
	uint8_t HMIEnd1[4]={'\"',0xff,0xff,0xff};		
	if(type_t==0)	
	{
		UART_Send(&huart6,HMIEnd,3,100);//发送数据结尾符  	
	}
	else if(type_t==1)
	{
		UART_Send(&huart6,HMIEnd1,4,100);
	}
		

} 
void HMISend_Start(uint16_t id_t)
{
	if(id_t<10)
	{
		char Start_t[8]="t0.txt=\"";
		Start_t[1]=id_t+48;
		UART_Send(&huart6,Start_t,8,1000);		
	}
	else if(id_t<100)
	{
		char Start_t1[9]="t10.txt=\"";
		Start_t1[1]=id_t/10+48;
		Start_t1[2]=(id_t%10)+48;
		UART_Send(&huart6,Start_t1,9,1000);			
	}
	else if(id_t<1000)
	{
		char Start_t2[10]="t100.txt=\"";
		Start_t2[1]=id_t/100+48;
		Start_t2[1]=(id_t/10)%10+48;
		Start_t2[1]=id_t%10+48;
		UART_Send(&huart6,Start_t2,10,1000);			
	}

}
/**
 * @brief  字符串发送函数
 * @param	 pData:要发送的字符串 
 * @retval  void
 * @author ZCD
 * @Time   2021年12月10日
*/
uint8_t LineIndex=0;
uint8_t RowIndex=0;
uint16_t Page0StrBuffUsedSpace=0;//已经使用的显示空间
uint16_t Page0StrCMDSpeace=0;//已经使用的CMD空间
//char Page0Str_Buff[Page0StrFullSpace]={0};
char Page0Str_Buff[Page0StrLine_MAX*Page0StrRow_MAX]={0};
 void HMISends(char *pData,uint16_t send_len)		
{
	UART_Send(&huart6,pData,send_len,1000);  
}

/**
 * @brief  完整显示一次字符串
 * @param	 pData:要发送的字符串  Timeout:超时时间
 * @retval  void
 * @author ZCD1300
 * @Time   2021年12月10日
*/
void Uart_Display_String(char *pData,uint32_t LineIndex)
{	
	if(HMI_Display_RefeshDisableFlag==1)
	{
		return ;
	}
	uint8_t Display_RetryCNT=0;
	char HMIBuffFullError[4]={0x24,0xff,0xff,0xff};
	char HMIOK[4]={0x01,0xff,0xff,0xff};	
	
	Display_Retry:
	HMISend_Start(LineIndex);
	HMISends(pData,strlen(pData));
	HMISend_End(1);
	UART6_Refresh_Flag=0;//检测串口触发前要置位
	Wait_Until_FlagTrue(&UART6_Refresh_Flag,2000);
	if(StringSearch_Assign(HMIOK,Rx_Buff6,4,5))
	{
		Rx_Buff6[StringSecanPointer_Assign_Buff]=0;
	}
	else
	{
		Rx_Buff6[0]=0;
		Rx_Buff6[1]=0;
		Rx_Buff6[2]=0;
		if(Display_RetryCNT<3)
		{
			
			Delay(200);
			Display_RetryCNT++;
			

			goto Display_Retry;			
		}
		else
		{
			RowIndex=0;
			Page0StrBuffUsedSpace=0;
			Page0StrCMDSpeace=0;
			Page0Str_Buff[0]=0;
			HMI_Rest();
			Delay(400);
			HMI_Init();			
			return ;
		}
	}

}
/**
 * @brief 自动多行、换行显示
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月10日
*/


void LCD_Clear_all(void)
{
	for(uint16_t i=0;i<Page0StrLine_MAX*Page0StrRow_MAX;i++)
	{
		Page0Str_Buff[i]=0;
	}
}

/**
 * @brief 自动多行、换行显示
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月10日
*/


uint8_t LCD_StringBUFF_Manage(char *InputChar_t)
{
	if(HMI_Display_RefeshDisableFlag==1)
	{
		return 9;
	}
	if(Page_NOW!=0)
	{
		return 9;
	}
	uint16_t InputLen_t=strlen(InputChar_t);//计算字符串长度	
	char InputChar_cp[560]={0};//InputChar_t 副本存放数组
	uint16_t Remaining_space=0;
	uint16_t NeedsSpace_t=0;
	uint8_t LineFeed_CNT=0;
	
	for(uint16_t i=0;i<InputLen_t;i++)
	{
		InputChar_cp[i]=InputChar_t[i];
	}
	
	while(StringSearch_Assign("\r\n",InputChar_cp,2,InputLen_t))//检查是否输入了换行符
	{//是	
		InputChar_cp[StringSecanPointer_Assign_Buff]='\\';	
		InputChar_cp[StringSecanPointer_Assign_Buff+1]='r';	
		LineFeed_CNT++;//换行符个数
	}
	

	Remaining_space=Page0StrFullSpace-Page0StrBuffUsedSpace;//计算剩余显示空间	
	{//计算需要的显示空间
		uint16_t ValidStr_LEN_t=0;
		ValidStr_LEN_t=InputLen_t-2*LineFeed_CNT;
		
		RowIndex+=ValidStr_LEN_t;
		if(RowIndex>Page0StrRow_MAX)
		{
			RowIndex=RowIndex%Page0StrRow_MAX;
		}
		uint16_t LineFeedStr_LEN_t=0;

		while(LineFeed_CNT)
		{
			if((Page0StrBuffUsedSpace+RowIndex)%Page0StrRow_MAX!=0)//
			{//当前行有内容换行
				LineFeedStr_LEN_t=((ValidStr_LEN_t/Page0StrRow_MAX)+1)*Page0StrRow_MAX;		
			}
			else
			{				
				if(RowIndex==Page0StrRow_MAX)
				{//满行换行
					LineFeedStr_LEN_t+=0;
				}
				else
				{//空行换行
					LineFeedStr_LEN_t+=Page0StrRow_MAX;
				}
			}		
			RowIndex=0;
			LineFeed_CNT--;
		}

		if(StringSecanPointer_Assign_Buff+2<InputLen_t)//最后一个换行符后还有字符
		{
			NeedsSpace_t=ValidStr_LEN_t+LineFeedStr_LEN_t;//LineFeed_CNT*Page0StrRow_MAX;			
			RowIndex+=ValidStr_LEN_t;
			if(RowIndex>Page0StrRow_MAX)
			{
				RowIndex=RowIndex%Page0StrRow_MAX;
			}			
		}
		else
		{//以换行结尾
			NeedsSpace_t=LineFeedStr_LEN_t;
		}
		
	}
	
	if(Remaining_space>=NeedsSpace_t)
	{//直接放入
		Page0StrBuffUsedSpace+=NeedsSpace_t;//更新屏幕已经使用的空间		

		for(uint16_t i=0;i<InputLen_t;i++)
		{
			Page0Str_Buff[i+Page0StrCMDSpeace]=InputChar_cp[i];
		}
		Page0StrCMDSpeace+=InputLen_t;		
		if(Page0StrBuffUsedSpace>Page0StrFullSpace)
		{
			Page0StrBuffUsedSpace=Page0StrFullSpace;
		}
		Page0Str_Buff[Page0StrCMDSpeace]=0;
	}

	else
	{//空间不足

		uint16_t AddedSpace=0;
		uint16_t DecCMDLen=0;
		while(1)
		{
			if(StringSearch_Assign("\\r",Page0Str_Buff,2,Page0StrRow_MAX))
			{//一行长度（40）中存在换行
				AddedSpace+=Page0StrRow_MAX;
				Page0Str_Buff[StringSecanPointer_Assign_Buff]=' ';
				DecCMDLen=StringSecanPointer_Assign_Buff+2;
				if(AddedSpace+Remaining_space>NeedsSpace_t)
				{//空间释放足够
					break;
				}
			}
			else
			{
				AddedSpace+=Page0StrRow_MAX;
				DecCMDLen+=40;
				if(AddedSpace+Remaining_space>NeedsSpace_t)
				{//空间释放足够
					break;
				}
			}
		}

		Page0StrBuffUsedSpace=Page0StrBuffUsedSpace-AddedSpace+NeedsSpace_t;//更新屏幕已经使用的空间	
		//Page0StrBuffUsedSpace+=NeedsSpace_t;
		uint16_t Diff_CMD=0;
		if(Page0StrCMDSpeace>DecCMDLen)
		{
			Diff_CMD=Page0StrCMDSpeace-DecCMDLen;	
		}

		for(uint16_t i=0;i<Diff_CMD;i++)
		{
			Page0Str_Buff[i]=Page0Str_Buff[i+DecCMDLen];
		}
		
		Page0StrCMDSpeace=Diff_CMD;
		Page0Str_Buff[Page0StrCMDSpeace]=0;
		for(uint16_t j=0;j<InputLen_t;j++)
		{
			Page0Str_Buff[j+Page0StrCMDSpeace]=InputChar_cp[j];
		}
		Page0StrCMDSpeace+=InputLen_t;
		Page0Str_Buff[Page0StrCMDSpeace]=0;
				
	}
	Uart_Display_String(Page0Str_Buff,0);
}

/**
 * @brief 进度条
 * @param	void
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月10日
*/

void LCD_Startup_ProgressBar(uint8_t StartUP_Progress)
{
	char LCD_ProgressBarHead[7]="j0.val=";
	char LCD_ProgressBarVAL[3]="   ";
	if(StartUP_Progress>=100)
	{
		LCD_ProgressBarVAL[0]='1';
		LCD_ProgressBarVAL[1]='0';
		LCD_ProgressBarVAL[2]='0';	
		HMISends(LCD_ProgressBarHead,7);
		HMISends(LCD_ProgressBarVAL,3);
		HMISend_End(0);		
	}
	else if(StartUP_Progress>=10)
	{
		LCD_ProgressBarVAL[0]=StartUP_Progress/10+48;
		LCD_ProgressBarVAL[1]=StartUP_Progress%10+48;
		LCD_ProgressBarVAL[2]=' ';
		HMISends(LCD_ProgressBarHead,7);
		HMISends(LCD_ProgressBarVAL,2);
		HMISend_End(0);				
	}
	else
	{
		LCD_ProgressBarVAL[0]=StartUP_Progress+48;
		LCD_ProgressBarVAL[1]=' ';
		LCD_ProgressBarVAL[2]=' ';	
		HMISends(LCD_ProgressBarHead,7);
		HMISends(LCD_ProgressBarVAL,1);
		HMISend_End(0);				
	}
	
}
/**
 * @brief 换页
 * @param	uint8_t page
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月12日
*/
uint8_t Page_Switch(uint8_t pageNum)
{
	if(Page_NOW!=pageNum)
	{
		if(pageNum<10)
		{
			char page_t[]="page 0";
			page_t[5]=pageNum+48;
			
			HMISends(page_t,6);
			HMISend_End(0);
			Page_NOW=pageNum;
		}
		
	}
	

}
/**
 * @brief 换页
 * @param	uint8_t page
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月12日
*/
uint8_t Check_box(uint8_t num,uint8_t state)
{
		char page_t[]="c0.val=0";
		if(num<10)
		{
			page_t[1]=num+48;
			if(state==0)
			{
				page_t[7]='0';
			}
			else
			{
				page_t[7]='1';
			}
			HMISends(page_t,8);
			HMISend_End(0);
		}

}
#if defined HostAPP
/**
 * @brief page1
 * @param	uint8_t page
 * @retval void
 * @author ZCD1300
 * @Time 2021年12月12日
*/
uint8_t page1(void)
{
	if(HMI_Display_RefeshDisableFlag==1)
	{
		return 9;
	}
	if(Page_NOW!=1)
	{
		return 9;
	}
	uint16_t OnlineSlaveNUM=0;
	for(uint8_t i=0;i<DeviceNUM_MAX;i++)
	{
		Delay(100);
		if(DeviceOnline_Falg[i]==1)
		{
			
			OnlineSlaveNUM++;
			Check_box(i,1);

		}	
		else
		{
			Check_box(i,0);
		}
	}
	char ttemp[]="t1.txt=\"1\"";//{OnlineSlaveNUM+48};
	ttemp[8]=OnlineSlaveNUM+48;
	HMISends(ttemp,10);
	HMISend_End(0);
}
#endif
