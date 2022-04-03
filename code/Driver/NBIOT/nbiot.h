#ifndef __nbiot_H
#define __nbiot_H
#include "stdint.h"
#include "hw_abst.h"
#include "string.h"
#include "BC20.h"

#define NBIOT_SEND_BUFF_MAX 500

extern uint8_t NBIOT_Connected_to_Server_Flag;
extern uint8_t SmartNest_Domain_NBIOT[];
extern uint8_t SmartNest_Domain_NBIOTLEN;

void NBIOT_Init(void);
uint8_t NBIOT_SEND_PUSH(uint8_t ConnNUM,uint16_t SendLen,char* Send_Buff);
void nbiot_build_connect(uint8_t *TargetIP,uint16_t len_t);


#endif


