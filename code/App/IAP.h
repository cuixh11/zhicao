#ifndef __IAP_H
#define __IAP_H

#include "Flash_Operation.h"
#include "hw_abst.h"
#include "md5.h"
#include "OTA.h"

#define FirmwareDownloadAreaFlash_StartPtr ADDR_Flash_Sector_7 //0x08060000
#define FirmwareINFO_Offset 3*16 //偏移区的空间用来存放固件信息
#define FirmwareINFO_Size 8

extern uint8_t IAP_packageBuff[1024+PackageAddtINFO_Size];

typedef struct 
{
    char FirmwareType[8];//固件适用的设备类型
		char FirmwareVersion[4];//固件版本号,固件版本有效数字是后3位,第一位只有Y\N 分别代表有无有效固件
    uint32_t FirmwareSize;//固件大小，单位（字节）
		uint8_t Firmware_MD5[16];//固件的MD5

}FirmwareINFO_t;


uint8_t Update_FirmwareINFO(void);
uint8_t Wirte_firmwareINFO(FirmwareINFO_t NewFirmwareINFO_t);
uint8_t Read_FirmwarePackage_To_RAMBuff(uint32_t FirmwareStartAddr,uint32_t Read_Len_t,uint32_t * RAM_pBuff);
uint8_t Write_FirmwarePackageBuff_To_Flash(uint8_t *packageBuff_t,uint32_t buffInfoSize_t);

#endif 

