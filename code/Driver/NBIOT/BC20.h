#ifndef __BC20_H
#define __BC20_H
#include "stdint.h"
#include "hw_abst.h"
#include "string.h"
#include "nbiot.h"

#define NBIOT_AT_TEST "AT"
#define NBIOTRST_Check "AT+QPOWD=?"//查询模块响应范围
#define NBIOTRST "AT+QPOWD="//AT+QPOWD=0 正常关机、1强制关机、2使模块重启
#define NBIOT_ATBACK0 "ATE0"//设置串口回显，0关闭回显
#define NBIOT_ATBACK1 "ATE1"//开启回显
#define NBIOT_PIN_Check "AT+CPIN?"//检查是否需要输入pin，不需要返回 READY
#define NBIOT_CSQ "AT+CSQ"//检查信号质量 （0-31）
#define NBIOT_OPEN_Socket "AT+QIOPEN="//打开Socket服务，用于建立TCP/IP连接
#define NBIOT_CLOSE_Socket "AT+QICLOSE="//关闭连接（0~4）
#define NBIOT_STATE_Socket "AT+QISTATE?"//查询连接状态
#define NBIOT_SEND "AT+QISEND="//发送数据
#define NBIOT_SWTMD "AT+QISWTMD="//切换数据读取模式 0 buff模式 1 push模式
#define NBIOT_PING "AT+QPING="//ping 某个地址
#define NBIOT_GETERROR "AT+QIGETERROR"//查询上个错误
#define NBIOT_READ_DATA "AT+QIRD="//buff接收模式下，读取收到的数据




void NBIOT_CMD_SEND(char *cmd,char* cmd2,uint8_t LenTemp,uint8_t LenTemp2);
void NBIOT_RST(void);
void NBIOT_Rece_PUSH(void);
void NBIOT_DATA_SEND(char *data,uint16_t data_len);
void NBIOT_AT_Test(void);

extern uint8_t NBIOT_TCPLink_Alive;
extern uint8_t NBIOT_Received_Flag;
#endif

