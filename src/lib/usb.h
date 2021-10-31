/* 
 * File:   usb.h
 * Author: mikha
 *
 * Created on October 31, 2021, 1:27 PM
 */

#ifndef USB_H
#define	USB_H


namespace usb {

	typedef struct {
		uint8_t bLength; // 18
		uint8_t bDescriptorType; // 0x01
		uint16_t bcdUSB; // 0x0210
		uint8_t bDeviceClass;
		uint8_t bDeviceSubclass;
		uint8_t bDeviceProtocol;
		uint8_t bMaxPacketSize;
		uint16_t idVendor;
		uint16_t idProduct;
		uint16_t bcdDevice;
		uint8_t iManufacturer;
		uint8_t iProduct;
		uint8_t iSerialNumber;
		uint8_t bNumConfigurations;
	} usb_descriptor_device;

	typedef struct {
		uint8_t bLength; // 9
		uint8_t bDescritptorType; // 0x02
		uint16_t wTotalLength;
		uint8_t bNumInterfaces;
		uint8_t bConfigurationValue;
		uint8_t iConfiguration;
		uint8_t bmAttributes;
		uint8_t bMaxPower;
	} usb_descriptor_configuration;

	typedef struct {
		uint8_t bLength; // 9
		uint8_t bDescriptorType; // 0x04
		uint8_t bInterfaceNumber;
		uint8_t bAlternateSetting;
		uint8_t bNumEndpoints;
		uint8_t bInterfaceClass;
		uint8_t bInterfaceSubclass;
		uint8_t bInterfaceProtocol;
		uint8_t iInterface;
	} usb_descriptor_interface;

	typedef struct {
		uint8_t bLength; // 7
		uint8_t bDescriptorType; // 0x05
		uint8_t bEndpointAddress;
		uint8_t bmAttributes;
		uint8_t wMaxPacketSize;
		uint8_t bInterval;
	} usb_descriptor_endpoint;

	typedef struct {
		uint8_t bLength;
		uint8_t bDescriptorType; // 0x03
		uint8_t wLANGID[];
	} usb_descriptor_string0;

	typedef struct {
		uint8_t bLength;
		uint8_t bDescriptorType; // 0x03
		unsigned char* bString;
	} usb_descriptor_string;
	
	extern usb_descriptor_device_registers_t EPDESCTBL[];
	
	extern usb_descriptor_device DESCRIPTOR_DEVICE;
	extern usb_descriptor_configuration DESCRIPTOR_CONFIGURATION[];
	extern usb_descriptor_interface DESCRIPTOR_INTERFACE[];
	extern usb_descriptor_endpoint DESCRIPTOR_ENDPOINT[];
	extern usb_descriptor_string0 DESCRIPTOR_STRING0;
	extern usb_descriptor_string DESCRIPTOR_STRING[];


	void init();
}

#endif	/* USB_H */

