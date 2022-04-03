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
 * @brief 跳转APP函数实体
 * @param	uint32_t APP_addr App地址
 * @retval uint8_t 
 * @author ZCD1300
 * @Time 2021年12月22日
*/
	uint32_t APP_Addr_t=0;	
typedef void (*jumpAPP)(void);
jumpAPP JumpAppStart;
uint8_t LoadAPP(uint32_t APP_addr)
{

	APP_Addr_t=APP_addr+4;
	if(((*(__IO uint32_t*)APP_addr) & 0x2FFE0000 ) == 0x20000000)//检查APP栈指针是否合法
	{
		__disable_irq();		
		JumpAppStart=(jumpAPP)*(__IO uint32_t*) APP_Addr_t;//指向函数段开始地址	
		__set_MSP(*(__IO uint32_t*) APP_Addr_t);//设置栈顶指针
		JumpAppStart();//开始跳转
	}
	else
	{
		return 1;
	}
}
/**
 * @brief APP跳转控制函数，实际应调用此函数
 * @param	void 
 * @retval int8_t return  0没有AP连接 1有AP连接
 * @author ZCD1300
 * @Time 2021年12月22日
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
 * @brief APP跳转控制函数，实际应调用此函数
 * @param	void 
 * @retval int8_t return  0没有AP连接 1有AP连接
 * @author ZCD1300
 * @Time 2021年12月26日
*/

