/**
 * @brief  Flash_Write and Read
 * @param	Address; Lenth; Sertor Num; ...
 * @retval
 * @author ZCD
 * @Time 2020 4 12 
*/

#include "stm32f4xx_hal.h"
#include "stm32_hal_legacy.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include "Flash_Operation.h"

uint32_t Addr_Start=0;
uint32_t Addr_END=0;
uint32_t Sector_ERROR=0;

uint8_t Erase_ERROR=0;
uint8_t Write_ERROR=0;
uint8_t WriteADDR_Start_ERROR=0;

HAL_StatusTypeDef FlashStatus;
FLASH_EraseInitTypeDef FlashErase_Init;



/**
 * @brief Read specified address data (32bit width)
 * @param address: Read_Addr
 * @retval *(volatile uint32_t*)Read_Addr
 * @Time 2020 4 12
*/
uint32_t Flash_ReadWord(uint32_t Read_Addr)
{
	return *(volatile uint32_t*)Read_Addr;
}

/**
 * @brief Erase specified Sector data
 * @param address:  uint8_t Erase_Sector_Num
 * @retval void
 * @WatchOut * ������������ж����������Ƿ������ݣ�����ȫ����������������ע�����ݱ���
			 * �����������ź��U ���� 5U
			 * �������������Ǵ洢��������顣����Ĭ��������ʽʱ�������һ������Ϊ 0 �����������忴�����С����ȷ��
 * @Time 2020 4 12
*/
void Flash_Erase_Sector(uint8_t Erase_Sector_Num)
{
	FlashStatus = HAL_OK;
	HAL_FLASH_Unlock();	
	FlashErase_Init.TypeErase = FLASH_TYPEERASE_SECTORS;
	FlashErase_Init.Sector = Erase_Sector_Num;
	FlashErase_Init.NbSectors = 1;
	FlashErase_Init.VoltageRange = FLASH_VOLTAGE_RANGE_3;	//2.7~3.6 �ڲ����� 			
	if(HAL_FLASHEx_Erase(&FlashErase_Init, &Sector_ERROR)!=HAL_OK)
	{
		Erase_ERROR=1;
		return ;
	}	
	HAL_FLASH_Lock();
}


/**
 * @brief Get sector number
 * @param address: uint32_t Read_Addr
 * @retval uint8_t SectorNum
 * @Time 2020 4 12
*/
uint8_t Flash_GetFlashSectorNUM(uint32_t addr)
{
	if(addr<ADDR_Flash_Sector_1)
		return FLASH_SECTOR_0;
	else if (addr<ADDR_Flash_Sector_2)
		return FLASH_SECTOR_1;
	else if (addr<ADDR_Flash_Sector_3)
		return FLASH_SECTOR_2;
	else if (addr<ADDR_Flash_Sector_4)
		return FLASH_SECTOR_3;
	else if (addr<ADDR_Flash_Sector_5)
		return FLASH_SECTOR_4;
	else if (addr<ADDR_Flash_Sector_6)
		return FLASH_SECTOR_5;
	else if (addr<ADDR_Flash_Sector_7)
		return FLASH_SECTOR_6;
	else if (addr<ADDR_Flash_Sector_8)
		return FLASH_SECTOR_7;
	else if (addr<ADDR_Flash_Sector_9)
		return FLASH_SECTOR_8;
	else if (addr<ADDR_Flash_Sector_10)
		return FLASH_SECTOR_9;
	else if (addr<ADDR_Flash_Sector_11)
		return FLASH_SECTOR_10;
	else if (addr<ADDR_Flash_Sector_12)
		return FLASH_SECTOR_11;
}
	

/**
 * @brief Write specified length data at the specified address
 * @param uint32_t WriteAddr; uint32_t Num_of_Write; uint32_t *Write_pBuffer
 * @retval void
 * @Time 2020 4 12
 * @WatchOut @ stm32f4�����������󣬲��ܱ���ȫ�����浽RAM�У�����
			   ��������ڲ����ǿ�����ʱ���Ὣ����Ԥ��ִ�в�������
			   ��������ݶ�ʧ��ע�������Ҫ���ݺʹ�������ݱ�����
			 @ �˺����ɶ�OTP��������ֻ��д��0�����ܲ�����
			   OTP��ַ��0X1FFF 7800~ 0X1FFF7A0F
			 @ WriteAddr������4�ı����� 
*/
void Flash_Write(uint32_t WriteAddr,\
				 uint32_t Num_of_Write,\
				 uint32_t *Write_pBuffer)
{
	FlashStatus = HAL_OK;
	
	if(WriteAddr<FLASH_BASE || WriteAddr%4)
	{	
		WriteADDR_Start_ERROR=1;
		return ;	
	}
	Addr_Start = WriteAddr;
//	if(!(Num_of_Write%4))
//	{
//		Addr_END = WriteAddr+Num_of_Write;
//	}
//	else
//	{
//		Addr_END = WriteAddr+Num_of_Write+(4-(Num_of_Write%4));	
//	}
	Addr_END = WriteAddr+Num_of_Write*4;

	HAL_FLASH_Unlock();
	__HAL_FLASH_DATA_CACHE_DISABLE();//FLASH�����ڼ�,�����ֹ���ݻ���

  if(Addr_Start<0x1fff0000)//0x1fff0000֮����ϵͳ�洢��������д����λ��ROP���û�ѡ���ֽڵȣ�0x1fff7800-0x1fff7a0f��OTP��
	{
		while(Addr_Start<Addr_END)
		{
			FLASH_WaitForLastOperation(50000);
			if(Flash_ReadWord(Addr_Start)!=0xFFFFFFFF)
			{
				FlashErase_Init.TypeErase = FLASH_TYPEERASE_SECTORS;
				FlashErase_Init.Sector = Flash_GetFlashSectorNUM(Addr_Start);
				FlashErase_Init.NbSectors = 1;
				FlashErase_Init.VoltageRange = FLASH_VOLTAGE_RANGE_3;//2.7~3.6 �ڲ����� 			
				if(HAL_FLASHEx_Erase(&FlashErase_Init, &Sector_ERROR)!=HAL_OK)
				{
					Erase_ERROR=1;
					break;
				}
			}
			else
			{
				Addr_Start+=4;
			}
		//	FLASH_WaitForLastOperation(50000);
		}
	}
	FlashStatus = FLASH_WaitForLastOperation(50000);
	if(FlashStatus == HAL_OK)
	{
		while(WriteAddr<Addr_END)
		{
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,WriteAddr,*Write_pBuffer)!=HAL_OK)
			{
				Write_ERROR=1;
				break;
			}
			WriteAddr+=4;
			Write_pBuffer++;
		}
		
	}
	
	__HAL_FLASH_DATA_CACHE_ENABLE();	//FLASH��������,�������ݻ���
	HAL_FLASH_Lock();
}

/**
 * @brief Read the specified length data from the specified address
 * @param uint32_t ReadAddr; uint32_t Num_of_Read; uint32_t *Read_pBuffer
					//Num_of_Readָ����һ���֣�32λ���������stm8����һ���ֽ�
 * @retval void
 * @Time 2020 4 13
 * @WatchOut  
*/
void Flash_Read(uint32_t ReadAddr,\
				uint32_t Num_of_Read,\
				uint32_t *Read_pBuffer)
{
	for(size_t i=0;i<Num_of_Read;i++)
	{
		Read_pBuffer[i]= Flash_ReadWord(ReadAddr);
		ReadAddr+=4;
	}
}


