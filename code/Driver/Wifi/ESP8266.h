 /**
 * @brief  ESP8266驱动
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021年1月24日
*/
#ifndef __ESP8266_H
#define __ESP8266_H

#include "usart.h"
#include "hw_abst.h"
//#include "freertos.h"
//#include "cmsis_os.h"
//#include "Time_Soft.h"

#define WifiRST "AT+RST"//RST
#define WifiMODECheck "AT+CWMODE?"//查看工作模式
#define WifiMODE "AT+CWMODE="//1-STA  2-AP  3-AP+STA 设置工作模式

#define WifiAPList "AT+CWLAP"//wifi列表 <ecn><ssid><rssi>
#define WifiJoinAP "AT+CWJAP="//加入AP；AT+CWJAP="www.zniot.com","littlebee" （其中littlebee为密码）
#define WifiJoinAPSuces "AT+CWJAP?"//加入成功？  返回AP的SSID、MAC；自己工作模式、信号强度

#define WifiSetAP "AT+CWSAP="//设置ap；AT+CWSAP=“ESP8266”,“12345678”,11,3（ssid,password,chl,ecn）
														 //chl-信道号 ecn-加密方式 0-open 1-WEP 2-WPA_PSK 3-WPA2_PSK 4-WPA_WPA2_PSK
#define WifiIPList "AT+CWLIF"//查看接入设备的IP

#define WifiIPCheck "AT+CIFSR=?"//查模块是否有IP,返回OK\ERROR
#define WifiIP "AT+CIFSR"//查看自己IP和MAC   （AP模式下不能用，有可能导致死机）
#define WifiNETSTA "AT+CIPSTA?"//查看网络信息 ip 网关 子网掩码

#define WifiMUXCheck "AT+CIPMUX?"//查看是否允许多连接
#define WifiMUX "AT+CIPMUX="//模块多连接 1-是 0-否  AT+CIPMUX?是否建立多连接	//AT+ CIPMUX=1 时才能开启服务器，关闭 server 模式需要重启；
														//开启 server 后自动建立 server 监听,当有 client 接入会自动按顺序占用一个连接。

#define WifiBuildConnect "AT+CIPSTART="//建立连接  AT+CIPSTART=4,"TCP"," X1.X2.X3.X4",9999
																								 //AT+CIPSTART="UDP","192.168.0.251",8080,1112,0  |8080 为目的端口 1112为本地端口 0表示固定远端 2表示远端可变
																								 //单路连接时(+CIPMUX=0)，指令为： AT+CIPSTART= <type>,<addr>,<port>
																								 //多路连接时(+CIPMUX=1)，指令为： AT+CIPSTART=<id>,<type>,<addr>,<port>
#define WifiConnectList "AT+CIPSTATUS"//查看当前连接 （id,type,addr,port,tetype）<tetype  0-cilent 1-server>

#define WifiSendMODE "AT+CIPMODE="//发送模式 1-透传 0非透传 AT+CIPMODE? 查询透传模式
#define WifiSEND "AT+CIPSEND"//透传发送 
#define WifiSENDLen "AT+CIPSEND="//定长发送  单路连接时(+CIPMUX=0)，指令为： AT+CIPSEND=<length>；
																 //多路连接时(+CIPMUX=1) ，指令为： AT+CIPSEND= <id>,<length> 最大长度为 2048

#define WifiTCPServer "AT+CIPSERVER="//AT+CIPSERVER=1,8888（其中1 为开启 如果设为0 则关闭 8888：要监听端口）
#define WifiServerTime "AT+CIPSTO="//设置服务超时时间  AT+CIPSTO？查询超时时间
#define WifiConnectClose "AT+CIPCLOSE="//AT+CIPCLOSE=4 断开四号连接
#define WifiConnectCloseALL "AT+CIPCLOSE"//AT+CIPCLOSE 断开连接;无连接时执行返回ERROR

#define WifiSYSID "AT+CSYSID"//查看wifi芯片ID
#define WifiGMR "AT+GMR"//查看模块固件

#define WifiUsedUART &huart1

void Wifi_CMD_SEND(char *cmd,char* cmd2,uint8_t LenTemp,uint8_t LenTemp2);
void Wifi_RST(void);
void Wifi_Connect_AP(char* SSID,char* PassWord);
void Wifi_Connect_AP2(void);
uint8_t StringSearch(char *cha,uint8_t Len/*Len<=10*/,uint16_t Deep);
uint8_t StringSearch_Assign(char *cha,uint8_t *Assign_Buff,uint8_t Len/*Len<=10*/,uint16_t Deep);
uint8_t Wifi_Read_IP_MAC(void);
uint8_t Wifi_Read_WorkState(void);
uint8_t Wifi_ExitTransp_Send(void);
uint8_t Wifi_CLOSE_Connection(char* Connect_ID);
uint8_t Wifi_ModeSet(uint8_t mode);
int8_t Check_TCP_Connection(void);
uint8_t Wifi_Check_AP_Name(void);
uint8_t Wifi_Read_Getway_Mask(void);
uint8_t Wifi_CIPMUX_Ctrl(uint8_t MUXMode_t);
uint8_t Wifi_TCPServer_Ctrl(uint8_t ServerState_t);



extern uint16_t StringSecanPointer;
extern uint16_t StringSecanPointer_Assign_Buff;

extern uint8_t Wifi_module_State;

extern char Wifi_MAC[12];
extern char Wifi_STAIP[16];
extern char wifi_AP_GETWAY[16];//自己做AP的网关

extern char wifi_GATEWAY[16];//连接的AP网关
extern char wifi_MASK[16];

extern uint8_t Connection_List[9];
extern uint8_t Close_Connection_Return;
extern uint8_t TCPLink_Alive;


//void Wifi_ControlThreadCreate(osPriority taskPriority);


#endif

