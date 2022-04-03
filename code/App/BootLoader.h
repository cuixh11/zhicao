/*
 * @brief  Boot loader
 * @time 2021 12 22
 * @paramer zcd 
*/

#ifndef __Bootloader_H
#define __Bootloader_H

#include "stdint.h"
#include "main.h"

#define FLASH_APP1_ADDR                 0x08008000 //Ӧ�÷���1��ʼ��ַ
#define APP1_VTOR_offset                0x8000
#define FLASH_APP2_ADDR                 0x08040000 //Ӧ�÷���2��ʼ��ַ
#define APP2_VTOR_offset                0x40000

#define APPDisk_1 1
#define APPDisk_2 2

extern uint8_t v1[];
extern uint8_t v2[];
extern uint8_t v3[];
extern uint8_t v4[];

uint8_t Jump_To_APPDisk(uint8_t APPDisk_NUM);

#endif

