#ifndef __DHT11_H
#define __DHT11_H 
#include "main.h"

/************************ DHT11 �������Ͷ���******************************/
typedef struct
{
        uint8_t  humi_high8bit;                 //ʪ�ȸ�8λ
        uint8_t  humi_low8bit;                  //ʪ�ȵ�8λ
        uint8_t  temp_high8bit;                 //�¶ȸ�8λ
        uint8_t  temp_low8bit;                  //�¶ȵ�8λ
        uint8_t  check_sum;                     //У���
        float    humidity;                      //ʵ��ʪ��
        float    temperature;                   //ʵ���¶� 
} DHT11_Data_TypeDef;

extern DHT11_Data_TypeDef DHT11_Data;

/* �������� ------------------------------------------------------------------*/
uint8_t Dht11_Init(void);//��ʼ��DHT11

uint8_t Dht11_ReadData(DHT11_Data_TypeDef *Data);//��ȡ��ʪ��

uint8_t Dht11_ReadByte(void);//����һ���ֽ�
uint8_t Dht11_ReadBit(void);//����һ��λ

uint8_t Dht11_ResponseSignal(void);//DHT11��Ӧ�ź�
void Dht11_StartSignal(void);//DHT11��ʼ�ź�

void Dht11_Show(void);



#endif

