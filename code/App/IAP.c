/**
 * @brief IAP 功能实现，此方案是IAP下载，flash下载操作在主APP中
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2022年3月25日
*/
#include "IAP.h"
#include "OTA.h"


uint8_t IAP_packageBuff[1024+PackageAddtINFO_Size]={0};//存放分包发送过来的Bin文件，大小是1K+PackageAddtINFO_Size字节的其他信息以及裕量
uint32_t FlashOperatePtr=FirmwareDownloadAreaFlash_StartPtr+FirmwareINFO_Offset;//Flash操作过程中的指针，指向固件进度的最高位
FirmwareINFO_t FirmwareINFO={0};//固件信息存储结构体



/**
 * @brief 写内存中的固件数据到Flash
 * @param	*packageBuff_t	buffInfoSize_t	数据地址指针、数据大小(单位 字节 ，实际写入转换成字-32位)
 * @retval void
 * @author ZCD1300 
 * @Time 2022年3月25日
 * @WatchOut @ 
			       @ 此函数可对OTP区操作（只能写入0，不能擦除）
			       @ WriteAddr必须是4的倍数,其实写入地址和写入后的指针也要是4的倍数
*/

uint8_t Write_FirmwarePackageBuff_To_Flash(uint8_t *packageBuff_t,uint32_t buffInfoSize_t)
{
	uint32_t ByteToWord_CNT=0;//实际写入的字数，Flash只能按字写入；
	ByteToWord_CNT=buffInfoSize_t/4;
	if(buffInfoSize_t%4==0)
	{
		Flash_Write(FlashOperatePtr,ByteToWord_CNT,(uint32_t *)IAP_packageBuff);
		FlashOperatePtr+=ByteToWord_CNT;		
	}
	else
	{
		return 1;
//		ByteToWord_CNT=ByteToWord_CNT+1;
//		Flash_Write(FlashOperatePtr,ByteToWord_CNT,(uint32_t *)IAP_packageBuff);
//		FlashOperatePtr+=ByteToWord_CNT;			
	}

	return 0;
}
/**
 * @brief 读取Flash中的固件数据到内存
 * @param :FirmwareStartAddr, Read_Len_t, *RAM_pBuff ；固件开始读取的存储地址，读取长度，读取后存储地址
 * @retval uint8_t 读取结果 1-读取错误 0-正常读取 2-没有有效的固件信息
 * @author ZCD1300 
 * @Time 2022年3月26日
*/
uint8_t Read_FirmwarePackage_To_RAMBuff(uint32_t FirmwareStartAddr,uint32_t Read_Len_t,uint32_t * RAM_pBuff)
{
	if(FirmwareINFO.FirmwareSize<=0)
	{//如果固件不存在、或者是没有更新固件信息
		return 2;
	}
	if(Read_Len_t>FirmwareINFO.FirmwareSize)//读取长度超过固件大小
	{//读取长度超过固件大小
		return 1;
	}
	else 
	{
		Flash_Read(FirmwareStartAddr,Read_Len_t,RAM_pBuff);
		return 0; 
	}
}
/**
 * @brief 更新（读取）固件信息，通过读取指定区域Flash内容更新Firmware信息
 * @param void
 * @retval uint8_t 读取结果 1-下载区无有效固件 0-正常读取 2-意外错误
 * @author ZCD1300 
 * @Time 2022年3月27日
*/
uint8_t Update_FirmwareINFO(void)
{
	uint8_t FirmwareINFO_RAW[FirmwareINFO_Offset]={0};
	Flash_Read(FirmwareDownloadAreaFlash_StartPtr,FirmwareINFO_Offset,(uint32_t *)FirmwareINFO_RAW);
	if (FirmwareINFO_RAW[8]=='Y')
	{
		/* 存在有效固件 */
		memcpy(FirmwareINFO.FirmwareType,FirmwareINFO_RAW,8+4+4+16);
		//memcpy(FirmwareINFO.FirmwareType,FirmwareINFO_RAW,8);
		//memcpy(FirmwareINFO.FirmwareVersion,FirmwareINFO_RAW+8,4);
		//memcpy(FirmwareINFO.FirmwareSize,FirmwareINFO_RAW+8+4,4);
		//memcpy(FirmwareINFO.Firmware_MD5,FirmwareINFO_RAW+8+4+4,16);

		return 0;
	}
	else if (FirmwareINFO_RAW[8]=='N')
	{
		/* 无有效固件 */
		return 1;
	}
	else
	{
		/* 无有效固件 */
		return 1;
	}
	return 2;
}
/**
 * @brief 复写（写入）固件信息，通过擦除指定的整个区块Flash内容,重新写入Firmware信息,此功能应该在开始下载固件之前完成
 * @param void
 * @retval uint8_t 读取结果 1-固件信息无效 0-正常读取 2-数据大小错误
 * @author ZCD1300 
 * @Time 2022年3月27日
*/
uint8_t Wirte_firmwareINFO(FirmwareINFO_t NewFirmwareINFO_t)
{
	
	if((FirmwareINFO_Size<4)||(FirmwareINFO_Size>FirmwareINFO_Offset))
	{
		return 2;
	}
	if(NewFirmwareINFO_t.FirmwareVersion[0]=='Y')
	{
		Flash_Write(FirmwareDownloadAreaFlash_StartPtr,FirmwareINFO_Size,(uint32_t *)&NewFirmwareINFO_t);
		return 0;
	}
	else
	{
		return 1;//固件信息无效
	}

}






