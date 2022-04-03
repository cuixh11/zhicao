 /**
 * @brief  BC20驱动
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年11月23日
*/
#include "BC20.h"
#include "ESP8266.h"//只依赖其中的StringSearch_Assign()函数，用来检索字符串；
uint8_t NBIOT_TCPLink_Alive=0;

 /**
 * @brief  BC20发送CMD函数
 * @param	指令1，指令2，指令1长度，指令2长度（实际发送为指令1+指令2；指令2为0则只发送指令1）
 * @retval	void
 * @author ZCD
 * @Time 2021年11月23日
*/
void NBIOT_CMD_SEND(char *cmd,char* cmd2,uint8_t LenTemp,uint8_t LenTemp2)
{
	if(cmd2!=0)
	{
		HAL_UART_Transmit(&huart4,cmd,LenTemp-1,0x100);//sizeof计算的长度包括“\0”占位符要减掉
		HAL_UART_Transmit(&huart4,cmd2,LenTemp2,0x100);	
		HAL_UART_Transmit(&huart4,"\r\n",2,0x100);	
	}
	else
	{
		HAL_UART_Transmit(&huart4,cmd,LenTemp-1,0x100);//sizeof计算的长度包括“\0”占位符要减掉
		HAL_UART_Transmit(&huart4,"\r\n",2,0x100);		
	}
}
 /**
 * @brief  BC20发送DATA函数
 * @param	指令1，指令2，指令1长度，指令2长度（实际发送为指令1+指令2；指令2为0则只发送指令1）
 * @retval	void
 * @author ZCD
 * @Time 2021年11月24日
*/
void NBIOT_DATA_SEND(char *data,uint16_t data_len)
{
	UART_Send(&huart4,data,data_len,0x500);
}
/**
 * @brief  BC20 push模式接收函数,应该放在串口中断中，检测到URC自动处理
 * @param		void
 * @retval	void
 * @author ZCD
 * @Time 2021年11月24日
*/
uint8_t NBIOT_Received_Flag=0;
void NBIOT_Rece_PUSH(void)
{
	if(StringSearch_Assign("+QIURC:",Rx_Buff4,7,100))
	{//接收到数据
		NBIOT_Received_Flag=1;;
		//DebugPrint_uart_LineFeed("NBIOT_OK",200);
	}

}

 /**
 * @brief  BC20复位函数
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年1月25日
*/
void NBIOT_RST(void)//初始化
{
	NBIOT_CMD_SEND(NBIOTRST,"2",sizeof(NBIOTRST),1);
}
 /**
 * @brief  BC20 AT测试函数
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年1月25日
*/
void NBIOT_AT_Test(void)//傻逼模块复位后给他发送东西才会返回信息
{
	NBIOT_CMD_SEND(NBIOT_AT_TEST,0,sizeof(NBIOTRST),1);
}


