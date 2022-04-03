/*
 * @brief  USB_hid 
 * @time 2019 11 22
 * @paramer zcd 
 * @为避免注释显示错误，注释采用英文编写。In order to avoid errors in the display of notes, the notes are written in English.
*/

#include "usbd_customhid.h"
#include "usbd_custom_hid_if.h"
#include "usb_device.h"
#include "USB_HID_Z.h"
#include "usart.h"

uint8_t USB_Rx_DataBuf[64]={0};
uint8_t USB_CallBack_Flag=0;

static uint16_t len_temp=0;

static USBD_CUSTOM_HID_HandleTypeDef* hhid;


uint8_t USB_Rx_count=0;
/*
* @brief usb hid transmit functions.
* @time2019 11 22
* @Send character data,Character length is not required.
* @为避免注释显示错误，注释采用英文编写。In order to avoid errors in the display of notes, the notes are written in English.
*/
uint8_t usb_hid_Tx(uint8_t *report)
{  
	static uint16_t len=0;
	char USB_buff_t[64*3]={0};
	strcpy(USB_buff_t,(char *)report);
	len=1+strlen((char *)USB_buff_t);
	
	if((len>64)&&(len%64!=0))
	{
		len_temp=(1+len/64)*64;
		
	}
	else if(len>=64)
	{
		len_temp=len;
	}	
	else
	{
		len_temp=64;
	}
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,(uint8_t *)USB_buff_t,len_temp);
	
	return HAL_OK;
}

/*
* @brief usb hid Callback functions.
* @time2019 11 22
* @This function needs to be called in the "CUSTOM_HID_OutEvent_FS".It is in "usbd_custom_hid_if.c"
* @为避免注释显示错误，注释采用英文编写。In order to avoid errors in the display of notes, the notes are written in English.
*/
uint8_t USB_RX_Callback(void)
{
	USB_CallBack_Flag=1;
	uint32_t USB_Received_Count=0;
	USB_Received_Count = USBD_GetRxCount( &hUsbDeviceFS,CUSTOM_HID_EPOUT_ADDR );  //第一参数是USB句柄，第二个参数的是接收的末端地址；要获取发送的数据长度的话就把第二个参数改为发送末端地址即可
    
	hhid = (USBD_CUSTOM_HID_HandleTypeDef*)hUsbDeviceFS.pClassData; 

		for(uint32_t i=0;i<USB_Received_Count;i++)
		{
		  USB_Rx_DataBuf[i] =hhid ->Report_buf[i];
		}

	{//模拟串口2被刷新，用于智巢HOST Debug功能拓展，其他工程看情况使用
		UART2_Refresh_Flag=1;
		for(uint32_t j=0;j<USB_Received_Count;j++)
		{
			Rx_Buff2[j]=USB_Rx_DataBuf[j];
		}
	}
	
	return HAL_OK;

}

//-------------------------此函数有点问题，先不要使用--------------------------------//
/*
* @brief usb hid RxMsg process functions.
* @time2019 11 23
* @This function needs to enter the array address and data length of the stored data.
* @为避免注释显示错误，注释采用英文编写。In order to avoid errors in the display of notes, the notes are written in English.
*/
//uint8_t USB_Rx_count_Flag=0;
//uint16_t USB_CallBack_LoopFlag=0;
//static	uint16_t len_Rx_temp=0;
//void usb_hid_Rx(uint8_t *usb_hid_RxMsg,uint16_t len)
//{


//	if((len>64)&&(len%64!=0))
//	{
//		len_Rx_temp=(1+len/64)*64;
//		
//	}
//	else if(len>=64)
//	{
//		len_Rx_temp=len;
//	}	
//	else
//	{
//		len_Rx_temp=64;
//	}
//	USB_Rx_count=len_Rx_temp/64;

////	while(USB_Rx_count_Flag<10)
////	{
//	usb_rx_restart:
//		if(USB_CallBack_Flag==0x01)
//		{	
//			for(uint8_t j=0;j<64;j++)
//			{
//				usb_hid_RxMsg[j+USB_CallBack_LoopFlag]=USB_Rx_DataBuf[j];
//			}
//			USB_CallBack_LoopFlag=USB_CallBack_LoopFlag+64;
//			USB_CallBack_Flag=0;
//			USB_Rx_count_Flag++;
//			if(USB_Rx_count_Flag!=USB_Rx_count)
//			{ goto usb_rx_restart;}
//			
//		}
//		else
//		{
//			//USB_CallBack_Flag=2;
//		}

////	}	
//	USB_Rx_count_Flag=0;
//	USB_CallBack_LoopFlag=0;

//}

