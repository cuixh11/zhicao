/*************�ʼ�****************
CubeMX 
   GPlO output level       --High
   GPIO mode               --Output Open Drain
   GPIO Pull-up/Pull-down  --No pull-up and no pull-down
   Maximum output speed    --LOW
   User label              --DHT11

2Ӧ�ú���:Dht11_ReadData(DHT11_Data_TypeDef *Data) ע:�ṹ���Ѷ���,����ֱ��ʹ�����º���
             Dht11_ReadData(&DHT11_Data); //������λ����� #include "dht11.h"

***********************************/
/* ����ͷ�ļ�----------------------------------------------------------------*/
#include "DHT11.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"
/* ˽�к궨�� ----------------------------------------------------------------*/
#define DHT11_DQ_OUT(N) HAL_GPIO_WritePin(DHT11_GPIO_Port,DHT11_Pin,N==1?GPIO_PIN_SET:GPIO_PIN_RESET)
#define DHT11_DQ_IN HAL_GPIO_ReadPin(DHT11_GPIO_Port,DHT11_Pin)

/* ȫ�ֱ��� ------------------------------------------------------------------*/
DHT11_Data_TypeDef DHT11_Data;

/*********************************************
������:Dht11_Delay
����:��ʱus
�� ��:
����ֵ:
�� ע:
**********************************************/
void Dht11_Delay( uint16_t cnt)
{
    uint8_t i;

    while(cnt)
    {
        for (i = 0; i < 10; i++)
        {

        }
        cnt--;
    }
}

/* �л�����״̬ ----------------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStruct = {0};
void DHT11_Out_init()
{
    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

void DHT11_In_init()
{
    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

/* ������ ----------------------------------------------------------------*/
/******************
������:Dht11_Init
����:��ʼ��DHT11��IO�� DQ ͬʱ���DHT11�Ĵ���
����ֵ:1--������  0--����
*******************/
uint8_t Dht11_Init(void)
{
    Dht11_StartSignal();

    return Dht11_ResponseSignal();
}

/******************
������:Dht11_StartSignal
����:DHT11��ʼ�ź�
*******************/
void Dht11_StartSignal(void)
{
    DHT11_Out_init();
    DHT11_DQ_OUT(0);   //����DQ
    HAL_Delay(18);       //��������18ms
    DHT11_DQ_OUT(1);   //����DQ
    DHT11_In_init();
    Dht11_Delay(40);   //��������20~40us

}
/******************
������:Dht11_ResponseSignal
����:DHT11��Ӧ�ź�,�ȴ�DHT11�Ļ�Ӧ
����ֵ:1--δ��⵽DHT11�Ĵ��� 0--����
*******************/
uint8_t Dht11_ResponseSignal(void)
{
    uint8_t retry = 0;

    /*��ѯֱ���ӻ����� ��80us �͵�ƽ ��Ӧ�źŽ���*/
    while(DHT11_DQ_IN == GPIO_PIN_RESET && retry < 100)
    {
        retry++;
        Dht11_Delay(1);
    };
    if(retry >= 100)
    {
        return 1;
    }
    retry = 0;
    /*��ѯֱ���ӻ����� 80us �ߵ�ƽ �����źŽ���*/
    while(DHT11_DQ_IN == GPIO_PIN_SET && retry < 100)
    {
        retry++;
        Dht11_Delay(1);
    };
    if(retry >= 100)
    {
        return 1;
    }
    return 0;
}



/**************************************
������:Dht11_Reset
����:��DHT11��ȡһ��λ
����ֵ:1/0
***************************************/
uint8_t Dht11_ReadBit(void)
{
    uint8_t retry = 0;
    /*ÿbit��50us�͵�ƽ��ʼ,��ѯֱ���ӻ����� ��50us �͵�ƽ ����*/
    while(DHT11_DQ_IN == GPIO_PIN_RESET && retry < 100)
    {
        retry++;
        Dht11_Delay(1);
    };

    /*DHT11 ��26~28us�ĸߵ�ƽ��ʾ0����70us�ߵ�ƽ��ʾ1
     *ͨ����� x us��ĵ�ƽ��������������״̬ ,x ���������ʱ
     */
    Dht11_Delay(40); //��ʱx us �����ʱ��Ҫ��������0������ʱ�伴��

    if(DHT11_DQ_IN == GPIO_PIN_SET) /* x us����Ϊ�ߵ�ƽ��ʾ����"1" */
    {
        /* �ȴ�����1�ĸߵ�ƽ���� */
        retry = 0;
        while(DHT11_DQ_IN == GPIO_PIN_SET && retry < 100)
        {
            retry++;
            Dht11_Delay(1);
        };
        return 1;
    }
    else         // x us��Ϊ�͵�ƽ��ʾ����"0"
    {
        return 0;
    }

}


/**************************************
������:Dht11_Reset
����:��DHT11��ȡһ���ֽ�
����ֵ:����������
***************************************/
uint8_t Dht11_ReadByte(void)
{
    uint8_t i, dat;
    dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= Dht11_ReadBit();
    }
    return dat;
}


/**************************************
������:Dht11_Reset
����:��DHT11��ȡһ������
����ֵ:0,����;1,��ȡʧ��

���ݸ�ʽ:8bitʪ����������+8bitʪ��С������
        +8bit�¶���������+8bit�¶�С������
        +8bitУ���

temp:�¶�ֵ(��Χ:0~50��)
humi:ʪ��ֵ(��Χ:20%~90%)
***************************************/
uint8_t Dht11_ReadData(DHT11_Data_TypeDef *Data)
{
    Dht11_StartSignal();//������ʼ�ź�
    /*�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ������,��Ӧ����������*/
    if(DHT11_DQ_IN == GPIO_PIN_RESET)
    {
        if(Dht11_ResponseSignal() == 0)//����������Ӧ�ź�
        {
            Data->humi_high8bit = Dht11_ReadByte();
            Data->humi_low8bit  = Dht11_ReadByte();
            Data->temp_high8bit = Dht11_ReadByte();
            Data->temp_low8bit  = Dht11_ReadByte();
            Data->check_sum     = Dht11_ReadByte();
            if((Data->humi_high8bit + Data->humi_low8bit + Data->temp_high8bit + Data->temp_low8bit) == Data->check_sum ) //У���
            {
                Data->humidity = (Data->humi_high8bit * 100 + Data->humi_low8bit) / 100.00;
                Data->temperature = (Data->temp_high8bit * 100 + Data->temp_low8bit) / 100.00;
            }
            else
            {
                return 1;
            }
        }
    }
    else
    {
        return 1;
    }
    return 0;
}

void Dht11_Show(void)
{
    uint8_t  TxBuffer[32];
    if(Dht11_ReadData(&DHT11_Data))
    {
        printf("error\r\n");
    }
    else
    {
			printf("�¶�:%2.2f��   ʪ��:%2.2f%%\r\n",DHT11_Data.temperature, DHT11_Data.humidity);
    }
}









