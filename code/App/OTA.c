/**
 * @brief OTA 功能实现以及联网下载功能、校验之类的
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2022年3月25日
*/
#include "OTA.h"

uint8_t FirmwareMD5_Target[33]={0};//固件正确的MD5值，MD5为32字节长度；
uint16_t Package_NUM=0;
uint16_t Package_total=0;

