#ifndef __SmartNest_server_H
#define __SmartNest_server_H

#define Server_SEND_BuffLen_MAX 800

#include "http_json.h"
#include "host_app.h"

extern int Upload_pressTest_CNT;
extern char UploadData_buff[Json_packed_BUFF_MAX-100];
extern char Upload_DataTemp[83];

int8_t Register_Host(void);
int8_t Post_to_Server(void);
int8_t Login_Uesr(char *UserName_t,char *Password_t);
int8_t Send_to_Server(uint8_t *Send_temp);
int8_t NetTime_Refesh(void);
int8_t Load_SN(char *ID_t,uint8_t ID_Len);
int8_t Load_host_name(char *hostName_t);
int8_t Register_Device(void);
int8_t Upload_DeviceData(void);
void Load_TimeStamp(char *OperationBuff);
int8_t Connect_Server_Smatnest(void);
int8_t Load_DeviceSN(uint8_t DeviceID);
int8_t Load_Sensor_Data(char* OperationBuff,uint8_t DeviceID_t);
void Reg_Device_ALL(void);
int8_t Upload_DeviceData_NBIOT(void);
int8_t PUSH_Serve(void);
#endif


