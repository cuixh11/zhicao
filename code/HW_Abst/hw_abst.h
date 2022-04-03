#ifndef __hw_abst_H
#define __hw_abst_H

#include "stdint.h"
#include "usart.h"
#include "string.h"
#include "USB_HID_Z.h"

#define _no_init __attribute__((section("NO_INIT"),zero_init))

#define WaitUntilFlagTrue_MAX 30000 //单位ms，最大30s

extern uint8_t KeyState[3];

void MCU_System_RST(void);
void Delay(uint32_t delay_Time);
void DebugPrint_uart(char *pData,  uint32_t Timeout);
void UART_Send(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
uint8_t StringMatch(char* input ,uint8_t input_Len,char *target,uint8_t target_Len);
uint16_t Val_MAX_Limit_U(uint16_t inputNUM,uint16_t MAX_NUM);
void DebugPrint_uart_LineFeed(char *pData,  uint32_t Timeout);
int8_t Wait_Until_FlagTrue(uint8_t *Flag,uint32_t Timeout);
int8_t Uart_Debug_CMD_ADD(char *Cmd_t,uint8_t *UartBuff,uint8_t CmdLen_t,uint16_t MatchDeep_t);
GPIO_PinState GPIO_Read(GPIO_TypeDef* GPIOx_t, uint16_t GPIO_Pin_t);
void Clear_Buff(char *Buff,uint16_t Len,uint16_t Buff_Max);
void Key_read(void);
void LED_ctrl(uint8_t LEDID,uint8_t LED_state);
#endif 

