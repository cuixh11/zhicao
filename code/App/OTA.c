/**
 * @brief OTA ����ʵ���Լ��������ع��ܡ�У��֮���
 * @param	void
 * @retval void
 * @author ZCD1300 
 * @Time 2022��3��25��
*/
#include "OTA.h"

uint8_t FirmwareMD5_Target[33]={0};//�̼���ȷ��MD5ֵ��MD5Ϊ32�ֽڳ��ȣ�
uint16_t Package_NUM=0;
uint16_t Package_total=0;

