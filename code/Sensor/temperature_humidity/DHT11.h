#ifndef __DHT11_H
#define __DHT11_H 
#include "main.h"

/************************ DHT11 数据类型定义******************************/
typedef struct
{
        uint8_t  humi_high8bit;                 //湿度高8位
        uint8_t  humi_low8bit;                  //湿度低8位
        uint8_t  temp_high8bit;                 //温度高8位
        uint8_t  temp_low8bit;                  //温度低8位
        uint8_t  check_sum;                     //校验和
        float    humidity;                      //实际湿度
        float    temperature;                   //实际温度 
} DHT11_Data_TypeDef;

extern DHT11_Data_TypeDef DHT11_Data;

/* 函数声明 ------------------------------------------------------------------*/
uint8_t Dht11_Init(void);//初始化DHT11

uint8_t Dht11_ReadData(DHT11_Data_TypeDef *Data);//读取温湿度

uint8_t Dht11_ReadByte(void);//读出一个字节
uint8_t Dht11_ReadBit(void);//读出一个位

uint8_t Dht11_ResponseSignal(void);//DHT11响应信号
void Dht11_StartSignal(void);//DHT11起始信号

void Dht11_Show(void);



#endif

