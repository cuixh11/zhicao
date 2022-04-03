#ifndef __http_json_H
#define __http_json_H

#include "wifi.h"

#if defined HostAPP
	#include "jansson.h"
	//path: C:\Program Files (x86)\keilv5\pack\Keil\Jansson\1.0.0\src
#endif

#define json_unpack_max 500
#define Json_packed_BUFF_MAX 500
#define Head_Len_MAX 300
#define UserName_LenMAX 20
#define Password_LenMAX 20
#define Host_sn_LenMAX 20
#define Host_type_LenMAX 20

typedef struct 
{
	int16_t State_code;
	char Date[30];
	//char *Content_Type;
	
	uint8_t Connection;

	uint16_t Content_Lenth;	
	uint16_t json_Start;

	char *Cooke;
	char utoken_Live;		
	char token[32];

}Body_head;

typedef struct 
{
	uint16_t Str_Len;	
	char *Str;	
	char *Str_p;

}Json_str_t;

//----------------------------------------------------------//
extern uint16_t year;
extern uint16_t month;
extern uint16_t day ;
extern uint16_t weekday;
extern uint16_t hour ;
extern uint16_t min ;   
extern uint16_t sec;
extern uint16_t year_token;
extern uint16_t month_token;
extern uint16_t day_token;
extern uint16_t hour_token;
extern uint16_t min_token;
extern uint16_t sec_token;

extern uint8_t deviceReg_SucCNT;
extern uint8_t deviceReg_ErrCNT;

//----------------------------------------------------------//
extern Json_str_t userName;
extern Json_str_t Login_token;
extern Json_str_t Password;
extern Json_str_t Host_sn;
extern Json_str_t Host_type;

extern Json_str_t Device_Suc_SN;
extern Json_str_t Device_Err_SN;

//----------------------------------------------------------//
extern int state_p;//服务器返回状态 0正常
extern char *info_p;
extern int16_t Unpack_return;
extern int16_t Pack_return;

extern uint8_t Json_Delete_Enable_Flag;//销毁Root指针释放堆空间,销毁前要求必须将堆内数据拷出保护
extern Body_head BodyHead_pro;

extern char Host_Name[20];
extern uint16_t UID;
extern uint16_t HID;

extern char Head_Send_Buff[Head_Len_MAX];
extern uint16_t Head_Send_BuffPoint;

extern char Json_packed_BUFF[300];
extern uint8_t PackBuff_Using_flag;
extern char DeviceSN_single[17];

extern uint8_t Display_unpackBuff_Flag;
extern uint8_t Display_packBuff_Flag;


uint8_t Json_Str_typeInit(uint16_t Len,Json_str_t *Str_struct);
int16_t JsonUnpack(char* json_unpack_type);
int8_t JsonPack(char* json_pack_tpye);
uint8_t json_unpack_copy(char *output,char* temp);
void json_Space_init(void);
uint8_t Body_Process(void);
uint8_t Body_head_Make(char *CMD,uint16_t CMD_Len ,uint8_t Content_Type,uint16_t Content_Length);
void utoken_input_manually(char * utoken_temp);

#endif 

