#ifndef __host_app_H
#define __host_app_H

#include "http_json.h"
#include "SmartNest_server.h"
#include "Flash_Operation.h"


#include "lcd.h"

#if defined HostAPP
	#include "IAP.h"
	#include "nbiot.h"
	#include "cc2530.h"
#endif

#define UserInfo_FlashSaveAddr 0x080E0000		//Sector_11
#define ExistenceCheck1 0xAA
#define ExistenceCheck2 0x55
#define ExistenceCheck3 0xBB
#define ExistenceCheck4 0x44

#define OfflineDATAStack_MAX 1000//字节
#define StackUnitSize 36//*\1_ID\4_Temper\3_RH\5_Lux\4_CO2\4_NH3\12_time#' '		//存储栈一个单元的大小（字节）


extern uint8_t Changed_UserInfo;
extern uint8_t Registed_User;
extern uint8_t Debug_enable_flag;

extern uint16_t StackHeader_NUM;//队列最前的数量
extern uint16_t StackRemainingSpace_BYTE;//剩余可用空间（字节）
extern uint8_t Offline_DATA_Stack[OfflineDATAStack_MAX] _no_init;
extern uint16_t OfflineDATA_PowerON_Judge;
extern uint8_t PowerON_Flag;

extern uint8_t StackTimeStamp_Flag;
extern char HistoryDataPOP_BUFF[StackUnitSize+1];
extern int8_t UploadDataAPP_Return;

extern uint8_t CheckTCP_Return;
extern uint8_t CloseTCP_Return;

int8_t Host_SystemStartUp_SmartNest(void);
int8_t Host_test_SmartNest(void);
int8_t Host_utoken_Check_SmartNest(void);
int8_t Reg_Host_App(void);
int8_t Reg_User_Check(void);
int8_t Flash_Failure_Mark(void);
int8_t Flash_ValidMark_Equilibrium(uint8_t Write_Flag);
int8_t Flash_Read_UserInfo(void);
void SmartNest_Debug_APP(void);
void UART_Print_Number(int NumtoPrint);
void Local_time_refresh(uint8_t TimeTick);
int8_t Distribution_via_app(void);

int8_t maintain_Offline_data_BUFF(void);
int8_t Offline_Data_PUSH(uint8_t *DataStackInput_t);
int8_t Offline_Data_POP(uint8_t *DataStackOutput_t);

int8_t Sensor_PUSH_APP(uint8_t DeviceID_t);
int8_t Sensor_POP_APP(char * OutputBUFF_t);
int8_t SmartNest_Main_Function(void);
int8_t Upload_Data_APP(void);
uint8_t Connection_ControlAPP(void);
uint8_t LED_State_CtrlAPP(void);
#endif

