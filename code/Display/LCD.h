#ifndef __LCD_H_
#define __LCD_H_

#include "hw_abst.h"

#define Wake_Up_Status 0
#define Sleep_Status   1

#define Page0StrRow_MAX 40
#define Page0StrLine_MAX 14
#define Page0StrFullSpace 40*9

void HMI_Init(void);                //初始化串口屏
void LCD_Status(uint8_t status);      //LCD是否休眠状态


void HMISend_End(uint8_t type); 
void HMISends(char *pData,uint16_t send_len)		;  //字符串发送函数
void Uart_Display_String(char *pData,uint32_t LineIndex);  //完整显示一次字符串
uint8_t LCD_StringBUFF_Manage(char *InputChar_t);
void LCD_Clear_all(void);
void LCD_Startup_ProgressBar(uint8_t StartUP_Progress);
void HMI_Rest(void);
uint8_t Page_Switch(uint8_t pageNum);
uint8_t Check_box(uint8_t num,uint8_t state);

extern uint16_t DisplaySleep_TimeCNT;
extern uint8_t HMI_Display_RefeshDisableFlag;
extern uint8_t Page_NOW;
uint8_t page1(void);
#endif

