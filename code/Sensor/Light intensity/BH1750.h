#ifndef __BH1750_H
#define __BH1750_H
#include "main.h"
#define scl GPIO_PIN_6
#define sda GPIO_PIN_7

#define SlaveAddress 0x46  //ADDR��GND
#define BH1750_PWR_DOWN     0x00 // �ر�ģ��
#define BH1750_PWR_ON       0x01 // ��ģ��ȴ���������
#define BH1750_RST          0x07 // �������ݼĴ���ֵ��PowerOnģʽ����Ч
#define BH1750_CON_H        0x10 // �����߷ֱ���ģʽ,1lx,120ms
#define BH1750_CON_H2       0x11 // �����߷ֱ���ģʽ.5lx,120ms
#define BH1750_CON_L        0x13 // �����ͷֱ���ģʽ�,4lx,16ms
#define BH1750_ONE_H        0x20 // һ�θ߷ֱ���ģʽ,1lx,120ms,������ģ��ת��PowerDownģʽ
#define BH1750_ONE_H2       0x21 // һ�θ߷ֱ���ģʽ,0.5lx,120ms,������ģ��ת��PowerDownģʽ
#define BH1750_ONE_L        0x23 // һ�εͷֱ���ģʽ,4lx,16ms,������ģ��ת��PowerDownģʽ


void BH1750_Start();
void BH1750_Stop();
void Init_BH1750();
uint16_t mread(void);
uint32_t Value_GY30(void);
void delay_us(uint16_t us);
void Delay_mms(uint16_t tmp);
void BH1750_SendACK(int ack);
int BH1750_RecvACK();
void BH1750_SendByte(uint8_t dat);
uint8_t BH1750_RecvByte();
int I2C_ReadData(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pData, uint16_t dataLen);
void Single_Write_BH1750(uint8_t REG_Address);



#endif