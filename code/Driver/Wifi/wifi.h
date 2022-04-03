#ifndef __wifi_H
#define __wifi_H
#include "stdint.h"
#include "usart.h"
#include "ESP8266.h"


#if defined Bootloader
	
#endif

#if defined HostAPP
	#include "jansson.h"
#endif

extern uint8_t UDP_Target[];
extern uint8_t TCP_Target[];
extern uint8_t SmartNest_Domain[];
extern uint8_t UDP_TargetLe;
extern uint8_t TCP_TargetLen;
extern uint8_t SmartNest_DomainLEN;


extern uint8_t UDP_TargetLen;
extern uint8_t UDP_Target[];
extern uint8_t WifiInitOKflag;
extern uint16_t FreqTimeCnt;

extern uint8_t Wifi_Connect_AP_Flag;
extern uint8_t WifiRST_OK_Flag;
extern uint8_t WifiInitOKflag;
extern uint8_t GotIP_Flag;	
extern uint8_t Connected_to_Server_Flag;
extern uint8_t Wifi_Transparent_Send_Flag;
extern uint8_t Wifi_Transparent_SendEntered_Flag;
extern uint8_t Switched_AP;

void wifi_build_connect(uint8_t Type,uint8_t *TargetIP,uint8_t len);
void Wifi_Init(void);
void Get_NETTime(void);
uint8_t Print_Time(uint16_t year_t,uint16_t month_t,uint16_t day_t,uint16_t hour_t,uint16_t min_t,uint16_t sec_t,uint16_t weekady_t);
uint8_t Switch_Wifi_AP(void);
int8_t Start_Transp_Send(void);
uint8_t Wifi_CheckSwitch_Workstate(uint8_t DefaultMode_t);
#endif

