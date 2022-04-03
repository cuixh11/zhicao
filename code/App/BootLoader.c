/*
 * @brief  Boot loader
 * @time 2021 12 22
 * @paramer zcd 
*/
#include "BootLoader.h"
#include "cmsis_armcc.h"
#include "LCD.h"

uint8_t v1[]="********************************\r\n";
uint8_t v2[]="*          BootLoader          *\r\n";
uint8_t v3[]="*      V0.0.6  Beta    ZCD     *\r\n";
uint8_t v4[]="********************************\r\n";

/**
 * @brief ��תAPP����ʵ��
 * @param	uint32_t APP_addr App��ַ
 * @retval uint8_t 
 * @author ZCD1300
 * @Time 2021��12��22��
*/
	uint32_t APP_Addr_t=0;	
typedef void (*jumpAPP)(void);
jumpAPP JumpAppStart;
uint8_t LoadAPP(uint32_t APP_addr)
{

	APP_Addr_t=APP_addr+4;
	if(((*(__IO uint32_t*)APP_addr) & 0x2FFE0000 ) == 0x20000000)//���APPջָ���Ƿ�Ϸ�
	{
		__disable_irq();		
		JumpAppStart=(jumpAPP)*(__IO uint32_t*) APP_Addr_t;//ָ�����ο�ʼ��ַ	
		__set_MSP(*(__IO uint32_t*) APP_Addr_t);//����ջ��ָ��
		JumpAppStart();//��ʼ��ת
	}
	else
	{
		return 1;
	}
}
/**
 * @brief APP��ת���ƺ�����ʵ��Ӧ���ô˺���
 * @param	void 
 * @retval int8_t return  0û��AP���� 1��AP����
 * @author ZCD1300
 * @Time 2021��12��22��
*/
uint8_t Jump_To_APPDisk(uint8_t APPDisk_NUM)
{
	if(APPDisk_NUM==APPDisk_1)
	{
		LoadAPP(FLASH_APP1_ADDR);

	}
	else if(APPDisk_NUM==APPDisk_2)
	{
		//LoadAPP(FLASH_APP2_ADDR);
	}
	else
	{//error
	
	}
}
/**
 * @brief APP��ת���ƺ�����ʵ��Ӧ���ô˺���
 * @param	void 
 * @retval int8_t return  0û��AP���� 1��AP����
 * @author ZCD1300
 * @Time 2021��12��26��
*/

