#ifndef __BC20_H
#define __BC20_H
#include "stdint.h"
#include "hw_abst.h"
#include "string.h"
#include "nbiot.h"

#define NBIOT_AT_TEST "AT"
#define NBIOTRST_Check "AT+QPOWD=?"//��ѯģ����Ӧ��Χ
#define NBIOTRST "AT+QPOWD="//AT+QPOWD=0 �����ػ���1ǿ�ƹػ���2ʹģ������
#define NBIOT_ATBACK0 "ATE0"//���ô��ڻ��ԣ�0�رջ���
#define NBIOT_ATBACK1 "ATE1"//��������
#define NBIOT_PIN_Check "AT+CPIN?"//����Ƿ���Ҫ����pin������Ҫ���� READY
#define NBIOT_CSQ "AT+CSQ"//����ź����� ��0-31��
#define NBIOT_OPEN_Socket "AT+QIOPEN="//��Socket�������ڽ���TCP/IP����
#define NBIOT_CLOSE_Socket "AT+QICLOSE="//�ر����ӣ�0~4��
#define NBIOT_STATE_Socket "AT+QISTATE?"//��ѯ����״̬
#define NBIOT_SEND "AT+QISEND="//��������
#define NBIOT_SWTMD "AT+QISWTMD="//�л����ݶ�ȡģʽ 0 buffģʽ 1 pushģʽ
#define NBIOT_PING "AT+QPING="//ping ĳ����ַ
#define NBIOT_GETERROR "AT+QIGETERROR"//��ѯ�ϸ�����
#define NBIOT_READ_DATA "AT+QIRD="//buff����ģʽ�£���ȡ�յ�������




void NBIOT_CMD_SEND(char *cmd,char* cmd2,uint8_t LenTemp,uint8_t LenTemp2);
void NBIOT_RST(void);
void NBIOT_Rece_PUSH(void);
void NBIOT_DATA_SEND(char *data,uint16_t data_len);
void NBIOT_AT_Test(void);

extern uint8_t NBIOT_TCPLink_Alive;
extern uint8_t NBIOT_Received_Flag;
#endif

