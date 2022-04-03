/*
 * @brief  USB_hid 
 * @time 2019 11 22
 * @paramer zcd 
*/

#ifndef __USB_HID_Z_
#define __USB_HID_Z_

#include "stdint.h"
#include "usbd_customhid.h"
#include "usbd_custom_hid_if.h"
#include "usb_device.h"


/*
* @brief usb hid RxMsg process functions.
* @time2019 11 23
* @为避免注释显示错误，注释采用英文编写。In order to avoid errors in the display of notes, the notes are written in English.
* @You need to replace the data in "’usbd_custom_hid_if.c‘——‘CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END’"
   with the following macro definition
* @需要将“usbd_custom_hid_if.c”文件中的”CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END“替换为以下宏定义数据。
*/
#define USB_HID_ReportDesc \
{\
	0x06,\
	0x00,\
	0xff,\
	0x09,\
	0x01,\
	0xa1,\
	0x01,\
	0x09,\
	0x01,\
	0x15,\
	0x00,\
	0x26,\
	0xff,\
	0x00,\
	0x95,\
	0x40,\
	0x75,\
	0x08,\
	0x81,\
	0x02,\
	0x09,\
	0x01,\
	0x15,\
	0x00,\
	0x26,\
	0xff,\
	0x00,\
	0x95,\
	0x40,\
	0x75,\
	0x08,\
	0x91,\
	0x02,\
	0xC0,\
}
extern uint8_t USB_RX_IT_Flag;
extern uint8_t USB_Rx_DataBuf[64];


uint8_t usb_hid_Tx(uint8_t *report);
uint8_t USB_RX_Callback(void);
void usb_hid_Rx(uint8_t *usb_hid_RxMsg,uint16_t len);

#endif


