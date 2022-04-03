/*************笔记****************
CubeMX 
   GPlO output level       --High
   GPIO mode               --Output Open Drain
   GPIO Pull-up/Pull-down  --No pull-up and no pull-down
   Maximum output speed    --LOW
   User label              --DHT11

2应用函数:Dht11_ReadData(DHT11_Data_TypeDef *Data) 注:结构体已定义,可以直接使用以下函数
             Dht11_ReadData(&DHT11_Data); //在引用位置添加 #include "dht11.h"

***********************************/
/* 包含头文件----------------------------------------------------------------*/
#include "DHT11.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"
/* 私有宏定义 ----------------------------------------------------------------*/
#define DHT11_DQ_OUT(N) HAL_GPIO_WritePin(DHT11_GPIO_Port,DHT11_Pin,N==1?GPIO_PIN_SET:GPIO_PIN_RESET)
#define DHT11_DQ_IN HAL_GPIO_ReadPin(DHT11_GPIO_Port,DHT11_Pin)

/* 全局变量 ------------------------------------------------------------------*/
DHT11_Data_TypeDef DHT11_Data;

/*********************************************
函数名:Dht11_Delay
功能:延时us
形 参:
返回值:
备 注:
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

/* 切换引脚状态 ----------------------------------------------------------------*/
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

/* 驱动层 ----------------------------------------------------------------*/
/******************
函数名:Dht11_Init
功能:初始化DHT11的IO口 DQ 同时检测DHT11的存在
返回值:1--不存在  0--存在
*******************/
uint8_t Dht11_Init(void)
{
    Dht11_StartSignal();

    return Dht11_ResponseSignal();
}

/******************
函数名:Dht11_StartSignal
功能:DHT11起始信号
*******************/
void Dht11_StartSignal(void)
{
    DHT11_Out_init();
    DHT11_DQ_OUT(0);   //拉低DQ
    HAL_Delay(18);       //拉低至少18ms
    DHT11_DQ_OUT(1);   //拉高DQ
    DHT11_In_init();
    Dht11_Delay(40);   //主机拉高20~40us

}
/******************
函数名:Dht11_ResponseSignal
功能:DHT11响应信号,等待DHT11的回应
返回值:1--未检测到DHT11的存在 0--存在
*******************/
uint8_t Dht11_ResponseSignal(void)
{
    uint8_t retry = 0;

    /*轮询直到从机发出 的80us 低电平 响应信号结束*/
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
    /*轮询直到从机发出 80us 高电平 标置信号结束*/
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
函数名:Dht11_Reset
功能:从DHT11读取一个位
返回值:1/0
***************************************/
uint8_t Dht11_ReadBit(void)
{
    uint8_t retry = 0;
    /*每bit以50us低电平开始,轮询直到从机发出 的50us 低电平 结束*/
    while(DHT11_DQ_IN == GPIO_PIN_RESET && retry < 100)
    {
        retry++;
        Dht11_Delay(1);
    };

    /*DHT11 以26~28us的高电平表示0，以70us高电平表示1
     *通过检测 x us后的电平可以区别这两个状态 ,x 即下面的延时
     */
    Dht11_Delay(40); //延时x us 这个延时需要大于数据0持续的时间即可

    if(DHT11_DQ_IN == GPIO_PIN_SET) /* x us后仍为高电平表示数据"1" */
    {
        /* 等待数据1的高电平结束 */
        retry = 0;
        while(DHT11_DQ_IN == GPIO_PIN_SET && retry < 100)
        {
            retry++;
            Dht11_Delay(1);
        };
        return 1;
    }
    else         // x us后为低电平表示数据"0"
    {
        return 0;
    }

}


/**************************************
函数名:Dht11_Reset
功能:从DHT11读取一个字节
返回值:读到的数据
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
函数名:Dht11_Reset
功能:从DHT11读取一次数据
返回值:0,正常;1,读取失败

数据格式:8bit湿度整数数据+8bit湿度小数数据
        +8bit温度整数数据+8bit温度小数数据
        +8bit校验和

temp:温度值(范围:0~50℃)
humi:湿度值(范围:20%~90%)
***************************************/
uint8_t Dht11_ReadData(DHT11_Data_TypeDef *Data)
{
    Dht11_StartSignal();//发送起始信号
    /*判断从机是否有低电平响应信号 如不响应则跳出,响应则向下运行*/
    if(DHT11_DQ_IN == GPIO_PIN_RESET)
    {
        if(Dht11_ResponseSignal() == 0)//接收器件响应信号
        {
            Data->humi_high8bit = Dht11_ReadByte();
            Data->humi_low8bit  = Dht11_ReadByte();
            Data->temp_high8bit = Dht11_ReadByte();
            Data->temp_low8bit  = Dht11_ReadByte();
            Data->check_sum     = Dht11_ReadByte();
            if((Data->humi_high8bit + Data->humi_low8bit + Data->temp_high8bit + Data->temp_low8bit) == Data->check_sum ) //校验和
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
			printf("温度:%2.2f℃   湿度:%2.2f%%\r\n",DHT11_Data.temperature, DHT11_Data.humidity);
    }
}









