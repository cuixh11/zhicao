#include "BH1750.h"

uint8_t mcy=0;
uint8_t BUF[3];

/**
 * @brief ��ʼ�ź�
 * @param	void
 * @retval void
 * @author LDX2000
 * @Time 2021��11��15��
*/
void BH1750_Start()
{
    HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_SET);                    //����������
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);                   //����ʱ����
    delay_us(5);                 
    HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_RESET);                    //�����½���
    delay_us(5);                
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_RESET);                    //����ʱ����
}


/**
 * @brief ֹͣ�ź�
 * @param	void
 * @retval void
 * @author LDX2000
 * @Time 2021��11��15��
*/
void BH1750_Stop()
{
    HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_RESET);                   //����������
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);                      //����ʱ����
    delay_us(5);                 
    HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_SET);                    //����������
    delay_us(5);                 
}

/**
 * @brief ��ʼ��BH1750
 * @param	void
 * @retval void
 * @author LDX2000
 * @Time 2021��11��15��
*/
void Init_BH1750()
{
		BH1750_Start();                                                 //��ʼ�ź�
		BH1750_SendByte(SlaveAddress);                                  //�����豸��ַ+д�ź�
		BH1750_SendByte(0x01);                                  //�ڲ��Ĵ�����ַ
		BH1750_Stop();                                                  //ֹͣ�ź�
	
}


/**
 * @brief ��������BH1750�ڲ�����
 * @param	void
 * @retval BUF[3] ����������
 * @author LDX2000
 * @Time 2021��11��15��
*/
uint16_t mread(void)
{   
	  uint8_t i;	
    BH1750_Start();                          //��ʼ�ź�
    BH1750_SendByte(SlaveAddress+1);         //�����豸��ַ+���ź�
	
	 for (i=0; i<3; i++)                      //������ȡ6����ַ���ݴ洢��BUF
    {
        BUF[i] = BH1750_RecvByte();          //BUF[0]�洢����
        if (i == 3)
        {
           BH1750_SendACK(1);                //���һ��������Ҫ��NOACK
        }
        else
        {		
          BH1750_SendACK(0);                //��ӦACK
        }
   }
 
    BH1750_Stop();                          //ֹͣ�ź�
    Delay_mms(5);

}

/**
 * @brief �������������ݲ�������ն�
 * @param	void
 * @retval Value_GY_30  ���ն�
 * @author LDX2000
 * @Time 2021��11��15��
*/
uint32_t Value_GY30(void)
{
		uint16_t dis_data;
	  uint16_t Value_GY_30;
    Single_Write_BH1750(0x01);   // power on
    Single_Write_BH1750(0x10);   // H- resolution mode   
    HAL_Delay(180);            //��ʱ180ms                    
    mread();       //������������,�洢��BUF��
    dis_data=BUF[0];
    dis_data=(dis_data<<8)+BUF[1];//�ֽںϳ�����   
    Value_GY_30=(float)dis_data/1.2;
    return Value_GY_30;
}




/**
 * @brief ��ʱ����
 * @param	void
 * @retval void
 * @author LDX2000
 * @Time 2021��11��15��
*/


//ϵͳ��Ƶ72MHZ
void delay_us(uint16_t us)
{
	while(us--)
	{
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();
	}
}
 
 
void Delay_mms(uint16_t tmp)
{
	  uint16_t i=0;
	  while(tmp--)
	  {
		i=12000;
		while(i--);
    }
}

/**************************************
����Ӧ���ź�
��ڲ���:ack (0:ACK 1:NAK)
**************************************/
void BH1750_SendACK(int ack)
{
			GPIO_InitTypeDef GPIO_InitStruct;
	
  GPIO_InitStruct.Pin = scl|sda;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 
    if(ack == 1)   //дӦ���ź�
			HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_SET); 
		else if(ack == 0)
			HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_RESET);
		else
			return;
			
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);      //����ʱ����
    delay_us(5);               
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_RESET);      //����ʱ����
    delay_us(5);               
}

/**************************************
����Ӧ���ź�
**************************************/
int BH1750_RecvACK()
{		
		
	  GPIO_InitTypeDef GPIO_InitStruct;
	
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  /*����һ��Ҫ���ó���������,�����ܶ�������*/
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = sda;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 	
	
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);            //����ʱ����
    delay_us(5);                
	
	  if(HAL_GPIO_ReadPin( GPIOB, sda ) == 1 )//��Ӧ���ź�
        mcy = 1 ;  
    else
        mcy = 0 ;			
	
    HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_RESET);                    //����ʱ����
    delay_us(5);               
  
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
	
    return mcy;
}

/**************************************
��IIC���߷���һ���ֽ�����
**************************************/
void BH1750_SendByte(uint8_t dat)
{
    uint8_t i;
 
    for (i=0; i<8; i++)         //8λ������
      {
				if( 0X80 & dat )
          HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_SET);
        else
          HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_RESET);
			 
				dat <<= 1;
        HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);               //����ʱ����
        delay_us(5);             
        HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_RESET);                //����ʱ����
        delay_us(5);            
      }
    BH1750_RecvACK();
}

//���Ƕ�BH1750���������ʱ��,��Ҫ�ȷ���������ַ+д��λ,Ȼ����ָ��
//��ȡ���ݵ�ʱ��,��Ҫ�ȷ���������ַ+��ȡλ,Ȼ���������������ֽ�����

//д��ָ��
void Single_Write_BH1750(uint8_t REG_Address)//REG_Address��Ҫд���ָ��
{
	BH1750_Start();                  //��ʼ�ź�
	BH1750_SendByte(SlaveAddress);  //�����豸��ַ+д�ź�
	BH1750_SendByte(REG_Address);   //д��ָ��,�ڲ��Ĵ�����ַ
	BH1750_Stop();                   //����ֹͣ�ź�
}



/**************************************
��IIC���߶�ȡһ���ֽ�����
**************************************/
uint8_t BH1750_RecvByte()
{
    uint8_t i;
    uint8_t dat = 0;
	  uint8_t bit;
	  
	 GPIO_InitTypeDef GPIO_InitStruct;
	
	 GPIO_InitStruct.Mode = GPIO_MODE_INPUT;   /*����һ��Ҫ���ó���������,�����ܶ�������*/
   GPIO_InitStruct.Pin = sda;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
	
    HAL_GPIO_WritePin(GPIOB, sda,GPIO_PIN_SET);          //ʹ���ڲ�����,׼����ȡ����,
    for (i=0; i<8; i++)         //8λ������
    {
        dat <<= 1;
        HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_SET);               //����ʱ����
        delay_us(5);            
			
			  if( SET == HAL_GPIO_ReadPin( GPIOB, sda ) )
             bit = 0X01;
       else
             bit = 0x00;  
			
        dat |= bit;             //������   
			
        HAL_GPIO_WritePin(GPIOB, scl,GPIO_PIN_RESET);                //����ʱ����
        delay_us(5);           
    }
		
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
    return dat;
}


