#ifndef __IAP_H
#define __IAP_H

#include "Flash_Operation.h"
#include "hw_abst.h"
#include "md5.h"
#include "OTA.h"

#define FirmwareDownloadAreaFlash_StartPtr ADDR_Flash_Sector_7 //0x08060000
#define FirmwareINFO_Offset 3*16 //ƫ�����Ŀռ�������Ź̼���Ϣ
#define FirmwareINFO_Size 8

extern uint8_t IAP_packageBuff[1024+PackageAddtINFO_Size];

typedef struct 
{
    char FirmwareType[8];//�̼����õ��豸����
		char FirmwareVersion[4];//�̼��汾��,�̼��汾��Ч�����Ǻ�3λ,��һλֻ��Y\N �ֱ����������Ч�̼�
    uint32_t FirmwareSize;//�̼���С����λ���ֽڣ�
		uint8_t Firmware_MD5[16];//�̼���MD5

}FirmwareINFO_t;


uint8_t Update_FirmwareINFO(void);
uint8_t Wirte_firmwareINFO(FirmwareINFO_t NewFirmwareINFO_t);
uint8_t Read_FirmwarePackage_To_RAMBuff(uint32_t FirmwareStartAddr,uint32_t Read_Len_t,uint32_t * RAM_pBuff);
uint8_t Write_FirmwarePackageBuff_To_Flash(uint8_t *packageBuff_t,uint32_t buffInfoSize_t);

#endif 

