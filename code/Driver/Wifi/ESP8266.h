 /**
 * @brief  ESP8266����
 * @param	void
 * @retval	void
 * @author ZCD
 * @Time 2021��1��24��
*/
#ifndef __ESP8266_H
#define __ESP8266_H

#include "usart.h"
#include "hw_abst.h"
//#include "freertos.h"
//#include "cmsis_os.h"
//#include "Time_Soft.h"

#define WifiRST "AT+RST"//RST
#define WifiMODECheck "AT+CWMODE?"//�鿴����ģʽ
#define WifiMODE "AT+CWMODE="//1-STA  2-AP  3-AP+STA ���ù���ģʽ

#define WifiAPList "AT+CWLAP"//wifi�б� <ecn><ssid><rssi>
#define WifiJoinAP "AT+CWJAP="//����AP��AT+CWJAP="www.zniot.com","littlebee" ������littlebeeΪ���룩
#define WifiJoinAPSuces "AT+CWJAP?"//����ɹ���  ����AP��SSID��MAC���Լ�����ģʽ���ź�ǿ��

#define WifiSetAP "AT+CWSAP="//����ap��AT+CWSAP=��ESP8266��,��12345678��,11,3��ssid,password,chl,ecn��
														 //chl-�ŵ��� ecn-���ܷ�ʽ 0-open 1-WEP 2-WPA_PSK 3-WPA2_PSK 4-WPA_WPA2_PSK
#define WifiIPList "AT+CWLIF"//�鿴�����豸��IP

#define WifiIPCheck "AT+CIFSR=?"//��ģ���Ƿ���IP,����OK\ERROR
#define WifiIP "AT+CIFSR"//�鿴�Լ�IP��MAC   ��APģʽ�²����ã��п��ܵ���������
#define WifiNETSTA "AT+CIPSTA?"//�鿴������Ϣ ip ���� ��������

#define WifiMUXCheck "AT+CIPMUX?"//�鿴�Ƿ����������
#define WifiMUX "AT+CIPMUX="//ģ������� 1-�� 0-��  AT+CIPMUX?�Ƿ���������	//AT+ CIPMUX=1 ʱ���ܿ������������ر� server ģʽ��Ҫ������
														//���� server ���Զ����� server ����,���� client ������Զ���˳��ռ��һ�����ӡ�

#define WifiBuildConnect "AT+CIPSTART="//��������  AT+CIPSTART=4,"TCP"," X1.X2.X3.X4",9999
																								 //AT+CIPSTART="UDP","192.168.0.251",8080,1112,0  |8080 ΪĿ�Ķ˿� 1112Ϊ���ض˿� 0��ʾ�̶�Զ�� 2��ʾԶ�˿ɱ�
																								 //��·����ʱ(+CIPMUX=0)��ָ��Ϊ�� AT+CIPSTART= <type>,<addr>,<port>
																								 //��·����ʱ(+CIPMUX=1)��ָ��Ϊ�� AT+CIPSTART=<id>,<type>,<addr>,<port>
#define WifiConnectList "AT+CIPSTATUS"//�鿴��ǰ���� ��id,type,addr,port,tetype��<tetype  0-cilent 1-server>

#define WifiSendMODE "AT+CIPMODE="//����ģʽ 1-͸�� 0��͸�� AT+CIPMODE? ��ѯ͸��ģʽ
#define WifiSEND "AT+CIPSEND"//͸������ 
#define WifiSENDLen "AT+CIPSEND="//��������  ��·����ʱ(+CIPMUX=0)��ָ��Ϊ�� AT+CIPSEND=<length>��
																 //��·����ʱ(+CIPMUX=1) ��ָ��Ϊ�� AT+CIPSEND= <id>,<length> ��󳤶�Ϊ 2048

#define WifiTCPServer "AT+CIPSERVER="//AT+CIPSERVER=1,8888������1 Ϊ���� �����Ϊ0 ��ر� 8888��Ҫ�����˿ڣ�
#define WifiServerTime "AT+CIPSTO="//���÷���ʱʱ��  AT+CIPSTO����ѯ��ʱʱ��
#define WifiConnectClose "AT+CIPCLOSE="//AT+CIPCLOSE=4 �Ͽ��ĺ�����
#define WifiConnectCloseALL "AT+CIPCLOSE"//AT+CIPCLOSE �Ͽ�����;������ʱִ�з���ERROR

#define WifiSYSID "AT+CSYSID"//�鿴wifiоƬID
#define WifiGMR "AT+GMR"//�鿴ģ��̼�

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
extern char wifi_AP_GETWAY[16];//�Լ���AP������

extern char wifi_GATEWAY[16];//���ӵ�AP����
extern char wifi_MASK[16];

extern uint8_t Connection_List[9];
extern uint8_t Close_Connection_Return;
extern uint8_t TCPLink_Alive;


//void Wifi_ControlThreadCreate(osPriority taskPriority);


#endif

