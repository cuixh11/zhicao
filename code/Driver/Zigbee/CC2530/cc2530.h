#ifndef __cc2530_H
#define __cc2530_H

#include "hw_abst.h"
#include "stdint.h"
#include "usart.h"
#include "string.h"

#define DeviceNUM_MAX 6////×î´ó6
#define CMD_Head1_cc2530 0xAA
#define CMD_Head2_cc2530 0xBB
#define CMD_End1_cc2530 0x0D
#define CMD_End2_cc2530 0x0A

#define RecvBUFF_cc2530_MAX 50

typedef struct 
{
	char DeviceSN[16];
	uint8_t DeviceID;
	float Temperture;
	uint32_t RH;
	uint16_t CO2;
	uint16_t NH3;
	uint16_t Lux;	
}Device_t;

extern float Temperture_D;
extern uint32_t RH_D;
extern uint16_t CO2_D;
extern uint16_t NH3_D;
extern uint16_t Lux_D;
extern Device_t DeviceSNList[6];
extern uint8_t DeviceOnline_Falg[DeviceNUM_MAX];

extern uint8_t Device_BUFF_T[RecvBUFF_cc2530_MAX];

void zigbee_Read_Sensor(uint8_t Device_num,uint8_t Sensor_type,Device_t *DeviceSave_t);
uint8_t ZigbeeRead_ALL_Sensor(void);
uint8_t zigbee_Read_MAC(uint8_t device_ID,Device_t *DeviceList_t);
int8_t zigbee_ReadData(uint8_t Device_num,uint8_t Sensor_type);
uint8_t zigbee_Refesh_OnlineDeviceNUM(void);
uint8_t ZigBeeNET_Init(void);
uint8_t zigbeeClear_StructData(uint8_t Device_ID_t);
#endif


