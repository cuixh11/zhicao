/**
 * @brief  Flash_Write and Read
 * @param	Address; Lenth; Sertor Num; ...
 * @retval
 * @author ZCD
 * @Time 2020 4 12 
*/

#ifndef __Flash_Operation_H
#define __Flash_Operation_H

#include "stdint.h"

#define FlashWrite_Lenth sizeof(FlashWrite_Buff)
#define FlashWrite_Size FlashWrite_Lenth/4 + ((FlashWrite_Lenth%4)?1:0)

//------------------Bank1---------------------//
#define ADDR_Flash_Sector_0 0x08000000	//16k
#define ADDR_Flash_Sector_1 0x08004000	//16k
#define ADDR_Flash_Sector_2 0x08008000	//16k
#define ADDR_Flash_Sector_3 0x0800C000	//16k
#define ADDR_Flash_Sector_4 0x08010000	//64k
#define ADDR_Flash_Sector_5 0x08020000	//128k
#define ADDR_Flash_Sector_6 0x08040000	//128k
#define ADDR_Flash_Sector_7 0x08060000	//128k
#define ADDR_Flash_Sector_8 0x08080000	//128k
#define ADDR_Flash_Sector_9 0x080A0000	//128k
#define ADDR_Flash_Sector_10 0x080C0000	//128k
#define ADDR_Flash_Sector_11 0x080E0000	//128k
//------------------Bank2--------------------//
#define ADDR_Flash_Sector_12 0x08100000	//16k
#define ADDR_Flash_Sector_13 0x08104000 //16k
#define ADDR_Flash_Sector_14 0x08108000 //16k
#define ADDR_Flash_Sector_15 0x0810C000 //16k
#define ADDR_Flash_Sector_16 0x08110000 //64k
#define ADDR_Flash_Sector_17 0x08120000 //128k
#define ADDR_Flash_Sector_18 0x08140000 //128k
#define ADDR_Flash_Sector_19 0x08160000 //128k
#define ADDR_Flash_Sector_20 0x08180000 //128k
#define ADDR_Flash_Sector_21 0x081A0000 //128k
#define ADDR_Flash_Sector_22 0x081C0000 //128k
#define ADDR_Flash_Sector_23 0x081E0000 //128k

//-------------------------------------------//
uint32_t Flash_ReadWord(uint32_t Read_Addr);
uint8_t Flash_GetFlashSectorNUM(uint32_t addr);
void Flash_Write(uint32_t WriteAddr,uint32_t Num_of_Write,uint32_t *Write_pBuffer);
void Flash_Read(uint32_t ReadAddr,uint32_t Num_of_Read,uint32_t *Read_pBuffer);
void Flash_Erase_Sector(uint8_t Erase_Sector_Num);
//-------------------------------------------//
extern uint32_t Sector_ERROR;
extern uint8_t Erase_ERROR;
extern uint8_t Write_ERROR;
extern uint8_t WriteADDR_Start_ERROR;


//-------------------------------------------//





#endif


