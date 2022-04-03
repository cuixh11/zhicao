 /**
 * @brief  BC20����
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��11��23��
*/
#include "BC20.h"
#include "ESP8266.h"//ֻ�������е�StringSearch_Assign()���������������ַ�����
uint8_t NBIOT_TCPLink_Alive=0;

 /**
 * @brief  BC20����CMD����
 * @param	ָ��1��ָ��2��ָ��1���ȣ�ָ��2���ȣ�ʵ�ʷ���Ϊָ��1+ָ��2��ָ��2Ϊ0��ֻ����ָ��1��
 * @retval	void
 * @author ZCD
 * @Time 2021��11��23��
*/
void NBIOT_CMD_SEND(char *cmd,char* cmd2,uint8_t LenTemp,uint8_t LenTemp2)
{
	if(cmd2!=0)
	{
		HAL_UART_Transmit(&huart4,cmd,LenTemp-1,0x100);//sizeof����ĳ��Ȱ�����\0��ռλ��Ҫ����
		HAL_UART_Transmit(&huart4,cmd2,LenTemp2,0x100);	
		HAL_UART_Transmit(&huart4,"\r\n",2,0x100);	
	}
	else
	{
		HAL_UART_Transmit(&huart4,cmd,LenTemp-1,0x100);//sizeof����ĳ��Ȱ�����\0��ռλ��Ҫ����
		HAL_UART_Transmit(&huart4,"\r\n",2,0x100);		
	}
}
 /**
 * @brief  BC20����DATA����
 * @param	ָ��1��ָ��2��ָ��1���ȣ�ָ��2���ȣ�ʵ�ʷ���Ϊָ��1+ָ��2��ָ��2Ϊ0��ֻ����ָ��1��
 * @retval	void
 * @author ZCD
 * @Time 2021��11��24��
*/
void NBIOT_DATA_SEND(char *data,uint16_t data_len)
{
	UART_Send(&huart4,data,data_len,0x500);
}
/**
 * @brief  BC20 pushģʽ���պ���,Ӧ�÷��ڴ����ж��У���⵽URC�Զ�����
 * @param		void
 * @retval	void
 * @author ZCD
 * @Time 2021��11��24��
*/
uint8_t NBIOT_Received_Flag=0;
void NBIOT_Rece_PUSH(void)
{
	if(StringSearch_Assign("+QIURC:",Rx_Buff4,7,100))
	{//���յ�����
		NBIOT_Received_Flag=1;;
		//DebugPrint_uart_LineFeed("NBIOT_OK",200);
	}

}

 /**
 * @brief  BC20��λ����
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��1��25��
*/
void NBIOT_RST(void)//��ʼ��
{
	NBIOT_CMD_SEND(NBIOTRST,"2",sizeof(NBIOTRST),1);
}
 /**
 * @brief  BC20 AT���Ժ���
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��1��25��
*/
void NBIOT_AT_Test(void)//ɵ��ģ�鸴λ��������Ͷ����Ż᷵����Ϣ
{
	NBIOT_CMD_SEND(NBIOT_AT_TEST,0,sizeof(NBIOTRST),1);
}


