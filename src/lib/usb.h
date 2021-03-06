/* 
 * File:   usb.h
 * Author: mikha
 *
 * Created on October 31, 2021, 1:27 PM
 */

#ifndef USB_H
#define	USB_H

#include <cstring>
#include "device.h"


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

	typedef struct __attribute__ ((packed)) {
		uint8_t bLength; // 7
		uint8_t bDescriptorType; // 0x05
		uint8_t bEndpointAddress;
		uint8_t bmAttributes;
		uint16_t wMaxPacketSize;
		uint8_t bInterval;
	} usb_descriptor_endpoint;

	typedef struct __attribute__ ((packed)) {
		uint8_t bLength; // 9
		uint8_t bDescriptorType; // 0x04
		uint8_t bInterfaceNumber;
		uint8_t bAlternateSetting;
		uint8_t bNumEndpoints;
		uint8_t bInterfaceClass;
		uint8_t bInterfaceSubclass;
		uint8_t bInterfaceProtocol;
		uint8_t iInterface;
		usb_descriptor_endpoint ENDPOINTS[2];  // TODO: make dynamic size
	}
	usb_descriptor_interface;

	typedef struct __attribute__ ((packed)) {
		uint8_t bLength; // 9
		uint8_t bDescritptorType; // 0x02
		uint16_t wTotalLength;
		uint8_t bNumInterfaces;
		uint8_t bConfigurationValue;
		uint8_t iConfiguration;
		uint8_t bmAttributes;
		uint8_t bMaxPower;
		usb_descriptor_interface INTERFACES[1];  // TODO: make dynamic size
	}
	usb_descriptor_configuration;

	typedef struct __attribute__ ((packed)) {
		uint8_t bLength;
		uint8_t bDescriptorType; // 0x03
		const char16_t bString[30];
	} usb_descriptor_string;

	extern usb_descriptor_device_registers_t EPDESCTBL[];

	extern usb_descriptor_device DESCRIPTOR_DEVICE;
	extern usb_descriptor_configuration DESCRIPTOR_CONFIGURATION[];
	extern usb_descriptor_string DESCRIPTOR_STRING[];


	void init();
	void write(const uint8_t* data, uint8_t len);
	void read(uint8_t* data, uint8_t len);
}

#endif	/* USB_H */

