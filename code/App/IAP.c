/**
 * @brief IAP ����ʵ�֣��˷�����IAP���أ�flash���ز�������APP��
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2022��3��25��
*/
#include "IAP.h"
#include "OTA.h"


uint8_t IAP_packageBuff[1024+PackageAddtINFO_Size]={0};//��ŷְ����͹�����Bin�ļ�����С��1K+PackageAddtINFO_Size�ֽڵ�������Ϣ�Լ�ԣ��
uint32_t FlashOperatePtr=FirmwareDownloadAreaFlash_StartPtr+FirmwareINFO_Offset;//Flash���������е�ָ�룬ָ��̼����ȵ����λ
FirmwareINFO_t FirmwareINFO={0};//�̼���Ϣ�洢�ṹ��



/**
 * @brief д�ڴ��еĹ̼����ݵ�Flash
 * @param	*packageBuff_t	buffInfoSize_t	���ݵ�ַָ�롢���ݴ�С(��λ �ֽ� ��ʵ��д��ת������-32λ)
 * @retval void
 * @author ZCD1300 
 * @Time 2022��3��25��
 * @WatchOut @ 
			       @ �˺����ɶ�OTP��������ֻ��д��0�����ܲ�����
			       @ WriteAddr������4�ı���,��ʵд���ַ��д����ָ��ҲҪ��4�ı���
*/

uint8_t Write_FirmwarePackageBuff_To_Flash(uint8_t *packageBuff_t,uint32_t buffInfoSize_t)
{
	uint32_t ByteToWord_CNT=0;//ʵ��д���������Flashֻ�ܰ���д�룻
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
 * @brief ��ȡFlash�еĹ̼����ݵ��ڴ�
 * @param :FirmwareStartAddr, Read_Len_t, *RAM_pBuff ���̼���ʼ��ȡ�Ĵ洢��ַ����ȡ���ȣ���ȡ��洢��ַ
 * @retval uint8_t ��ȡ��� 1-��ȡ���� 0-������ȡ 2-û����Ч�Ĺ̼���Ϣ
 * @author ZCD1300 
 * @Time 2022��3��26��
*/
uint8_t Read_FirmwarePackage_To_RAMBuff(uint32_t FirmwareStartAddr,uint32_t Read_Len_t,uint32_t * RAM_pBuff)
{
	if(FirmwareINFO.FirmwareSize<=0)
	{//����̼������ڡ�������û�и��¹̼���Ϣ
		return 2;
	}
	if(Read_Len_t>FirmwareINFO.FirmwareSize)//��ȡ���ȳ����̼���С
	{//��ȡ���ȳ����̼���С
		return 1;
	}
	else 
	{
		Flash_Read(FirmwareStartAddr,Read_Len_t,RAM_pBuff);
		return 0; 
	}
}
/**
 * @brief ���£���ȡ���̼���Ϣ��ͨ����ȡָ������Flash���ݸ���Firmware��Ϣ
 * @param void
 * @retval uint8_t ��ȡ��� 1-����������Ч�̼� 0-������ȡ 2-�������
 * @author ZCD1300 
 * @Time 2022��3��27��
*/
uint8_t Update_FirmwareINFO(void)
{
	uint8_t FirmwareINFO_RAW[FirmwareINFO_Offset]={0};
	Flash_Read(FirmwareDownloadAreaFlash_StartPtr,FirmwareINFO_Offset,(uint32_t *)FirmwareINFO_RAW);
	if (FirmwareINFO_RAW[8]=='Y')
	{
		/* ������Ч�̼� */
		memcpy(FirmwareINFO.FirmwareType,FirmwareINFO_RAW,8+4+4+16);
		//memcpy(FirmwareINFO.FirmwareType,FirmwareINFO_RAW,8);
		//memcpy(FirmwareINFO.FirmwareVersion,FirmwareINFO_RAW+8,4);
		//memcpy(FirmwareINFO.FirmwareSize,FirmwareINFO_RAW+8+4,4);
		//memcpy(FirmwareINFO.Firmware_MD5,FirmwareINFO_RAW+8+4+4,16);

		return 0;
	}
	else if (FirmwareINFO_RAW[8]=='N')
	{
		/* ����Ч�̼� */
		return 1;
	}
	else
	{
		/* ����Ч�̼� */
		return 1;
	}
	return 2;
}
/**
 * @brief ��д��д�룩�̼���Ϣ��ͨ������ָ������������Flash����,����д��Firmware��Ϣ,�˹���Ӧ���ڿ�ʼ���ع̼�֮ǰ���
 * @param void
 * @retval uint8_t ��ȡ��� 1-�̼���Ϣ��Ч 0-������ȡ 2-���ݴ�С����
 * @author ZCD1300 
 * @Time 2022��3��27��
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
		return 1;//�̼���Ϣ��Ч
	}

}






